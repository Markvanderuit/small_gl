#pragma once

#include <glad/glad.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/compile.h>
#include <exception>
#include <iterator>
#include <string>
#include <vector>
#include <utility>

namespace gl::detail {
  // Provide a readable translation of error values returned by glGetError();
  constexpr inline
  std::string readable_gl_error(GLenum err) {
    switch (err) {
      case GL_INVALID_ENUM:                   return "GL_INVALID_ENUM";
      case GL_INVALID_VALUE:                  return "GL_INVALID_VALUE";
      case GL_INVALID_OPERATION:              return "GL_INVALID_OPERATION";
      case GL_STACK_OVERFLOW:                 return "GL_STACK_OVERFLOW";
      case GL_STACK_UNDERFLOW:                return "GL_STACK_UNDERFLOW";
      case GL_OUT_OF_MEMORY:                  return "GL_OUT_OF_MEMORY";
      case GL_INVALID_FRAMEBUFFER_OPERATION:  return "GL_INVALID_FRAMEBUFFER_OPERATION";
      case GL_CONTEXT_LOST:                   return "GL_CONTEXT_LOST";
      case GL_NO_ERROR:                       return "GL_NO_ERROR";
      default:                                return "readable_gl_error(...) failed to map code";
    }
  }

  /**
   * Message class which stores a keyed list of strings, which
   * are output line-by-line in a formatted manner, in the order
   * in which they were provided.
   */
  class Message {
    std::vector<std::pair<std::string, std::string>> _messages;
    std::string _buffer;

  public:
    void put(std::string_view key, std::string_view message) {
      fmt::format_to(std::back_inserter(_buffer),
                     FMT_COMPILE("  {:<8} : {}\n"), 
                     key, 
                     message);
    }

    std::string get() const {
      return _buffer;
    }
  };

  /**
   * Exception class which stores a keyed list of strings, which
   * are output line-by-line in a formatted manner, in the order
   * in which they were provided..
   */
  class Exception : public std::exception, 
                             public Message {
    mutable std::string _what;

  public:
    const char * what() const noexcept override {
      _what = fmt::format("gl::detail::Exception thrown\n{}", get());
      return _what.c_str();
    }
  };
} // namespace gl::detail
