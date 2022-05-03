#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/dispatch.hpp>
#include <chrono>
#include <filesystem>

namespace gl {
  // Load shader binary or char data from the given filepath
  std::vector<std::byte> load_shader_binary(std::filesystem::path path);

  // Construct an indirect buffer object from a gl::DrawInfo/ComputeInfo object
  gl::Buffer to_indirect(DrawInfo info, BufferStorageFlags flags = { });
  gl::Buffer to_indirect(ComputeInfo info, BufferStorageFlags flags = { });
  
  namespace sync {
    // Insert one or more memory barriers for shader-memory operations
    void set_barrier(BarrierFlags flags);

    // Shorthands for std::chrono::duration types
    using time_ns = std::chrono::nanoseconds;
    using time_mus = std::chrono::microseconds;
    using time_mis = std::chrono::milliseconds;
    using time_s = std::chrono::seconds;

    /**
     * Fence object wrapping OpenGL buffer object.
     */
    struct Fence : public detail::Handle<void *> {
      /* constr/destr */

      Fence();
      ~Fence();

      /* wait operands */

      void cpu_wait(time_ns max_time = time_s(1)); // blocking
      void gpu_wait();

    private:
      using Base = Handle<void *>;
    };
  }; // namespace sync

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
    struct ScopedSet {
      ScopedSet(DrawCapability capability, bool enabled);
      ~ScopedSet();
    
    private:
      DrawCapability _capability;
      bool _prev, _curr;
    };
  } // namespace state
} // namespace gl