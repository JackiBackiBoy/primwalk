#pragma once

#define STB_IMAGE_IMPLEMENTATION

// std
#include <string>
#include <memory>

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glad/glad.h>

namespace fz {
  class FZ_API Texture {
    public:
      Texture();
      ~Texture() {};

      void loadFromFile(const std::string& path);
      void create(const int& width, const int& height, unsigned char* pixels, int internalFormat = GL_RGBA, int format = GL_RGBA);
      void update(const int& width, const int& height, unsigned char* pixels, int internalFormat = GL_RGBA, int format = GL_RGBA);
      void bind() const;

      // Getters
      unsigned int getID() const;
      unsigned int getWidth() const;
      unsigned int getHeight() const;
      float getAspectRatio() const;

      static Texture* create(const std::string& path);

    private:
      unsigned int m_ID = 0;
      unsigned int m_Width = 0;
      unsigned int m_Height = 0;
  };
}
