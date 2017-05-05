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
  Element *els[N];
  std::set<int> visited;

  virtual void SetUp() {
    els[0] = new Element(0, 0);

    for (unsigned i = 1; i < N; i++) {
      els[i] = new Element(i, 0);
      Manipulator<Vertical>::Insert(els[0], els[i]);
    }

  }

  virtual void TearDown() {
    for (auto el: els) {
      delete el;
    }

    visited.clear();
  }

  template<typename T>
  void VisitWithIter(Iter<T> it) {
    for (; *it; ++it) {
      visited.insert((*it)->rowId);
    }
  }

  template<typename T>
  void ExpectCountsAfterIteration(T it, unsigned expected_size) {
    VisitWithIter(it);
    EXPECT_EQ(visited.size(), expected_size);
  }

  template<typename T>
  void ExpectCountsAfterIterationWithout(T it, unsigned expected_size, int without) {
    ExpectCountsAfterIteration(it, expected_size);
    EXPECT_TRUE(visited.find(without) == end(visited));
  }

  template<typename DIR>
  void ExpectIterationCorrect(Element *start, size_t max_len = 2 * N) {
    unsigned len = 0;
    for (auto cur = Iter<DIR>::All(start); *cur; ++cur) {
      EXPECT_EQ(DIR::Next(DIR::Prev(*cur)), *cur);
      EXPECT_EQ(DIR::Prev(DIR::Next(*cur)), *cur);
      len++;
    }

    EXPECT_FALSE(len == max_len);
  }
};

TEST_F(TestListMatrix, TwoElemsCircularListWhenAddedHorizontal) {
  Element el(0, 0);
  EXPECT_EQ(el.r, &el);

  Element el2(0, 0);
  Manipulator<Horizontal>::Insert(&el, &el2);
  EXPECT_EQ(el.r, &el2);
  EXPECT_EQ(el.l, &el2);
  EXPECT_EQ(el2.r, &el);
  EXPECT_EQ(el2.l, &el);
}

TEST_F(TestListMatrix, TwoElemsCircularListWhenAddedVertical) {
  Element el(0, 0);
  EXPECT_EQ(el.d, &el);

  Element el2(0, 0);
  Manipulator<Vertical>::Insert(&el, &el2);
  EXPECT_EQ(el.d, &el2);
  EXPECT_EQ(el.u, &el2);
  EXPECT_EQ(el2.d, &el);
  EXPECT_EQ(el2.u, &el);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenIterateOverAll) {
  auto it = Iter<Vertical>::All(els[0]);
  ExpectCountsAfterIteration(it, N);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenInvertedIterateOverAll) {
  auto it = Iter<Invert<Vertical>>::All(els[0]);
  ExpectCountsAfterIteration(it, N);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenIterateAllButMe) {
  auto it = Iter<Vertical>::AllButMe(els[0]);
  ExpectCountsAfterIterationWithout(it, N - 1, 0);
}

TEST_F(TestListMatrix, AllRequiredVisitedWehenInvertIterateAllButMe) {
  auto it = Iter<Invert<Vertical>>::AllButMe(els[0]);
  ExpectCountsAfterIterationWithout(it, N - 1, 0);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenFirstRemoved) {
  Manipulator<Vertical>::Remove(els[0]);
  ExpectIterationCorrect<Vertical>(els[1]);
  auto it = Iter<Vertical>::All(els[1]);
  ExpectCountsAfterIterationWithout(it, N - 1, 0);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenSecondRemoved) {
  Manipulator<Vertical>::Remove(els[1]);
  ExpectIterationCorrect<Vertical>(els[0]);
  auto it = Iter<Vertical>::All(els[0]);
  ExpectCountsAfterIterationWithout(it, N - 1, 1);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenLastRemoved) {
  Manipulator<Vertical>::Remove(els[N - 1]);

  ExpectIterationCorrect<Vertical>(els[0]);
  auto it = Iter<Vertical>::All(els[0]);
  ExpectCountsAfterIterationWithout(it, N - 1, 9);
}

TEST_F(TestListMatrix, ListContainsOnlyOtherWhenTwoRemoved) {
  Manipulator<Vertical>::Remove(els[0]);
  Manipulator<Vertical>::Remove(els[1]);

  ExpectIterationCorrect<Vertical>(els[2]);

  auto it = Iter<Vertical>::All(els[2]);
  ExpectCountsAfterIteration(it, N - 2);
  EXPECT_TRUE(visited.find(0) == end(visited));
  EXPECT_TRUE(visited.find(1) == end(visited));
}

TEST_F(TestListMatrix, ListContainsOnlyValidWhenTwoRemovedOneReinsertedInFIFOOrder) {
  Manipulator<Vertical>::Remove(els[0]);
  Manipulator<Vertical>::Remove(els[1]);
  Manipulator<Vertical>::Reinsert(els[1]);

  ExpectIterationCorrect<Vertical>(els[2]);

  auto it = Iter<Vertical>::All(els[2]);
  ExpectCountsAfterIteration(it, N - 1);
  EXPECT_TRUE(visited.find(0) == end(visited));
}