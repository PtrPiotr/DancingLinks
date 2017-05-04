//
// Created by piotr on 02.05.17.
//

#include <gtest/gtest.h>
#include <set>

#include "dancing_links.hpp"

using namespace DancingLinks;
using namespace DancingLinks::Internal;

class TestListMatrix : public ::testing::Test {
 protected:
  // DLSolver *dl;
  static const unsigned N = 10;
  El *els[N];
  std::set<int> visited;

  virtual void SetUp() {
    els[0] = new El(0, 0);

    for (unsigned i = 1; i < N; i++) {
      els[i] = new El(i, 0);
      Manipulator<Vertical>::insert(els[0], els[i]);
    }

  }

  virtual void TearDown() {
    for (auto el: els) {
      delete el;
    }

    visited.clear();
  }

  template<typename T>
  void visit_with_iter(Iter<T> &it) {
    for (; *it; ++it) {
      visited.insert((*it)->rowId);
    }
  }

  template<typename T>
  void expect_counts_after_iteration(T &it, unsigned expected_size) {
    visit_with_iter(it);
    EXPECT_EQ(visited.size(), expected_size);
  }

  template<typename T>
  void expect_counts_after_iteration_without(T &it, unsigned expected_size, int without) {
    expect_counts_after_iteration(it, expected_size);
    EXPECT_TRUE(visited.find(without) == end(visited));
  }

  template<typename DIR>
  void assert_iteration_correct(El *start, size_t max_len = 2 * N) {
    unsigned len = 0;
    for (auto cur = Iter<DIR>::all(start); *cur; ++cur) {
      EXPECT_EQ(DIR::next(DIR::prev(*cur)), *cur);
      EXPECT_EQ(DIR::prev(DIR::next(*cur)), *cur);
      len++;
    }

    EXPECT_FALSE(len == max_len);
  }
};

TEST_F(TestListMatrix, TwoElemsCircularListWhenAddedHorizontal) {
  El el(0, 0);
  EXPECT_EQ(el.r, &el);

  El el2(0, 0);
  Manipulator<Horizontal>::insert(&el, &el2);
  EXPECT_EQ(el.r, &el2);
  EXPECT_EQ(el.l, &el2);
  EXPECT_EQ(el2.r, &el);
  EXPECT_EQ(el2.l, &el);
}

TEST_F(TestListMatrix, TwoElemsCircularListWhenAddedVertical) {
  El el(0, 0);
  EXPECT_EQ(el.d, &el);

  El el2(0, 0);
  Manipulator<Vertical>::insert(&el, &el2);
  EXPECT_EQ(el.d, &el2);
  EXPECT_EQ(el.u, &el2);
  EXPECT_EQ(el2.d, &el);
  EXPECT_EQ(el2.u, &el);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenIterateOverAll) {
  auto it = Iter<Vertical>::all(els[0]);
  expect_counts_after_iteration(it, N);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenInvertedIterateOverAll) {
  auto it = Iter<Invert<Vertical>>::all(els[0]);
  expect_counts_after_iteration(it, N);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenIterateAllButMe) {
  auto it = Iter<Vertical>::all_but_me(els[0]);
  expect_counts_after_iteration_without(it, N - 1, 0);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenInvertIterateAllButMe) {
  auto it = Iter<Invert<Vertical>>::all_but_me(els[0]);
  expect_counts_after_iteration_without(it, N - 1, 0);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenFirstRemoved) {
  Manipulator<Vertical>::remove(els[0]);
  assert_iteration_correct<Vertical>(els[1]);
  auto it = Iter<Vertical>::all(els[1]);
  expect_counts_after_iteration_without(it, N - 1, 0);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenSecondRemoved) {
  Manipulator<Vertical>::remove(els[1]);
  assert_iteration_correct<Vertical>(els[0]);
  auto it = Iter<Vertical>::all(els[0]);
  expect_counts_after_iteration_without(it, N - 1, 1);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenLastRemoved) {
  Manipulator<Vertical>::remove(els[N - 1]);

  assert_iteration_correct<Vertical>(els[0]);
  auto it = Iter<Vertical>::all(els[0]);
  expect_counts_after_iteration_without(it, N - 1, 9);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenTwoRemoved) {
  Manipulator<Vertical>::remove(els[0]);
  Manipulator<Vertical>::remove(els[1]);

  assert_iteration_correct<Vertical>(els[2]);

  auto it = Iter<Vertical>::all(els[2]);
  expect_counts_after_iteration(it, N - 2);
  EXPECT_TRUE(visited.find(0) == end(visited));
  EXPECT_TRUE(visited.find(1) == end(visited));
}

TEST_F(TestListMatrix, ListContainsOnlyValidWhenTwoRemovedOneReinsertedInFIFOOrder) {
  Manipulator<Vertical>::remove(els[0]);
  Manipulator<Vertical>::remove(els[1]);
  Manipulator<Vertical>::reinsert(els[1]);

  assert_iteration_correct<Vertical>(els[2]);

  auto it = Iter<Vertical>::all(els[2]);
  expect_counts_after_iteration(it, N - 1);
  EXPECT_TRUE(visited.find(0) == end(visited));
}