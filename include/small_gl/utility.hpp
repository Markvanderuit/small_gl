#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/dispatch.hpp>
#include <filesystem>

namespace gl {
  namespace fs = std::filesystem;
  
  // Load shader binary or char data from the given filepath
  std::vector<std::byte> load_shader_binary(fs::path path);

  // Construct an indirect buffer object from a gl::DrawInfo/ComputeInfo object
  gl::Buffer to_indirect(DrawInfo info, BufferStorageFlags flags = { });
  gl::Buffer to_indirect(ComputeInfo info, BufferStorageFlags flags = { });
  
  namespace state {
    // Enable/disable/read draw capabilities; see gl::DrawCapability
    void set(DrawCapability capability, bool enabled);
    bool get(DrawCapability capability);

    // Configure framebuffer blending and logic operations
    void set_op(BlendOp src_operand, BlendOp dst_operand);
    void set_op(LogicOp operand);
    
    // Configure framebuffer viewport transformation
    void set_viewport(Array2i size, Array2i offset = Array2i::Zero());

    // Helper object to set/unset capabilities in a local scope using RAII
    class ScopedSet {
      DrawCapability _capability;
      bool _prev, _curr;
      
    public:
      ScopedSet(DrawCapability capability, bool enabled);
      ~ScopedSet();
    };
  } // namespace state
} // namespace gl