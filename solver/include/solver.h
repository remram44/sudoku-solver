#ifndef SOLVER_H
#define SOLVER_H

#include <stdio.h>

char *sudoku_load(FILE *fp);
void sudoku_free(char *grid);
int sudoku_solve(char *grid);

#endif
