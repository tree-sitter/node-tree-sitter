#ifndef RANGE_H_
#define RANGE_H_

#include <ostream>
#include "point.h"

struct Range {
  Point start;
  Point end;

  static Range all_inclusive();

  Point extent() const;

  bool operator==(const Range &other) const {
    return start == other.start && end == other.end;
  }
};

inline std::ostream &operator<<(std::ostream &stream, const Range &range) {
  return stream << "(" << range.start << ", " << range.end << ")";
}

#endif // RANGE_H_
