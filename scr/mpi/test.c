#include <ncurses.h>
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>

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

void receiveVector(int *vector){
    for (int i=0; i<3; i++){
       printf("%d \n", vector[i]);
    }
}

int main(){
    int **matrix;
    allocMemory(&matrix, 3,3);
    int cont=0;
    for (int i=0; i<3; i++){
        for (int j=0 ; j<3; j++){
            matrix[i][j] = cont;
            cont++;
        }
    }
    receiveVector(matrix[1]);
    return 0;
}