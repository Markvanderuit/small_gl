#pragma once

#include <small_gl/detail/fwd.hpp>
#include <fmt/format.h>
#include <exception>
#include <map>
#include <string>
#include <source_location>

namespace gl::detail {
  using source_location = std::source_location;

  /**
   * Exception class wrapper which can store a map of strings,
   * which are output line-by-line in a formatted manner.
   */
  class Exception : public std::exception {
    mutable std::string _what;
    std::map<std::string, std::string> _attached;

  public:
    std::string& operator[](const std::string &key) {
      return _attached[key];
    }

    const char * what() const noexcept override {
      constexpr std::string_view fmt = "- {:<7} : {}\n";
      std::string s = "Exception\n";

      for (const auto &[key, msg] : _attached) {
        if (!msg.empty()) {
          s += fmt::format(fmt, key, msg);
        }
      }

      return (_what = s).c_str();
    }
  };

  // Check whether an expression evaluates to true, and otherwise throw
  // a detailed exception, pointing to the expression's origin, with an
  // optional message attached.
  inline
  void expr_check(bool expr,
                  const std::string_view &msg = "",
                  const source_location sl = source_location::current()) {
    guard(!expr);

    Exception e;

    e["who"] = "gl::detail::expr_check(...), from the small_gl library";
    e["msg"] = msg;
    e["file"] = fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column());
    e["func"] = sl.function_name();

    throw e;
  }

  // Check whether OpenGL's glGetError() passes, and otherwise throw
  // a detailed exception, pointing to the expression's origin, with
  // an optional message attached.
  inline
  void gl_check(const std::string_view &msg = "",
                const source_location sl = source_location::current()) {
    GLenum err = glGetError();
    guard(err != GL_NO_ERROR);

    Exception e;

    e["who"] = "gl::detail::gl_check(...), from the small_gl library";
    e["msg"] = msg;
    e["file"] = fmt::format("{}({}:{})", sl.file_name(), sl.line(), sl.column());
    e["func"] = sl.function_name();
    e["gl_err"] = std::to_string(err);

    throw e;
  }
} // namespace gl::detail