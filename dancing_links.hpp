/**
MIT License

Copyright (c) 2017 piotr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef DANCING_LINKS_HPP_
#define DANCING_LINKS_HPP_

#include <vector>
#include <sstream>
#include <cassert>
#include <cmath>

#include <memory_resource>

namespace DancingLinks {

  namespace Internal
  {
    const int HEADER_ROOT = -10;

struct Element {
  int rowId, colId;
  // pointers up, down, left, right of the grid matrix.
  Element *u, *d, *l, *r;


  Element(int rowId, int colId) :
      rowId(rowId), colId(colId),
      u(this), d(this), l(this), r(this) {
  }
};

struct Header : public Element {
  long count;

  Header(int rowId, int colId) :
      Element(rowId, colId), count(0) {

  }
};

struct Horizontal {
  static Element *Next(Element *in) { return in->r; }

  static Element **NextP(Element *in) { return &in->r; }

  static Element *Prev(Element *in) { return in->l; }

  static Element **PrevP(Element *in) { return &in->l; }
};

struct Vertical {
  static Element *Next(Element *in) { return in->d; }

  static Element **NextP(Element *in) { return &in->d; }

  static Element *Prev(Element *in) { return in->u; }

  static Element **PrevP(Element *in) { return &in->u; }
};

template<typename HV>
struct Invert {
  static Element *Next(Element *in) { return HV::Prev(in); }

  static Element **NextP(Element *in) { return HV::Prevp(in); }

  static Element *Prev(Element *in) { return HV::Next(in); }

  static Element **PrevP(Element *in) { return HV::Nextp(in); }
};

template<typename DIR>
struct Manipulator {
  static void Insert(Element *after, Element *what) {
    *DIR::NextP(what) = DIR::Next(after);
    *DIR::PrevP(what) = after;

    *DIR::PrevP(DIR::Next(after)) = what;
    *DIR::NextP(after) = what;
  }

  static void Remove(Element *cur) {
    *DIR::NextP(DIR::Prev(cur)) = DIR::Next(cur);
    *DIR::PrevP(DIR::Next(cur)) = DIR::Prev(cur);
  }

  static void Reinsert(Element *cur) {
    *DIR::NextP(DIR::Prev(cur)) = cur;
    *DIR::PrevP(DIR::Next(cur)) = cur;
  }
};

template<typename DIR>
class Iter {

 public:
  static Iter AllButMe(Element *me) {
    return Iter<DIR> {DIR::Next(me), me, true};
  }

  static Iter All(Element *me) {
    return Iter<DIR> {me, me, false};
  }

  Iter<DIR> &operator++() {
    if (started && cur == end) return *this;
    started = true;
    cur = DIR::Next(cur);
    return *this;
  }

  Iter<DIR> &operator++(int q)  {
    Iter<DIR> pom = *this;
    ++(*this);
    return pom;
  }

  Element *operator*() {
    if (started && cur == end) return nullptr;
    return cur;
  }

 private:
  Element *cur;
  Element *end;
  bool started;

  Iter(Element *cur, Element *end, bool started) : cur(cur), end(end), started(started) {

  }
};

class DLSolver final {
 public:
  DLSolver(const DLSolver&) = delete;
  DLSolver& operator=(const DLSolver&) = delete;

  /***
   * Instance of the solver.
   * @param n_rows upper limit of the number of provided rows
   * @param n_cols number of columns
   */
  DLSolver(unsigned n_rows, unsigned n_cols)
      : n_rows(n_rows), n_cols(n_cols), rows(n_rows), cols(n_cols),
        el_alloc(&memory_resource), header_alloc(&memory_resource) {
    solution.assign(n_rows, 0);

    for (unsigned i = 0; i < n_rows; i++) {
      rows[i] = nullptr;
    }

    root = header_alloc.allocate(1);
    new(root) Header(-10, -10);
    root->d = nullptr;
    root->u = nullptr;

    for (int i = (int) n_cols - 1; i >= 0; i--) {
      cols[i] = header_alloc.allocate(1);
      new(cols[i]) Header(-1, i);
      Manipulator<Horizontal>::Insert(root, cols[i]);
    }
  }

  /**
   * add "one" to the Algorithm X matrix
   * @param rowId row number
   * @param colId column number
   */
  void Add(unsigned rowId, unsigned colId) {
    assert(rowId < n_rows && colId < n_cols);
    Element *me = el_alloc.allocate(1);
    new(me) Element((int) rowId, (int) colId);

    Manipulator<Vertical>::Insert(cols[colId], me);
    cols[colId]->count++;

    if (!rows[rowId]) {
      rows[rowId] = me;
    } else {
      Manipulator<Horizontal>::Insert(rows[rowId], me);
    }
  }


  /**
   * Delete given row from the Algorithm X matrix. This is useful if you create generic instance
   * if the problem first, and than adjust it by marking few positions as impossible.
   * @param row_id id of the row to remove
   */
  void DeleteRow(unsigned row_id) {
    Element *row = rows[row_id];

    for (auto cur = Iter<Horizontal>::All(row); *cur; ++cur) {
      Header *h = cols[(*cur)->colId];
      if (h->r->l == h && h->l->r == h) {
        // delete only if this column was not deleted before
        Cover(h);
      }
    }
  }

  /**
   * Solve this instance.
   * @return vector containing ids of rows included in the solution. RowId are consistant
   * with ids provided in  "add" and "delete" methods.
   */
  std::vector<int> Solve() {
    int ret = Solve(0);
    return std::vector<int>(begin(solution), begin(solution) + ret);
  }

 protected:

  size_t n_rows, n_cols;

  Header *root;
  std::vector<Element*> rows;
  std::vector<Header*> cols;

  std::vector<int> solution;

  std::pmr::monotonic_buffer_resource memory_resource;
  std::pmr::polymorphic_allocator<Element> el_alloc;
  std::pmr::polymorphic_allocator<Header> header_alloc;

  void Cover(Header *head) {
    Manipulator<Horizontal>::Remove(head);

    for (auto row = Iter<Vertical>::AllButMe(head); *row; ++row) {
      for (auto cur = Iter<Horizontal>::AllButMe(*row); *cur; ++cur) {
        Manipulator<Vertical>::Remove(*cur);
        cols[(*cur)->colId]->count--;
      }
    }

    assert(head->count >= 0);
  }

  void Uncover(Header *head) {

    for (auto row = Iter<Invert<Vertical>>::AllButMe(head); *row; ++row) {
      for (auto cur = Iter<Invert<Horizontal>>::AllButMe(*row); *cur; ++cur) {
        cols[(*cur)->colId]->count++;
        Manipulator<Vertical>::Reinsert(*cur);
      }
    }

    Manipulator<Horizontal>::Reinsert(head);
  }

  void CoverRow(Element *row) {
    for (auto cur = Iter<Horizontal>::AllButMe(row); *cur; ++cur) {
      Cover(cols[(*cur)->colId]);
    }
  }

  void UncoverRow(Element *row) {
    for (auto cur = Iter<Invert<Horizontal>>::AllButMe(row); *cur; ++cur) {
      Uncover(cols[(*cur)->colId]);
    }
  }

  Header *GetSmallColumn() {
    Header *ret = nullptr;
    for (auto it = Iter<Horizontal>::AllButMe(root); *it; ++it) {
      Header *h = (Header *) *it;
      if (ret == nullptr || h->count < ret->count) {
        ret = h;
      }
    }

    return ret;
  }

  unsigned Solve(unsigned step) {
    if (root == root->r) {
      return step;
    }

    Header *header = GetSmallColumn();

    if (header == header->d) {
      return 0;
    }

    Cover(header);
    for (auto row = Iter<Vertical>::AllButMe(header); *row; ++row) {
      solution[step] = (*row)->rowId;
      CoverRow(*row);

      unsigned solved = Solve(step + 1);
      if (solved != 0) return solved;

      UncoverRow(*row);
      solution[step] = -1;
    }
    Uncover(header);

    return 0;
  }
};

}

using Internal::DLSolver;
}

#endif
