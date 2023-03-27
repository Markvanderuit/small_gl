#include <small_gl/program.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl_parser/parser.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <functional>
#include <ranges>
#include <sstream>

namespace gl {
  namespace detail {
    GLint get_shader_iv(GLuint object, GLenum name) {
      gl_trace_full();
      GLint value;
      glGetShaderiv(object, name, &value);
      return value;
    }

    GLint get_program_iv(GLuint object, GLenum name) {
      gl_trace_full();
      GLint value;
      glGetProgramiv(object, name, &value);
      return value;
    }

    std::string fmt_info_log(const std::string &log) {
      gl_trace();
      std::stringstream ss_o, ss_i(log);
      for (std::string line; std::getline(ss_i, line);) {
        guard_continue(line.length() > 2);
        ss_o << fmt::format("{:<8}\n", line);
      }
      return ss_o.str();
    }

    void check_shader_compile(GLuint object) {
      gl_trace_full();
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
      gl_trace_full();
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
      gl_trace_full();

      auto *ptr = (GLchar *) i.data.data();
      auto size = (GLint)    i.data.size_bytes();
      
      // If a parser is provided; perform parse into parser_buffer
      std::string parser_buffer;
      if (i.parser) {
        // Resize buffer to accomodate
        parser_buffer.resize(size);
        std::copy(ptr, ptr + size, parser_buffer.begin());
        
        parser_buffer = i.parser->parse_str(parser_buffer);

        // Redirect pointers to parsed buffer instead of provided input buffer
        ptr  = (GLchar *) parser_buffer.data();
        size = (GLint) parser_buffer.size();
      }

      // Assemble shader object
      GLuint object = glCreateShader((uint) i.type);
      if (i.is_spirv) {
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
      gl_trace_full();
      glDetachShader(program, object);
      glDeleteShader(object);
    }

    GLuint create_program_object(const std::vector<ShaderCreateInfo> &info) {
      gl_trace_full();

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
      return ShaderCreateInfo {
        .type              = load_info.type,  
        .data              = data, 
        .is_spirv          = load_info.is_spirv, 
        .spirv_entry_point = load_info.spirv_entry_point,
        .parser            = load_info.parser
      };
    }
  } // namespace detail

  Program::Program(std::initializer_list<ShaderLoadInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr_dbg(load_info.size() > 0, "no shader info was provided");
    
    // Load binary shader data into shader_bins using info's paths
    std::vector<std::vector<std::byte>> shader_bins(load_info.size());
    std::ranges::transform(load_info, shader_bins.begin(),
      [](const auto &i) { return io::load_shader_binary(i.path); });

    // Construct ShaderCreateInfo objects with shader_bins as backing data
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    std::transform(range_iter(shader_bins), load_info.begin(), create_info.begin(), detail::zip_loaded_info);

    m_object = detail::create_program_object(create_info);
  }

  Program::Program(std::initializer_list<ShaderCreateInfo> create_info)
  : Base(true) {
    gl_trace_full();
    debug::check_expr_dbg(create_info.size() > 0, "no shader info was provided");
    m_object = detail::create_program_object(create_info);
  }

  Program::Program(ShaderLoadInfo load_info)
  : Program({ load_info }) { }

  Program::Program(ShaderCreateInfo create_info)
  : Program({ create_info }) { }

  Program::~Program() {
    guard(m_is_init);
    glDeleteProgram(m_object);
  }

  void Program::bind() const {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glUseProgram(m_object);
  }

  void Program::unbind() const {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glUseProgram(0);
  }

  void Program::unbind_all() {
    gl_trace_full();
    glUseProgram(0);
  }

  int Program::loc(std::string_view s) {
    gl_trace_full();

    // Search map for the provided value; reflect if value is not yet located
    auto f = m_locations_uniform.find(s.data());
    if (f == m_locations_uniform.end()) {
      // Obtain handle and check if it is actually valid
      GLint handle = glGetUniformLocation(m_object, s.data());
      debug::check_expr_dbg(handle >= 0, fmt::format("failed for uniform name \"{}\"", s));

      // Insert value into map
      f = m_locations_uniform.insert({s.data(), handle}).first;
    }

    return f->second;
  }

  
  void Program::populate(fs::path refl_path) {
    return populate(io::load_json(refl_path));
  }

