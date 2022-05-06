#pragma once

#include <small_gl/detail/exception.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/utility.hpp>
#include <source_location>

// Simple guard statement syntactic sugar
#define guard(expr,...) if (!(expr)) { return __VA_ARGS__ ; }
#define guard_continue(expr) if (!(expr)) { continue; }
#define guard_break(expr) if (!(expr)) { break; }

namespace gl {
  using source_location = std::source_location;

  // Check whether an expression evaluates to true, and otherwise throw
  // a detailed exception, pointing to the expression's origin, with an
  // optional message attached.
  inline
  void expr_check(bool expr,
                  const std::string_view &msg = "",
                  const source_location sl = source_location::current()) {
#ifdef NDEBUG
#else
    guard(!expr);

    detail::Exception e;
    e.put("src", "gl::expr_check(...) failed, checked expression evaluated to false");
    e.put("message", msg);
    e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
    throw e;
#endif
  }

  // Check whether OpenGL's glGetError() passes, and otherwise throw
  // a detailed exception, pointing to the expression's origin, with
  // an optional message attached.
  inline
  void gl_check(const std::string_view &msg = "",
                const source_location sl = source_location::current()) {
#ifdef NDEBUG
#else
    GLenum err = glGetError();
    guard(err != GL_NO_ERROR);

    detail::Exception e;
    e.put("src", "gl::gl_check(...) failed, OpenGL returned an error");
    e.put("error", detail::readable_gl_error(err));
    e.put("message", msg);
    e.put("in file", fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column()));
    throw e;
#endif
  }

  namespace debug {
    void enable_messages(DebugMessageSeverity minimum_severity = DebugMessageSeverity::eLow,
                         DebugMessageTypeFlags type_flags = DebugMessageTypeFlags::eAll);

    void insert_message(std::string_view message, DebugMessageSeverity severity);
    void begin_message_group(std::string_view group_name);
    void end_message_group();

    struct scoped_group {
      scoped_group(std::string_view group_name);
      scoped_group();
    };
  } // namespace debug

} // namespace gl