#include <ncurses.h>
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include<time.h>

int allocMemory(int ***matrix, int nrows, int ncolumns);
void allocVectorMemory(int **vector, int ncolumns);
void getPreviousAndNextRanks(int my_rank, int *previous_rank, int *next_rank, int nprocesses);
void fieldCreation(int **matrix, int rank, int nrows, int ncolums);
int isCorner(int i, int j, int nrows, int ncolumns);
int getCorrespondingCorner(int **current_grid, int i, int j, int nrows, int ncolumns);
int getCorrespondingValue(int **current_grid, int i, int j, int nrows, int ncolumns, int *top_ghost, int *bottom_ghost, int rank);
int countNeighboursCells(int **current_grid, int i, int j, int nrows, int ncolumns, int *top_ghost, int *bottom_ghost, int rank);
void fillTotalGrid(int **completedGrid, int rank, int **gridOfRank, int rows, int columns);
int cellUpdate(int cell, int count);
void printTotalGrid(int **totalGrid, int rows, int columns, int timeStep);
void freeMemory(int **matrix, int nrows);
void reportResults(double time, int rows, int columns, int timesteps, int nprocesses);

int ACTIVE_INTERFACE;
char MODE_PARALELL[] = "MPI";

int main(int argc, char** argv) {
   	int nprocesses;
  	int my_rank;
  	int nrows;
  	int ncolumns;
	int timesteps;
    int next_rank;
	int previous_rank;
    int tag=200;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocesses);

    nrows = atoi(argv[1]);
	ncolumns = atoi(argv[2]);
	timesteps = atoi(argv[3]);
	ACTIVE_INTERFACE = atoi(argv[4]);

	int rows_per_process = nrows/nprocesses;
	int **current_grid;
	int **next_grid;
	int **total_grid;
	int *top_ghost;
	int *bottom_ghost;
    double startTime, endTime;
    
	allocMemory(&current_grid, rows_per_process, ncolumns);
	allocMemory(&next_grid, rows_per_process, ncolumns);
    allocVectorMemory(&top_ghost,ncolumns);
	allocVectorMemory(&bottom_ghost,ncolumns);

	if (my_rank==0){
		allocMemory(&total_grid, nrows, ncolumns);
	}

	getPreviousAndNextRanks(my_rank, &previous_rank, &next_rank, nprocesses);
	fieldCreation(current_grid, my_rank, rows_per_process, ncolumns); 
	 
	double elapsedTime;
	startTime = MPI_Wtime();
	for (int time=1; time <= timesteps; time++){
		MPI_Send(current_grid[0], ncolumns, MPI_INT, previous_rank, tag, MPI_COMM_WORLD);
		MPI_Send(current_grid[rows_per_process-1], ncolumns, MPI_INT, next_rank, tag, MPI_COMM_WORLD);

        MPI_Recv(bottom_ghost, ncolumns, MPI_INT, next_rank, tag,MPI_COMM_WORLD, &status);
		MPI_Recv(top_ghost, ncolumns, MPI_INT, previous_rank, tag,MPI_COMM_WORLD, &status);
	
        for (int i = 0; i < rows_per_process; i++) {
			for (int j = 0; j < ncolumns; j++) {
				int count = countNeighboursCells(current_grid, i, j, rows_per_process, ncolumns, top_ghost, bottom_ghost, my_rank);	
				next_grid[i][j] = cellUpdate(current_grid[i][j], count);
			}
		}

        endTime = MPI_Wtime();
        double elapsedTime = endTime - startTime;
        *current_grid = *next_grid;

        if (ACTIVE_INTERFACE){
			if (my_rank == 0){
				int ** temp;
				allocMemory(&temp,rows_per_process, ncolumns);

				fillTotalGrid(total_grid ,my_rank, next_grid, rows_per_process, ncolumns);
				for (int rank=1; rank<nprocesses; rank++){
					MPI_Recv(&temp[0][0], rows_per_process*ncolumns, MPI_INT, rank, tag,MPI_COMM_WORLD, &status);
						fillTotalGrid(total_grid ,rank, temp, rows_per_process, ncolumns);
				}
				freeMemory(temp, rows_per_process);
				printTotalGrid(total_grid,nrows,ncolumns,time);
			}else{
				MPI_Send(&next_grid[0][0], rows_per_process*ncolumns, MPI_INT, 0, tag, MPI_COMM_WORLD);	
			}
		}

	} 

	printf("Rank %d - Elapsed Time : %f \n", my_rank, elapsedTime);

	if (my_rank == 0){
			double elapsedTimeCurrentRank;
			for (int rank=1; rank<nprocesses; rank++){
				MPI_Recv(&elapsedTimeCurrentRank, 1, MPI_DOUBLE, rank, tag,MPI_COMM_WORLD, &status);
				if (elapsedTimeCurrentRank > elapsedTime)
					elapsedTime = elapsedTimeCurrentRank;
			}
			printf("Elapsed Time: %f \n", elapsedTime);
			reportResults(elapsedTime, nrows, ncolumns, timesteps, nprocesses);
			
	}else{
		MPI_Send(&elapsedTime,1, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);	
	}
	

	MPI_Finalize();

	return 0;
}


