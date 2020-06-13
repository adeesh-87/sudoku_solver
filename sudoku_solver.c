// sudoku_solver.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct cell {
  int val;
  int init;
} cell;

typedef struct sudoku_stats {
  unsigned int row[9];
  unsigned int col[9];
  unsigned int box[9];
} sudoku_stats;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int num_solved;

int sudoku_parser(cell **grid, FILE *f);
int sudoku_preprocessor(cell **grid, sudoku_stats *st);
void sudoku_printer(cell **grid);
void sudoku_solver_row(cell **grid, sudoku_stats *st);
void sudoku_solver_box(cell **grid, sudoku_stats *st);
void sudoku_solver_cell(cell **grid, sudoku_stats *st);
static void sudoku_printStats(sudoku_stats *st);

static inline int count_ones(int a)
{
  int c = 0;
  while (a) {
	c++;
	a = a & (a-1);
  }
  return c;
}

int sudoku_parser(cell **grid, FILE *f)
{
  char buf[64];
  int nums[9];
  for (int row = 0; row < 9; row++) {
	fgets(buf, 20, f);
	sscanf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d", 
		   &grid[row][0].val, 
		   &grid[row][1].val, 
		   &grid[row][2].val, 
		   &grid[row][3].val, 
		   &grid[row][4].val, 
		   &grid[row][5].val, 
		   &grid[row][6].val, 
		   &grid[row][7].val, 
		   &grid[row][8].val);
  }
  return 0;
}

int sudoku_preprocessor(cell **grid, sudoku_stats *st)
{
  for (int k = 0; k < 9; k++) {
	st->row[k] = 0x1FF;
	st->col[k] = 0x1FF;
	st->box[k] = 0x1FF;
  }

  for (int i = 0; i < 9; i++) {
	for (int j = 0; j < 9; j++) {
	  if (grid[i][j].val != 0) {
		grid[i][j].init = 1;
		num_solved++;
	  }
	  st->row[i] &= ~(1 << (grid[i][j].val - 1));
	  st->col[j] &= ~(1 << (grid[i][j].val - 1));
	  st->box[3*(i/3) + (j/3)] &= ~(1 << (grid[i][j].val - 1));
	}
  }

  //sudoku_printStats(st);
}

static void sudoku_printStats(sudoku_stats *st)
{
  // For debugging purposes
  for (int i = 0; i < 9; i++) {
	for (int j = 0; j < 9; j++) {
	  printf("%2c", ((1 << j) & st->row[i]) ? '1' : '0');
	}
	printf("\t");
	for (int j = 0; j < 9; j++) {
	  printf("%2c", ((1 << j) & st->col[i]) ? '1' : '0');
	}
	printf("\t");
	for (int j = 0; j < 9; j++) {
	  printf("%2c", ((1 << j) & st->box[i]) ? '1' : '0');
	}
	printf("\n");
  }
}

static char ans;

int main(int argc, char *argv[])
{
  int m_siz = 9;
  cell **grid = (cell**)malloc(m_siz*sizeof(cell*));
  for (int i = 0; i < m_siz; i++) {
	grid[i] = (cell*) malloc(m_siz * sizeof(cell));
	memset(grid[i], 0, 9*sizeof(**grid));
  }

  FILE *test_file;
  if (argc > 1) {
	test_file = fopen(argv[1], "r");
  } else {
	test_file = fopen("puzzle2.txt", "r");
  }

  int num_solved_prev = 0;

  sudoku_stats g_stats;
  printf("Parsed puzzle res %d\n\n", sudoku_parser(grid, test_file));
  sudoku_preprocessor(grid, &g_stats);

  sudoku_printer(grid);

  while (num_solved_prev != num_solved) {
	num_solved_prev = num_solved;
	sudoku_solver_row(grid, &g_stats);
	sudoku_solver_box(grid, &g_stats);
	sudoku_solver_cell(grid, &g_stats);
  }
  sudoku_printer(grid);
	
  for (int i = 0; i < m_siz; i++)
	free(grid[i]);
  free(grid);
}

