// std
#include <iostream>

// FZUI
#include "fzui/data/texture.hpp"
#include "fzui/vendor/stb/stb_image.hpp"

// vendor
#include <glad/glad.h>

namespace fz {
  Texture::Texture() {
    stbi_set_flip_vertically_on_load(true);
  }

  void Texture::loadFromFile(const std::string& path) {
    std::string truePath = BASE_DIR + path;

    // Load file from path
    int width;
    int height;
    int channels;

    stbi_uc* pixels = stbi_load(truePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
      std::cout << "ERROR: Could not load texture at: " << truePath << std::endl;
    }

    // Generate texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);

    // Binds texture object as the currently bound texture object
    glBindTexture(GL_TEXTURE_2D, m_ID);

    // Texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Fill the texture with the pixel data
    // TODO: Let the pixel format be dynamic in the future
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixels);
  }

  void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, m_ID);
  }

  unsigned int Texture::getID() const {
    return m_ID;
  }
}
