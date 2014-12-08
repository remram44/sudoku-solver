#include <stdio.h>

#include "solver.h"

int main(int argc, char **argv)
{
    FILE *input = NULL;
    char *grid;

    if(argc == 2)
    {
        input = fopen(argv[1], "r");
        if(input == NULL)
        {
            fprintf(stderr, "Error opening file\n");
            return 2;
        }
    }
    else
        input = stdin;

    grid = sudoku_load(input);
    if(grid == NULL)
    {
        fprintf(stderr, "Error loading grid\n");
        return 2;
    }

    if(sudoku_solve(grid) != 0)
    {
        fprintf(stderr, "Error solving grid\n");
        return 1;
    }

    {
        int row, col;
        const char *grid_pos = grid;
        for(row = 0; row < 9; ++row)
        {
            for(col = 0; col < 9; ++col)
                fputc((unsigned char)*grid_pos++, stdout);
            fputc((unsigned char)'\n', stdout);
        }
    }

    return 0;
}
