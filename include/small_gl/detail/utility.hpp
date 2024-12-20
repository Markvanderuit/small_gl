#pragma once

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/compile.h>
#include <fmt/ranges.h>
#include <glad/glad.h>
#include <exception>
#include <iterator>
#include <span>
#include <string>
#include <vector>
#include <utility>

namespace gl::detail {
  // Convert a span over type U to 
  template <class T, class U>
  std::span<T> cast_span(std::span<U> s) {
    auto data = s.data();
    if (!data)
      return {};
    return { reinterpret_cast<T*>(data), s.size_bytes() / sizeof(T) };
  }

  // Provide a readable translation of error values returned by glGetError();
  inline
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
                     "  {:<8} : {}\n", 
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
  class Exception : public std::exception, public Message {
    mutable std::string _what;

  public:
    const char * what() const noexcept override {
      _what = fmt::format("Exception thrown\n{}", get());
      return _what.c_str();
    }
  };
} // namespace gl::detail
