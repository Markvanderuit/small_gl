#include <small_gl/exception.hpp>
#include <ranges>

namespace gl {
  namespace detail {
    inline
    void APIENTRY debug_callback(GLenum src, GLenum type, GLuint err, GLenum severity, GLsizei length,
                                const char *msg, const void *userParam) {
      // Filter out insignificant codes
      constexpr static auto ignored_err = { 131169u, 131185u, 131204u, 131218u };
      guard(!std::ranges::binary_search(ignored_err, err));

      detail::Exception e;
      e["reason"] = "gl::detail::debug_callback(...) triggered, OpenGL sent a debug message";
      // TODO implement format
      throw e;
    }
  } // namespace detail
  
  void enable_debug_callbacks() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(detail::debug_callback, nullptr);

    // Enable all SEVERITY_HIGH messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    /* glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_MARKER, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE); */

    // Enable select SEVERITY_MEDIUM messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);

    // Disable others
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
  }
} // namespace gl