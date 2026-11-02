#include "frame_buffer.h"
#include "SceneLoader.h"
#include "SceneParser_JSON.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
  std::vector<std::string> arguments;
  bool useBvh = false;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--bvh") useBvh = true;
    else if (argument == "--no-bvh") useBvh = false;
    else if (argument == "--help" || argument == "-h") {
      std::cout << "Usage: render_json <scene.json> [output.png] [width] [height] [--bvh|--no-bvh]\n";
      return 0;
    } else arguments.push_back(argument);
  }

  if (arguments.empty() || arguments.size() > 4) {
    std::cerr << "Usage: render_json <scene.json> [output.png] [width] [height] [--bvh|--no-bvh]\n";
    return 2;
  }

  try {
    const std::string inputPath = arguments[0];
    const std::string outputPath = arguments.size() > 1 ? arguments[1] : "render.png";
    const int width = arguments.size() > 2 ? std::stoi(arguments[2]) : 800;
    const int height = arguments.size() > 3 ? std::stoi(arguments[3]) : 600;
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

    scene.shapes.use_bvh(useBvh);

    Framebuffer framebuffer(width, height);
    const auto start = std::chrono::steady_clock::now();
    framebuffer.render(*scene.camera, scene.shapes, scene.lights, scene.background);
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    framebuffer.exportAsPNG(outputPath);
    std::cout << "Rendered " << inputPath << " to " << outputPath << " (" << width << "x" << height << ") "
              << (useBvh ? "with" : "without") << " BVH in " << elapsed << " seconds.\n";
  } catch (const std::exception &error) {
    std::cerr << "Render failed: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
