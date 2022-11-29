#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/trace.hpp>
#include <small_gl/detail/utility.hpp>
#include <small_gl/dispatch.hpp>
#include <chrono>
#include <filesystem>
#include <source_location>

// Simple guard statement syntactic sugar
#define guard(expr,...) if (!(expr)) { return __VA_ARGS__ ; }
#define guard_continue(expr) if (!(expr)) { continue; }
#define guard_break(expr) if (!(expr)) { break; }

// Simple range-like syntactic sugar
#define range_iter(c) c.begin(), c.end()

namespace gl {  
  namespace fs = std::filesystem; // STL filesystem namespace shorthand

  namespace io {
    // Load shader binary or char data from the given filepath
    std::vector<std::byte> load_shader_binary(fs::path path);
  } // namespace io

  namespace sync {
    // Insert one or more memory barriers for shader-memory operations
    void memory_barrier(BarrierFlags flags);
    void texture_barrier();

    // Shorthands for std::chrono::duration types
    using time_ns = std::chrono::nanoseconds;
    using time_mus = std::chrono::microseconds;
    using time_mis = std::chrono::milliseconds;
    using time_s = std::chrono::seconds;

    /**
     * Fence object wrapping OpenGL buffer object.
     */
    class Fence : public detail::Handle<void *> {
      using Base = Handle<void *>;

      time_ns m_wait_time;

    public:
      /* constr/destr */

      Fence() = default;
      Fence(time_ns wait_time);
      ~Fence();

      /* wait operands */

      void cpu_wait(); // blocking time
      void gpu_wait();

      inline void swap(Fence &o) {
        gl_trace();
        using std::swap;
        Base::swap(o);
        swap(m_wait_time, o.m_wait_time);
      }

      inline constexpr bool operator==(const Fence &o) const {
        using std::tie;
        return Base::operator==(o) && m_wait_time == o.m_wait_time;
      }

      gl_declare_noncopyable(Fence);
    };
  }; // namespace sync

  namespace state {
    // Enable/disable/obtain draw capabilities; see gl::DrawCapability
    void set(DrawCapability capability, bool enabled);
    bool get(DrawCapability capability);

    // Configure framebuffer blending and logic operations
    void set_op(BlendOp src_operand, BlendOp dst_operand);
    void set_op(DrawOp  operand);
    void set_op(LogicOp operand);
    void set_op(CullOp  operand);

    // Helper object to set/unset capabilities in a local scope using RAII
    class ScopedSet {
      DrawCapability m_capability;
      bool m_prev, m_curr;
    
    public:
      ScopedSet(DrawCapability capability, bool enabled);
      ~ScopedSet();
    };
    
    // Various state components
    void set_viewport(const eig::Array2u &size, const eig::Array2u &offset = 0u);
    void set_line_width(float width);
    void set_point_size(float size);

    // OpenGL variable queries
    int get_variable_int(VariableName name);
  } // namespace state

  namespace debug {
    // Enable OpenGL's debug output message feature; requires a debug context!
    void enable_messages(DebugMessageSeverity minimum_severity = DebugMessageSeverity::eLow,
                         DebugMessageTypeFlags type_flags = DebugMessageTypeFlags::eAll);

    // Insert a message into OpenGL's debug output message stream
    void insert_message(std::string_view message, DebugMessageSeverity severity);

    // Evaluate a boolean expression, throwing a detailed exception pointing
    // to the expression's origin if said expression fails
    constexpr inline
    void check_expr_rel(bool expr,
                        const std::string_view &msg = "",
                        const std::source_location sl = std::source_location::current()) {
      guard(!expr);

      detail::Exception e;
      e.put("src", "gl::debug::check_expr_dbg(...) failed, checked expression evaluated to false");
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
    }

    // Evaluate a boolean expression, throwing a detailed exception pointing
    // to the expression's origin if said expression fails
    // Note: removed on release builds
  #if defined(NDEBUG) || defined(GL_ENABLE_DBG_EXCEPTIONS)
    constexpr inline
    void check_expr_dbg(bool expr,
                        const std::string_view &msg = "",
                        const std::source_location sl = std::source_location::current()) {
      guard(!expr);

      detail::Exception e;
      e.put("src", "gl::debug::check_expr_dbg(...) failed, checked expression evaluated to false");
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
    }
  #else
  #define check_expr_dbg(expr, msg, sl)
  #endif

    // Evaluate OpenGL's glGetError(), throwing a detailed exception pointing
    // to the function call's origin if glGetError() fails
    // Note: enabling debug output with enable_messages(...) is seriously a much better option
    inline
    void check_gl_rel(const std::string_view &msg = "",
                      const std::source_location sl = std::source_location::current()) {
      GLenum err = glGetError();
      guard(err != GL_NO_ERROR);

      detail::Exception e;
      e.put("src", "gl::debug::check_gl_dbg(...) failed, OpenGL returned an error");
      e.put("error", detail::readable_gl_error(err));
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
    }
    
    // Evaluate OpenGL's glGetError(), throwing a detailed exception pointing
    // to the function call's origin if glGetError() fails
    // Note: enabling debug output with enable_messages(...) is seriously a much better option
    // Note: removed on release builds
  #if defined(NDEBUG) || defined(GL_ENABLE_DBG_EXCEPTIONS)
    inline
    void check_gl_dbg(const std::string_view &msg = "",
                      const std::source_location sl = std::source_location::current()) {
      GLenum err = glGetError();
      guard(err != GL_NO_ERROR);

      detail::Exception e;
      e.put("src", "gl::debug::check_gl_dbg(...) failed, OpenGL returned an error");
      e.put("error", detail::readable_gl_error(err));
      e.put("message", msg);
      e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
      throw e;
    }
  #else
  #define check_gl_dbg(expr, sl)
  #endif
  } // namespace debug
} // namespace gl