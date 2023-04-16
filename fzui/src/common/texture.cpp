// std
#include <iostream>

// FZUI
#include "fzui/data/texture.hpp"
#include "fzui/vendor/stb/stb_image.hpp"

namespace fz {
  Texture::Texture() {
    stbi_set_flip_vertically_on_load(true);
  }

  void Texture::loadFromFile(const std::string& path) {
    std::string truePath = BASE_DIR + path;

    // Load file from path
    int width = 0;
    int height = 0;
    int channels = 0;

    

    stbi_uc* pixels = stbi_load(truePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
      std::cout << "ERROR: Could not load texture at: " << truePath << std::endl;
    }

    std::cout << width << ", " << height << std::endl;

    create(width, height, pixels);
    stbi_image_free(pixels);
  }

  void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, m_ID);
  }

  // Getters
  unsigned int Texture::getID() const {
    return m_ID;
  }

  unsigned int Texture::getWidth() const {
    return m_Width;
  }

  unsigned int Texture::getHeight() const {
    return m_Height;
  }

  float Texture::getAspectRatio() const {
    return float(m_Width) / m_Height;
  }

  void Texture::create(const int& width, const int& height, unsigned char* pixels, int internalFormat, int format) {
    m_Width = width;
    m_Height = height;

    // Generate texture
    //glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
    glGenTextures(1, &m_ID);

    // Binds texture object as the currently bound texture object
    glBindTexture(GL_TEXTURE_2D, m_ID);

    // Texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Fill the texture with the pixel data
    // TODO: Let the pixel format be dynamic in the future
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  void Texture::update(const int& width, const int& height, unsigned char* pixels, int internalFormat, int format) {
    m_Width = width;
    m_Height = height;

    // Fill the texture with the pixel data
    // TODO: Let the pixel format be dynamic in the future
    glBindTexture(GL_TEXTURE_2D, m_ID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  Texture* Texture::create(const std::string& path) {
    Texture* texture = new Texture();
    texture->loadFromFile(path);

    return texture;
  }
}