  void Program::populate(io::json js) {
    using namespace std::placeholders;
    
    auto func = [&](const auto &iter, BindingType type) {
      guard(iter.contains("name") && iter.contains("binding"));
      BindingData data { type, iter.at("binding").get<int>() };
      m_locations_data.emplace(iter.at("name").get<std::string>(), data);
    };

    if (js.contains("ubos"))
      std::ranges::for_each(js.at("ubos"), std::bind(func, _1, BindingType::eUniformBuffer));
    
    if (js.contains("ssbos"))
      std::ranges::for_each(js.at("ubos"), std::bind(func, _1, BindingType::eShaderStorageBuffer));

    if (js.contains("images"))
      std::ranges::for_each(js.at("ubos"), std::bind(func, _1, BindingType::eImage));

    if (js.contains("textures")) // textures/samplers share name/binding
      std::ranges::for_each(js.at("ubos"), std::bind(func, _1, BindingType::eSampler));
  }
  
  /* Explicit template instantiations of gl::Program::uniform<...>(...) */
    
  #define gl_explicit_uniform_template_1(type, short_type)                                         \
    template <> void Program::uniform<type>                                                        \
    (std::string_view s, const type &v)                                                            \
    { glProgramUniform1 ## short_type (m_object, loc(s), v); }
    
  #define gl_explicit_uniform_template_vector(type, vector_type, short_type)                       \
    template <> void Program::uniform<vector_type<type, 2, 1>>                                     \
      (std::string_view s, const vector_type<type, 2, 1> &v)                                       \
      { glProgramUniform2 ## short_type (m_object, loc(s), v[0], v[1]); }                          \
    template <> void Program::uniform<vector_type<type, 3, 1>>                                     \
      (std::string_view s, const vector_type<type, 3, 1> &v)                                       \
      { glProgramUniform3 ## short_type (m_object, loc(s), v[0], v[1], v[2]); }                    \
    template <> void Program::uniform<vector_type<type, 4, 1>>                                     \
      (std::string_view s, const vector_type<type, 4, 1> &v)                                       \
      { glProgramUniform4 ## short_type (m_object, loc(s), v[0], v[1], v[2], v[3]); }

  #define gl_explicit_uniform_template_matrix(type, matrix_type, short_type)                          \
    template <> void Program::uniform<matrix_type<type, 2, 2>>                                     \
    (std::string_view s, const matrix_type<type, 2, 2> &v)                                         \
    { glProgramUniformMatrix2 ## short_type ## v(m_object, loc(s), 1, false, v.data()); }          \
    template <> void Program::uniform<matrix_type<type, 3, 3>>                                     \
    (std::string_view s, const matrix_type<type, 3, 3> &v)                                         \
    { glProgramUniformMatrix3 ## short_type ## v(m_object, loc(s), 1, false, v.data()); }          \
    template <> void Program::uniform<matrix_type<type, 4, 4>>                                     \
    (std::string_view s, const matrix_type<type, 4, 4> &v)                                         \
    { glProgramUniformMatrix4 ## short_type ## v(m_object, loc(s), 1, false, v.data()); }

  #define gl_explicit_uniform_template(type, short_type)                                           \
    gl_explicit_uniform_template_1(type, short_type)                                               \
    gl_explicit_uniform_template_vector(type, eig::Array, short_type)                              \
    gl_explicit_uniform_template_vector(type, eig::Matrix, short_type)

  gl_explicit_uniform_template(bool, ui)
  gl_explicit_uniform_template(uint, ui)
  gl_explicit_uniform_template(int, i)
  gl_explicit_uniform_template(float, f)
  gl_explicit_uniform_template_matrix(float, eig::Matrix, f)
  gl_explicit_uniform_template_matrix(float, eig::Array, f)
} // namespace gl