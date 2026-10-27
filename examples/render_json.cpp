#include "frame_buffer.h"
#include "SceneLoader.h"
#include "SceneParser_JSON.h"

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv)
{
  if (argc < 2 || argc > 5) {
    std::cerr << "Usage: render_json <scene.json> [output.png] [width] [height]\n";
    return 2;
  }

  try {
    const std::string inputPath = argv[1];
    const std::string outputPath = argc > 2 ? argv[2] : "render.png";
    const int width = argc > 3 ? std::stoi(argv[3]) : 800;
    const int height = argc > 4 ? std::stoi(argv[4]) : 600;
    if (width <= 0 || height <= 0) {
      throw std::runtime_error("Image width and height must be positive.");
    }

    RenderScene scene;
    auto loader = std::make_shared<SceneLoader>(scene, width, height);
    SceneParser_JSON parser(loader);
    parser.parseFileData(inputPath);

    if (!scene.camera) {
      throw std::runtime_error("Scene JSON must contain a perspective camera.");
    }

    Framebuffer framebuffer(width, height);
    framebuffer.render(*scene.camera, scene.shapes, scene.lights, scene.background);
    framebuffer.exportAsPNG(outputPath);
    std::cout << "Rendered " << inputPath << " to " << outputPath << " (" << width << "x" << height << ").\n";
  } catch (const std::exception &error) {
    std::cerr << "Render failed: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
