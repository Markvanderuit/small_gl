#include <small_gl/exception.hpp>
#include <ranges>
#include <fmt/core.h>

namespace gl {
  namespace detail {
    inline constexpr
    std::string readable_debug_src(GLenum src) {
      switch (src) {
        case GL_DEBUG_SOURCE_API:             return "api";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "window system";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:     return "third party";
        case GL_DEBUG_SOURCE_APPLICATION:     return "application"; 
        case GL_DEBUG_SOURCE_OTHER:           return "other";      
        default:                              return "gl::detail::readable_debug_src(...) failed to map sec";
      }
    }

    inline constexpr
    std::string readable_debug_type(GLenum type) {
      switch (type) {
        case GL_DEBUG_TYPE_ERROR:               return "error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "unefined behavior";
        case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
        case GL_DEBUG_TYPE_MARKER:              return "marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:          return "push group";
        case GL_DEBUG_TYPE_POP_GROUP:           return "pop group";
        case GL_DEBUG_TYPE_OTHER:               return "other";
        default:                                return "gl::detail::readable_debug_type(...) failed to map type";
      }
    }

    inline constexpr
    std::string readable_debug_severity(GLenum severity) {
      switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:          return "high";
        case GL_DEBUG_SEVERITY_MEDIUM:        return "medium";
        case GL_DEBUG_SEVERITY_LOW:           return "low";
        case GL_DEBUG_SEVERITY_NOTIFICATION:  return "notification";
        default:                              return "gl::detail::readable_debug_severity(...) failed to map severity";
      }
    }
    
    inline
    void APIENTRY debug_callback(GLenum src, GLenum type, GLuint err, GLenum severity, GLsizei length,
                                const char *msg, const void *user_param) {
      // Filter out insignificant oft-encountered codes
      constexpr static auto ignored_err = { 131169u, 131185u, 131204u, 131218u };
      guard(!std::ranges::binary_search(ignored_err, err));

      detail::Exception e;
      e["reason"] = "gl::detail::debug_callback(...) triggered, OpenGL sent a debug message";
      e["source"] = readable_debug_src(src);
      e["type"] = readable_debug_type(type);
      e["severity"] = readable_debug_severity(severity);
      e["msg"] = std::string_view(msg);

      if (user_param) {
        source_location &sl = *(source_location *) user_param;
        e["file"] = fmt::format("{}({}:{})", 
                                sl.file_name(), 
                                sl.line(), 
                                sl.column());
        e["function"] = fmt::format("gl::scoped_local_callback in {}(...);", 
                                    sl.function_name());
      }

      throw e;
    }
  } // namespace detail
  
  void enable_debug_callbacks() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(detail::debug_callback, nullptr);

    // Enable all messages
    // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    // Enable all SEVERITY_HIGH messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

    // Enable select SEVERITY_MEDIUM messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);

    // Disable others
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
  }

  namespace debug {
    void begin_local_callback(const source_location &sl) {
      glDebugMessageCallback(detail::debug_callback, &sl);
    }

    void end_local_callback() {
      glDebugMessageCallback(detail::debug_callback, nullptr);
    }
    
    scoped_local_callback::scoped_local_callback(const source_location &sl)
    : _sl(sl) {
      begin_local_callback(_sl);
    }

    scoped_local_callback::~scoped_local_callback() {
      end_local_callback();
    }

    void insert_message(std::string_view message) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_THIRD_PARTY,
                           GL_DEBUG_TYPE_ERROR,
                           0,
                           GL_DEBUG_SEVERITY_HIGH,
                           message.size(),
                           message.data());
    }

    void begin_group(std::string_view group_name, const detail::Handle<> &object) {
      glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY,
                       object.object(), group_name.length(), group_name.data());
    }

    void end_group() {
      glPopDebugGroup();
    }
  } // namespace debug
} // namespace gl