#pragma once

#include <glad/glad.h>
#include <fmt/core.h>
#include <exception>
#include <iterator>
#include <map>
#include <string>

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
   * Exception class wrapper which can store a map of strings,
   * which are output line-by-line in a nicely formatted manner.
   */
  class Exception : public std::exception {
    mutable std::string _message = "gl::detail::Exception\n";
    std::map<std::string, std::string> _messages;

  public:
    std::string& operator[](const std::string &key) {
      return _messages[key];
    }

    const char * what() const noexcept override {
      for (const auto &[key, msg] : _messages) {
        if (!msg.empty()) {
          fmt::format_to(std::back_inserter(_message), "- {:<8} : {}\n", key, msg);
        }
      }
      return _message.c_str();
    }
  };

  // inline
  // void APIENTRY debug_callback(GLenum src, GLenum type, GLuint err, GLenum severity, GLsizei length,
  //                              const char *msg, const void *userParam) {
  //   // Filter out insignificant codes
  //   constexpr static auto ignored_err = { 131169u, 131185u, 131204u, 131218u };
  //   if (std::ranges::binary_search(ignored_err, err)) {

  //   }
  // }
} // namespace gl::detail
