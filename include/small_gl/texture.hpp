#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/texture.hpp>
#include <small_gl/detail/eigen.hpp>
#include <span>

namespace gl {
  /**
   * Helper object to create texture object.
   */
  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  class TextureCreateInfo {
    using Array = Eigen::Array<int, detail::texture_dims<D, Ty>(), 1>;

  public:
    Array size;
    uint levels = 1;
    std::span<T> data = { };
  };

  /* Abstract intermediate for storing pointer to any specific texture */
  struct AbstractTexture : public detail::Handle<> {
  protected:
    constexpr AbstractTexture() = default;
    constexpr AbstractTexture(bool init) noexcept : detail::Handle<>(init) { }
    constexpr virtual ~AbstractTexture() = default;
  };

  /* Tag objects designating special depth/stencil types for a texture's value type. */
  struct DepthComponent { };
  struct StencilComponent { };

  /**
   * Texture object wrapping OpenGL texture object.
   * 
   * Supports 1d/2d/3d textures, 1d/2d texture arrays, 2d cubemaps, 2d cubemap arrays, 
   * 2d multisampled textures, and 2d multisampled arrays. A large collection of template 
   * specializations is accessible through names declared in 'small_gl/detail/fwd.hpp'.
   */
  template <typename T,       // Underlying pixel type (float, int, uint, DepthComponent, ...)
            uint D,           // Nr. of dimensions (1, 2, 3)
            uint Components,  // Nr. of pixel components (1, 2, 3, 4)
            TextureType Ty>   // Special texture type (array, cubemap, multisampled)
  class Texture : public AbstractTexture {
    using Array = Eigen::Array<int, detail::texture_dims<D, Ty>(), 1>;
    using TextureCreateInfo = TextureCreateInfo<T, D, Ty>;

  public:
    /* constr/destr */
    
    Texture() = default;
    Texture(TextureCreateInfo info);
    ~Texture();

    /* getters/setters */

    inline uint levels() const { return _levels; }
    inline Array size() const { return _size; }

    /* state */

    void bind_to(uint index) const;

    /* operands for most texture types */

    void get(std::span<T> data,
             uint level = 0,
             Array size = Array::Zero(),
             Array offset = Array::Zero()) const
             requires(!detail::is_cubemap_type<Ty>);

    void set(std::span<const T> data,
             uint level = 0,
             Array size = Array::Zero(),
             Array offset = Array::Zero()) 
             requires(!detail::is_cubemap_type<Ty>);

    void clear(std::span<const T> data = { }, 
               uint level = 0,
               Array size = Array::Zero(),
               Array offset = Array::Zero()) 
               requires(!detail::is_cubemap_type<Ty>);

    /* operands for cubemap texture types */

    void get(std::span<T> data,
             uint face = 0,
             uint level = 0,
             Array size = Array::Zero(),
             Array offset = Array::Zero()) const
             requires(detail::is_cubemap_type<Ty>);
    
    void set(std::span<const T> data,
             uint face = 0,
             uint level = 0,
             Array size = Array::Zero(),
             Array offset = Array::Zero()) 
             requires(detail::is_cubemap_type<Ty>);

    void clear(std::span<const T> data = { }, 
               uint face = 0,
               uint level = 0,
               Array size = Array::Zero(),
               Array offset = Array::Zero()) 
               requires(detail::is_cubemap_type<Ty>);

    /* miscellaneous */

    void generate_mipmaps();

  private:
    using Base = AbstractTexture;

    uint _levels;
    Array _size;

  public:
    inline void swap(Texture &o) {
      using std::swap;
      Base::swap(o);
      swap(_levels, o._levels);
      swap(_size, o._size);
    }

    inline bool operator==(const Texture &o) const {
      return Base::operator==(o) && _levels == o._levels && _size.isApprox(o._size);
    }

    gl_declare_noncopyable(Texture)
  };
} // namespace gl