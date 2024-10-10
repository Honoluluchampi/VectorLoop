#pragma once

#include <string>
#include <vector>
#include <array>
#include <fstream>

namespace VL {

template <typename T>
struct Vec2 {
  T x, y;
  static Vec2 zero() { return { 0, 0 }; }
};

enum class SegType{
  Line,
  QBezier, // Quadratic Bezier
  CBezier, // Cubic Bezier
};

template <typename T>
class Segment {
  public:
    Segment(const std::array<Vec2<T>, 4>& points, const SegType type)
      : points_(points), type_(type) {}

    static Segment create_line(const Vec2<T>& s, const Vec2<T>& e)
    { return Segment({ s, e, Vec2<T>::zero(), Vec2<T>::zero() }, SegType::Line); }

    static Segment create_q_bezier(const Vec2<T>& s, const Vec2<T>& e, const Vec2<T>& c)
    { return Segment({ s, e, c, Vec2<T>::zero() }, SegType::QBezier); }

    static Segment create_c_bezier(const Vec2<T>& s, const Vec2<T>& e, const Vec2<T>& c0, const Vec2<T>& c1)
    { return Segment({ s, e, c0, c1 }, SegType::CBezier); }

  private:
    std::array<Vec2<T>, 4> points_; // start point, end point, control points (if necessary)
    SegType type_;
};

template <typename T>
class Path {
  public:

  private:
    std::vector<Segment<T>> segments_;
    bool is_closed_ = true;
};

struct Field {
  std::string tag; std::string field;
};

// extract the first field of the input text
Field extract_field(std::string& input) {
  int pos = 0;
  // detect tag start
  while (input[pos] != '<') { pos++; }
  int start = pos;
  // get tag
  while (input[pos] != ' ') { pos++; }
  int tag_end = pos;
  // get field
  while (input[pos] != '>') { pos++; }
  int field_end = pos;

  Field field = {
    input.substr(start + 1, tag_end - start - 1),
    input.substr(tag_end + 1, field_end - tag_end - 1)
  };

  input = input.substr(field_end + 1);

  return field;
}

template <typename T>
Path<T> extract_path(const std::string& filepath) {
  // read whole file
  std::string line;
  std::string file_contents;
  std::ifstream file_in(filepath);
  while (std::getline(file_in, line)) {
    file_contents += line;
  }
  file_in.close();

  // ignore svg field
  auto svg_field = extract_field(file_contents);

}

// returns polyloop's coordinates in one line
template <typename T>
std::vector<T> parse_svg() {}

} // namespace VL