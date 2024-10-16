#include "VectorLoop.hpp"

int main() {
  auto pathloop = VL::parse_svg<float>("sample_assets/walking_cat.svg");
  auto polyloop = VL::polyrize_pathloop(pathloop, 1000);

  auto svgloop = VL::polyrize_svg<float>("sample_assets/sphere.svg", 8);
}