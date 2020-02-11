// Copyright 2017-2020, Christopher Yu, Nicholas Sharp and the
// Polyscope contributors. http://polyscope.run.

#include "polyscope/implicit_surface.h"

#include "polyscope/combining_hash_functions.h"
#include "polyscope/gl/gl_utils.h"
#include "polyscope/gl/materials/materials.h"
#include "polyscope/gl/shaders.h"
#include "polyscope/gl/shaders/surface_shaders.h"
#include "polyscope/gl/shaders/wireframe_shaders.h"
#include "polyscope/pick.h"
#include "polyscope/polyscope.h"

#include "imgui.h"

namespace polyscope {
// Initialize statics
const std::string ImplicitSurface::structureTypeName = "Implicit Surface";

ImplicitSurface::ImplicitSurface(std::string name, double* f, size_t nValuesPerSide, glm::vec3 center, double sideLen)
    : QuantityStructure<ImplicitSurface>(name, typeName()), color(uniquePrefix() + "#color", getNextUniqueColor()) {
  nCornersPerSide = nValuesPerSide;
  sideLength = sideLen;
  gridCenter = center;
  field = new double[nCells()];

  int nPerSlice = nCornersPerSide * nCornersPerSide;

  for (size_t x = 0; x < nCornersPerSide; x++) {
    for (size_t y = 0; y < nCornersPerSide; y++) {
      for (size_t z = 0; z < nCornersPerSide; z++) {
        int index = nPerSlice * z + nCornersPerSide * y + x;
        field[index] = f[index];
      }
    }
  }
}

ImplicitSurface* ImplicitSurface::setColor(glm::vec3 newVal) {
  color = newVal;
  polyscope::requestRedraw();
  return this;
}

glm::vec3 ImplicitSurface::getColor() { return color.get(); }

void ImplicitSurface::buildCustomUI() {
  ImGui::Text("samples: %lld  (%lld per side)", static_cast<long long int>(nCells()),
              static_cast<long long int>(nCornersPerSide));
  if (ImGui::ColorEdit3("Color", &color.get()[0], ImGuiColorEditFlags_NoInputs)) {
    setColor(getColor());
  }
  ImGui::SameLine();
}

void ImplicitSurface::buildPickUI(size_t localPickID) {
  // For now do nothing
}

void ImplicitSurface::draw() {
  // For now do nothing
}

void ImplicitSurface::drawPick() {
  // For now do nothing
}

double ImplicitSurface::lengthScale() { return sideLength; }

std::tuple<glm::vec3, glm::vec3> ImplicitSurface::boundingBox() {
    double r = sideLength / 2;
    glm::vec3 minCorner = gridCenter - glm::vec3{r, r, r};
    glm::vec3 maxCorner = gridCenter + glm::vec3{r, r, r};

    return std::make_tuple(minCorner, maxCorner);
}

std::string ImplicitSurface::typeName() { return structureTypeName; }

} // namespace polyscope