#include "dancing_links.hpp"

#include <memory>
#include <vector>
#include <iostream>
#include <map>
#include <chrono>

using std::cout;
using std::endl;
using std::vector;
using std::chrono::microseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;

const char *COLOR_RED = "\033[1;31m";
const char *COLOR_DEFAULT = "\033[0;49m";

vector<std::string> examples = {
    //  empty
    "... ... ..."
        "... ... ..."
        "... ... ..."

        "... ... ..."
        "... ... ..."
        "... ... ..."

        "... ... ..."
        "... ... ..."
        "... ... ...",

    // http://www.telegraph.co.uk/news/science/science-news/9359579/Worlds-hardest-sudoku-can-you-crack-it.html

    "8.. ... ..."
        "..3 6.. ..."
        ".7. .9. 2.."

        ".5. ..7 ..."
        "... .45 7.."
        "... 1.. .3."

        "..1 ... .68"
        "..8 5.. .1."
        ".9. ... 4..",


    // impossible (http://www.jibble.org/impossible-sudoku/)
    ".7. ..6 ..."
        "9.. ... .41"
        "..8 ..9 .5."

        ".9. ..7 ..2"
        "..3 ... 8.."
        "4.. 8.. .1."

        ".8. 3.. 9.."
        "16. ... ..7"
        "... 5.. .8."

};

struct SudokuBoard {

  std::vector<int> vals;
  std::vector<bool> predefined;

  SudokuBoard(const std::string &example) {
    vals.reserve(9 * 9);
    predefined.reserve(9 * 9);

    for (unsigned i = 0; example[i]; i++) {
      if (example[i] != '.' && (example[i] < '0' || example[i] > '9')) continue;
      vals.push_back(example[i] == '.' ? -1 : example[i] - '1');
      predefined.push_back(vals.back() != -1);
    }

    assert(vals.size() == 81);
  }

  void Set(unsigned int r, unsigned c, unsigned n) {
    assert(r < 9 && c < 9 && n >= 0 && n < 9);
    assert(vals[r * 9 + c] == -1);
    vals[r * 9 + c] = (int) n;
  }

  int Get(unsigned r, unsigned c) const {
    assert (r < 9 && c < 9);
    return vals[r * 9 + c];
  }

  void Print(bool ignore_colors = false) {
    for (unsigned r = 0; r < 9; r++) {
      if (r % 3 == 0) cout << '\n';
      for (unsigned c = 0; c < 9; c++) {

        bool me_predefined = predefined[r * 9 + c];

        if (c % 3 == 0) cout << ' ';
        if (!ignore_colors && me_predefined) cout << COLOR_RED;
        cout << Get(r, c) + 1;
        if (!ignore_colors && me_predefined) cout << COLOR_DEFAULT;

      }
      cout << "\n";
    }
  }
};

class SudokuMapper {
 public:

  SudokuMapper(std::shared_ptr<SudokuBoard> board) : board(board) {

  }

  std::unique_ptr<DancingLinks::DLSolver> DlInstance() {
    auto &sb = *board;
    auto ret = std::make_unique<DancingLinks::DLSolver>(9 * 9 * 9, 9 * 9 * 4);
    auto &solver = *ret;

    Populate(&solver);

    // remove DL rows for filled grids.
    for (unsigned r = 0; r < 9; r++) {
      for (unsigned c = 0; c < 9; c++) {
        int n = sb.Get(r, c);
        if (n >= 0) {
          Mapping m(r, c, (unsigned) n);
          solver.DeleteRow(m.Row());
        }
      }
    }

    return ret;
  }

  void RevMap(const std::vector<int> &solution) {
    for (auto i: solution) {
      auto coord = dl_row_to_sudoku_field[i];
      board->Set(coord.r, coord.c, coord.n);
    }
  }

 private:
  struct Coord {
    unsigned r, c, n;
    Coord() {}
    Coord(unsigned r, unsigned c, unsigned n) : r(r), c(c), n(n) {}
  };

  struct Mapping {
    unsigned r, c, n;

    Mapping(unsigned r, unsigned c, unsigned n)
        : r(r), c(c), n(n) {}

    /*
     * In the DancingLinks table, row id for this assignment
     */
    unsigned Row() const {
      return r * 9 * 9 + c * 9 + n;
    }

    /* meaning of columns:
     * column_1_has_1
     * column_1_has_2
     * ...
     * column_9_has_9
     * row_1_has_1
     * ...
     * row_9_has_9
     * area_1_has_1
     * ...
     * area_9_has_9
     * there_is_assignment_at_1_1
     * ...
     * there_is_assignment_at_9_9
     */
    unsigned ColumnCol() const { return c * 9 + n; }

    unsigned RowCol() const { return 9 * 9 + r * 9 + n; }

    unsigned AreaCol() const {
      unsigned area = (r / 3) * 3 + c / 3;
      return 2 * 9 * 9 + area * 9 + n;
    }
    unsigned IntersectionCol() const { return 3 * 9 * 9 + r * 9 + c; }
  };

  /*
   * Populate DL solver with "clean" Sudoku board
   */
  void Populate(DancingLinks::DLSolver *solver) {
    for (unsigned sudo_row = 0; sudo_row < 9; sudo_row++) {
      for (unsigned sudo_col = 0; sudo_col < 9; sudo_col++) {
        for (unsigned num = 0; num < 9; num++) {

          Mapping m(sudo_row, sudo_col, num);
          unsigned row = m.Row();
          dl_row_to_sudoku_field[row] = Coord(sudo_row, sudo_col, num);

          solver->Add(row, m.ColumnCol());
          solver->Add(row, m.RowCol());
          solver->Add(row, m.AreaCol());
          solver->Add(row, m.IntersectionCol());
        }
      }
    }
  }

  std::map<unsigned, Coord> dl_row_to_sudoku_field;
  std::shared_ptr<SudokuBoard> board;

};

int main() {

  int num = 1;
  for (auto example: examples) {

    auto sb = std::make_shared<SudokuBoard>(example);
    SudokuMapper m(sb);
    auto d = m.DlInstance();

    auto t_start = high_resolution_clock::now();
    auto solution = d->Solve();
    auto t_end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t_end - t_start).count();

    cout << "Example " << num << " (" << duration << " microseconds)" << endl;
    if (solution.empty()) {
      cout << "NO ANSWER" << endl;
    } else {
      m.RevMap(solution);
      sb->Print();
    }
    cout << endl;
    num++;
  }

  cout << endl;
  cout << "Done" << endl;

}

