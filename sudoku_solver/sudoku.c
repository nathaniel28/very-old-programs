#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void display(char *board) {
  for (int i = 0; board[i] != '\0'; i++) {
    printf("%c%c", board[i], (i + 1)%9 ? ' ' : '\n');
  }
}

bool legal_move(char *board, int pos, char val) {
  int x = pos%9;
  int y = pos/9;
  for (int i = 0; i < 9; i++) {
    if (board[i*9 + x] == val) return false;
    if (board[y*9 + i] == val) return false;
  }
  x = (x/3)*3;
  y = (y/3)*3;
  for (int cy = y; cy < y + 3; cy++) {
    for (int cx = x; cx < x + 3; cx++) {
      if (board[cy*9 + cx] == val) return false;
    }
  }
  
  return true;
}

bool solve(char *board, int pos) {
  while (board[++pos] != '.') {
    if (pos >= 81) return true;
  }
  for (char move = '1'; move <= '9'; move++) {
    if (legal_move(board, pos, move)) {
      board[pos] = move;
      if (solve(board, pos)) return true;
    }
  }
  board[pos] = '.';
  return false;
}

/*
  ".7.3....."
  "....2..5."
  ".2....6.3"
  "..6..1.94"
  "1...9...2"
  "95.6..1.."
  "6.4....8."
  ".9..4...."
  ".....9.3."
*/

int main() {
  char *board = strdup(
    ".6.9..78."
    "3....8..5"
    "9..23...."
    ".8....6.2"
    "..5...3.."
    "6.3....5."
    "....19..7"
    "7..6....4"
    ".49..3.6."
  );
  if (!board) return 1;

  if (!solve(board, -1)) {
    printf("No solution.\n");
  }
  display(board);

  free(board);
  return 0;
}
