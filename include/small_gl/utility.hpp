#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/utility.hpp>
#include <small_gl/dispatch.hpp>
#include <chrono>
#include <filesystem>
#include <source_location>

// Simple guard statement syntactic sugar
#define guard(expr,...) if (!(expr)) { return __VA_ARGS__ ; }
#define guard_continue(expr) if (!(expr)) { continue; }
#define guard_break(expr) if (!(expr)) { break; }

namespace gl {
  namespace io {
    // Load shader binary or char data from the given filepath
    std::vector<std::byte> load_shader_binary(std::filesystem::path path);
  } // namespace io

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
    // Enable/disable/obtain draw capabilities; see gl::DrawCapability
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

  namespace debug {
    // Enable OpenGL's debug output message feature; requires a debug context!
    void enable_messages(DebugMessageSeverity minimum_severity = DebugMessageSeverity::eLow,
                         DebugMessageTypeFlags type_flags = DebugMessageTypeFlags::eAll);

    // Insert a message into OpenGL's debug output message stream
    void insert_message(std::string_view message, DebugMessageSeverity severity);

    // Evaluate a boolean expression, throwing a detailed exception pointing
    // to the expression's origin if said expression fails
    inline
    void check_expr(bool expr,
                    const std::string_view &msg = "",
                    const std::source_location sl = std::source_location::current()) {
  #ifdef NDEBUG
  #else
      guard(!expr);

      detail::Exception e;
      e.put("src", "gl::debug::check_expr(...) failed, checked expression evaluated to false");
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
  #endif
    }

    // Evaluate OpenGL's glGetError(), throwing a detailed exception pointing
    // to the function call's origin if glGetError() fails
    // Note: enabling debug output with enable_messages(...) is seriously a much better option
    inline
    void check_gl(const std::string_view &msg = "",
                  const std::source_location sl = std::source_location::current()) {
  #ifdef NDEBUG
  #else
      GLenum err = glGetError();
      guard(err != GL_NO_ERROR);

      detail::Exception e;
      e.put("src", "gl::debug::check_gl(...) failed, OpenGL returned an error");
      e.put("error", detail::readable_gl_error(err));
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
  #endif
    }
  } // namespace debug
} // namespace gl