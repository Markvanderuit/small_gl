#pragma once

#include <small_gl/detail/math.hpp>
#include <small_gl/detail/enum.hpp>

namespace gl {
  // Non-templated OpenGL object wrappers
  class Buffer;
  class Fence;
  class Framebuffer;
  class Program;
  class Sampler;
  class Shader;
  class Vertexarray;
  class Window;
  class Query;

  // Templated OpenGL object wrappers
  template <typename T, 
            uint D,
            uint Components,
            TextureType Ty = TextureType::eBase>
  class Texture;

} // namespace gl