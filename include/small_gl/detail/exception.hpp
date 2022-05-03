#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <fmt/format.h>
#include <exception>
#include <map>
#include <string>
#include <source_location>

// Simple guard statement syntactic sugar
#define guard(expr,...) if (!(expr)) { return __VA_ARGS__ ; }
#define guard_continue(expr) if (!(expr)) { continue; }
#define guard_break(expr) if (!(expr)) { break; }

namespace gl::detail {
  using source_location = std::source_location;

  /**
   * Exception class wrapper which can store a map of strings,
   * which are output line-by-line in a formatted manner.
   */
  class Exception : public std::exception {
    mutable std::string _message = "Exception\n";
    std::map<std::string, std::string> _messages;

  public:
    std::string& operator[](const std::string &key) {
      return _messages[key];
    }

    const char * what() const noexcept override {
      for (const auto &[key, msg] : _messages) {
        if (!msg.empty()) {
          _message += fmt::format("- {:<7} : {}\n", key, msg);
        }
      }
      return _message.c_str();
    }
  };

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

    Exception e;
    e["who"] = "gl::detail::expr_check(...), from the small_gl library";
    e["msg"] = msg;
    e["file"] = fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column());
    e["func"] = sl.function_name();
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

    Exception e;
    e["who"] = "gl::detail::gl_check(...), from the small_gl library";
    e["msg"] = msg;
    e["file"] = fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column());
    e["func"] = sl.function_name();
    e["gl_err"] = std::to_string(err);
    throw e;
#endif
  }
} // namespace gl::detail