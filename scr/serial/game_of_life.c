#include <ncurses.h>
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HEIGHT 25
#define LENGTH 80
#define LIFE "*"
#define DEAD " "
#define MAXSPEED 1.9
#define MINSPEED 0.1
#define ITERSPEED 0.1
#define STARTSPEED 100000

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
void fieldOutput(char **matrix, WINDOW *win);
void changeSpeed(char button, float *speed);
void createRandomInput();

int main() {
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
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
  float speed = 1.0f; 
  char button = '\0';

  allocMemory(&matrix);
  allocMemory(&buff);
  changeStream(mode);
  fieldCreation(matrix);

  stdin = freopen("/dev/tty", "r", stdin);
  initscr();
  noecho();
  WINDOW *win = newwin(HEIGHT, LENGTH, 0, 0);
  wrefresh(win);
  fieldOutput(matrix, win);
  wrefresh(win);

  while (fieldUpdate(&matrix, &buff) && !(button == 'q' || button == 'Q')) {
    fieldOutput(matrix, win);
    mvwprintw(win, HEIGHT - 1, 5, "Speed: x%.1f", 2.0 - speed);
    halfdelay(1);
    button = wgetch(win);
    changeSpeed(button, &speed);
    usleep(STARTSPEED * speed);
    wrefresh(win);
  }
  freeMemory(matrix);
  freeMemory(buff);
  endwin();
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
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LENGTH; j++) {
      count = countAliveCells(*matrix, i, j);
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

void fieldOutput(char **matrix, WINDOW *win) {
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < LENGTH; j++) {
      if (matrix[i][j] == '1') {
        mvwprintw(win, i, j, LIFE);
        printw(LIFE);
      } else {

        mvwprintw(win, i, j, DEAD);
      }
    }
  }
}

void changeSpeed(char button, float *speed) {
  if (*speed > MINSPEED && (button == 'a' || button == 'A')) {
    *speed -= ITERSPEED;
  }
  if (*speed < MAXSPEED && (button == 'z' || button == 'Z')) {
    *speed += ITERSPEED;
  }
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
