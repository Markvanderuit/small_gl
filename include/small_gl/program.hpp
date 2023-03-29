#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/utility.hpp>
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
    std::vector<std::byte> data;

    // SPIRV-Cross generated reflection json files
    std::vector<io::json> cross_json;

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

    // Internal enum used for reflectance data
    enum class BindingType {
      eAuto,          // Default for program.bind(...), defers to lookup table
      eImage,
      eSampler,       // Textures/samplers share name/binding
      eShaderStorage,
      eUniform,       // Classical uniforms; not supported in the SPIR-V pipeline
      eUniformBuffer, // UBOs, not uniforms
    };

    // Internal enum used for reflectance data
    enum class BindingAccess { eReadOnly, eWriteOnly, eReadWrite };

    // Internal struct used for reflectance data
    struct BindingData {
      BindingType   type    = BindingType::eAuto;
      BindingAccess access  = BindingAccess::eReadWrite;
      int           binding = -1;

      auto operator<=>(const BindingData&) const = default;
    };
    
    // Map populated with object locations for reflectable string names, if available
    std::unordered_map<std::string, BindingData> m_binding_data;

    // Populate some reflectance data
    void populate(fs::path refl_path); // Populate reflectance data from SPIRV-CROSS generated .json file
    void populate(io::json refl_json); // Populate reflectance data from SPIRV-CROSS generated .json data
    int loc(std::string_view s);       // Look up classic uniform location for given string name

    Program(ShaderCreateInfo);
    Program(std::initializer_list<ShaderCreateInfo>);
    
  public:
    /* constr/destr */

    Program() = default;
    ~Program();
    
    Program(ShaderLoadSPIRVInfo);
    Program(std::initializer_list<ShaderLoadSPIRVInfo>);

    Program(ShaderLoadInfo);
    Program(std::initializer_list<ShaderLoadInfo>);

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
    
    inline void swap(Program &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_binding_data, o.m_binding_data);
    }

    inline bool operator==(const Program &o) const {
      return Base::operator==(o) 
        && m_binding_data == m_binding_data;
    }

    gl_declare_noncopyable(Program);
  };
} // namespace gl