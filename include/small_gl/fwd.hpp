#pragma once

namespace gl {
  // Shorthand unsigned types
  using uint = unsigned int;
  using ushort = unsigned short;
  
  // OpenGL object wrappers
  struct Array;
  struct Buffer;
  struct Fence;
  struct Framebuffer;
  struct Program;
  struct ProgramCache;
  struct Sampler;
  struct Shader;
  struct Window;
  struct Query;

  // Templated OpenGL object wrappers
  struct AbstractFramebufferAttachment;
  struct AbstractRenderbuffer;
  struct AbstractTexture;
} // namespace gl