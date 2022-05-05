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

  void enable_debug_callbacks();

  namespace debug {
    void begin_local_callback(const source_location &sl);
    void end_local_callback();

    struct scoped_local_callback {
      scoped_local_callback(const source_location &sl = source_location::current());
      ~scoped_local_callback();

    private:
      const std::source_location &_sl;
    };

    void insert_message(std::string_view message);

    void assign_name(std::string_view object_name, const detail::Handle<> &object);

    void begin_group(std::string_view group_name, const detail::Handle<> &object);
    void end_group();


    struct scoped_group {
      scoped_group(std::string_view group_name, const detail::Handle<> &object);
      scoped_group();
    };
  } // namespace debug

} // namespace gl