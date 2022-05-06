#include <small_gl/exception.hpp>
#include <fmt/core.h>
#include <fmt/color.h>
#include <array>
#include <ranges>

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
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined behavior";
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
    void APIENTRY debug_callback(GLenum src, GLenum type, GLuint code, GLenum severity, GLsizei length,
                                const char *msg, const void *user_param) {
      constexpr static std::initializer_list<uint> guard_codes = { 
        131169, 131185, 131204, 131218 };
      constexpr static std::initializer_list<uint> guard_types = { 
        GL_DEBUG_TYPE_ERROR, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR };
                                                                   
      // Output formatted message to stdout for now
      detail::Message message;
      message.put("info", fmt::format("type = {}, severity = {}, src = {}",
                          readable_debug_type(type),
                          readable_debug_severity(severity),
                          readable_debug_src(src)));
      message.put("message", msg);
      fmt::print(stdout, "OpenGL debug message\n{}", message.get());

      // Guard against insigificant codes and non-errorneous message types
      guard(!std::ranges::binary_search(guard_codes, code));
      guard(std::ranges::binary_search(guard_types, type));

      // Unrecoverable (or unwanted) message; throw exception indicating this
      detail::Exception e;
      e.put("src", "gl::detail::debug_callback(...)");
      e.put("message", "OpenGL debug message indicates a potential error");
      throw e;
    }
  } // namespace detail
  
  /* void enable_debug_callbacks(DebugMessageSeverity minimum_severity, DebugMessageTypeFlags type_flags) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(detail::debug_callback, nullptr);

    constexpr static std::array<uint, 4> severity_types = { 
      GL_DEBUG_SEVERITY_NOTIFICATION, GL_DEBUG_SEVERITY_LOW, 
      GL_DEBUG_SEVERITY_MEDIUM, GL_DEBUG_SEVERITY_HIGH
    };

    // Disable messages below minimum severity 
    for (uint i = 0; i < (uint) minimum_severity; i++) {
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, severity_types[i], 0, nullptr, GL_FALSE);
    }

    // Enable flagged messages for and above minimum severity
    for (uint i = (uint) minimum_severity; i <= (uint) DebugMessageSeverity::eHigh; i++) {
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, 
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::eError));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::eDeprecated));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::eUndefinedBehavior));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PORTABILITY,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::ePortability));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::ePerformance));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_MARKER,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::eMarker));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::ePushGroup));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::ePopGroup));
      glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER,
        severity_types[i], 0, nullptr, 
        has_flag(type_flags, DebugMessageTypeFlags::eOther));
    }
  } */

  namespace debug {
    void enable_messages(DebugMessageSeverity minimum_severity, DebugMessageTypeFlags type_flags) {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(detail::debug_callback, nullptr);

      constexpr static std::array<uint, 4> severity_types = { 
        GL_DEBUG_SEVERITY_NOTIFICATION, GL_DEBUG_SEVERITY_LOW, 
        GL_DEBUG_SEVERITY_MEDIUM, GL_DEBUG_SEVERITY_HIGH
      };

      // Disable messages below minimum severity 
      for (uint i = 0; i < (uint) minimum_severity; i++) {
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, severity_types[i], 0, nullptr, GL_FALSE);
      }

      // Enable flagged messages for and above minimum severity
      for (uint i = (uint) minimum_severity; i <= (uint) DebugMessageSeverity::eHigh; i++) {
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, 
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::eError));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::eDeprecated));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::eUndefinedBehavior));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PORTABILITY,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::ePortability));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::ePerformance));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_MARKER,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::eMarker));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::ePushGroup));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::ePopGroup));
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER,
          severity_types[i], 0, nullptr, 
          has_flag(type_flags, DebugMessageTypeFlags::eOther));
      }
    }

    void insert_message(std::string_view message, DebugMessageSeverity severity) {
      constexpr static std::array<uint, 4> severity_types = { 
        GL_DEBUG_SEVERITY_NOTIFICATION, GL_DEBUG_SEVERITY_LOW, 
        GL_DEBUG_SEVERITY_MEDIUM, GL_DEBUG_SEVERITY_HIGH
      };
      glDebugMessageInsert(GL_DEBUG_SOURCE_THIRD_PARTY,
                           GL_DEBUG_TYPE_OTHER,
                           0,
                           severity_types[(uint) severity],
                           message.size(),
                           message.data());
    }

    void begin_message_group(std::string_view group_name) {
      glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY,
                       0, 
                       group_name.length(), 
                       group_name.data());
    }

    void end_message_group() {
      glPopDebugGroup();
    }
  } // namespace debug
} // namespace gl