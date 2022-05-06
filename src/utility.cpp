#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/utility.hpp>
#include <fmt/format.h>
#include <fmt/core.h>
#include <array>
#include <fstream>
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

      // Guard against non-errorneous message types
      guard(std::ranges::binary_search(guard_types, type));

      // Unrecoverable (or unwanted) message; throw exception indicating this
      detail::Exception e;
      e.put("src", "gl::detail::debug_callback(...)");
      e.put("message", "OpenGL debug message indicates a potential error");
      throw e;
    }
  } // namespace detail

  namespace io {
    std::vector<std::byte> load_shader_binary(std::filesystem::path path) {
      // Check that file path exists
      debug::check_expr(std::filesystem::exists(path),
        fmt::format("failed to resolve path \"{}\"", path.string()));

      // Attempt to open file stream
      std::ifstream ifs(path, std::ios::ate | std::ios::binary);
      debug::check_expr(ifs.is_open(),
        fmt::format("failed to open file \"{}\"", path.string()));

      // Read file size and construct vector to hold data
      size_t file_size = static_cast<size_t>(ifs.tellg());
      std::vector<std::byte> buffer(file_size);
      
      // Set inpout position to start, then read full file into vector
      ifs.seekg(0);
      ifs.read((char *) buffer.data(), file_size);
      ifs.close();
      
      return buffer;
    }
  } // namespace io

  namespace sync {
    void set_barrier(BarrierFlags flags) {
      glMemoryBarrier((uint) flags);
    }

    Fence::Fence() : Base(true) {
      _object = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    Fence::~Fence() {
      guard(_is_init);
      glDeleteSync((GLsync) _object);
    }

    void Fence::cpu_wait(time_ns max_time) {
      glClientWaitSync((GLsync) _object, 0, max_time.count());
    }

    void Fence::gpu_wait() {
      glWaitSync((GLsync) _object, 0, GL_TIMEOUT_IGNORED);
    }
  } // namespace sync

  namespace state {
    void set(DrawCapability capability, bool enabled) {
      if (enabled) {
        glEnable((uint) capability);
      } else {
        glDisable((uint) capability);
      }
    }

    bool get(DrawCapability capability) {
      return glIsEnabled((uint) capability);
    }

    void set_op(BlendOp src_operand, BlendOp dst_operand) {
      glBlendFunc((uint) src_operand, (uint) dst_operand);
    }

    void set_op(LogicOp operand) {
      glLogicOp((uint) operand);
    }

    void set_viewport(Array2i size, Array2i offset) {
      glViewport(offset[0], offset[1], size[0], size[1]);
    }

    ScopedSet::ScopedSet(DrawCapability capability, bool enabled)
    : _capability(capability), _prev(get(capability)), _curr(enabled) {
      guard(_curr != _prev);
      set(_capability, _curr);
    }

    ScopedSet::~ScopedSet() {
      guard(_curr != _prev);
      set(_capability, _prev);
    }
  } // namespace state

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
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_OTHER,
                           0,
                           severity_types[(uint) severity],
                           message.size(),
                           message.data());
    }
  } // namespace debug
} // namespace gl