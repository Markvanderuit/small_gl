#pragma once

#include <small_gl/detail/exception.hpp>
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
    e["reason"] = "gl::expr_check(...) failed, the checked expression evaluated to false";
    e["message"] = msg;
    e["file"] = fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column());
    e["function"] = fmt::format("{}(...);", sl.function_name());
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
    e["reason"] = "gl::gl_check(...) failed, OpenGL returned an error code";
    e["message"] = msg;
    e["file"] = fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column());
    e["function"] = fmt::format("{}(...);", sl.function_name());
    e["code"] = detail::readable_gl_error(err);
    throw e;
#endif
  }
} // namespace gl