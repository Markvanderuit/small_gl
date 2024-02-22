#include <small_gl/buffer.hpp>
#include <small_gl/program.hpp>
#include <small_gl/sampler.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/detail/eigen.hpp>
#include <nlohmann/json.hpp>
#include <functional>
#include <ranges>
#include <format>
#include <sstream>

namespace gl {
  // Ranges shorthands
  namespace rng = std::ranges;
  namespace vws = std::views;
  
  // Internal struct used for construction 
  struct ShaderCreateInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Shader data in binary format
    std::vector<std::byte> data;

    // SPIRV-Cross generated reflection json files
    std::vector<io::json> cross_json;

    // Is the attached shader data a spir-v binary?
    bool is_spirv = false;
    std::string spirv_entry_point = "main";
    std::vector<std::pair<uint, uint>> spirv_spec_const = { };
  };

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
        ss_o << std::format("{:<8}\n", line);
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
      auto size = (GLint)    i.data.size();

      // Assemble shader object
      GLuint object = glCreateShader((uint) i.type);
      if (i.is_spirv) {
        // Split specialization constants into index/value
        auto const_i = i.spirv_spec_const | vws::elements<0> | rng::to<std::vector>();
        auto const_v = i.spirv_spec_const | vws::elements<1> | rng::to<std::vector>();

        // Submit spir-v binary and specialize shader
        glShaderBinary(1, &object, GL_SHADER_BINARY_FORMAT_SPIR_V, ptr, size);
        glSpecializeShader(object, i.spirv_entry_point.c_str(), 
          i.spirv_spec_const.size(), const_i.data(), const_v.data());
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
      rng::transform(info, std::back_inserter(shader_objects),
        [object] (const auto &i) { return attach_shader_object(object, i); });
      
      glLinkProgram(object);
      check_program_link(object);

      // Detach and destroy shader objects
      rng::for_each(shader_objects, 
        [object] (const auto &i) { detach_shader_object(object, i); });

      return object;
    }
  } // namespace detail

  /* Program code */

  // Program::Program(const ShaderLoadSPIRVInfo &load_info)
  // : Program({ load_info }) { }

  // Program::Program(const ShaderLoadGLSLInfo &load_info)
  // : Program({ load_info }) { }

  // Program::Program(const ShaderLoadSPIRVStringInfo &load_info)
  // : Program({ load_info }) { }

  // Program::Program(const ShaderLoadGLSLStringInfo &load_info)
  // : Program({ load_info }) { }
  
  
  Program::Program(std::initializer_list<ShaderLoadSPIRVInfo> info)       
  : Program(std::span(info.begin(), info.end())) {}
  Program::Program(std::initializer_list<ShaderLoadGLSLInfo> info)        
  : Program(std::span(info.begin(), info.end())) {}
  Program::Program(std::initializer_list<ShaderLoadSPIRVStringInfo> info) 
  : Program(std::span(info.begin(), info.end())) {}
  Program::Program(std::initializer_list<ShaderLoadGLSLStringInfo> info)  
  : Program(std::span(info.begin(), info.end())) {}
    
  Program::Program(const ShaderLoadSPIRVInfo       &info) 
  : Program({ info }) { }
  Program::Program(const ShaderLoadGLSLInfo        &info) 
  : Program({ info }) { }
  Program::Program(const ShaderLoadSPIRVStringInfo &info) 
  : Program({ info }) { }
  Program::Program(const ShaderLoadGLSLStringInfo  &info) 
  : Program({ info }) { }

  Program::Program(std::span<const ShaderLoadSPIRVInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr(load_info.size() > 0, "no shader info was provided");
    
    // Transform to internal load info object
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    rng::transform(load_info, create_info.begin(), [](const ShaderLoadSPIRVInfo &info) {
      return ShaderCreateInfo { .type              = info.type,  
                                .data              = io::load_shader_binary(info.spirv_path), 
                                .is_spirv          = true, 
                                .spirv_entry_point = info.entry_point,
                                .spirv_spec_const  = info.spec_const };
    });

    // Initialize program
    m_object = detail::create_program_object(create_info);

    // Handle reflectance data population, if available
    auto filt = load_info | vws::filter([](const auto &info) { return !info.cross_path.empty(); });
    for (const auto &info : filt) populate(info.cross_path);
  }

  Program::Program(std::span<const ShaderLoadSPIRVStringInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr(load_info.size() > 0, "no shader info was provided");
    
    // Transform to internal load info object
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    rng::transform(load_info, create_info.begin(), [](const ShaderLoadSPIRVStringInfo &info) {
      return ShaderCreateInfo { .type              = info.type,  
                                .data              = info.spirv_data, 
                                .is_spirv          = true, 
                                .spirv_entry_point = info.entry_point,
                                .spirv_spec_const  = info.spec_const };
    });

    // Initialize program
    m_object = detail::create_program_object(create_info);

    // Handle reflectance data population, if available
    auto filt = load_info | vws::filter([](const auto &info) { return !info.cross_json.empty(); });
    for (const auto &info : filt) populate(info.cross_json);
  }

  Program::Program(std::span<const ShaderLoadGLSLInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr(load_info.size() > 0, "no shader info was provided");
    
    // Transform to internal load info object
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    rng::transform(load_info, create_info.begin(), [](const ShaderLoadGLSLInfo &info) {
      return ShaderCreateInfo { .type              = info.type,  
                                .data              = io::load_shader_binary(info.glsl_path), 
                                .is_spirv          = false };
    });

    // Initialize program
    m_object = detail::create_program_object(create_info);

    // Handle reflectance data population, if available
    auto filt = load_info | vws::filter([](const auto &info) { return !info.cross_path.empty(); });
    for (const auto &info : filt) populate(info.cross_path);
  }

  Program::Program(std::span<const ShaderLoadGLSLStringInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr(load_info.size() > 0, "no shader info was provided");
    
    // Transform to internal load info object
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    rng::transform(load_info, create_info.begin(), [](const ShaderLoadGLSLStringInfo &info) {
      return ShaderCreateInfo { .type              = info.type,  
                                .data              = info.glsl_data, 
                                .is_spirv          = false };
    });

    // Initialize program
    m_object = detail::create_program_object(create_info);

    // Handle reflectance data population, if available
    auto filt = load_info | vws::filter([](const auto &info) { return !info.cross_json.empty(); });
    for (const auto &info : filt) populate(info.cross_json);
  }

  Program::~Program() {
    guard(m_is_init);
    glDeleteProgram(m_object);
  }

  void Program::bind() const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glUseProgram(m_object);
  }

  void Program::unbind() const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glUseProgram(0);
  }

  void Program::unbind_all() {
    gl_trace_full();
    glUseProgram(0);
  }

  int Program::loc(std::string_view s) {
    gl_trace_full();

    // Search map for the provided value; reflect if value is not yet located
    auto f = m_binding_data.find(s.data());
    if (f == m_binding_data.end()) {
      // Obtain handle and check if it is actually valid
      GLint handle = glGetUniformLocation(m_object, s.data());
      debug::check_expr(handle >= 0, 
        std::format("Program::uniform(...) failed with name lookup for uniform name: \"{}\"", s));

      BindingData data { .type       = BindingType::eUniform, 
                         .access     = BindingAccess::eReadOnly,
                         .binding    = handle };
      f = m_binding_data.emplace(s.data(), data).first;
    }

    // Test extracted value correctness
    const BindingData &data = f->second;
    debug::check_expr(data.type == BindingType::eUniform,
      std::format("Program::bind(...) failed with type mismatch for buffer name: \"{}\"", s));

    return data.binding;
  }
  
  void Program::populate(fs::path refl_path) {
    gl_trace_full();
    populate(io::load_json(refl_path));
  }

  void Program::populate(io::json js) {
    gl_trace_full();

    // Function to consume reflectance data from the json, for general bindings
    auto func_general = [&](const auto &iter, BindingType type) {
      guard(iter.contains("name") && iter.contains("binding"));
      BindingData data { .type       = type, 
                         .access     = BindingAccess::eReadOnly,
                         .binding    = iter.at("binding").get<int>() };
      m_binding_data.emplace(iter.at("name").get<std::string>(), data);
    };

    // Function to consume reflectance data from the json, for bindings with read/write qualifiers
    auto func_qualifier = [&](const auto &iter, BindingType type) {
      guard(iter.contains("name") && iter.contains("binding"));
      BindingData data { .type       = type, 
                         .binding    = iter.at("binding").get<int>() };
      if (iter.contains("writeonly") && iter.at("writeonly").get<bool>()) 
        data.access = BindingAccess::eWriteOnly;
      if (iter.contains("readonly") && iter.at("readonly").get<bool>()) 
        data.access = BindingAccess::eReadOnly;
      m_binding_data.emplace(iter.at("name").get<std::string>(), data);
    };

    using namespace std::placeholders;

    // Consume reflectance data for bindable types; buffers, textures, samplers, images, ...
    if (js.contains("ubos"))
      rng::for_each(js.at("ubos"), std::bind(func_general, _1, BindingType::eUniformBuffer));
    if (js.contains("textures")) // textures/samplers share name/binding
      rng::for_each(js.at("textures"), std::bind(func_general, _1, BindingType::eSampler));
    if (js.contains("ssbos"))
      rng::for_each(js.at("ssbos"), std::bind(func_qualifier, _1, BindingType::eStorageBuffer));
    if (js.contains("images"))
      rng::for_each(js.at("images"), std::bind(func_qualifier, _1, BindingType::eImage));
  }
  
  void Program::bind(std::string_view s, const gl::AbstractTexture &texture, BindingType binding) {
    gl_trace_full();

    auto f = m_binding_data.find(s.data());
    debug::check_expr(f != m_binding_data.end(),
      std::format("Program::bind(...) failed with name lookup for texture name: \"{}\"", s));
      
    const BindingData &data = f->second;
    debug::check_expr(data.type == BindingType::eSampler || data.type == BindingType::eImage,
      std::format("Program::bind(...) failed with type mismatch for texture name: \"{}\"", s));

    if (data.type == BindingType::eSampler) {
      texture.bind_to(gl::TextureTargetType::eTextureUnit, data.binding, 0);
    } else {
      switch (data.access) {
        case BindingAccess::eReadOnly:  
          texture.bind_to(gl::TextureTargetType::eImageReadOnly, data.binding, 0);
          break;
        case BindingAccess::eWriteOnly: 
          texture.bind_to(gl::TextureTargetType::eImageWriteOnly, data.binding, 0);
          break;
        case BindingAccess::eReadWrite: 
          texture.bind_to(gl::TextureTargetType::eImageReadWrite, data.binding, 0);
          break;
      }
    }
  }

  void Program::bind(std::string_view s, const gl::Buffer &buffer, size_t size, size_t offset, BindingType binding) {
    gl_trace_full();

    auto f = m_binding_data.find(s.data());
    debug::check_expr(f != m_binding_data.end(),
      std::format("Program::bind(...) failed with name lookup for buffer name: \"{}\"", s));

    const BindingData &data = f->second;
    debug::check_expr(
      data.type == BindingType::eUniformBuffer || data.type == BindingType::eStorageBuffer,
      std::format("Program::bind(...) failed with type mismatch for buffer name: \"{}\"", s));

    // TODO; expand secondary types
    auto target = data.type == BindingType::eUniformBuffer 
                ? gl::BufferTargetType::eUniform
                : gl::BufferTargetType::eStorage;
    buffer.bind_to(target, data.binding, size, offset);
  }

  void Program::bind(std::string_view s, const gl::Sampler &sampler, BindingType binding) {
    gl_trace_full();

    auto f = m_binding_data.find(s.data());
    debug::check_expr(f != m_binding_data.end(),
      std::format("Program::bind(...) failed with name lookup for sampler name: \"{}\"", s));
      
    const BindingData &data = f->second;
    debug::check_expr(
      data.type == BindingType::eSampler,
      std::format("Program::bind(...) failed with type mismatch for sampler name: \"{}\"", s));
    
    sampler.bind_to(data.binding);
  }

  /* ProgramCache code */

  std::pair<std::string, gl::Program &> ProgramCache::set(InfoType &&info) {
    gl_trace();

    // Visitor generates key, and tests if program exists in cache
    auto key = std::visit([](const auto &i) { return i.to_string(); }, info);
    auto it  = m_prog_cache.find(key);

    // If program is not in cache
    if (it == m_prog_cache.end()) {
      // Visitor generates program
      auto prog = std::visit([](const auto &i) { return gl::Program(i); }, info);
      
      // Program and info object are newly cached
           m_info_cache.emplace(key, std::vector { std::move(info) });
      it = m_prog_cache.emplace(key, std::move(prog)).first;
    }

    return { key, it->second };
  }

  std::pair<std::string, gl::Program &> ProgramCache::set(InfoList &&info) {
    gl_trace();

    // Visitor generates key, and tests if program exists in cache,
    // by joining consecutive info objects as keys
    auto key = std::visit([](const auto &l) {
      return l | vws::transform([](const auto &i) { return i.to_string(); }) 
               | vws::join 
               | rng::to<std::string>();
    }, info);
    auto it  = m_prog_cache.find(key);

    // If program is not in cache
    if (it == m_prog_cache.end()) {
      // Visitor generates program from list
      auto prog = std::visit([](const auto &l) { return gl::Program(l); }, info);
      
      // Program and info object are newly cached
      auto cval = std::visit([](const auto &l) { 
        return l | vws::transform([](const auto &t) { return InfoType(t); }) 
                 | rng::to<std::vector>();  }, info);
           m_info_cache.emplace(key, std::move(cval));
      it = m_prog_cache.emplace(key, std::move(prog)).first;
    }

    return { key, it->second };
  }

  gl::Program & ProgramCache::at(const KeyType &k) {
    gl_trace();
    auto f = m_prog_cache.find(k);
    debug::check_expr(f != m_prog_cache.end(),
      std::format("ProgramCache::at(...) failed with key lookup for key: \"{}\"", k));
    return f->second;
  }

  void ProgramCache::clear() {
    gl_trace();
    m_info_cache.clear();
    m_prog_cache.clear();
  }

  void ProgramCache::reload() {
    gl_trace_full();
    /* for (const auto &[key, info] : m_info_cache)
      m_prog_cache[key] = std::visit([](const auto &i) {
        std::span s = i;
        return gl::Program(s); 
      }, info); */
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