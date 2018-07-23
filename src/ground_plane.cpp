#include "polyscope/ground_plane.h"

#include "polyscope/gl/gl_utils.h"
#include "polyscope/gl/materials/materials.h"
#include "polyscope/gl/shaders/ground_plane_shaders.h"
#include "polyscope/polyscope.h"

#include "imgui.h"
#include "stb_image.h"

namespace polyscope {


bool groundPlaneEnabled = true;
float groundPlaneHeightFactor = 0.25;

// Global variables and helpers for ground plane
namespace {

bool groundPlanePrepared = false;
gl::GLProgram* groundPlaneProgram = nullptr;
gl::GLTexturebuffer* mirroredSceneColorTexture = nullptr;
gl::GLFramebuffer* mirroredSceneFramebuffer = nullptr;


void prepareGroundPlane() {

  // The program that draws the ground plane
  groundPlaneProgram = new gl::GLProgram(&GROUND_PLANE_VERT_SHADER, &GROUND_PLANE_FRAG_SHADER, gl::DrawMode::Triangles);

  // Geometry of the ground plane, using triangles with vertices at infinity
  glm::vec4 cVert{0., 0., 0., 1.};
  glm::vec4 v1{1., 0., 0., 0.};
  glm::vec4 v2{0., 0., 1., 0.};
  glm::vec4 v3{-1., 0., 0., 0.};
  glm::vec4 v4{0., 0., -1., 0.};

  // clang-format off
  std::vector<glm::vec4> positions = {
    cVert, v2, v1,
    cVert, v3, v2,
    cVert, v4, v3,
    cVert, v1, v4
  };
  // clang-format on

  groundPlaneProgram->setAttribute("a_position", positions);


  { // Load the ground texture
    int w, h, comp;
    unsigned char* image = nullptr;
    image = stbi_load("concrete_seamless.jpg", &w, &h, &comp, STBI_rgb);
    if (image == nullptr) throw std::logic_error("Failed to load material image");
    groundPlaneProgram->setTexture2D("t_ground", image, w, h, false, false, true);
  }


  { // Mirored scene buffer
    using namespace gl;
    mirroredSceneColorTexture = new GLTexturebuffer(GL_RGBA, view::bufferWidth, view::bufferHeight);
    GLRenderbuffer* mirroredSceneDepthBuffer =
        new GLRenderbuffer(RenderbufferType::Depth, view::bufferWidth, view::bufferHeight);

    mirroredSceneFramebuffer = new GLFramebuffer();
    mirroredSceneFramebuffer->bindToColorTexturebuffer(mirroredSceneColorTexture);
    mirroredSceneFramebuffer->bindToDepthRenderbuffer(mirroredSceneDepthBuffer);
  
    groundPlaneProgram->setTextureFromBuffer("t_mirrorImage", mirroredSceneColorTexture);
  }


  groundPlanePrepared = true;
}
};


void drawGroundPlane() {

  if (!groundPlaneEnabled) return;

  if (!groundPlanePrepared) {
    prepareGroundPlane();
  }

  // Location for ground plane
  double bboxBottom = std::get<0>(state::boundingBox).y;
  double bboxHeight = std::get<1>(state::boundingBox).y - std::get<0>(state::boundingBox).y;
  double groundHeight = bboxBottom - groundPlaneHeightFactor * bboxHeight;

  // Implement the mirror effect
  {

    // Get the current framebuffer, so we can restore it after
    GLint drawFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

    // Render to a texture so we can sample from it on the ground
    mirroredSceneFramebuffer->resizeBuffers(view::bufferWidth, view::bufferHeight);
    mirroredSceneFramebuffer->setViewport(0, 0, view::bufferWidth, view::bufferHeight);
    mirroredSceneFramebuffer->bindForRendering();
    mirroredSceneFramebuffer->clearColor = {view::bgColor[0], view::bgColor[1], view::bgColor[2]};
    mirroredSceneFramebuffer->clear();

    // Push a reflected view matrix
    glm::mat4 origViewMat = view::viewMat;

    glm::vec3 mirrorN{0., 1., 0.};
    glm::mat3 mirrorMat3 = glm::mat3(1.0) - 2.0f * glm::outerProduct(mirrorN, mirrorN);
    glm::vec3 tVec{0., -groundHeight, 0.};
    glm::mat4 mirrorMat =
        glm::translate(glm::mat4(1.0), -tVec) * glm::mat4(mirrorMat3) * glm::translate(glm::mat4(1.0), tVec);
    view::viewMat = view::viewMat * mirrorMat;

    // Draw everything
    drawStructures();

    // Restore original view matrix
    view::viewMat = origViewMat;

    // Rebind to the previous framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
  }


  // Set uniforms
  glm::mat4 viewMat = view::getCameraViewMatrix();
  groundPlaneProgram->setUniform("u_viewMatrix", glm::value_ptr(viewMat));

  glm::mat4 projMat = view::getCameraPerspectiveMatrix();
  groundPlaneProgram->setUniform("u_projMatrix", glm::value_ptr(projMat));
  
  glm::vec2 viewportDim{view::bufferWidth, view::bufferHeight};
  groundPlaneProgram->setUniform("u_viewportDim", viewportDim);

  glm::vec2 centerXZ{state::center.x, state::center.z};
  groundPlaneProgram->setUniform("u_centerXZ", centerXZ);

  groundPlaneProgram->setUniform("u_lengthScale", state::lengthScale);
  groundPlaneProgram->setUniform("u_groundHeight", groundHeight);


  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  groundPlaneProgram->draw();


  // Disable culling afterwards, so we don't confuse ourselves
  glDisable(GL_CULL_FACE);
}

void buildGroundPlaneGui() {

  ImGui::SetNextTreeNodeOpen(false, ImGuiCond_FirstUseEver);
  if (ImGui::TreeNode("ground plane")) {

    ImGui::Checkbox("Enabled", &groundPlaneEnabled);

    ImGui::SliderFloat("Height", &groundPlaneHeightFactor, 0.0, 1.0);


    ImGui::TreePop();
  }
}

void deleteGroundPlaneResources() {

  if (!groundPlanePrepared) {
    delete mirroredSceneColorTexture;
    delete mirroredSceneFramebuffer->getDepthRenderBuffer();
    delete mirroredSceneFramebuffer;
    safeDelete(groundPlaneProgram);
  }
}
};

