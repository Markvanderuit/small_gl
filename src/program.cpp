#include <small_gl/buffer.hpp>
#include <small_gl/program.hpp>
#include <small_gl/sampler.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/utility.hpp>
#include <small_gl/detail/eigen.hpp>
#include <nlohmann/json.hpp>
#include <zstr.hpp>
#include <algorithm>
#include <execution>
#include <functional>
#include <ranges>
#include <sstream>

namespace gl {
  // Ranges shorthands
  namespace rng = std::ranges;
  namespace vws = std::views;
  
  // Helper code for gl::view_to<Ty>
  // Src: https://stackoverflow.com/questions/58808030/range-view-to-stdvector
  namespace detail {
    // Type acts as a tag to find the correct operator| overload
    template <typename C>
    struct view_to { };
    
    // This actually does the work
    template <typename CTy, rng::range RTy> requires (std::convertible_to<rng::range_value_t<RTy>, typename CTy::value_type>)
    CTy operator|(RTy&& r, view_to<CTy>) {
      return CTy { r.begin(), r.end() };
    }
  } // namespace detail

  // Helper view; replaces std::ranges::to<Ty> given it is only supported from gcc 14 or something
  template <rng::range CTy> requires (!rng::view<CTy>)
  inline constexpr auto view_to() { return detail::view_to<CTy>{}; }

  namespace io {
    // Serialization for std::unordered_map<std::string,...>
    template <typename Ty> /* requires(!is_serializable<Ty>) */
    void to_stream(const std::unordered_map<std::string, Ty> &v, std::ostream &str) {
      gl_trace();
      
      to_stream(v.size(), str);
      for (const auto &[key, value] : v) {
        to_stream(key, str);
        to_stream(value, str);
      }
    }

