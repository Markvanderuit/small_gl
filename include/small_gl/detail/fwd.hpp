#pragma once

namespace gl {
  // Shorthand unsigned types
  using uint = unsigned int;
  using uchar = unsigned char;
  using ushort = unsigned short;
  
  // OpenGL object wrappers
  struct Array;
  struct Buffer;
  struct Fence;
  struct Framebuffer;
  struct Program;
  struct Sampler;
  struct Shader;
  struct Window;
  struct Query;

  // Special texture types
  struct DepthComponent;
  struct StencilComponent;
  struct AbstractTexture;
} // namespace gl