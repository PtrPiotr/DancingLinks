#pragma once

#include "dancing_links.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sudoku {
class SudokuBoard final {
private:
  unsigned side;
  std::vector<int> vals;
  std::vector<bool> predefined;

  SudokuBoard(unsigned side);

public:
  static SudokuBoard FromString(const std::string &example);
  static SudokuBoard Empty(unsigned side);

  void Set(unsigned int r, unsigned c, unsigned n);
  int Get(unsigned r, unsigned c) const;
  void Print(bool ignore_colors = false);

  unsigned GetSide() const { return side; }
  bool IsPredefined(unsigned r, unsigned c) const {
    return predefined[r * side + c];
  }

private:
  static std::vector<int> GetSingleDigitTokens(const std::string &example);
  static std::vector<int> GetMultiDigitTokens(const std::string &example);
  int CalculateSide(const std::string &example);
};

class SudokuMapper final {
public:
  SudokuMapper(std::shared_ptr<SudokuBoard> board);

  std::unique_ptr<DancingLinks::DLSolver> DlInstance();
  void RevMap(const std::vector<int> &solution);

private:
  struct Coord {
    unsigned r, c, n;
    Coord() {}
    Coord(unsigned r, unsigned c, unsigned n) : r(r), c(c), n(n) {}
  };

  struct Mapping {
    unsigned r, c, n, side;

    Mapping(unsigned r, unsigned c, unsigned n, unsigned side);
    unsigned Row() const;
    unsigned ColumnCol() const;
    unsigned RowCol() const;
    unsigned AreaCol() const;
    unsigned IntersectionCol() const;
  };

  void Populate(DancingLinks::DLSolver *solver);

  std::map<unsigned, Coord> dl_row_to_sudoku_field;
  std::shared_ptr<SudokuBoard> board;
};

std::unique_ptr<DancingLinks::DLSolver>
CreateSudokuSolver(const std::string &puzzle);
std::unique_ptr<DancingLinks::DLSolver> CreateEmptySudokuSolver(unsigned side);
} // namespace sudoku
