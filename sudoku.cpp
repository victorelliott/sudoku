/* sudoku.cpp
 * Play a game of sudoku.
 * Written by Victor Elliott
 * 12/30/2020
 */

/* Issues:
 */

/* Planned features:
 * -Hint system
 * -Improved visuals
 * -More grid sizes (4x4? 16x16?)
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <time.h>
 
using namespace std;

const int SIZE = 9;

void initialize_grid(int **grid, int **starting_grid);
void setup(int **grid, int **starting_grid);
void generate(int **grid, int **starting_grid);
void shuffle(int *shuffled);
void read_digits(int **grid, int **starting_grid, string filename);
bool solvable(int **grid);
void draw_grid(int **grid, int **starting_grid);
void take_turn(int **grid, int **starting_grid, bool *quit);
bool valid_entry(int **grid, int **starting_grid, int row, int column, 
                 int digit);
bool repeated_entry(int **grid, int row, int column, int digit);
void find_subgrid(int row, int column, int *subgrid_row, int *subgrid_column);
bool valid_delete(int **starting_grid, int row, int column);
bool check_solved(int **grid);
void solve(int **grid, int *shuffled);
bool valid_solve(int **grid, int row, int column, int digit);
void reset(int **grid, int **starting_grid);
void clear_memory(int **grid, int **starting_grid);

int main() {
  
  int *grid[SIZE];
  int *starting_grid[SIZE];
  initialize_grid(grid, starting_grid);
  setup(grid, starting_grid);
  
  bool quit = false;
  if (!solvable(grid)) {
    draw_grid(grid, starting_grid);
    cout << "INVALID SUDOKU: Sudoku has no solution." << endl << endl;
    quit = true;
  }
  while (!quit) {
    draw_grid(grid, starting_grid);
    if (check_solved(grid)) {
      cout << "Sudoku solved!" << endl << endl;
      break;
    }
    take_turn(grid, starting_grid, &quit);
  }
  
  clear_memory(grid, starting_grid);
  return 0;
}

// Initialize the sudoku grid.
void initialize_grid(int **grid, int **starting_grid) {
   
  for (int i = 0; i < SIZE; i++) {
    grid[i] = new int[SIZE];
    starting_grid[i] = new int[SIZE];
    for (int j = 0; j < SIZE; j++) {
      grid[i][j] = 0;
      starting_grid[i][j] = 0;
    }
  }
}

// Set up the game.
void setup(int **grid, int **starting_grid) {
  
  cout << endl << "Play a random sudoku, or provide a sudoku data file?"
       << endl << endl
       << "(1) Play a random sudoku" << endl
       << "(2) Provide a sudoku data file" << endl;
  
  string query;
  while (true) {
    cout << endl << "Enter an option: ";
    cin >> query;
    if (query == "1" || query == "2") break;
    cout << "INVALID OPTION: Must enter 1 or 2." << endl;
  }
  
  if (query == "1") generate(grid, starting_grid);
  if (query == "2") {
    cout << "Enter the file name: ";
    string filename;
    cin >> filename;
    read_digits(grid, starting_grid, filename);
  }
  
  cin.ignore();
}

// Generate a sudoku.
void generate(int **grid, int **starting_grid) {
  
  int shuffled[SIZE];
  shuffle(shuffled);
  solve(grid, shuffled);
  
  srand(time(NULL));
  int clues = rand() % 11 + 25;
  int deletions = SIZE * SIZE - clues;
  for (int i = 0; i < deletions; i++) {
    while (true) {
      int row = rand() % 9;
      int column = rand() % 9;
      if (grid[row][column] != 0) {
        grid[row][column] = 0;
        break;
      }
    }
  }
  
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      starting_grid[i][j] = grid[i][j];
    }
  }
}

// Populate an array of integers with shuffled numbers.
void shuffle(int *shuffled) {
  
  for (int i = 0; i < SIZE; i++) {
    shuffled[i] = 0;
  }
  
  srand(time(NULL));
  for (int i = 0; i < SIZE; i++) {
    while (true) {
      bool repeated_element = false;
      shuffled[i] = rand() % 9 + 1;
      for (int j = 0; j < SIZE; j++) {
        if (shuffled[i] == shuffled[j] && i != j) repeated_element = true;
      }
      if (!repeated_element) break;
    }
  }
}

// Read the digits from a player provided file and populate the grid.
void read_digits(int **grid, int **starting_grid, string filename) {
  
  ifstream infile(filename);
  if (infile.fail()) {
    cerr << "ERROR: Failed to open the file." << endl;
    exit(EXIT_FAILURE);
  }
  
  char next_digit;
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      infile.get(next_digit);
      grid[i][j] = (int) next_digit - 48;
      starting_grid[i][j] = (int) next_digit - 48;
    }
    infile.ignore();
    infile.ignore();
  }
  
  infile.close();
}

// Determine if a given sudoku has a solution.
bool solvable(int **grid) {
  
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      if (!(grid[i][j] == 0 || valid_solve(grid, i, j, grid[i][j]))) {
        return false;
      }
    }
  }
  
  return true;
}

// Draw the sudoku grid.
void draw_grid(int **grid, int **starting_grid) {
  
  string cyan = "\033[36m";
  string reset = "\033[0m";
  
  cout << endl << "   1 2 3 4 5 6 7 8 9" << endl << endl;
  for (int i = 0; i < SIZE; i++) {
    cout << i + 1 << " ";
    for (int j = 0; j < SIZE; j++) {
      if (grid[i][j] == 0) {
        cout << "| ";
      } else if (starting_grid[i][j] == 0) {
        cout << "|" << grid[i][j];
      } else {
        cout << "|" << cyan << grid[i][j] << reset;
      }
    }
    cout << "|" << endl;
  }
  cout << endl;
} 

// Perform an action on the sudoku.
void take_turn(int **grid, int **starting_grid, bool *quit) {
  
  cout << "Make an entry:     e [row] [column] [digit]" << endl
       << "Delete an entry:   d [row] [column]" << endl
       << "Show solution:     SOLVE" << endl
       << "Reset sudoku:      RESET" << endl
       << "Quit:              QUIT" << endl;
  
  stringstream sstream;
  string line, query;
  int row, column, digit;
  while (true) {
    cout << endl << "Enter a query: ";
    sstream.clear();
    getline(cin, line);
    sstream.str(line);
    sstream >> query;
    if (query == "e") {
      sstream >> row >> column >> digit;
      if (!valid_entry(grid, starting_grid, row, column, digit)) continue;
      grid[row - 1][column - 1] = digit;
      break;
    }
    if (query == "d") {
      sstream >> row >> column;
      if (!valid_delete(starting_grid, row, column)) continue;
      grid[row - 1][column - 1] = 0;
      break;
    }
    if (query == "SOLVE") {
      reset(grid, starting_grid);
      int shuffled[SIZE];
      shuffle(shuffled);
      solve(grid, shuffled);
      draw_grid(grid, starting_grid);
      *quit = true;
      break;
    }
    if (query == "RESET") {
      reset(grid, starting_grid);
      break;
    }
    if (query == "QUIT") {
      *quit = true;
      break;
    }
    cout << "INVALID QUERY: Enter a valid query." << endl;
  }
}

// Determine if a player entry is valid.
bool valid_entry(int **grid, int **starting_grid, int row, int column, 
                 int digit) {
  
  row--;
  column--;
  
  bool valid_digit = false;
  bool valid_row = false;
  bool valid_column = false;
  for (int i = 0; i < SIZE; i++) {
    if (row == i) valid_row = true;
    if (column == i) valid_column = true;
    if (digit == i + 1) valid_digit = true;
  }
  
  if (!(valid_row && valid_column && valid_digit)) {
    if (!valid_row) cout << "INVALID ROW: Enter a row 1-9." << endl;
    if (!valid_column) cout << "INVALID COLUMN: Enter a column 1-9." << endl;
    if (!valid_digit) cout << "INVALID DIGIT: Enter a number 1-9." << endl;
    return false;
  }
  
  if (starting_grid[row][column] != 0) {
    cout << "INVALID ENTRY: Cannot overwrite starting entries." << endl;
    return false;
  }
  
  if (repeated_entry(grid, row, column, digit)) return false;
  
  return true;
}

// Determine if a player entry is found in the same row, column, or subgrid.
bool repeated_entry(int **grid, int row, int column, int digit) {
  
  bool row_contains = false;
  bool column_contains = false;
  for (int i = 0; i < SIZE; i++) {
    if (grid[row][i] == digit) row_contains = true;
    if (grid[i][column] == digit) column_contains = true;
  }
  
  bool subgrid_contains = false;
  int subgrid_row, subgrid_column;
  find_subgrid(row, column, &subgrid_row, &subgrid_column);
  for (int i = subgrid_row; i < subgrid_row + 3; i++) {
    for (int j = subgrid_column; j < subgrid_column + 3; j++) {
      if (grid[i][j] == digit) subgrid_contains = true;
    }
  }
  
  if (row_contains || column_contains || subgrid_contains) {
    if (row_contains) cout << "INVALID ENTRY: Row contains the same digit." 
                           << endl;
    if (column_contains) cout << "INVALID ENTRY: Column contains the same "
                              << "digit." << endl;
    if (subgrid_contains) cout << "INVALID ENTRY: Subgrid contains the same "
                               << "digit." << endl;
    return true;
  }
  
  return false;
}

// Given the row and column of an entry, find the subgrid where it is located.
void find_subgrid(int row, int column, int *subgrid_row, int *subgrid_column) {
  
  bool subgrid_found = false;
  for (int i = 3; i <= SIZE; i += 3) {
    for (int j = 3; j <= SIZE; j += 3) {
      if (row < i && column < j) {
        subgrid_found = true;
        *subgrid_row = i - 3;
        *subgrid_column = j - 3;
        break;
      }
    }
    if (subgrid_found) break;
  }
}

// Determine if a player deletion is valid.
bool valid_delete(int **starting_grid, int row, int column) {
  
  row--;
  column--;
  
  bool valid_row = false;
  bool valid_column = false;
  for (int i = 0; i < SIZE; i++) {
    if (row == i) valid_row = true;
    if (column == i) valid_column = true;
  }
  
  if (!(valid_row && valid_column)) {
    if (!valid_row) cout << "INVALID ROW: Enter a row 1-9." << endl;
    if (!valid_column) cout << "INVALID COLUMN: Enter a column 1-9." << endl;
    return false;
  }
  
  if (starting_grid[row][column] != 0) {
    cout << "INVALID DELETION: Cannot overwrite starting entries." << endl;
    return false;
  }
  
  return true;
}

// Determine if the sudoku has been solved.
bool check_solved(int **grid) {
  
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      if (grid[i][j] == 0) return false;
    }
  }
  
  return true;
}

// Solve the sudoku.
void solve(int **grid, int *shuffled) {
  
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      if (grid[i][j] == 0) {
        for (int k = 0; k < SIZE; k++) {
          if (valid_solve(grid, i, j, shuffled[k])) {
            grid[i][j] = shuffled[k];
            solve(grid, shuffled);
            if (check_solved(grid)) return;
          }
        }
        grid[i][j] = 0;
        return;
      }
    }
  }
}

// Determine if a single entry solution is valid.
bool valid_solve(int **grid, int row, int column, int digit) {
  
  for (int i = 0; i < SIZE; i++) {
    if (grid[row][i] == digit && i != column) return false;
    if (grid[i][column] == digit && i != row) return false;
  }
  
  int subgrid_row, subgrid_column;
  find_subgrid(row, column, &subgrid_row, &subgrid_column);
  for (int i = subgrid_row; i < subgrid_row + 3; i++) {
    for (int j = subgrid_column; j < subgrid_column + 3; j++) {
      if (grid[i][j] == digit && !(i == row && j == column)) return false;
    }
  }
  
  return true;
}

// Reset the sudoku grid.
void reset(int **grid, int **starting_grid) {
  
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      grid[i][j] = starting_grid[i][j];
    }
  }
}

// Clear used memory.
void clear_memory(int **grid, int **starting_grid) {
  
  for (int i = 0; i < SIZE; i++) {
    if (grid[i] != nullptr) delete [] grid[i];
    if (starting_grid[i] != nullptr) delete [] starting_grid[i];
  }
}
