#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include<time.h>
#include <omp.h>

#define LIFE "*"
#define DEAD " "

void gameMenu();
void printMenuOptions();
void printAboutGame();
void saveScan(int *command);
void game(int mode);
void changeStream(int mode);
int allocMemory(char ***matrix);
void freeMemory(char **matrix);
void fieldCreation(char **matrix);
int fieldUpdate(char ***matrix, char ***buff);
int countAliveCells(char **matrix, int i, int j);
char cellUpdate(char cell, int count, int *changeFlag);
void printTotalGrid(char **totalGrid, int timeStep);
void createRandomInput();
void reportResults(double time);

int HEIGHT;
int LENGTH;
int TIMESTEPS;
int ACTIVE_INTERFACE;
int NUMBER_THREADS;
char MODE_PARALELL[] = "OPEN MP";
int main(int argc, char** argv) {
   HEIGHT = atoi(argv[1]);
	 LENGTH = atoi(argv[2]);
	 TIMESTEPS = atoi(argv[3]);
	 ACTIVE_INTERFACE = atoi(argv[4]);
	 NUMBER_THREADS = atoi(argv[5]);
	 
  gameMenu();
  return 0;
}

void gameMenu() {
  int command = -1;
  printAboutGame();
  printMenuOptions();
  saveScan(&command);
  game(command);
}

void printMenuOptions() {
  printf("1 - Random Input\n");
  printf("2 - Cow Pattern \n");
  printf("3 - Gun Gospy Pattern\n");
  printf("4 - Gun Sim Pattern\n");
  printf("5 - Ágar map Pattern\n");
  printf("6 - New Ship Map Pattern\n");
}

void printAboutGame() {
  printf("Game of life\n");
}

void saveScan(int *command) {
  int check = 0;
  while (!check) {
    printf("Command enter: ");
    check = scanf("%d", command);
    if (!check) {
      printf("Input error. Try again: \n");
      getchar();
    } else if (*command < 1 || *command > 6) {
      printf("Unknown commando.Try again: \n");
      check = 0;
      stdin = freopen("/dev/tty", "r", stdin);
    }
  }
}

void game(int mode) {
  char **matrix; 
  char **buff;
  
  allocMemory(&matrix);
  allocMemory(&buff);
  changeStream(mode);
  fieldCreation(matrix);

  stdin = freopen("/dev/tty", "r", stdin);
  double tstart = omp_get_wtime();  
  for (int time=1; time<=TIMESTEPS; time ++){    
    fieldUpdate(&matrix, &buff);
    if(ACTIVE_INTERFACE){
        printTotalGrid(matrix,time);
    }
  }  
  double tend = omp_get_wtime();
  double time = tend - tstart;
  reportResults(time);
  printf("Tempo total de execução %.4f segundos", time);  
             
  freeMemory(matrix);
  freeMemory(buff);
}


void changeStream(int mode) {
  switch (mode) {
  case 1:
    createRandomInput();
    stdin = freopen("../../presets/randomInput.txt", "r", stdin);
    break;
  case 2:
    stdin = freopen("../../presets/cow.txt", "r", stdin);
    break;
  case 3:
    stdin = freopen("../../presets/gunGospy.txt", "r", stdin);
    break;
  case 4:
    stdin = freopen("../../presets/gunSim.txt", "r", stdin);
    break;
  case 5:
    stdin = freopen("../../presets/agar.txt", "r", stdin);
    break;
  case 6:
    stdin = freopen("../../presets/shipNew.txt", "r", stdin);
    break;
  }
}

int allocMemory(char ***matrix) {
  int check = 1;
  (*matrix) = malloc(HEIGHT * sizeof(char *));
  if (*matrix != NULL) {
    for (int i = 0; i < HEIGHT; i++) {
      (*matrix)[i] = malloc(LENGTH * sizeof(char));
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

void freeMemory(char **matrix) {
  for (int i = 0; i < HEIGHT; i++)
    free(matrix[i]);
  free(matrix);
}

void fieldCreation(char **matrix) {
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LENGTH; j++) {
      char c;
      scanf("%c ", &c);
      if (c == '-')
        c = '0';
      if (c == 'o')
        c = '1';
      matrix[i][j] = c;
    }
  }
}

int fieldUpdate(char ***matrix, char ***buff) {
  int livCells = 0;
  int changeFlag = 0;
  int count;
  int check = 1;
  
  #pragma omp parallel for collapse(2) private(count) num_threads(NUMBER_THREADS)
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LENGTH; j++) {
      count =       (*matrix, i, j);
      livCells += count;
      (*buff)[i][j] = cellUpdate((*matrix)[i][j], count, &changeFlag);
    }
  }
  char **temp = *matrix;
  *matrix = *buff;
  *buff = temp;

  if (livCells == 0 || changeFlag == 0)
    check = 0;
  return check;
}

int countAliveCells(char **matrix, int i, int j) {
  int count = 0;
  for (int istep = -1; istep <= 1; istep++) {
    for (int jstep = -1; jstep <= 1; jstep++) {
      if ((matrix[(HEIGHT + i + istep) % HEIGHT]
                 [(LENGTH + j + jstep) % LENGTH] == '1') &&
          !(istep == 0 && jstep == 0))
        count++;
    }
  }
  return count;
}

char cellUpdate(char cell, int count, int *changeFlag) {
  char newCell;
  if (cell == '1') {
    if (count != 2 && count != 3) {
      newCell = '0';
      *changeFlag = 1;
    } else {
      newCell = '1';
    }
  } else {
    if (count == 3) {
      newCell = '1';
      *changeFlag = 1;
    } else {
      newCell = '0';
    }
  }
  return newCell;
}


void createRandomInput(){
  FILE *arq;
  arq = fopen("../../presets/randomInput.txt", "w");
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LENGTH; j++) { 
       if (( (double)rand() / (double)RAND_MAX ) > 0.5) {
         fputc('1',arq);
       }else{
         fputc('0', arq);
       }
    }
  }  

  fclose(arq);
}


void printTotalGrid(char **totalGrid,int timeStep){
	system("clear");
	printf("Time step %d\n", timeStep);
	for (int i=0;i<HEIGHT; i++){		
		for (int j=0; j<LENGTH; j++){
			if (totalGrid[i][j] == '1'){
				printf("%c ",'*');
			}else{
				printf("%c ",' ');
			}
			
		}
		printf("\n");
	}
	usleep(500 *1000);

}


void reportResults(double time){
  FILE *fpt;
  fpt = fopen("../report.csv", "a+");
  fprintf(fpt,"%s; %.4f; %d; %d; %d; %d\n", MODE_PARALELL, time, HEIGHT, LENGTH, TIMESTEPS, NUMBER_THREADS);
  fclose(fpt);
  
  
}

