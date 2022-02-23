#include <stdio.h>
#include <stdbool.h>

#define SIZEX 5
#define SIZEY 5

bool eval(char (*maze)[SIZEX][SIZEY], bool (*dead)[SIZEX][SIZEY], int x, int y) {
  for (int i = 0; i < SIZEY; i++) {
    for (int j = 0; j < SIZEX; j++) {
      if (j == x && i == y) {
	printf("@");
      } else {
	printf("%c", (*maze)[j][i]);
      }
    }
    printf("\n");
  }
  printf("\n");
  
  (*dead)[x][y] = true;
  int next[4] = {0, 0, 0, 0};
  
  for (int i = -1; i < 2; i += 2) {
    if (!(*dead)[x + i][y] && x + i >= 0 && x + i < SIZEX) {
      if ((*maze)[x + i][y] == '#') {
	next[(i + 1)/2] = i;
      } else if ((*maze)[x + i][y] == 'e') {
	return true;
      }
    }
    if (!(*dead)[x][y + i] && y + i >= 0 && y + i < SIZEY) {
      if ((*maze)[x][y + i] == '#') {
	next[(i + 1)/2 + 2] = i;
      } else if ((*maze)[x + i][y] == 'e') {
	return true;
      }
    }
  }
  
  for (int i = 0; i < 4; i++) {
    if (next[i] != 0) {
      if (i < 2) {
	if (eval(maze, dead, x + next[i], y)) {
	  return true;
	}
      } else {
	if (eval(maze, dead, x, y + next[i])) {
	  return true;
	}
      }
    }
  }

  return false;
}

int main() {
  char maze[SIZEX][SIZEY] = {{'#','#','#','+','+'},
			     {'+','#','+','+','#'},
			     {'#','#','#','#','#'},
			     {'+','+','#','+','#'},
			     {'s','#','#','+','e'}};
  bool dead[SIZEX][SIZEY] = {false};
  
  if (eval(&maze, &dead, 4, 0)) {
    printf("Solved maze.\n");
  } else {
    printf("Impossible maze.\n");
  }
  
  return 0;
}