    // Serialization for std::unordered_map<std::string,...>
    template <typename Ty> /* requires (!is_serializable<Ty>) */
    void from_stream(std::unordered_map<std::string, Ty> &v, std::istream &str) {
      gl_trace();

      size_t n;
      from_stream(n, str);
      for (size_t i = 0; i < n; ++i) {
        std::string key;
        Ty value;
        from_stream(key, str);
        from_stream(value, str);
        v.insert({ key, std::move(value) });
      }
    }
  } // namespace io

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
    std::vector<std::pair<uint, uint>> spirv_spec_const = { };
  };

  std::string ShaderLoadFileInfo::to_string() const {
    std::stringstream ss;
    ss << fmt::format("{}_{}_{}_{}", 
                      static_cast<gl::uint>(type), 
                      glsl_path.string(), 
                      spirv_path.string(), 
                      cross_path.string());
    for (const auto &[i, value] : spec_const)
      ss << fmt::format("_({},{})", i, value);
    return ss.str();
  }

  std::string program_name_from_paths(std::span<const ShaderLoadFileInfo> info) {
    // Gather filenames of relevant shader files or spirv binaries
    auto names 
      = info 
      | vws::transform([](const auto &i) { 
        if (!i.spirv_path.empty() && get_vendor() != VendorType::eIntel) {
          return i.spirv_path.filename().string();
        } else {
          return i.glsl_path.filename().string();
        }
      });
    
    // Assemble into readable string
    std::stringstream ss;
    for (const auto &str : names | vws::take(names.size() - 1)) {
      ss << str << " -> ";
    }
    ss << names.back();
    return ss.str();
  }

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

    GLuint attach_shader_object(GLuint program, const ShaderCreateInfo &info) {
      gl_trace_full();

      // Assemble shader object
      GLuint object = glCreateShader((uint) info.type);
      if (info.is_spirv) {
        // Get raw ptr/size in requested types
        auto *ptr = (GLchar *) info.data.data();
        auto size = (GLint)    info.data.size();

        // Split specialization constants into index/value
        auto const_i = info.spirv_spec_const | vws::elements<0> | view_to<std::vector<uint>>();
        auto const_v = info.spirv_spec_const | vws::elements<1> | view_to<std::vector<uint>>();

        // Submit spir-v binary and specialize shader
        glShaderBinary(1, &object, GL_SHADER_BINARY_FORMAT_SPIR_V, ptr, size);
        glSpecializeShader(object, "main", info.spirv_spec_const.size(), const_i.data(), const_v.data());
      } else {
        // As the glsl path does not support specialization constants, we replace these manually
        std::string copy(range_iter(detail::cast_span<const char>(std::span(info.data))));

        // Split into vector of lines
        std::vector<std::string> lines;
        auto split_view = vws::split('\n') | vws::transform([](const auto &l) { return std::string(range_iter(l)); });
        rng::copy(copy | split_view, std::back_inserter(lines));

        // Find lines of interest and override them to a specified value;
        // expected format of line is approximately 'layout(constant_id = 2) const uint bla = 1;'
        // TODO clean this up after presentation, no time no time
        std::for_each(std::execution::par_unseq, range_iter(lines), [&](std::string &line) {
          guard(line.contains("constant_id"));
          
          // Strip '\r'
          line.erase(std::remove_if(range_iter(line), [](char c) { return c == '\r'; }), line.end());

          // Find substring beyond '=' and before ')', then strip ' ' from what remians
          auto subitr = line.find_first_of('=');
          auto substr = line.substr(subitr + 1);
          
          // On ')', we know the constant id is the previous part
          subitr = substr.find_first_of(')');
          auto substr_before = substr.substr(0, subitr);
          auto substr_after  = substr.substr(subitr + 1);

          // Strip whitespace from previous part, what remains is constant id
          substr.erase(std::remove_if(range_iter(substr), [](char c) { return c == ' '; }), substr.end());

          // Split on whitespace in after part, get iterator over line data
          std::vector<std::string> line_split;
          auto split_view = vws::split(' ') | vws::transform([](const auto &l) { return std::string(range_iter(l)); });
          rng::copy_if(substr_after | split_view, std::back_inserter(line_split), [](auto c) { return !c.empty(); });
          auto line_itr = line_split.begin();

          // Extract variable type, variable name, variable value;
          // expected format is 'const type name = value', though const is optional
          if (*line_itr == "const") 
            line_itr++;
          std::string type_name = *line_itr;
          line_itr++;
          std::string varl_name = *line_itr;
          line_itr++;
          line_itr++;
          std::string varl_value = *line_itr;

          // Find new spec const value and replace line entirely if specified
          auto it = rng::find(info.spirv_spec_const, 
                              static_cast<uint>(std::stoi(substr)), 
                              [](const auto &pair) { return pair.first; });
          if (it != info.spirv_spec_const.end())
            varl_value = fmt::format("{}", it->second);
          
          // Finally, replace line
          line = fmt::format("const {} {} = {};", type_name, varl_name, varl_value);
        });

        // Merge back into string
        copy.clear();
        rng::copy(lines | vws::join_with('\n'), std::back_inserter(copy));

        // Get raw ptr/size in requested types
        auto *ptr = (GLchar *) copy.data();
        auto size = (GLint)    copy.size();

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

      std::vector<GLuint> shader_objects(info.size());

      // Generate, compile, and attach shader objects
      rng::transform(info, shader_objects.begin(),
        [object](const auto &i) { return attach_shader_object(object, i); });
      
      // Perform program link
      {
        gl_trace_full_n("Program link");
        glLinkProgram(object);
        check_program_link(object);
      }  

      // Detach and destroy shader objects
      rng::for_each(shader_objects, 
        [object](const auto &i) { detach_shader_object(object, i); });

      return object;
    }

    GLuint create_program_object_from_binary(uint format, const std::vector<std::byte> &data) {
      GLuint object = glCreateProgram();
      glProgramBinary(object, format, data.data(), data.size());
      check_program_link(object);
      return object;
    }
  } // namespace detail

  /* Program code */  
  
  Program::Program(std::initializer_list<ShaderLoadFileInfo> info)       
  : Program(std::span(info.begin(), info.end())) { }
  Program::Program(std::initializer_list<ShaderLoadStringInfo> info)       
  : Program(std::span(info.begin(), info.end())) { }
  
  Program::Program(const ShaderLoadFileInfo &info) 
  : Program({ info }) { }
  Program::Program(const ShaderLoadStringInfo &info) 
  : Program({ info }) { }

  Program::Program(std::span<const ShaderLoadFileInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr(load_info.size() > 0, "no shader info was provided");

    // Output OpenGL debug message to warn of shader load+compile
    // Format shader name from set of shader paths
    debug::insert_message(
      fmt::format("Program load and compile: {}", program_name_from_paths(load_info)), 
      gl::DebugMessageSeverity::eLow);
    
    // Transform to internal load info object
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    rng::transform(load_info, create_info.begin(), [](const ShaderLoadFileInfo &info) {
      if (!info.spirv_path.empty() && get_vendor() != VendorType::eIntel) {
        // Return necessary info for spirv path
        return ShaderCreateInfo { .type              = info.type,  
                                  .data              = io::load_binary(info.spirv_path), 
                                  .is_spirv          = true, 
                                  .spirv_spec_const  = info.spec_const };
      } else if (!info.glsl_path.empty()) {
        // Fall back to glsl path
        return ShaderCreateInfo { .type     = info.type,  
                                  .data     = io::load_binary(info.glsl_path), 
                                  .is_spirv = false };
      } else {
        debug::check_expr(false, "ShaderLoadFileInfo is in an incomplete state.");
      }
    });
    
    // Initialize program from shader info
    m_object = detail::create_program_object(create_info);

    // Handle reflectance data population, if available
    auto filt = load_info | vws::filter([](const auto &info) { return !info.cross_path.empty(); });
    for (const auto &info : filt) populate(info.cross_path);
  }

  Program::Program(std::span<const ShaderLoadStringInfo> load_info) 
  : Base(true) {
    gl_trace_full();
    debug::check_expr(load_info.size() > 0, "no shader info was provided");
    
    // Transform to internal load info object
    std::vector<ShaderCreateInfo> create_info(load_info.size());
    rng::transform(load_info, create_info.begin(), [](const ShaderLoadStringInfo &info) {
      if (!info.spirv_data.empty() && get_vendor() != VendorType::eIntel) {
        return ShaderCreateInfo { .type              = info.type,  
                                  .data              = info.spirv_data, 
                                  .is_spirv          = true, 
                                  .spirv_spec_const  = info.spec_const };
      } else if (!info.glsl_data.empty()) {
        return ShaderCreateInfo { .type              = info.type,  
                                  .data              = info.glsl_data, 
                                  .is_spirv          = false };
      } else {
        debug::check_expr(false, "ShaderLoadStringInfo is in an incomplete state.");
      }
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
        fmt::format("Program::uniform(...) failed with name lookup for uniform name: \"{}\"", s));

      BindingData data { .type       = BindingType::eUniform, 
                         .access     = BindingAccess::eReadOnly,
                         .binding    = handle };
      f = m_binding_data.emplace(s.data(), data).first;
    }

    // Test extracted value correctness
    const BindingData &data = f->second;
    debug::check_expr(data.type == BindingType::eUniform,
      fmt::format("Program::bind(...) failed with type mismatch for buffer name: \"{}\"", s));

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
                         .binding    = iter.at("binding").template get<int>() };
      m_binding_data.emplace(iter.at("name").template get<std::string>(), data);
    };

    // Function to consume reflectance data from the json, for bindings with read/write qualifiers
    auto func_qualifier = [&](const auto &iter, BindingType type) {
      guard(iter.contains("name") && iter.contains("binding"));
      BindingData data { .type       = type, 
                         .binding    = iter.at("binding").template get<int>() };
      if (iter.contains("writeonly") && iter.at("writeonly").template get<bool>()) 
        data.access = BindingAccess::eWriteOnly;
      if (iter.contains("readonly") && iter.at("readonly").template get<bool>()) 
        data.access = BindingAccess::eReadOnly;
      m_binding_data.emplace(iter.at("name").template get<std::string>(), data);
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

  void Program::bind(std::string_view s, const gl::AbstractTexture &texture, const gl::Sampler &sampler, BindingType binding) {
    gl_trace_full();

    auto f = m_binding_data.find(s.data());
    debug::check_expr(f != m_binding_data.end(),
      fmt::format("Program::bind(...) failed with name lookup for texture name: \"{}\"", s));
      
    const BindingData &data = f->second;
    debug::check_expr(data.type == BindingType::eSampler,
      fmt::format("Program::bind(...) failed with type mismatch for texture name: \"{}\"", s));
    
    texture.bind_to(gl::TextureTargetType::eTextureUnit, data.binding, 0);
    sampler.bind_to(data.binding);
  }

  
  void Program::bind(std::string_view s, const gl::AbstractTexture &texture, BindingType binding) {
    gl_trace_full();

    auto f = m_binding_data.find(s.data());
    debug::check_expr(f != m_binding_data.end(),
      fmt::format("Program::bind(...) failed with name lookup for texture name: \"{}\"", s));
      
    const BindingData &data = f->second;
    debug::check_expr(data.type == BindingType::eSampler || data.type == BindingType::eImage,
      fmt::format("Program::bind(...) failed with type mismatch for texture name: \"{}\"", s));

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
      fmt::format("Program::bind(...) failed with name lookup for buffer name: \"{}\"", s));

    const BindingData &data = f->second;
    debug::check_expr(
      data.type == BindingType::eUniformBuffer || data.type == BindingType::eStorageBuffer,
      fmt::format("Program::bind(...) failed with type mismatch for buffer name: \"{}\"", s));

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
      fmt::format("Program::bind(...) failed with name lookup for sampler name: \"{}\"", s));
      
    const BindingData &data = f->second;
    debug::check_expr(
      data.type == BindingType::eSampler,
      fmt::format("Program::bind(...) failed with type mismatch for sampler name: \"{}\"", s));
    
    sampler.bind_to(data.binding);
  }
  
  void Program::to_stream(std::ostream &str) const {
    gl_trace();

    // Get program binary length
    int program_length;
    glGetProgramiv(m_object, GL_PROGRAM_BINARY_LENGTH , &program_length);

    // Get program binary format and data
    uint program_format;
    std::vector<std::byte> program_data(program_length);
    glGetProgramBinary(m_object, program_length, nullptr, &program_format, program_data.data());

    // Serialize binary and uniform binding data
    io::to_stream(program_format, str);
    io::to_stream(program_data, str);
    io::to_stream(m_binding_data, str);
  }

  void Program::from_stream(std::istream &str) {
    gl_trace();

    // Deserialize binary and uniform binding data
    uint program_format;
    std::vector<std::byte> program_data;
    io::from_stream(program_format, str);
    io::from_stream(program_data, str);
    io::from_stream(m_binding_data, str);
    
    // Instantiate program and assume ownersip over resulting object handle
    m_is_init = true;
    m_object  = detail::create_program_object_from_binary(program_format, program_data);
  }

  std::pair<std::string, gl::Program &> ProgramCache::set(InfoType &&info) {
    gl_trace();

    // Generate key, and test if program is resident
    auto key = info.to_string();
    auto it  = m_prog_cache.find(key);

    // If program is not resident, generate program, then cache it
    if (it == m_prog_cache.end())
      it = m_prog_cache.emplace(key, Program(info)).first;

    return { key, it->second };
  }

  std::pair<std::string, gl::Program &> ProgramCache::set(InfoList &&info) {
    gl_trace();

    // Generate key from join of consecutive info object keys
    auto in = info | vws::transform([](const auto &i) { return i.to_string(); }) | vws::join;
    std::string key;
    rng::copy(in, std::back_inserter(key));

    // Test if program is resident
    auto it  = m_prog_cache.find(key);

    // If program is not resident, generate program, then cache it
    if (it == m_prog_cache.end())
      it = m_prog_cache.emplace(key, gl::Program(info)).first;

    return { key, it->second };
  }

  gl::Program & ProgramCache::at(const KeyType &k) {
    gl_trace();
    auto f = m_prog_cache.find(k);
    debug::check_expr(f != m_prog_cache.end(),
      fmt::format("ProgramCache::at(...) failed with key lookup for key: \"{}\"", k));
    return f->second;
  }

  void ProgramCache::clear() {
    gl_trace();
    m_prog_cache.clear();
  }

  ProgramCache::ProgramCache(fs::path cache_file_path) { load(cache_file_path); }

  void ProgramCache::save(fs::path cache_file_path) const {
    gl_trace();

    // Attempt opening zlib compressed stream
    constexpr auto str_flags = std::ios::out | std::ios::binary | std::ios::trunc;
    auto str = zstr::ofstream(cache_file_path.string(), str_flags, Z_BEST_SPEED);
    debug::check_expr(str.good());

    // Serialize program cache to stream
    io::to_stream(m_prog_cache, str);

    // Output OpenGL debug message to warn of cache save
    debug::insert_message(
      fmt::format("Program cache saved to: {}", cache_file_path.string()), 
      gl::DebugMessageSeverity::eLow);
  }

  void ProgramCache::load(fs::path cache_file_path) {
    gl_trace();
    
    // Sanity check file path
    debug::check_expr(fs::exists(cache_file_path),
      fmt::format("Program cache cannot load; cache does not exist at: {}", cache_file_path.string()));

    // Clear out cache first
    *this = { };

    // Next, attempt opening zlib compressed stream, and deserialize to scene object
    constexpr auto str_flags = std::ios::in  | std::ios::binary;
    auto str = zstr::ifstream(cache_file_path.string(), str_flags);
    debug::check_expr(str.good());

    // Deserialize program cache from stream
    io::from_stream(m_prog_cache, str);

    // Output OpenGL debug message to warn of cache load
    debug::insert_message(
      fmt::format("Program cache loaded from: {}", cache_file_path.string()), 
      gl::DebugMessageSeverity::eLow);
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