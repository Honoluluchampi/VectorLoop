#pragma once

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <cassert>

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
  std::string tag;
  std::string content;
  bool is_end = false;
};

// extract the first field of the input text
Field extract_field(std::string& input) {
  int pos = 0;
  // detect tag start
  while (input[pos] != '<') { pos++; }
  int start = pos;
  // check weather this field is end field
  bool is_end = false;
  if (input[start + 1] == '\\')
    is_end = true;
  // get tag
  while (input[pos] != ' ') { pos++; }
  int tag_end = pos;
  // get field
  while (input[pos] != '>') { pos++; }
  int field_end = pos;

  Field field = {
    input.substr(start + 1, tag_end - start - 1),
    input.substr(tag_end + 1, field_end - tag_end - 1),
    is_end
  };

  input = input.substr(field_end + 1);

  return field;
}

Field extract_attribute(std::string& input) {
  int pos = 0;
  // extract attr tag
  while (input[pos] == ' ') { pos++; }
  int attr_start = pos;
  while (input[pos] != '=') { pos++; }
  int attr_end = pos;

  auto attr = input.substr(attr_start, attr_end - attr_start);

  // extract attr content
  while (input[pos] != '\"') { pos++; }
  int cont_start = pos + 1;
  pos++;
  while (input[pos] != '\"') { pos++; }
  int cont_end = pos;

  auto cont = input.substr(cont_start, cont_end - cont_start);

  input = input.substr(cont_end + 1);

  return Field { attr, cont, false };
}

template <typename T>
Path<T> process_path(const std::string& path_string) {

}

template <typename T>
Path<T> parse_svg(const std::string& filepath) {
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
  assert(svg_field.tag == "svg");

  // also ignore g field and extract path field
  Field new_field;
  while (new_field.tag != "g") {
    new_field = extract_field(file_contents);
  }
  while (new_field.tag != "path") {
    new_field = extract_field(file_contents);
  }

  // search 'd' attribute
  Field attr;
  while (attr.tag != "d") {
    attr = extract_attribute(new_field.content);
  }

  return process_path<T>(new_field.content);
}

} // namespace VL