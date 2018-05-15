#ifndef SUPERSTRING_TEXT_BUFFER_H_
#define SUPERSTRING_TEXT_BUFFER_H_

#include <string>
#include <vector>
#include "text.h"
#include "patch.h"
#include "point.h"
#include "range.h"
#include "regex.h"
#include "marker-index.h"

class TextBuffer {
  struct Layer;
  Layer *base_layer;
  Layer *top_layer;
  void squash_layers(const std::vector<Layer *> &);
  void consolidate_layers();

public:
  static uint32_t MAX_CHUNK_SIZE_TO_COPY;

  TextBuffer();
  TextBuffer(std::u16string &&);
  TextBuffer(const std::u16string &text);
  ~TextBuffer();

  uint32_t size() const;
  Point extent() const;
  optional<std::u16string> line_for_row(uint32_t row);
  void with_line_for_row(uint32_t row, const std::function<void(const char16_t *, uint32_t)> &);

  optional<uint32_t> line_length_for_row(uint32_t row);
  const uint16_t *line_ending_for_row(uint32_t row);
  ClipResult clip_position(Point);
  Point position_for_offset(uint32_t offset);
  std::u16string text();
  std::u16string text_in_range(Range range);
  void set_text(std::u16string &&);
  void set_text(const std::u16string &);
  void set_text_in_range(Range old_range, std::u16string &&);
  void set_text_in_range(Range old_range, const std::u16string &);
  bool is_modified() const;
  std::vector<TextSlice> chunks() const;

  void reset(Text &&);
  void flush_changes();
  void serialize_changes(Serializer &);
  bool deserialize_changes(Deserializer &);
  const Text &base_text() const;

  optional<Range> find(const Regex &, Range range = Range::all_inclusive()) const;
  std::vector<Range> find_all(const Regex &, Range range = Range::all_inclusive()) const;
  unsigned find_and_mark_all(MarkerIndex &, MarkerIndex::MarkerId, bool exclusive,
                             const Regex &, Range range = Range::all_inclusive()) const;

  struct SubsequenceMatch {
    std::u16string word;
    std::vector<Point> positions;
    std::vector<uint32_t> match_indices;
    int32_t score;
    bool operator==(const SubsequenceMatch &) const;
  };

  std::vector<SubsequenceMatch> find_words_with_subsequence_in_range(const std::u16string &, const std::u16string &, Range) const;

  class Snapshot {
    friend class TextBuffer;
    TextBuffer &buffer;
    Layer &layer;
    Layer &base_layer;

    Snapshot(TextBuffer &, Layer &, Layer &);

  public:
    ~Snapshot();
    void flush_preceding_changes();

    uint32_t size() const;
    Point extent() const;
    uint32_t line_length_for_row(uint32_t) const;
    std::vector<TextSlice> chunks() const;
    std::vector<TextSlice> chunks_in_range(Range) const;
    std::u16string text() const;
    std::u16string text_in_range(Range) const;
    const Text &base_text() const;
    optional<Range> find(const Regex &, Range range = Range::all_inclusive()) const;
    std::vector<Range> find_all(const Regex &, Range range = Range::all_inclusive()) const;
    std::vector<SubsequenceMatch> find_words_with_subsequence_in_range(std::u16string query, const std::u16string &extra_word_characters, Range range) const;
  };

  friend class Snapshot;
  Snapshot *create_snapshot();

  bool is_modified(const Snapshot *) const;
  Patch get_inverted_changes(const Snapshot *) const;

  size_t layer_count()  const;
  std::string get_dot_graph() const;
};

#endif  // SUPERSTRING_TEXT_BUFFER_H_
