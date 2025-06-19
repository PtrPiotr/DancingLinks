#include "sudoku.h"
#include <benchmark/benchmark.h>

int SolveSudoku(unsigned side) {
  auto solver = sudoku::CreateEmptySudokuSolver(side);
  auto solution = solver->Solve();
  return 0;
}

static void BM_DancingLinksSolverForSudoku25(benchmark::State &state) {
  for (auto _ : state) {
    SolveSudoku(25);
  }
}

static void BM_DancingLinksSolverForSudoku36(benchmark::State &state) {
  for (auto _ : state) {
    SolveSudoku(36);
  }
}

static void BM_DancingLinksSolverForSudoku49(benchmark::State &state) {
  for (auto _ : state) {
    SolveSudoku(49);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_DancingLinksSolverForSudoku25);

BENCHMARK(BM_DancingLinksSolverForSudoku36);

BENCHMARK(BM_DancingLinksSolverForSudoku49);

// Run the benchmark
BENCHMARK_MAIN();