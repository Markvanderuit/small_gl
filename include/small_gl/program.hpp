#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/utility.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl_parser/fwd.hpp>
#include <initializer_list>
#include <filesystem>
#include <string>
#include <span>
#include <unordered_map>

namespace gl {  
  /**
   * Helper object to create program object with path to shader
   * object's file data; SPIR-V shader loading will be used
   */
  struct ShaderLoadSPIRVInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards SPIRV file, which will be loaded
    fs::path spirv_path;

    // Path towards SPIRV-Cross generated reflection json file
    fs::path cross_path;

    // Override spirv shader entry point, if necessary
    std::string entry_point = "main";
  };

  /**
   * Helper object to create program object with path to shader
   * object's file data; OpenGL shader loading will be used
   */
  struct ShaderLoadOGLInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards GLSL file, which will be loaded
    fs::path glsl_path;

    // Path towards SPIRV-Cross generated reflection json file
    fs::path cross_path;
  };

  /**
   * Helper object to create program object with path to shader
   * object's file data.
   */
  struct ShaderLoadInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards shader file, which will be loaded
    fs::path path;

    // Path towards SPIRV-Cross generated reflection json file
    fs::path cross_path;

    // Is the attached shader data a spir-v binary?
    bool is_spirv = false;

    // Override spirv shader entry point, if necessary
    std::string spirv_entry_point = "main";

    // Optional parser; will process shader first
    glp::Parser *parser = nullptr;
  };

  /**
   * Helper object to create program object with pre-loaded
   * shader object file data.
   */
  struct ShaderCreateInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Shader data in binary format
    std::span<const std::byte> data;

    // Is the attached shader data a spir-v binary?
    bool is_spirv = false;

    // Override spirv shader entry point, if necessary
    std::string spirv_entry_point = "main";

    // Optional parser; will process shader first
    glp::Parser *parser = nullptr;
  };

  /**
   * Program object wrapping OpenGL shader program object.
   */
  class Program : public detail::Handle<> {
    using Base = detail::Handle<>;

    enum class BindingType {
      eAuto,                // Default for program.bind(...), defers to lookup table
      eImage,
      eSampler,             // Textures/samplers share name/binding
      eShaderStorageBuffer,
      eUniform,             // Unsupported in SPIR-V pipeline; use UBOs instead
      eUniformBuffer,
    };

    struct BindingData {
      BindingType type = BindingType::eAuto;
      int         indx = -1;
    };
    
    // Maps populate with object locations for reflectable string names, if available
    std::unordered_map<std::string, int>         m_locations_uniform;
    std::unordered_map<std::string, BindingData> m_locations_data;

    // Look up uniform location for uniform string name
    int loc(std::string_view s);
    
  public:
    /* constr/destr */

    Program() = default;
    ~Program();

    Program(ShaderLoadInfo);
    Program(std::initializer_list<ShaderLoadInfo>);

    Program(ShaderCreateInfo);
    Program(std::initializer_list<ShaderCreateInfo>);

    /* state */  

    template <typename T>
    void uniform(std::string_view s, const T &t);

    // Bind specific object to a name; on BindingType::eAuto, populated object type is used
    void bind(std::string_view s, const gl::AbstractTexture &, BindingType binding = BindingType::eAuto);
    void bind(std::string_view s, const gl::Buffer          &, BindingType binding = BindingType::eAuto);
    void bind(std::string_view s, const gl::Sampler         &, BindingType binding = BindingType::eAuto);

    void bind() const;
    void unbind() const;
    static void unbind_all();

    /* miscellaneous */

    // Populate reflectance data from SPIRV-Cross generated .json file
    void populate(fs::path refl_path);
    void populate(io::json refl_json);
    
    inline void swap(Program &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_locations_uniform, o.m_locations_uniform);
    }

    inline bool operator==(const Program &o) const {
      return Base::operator==(o) && m_locations_uniform == o.m_locations_uniform;
    }

    gl_declare_noncopyable(Program);
  };
} // namespace gl