#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/utility.hpp>
#include <initializer_list>
#include <filesystem>
#include <format>
#include <string>
#include <span>
#include <unordered_map>
#include <variant>

namespace gl {  
  /**
   * Helper object to create program object with path to shader
   * object's file data; OpenGL's SPIR-V shader loading will be used
   */
  struct ShaderLoadSPIRVInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards SPIRV file, which will be loaded
    fs::path spirv_path;

    // Path towards SPIRV-Cross generated reflection json file
    fs::path cross_path;

    // Override SPIRV shader entry point, if necessary
    std::string entry_point = "main";

    // Pass in indexed SPIRV specialization constants
    std::vector<std::pair<uint, uint>> spec_const = { };

  public: // Helpers for use in std::unordered_map in gl::ProgramCache
    std::string to_string() const {
      std::stringstream ss;
      ss << std::format("{}_{}_{}_{}", 
                        static_cast<gl::uint>(type), 
                        spirv_path.string(), 
                        cross_path.string(), 
                        entry_point);
      for (const auto &[i, value] : spec_const)
        ss << std::format("_({},{})", i, value);
      return ss.str();
    }
  };

  /**
   * Helper object to create program object with path to shader
   * object's file data; OpenGL's GLSL shader loading will be used
   */
  struct ShaderLoadGLSLInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards GLSL file, which will be loaded
    fs::path glsl_path;

    // Path towards SPIRV-Cross generated reflection json file
    fs::path cross_path;

  public: // Helpers for use in std::unordered_map in gl::ProgramCache
    std::string to_string() const {
      return std::format("{}_{}_{}", 
                         static_cast<gl::uint>(type), 
                         glsl_path.string(), 
                         cross_path.string());
    }
  };

  /**
   * Helper object to create program object with shader object's byte
   * data provided; OpenGL's SPIR-V shader loading will be used
  */
  struct ShaderLoadSPIRVStringInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // SPIRV shader data in byte format
    std::vector<std::byte> spirv_data;

    // SPIRV-Cross generated reflection json data
    std::vector<io::json> cross_json;

    // Override SPIRV shader entry point, if necessary
    std::string entry_point = "main";

    // Pass in indexed SPIRV specialization constants
    std::vector<std::pair<uint, uint>> spec_const = { };
  };
  
  /**
   * Helper object to create program object with shader object's byte
   * data provided; OpenGL's GLSL shader loading will be used
  */
  struct ShaderLoadGLSLStringInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // GLSL shader data in byte format
    std::vector<std::byte> glsl_data;

    // SPIRV-Cross generated reflection json data
    std::vector<io::json> cross_json;
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
      eStorageBuffer, // SSBOs
      eUniformBuffer, // UBOs, not uniforms
      eUniform,       // Classical uniforms; not supported in the SPIR-V pipeline
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
    int  loc(std::string_view s);      // Look up classic uniform location for given string name

  public:
    /* constr/destr */

    Program() = default;
    ~Program();
    
    Program(ShaderLoadSPIRVInfo);
    Program(ShaderLoadGLSLInfo);
    Program(ShaderLoadSPIRVStringInfo);
    Program(ShaderLoadGLSLStringInfo);

    Program(std::initializer_list<ShaderLoadSPIRVInfo>);
    Program(std::initializer_list<ShaderLoadGLSLInfo>);
    Program(std::initializer_list<ShaderLoadSPIRVStringInfo>);
    Program(std::initializer_list<ShaderLoadGLSLStringInfo>);

    /* state */  

    template <typename T>
    void uniform(std::string_view s, const T &t);

    // Bind specific object to a name; on BindingType::eAuto, populated object type is used
    void bind(std::string_view s, const gl::AbstractTexture &, BindingType binding = BindingType::eAuto);
    void bind(std::string_view s, const gl::Sampler         &, BindingType binding = BindingType::eAuto);
    void bind(std::string_view s, const gl::Buffer          &, 
              size_t size = 0, size_t offset = 0, BindingType binding = BindingType::eAuto);

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
  	
  /**
   * Helper object to initialize, store and load program objects
   * based on their construction objects, to avoid unnecessary
   * recreation of some heavier program objects.
   */
  class ProgramCache {
    using KeyType  = std::string;
    using InfoType = std::variant<ShaderLoadSPIRVInfo, ShaderLoadGLSLInfo>;
    
    std::unordered_map<KeyType, InfoType> m_info_cache;
    std::unordered_map<KeyType, Program>  m_prog_cache;

    // Initialize-and-return a reference to a program object
    // Overloaded below for variant types to support 
    // aggregate initialization from the front-end
    std::pair<KeyType, gl::Program&> set(InfoType info); 

  public:
    // Return an existing program for a given key
    gl::Program& at(const KeyType &k);
    
    // Reload and rebuild all cached programs
    void reload();

    // Clear out program cache
    void clear();

  public:
    // Forward to private variant constructor
    std::pair<KeyType, gl::Program&> set(ShaderLoadSPIRVInfo info) {
      return set(InfoType(std::move(info)));
    }
    
    // Forward to private variant constructor
    std::pair<KeyType, gl::Program&> set(ShaderLoadGLSLInfo info) {
      return set(InfoType(std::move(info)));
    }
  };
} // namespace gl