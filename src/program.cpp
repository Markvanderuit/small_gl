#include <small_gl/detail/glm.hpp>
#include <small_gl/program.hpp>
#include <small_gl/utility.hpp>
#include <fmt/format.h>
#include <ranges>
#include <sstream>

namespace gl {
  namespace detail {
    GLint get_shader_iv(GLuint object, GLenum name) {
      GLint value;
      glGetShaderiv(object, name, &value);
      return value;
    }

    GLint get_program_iv(GLuint object, GLenum name) {
      GLint value;
      glGetProgramiv(object, name, &value);
      return value;
    }

    std::string fmt_info_log(const std::string &log) {
      std::stringstream ss_o, ss_i(log);
      for (std::string line; std::getline(ss_i, line);) {
        guard_continue(line.length() > 2);
        ss_o << fmt::format("{:<8}\n", line);
      }
      return ss_o.str();
    }

    void check_shader_compile(GLuint object) {
      guard(!get_shader_iv(object, GL_COMPILE_STATUS));

      // Compilation failed, obtain error log
      std::string info(get_shader_iv(object, GL_INFO_LOG_LENGTH), ' ');
      glGetShaderInfoLog(object, GLint(info.size()), nullptr, info.data());

      Exception e;
      e.put("src", "gl::detail::check_shader_compile(...)");
      e.put("message", "shader compilation/specialization failed, see log");
      e.put("log", fmt_info_log(info));
      throw e;
    }

    void check_program_link(GLuint object) {
      guard(!get_program_iv(object, GL_LINK_STATUS));

      // Compilation failed, obtain error log
      std::string info(get_program_iv(object, GL_INFO_LOG_LENGTH), ' ');
      glGetProgramInfoLog(object, GLint(info.size()), nullptr, info.data());

      Exception e;
      e.put("src", "gl::detail::check_program_link(...)");
      e.put("message", "program linking failed, see log");
      e.put("log", fmt_info_log(info));
      throw e;
    }

    GLuint attach_shader_object(GLuint program, const ShaderCreateInfo &i) {
      GLuint object = glCreateShader((uint) i.type);
      const GLchar *ptr = (const GLchar *) i.data.data();
      const GLint size = (GLint) i.data.size_bytes();

      if (i.is_spirv_binary) {
        // Submit spir-v binary and specialize shader
        glShaderBinary(1, &object, GL_SHADER_BINARY_FORMAT_SPIR_V, ptr, size);
        glSpecializeShader(object, i.spirv_entry_point.c_str(), 0, nullptr, nullptr);
      } else {
        // Submit glsl character data and compile shader
        glShaderSource(object, 1, &ptr, &size);
        glCompileShader(object);
      }

      check_shader_compile(object);
      glAttachShader(program, object);

      return object;
    }

    void detach_shader_object(GLuint program, GLuint object) {
      glDetachShader(program, object);
      glDeleteShader(object);
    }

    GLuint create_program_object(const std::vector<ShaderCreateInfo> &info) {
      GLuint object = glCreateProgram();

      std::vector<GLuint> shader_objects;
      shader_objects.reserve(info.size());
      
      // Generate, compile, and attach shader objects
      std::ranges::transform(info, std::back_inserter(shader_objects),
        [object] (const auto &i) { return attach_shader_object(object, i); });

      glLinkProgram(object);
      check_program_link(object);

      // Detach and destroy shader objects
      std::ranges::for_each(shader_objects, 
        [object] (const auto &i) { detach_shader_object(object, i); });

      return object;
    }

    ShaderCreateInfo zip_loaded_info(const std::vector<std::byte> &data, const ShaderLoadInfo &load_info) {
      return ShaderCreateInfo{ .type = load_info.type,  .data = data, 
                               .is_spirv_binary = load_info.is_spirv_binary, 
                               .spirv_entry_point = load_info.spirv_entry_point };
    }
  } // namespace detail


