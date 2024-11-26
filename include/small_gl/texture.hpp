#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/enum.hpp>
#include <small_gl/utility.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/texture.hpp>
#include <span>

namespace gl {
  /**
   * Helper object to create texture object.
   */
  template <typename T,     // Underlying pixel type (float, int, uint, DepthComponent, ...)
            uint D,         // Nr. of dimensions (1, 2, 3)
            TextureType Ty  // Special texture type (array, cubemap, multisampled)
            = TextureType::eImage>
  class TextureInfo {
    using vect = eig::Array<uint, detail::texture_dims<D, Ty>(), 1>;

  public:
    // Multi-dimensional size of the texture; not in bytes
    vect size;

    // Mipmap levels; 1 = no mipmap
    uint levels = 1;

    // Non-owning span to data passed into buffer
    std::span<const T> data = { };
  };

  /**
   * Texture object wrapping OpenGL texture object.
   * 
   * Supports 1d/2d/3d textures, 1d/2d texture arrays, 2d cubemaps, 2d cubemap arrays, 
   * 2d multisampled textures, and 2d multisampled arrays..
   */
  template <typename T,       // Underlying components type (float, int, uint, DepthComponent, ...)
            uint D,           // Nr. of dimensions (1, 2, 3)
            uint C,           // Nr. of components (1, 2, 3, 4)
            TextureType Ty    // Special texture type (array, cubemap, multisampled)
            = TextureType::eImage>   
  class Texture : public AbstractTexture {
    using Base = detail::Handle<>;
    using vect = eig::Array<uint, detail::texture_dims<D, Ty>(), 1>;

    uint m_levels;
    vect m_size;

  public:
    using InfoType = TextureInfo<T, D, Ty>;

    /* constr/destr */
    
    Texture() = default;
    Texture(InfoType info);
    ~Texture();

    /* getters */

    uint layers() const override {
      if constexpr (D == 3)
        return m_size.z();
      else
        return 0;
    }

    uint levels() const override { 
      return m_levels; 
    }

    vect size() const { 
      return m_size; 
    }

    /* state */

    void bind_to(TextureTargetType target, uint index, uint level = 0) const override;

    /* operands for most texture types */

    void get(std::span<T> data,
             uint level                  = 0,
             vect size                   = vect(0),
             vect offset                 = vect(0)) const
             requires(!detail::is_cubemap_type<Ty>);

    void set(const gl::Buffer &data,
             uint level                  = 0,
             vect size                   = vect(0),
             vect offset                 = vect(0)) 
             requires(!detail::is_cubemap_type<Ty>);

    void set(std::span<const T> data,
             uint level                  = 0,
             vect size                   = vect(0),
             vect offset                 = vect(0)) 
             requires(!detail::is_cubemap_type<Ty>);

    void clear(std::span<const T> data     = { }, 
               uint level                  = 0,
               vect size                   = vect(0),
               vect offset                 = vect(0)) 
               requires(!detail::is_cubemap_type<Ty>);

    void copy_to(AbstractTexture &dst,
                 uint level                  = 0,
                 vect size                   = vect(0),
                 vect src_offset             = vect(0),
                 vect dst_offset             = vect(0)) const
                 requires(!detail::is_cubemap_type<Ty>);

    /* operands for cubemap texture types */

    void get(std::span<T> data,
             uint face                  = 0,
             uint level                 = 0,
             vect size                  = vect(0),
             vect offset                = vect(0)) const
             requires(detail::is_cubemap_type<Ty>);
    
    void set(std::span<const T> data,
             uint face                  = 0,
             uint level                 = 0,
             vect size                  = vect(0),
             vect offset                = vect(0)) 
             requires(detail::is_cubemap_type<Ty>);

    void clear(std::span<const T> data    = { }, 
               uint face                  = 0,
               uint level                 = 0,
               vect size                  = vect(0),
               vect offset                = vect(0)) 
               requires(detail::is_cubemap_type<Ty>);

    void copy_to(AbstractTexture &dst,
                 uint face                   = 0,
                 uint level                  = 0,
                 vect size                   = vect(0),
                 vect src_offset             = vect(0),
                 vect dst_offset             = vect(0)) const
                 requires(detail::is_cubemap_type<Ty>);

    /* miscellaneous */

    // Generate mipmaps if levels > 1
    void generate_mipmaps() override;

    // Format queries
    uint internal_format() const override { return detail::texture_internal_format<C, T>(); }
    uint format()          const override { return detail::texture_format<C, T>();          }
    uint target()          const override { return detail::texture_target<D, Ty>();         }
    
