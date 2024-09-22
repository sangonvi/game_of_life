#include <ncurses.h>
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void createRandomInput(int rows, int columns);

int main(int argc, char** argv){
    int rows = atoi(argv[1]);
	  int columns = atoi(argv[2]);
    createRandomInput(rows, columns);
    return 0;
}

void createRandomInput(int rows, int columns){
  srand(time(NULL));
  FILE *arq;
  arq = fopen("../../presets/randomInput.txt", "w");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) { 
       if (( (double)rand() / (double)RAND_MAX ) > 0.5) {
         fputc('1',arq);
       }else{
         fputc('0', arq);
       }
    }
  }  

  fclose(arq);
}