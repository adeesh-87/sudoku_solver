clean:
	@rm -f SudokuSolver
	@echo Cleaned project

all: clean
	gcc -g src/sudoku_solver.c -lm -o SudokuSolver
	@echo Built project

check: clean all
	./SudokuSolver test/puzzle.txt