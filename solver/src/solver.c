#include "solver.h"

#include <stdlib.h>
#include <string.h>

/*
 * Bit trick: counts number of bits set in integer
 *
 * http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 * https://groups.google.com/forum/?hl=en#!msg/comp.graphics.algorithms/ZKSegl2
 * sr4c/QYTwoPSx30MJ
 */
static unsigned short sideways_add(int v)
{
#if 0
    /* Non-magic version */
    unsigned short result = 0;
    while(v != 0)
    {
      result += v & 0x01;
      v >>= 1;
    }
    return result;
#endif

    /* Divice-and-conquer parallel "magic" version */
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

#ifdef DEBUG_SOLVER
void print_constraints(char *grid, unsigned short *constraints)
{
    int row, col;
    for(row = 0; row < 9; ++row)
    {
        for(col = 0; col < 9; ++col)
        {
            int j;
            if(grid[row * 9 + col] != '.')
                fprintf(stderr, "    %c    ", grid[row * 9 + col]);
            else for(j = '1'; j <= '9'; ++j)
            {
                unsigned short mask = 1 << (j - '1');
                if((mask & constraints[row * 9 + col]) == 0)
                    putc(j, stderr);
                else
                    putc('.', stderr);
            }
            putc(' ', stderr);
        }
        putc('\n', stderr);
    }
}

void print_nb_constraints(char *grid, unsigned short *constraints)
{
    int row, col;
    for(row = 0; row < 9; ++row)
    {
        for(col = 0; col < 9; ++col)
        {
            if(grid[row * 9 + col] == '.')
            {
                int nb_constraints = sideways_add(
                        constraints[row * 9 + col]);
                fprintf(stderr, "%d ", nb_constraints);
            }
            else
                fprintf(stderr, "  ");
        }
        fputc('\n', stderr);
    }
}
#endif

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

/*
 * Adds a number to a position in the grid, updating constraints
 */
static void add_number(unsigned short *constraints, char number,
                       int row, int col)
{
    unsigned short mask = 1 << (number - '1');
    int i, j;
    /* Can't have the same number on this row */
    for(i = 0; i < 9; ++i)
        constraints[row * 9 + i] |= mask;
    /* Can't have the same number on this column */
    for(i = 0; i < 9; ++i)
        constraints[i * 9 + col] |= mask;
    /* Can't have the same number in this 3*3 square */
    {
        const int sq_row = (int)(row/3) * 3;
        const int sq_col = (int)(col/3) * 3;
        for(i = sq_row; i < sq_row + 3; ++i)
            for(j = sq_col; j < sq_col + 3; ++j)
                constraints[i * 9 + j] |= mask;
    }
}

/*
 * Initializes constraints for the given grid
 *
 * The constraints array is assumed to be zeroed.
 */
static void init_constraints(char *grid, unsigned short *constraints)
{
    int row, col;
    for(row = 0; row < 9; ++row)
        for(col = 0; col < 9; ++col)
        {
            char number = grid[row * 9 + col];
            if(number != '.')
                add_number(constraints, number,
                           row, col);
        }
}

/*
 * Recursively solves the sudoku
 */
static int solve(char *grid, unsigned short *constraints)
{
    /* Finds the most constrained location in the grid */
    int max_constraints = 0;
    int max_pos_row = -1, max_pos_col;
    int row, col;
#ifdef DEBUG_SOLVER
    fprintf(stderr, "Solving...\n");
    print_constraints(grid, constraints);
    fprintf(stderr, "Nb constraints:\n");
    print_nb_constraints(grid, constraints);
#endif
    for(row = 0; row < 9; ++row)
        for(col = 0; col < 9; ++col)
        {
            if(grid[row * 9 + col] == '.')
            {
                int nb_constraints = sideways_add(
                        constraints[row * 9 + col]);
                if(nb_constraints > max_constraints)
                {
                    max_constraints = nb_constraints;
                    max_pos_row = row;
                    max_pos_col = col;
                }
            }
        }
    if(max_pos_row == -1)
    {
        /* Solved */
#ifdef DEBUG_SOLVER
        fprintf(stderr, "Grid is solved\n");
#endif
        return 0;
    }
#ifdef DEBUG_SOLVER
    fprintf(stderr, "Selected position %d;%d\n", max_pos_row, max_pos_col);
#endif
    {
        int nb;
        unsigned short con = constraints[max_pos_row * 9 + max_pos_col];
        /* Let's try every possible option */
        for(nb = '1'; nb <= '9'; ++nb)
        {
            unsigned short temp_cons[9 * 9];
            unsigned short mask = 1 << (nb - '1');
            if((mask & con) != 0)
                continue;
#ifdef DEBUG_SOLVER
            fprintf(stderr, "Trying %c...\n", nb);
#endif
            /* Set it in the grid */
            grid[max_pos_row * 9 + max_pos_col] = nb;
            memcpy(temp_cons, constraints, 9 * 9 * sizeof(unsigned short));
            add_number(temp_cons, nb, max_pos_row, max_pos_col);
            /* Recursively try to solve */
            if(solve(grid, temp_cons) == 0)
                return 0;
            /* Didn't find a solution: backtrack */
#ifdef DEBUG_SOLVER
            fprintf(stderr, "Backtracking!\n");
#endif
        }
        /* No need to reset the grid here, we failed anyway; we only reset it
         * so the caller gets the initial state back */
        grid[max_pos_row * 9 + max_pos_col] = '.';
    }
    return -1;
}

int sudoku_solve(char *grid)
{
    int result;
    unsigned short *constraints = malloc(9 * 9 *
                                         sizeof(unsigned short));
    int i;
    for(i = 0; i < 9 * 9; ++i)
        constraints[i] = 0;
    init_constraints(grid, constraints);
#ifdef DEBUG_SOLVER
    fprintf(stderr, "Initial constraints:\n");
    print_constraints(grid, constraints);
#endif
    result = solve(grid, constraints);
    free(constraints);
    return result;
}
