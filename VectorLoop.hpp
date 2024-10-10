#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <cassert>
#include <complex.h>
#include <iostream>

namespace VL {

template <typename T>
struct Vec2 {
  T x, y;
  static Vec2 zero() { return { 0, 0 }; }

  Vec2 operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
  Vec2 operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
  Vec2 operator*=(const T& s) { x *= s; y *= s; return *this; }
};

template <typename T>
Vec2<T> operator+(const Vec2<T>& v1, const Vec2<T>& v2) { return { v1.x + v2.x, v1.y + v2.y }; }
template <typename T>
Vec2<T> operator-(const Vec2<T>& v1, const Vec2<T>& v2) { return { v1.x - v2.x, v1.y - v2.y }; }
template <typename T>
Vec2<T> operator*(const T& s, const Vec2<T>& v) { return { s * v.x, s * v.y }; }
template <typename T>
Vec2<T> operator*(const Vec2<T>& v, const T& s) { return { s * v.x, s * v.y }; }

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

    const std::array<Vec2<T>, 4>& points() const { return points_; }
    SegType type() const { return type_; }

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

template <int N, typename T>
Vec2<T> sample_bezier(T param, const Segment<T>& bezier) {
  assert(N == 2 || N == 3);
  assert(0 <= param && param <= 1);

  // value for computation
  const auto& points = bezier.points();
  constexpr T nCk[2][4] = { { 1, 2, 1, 0 }, { 1, 3, 3, 1 } };

  auto ret = Vec2<T>::zero();
  for (int k = 0; k < N + 1; k++) {
    auto coef = static_cast<T>(nCk[N - 2][k] * std::pow(param, k) * std::pow(1. - param, N - k));
    ret += Vec2<T>{ coef * points[k].x, coef * points[k].y };
  }

  return ret;
}

template <typename T>
T distance(const Vec2<T>& a, const Vec2<T>& b) {
  auto diff = a - b;
  return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

template <typename T>
std::vector<Vec2<T>> polyrize_pathloop(const Path<T>& pathloop, int div_count) {
  // calculate whole length and each segment length
  const int NUM_SEG_FOR_DIST = 10;
  size_t num_segment = pathloop.size();

  T whole_length = 0.;
  std::vector<T> segment_lengths(num_segment, 0);

  for (int i = 0; i < num_segment; i++) {
    const auto& segment = pathloop[i];
    const auto& points = segment.points();
    T local_length = 0.;

    switch (segment.type()) {
      case SegType::Line : {
        local_length += distance(points[0], points[1]);
        break;
      }
      case SegType::QBezier : {
        auto current_point = points[0];
        for (int j = 0; j < NUM_SEG_FOR_DIST; j++) {
          T param = T(j) / T(NUM_SEG_FOR_DIST);
          auto next_point = sample_bezier<2, T>(param, segment);
          local_length += distance(current_point, next_point);
          current_point = next_point;
        }
        break;
      }
      case SegType::CBezier : {
        auto current_point = points[0];
        for (int j = 0; j < NUM_SEG_FOR_DIST; j++) {
          T param = T(j) / T(NUM_SEG_FOR_DIST);
          auto next_point = sample_bezier<3, T>(param, segment);
          local_length += distance(current_point, next_point);
          current_point = next_point;
        }
        break;
      }
    }
    segment_lengths[i] = local_length;
    whole_length += local_length;
  }

  // sample points on the loop
  std::vector<Vec2<T>> ret;
  ret.reserve(div_count);
  for (int i = 0; i < num_segment; i++) {
    const auto& segment = pathloop[i];
    const auto& points = segment.points();
    int local_div = static_cast<int>(segment_lengths[i] / whole_length * div_count) + 1;
    for (int j = 0; j < local_div; j++) {
      T param = T(j) / T(local_div);
      switch (segment.type()) {
        case SegType::Line : {
          auto sample = points[0] + (points[1] - points[0]) * param;
          ret.emplace_back(std::move(sample));
          break;
        }
        case SegType::QBezier : {
          auto sample = sample_bezier<2, T>(param, segment);
          ret.emplace_back(std::move(sample));
          break;
        }
        case SegType::CBezier : {
          auto sample = sample_bezier<3, T>(param, segment);
          ret.emplace_back(std::move(sample));
          break;
        }
        default : {
          std::cerr << "unsupported segment type." << std::endl;
          break;
        }
      }
    }
  }

  return ret;
}

template <typename T>
std::vector<T> polyrize_svg(const std::string& file_path, int div_count) {
  auto path = parse_svg<T>(file_path);
  auto poly = polyrize_pathloop(path, div_count);
  std::vector<T> ret(poly.size() * 2, 0);
  for (int i = 0; i < poly.size(); i++) {
    ret[2 * i + 0] = poly[i].x;
    ret[2 * i + 1] = poly[i].y;
  }
  return ret;
}

} // namespace VL