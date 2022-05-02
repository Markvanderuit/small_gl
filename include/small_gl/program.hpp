#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/utility.hpp>
#include <initializer_list>
#include <string>
#include <span>
#include <unordered_map>

namespace gl {
  /**
   * Helper object to create program object with path to shader
   * object's file data.
   */
  struct ShaderLoadInfo {
    ShaderType type;
    fs::path path;

    bool is_spirv_binary = true;
    std::string spirv_entry_point = "main";
  };

  /**
   * Helper object to create program object with pre-loaded
   * shader object file data.
   */
  struct ShaderCreateInfo {
    ShaderType type;
    std::span<const std::byte> data;

    bool is_spirv_binary = true;
    std::string spirv_entry_point = "main";
  };

  /**
   * Program object wrapping OpenGL shader program object.
   */
  struct Program : public detail::Handle<> {
    /* constr/destr */

    Program() = default;
    Program(std::initializer_list<ShaderLoadInfo>);
    Program(std::initializer_list<ShaderCreateInfo>);
    Program(ShaderLoadInfo);
    Program(ShaderCreateInfo);
    ~Program();

    /* state */  

    template <typename T>
    void uniform(std::string_view s, T t);
    void bind() const;
    void unbind() const;

  private:
    using Base = detail::Handle<>;
    
    // Unordered map caches uniform locations matching string values
    int loc(std::string_view s);
    std::unordered_map<std::string, int> _loc;

  public:
    inline void swap(Program &o) {
      using std::swap;
      Base::swap(o);
      swap(_loc, o._loc);
    }

    inline bool operator==(const Program &o) const {
      return Base::operator==(o) && _loc == o._loc;
    }

    gl_declare_noncopyable(Program);
  };

} // namespace gl