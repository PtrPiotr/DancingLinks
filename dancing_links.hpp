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

#include <vector>
#include <sstream>
#include <cassert>
#include <cmath>

//#include <experimental/memory_resource>

namespace DancingLinks {

namespace Internal {
template<typename T>
struct Alloc {
  std::vector<T *> allocs;

  virtual ~Alloc() {
    for (auto e: allocs) {
      delete e;
    }
  }

  template<typename... Args>
  T *allocate(Args &&... args) {
    allocs.emplace_back(new T(std::forward<Args>(args)...));
    return allocs.back();
  }
};

struct El {
  int rowId, colId;
  El *u, *d, *l, *r;

  El(int rowId, int colId) :
      rowId(rowId), colId(colId),
      u(this), d(this), l(this), r(this) {

  }
};

struct Header : public El {
  long count;

  Header(int rowId, int colId) :
      El(rowId, colId), count(0) {

  }
};

struct Horizontal {
  static El *next(El *in) { return in->r; }

  static El **nextp(El *in) { return &in->r; }

  static El *prev(El *in) { return in->l; }

  static El **prevp(El *in) { return &in->l; }
};

struct Vertical {
  static El *next(El *in) { return in->d; }

  static El **nextp(El *in) { return &in->d; }

  static El *prev(El *in) { return in->u; }

  static El **prevp(El *in) { return &in->u; }
};

template<typename HV>
struct Invert {
  static El *next(El *in) { return HV::prev(in); }

  static El **nextp(El *in) { return HV::prevp(in); }

  static El *prev(El *in) { return HV::next(in); }

  static El **prevp(El *in) { return HV::nextp(in); }
};

template<typename DIR>
struct Manipulator {
  static void insert(El *after, El *what) {
    *DIR::nextp(what) = DIR::next(after);
    *DIR::prevp(what) = after;

    *DIR::prevp(DIR::next(after)) = what;
    *DIR::nextp(after) = what;
  }

  static void remove(El *cur) {
    *DIR::nextp(DIR::prev(cur)) = DIR::next(cur);
    *DIR::prevp(DIR::next(cur)) = DIR::prev(cur);
  }

  static void reinsert(El *cur) {
    *DIR::nextp(DIR::prev(cur)) = cur;
    *DIR::prevp(DIR::next(cur)) = cur;
  }
};

template<typename DIR>
struct Iter {
  El *cur;
  El *end;
  bool started;

  static Iter all_but_me(El *me) {
    return Iter<DIR> {DIR::next(me), me, true};
  }

  static Iter all(El *me) {
    return Iter<DIR> {me, me, false};
  }

  Iter<DIR> &operator++() {
    if (started && cur == end) return *this;
    started = true;
    cur = DIR::next(cur);
    return *this;
  }

  Iter<DIR> &operator++(int q) {
    Iter<DIR> pom = *this;
    ++(*this);
    return pom;
  }

  El *operator*() {
    if (started && cur == end) return nullptr;
    return cur;
  }

 private:
  Iter(El *cur, El *end, bool started) : cur(cur), end(end), started(started) {

  }
};

class DLSolver {
 protected:

  size_t n_rows, n_cols;

  Header *root;
  Header **cols;
  El **rows;

  std::vector<int> solution;

  //std::experimental::fundamentals_v2::pmr::monotonic_buffer_resource
  Alloc<El> el_alloc;
  Alloc<Header> header_alloc;

  void cover(Header *head) {
    Manipulator<Horizontal>::remove(head);

    for (auto row = Iter<Vertical>::all_but_me(head); *row; ++row) {
      for (auto cur = Iter<Horizontal>::all_but_me(*row); *cur; ++cur) {
        Manipulator<Vertical>::remove(*cur);
        cols[(*cur)->colId]->count--;
      }
    }

    assert(head->count >= 0);
  }

