#include "sudoku.h"
#include <iostream>
#include <chrono>
#include <memory>

using std::cout;
using std::endl;
using std::vector;
using std::chrono::microseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using namespace sudoku;

// Main function with examples
int main() {
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
        "... 5.. .8.",


        // https://gist.github.com/vaskoz/8212615
        "|  . 15  .  1 |  .  2 10 14 | 12  .  .  . |  .  .  .  . |"
        "|  .  6  3 16 | 12  .  8  4 | 14 15  1  . |  2  .  .  . |"
        "| 14  .  9  7 | 11  3 15  . |  .  .  .  . |  .  .  .  . |"
        "|  4 13  2 12 |  .  .  .  . |  6  .  .  . |  . 15  .  . |"
        "---------------------------------------------------------"
        "|  .  .  .  . | 14  1 11  7 |  3  5 10  . |  .  8  . 12 |"
        "|  3 16  .  . |  2  4  .  . |  . 14  7 13 |  .  .  5 15 |"
        "| 11  .  5  . |  .  .  .  . |  .  9  4  . |  .  6  .  . |"
        "|  .  .  .  . | 13  . 16  5 | 15  .  . 12 |  .  .  .  . |"
        "---------------------------------------------------------"
        "|  .  .  .  . |  9  .  1 12 |  .  8  3 10 | 11  . 15  . |"
        "|  2 12  . 11 |  .  . 14  3 |  5  4  .  . |  .  .  9  . |"
        "|  6  3  .  4 |  .  . 13  . |  . 11  9  1 |  . 12 16  2 |"
        "|  .  . 10  9 |  .  .  .  . |  .  . 12  . |  8  .  6  7 |"
        "---------------------------------------------------------"
        "| 12  8  .  . | 16  .  . 10 |  . 13  .  . |  .  5  .  . |"
        "|  5  .  .  . |  3  .  4  6 |  .  1 15  . |  .  .  .  . |"
        "|  .  9  1  6 |  . 14  . 11 |  .  .  2  . |  .  . 10  8 |"
        "|  . 14  .  . |  . 13  9  . |  4 12 11  8 |  .  .  2  . |"
    };

    int num = 1;
    for (auto example: examples) {
        auto sb = std::make_shared<SudokuBoard>(SudokuBoard::FromString(example));
        SudokuMapper m(sb);
        auto d = m.DlInstance();

        auto t_start = high_resolution_clock::now();
        auto solution = d->Solve();
        auto t_end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t_end - t_start).count();

        cout << "Example " << num << " (" << duration << " microseconds)" << endl;
        if (solution.empty()) {
            cout << "NO ANSWER" << endl;
            sb->Print();
        } else {
            m.RevMap(solution);
            sb->Print();
        }
        cout << endl;
        num++;
    }

    {
        cout << "Solving empty board of side 25" << endl;
        auto sb = std::make_shared<SudokuBoard>(SudokuBoard::Empty(25));
        SudokuMapper m(sb);
        auto d = m.DlInstance();

        auto t_start = high_resolution_clock::now();
        auto solution = d->Solve();
        auto t_end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t_end - t_start).count();

        cout << "Empty board solved in " << duration << " microseconds" << endl;
        if (solution.empty()) {
            cout << "NO ANSWER" << endl;
            sb->Print();
        } else {
            m.RevMap(solution);
            sb->Print();
        }
    }

    cout << endl;
    cout << "Done" << endl;
    return 0;
}