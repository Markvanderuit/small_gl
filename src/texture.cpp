#include <small_gl/texture.hpp>
#include <small_gl/exception.hpp>

namespace gl {
  template <typename T, uint D, uint Components, TextureType Ty>
  Texture<T, D, Components, Ty>::Texture(TextureCreateInfo info)
  : Base(true), _size(info.size), _levels(info.levels) {
    expr_check((_size >= 1).all(), "texture size must be all >= 1");
    expr_check(_levels >= 1, "texture level must be >= 1");

    glCreateTextures(detail::texture_target<D, Ty>(), 1, &_object);

    constexpr auto internal_format = detail::texture_internal_format<Components, T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    
    if constexpr (storage_type == detail::StorageType::e1D) {
      glTextureStorage1D(_object, _levels, internal_format, _size.x());
    } else if constexpr (storage_type == detail::StorageType::e2D) {
      glTextureStorage2D(_object, _levels, internal_format, _size.x(), _size.y());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      GLsizei size_z = detail::is_cubemap_type<Ty> ? _size.z() * 6 : _size.z(); // array cubemap
      glTextureStorage3D(_object, _levels, internal_format, _size.x(), _size.y(), size_z);
    } else if constexpr (storage_type == detail::StorageType::e2DMSAA) {
      glTextureStorage2DMultisample(_object, 4, internal_format, _size.x(), _size.y(), true);
    } else if constexpr (storage_type == detail::StorageType::e3DMSAA) {
      glTextureStorage3DMultisample(_object, 4, internal_format, _size.x(), _size.y(), _size.z(), true);
    }
    gl_check();

    // If prior data was provided, process this or return early
    guard(info.data.data());
    set(info.data);
    generate_mipmaps();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  Texture<T, D, Components, Ty>::~Texture() {
    guard(_is_init);
    glDeleteTextures(1, &_object);
    gl_check();
  }

  /* operands for most texture types follow */

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::get(std::span<T> data, uint level, Array size, Array offset) const
  requires(!detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

    const size_t size_bytes = (safe_size - offset).prod() * Components * pixel_size;
    expr_check(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be written");

    if constexpr (storage_type == detail::StorageType::e1D) {
      glGetTextureSubImage(_object, level, 
        offset.x(), 0, 0, 
        safe_size.x(), 1, 1, 
        format, pixel_format, data.size_bytes(), data.data());
    } else if constexpr (storage_type == detail::StorageType::e2D
                      || storage_type == detail::StorageType::e2DMSAA) {
      glGetTextureSubImage(_object, level, 
        offset.x(), offset.y(), 0, 
        safe_size.x(), safe_size.y(), 1, 
        format, pixel_format, data.size_bytes(), data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D
                      || storage_type == detail::StorageType::e3DMSAA) {
      glGetTextureSubImage(_object, level, 
        offset.x(), offset.y(), offset.z(), 
        safe_size.x(), safe_size.y(), safe_size.z(), 
        format, pixel_format, data.size_bytes(), data.data());
    }
    
    gl_check();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::set(std::span<const T> data, uint level, Array size, Array offset)
  requires(!detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

    const size_t size_bytes = (safe_size - offset).prod() * Components * pixel_size;
    expr_check(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be read");

    if constexpr (storage_type == detail::StorageType::e1D) {
      glTextureSubImage1D(_object, level, 
        offset.x(), 
        safe_size.x(), 
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e2D
                      || storage_type == detail::StorageType::e2DMSAA) {
      glTextureSubImage2D(_object, level, 
        offset.x(), offset.y(), 
        safe_size.x(), safe_size.y(), 
        format, pixel_format, data.data());              
    } else if constexpr (storage_type == detail::StorageType::e3D
                      || storage_type == detail::StorageType::e3DMSAA) {
      glTextureSubImage3D(_object, level, 
        offset.x(), offset.y(), offset.z(),
        safe_size.x(), safe_size.y(), safe_size.z(),
        format, pixel_format, data.data());
    }
    
    gl_check();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::clear(std::span<const T> data, uint level, Array size, Array offset)
  requires(!detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

    const size_t size_bytes = Components * pixel_size;
    expr_check(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture to be cleared");

    if constexpr (storage_type == detail::StorageType::e1D) {
      glClearTexSubImage(_object, level, 
        offset.x(), 0, 0, 
        safe_size.x(), 1, 1, 
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e2D
                      || storage_type == detail::StorageType::e2DMSAA) {
      glClearTexSubImage(_object, level, 
        offset.x(), offset.y(), 0, 
        safe_size.x(), safe_size.y(), 1, 
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D
                      || storage_type == detail::StorageType::e3DMSAA) {
      glClearTexSubImage(_object, level, 
        offset.x(), offset.y(), offset.z(), 
        safe_size.x(), safe_size.y(), safe_size.z(), 
        format, pixel_format, data.data());
    }
    
    gl_check();
  }

  /* operands for cubemap texture types follow */

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::get(std::span<T> data, uint face, uint level, Array size, Array offset) const 
  requires(detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

    const size_t size_bytes = (safe_size - offset).prod() * Components * pixel_size;
    expr_check(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be written");
    
    if constexpr (storage_type == detail::StorageType::e2D) {
      glGetTextureSubImage(_object, level, 
        offset.x(), offset.y(), face, 
        safe_size.x(), safe_size.y(), 1, 
        format, pixel_format, data.size_bytes(), data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      glGetTextureSubImage(_object, level, 
        offset.x(), offset.y(), offset.z() * face, 
        safe_size.x(), safe_size.y(), safe_size.z(), 
        format, pixel_format, data.size_bytes(), data.data());
    }

    gl_check();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::set(std::span<const T> data, uint face, uint level, Array size, Array offset)
  requires(detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

    const size_t size_bytes = (safe_size - offset).prod() * Components * pixel_size;
    expr_check(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be read");

    if constexpr (storage_type == detail::StorageType::e2D) {
      glTextureSubImage3D(_object, level, 
      offset.x(), offset.y(), face, 
      safe_size.x(), safe_size.y(), 1, 
      format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      glTextureSubImage3D(_object, level, 
      offset.x(), offset.y(), safe_size.z() * face, 
      safe_size.x(), safe_size.y(), safe_size.z(), 
      format, pixel_format, data.data());
    }

    gl_check();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::clear(std::span<const T> data, uint face, uint level, Array size, Array offset)
  requires(detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

    const size_t size_bytes = Components * pixel_size;
    expr_check(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture to be cleared");
    
    if constexpr (storage_type == detail::StorageType::e2D) {
      glClearTexSubImage(_object, level,
        offset.x(), offset.y(), face,
        safe_size.x(), safe_size.y(), 1,
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      glClearTexSubImage(_object, level,
        offset.x(), offset.y(), offset.z() * face,
        safe_size.x(), safe_size.y(), safe_size.z(),
        format, pixel_format, data.data());
    }

    gl_check();
  }
  
  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::bind_to(uint index) const {
    glBindTextureUnit(index, _object);
    gl_check();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::generate_mipmaps() {
    guard(_levels > 1);
    glGenerateTextureMipmap(_object);
    gl_check();
  }

  /* Explicit template instantiations of gl::Texture<...> */
  
  #define gl_explicit_texture(type, dims, components, texture_type)\
    template class Texture<type, dims, components, texture_type>;

  #define gl_explicit_texture_components_123(type, dims, texture_type)\
    gl_explicit_texture(type, dims, 1, texture_type)\
    gl_explicit_texture(type, dims, 2, texture_type)\
    gl_explicit_texture(type, dims, 3, texture_type)

  #define gl_explicit_texture_components_1234(type, dims, texture_type)\
    gl_explicit_texture(type, dims, 1, texture_type)\
    gl_explicit_texture(type, dims, 2, texture_type)\
    gl_explicit_texture(type, dims, 3, texture_type)\
    gl_explicit_texture(type, dims, 4, texture_type)

  // Image texture explicit template instantiations (1/2/3d, rgba, all pixel types)
  #define gl_explicit_texture_dims_123(type, texture_type)\
    gl_explicit_texture_components_1234(type, 1, texture_type)\
    gl_explicit_texture_components_1234(type, 2, texture_type)\
    gl_explicit_texture_components_1234(type, 3, texture_type)
  gl_explicit_texture_dims_123(ushort, TextureType::eImage)
  gl_explicit_texture_dims_123(short, TextureType::eImage)
  gl_explicit_texture_dims_123(uint, TextureType::eImage)
  gl_explicit_texture_dims_123(int, TextureType::eImage)
  gl_explicit_texture_dims_123(float, TextureType::eImage)

  // Image array texture explicit template instantiations (1/2d, rgba, all pixel types)
  #define gl_explicit_texture_dims_12(type, texture_type)\
    gl_explicit_texture_components_1234(type, 1, texture_type)\
    gl_explicit_texture_components_1234(type, 2, texture_type)
  gl_explicit_texture_dims_12(ushort, TextureType::eImageArray)
  gl_explicit_texture_dims_12(short, TextureType::eImageArray)
  gl_explicit_texture_dims_12(uint, TextureType::eImageArray)
  gl_explicit_texture_dims_12(int, TextureType::eImageArray)
  gl_explicit_texture_dims_12(float, TextureType::eImageArray)

  // Cubemap/MSAA texture explicit template instantiations (2d only, rgba, all pixel types)
  #define gl_explicit_texture_2d_rgba_all_types(texture_type)\
    gl_explicit_texture_components_1234(ushort, 2, texture_type)\
    gl_explicit_texture_components_1234(short, 2, texture_type)\
    gl_explicit_texture_components_1234(uint, 2, texture_type)\
    gl_explicit_texture_components_1234(int, 2, texture_type)\
    gl_explicit_texture_components_1234(float, 2, texture_type)
  gl_explicit_texture_2d_rgba_all_types(TextureType::eCubemap)
  gl_explicit_texture_2d_rgba_all_types(TextureType::eCubemapArray)
  gl_explicit_texture_2d_rgba_all_types(TextureType::eMultisample)
  gl_explicit_texture_2d_rgba_all_types(TextureType::eMultisampleArray)

  // Depth texture explicit template instantiations (1/2/3d/array/cubemap/msaa, r, depth component)
  gl_explicit_texture(gl::DepthComponent, 1, 1, TextureType::eImage)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eImage)
  gl_explicit_texture(gl::DepthComponent, 3, 1, TextureType::eImage)
  gl_explicit_texture(gl::DepthComponent, 1, 1, TextureType::eImageArray)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eImageArray)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eCubemap)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eCubemapArray)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eMultisample)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eMultisampleArray)

  // Stemcil texture explicit template instantiations (1/2/3d/array/cubemap/msaa, r, stencil component)
  gl_explicit_texture(gl::StencilComponent, 1, 1, TextureType::eImage)
  gl_explicit_texture(gl::StencilComponent, 2, 1, TextureType::eImage)
  gl_explicit_texture(gl::StencilComponent, 3, 1, TextureType::eImage)
  gl_explicit_texture(gl::StencilComponent, 1, 1, TextureType::eImageArray)
  gl_explicit_texture(gl::StencilComponent, 2, 1, TextureType::eImageArray)
  gl_explicit_texture(gl::StencilComponent, 2, 1, TextureType::eCubemap)
  gl_explicit_texture(gl::StencilComponent, 2, 1, TextureType::eCubemapArray)
  gl_explicit_texture(gl::StencilComponent, 2, 1, TextureType::eMultisample)
  gl_explicit_texture(gl::StencilComponent, 2, 1, TextureType::eMultisampleArray)
} // namespace gl