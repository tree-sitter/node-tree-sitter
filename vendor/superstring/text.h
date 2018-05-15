#ifndef SUPERSTRING_TEXT_H_
#define SUPERSTRING_TEXT_H_

#include <istream>
#include <functional>
#include <vector>
#include <ostream>
#include "serializer.h"
#include "point.h"
#include "optional.h"

class TextSlice;

struct ClipResult {
  Point position;
  uint32_t offset;
};

class Text {
  friend class TextSlice;

 public:
  static Point extent(const std::u16string &);

  std::u16string content;
  std::vector<uint32_t> line_offsets;
  Text(const std::u16string &&, const std::vector<uint32_t> &&);

  using const_iterator = std::u16string::const_iterator;

  Text();
  Text(const std::u16string &);
  Text(std::u16string &&);
  Text(TextSlice slice);
  Text(Deserializer &deserializer);
  template<typename Iter>
  Text(Iter begin, Iter end) : Text(std::u16string{begin, end}) {}

  static Text concat(TextSlice a, TextSlice b);
  static Text concat(TextSlice a, TextSlice b, TextSlice c);
  void splice(Point start, Point deletion_extent, TextSlice inserted_slice);

  uint16_t at(Point position) const;
  uint16_t at(uint32_t offset) const;
  const_iterator begin() const;
  const_iterator end() const;
  inline const_iterator cbegin() const { return begin(); }
  inline const_iterator cend() const { return end(); }
  ClipResult clip_position(Point) const;
  Point extent() const;
  bool empty() const;
  uint32_t offset_for_position(Point) const;
  Point position_for_offset(uint32_t, uint32_t min_row = 0, bool clip_crlf = true) const;
  uint32_t line_length_for_row(uint32_t row) const;
  void append(TextSlice);
  void assign(TextSlice);
  void serialize(Serializer &) const;
  uint32_t size() const;
  const char16_t *data() const;
  size_t digest() const;
  void clear();

  bool operator!=(const Text &) const;
  bool operator==(const Text &) const;

  friend std::ostream &operator<<(std::ostream &, const Text &);
};

#endif // SUPERSTRING_TEXT_H_
