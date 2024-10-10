#pragma once

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <cassert>
#include <iostream>

namespace VL {

template <typename T>
struct Vec2 {
  T x, y;
  static Vec2 zero() { return { 0, 0 }; }

  Vec2 operator+(const Vec2& v) { return { x + v.x, y + v.y }; }
  Vec2 operator-(const Vec2& v) { return { x - v.x, y - v.y }; }
};

enum class SegType{
  Line,
  QBezier, // Quadratic Bezier
  CBezier, // Cubic Bezier
};

// all the coordinates are absolute value
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
using Path = std::vector<Segment<T>>;

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

void erase_specific_string(std::string& input, const char c) {
  int pos = 0;
  while (pos < input.length()) {
    bool remove_flag = false;
    while (input[pos] != c && pos < input.length()) {
      pos++;
      remove_flag = true;
    }
    if (remove_flag)
      input.erase(pos, 1);
  }
}

template <typename T>
std::vector<T> extract_values(std::string& input) {
  int pos = 0;

  auto is_letter = [](const char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); };

  // extract first segment
  while (!is_letter(input[pos]) && pos < input.length()) { pos++; }
  auto value_str = input.substr(0, pos);
  input = input.substr(pos);

  std::vector<T> raw_values;
  int current_pos = 1;
  int last_pos = 0;
  while (current_pos < value_str.length()) {
    if (value_str[current_pos] == ',') {
      raw_values.push_back(static_cast<T>(std::stod(value_str.substr(last_pos, current_pos))));
      last_pos = current_pos + 1;
    }
    else if (value_str[current_pos] == '-') {
      raw_values.push_back(static_cast<T>(std::stod(value_str.substr(last_pos, current_pos))));
      last_pos = current_pos;
    }
    else if (current_pos == value_str.length() - 1) {
      raw_values.push_back(static_cast<T>(std::stod(value_str.substr(last_pos, current_pos))));
    }
    current_pos++;
  }

  return raw_values;
}

template <typename T>
Path<T> process_path(std::string& path_string) {
  // erase space
  erase_specific_string(path_string, ' ');

  Vec2<T> last_point = Vec2<T>::zero();

  // check closed
  bool is_closed = false;
  if (path_string[path_string.length() - 1] == 'z' || path_string[path_string.length() - 1] == 'Z') {
    is_closed = true;
  }
  assert (is_closed && "path must be closed.");

  // get beginning point
  char type = path_string[0];
  path_string = path_string.substr(1);
  auto values = extract_values<T>(path_string);
  assert(type == 'm' || type == 'M');
  assert(values.size() == 2);

  std::vector<Segment<T>> segments;
  Vec2<T> current_point = { values[0], values[1]};
  Vec2<T> end_point;
  Vec2<T> previous_control;

  while (path_string != "z" && path_string != "Z") {
    type = path_string[0];
    path_string = path_string.substr(1);
    values = extract_values<T>(path_string);

    switch (type) {
      case 'l' : {
        assert(values.size() == 2);
        end_point = current_point + Vec2<T>{ values[0], values[1] };
        segments.emplace_back(Segment<T>::create_line(current_point, end_point));
        break;
      }
      case 'L' : {
        assert(values.size() == 2);
        end_point = Vec2<T>{ values[0], values[1] };
        segments.emplace_back(Segment<T>::create_line(current_point, end_point));
        break;
      }
      case 'h' : {
        assert(values.size() == 1);
        end_point = Vec2<T>{ current_point.x + values[0], current_point.y };
        segments.emplace_back(Segment<T>::create_line(current_point, end_point));
        break;
      }
      case 'H' : {
        assert(values.size() == 1);
        end_point = Vec2<T>{ values[0], current_point.y };
        segments.emplace_back(Segment<T>::create_line(current_point, end_point));
        break;
      }
      case 'v' : {
        assert(values.size() == 1);
        end_point = Vec2<T>{ current_point.x, current_point.y + values[0] };
        segments.emplace_back(Segment<T>::create_line(current_point, end_point));
        break;
      }
      case 'V' : {
        assert(values.size() == 1);
        end_point = Vec2<T>{ current_point.x, values[0] };
        segments.emplace_back(Segment<T>::create_line(current_point, end_point));
        break;
      }
      case 'q' : {
        assert(values.size() == 4);
        auto cp = Vec2<T>{ values[0], values[1] } + current_point;
        previous_control = cp;
        end_point = Vec2<T>{ values[2], values[3] } + current_point;
        segments.emplace_back(Segment<T>::create_q_bezier(current_point, end_point, cp));
        break;
      }
      case 'Q' : {
        assert(values.size() == 4);
        auto cp = Vec2<T>{ values[0], values[1] };
        previous_control = cp;
        end_point = Vec2<T>{ values[2], values[3] };
        segments.emplace_back(Segment<T>::create_q_bezier(current_point, end_point, cp));
        break;
      }
      case 't' : {
        assert(values.size() == 2);
        auto cp = current_point - (previous_control + current_point);
        previous_control = cp;
        end_point = Vec2<T>{ values[0], values[1] } + current_point;
        segments.emplace_back(Segment<T>::create_q_bezier(current_point, end_point, cp));
        break;
      }
      case 'T' : {
        assert(values.size() == 2);
        auto cp = current_point - (previous_control + current_point);
        previous_control = cp;
        end_point = Vec2<T>{ values[0], values[1] };
        segments.emplace_back(Segment<T>::create_q_bezier(current_point, end_point, cp));
        break;
      }
      case 'c' : {
        assert(values.size() == 6);
        auto cp0 = Vec2<T>{ values[0], values[1] } + current_point;
        auto cp1 = Vec2<T>{ values[2], values[3] } + current_point;
        previous_control = cp1;
        end_point = Vec2<T>{ values[4], values[5] } + current_point;
        segments.emplace_back(Segment<T>::create_c_bezier(current_point, end_point, cp0, cp1));
        break;
      }
      case 'C' : {
        assert(values.size() == 6);
        auto cp0 = Vec2<T>{ values[0], values[1] };
        auto cp1 = Vec2<T>{ values[2], values[3] };
        previous_control = cp1;
        end_point = Vec2<T>{ values[4], values[5] };
        segments.emplace_back(Segment<T>::create_c_bezier(current_point, end_point, cp0, cp1));
        break;
      }
      case 's' : {
        assert(values.size() == 4);
        auto cp0 = current_point - (previous_control - current_point);
        auto cp1 = Vec2<T>{ values[0], values[1] } + current_point;
        previous_control = cp1;
        end_point = Vec2<T>{ values[2], values[3] } + current_point;
        segments.emplace_back(Segment<T>::create_c_bezier(current_point, end_point, cp0, cp1));
        break;
      }
      case 'S' : {
        assert(values.size() == 4);
        auto cp0 = current_point - (previous_control - current_point);
        auto cp1 = Vec2<T>{ values[0], values[1] };
        previous_control = cp1;
        end_point = Vec2<T>{ values[2], values[3] };
        segments.emplace_back(Segment<T>::create_c_bezier(current_point, end_point, cp0, cp1));
        break;
      }
      default : {
        std::cerr << "unsupported path type." << std::endl;
        break;
      }
    }
    current_point = end_point;
  }

  return segments;
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

  return process_path<T>(attr.content);
}

} // namespace VL