  void uncover(Header *head) {

    for (auto row = Iter<Invert<Vertical>>::all_but_me(head); *row; ++row) {
      for (auto cur = Iter<Invert<Horizontal>>::all_but_me(*row); *cur; ++cur) {
        cols[(*cur)->colId]->count++;
        Manipulator<Vertical>::reinsert(*cur);
      }
    }

    Manipulator<Horizontal>::reinsert(head);
  }

  void cover_row(El *row) {
    for (auto cur = Iter<Horizontal>::all_but_me(row); *cur; ++cur) {
      cover(cols[(*cur)->colId]);
    }
  }

  void uncover_row(El *row) {
    for (auto cur = Iter<Invert<Horizontal>>::all_but_me(row); *cur; ++cur) {
      uncover(cols[(*cur)->colId]);
    }
  }

  Header *get_small_column() {
    Header *ret = nullptr;
    for (auto it = Iter<Horizontal>::all_but_me(root); *it; ++it) {
      Header *h = (Header *) *it;
      if (ret == nullptr || h->count < ret->count) {
        ret = h;
      }
    }

    return ret;
  }

  unsigned solve(unsigned step) {
    if (root == root->r) {
      return step;
    }

    Header *header = get_small_column();

    if (header == header->d) {
      return 0;
    }

    cover(header);
    for (auto row = Iter<Vertical>::all_but_me(header); *row; ++row) {
      solution[step] = (*row)->rowId;
      cover_row(*row);

      unsigned solved = solve(step + 1);
      if (solved != 0) return solved;

      uncover_row(*row);
      solution[step] = -1;
    }
    uncover(header);

    return 0;
  }

 public:
  /***
   * Instance of the solver.
   * @param n_rows upper limit of the number of provided rows
   * @param n_cols number of columns
   */
  DLSolver(unsigned n_rows, unsigned n_cols)
      : n_rows(n_rows), n_cols(n_cols) {
    rows = new El *[n_rows];
    cols = new Header *[n_cols];

    solution.assign(n_rows, 0);

    for (unsigned i = 0; i < n_rows; i++) {
      rows[i] = nullptr;
    }

    root = header_alloc.allocate(-10, -10);
    root->d = nullptr;
    root->u = nullptr;

    for (int i = (int) n_cols - 1; i >= 0; i--) {
      cols[i] = header_alloc.allocate(-1, i);
      Manipulator<Horizontal>::insert(root, cols[i]);
    }
  }

  virtual ~DLSolver() {
    delete[] rows;
    delete[] cols;
  }

  /**
   * add "one" to the Algorithm X matrix
   * @param rowId row number
   * @param colId column number
   */
  void add(unsigned rowId, unsigned colId) {
    assert(rowId < n_rows && colId < n_cols);
    El *me = el_alloc.allocate((int) rowId, (int) colId);

    Manipulator<Vertical>::insert(cols[colId], me);
    cols[colId]->count++;

    if (!rows[rowId]) {
      rows[rowId] = me;
    } else {
      Manipulator<Horizontal>::insert(rows[rowId], me);
    }
  }


  /**
   * Delete given row from the Algorithm X matrix. This is useful if you create generic instance
   * if the problem first, and than adjust it by marking few positions as impossible.
   * @param row_id id of the row to remove
   */
  void delete_row(unsigned row_id) {
    El *row = rows[row_id];

    for (auto cur = Iter<Horizontal>::all(row); *cur; ++cur) {
      Header *h = cols[(*cur)->colId];
      if (h->r->l == h && h->l->r == h) {
        // delete only if this column was not deleted before
        cover(h);
      }
    }
  }

  /**
   * Solve this instance.
   * @return vector containing ids of rows included in the solution. RowId are consistant
   * with ids provided in  "add" and "delete" methods.
   */
  std::vector<int> solve() {
    int ret = solve(0);
    return std::vector<int>(begin(solution), begin(solution) + ret);
  }
};

}

using Internal::DLSolver;
}
