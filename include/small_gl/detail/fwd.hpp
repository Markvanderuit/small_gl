#pragma once

#include <small_gl/detail/math.hpp>
#include <small_gl/detail/enum.hpp>

namespace gl {
  // OpenGL object wrappers
  class Buffer;
  class Fence;
  class Framebuffer;
  class Program;
  class Sampler;
  class Shader;
  class Vertexarray;
  class Window;
  class Query;

  // OpenGL texture object wrappers
  class AbstractTexture;
  template <typename T, 
            uint D,
            uint Components,
            TextureType Ty = TextureType::eImage>
  class Texture;

  // Special texture types
  struct DepthComponent;
  struct StencilComponent;

} // namespace gl