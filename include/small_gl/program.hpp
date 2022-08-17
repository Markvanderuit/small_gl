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
   * object's file data.
   */
  struct ShaderLoadInfo {
    // Shader type (vertex, fragment, compute, geometry, tessel...)
    ShaderType type;

    // Path towards shader file, which will be loaded
    fs::path path;

    // Is the attached shader data a spirv binary?
    bool is_spirv_binary = false;

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

    // Is the attached shader data a spirv binary?
    bool is_spirv_binary = false;

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
    void uniform(std::string_view s, const T &t);

    void bind() const;
    void unbind() const;
    static void unbind_all();

    /* miscellaneous */

   /*  static void add_include(std::initializer_list<ShaderIncludeLoadInfo>);
    static void add_include(std::initializer_list<ShaderIncludeCreateInfo>); */
    
    inline void swap(Program &o) {
      gl_trace();
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