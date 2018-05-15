#ifndef PATCH_H_
#define PATCH_H_

#include "optional.h"
#include "point.h"
#include "serializer.h"
#include "text.h"
#include <memory>
#include <vector>
#include <ostream>

class Patch {
  struct Node;
  struct OldCoordinates;
  struct NewCoordinates;
  struct PositionStackEntry;

  Node *root;
  std::vector<Node *> node_stack;
  std::vector<PositionStackEntry> left_ancestor_stack;
  uint32_t change_count;
  bool merges_adjacent_changes;

public:
  struct Change {
    Point old_start;
    Point old_end;
    Point new_start;
    Point new_end;
    Text *old_text;
    Text *new_text;
    uint32_t preceding_old_text_size;
    uint32_t preceding_new_text_size;
    uint32_t old_text_size;
  };

  // Construction and destruction
  Patch(bool merges_adjacent_changes = true);
  Patch(Patch &&);
  Patch(Deserializer &input);
  Patch &operator=(Patch &&);
  ~Patch();
  void serialize(Serializer &serializer);

  Patch copy();
  Patch invert();

  // Mutations
  bool splice(Point new_splice_start,
              Point new_deletion_extent, Point new_insertion_extent,
              optional<Text> &&deleted_text = optional<Text>{},
              optional<Text> &&inserted_text = optional<Text>{},
              uint32_t deleted_text_size = 0);
  void splice_old(Point start, Point deletion_extent, Point insertion_extent);
  bool combine(const Patch &other, bool left_to_right = true);
  void clear();
  void rebalance();

  // Non-splaying reads
  std::vector<Change> get_changes() const;
  size_t get_change_count() const;
  std::vector<Change> get_changes_in_old_range(Point start, Point end) const;
  std::vector<Change> get_changes_in_new_range(Point start, Point end) const;
  optional<Change> get_change_starting_before_old_position(Point position) const;
  optional<Change> get_change_starting_before_new_position(Point position) const;
  optional<Change> get_change_ending_after_new_position(Point position) const;
  optional<Change> get_bounds() const;
  Point new_position_for_new_offset(uint32_t new_offset,
                                    std::function<uint32_t(Point)> old_offset_for_old_position,
                                    std::function<Point(uint32_t)> old_position_for_old_offset) const;

  // Splaying reads
  std::vector<Change> grab_changes_in_old_range(Point start, Point end);
  std::vector<Change> grab_changes_in_new_range(Point start, Point end);
  optional<Change> grab_change_starting_before_old_position(Point position);
  optional<Change> grab_change_starting_before_new_position(Point position);
  optional<Change> grab_change_ending_after_new_position(Point position, bool exclusive = false);

  // Debugging
  std::string get_dot_graph() const;
  std::string get_json() const;

private:
  Patch(Node *root, uint32_t change_count, bool merges_adjacent_changes);

  template <typename CoordinateSpace>
  std::vector<Change> get_changes_in_range(Point, Point, bool inclusive) const;

  template <typename CoordinateSpace>
  optional<Change> get_change_starting_before_position(Point target) const;

  template <typename CoordinateSpace>
  optional<Change> get_change_ending_after_position(Point target) const;

  template <typename CoordinateSpace>
  std::vector<Change> grab_changes_in_range(Point, Point, bool inclusive = false);

  template <typename CoordinateSpace>
  optional<Change> grab_change_starting_before_position(Point position);

  template <typename CoordinateSpace>
  Node *splay_node_starting_before(Point target);

  template <typename CoordinateSpace>
  Node *splay_node_starting_after(Point target, optional<Point> exclusive_lower_bound);

  template <typename CoordinateSpace>
  Node *splay_node_ending_before(Point target);

  template <typename CoordinateSpace>
  Node *splay_node_ending_after(Point target, optional<Point> exclusive_lower_bound);

  Change change_for_root_node();

  std::pair<optional<Text>, bool> compute_old_text(optional<Text> &&, Point, Point);
  uint32_t compute_old_text_size(uint32_t, Point, Point);

  void splay_node(Node *);
  void rotate_node_right(Node *, Node *, Node *);
  void rotate_node_left(Node *, Node *, Node *);
  void delete_root();
  void perform_rebalancing_rotations(uint32_t);
  Node *build_node(Node *, Node *, Point, Point, Point, Point,
                  optional<Text> &&, optional<Text> &&, uint32_t old_text_size);
  void delete_node(Node **);
  void remove_noop_change();
};

std::ostream &operator<<(std::ostream &, const Patch::Change &);

#endif // PATCH_H_
