#include "solver.h"

#include <stdlib.h>

char *sudoku_load(FILE *fp)
{
    char *grid = malloc(9*9);
    int c;
    int row;
    for(row = 0; row < 9; ++row)
    {
        int col;
        for(col = 0; col < 9; ++col)
        {
            c = fgetc(fp);
            if(c == EOF ||
                    ( (c > (unsigned char)'9' ||
                       c < (unsigned char)'1') &&
                      c != (unsigned char)'.'))
            {
                fprintf(stderr, "Invalid character at %d, %d\n",
                        row, col);
                goto fail;
            }
            grid[row * 9 + col] = c;
        }
        c = fgetc(fp);
        if(c == EOF)
        {
            if(row == 8)
                return grid;
            else
            {
                fprintf(stderr, "Premature end of file after line %d\n",
                        row);
                goto fail;
            }
        }
        if(c == (unsigned char)'\r')
            c = fgetc(fp);
        if(c != (unsigned char)'\n')
        {
            fprintf(stderr, "Invalid end of line character line %d\n",
                    row);
            goto fail;
        }
    }
    return grid;

fail:
    free(grid);
    return NULL;
}

void sudoku_free(char *grid)
{
    free(grid);
}

static int solve(char *grid, unsigned short *constraints)
{
    return 0;
}

int sudoku_solve(char *grid)
{
    int result;
    unsigned short *constraints = malloc(9 * 9 *
                                         sizeof(unsigned short));
    int i;
    for(i = 0; i < 9 * 9; ++i)
        constraints[i] = 0;
    result = solve(grid, constraints);
    free(constraints);
    return result;
}
