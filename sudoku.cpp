#include "sudoku.h"
#include <iostream>
#include <map>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <cmath>
#include <cctype>

using std::cout;
using std::endl;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

const char *COLOR_RED = "\033[1;31m";
const char *COLOR_DEFAULT = "\033[0;49m";

namespace sudoku
{
    // SudokuBoard implementation
    SudokuBoard::SudokuBoard(unsigned side) : side(side)
    {
        unsigned total = side * side;
        vals.reserve(total);
        predefined.reserve(total);
        for (unsigned i = 0; i < total; i++)
        {
            vals.push_back(-1);
            predefined.push_back(false);
        }
    }

    SudokuBoard SudokuBoard::FromString(const std::string &example)
    {
        vector<int> tokens;
        if (example.find('|') == std::string::npos)
        {
            tokens = GetSingleDigitTokens(example);
        }
        else
        {
            tokens = GetMultiDigitTokens(example);
        }
        unsigned total = tokens.size();
        unsigned side = std::sqrt(total);
        assert(side * side == total);

        SudokuBoard board(side);
        board.vals.clear();
        board.predefined.clear();
        board.vals.reserve(total);
        board.predefined.reserve(total);

        for (int t : tokens)
        {
            board.vals.push_back(t);
            board.predefined.push_back(t != -1);
        }

        return board;
    }

    SudokuBoard SudokuBoard::Empty(unsigned side)
    {
        return SudokuBoard(side);
    }

    vector<int> SudokuBoard::GetSingleDigitTokens(const std::string &example)
    {
        vector<int> tokens;
        for (char c : example)
        {
            if (c == '.' || std::isdigit(c))
                tokens.push_back(c == '.' ? -1 : c - '1');
        }
        return tokens;
    }

    vector<int> SudokuBoard::GetMultiDigitTokens(const std::string &example)
    {
        vector<int> tokens;
        std::istringstream iss(example);
        std::string token;
        while (iss >> token)
        {
            if (token[0] == '|' || token[0] == '-')
                continue;
            if (token == ".." || token == ".")
                tokens.push_back(-1);
            else
            {
                unsigned num = std::stoi(token);
                tokens.push_back(num - 1);
            }
        }
        return tokens;
    }

    void SudokuBoard::Set(unsigned int r, unsigned c, unsigned n)
    {
        assert(r < side && c < side && n >= 0 && n < side);
        assert(vals[r * side + c] == -1);
        vals[r * side + c] = (int)n;
    }

    int SudokuBoard::Get(unsigned r, unsigned c) const
    {
        assert(r < side && c < side);
        return vals[r * side + c];
    }

    void SudokuBoard::Print(bool ignore_colors)
    {
        int box_size = std::sqrt(side);
        int width = std::to_string(side).size();

        for (unsigned r = 0; r < side; r++)
        {
            if (r % box_size == 0)
                cout << "\n";
            for (unsigned c = 0; c < side; c++)
            {
                bool me_predefined = predefined[r * side + c];

                if (c % box_size == 0)
                    cout << " ";
                if (!ignore_colors && me_predefined)
                    cout << COLOR_RED;
                cout << std::setw(width) << Get(r, c) + 1;
                if (!ignore_colors && me_predefined)
                    cout << COLOR_DEFAULT;
                cout << " ";
            }
            cout << "\n";
        }
    }

    // SudokuMapper implementation
    SudokuMapper::SudokuMapper(std::shared_ptr<SudokuBoard> board) : board(board)
    {
    }

    SudokuMapper::Mapping::Mapping(unsigned r, unsigned c, unsigned n, unsigned side)
        : r(r), c(c), n(n), side(side) {}

    unsigned SudokuMapper::Mapping::Row() const
    {
        return r * side * side + c * side + n;
    }

    unsigned SudokuMapper::Mapping::ColumnCol() const
    {
        return c * side + n;
    }

    unsigned SudokuMapper::Mapping::RowCol() const
    {
        return side * side + r * side + n;
    }

    unsigned SudokuMapper::Mapping::AreaCol() const
    {
        int box_size = std::sqrt(side);
        unsigned area = (r / box_size) * box_size + c / box_size;
        return 2 * side * side + area * side + n;
    }

    unsigned SudokuMapper::Mapping::IntersectionCol() const
    {
        return 3 * side * side + r * side + c;
    }

    std::unique_ptr<DancingLinks::DLSolver> SudokuMapper::DlInstance()
    {
        auto &sb = *board;
        auto ret = std::make_unique<DancingLinks::DLSolver>(sb.GetSide() * sb.GetSide() * sb.GetSide(), sb.GetSide() * sb.GetSide() * 4);
        auto &solver = *ret;

        Populate(&solver);

        // remove DL rows for filled grids.
        for (unsigned r = 0; r < sb.GetSide(); r++)
        {
            for (unsigned c = 0; c < sb.GetSide(); c++)
            {
                int n = sb.Get(r, c);
                if (n >= 0)
                {
                    Mapping m(r, c, (unsigned)n, board->GetSide());
                    solver.DeleteRow(m.Row());
                }
            }
        }

        return ret;
    }

    void SudokuMapper::RevMap(const std::vector<int> &solution)
    {
        for (auto i : solution)
        {
            auto coord = dl_row_to_sudoku_field[i];
            board->Set(coord.r, coord.c, coord.n);
        }
    }

    void SudokuMapper::Populate(DancingLinks::DLSolver *solver)
    {
        for (unsigned sudo_row = 0; sudo_row < board->GetSide(); sudo_row++)
        {
            for (unsigned sudo_col = 0; sudo_col < board->GetSide(); sudo_col++)
            {
                for (unsigned num = 0; num < board->GetSide(); num++)
                {
                    Mapping m(sudo_row, sudo_col, num, board->GetSide());
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

    std::unique_ptr<DancingLinks::DLSolver> CreateSudokuSolver(const std::string &puzzle)
    {
        auto board = std::make_shared<SudokuBoard>(SudokuBoard::FromString(puzzle));
        SudokuMapper mapper(board);
        return mapper.DlInstance();
    }

    std::unique_ptr<DancingLinks::DLSolver> CreateEmptySudokuSolver(unsigned side)
    {
        auto board = std::make_shared<SudokuBoard>(SudokuBoard::Empty(side));
        SudokuMapper mapper(board);
        return mapper.DlInstance();
    }
}