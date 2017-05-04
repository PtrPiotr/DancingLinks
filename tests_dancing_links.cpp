//
// Created by piotr on 02.05.17.
//

#include <bitset>
#include <iostream>

#include <gtest/gtest.h>

#include "dancing_links.hpp"


using namespace DancingLinks;
using namespace DancingLinks::Internal;

using namespace std;

const unsigned COLS = 7;
typedef std::bitset<COLS> B7;


class TestDancingLinks : public ::testing::Test {
protected:
    DancingLinks::DLSolver *dl = nullptr;

    std::vector<B7> hardcoded1 = std::vector<B7> {
            B7("1010000"),
            B7("0100000"),
            B7("0001101"),
            B7("0011001"),
            B7("0000010"),
            B7("1000000"),
            B7("1100000"),
            B7("0001011"),
            B7("0001101"),
            B7("0000010"),
    };

    std::vector<B7> impossible_column_3 = std::vector<B7> {
            B7("1010000"),
            B7("0100000"),
            B7("0000111"),
            B7("1100100"),
    };

    std::vector<B7> no_feasible_subset = std::vector<B7> {
            B7("1111000"),
            B7("0001111"),
            B7("1010101"),
            B7("1111101"),
    };

    virtual void SetUp() {

    }

    virtual void TearDown() {
        delete dl;
        dl = nullptr;
    }

    DancingLinks::DLSolver *populated(const std::vector<B7> &data) {
        dl = new DancingLinks::DLSolver(data.size(), COLS);

        for (unsigned r = 0; r < data.size(); r++) {
            for (unsigned c = 0; c < COLS; c++) {
                if (data[r][c]) {
                    dl->add(r, c);
                }
            }
        }
        return dl;
    }
};

TEST_F(TestDancingLinks, AllRowsCoverDistinctColumnsWhenRunOnSimpleExample) {
    DancingLinks::DLSolver *dl = populated(hardcoded1);

    auto solution = dl->solve();
    auto cols = B7();

    for (auto row: solution) {
        auto covered = hardcoded1[row];
        EXPECT_FALSE((cols & covered).any());
        cols |= covered;
    }
}

TEST_F(TestDancingLinks, AllColumnsCoveredWhenRunOnSimpleExample) {
    DancingLinks::DLSolver *dl = populated(hardcoded1);

    auto solution = dl->solve();
    auto cols = B7();

    for (auto row: solution) {
        cols |= hardcoded1[row];
    }

    EXPECT_TRUE(cols.all());
}

TEST_F(TestDancingLinks, EmptySolutionWhenCantCoverColumn) {
    DancingLinks::DLSolver *dl = populated(impossible_column_3);
    auto solution = dl->solve();
    EXPECT_EQ(solution.size(), 0);
}

TEST_F(TestDancingLinks, EmptySolutionWhenAllRowsInConflict) {
    DancingLinks::DLSolver *dl = populated(no_feasible_subset);
    auto solution = dl->solve();
    EXPECT_EQ(solution.size(), 0);
}


TEST_F(TestDancingLinks, CorrectAnswerWhenMoreRowsDeclaredThanUsed) {
    dl = new DancingLinks::DLSolver(100, COLS);

    for (unsigned r = 0; r < hardcoded1.size(); r++) {
        for (unsigned c = 0; c < COLS; c++) {
            if (hardcoded1[r][c]) {
                dl->add(r, c);
            }
        }
    }

    auto solution = dl->solve();
    auto cols = B7();

    for (auto row: solution) {
        cols |= hardcoded1[row];
    }

    EXPECT_TRUE(cols.all());
}