int allocMemory(int ***matrix, int nrows, int ncolumns) {
  int check = 1;
  (*matrix) = malloc(nrows * sizeof(int *));
  if (*matrix != NULL) {
    for (int i = 0; i < nrows; i++) {
      (*matrix)[i] = malloc(ncolumns * sizeof(int));
      if ((*matrix)[i] == NULL) {
        check = 0;
        for (int j = 0; j < i; j++) 
          free((*matrix)[i]);
        free(matrix);
        break;
      }
    }
  } else {
    check = 0;
  }
  return check;
}

void allocVectorMemory(int **vector, int ncolums){
	(*vector) = malloc(ncolums * sizeof(int *));
}

void freeMemory(int **matrix,int nrows) {
  for (int i = 0; i < nrows; i++)
    free(matrix[i]);
  free(matrix);
}

void getPreviousAndNextRanks(int my_rank, int *previous_rank, int *next_rank, int nprocesses){
	*next_rank = my_rank + 1;
	*previous_rank= my_rank - 1;

	if (my_rank == 0){
		*previous_rank +=nprocesses;

	}
	if (my_rank == (nprocesses-1)){
		*next_rank -= nprocesses;
	}
}

void fieldCreation(int **matrix, int rank, int nrows, int ncolumns) {
   FILE *arq;
   arq = freopen("../../presets/randomInput.txt", "r", stdin);

   int pos = rank*ncolumns*nrows;
   fseek(arq,pos, SEEK_SET);

   for (int i=0; i < nrows; i++) {
		for (int j = 0; j < ncolumns; j++) {
			char c;
			fscanf(arq,"%c ", &c);
			if (c == '0')
				matrix[i][j] = 0;
			
			if (c == '1')
				matrix[i][j] = 1;
				
		}
		printf("\n");
   }
}


int isCorner(int i, int j, int nrows, int ncolumns){
	if( (i==-1 || i==nrows) && (j==-1 || j==ncolumns)){
		return 1;
	}else{
		return 0;
	}
}

int getCorrespondingCorner(int **current_grid, int i, int j, int nrows, int ncolumns){
	if(i==-1){
		if (j==-1)
		  return current_grid[nrows-1][ncolumns-1];
		else 
			return current_grid[nrows-1][0];
     }else{
		if (j==-1)
		  return current_grid[0][ncolumns-1];
		else 
			return current_grid[0][0];
	 }
}

int getCorrespondingValue(int **current_grid, int i, int j, int nrows, int ncolumns, int *top_ghost, int *bottom_ghost, int rank){
	if(i == -1)
		return top_ghost[j];
	if (i == nrows)
		  return bottom_ghost[j];
	if (j == -1)
		  return current_grid[i][ncolumns-1];
	if (j == ncolumns)
		 return current_grid[i][0];

	return current_grid[i][j];
}

int countNeighboursCells(int **current_grid, int i, int j, int nrows, int ncolumns, int *top_ghost, int *bottom_ghost, int rank) {
  int count = 0;
  for (int index_i= i-1; index_i <= i+1; index_i++) {
    for (int index_j = j-1; index_j <= j+1; index_j++) {	
		if (i!=index_i || j!=index_j){
			if (isCorner(index_i,index_j,nrows,ncolumns)){
				count += getCorrespondingCorner(current_grid,index_i,index_j,nrows,ncolumns);
			}else{
				count += getCorrespondingValue(current_grid, index_i, index_j, nrows, ncolumns, top_ghost,bottom_ghost, rank);
			}
		}
    }
  }
  return count;
}

int cellUpdate(int cell, int count) {
  if (cell == 1) {
    if (count != 2 && count != 3) 
      return 0;
    else 
      return 1;
  } else {
    if (count == 3) 
      return 1;
    else 
      return 0;
  }
}

void fillTotalGrid(int **completedGrid, int rank, int **gridOfRank, int rows, int columns){
	int i_total = rank*rows;  
	for (int i=0;i<rows; i++){		
		for (int j=0; j<columns; j++){
			completedGrid[i_total][j] = gridOfRank[i][j];
		}
		i_total++;
	}
}

void printTotalGrid(int **totalGrid, int rows, int columns, int timeStep){
	system("clear");
	printf("Time step %d\n", timeStep);
	for (int i=0;i<rows; i++){		
		for (int j=0; j<columns; j++){
			// printf("%d ",totalGrid[i][j]);
			if (totalGrid[i][j] == 1){
				printf("%c ",'*');
			}else{
				printf("%c ",' ');
			}
			
		}
		printf("\n");
	}
	printf("\n");
	usleep(500 *1000);

}

void reportResults(double time, int rows, int columns, int timesteps, int nprocesses){
  FILE *fpt;
  fpt = fopen("../report.csv", "a+");
  fprintf(fpt,"%s,%.4f; %d; %d; %d; %d\n", MODE_PARALELL, time, rows, columns, timesteps, nprocesses);
  fclose(fpt);
  
  
}

