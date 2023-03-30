#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/utility.hpp>
#include <nlohmann/json.hpp>
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
      // Guard against outputting unnecessary messages that can't be filtered with severity
      constexpr static std::initializer_list<uint> guard_codes = {
        131169, /* nvidia: The driver allocated multisample storage for renderbuffer X */
      };
      guard(!std::ranges::binary_search(guard_codes, code));
          
      // Output formatted message to stdout for now
      detail::Message message;
      message.put("info", fmt::format("type = {}, severity = {}, src = {}, code = {}",
                          readable_debug_type(type),
                          readable_debug_severity(severity),
                          readable_debug_src(src),
                          code));
      message.put("message", msg);
      fmt::print(stdout, "OpenGL debug message\n{}", message.get());

      // Guard against throwing on non-errorneous message types
      constexpr static std::initializer_list<uint> guard_types = { 
        GL_DEBUG_TYPE_ERROR, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR };
      guard(std::ranges::binary_search(guard_types, type));

      // Unrecoverable (or unwanted) message; throw exception indicating this
      detail::Exception e;
      e.put("src", "gl::detail::debug_callback(...)");
      e.put("message", "OpenGL debug message indicates a potential error");
      throw e;
    }
  } // namespace detail

  namespace io {
    std::vector<std::byte> load_shader_binary(const fs::path &path) {
      gl_trace();

      // Check that file path exists
      debug::check_expr(fs::exists(path),
        fmt::format("failed to resolve path \"{}\"", path.string()));

      // Attempt to open file stream
      std::ifstream ifs(path, std::ios::ate | std::ios::binary);
      debug::check_expr(ifs.is_open(),
        fmt::format("failed to open file \"{}\"", path.string()));

      // Read file size and construct vector to hold data
      size_t file_size = static_cast<size_t>(ifs.tellg());
      std::vector<std::byte> buffer(file_size);
      
      // Set input position to start, then read full file into vector
      ifs.seekg(0);
      ifs.read((char *) buffer.data(), file_size);
      ifs.close();
      
      return buffer;
    }
    
    std::string load_string(const fs::path &path) {
      gl_trace();

      // Check that file path exists
      debug::check_expr(fs::exists(path),
        fmt::format("failed to resolve path \"{}\"", path.string()));
        
      // Attempt to open file stream
      std::ifstream ifs(path, std::ios::ate);
      debug::check_expr(ifs.is_open(),
        fmt::format("failed to open file \"{}\"", path.string()));
        
      // Read file size and construct string to hold data
      size_t file_size = static_cast<size_t>(ifs.tellg());
      std::string str(file_size, ' ');

      // Set input position to start, then read full file into buffer
      ifs.seekg(0);
      ifs.read((char *) str.data(), file_size);
      ifs.close();

      return str;
    }

    json load_json(const fs::path &path) {
      return json::parse(load_string(path));
    }
  } // namespace io

  namespace sync {
    void memory_barrier(BarrierFlags flags) {
      gl_trace_full();
      glMemoryBarrier((uint) flags);
    }

    void texture_barrier() {
      gl_trace_full();
      glTextureBarrier();
    }

    Fence::Fence(time_ns wait_time)
    : Base(true),
      m_wait_time(wait_time) {
      gl_trace_full();
      m_object = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    Fence::~Fence() {
      gl_trace_full();
      guard(m_is_init);
      glDeleteSync((GLsync) m_object);
    }

    void Fence::cpu_wait() {
      gl_trace_full();
      glClientWaitSync((GLsync) m_object, GL_SYNC_FLUSH_COMMANDS_BIT, m_wait_time.count());
    }

    void Fence::gpu_wait() {
      gl_trace_full();
      glWaitSync((GLsync) m_object, 0, GL_TIMEOUT_IGNORED);
    }
  } // namespace sync

  namespace state {
    void set(DrawCapability capability, bool enabled) {
      gl_trace_full();
      if (enabled) {
        glEnable((uint) capability);
      } else {
        glDisable((uint) capability);
      }
    }

    bool get(DrawCapability capability) {
      gl_trace_full();
      return glIsEnabled((uint) capability);
    }

    void set_op(BlendOp src_operand, BlendOp dst_operand) {
      gl_trace_full();
      glBlendFunc((uint) src_operand, (uint) dst_operand);
    }

    void set_op(DrawOp operand) {
      gl_trace_full();
      glPolygonMode(GL_FRONT_AND_BACK, (uint) operand);
    }

    void set_op(LogicOp operand) {
      gl_trace_full();
      glLogicOp((uint) operand);
    }

    void set_op(CullOp operand) {
      gl_trace_full();
      glCullFace((uint) operand);
    }

    void set_op(DepthOp operand) {
      gl_trace_full();
      glDepthFunc((uint) operand);
    }

    ScopedSet::ScopedSet(DrawCapability capability, bool enabled)
    : m_capability(capability), m_prev(get(capability)), m_curr(enabled) {
      gl_trace_full();
      guard(m_curr != m_prev);
      set(m_capability, m_curr);
    }

    ScopedSet::~ScopedSet() {
      gl_trace_full();
      guard(m_curr != m_prev);
      set(m_capability, m_prev);
    }

    void set_viewport(const eig::Array2u &size, const eig::Array2u &offset) {
      gl_trace_full();
      glViewport(offset.x(), offset.y(), size.x(), size.y());
    }
    
    void set_line_width(float width) {
      gl_trace_full();
      glLineWidth(width);
    }

    void set_point_size(float size) {
      gl_trace_full();
      glPointSize(size);
    }

    void set_depth_range(float z_near, float z_far) {
      gl_trace_full();
      glDepthRangef(z_near, z_far);
    }
    
    int get_variable_int(VariableName name) {
      gl_trace_full();
      int i;
      glGetIntegerv((uint) name, &i);
      return i;
    }

    // set_color_mask
  } // namespace state

  namespace debug {
    void enable_messages(DebugMessageSeverity minimum_severity, DebugMessageTypeFlags type_flags) {
      gl_trace_full();

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
      gl_trace_full();

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