    inline void swap(Texture &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_levels, o.m_levels);
      swap(m_size, o.m_size);
    }

    inline bool operator==(const Texture &o) const {
      return Base::operator==(o) 
        && m_levels == o.m_levels 
        && (m_size == o.m_size).all();
    }

    gl_declare_noncopyable(Texture);
  };

  /**
   * Helper object to create texture view object.
   */
  struct TextureViewInfo {
    // Object handle to viewed underlying texture
    const AbstractTexture *texture;

    // Range of mip levels included in the texture view
    uint levels    = 1;
    uint min_level = 0;

    // Range of array layers included in the texture view
    uint layers    = 1;
    uint min_layer = 0;
  };

  /**
   * Texture view object wrapping OpenGL view textures
   * 
   * Note; conversions are not currently verified, so OpenGL will complain
   * if an incompatible view is created over a certain texture. 
   */
  template <typename T,        // Underlying components type (float, int, uint, DepthComponent, ...)
            uint D,            // Nr. of dimensions (1, 2, 3)
            uint C,            // Nr. of components (1, 2, 3, 4)
            TextureType Ty     // Special texture type (array, cubemap, multisampled)
            = TextureType::eImage>
  class TextureView : public AbstractTexture {
    using Base = detail::Handle<>;

    uint m_levels;

  public:
    using InfoType = TextureViewInfo;

    /* constr/destr */
    
    TextureView() = default;
    TextureView(TextureViewInfo info);
    ~TextureView();

    /* state */

    void bind_to(TextureTargetType target, uint index, uint level = 0) const override;

    /* getters */

    uint layers() const override {
      /* if constexpr (D == 3)
        return m_size.z();
      else */
      return 0; // Not properly handled r.n.
    }

    uint levels() const override {
      return m_levels;
    }

    /* miscellaneous */

    void generate_mipmaps() override { /*  */ }

    // Format queries
    uint internal_format() const override { return detail::texture_internal_format<C, T>(); }
    uint format()          const override { return detail::texture_format<C, T>();          }
    uint target()          const override { return detail::texture_target<D, Ty>();         }

    inline void swap(TextureView &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_levels, o.m_levels);
    }

    inline bool operator==(const TextureView &o) const {
      return Base::operator==(o) && m_levels == o.m_levels;
    }

    gl_declare_noncopyable(TextureView);
  };

  /* Shorthand notations for common texture types follow */

  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  using Texture1d = Texture<T, 1, D, Ty>;
  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  using Texture2d = Texture<T, 2, D, Ty>;
  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  using Texture3d = Texture<T, 3, D, Ty>;

  using Texture1d1f = Texture1d<float, 1>;
  using Texture1d2f = Texture1d<float, 2>;
  using Texture1d3f = Texture1d<float, 3>;
  using Texture1d4f = Texture1d<float, 4>;
  using Texture2d1f = Texture2d<float, 1>;
  using Texture2d2f = Texture2d<float, 2>;
  using Texture2d3f = Texture2d<float, 3>;
  using Texture2d4f = Texture2d<float, 4>;
  using Texture3d1f = Texture3d<float, 1>;
  using Texture3d2f = Texture3d<float, 2>;
  using Texture3d3f = Texture3d<float, 3>;
  using Texture3d4f = Texture3d<float, 4>;
  
  using Texture1d1i = Texture1d<int, 1>;
  using Texture1d2i = Texture1d<int, 2>;
  using Texture1d3i = Texture1d<int, 3>;
  using Texture1d4i = Texture1d<int, 4>;
  using Texture2d1i = Texture2d<int, 1>;
  using Texture2d2i = Texture2d<int, 2>;
  using Texture2d3i = Texture2d<int, 3>;
  using Texture2d4i = Texture2d<int, 4>;
  using Texture3d1i = Texture3d<int, 1>;
  using Texture3d2i = Texture3d<int, 2>;
  using Texture3d3i = Texture3d<int, 3>;
  using Texture3d4i = Texture3d<int, 4>;
  
  using Texture1d1ui = Texture1d<uint, 1>;
  using Texture1d2ui = Texture1d<uint, 2>;
  using Texture1d3ui = Texture1d<uint, 3>;
  using Texture1d4ui = Texture1d<uint, 4>;
  using Texture2d1ui = Texture2d<uint, 1>;
  using Texture2d2ui = Texture2d<uint, 2>;
  using Texture2d3ui = Texture2d<uint, 3>;
  using Texture2d4ui = Texture2d<uint, 4>;
  using Texture3d1ui = Texture3d<uint, 1>;
  using Texture3d2ui = Texture3d<uint, 2>;
  using Texture3d3ui = Texture3d<uint, 3>;
  using Texture3d4ui = Texture3d<uint, 4>;
  
  using Texture1d1s = Texture1d<short, 1>;
  using Texture1d2s = Texture1d<short, 2>;
  using Texture1d3s = Texture1d<short, 3>;
  using Texture1d4s = Texture1d<short, 4>;
  using Texture2d1s = Texture2d<short, 1>;
  using Texture2d2s = Texture2d<short, 2>;
  using Texture2d3s = Texture2d<short, 3>;
  using Texture2d4s = Texture2d<short, 4>;
  using Texture3d1s = Texture3d<short, 1>;
  using Texture3d2s = Texture3d<short, 2>;
  using Texture3d3s = Texture3d<short, 3>;
  using Texture3d4s = Texture3d<short, 4>;
  
  using Texture1d1us = Texture1d<ushort, 1>;
  using Texture1d2us = Texture1d<ushort, 2>;
  using Texture1d3us = Texture1d<ushort, 3>;
  using Texture1d4us = Texture1d<ushort, 4>;
  using Texture2d1us = Texture2d<ushort, 1>;
  using Texture2d2us = Texture2d<ushort, 2>;
  using Texture2d3us = Texture2d<ushort, 3>;
  using Texture2d4us = Texture2d<ushort, 4>;
  using Texture3d1us = Texture3d<ushort, 1>;
  using Texture3d2us = Texture3d<ushort, 2>;
  using Texture3d3us = Texture3d<ushort, 3>;
  using Texture3d4us = Texture3d<ushort, 4>;

  using Texture1dDepth = Texture1d<DepthComponent, 1>;
  using Texture2dDepth = Texture2d<DepthComponent, 1>;
  using Texture3dDepth = Texture3d<DepthComponent, 1>;

  using Texture1dStencil = Texture1d<StencilComponent, 1>;
  using Texture2dStencil = Texture2d<StencilComponent, 1>;
  using Texture3dStencil = Texture3d<StencilComponent, 1>;

  /* Shorthand notations for common array texture types follow */

  template <typename T, uint N, uint D> 
  using TextureArray = Texture<T, N, D, TextureType::eImageArray>;
  template <typename T, uint D>
  using TextureArray1d = TextureArray< T, 1, D>;
  template <typename T, uint D>
  using TextureArray2d = TextureArray< T, 2, D>;
  template <typename T, uint D>
  using TextureArray3d = TextureArray< T, 3, D>;

  using TextureArray1d1f = TextureArray1d<float, 1>;
  using TextureArray1d2f = TextureArray1d<float, 2>;
  using TextureArray1d3f = TextureArray1d<float, 3>;
  using TextureArray1d4f = TextureArray1d<float, 4>;
  using TextureArray2d1f = TextureArray2d<float, 1>;
  using TextureArray2d2f = TextureArray2d<float, 2>;
  using TextureArray2d3f = TextureArray2d<float, 3>;
  using TextureArray2d4f = TextureArray2d<float, 4>;
  using TextureArray3d1f = TextureArray3d<float, 1>;
  using TextureArray3d2f = TextureArray3d<float, 2>;
  using TextureArray3d3f = TextureArray3d<float, 3>;
  using TextureArray3d4f = TextureArray3d<float, 4>;

  /* Shorthand notations for common texture view types follow */

  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  using TextureView1d = TextureView<T, 1, D, Ty>;
  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  using TextureView2d = TextureView<T, 2, D, Ty>;
  template <typename T, uint D, TextureType Ty = TextureType::eImage>
  using TextureView3d = TextureView<T, 3, D, Ty>;

  using TextureView1d1f = TextureView1d<float, 1>;
  using TextureView1d2f = TextureView1d<float, 2>;
  using TextureView1d3f = TextureView1d<float, 3>;
  using TextureView1d4f = TextureView1d<float, 4>;
  using TextureView2d1f = TextureView2d<float, 1>;
  using TextureView2d2f = TextureView2d<float, 2>;
  using TextureView2d3f = TextureView2d<float, 3>;
  using TextureView2d4f = TextureView2d<float, 4>;
  using TextureView3d1f = TextureView3d<float, 1>;
  using TextureView3d2f = TextureView3d<float, 2>;
  using TextureView3d3f = TextureView3d<float, 3>;
  using TextureView3d4f = TextureView3d<float, 4>;
} // namespace gl