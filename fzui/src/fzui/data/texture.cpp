// std
#include <iostream>

// FZUI
#include "fzui/data/texture.hpp"
#include "fzui/vendor/stb/stb_image.hpp"

namespace fz {
  Texture::Texture(const std::string& path) {
    // Load file from path
    int width;
    int height;
    int channels;

    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
      std::cout << "ERROR: Could not load texture at: " << path << std::endl;
    }

    stbi_image_free(pixels);
  }
}
