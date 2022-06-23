#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <initializer_list>
#include <filesystem>
#include <string>
#include <span>
#include <unordered_map>

namespace gl {  
  /**
   * Helper object to create program object with path to shader
   * object's file data.
   */
  struct ShaderLoadInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards shader file, which will be loaded
    std::filesystem::path path;

    // Is the attached shader data a spirv binary?
    bool is_spirv_binary = true;

    // Override spirv shader entry point, if necessary
    std::string spirv_entry_point = "main";
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

    // Is the attached shader data a spirv binary?
    bool is_spirv_binary = true;

    // Override spirv shader entry point, if necessary
    std::string spirv_entry_point = "main";
  };

  struct ShaderIncludeLoadInfo {
    // Path towards shader file, which will be loaded
    // and used as its registered name
    std::filesystem::path path;
  };

  struct ShaderIncludeCreateInfo {
    // Registered name of shader include
    std::string name;

    // Shader data in binary format
    std::span<const std::byte> data;
  };
  

  /**
   * Program object wrapping OpenGL shader program object.
   */
  class Program : public detail::Handle<> {
    using Base = detail::Handle<>;
    
    // Unordered map caches uniform locations for uniform string names
    std::unordered_map<std::string, int> m_loc;

    // Look up uniform location for uniform string name
    int loc(std::string_view s);
    
  public:
    /* constr/destr */

    Program() = default;
    Program(ShaderLoadInfo);
    Program(std::initializer_list<ShaderLoadInfo>);
    Program(ShaderCreateInfo);
    Program(std::initializer_list<ShaderCreateInfo>);
    ~Program();

    /* state */  

    template <typename T>
    void uniform(std::string_view s, T t);

    void bind() const;
    void unbind() const;

    /* miscellaneous */

    static void add_include(std::initializer_list<ShaderIncludeLoadInfo>);
    static void add_include(std::initializer_list<ShaderIncludeCreateInfo>);
    
    inline void swap(Program &o) {
      using std::swap;
      Base::swap(o);
      swap(m_loc, o.m_loc);
    }

    inline bool operator==(const Program &o) const {
      return Base::operator==(o) && m_loc == o.m_loc;
    }

    gl_declare_noncopyable(Program);
  };
} // namespace gl