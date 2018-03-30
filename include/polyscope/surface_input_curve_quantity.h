#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/histogram.h"
#include "polyscope/gl/colormap_sets.h"

#include "geometrycentral/mesh_embedded_curve.h"

namespace polyscope {

class SurfaceInputCurveQuantity : public SurfaceQuantity {
public:
  SurfaceInputCurveQuantity(std::string name, SurfaceMesh* mesh_);
  ~SurfaceInputCurveQuantity();

  virtual void draw() override;
  virtual void drawUI() override;
  void userEdit();
  
  void fillBuffers();

  geometrycentral::MeshEmbeddedCurve getCurve();

  // === Members
  bool allowEditingFromDefaultUI = true;

protected:

  geometrycentral::MeshEmbeddedCurve curve; 

  // GL program
  gl::GLProgram* program = nullptr;
  bool bufferStale = true; 

  // UI internals
  float radiusParam = 0.001;
  Color3f curveColor;
  void userEditCallback();
};


} // namespace polyscope