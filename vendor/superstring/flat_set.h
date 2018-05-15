#ifndef SUPERSTRING_FLAT_SET_H
#define SUPERSTRING_FLAT_SET_H

#include <vector>
#include <algorithm>

template <typename T> class flat_set {
  typedef std::vector<T> contents_type;
  contents_type contents;

public:
  typedef typename contents_type::iterator iterator;
  typedef typename contents_type::const_iterator const_iterator;

  void insert(T value) {
    auto iter = std::lower_bound(contents.begin(), contents.end(), value);
    if (iter == contents.end() || *iter != value) {
      contents.insert(iter, value);
    }
  }

  void insert(const_iterator start, const_iterator end) {
    for (auto i = start; i != end; i++) {
      insert(*i);
    }
  }

  iterator erase(const iterator &iter) {
    return contents.erase(iter);
  }

  void erase(T value) {
    auto end = this->end();
    auto iter = std::lower_bound(begin(), end, value);
    if (iter != end && *iter == value) {
      erase(iter);
    }
  }

  iterator begin() {
    return contents.begin();
  }

  const_iterator begin() const {
    return contents.begin();
  }

  iterator end() {
    return contents.end();
  }

  const_iterator end() const {
    return contents.end();
  }

  size_t count(T value) const {
    return std::binary_search(contents.begin(), contents.end(), value) ? 1 : 0;
  }

  size_t size() const {
    return contents.size();
  }
};

#endif // SUPERSTRING_FLAT_SET_H
