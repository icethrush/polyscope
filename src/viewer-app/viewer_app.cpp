#include "polyscope/polyscope.h"

#include "geometrycentral/halfedge_mesh.h"

using namespace geometrycentral;

int main(int argc, char** argv) {
    

    // Initialize polyscope
    polyscope::init();

    // Create a point cloud
    std::vector<Vector3> points;
    for(size_t i = 0; i < 300000; i++) {
    // for(size_t i = 0; i < 1; i++) {
        points.push_back(Vector3{10,10,10} + 1000.01*Vector3{unitRand()-.5, unitRand()-.5, unitRand()-.5});
    }

    // Load the point cloud in to polyscope
    polyscope::registerPointCloud("really great points", points);


    // Read a mesh

    // Load the mesh in to polyscope

    // Add a few gui elements

    // Show the gui
    polyscope::show();

    return 0;

}