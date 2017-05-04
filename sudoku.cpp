#include "dancing_links.hpp"

#include <vector>
#include <iostream>
#include <map>
#include <chrono>

using namespace std;
using namespace std::chrono;

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

  void set(unsigned int r, unsigned c, unsigned n) {
    assert(r < 9 && c < 9 && n >= 0 && n < 9);
    assert(vals[r * 9 + c] == -1);
    vals[r * 9 + c] = (int) n;
  }

  int get(unsigned r, unsigned c) const {
    assert (r < 9 && c < 9);
    return vals[r * 9 + c];
  }

  void print(bool ignore_colors = false) {
    for (unsigned r = 0; r < 9; r++) {
      if (r % 3 == 0) cout << '\n';
      for (unsigned c = 0; c < 9; c++) {

        bool me_predefined = predefined[r * 9 + c];

        if (c % 3 == 0) cout << ' ';
        if (!ignore_colors && me_predefined) cout << "\033[1;31m";
        cout << get(r, c) + 1;
        if (!ignore_colors && me_predefined) cout << "\033[0;49m";

      }
      cout << "\n";
    }
  }
};

class SudokuMapper {

  struct Coord {
    unsigned r, c, n;
    Coord() {}
    Coord(unsigned r, unsigned c, unsigned n) : r(r), c(c), n(n) {}
  };

  struct Mapping {
    unsigned r, c, n;

    Mapping(unsigned r, unsigned c, unsigned n)
        : r(r), c(c), n(n) {}

    unsigned row() {
      return r * 9 * 9 + c * 9 + n;
    }

    unsigned column_col() { return c * 9 + n; }
    unsigned row_col() { return 9 * 9 + r * 9 + n; }
    unsigned area_col() {
      unsigned area = (r / 3) * 3 + c / 3;
      return 2 * 9 * 9 + area * 9 + n;
    }
    unsigned intersection_col() { return 3 * 9 * 9 + r * 9 + c; }
  };

  std::map<unsigned, Coord> r2n;
  SudokuBoard *board;

 public:

  SudokuMapper(SudokuBoard *board) : board(board) {

  }

  DancingLinks::DLSolver dl_instnce() {
    auto &sb = *board;
    DancingLinks::DLSolver ret(9 * 9 * 9, 9 * 9 * 4);

    for (unsigned sodo_row = 0; sodo_row < 9; sodo_row++) {
      for (unsigned sodo_col = 0; sodo_col < 9; sodo_col++) {
        for (unsigned num = 0; num < 9; num++) {

          Mapping m(sodo_row, sodo_col, num);
          unsigned row = m.row();
          r2n[row] = Coord(sodo_row, sodo_col, num);

          ret.add(row, m.column_col());
          ret.add(row, m.row_col());
          ret.add(row, m.area_col());
          ret.add(row, m.intersection_col());
        }
      }
    }

    for (unsigned r = 0; r < 9; r++) {
      for (unsigned c = 0; c < 9; c++) {
        int n = sb.get(r, c);
        if (n >= 0) {
          Mapping m(r, c, (unsigned) n);
          ret.delete_row(m.row());
        }
      }
    }

    return ret;
  }

  void rev_map(const std::vector<int> &solution) {
    for (auto i: solution) {
      auto coord = r2n[i];
      board->set(coord.r, coord.c, coord.n);
    }
  }

};

int main() {

    int num = 1;
    for (auto example: examples) {

      SudokuBoard sb(example);
      SudokuMapper m(&sb);
      DancingLinks::DLSolver d = m.dl_instnce();

      auto t_start = high_resolution_clock::now();
      auto solution = d.solve();
      auto t_end = high_resolution_clock::now();
      auto duration = duration_cast<microseconds>(t_end - t_start).count();

      cout << "Example " << num << " (" << duration << " microseconds)" << endl;
      if (solution.empty()) {
        cout << "NO ANSWER" << endl;
      } else {
        m.rev_map(solution);
        sb.print();
      }
      cout << endl;
      num++;
    }

    cout << endl;
    cout << "Done" << endl;

}