  Program::Program(std::initializer_list<ShaderLoadInfo> load_info) 
  : Base(true) {
    debug::check_expr(load_info.size() > 0, "no shader info was provided");
    
    std::vector<std::vector<std::byte>> shader_bins;
    shader_bins.reserve(load_info.size());

    // Load binary shader data into shader_bins using info's paths
    std::ranges::transform(load_info, std::back_inserter(shader_bins),
      [](const auto &i) { return io::load_shader_binary(i.path); });

    std::vector<ShaderCreateInfo> create_info;
    create_info.reserve(load_info.size());

    // Construct ShaderCreateInfo objects with shader_bins as backing data
    std::transform(shader_bins.begin(), shader_bins.end(), 
                   load_info.begin(), std::back_inserter(create_info),
                   detail::zip_loaded_info);

    m_object = detail::create_program_object(create_info);
  }

  Program::Program(std::initializer_list<ShaderCreateInfo> create_info)
  : Base(true) {
    debug::check_expr(create_info.size() > 0, "no shader info was provided");
    m_object = detail::create_program_object(create_info);
  }

  Program::Program(ShaderLoadInfo load_info)
  : Program({ load_info }) { }

  Program::Program(ShaderCreateInfo create_info)
  : Program({ create_info }) {  }

  Program::~Program() {
    guard(m_is_init);
    glDeleteProgram(m_object);
  }

  void Program::bind() const {
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glUseProgram(m_object);
  }

  void Program::unbind() const {
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glUseProgram(0);
  }

  int Program::loc(std::string_view s) {
    // Search map for the provided value
    auto f = m_loc.find(s.data());
    if (f == m_loc.end()) {
      // Obtain handle and check if it is actually valid
      GLint handle = glGetUniformLocation(m_object, s.data());
      debug::check_expr(handle >= 0, fmt::format("failed for uniform name \"{}\"", s));

      // Insert value into map
      f = m_loc.insert({s.data(), handle}).first;
    }
    return f->second;
  }

  /* Explicit template instantiations of gl::Program::uniform<...>(...) */
    
  #define gl_explicit_uniform_template(type, type_short)\
    template <> void Program::uniform<type>\
    (std::string_view s, type v)\
    { glProgramUniform1 ## type_short (m_object, loc(s), v); }\
    template <> void Program::uniform<glm::vec<2, type, glm::defaultp>>\
    (std::string_view s, glm::vec<2, type, glm::defaultp> v)\
    { glProgramUniform2 ## type_short (m_object, loc(s), v[0], v[1]); }\
    template <> void Program::uniform<glm::vec<3, type, glm::defaultp>>\
    (std::string_view s, glm::vec<3, type, glm::defaultp> v)\
    { glProgramUniform3 ## type_short (m_object, loc(s), v[0], v[1], v[2]); }\
    template <> void Program::uniform<glm::vec<4, type, glm::defaultp>>\
    (std::string_view s, glm::vec<4, type, glm::defaultp> v)\
    { glProgramUniform4 ## type_short (m_object, loc(s), v[0], v[1], v[2], v[3]); }

  #define gl_explicit_uniform_template_mat(type, type_short)\
    template <> void Program::uniform<glm::mat<2, 2, type, glm::defaultp>>\
    (std::string_view s, glm::mat<2, 2, type, glm::defaultp> v)\
    { glProgramUniformMatrix2 ## type_short ## v(m_object, loc(s), 1, false, glm::value_ptr(v)); }\
    template <> void Program::uniform<glm::mat<3, 3, type, glm::defaultp>>\
    (std::string_view s, glm::mat<3, 3, type, glm::defaultp> v)\
    { glProgramUniformMatrix4 ## type_short ## v(m_object, loc(s), 1, false, glm::value_ptr(v)); }\
    template <> void Program::uniform<glm::mat<4, 4, type, glm::defaultp>>\
    (std::string_view s, glm::mat<4, 4, type, glm::defaultp> v)\
    { glProgramUniformMatrix4 ## type_short ## v(m_object, loc(s), 1, false, glm::value_ptr(v)); }

  gl_explicit_uniform_template(bool, ui)
  gl_explicit_uniform_template(uint, ui)
  gl_explicit_uniform_template(int, i)
  gl_explicit_uniform_template(float, f)
  gl_explicit_uniform_template_mat(float, f)
} // namespace gl