void sudoku_solver_box(cell **grid, sudoku_stats *st)
{
  for (int b = 0; b < 9; b++) {
	for (int e = 0; e < 9; e++) {
	  unsigned int avl = 0x01FF;
	  if (st->box[b] & (1 << e)) {
		for (int x = 0; x < 9; x++) {
		  if (grid[3*(b/3) + x/3][3*(b%3) + (x%3)].val)
			avl &= ~(1 << x);
		}
		for (int z = 0; z < 3; z++) {
		  if ((st->row[3*(b/3) + z] & (1 << e)) == 0) {
			avl &= ~(0x00000007 << 3*z);
		  }
		  // + z][3*(b%3) + z].val || ((st->col[c] & (1 << e)) == 0))
		  if ((st->col[3*(b%3) + z] & (1 << e)) == 0) {
			avl &= ~(0x00000049 << z);
		  }
		}
		int num;
		if ((num = count_ones(avl)) == 1) {
		  int v = 0;
		  while (avl) {
			avl >>= 1;
			v++;
		  }
		  int x = 3*(b/3) + ((v-1)/3);
		  int y = 3*(b%3) + ((v-1)%3);
		  grid[x][y].val = e+1;
		  num_solved++;
		  st->row[3*(b/3) + ((v-1)/3)] &= ~(1<<e);
		  st->col[3*(b%3) + ((v-1)%3)] &= ~(1<<e);
		  st->box[b] &= ~(1<<e);
		  printf("Box solver put %d @ [%d,%d]\n", e+1, x, y);
		  //sudoku_printer(grid);
		  //sudoku_printStats(st);
		  //ans = (char) getchar();
		} else {
		  //printf("gave up on %d @ box %d : p %d\n", e+1, b+1, num);
		}
	  }
	}
  }
}

void sudoku_solver_row(cell **grid, sudoku_stats *st)
{
  // Brute force approach:
  // Iterate through each square:
  for (int r = 0; r < 9; r++) {
	for (int e = 0; e < 9; e++) {
	  unsigned int avl = 0x01FF;
	  if (st->row[r] & (1 << e)) {
		for (int c = 0; c < 9; c++) {
		  if (grid[r][c].val || ((st->col[c] & (1 << e)) == 0))
			avl &= ~(1 << c);
		}
		for (int b = 0; b < 3; b++) {
		  if ((st->box[3*(r/3) + b] & (1 << e)) == 0) 
			avl &= ~(0x00000007 << (3*b));
		}
		int num;
		if ((num = count_ones(avl)) == 1) {
		  int v = 0;
		  while (avl) {
			avl >>= 1;
			v++;
		  }
		  grid[r][v-1].val = e+1;
		  st->row[r] &= ~(1<<e);
		  st->col[v-1] &= ~(1<<e);
		  st->box[3*(r/3) + ((v-1)/3)] &= ~(1<<e);
		  num_solved++;
		  printf("Row solver put %d @ [%d,%d]\n", e+1, r, v-1);
		  //sudoku_printer(grid);
		  //sudoku_printStats(st);
		  //ans = (char) getchar();
		} else {
		  //printf("gave up on %d @ row %d : p %d\n", e+1, r+1, num);
		}
	  }
	}
  }
}

void sudoku_solver_column(cell **grid, sudoku_stats *st)
{
  for (int c = 0; c < 9; c++) {
	for (int e = 0; e < 9; e++) {
	  unsigned int avl = 0x01FF;
	}
  }
}

void sudoku_solver_cell(cell **grid, sudoku_stats *st) {
  for (int r = 0; r < 9; r++) {
	for (int c = 0; c < 9; c++) {
	  if (grid[r][c].val != 0) {
		continue;
	  }
	  int avl = 0x1FF;
	  avl &= st->row[r];
	  avl &= st->col[c];
	  avl &= st->box[3*(r/3) + (c/3)];
	  if (count_ones(avl) == 1) {
		grid[r][c].val = log2(avl) + 1;
		num_solved++;
		st->row[r] &= ~(avl);
		st->col[c] &= ~avl;
		st->box[3*(r/3) + (c/3)] &= ~avl;
		//sudoku_printer(grid);
		printf("Cell solver put %d @ [%d,%d]\n", grid[r][c].val, r, c);
		//ans = getchar();
	  }
	}
  }
}

void sudoku_printer(cell **grid)
{
  printf(" +");
  for (int k = 0; k < 9; k++) {
	printf("%2c", '-');
	if (!((k+1) % 3))
	  printf("%2c",'+');
  }
  printf("\n");
  for (int i = 0; i < 9; i++) {
	printf("%2c",'|');
	for (int j = 0; j < 9; j++) {
	  if (grid[i][j].init == 1) {
		printf(ANSI_COLOR_GREEN "%2d" ANSI_COLOR_RESET, grid[i][j].val);
	  } else {
		if (grid[i][j].val != 0) {
		  printf(ANSI_COLOR_RED "%2d" ANSI_COLOR_RESET, grid[i][j].val);
		} else {
		  printf("%2c", ' ');
		}
	  }
	  if (!((j+1) % 3))
		printf("%2c",'|');
	}
	printf("\n");
	if (!((i+1) % 3)) {
	  printf("%2c", '+');
	  for (int k = 0; k < 9; k++) {
		printf("%2c", '-');
		if (!((k+1) % 3))
		  printf("%2c",'+');
	  }
	  printf("\n");
	}
  }

}
