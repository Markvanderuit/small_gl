#include <small_gl/texture.hpp>

namespace gl {
  template <typename T, uint D, uint C, TextureType Ty>
  Texture<T, D, C, Ty>::Texture(TextureCreateInfo info)
  : Base(true), m_size(info.size), m_levels(info.levels) {
    gl_trace_full();
    debug::check_expr((m_size >= vect(1)).all(), "texture size must be all >= 1");
    debug::check_expr(m_levels >= 1,  "texture level must be >= 1");

    glCreateTextures(detail::texture_target<D, Ty>(), 1, &m_object);
  
    constexpr auto internal_format = detail::texture_internal_format<C, T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    
    if constexpr (storage_type == detail::StorageType::e1D) {
      glTextureStorage1D(m_object, m_levels, internal_format, m_size.x());
    } else if constexpr (storage_type == detail::StorageType::e2D) {
      glTextureStorage2D(m_object, m_levels, internal_format, m_size.x(), m_size.y());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      GLsizei size_z = detail::is_cubemap_type<Ty> ? m_size.z() * 6 : m_size.z(); // vect cubemap
      glTextureStorage3D(m_object, m_levels, internal_format, m_size.x(), m_size.y(), size_z);
    } else if constexpr (storage_type == detail::StorageType::e2DMSAA) {
      glTextureStorage2DMultisample(m_object, 4, internal_format, m_size.x(), m_size.y(), true);
    } else if constexpr (storage_type == detail::StorageType::e3DMSAA) {
      glTextureStorage3DMultisample(m_object, 4, internal_format, m_size.x(), m_size.y(), m_size.z(), true);
    }

    // If prior data was provided, upload it
    if (info.data.data()) {
      set(info.data);
      generate_mipmaps();
    }
    
    // Estimate texture size in bytes
#ifdef GL_ENABLE_TRACY
    size_t alloc_size = m_size.prod() * C * detail::texture_pixel_size_bytes<T>();
    for (size_t lvl_alloc_size = alloc_size, i = 1; 1 < static_cast<size_t>(m_levels); ++i) {
      lvl_alloc_size /= 2;
      alloc_size += lvl_alloc_size;
    }
#endif // GL_ENABLE_TRACY
    gl_trace_gpu_alloc("gl::Texture", object(), alloc_size);
  }

  template <typename T, uint D, uint C, TextureType Ty>
  Texture<T, D, C, Ty>::~Texture() {
    guard(m_is_init);
    gl_trace_gpu_free("gl::Texture", object());
    glDeleteTextures(1, &m_object);
  }

  /* operands for most texture types follow */

  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::get(std::span<T> data, uint level, vect size, vect offset) const
  requires(!detail::is_cubemap_type<Ty>) {
    gl_trace_full();

    constexpr auto format = detail::texture_format<C, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const vect safe_size = size.isZero() ? m_size : size;

    const size_t size_bytes = safe_size.prod() * C * pixel_size;
    debug::check_expr(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be written");

    if constexpr (storage_type == detail::StorageType::e1D) {
      glGetTextureSubImage(m_object, level, 
        offset.x(), 0, 0, 
        safe_size.x(), 1, 1, 
        format, pixel_format, data.size_bytes(), data.data());
    } else if constexpr (storage_type == detail::StorageType::e2D
                      || storage_type == detail::StorageType::e2DMSAA) {
      glGetTextureSubImage(m_object, level, 
        offset.x(), offset.y(), 0, 
        safe_size.x(), safe_size.y(), 1, 
        format, pixel_format, data.size_bytes(), data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D
                      || storage_type == detail::StorageType::e3DMSAA) {
      glGetTextureSubImage(m_object, level, 
        offset.x(), offset.y(), offset.z(), 
        safe_size.x(), safe_size.y(), safe_size.z(), 
        format, pixel_format, data.size_bytes(), data.data());
    }
  }

  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::set(std::span<const T> data, uint level, vect size, vect offset)
  requires(!detail::is_cubemap_type<Ty>) {
    gl_trace_full();

    constexpr auto format = detail::texture_format<C, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const vect safe_size = size.isZero() ? m_size : size;

    const size_t size_bytes = safe_size.prod() * C * pixel_size;
    debug::check_expr(data.data() && data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be read");

    if constexpr (storage_type == detail::StorageType::e1D) {
      glTextureSubImage1D(m_object, level, 
        offset.x(), 
        safe_size.x(), 
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e2D
                      || storage_type == detail::StorageType::e2DMSAA) {
      glTextureSubImage2D(m_object, level, 
        offset.x(), offset.y(), 
        safe_size.x(), safe_size.y(), 
        format, pixel_format, data.data());              
    } else if constexpr (storage_type == detail::StorageType::e3D
                      || storage_type == detail::StorageType::e3DMSAA) {
      glTextureSubImage3D(m_object, level, 
        offset.x(), offset.y(), offset.z(),
        safe_size.x(), safe_size.y(), safe_size.z(),
        format, pixel_format, data.data());
    }
  }

  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::clear(std::span<const T> data, uint level, vect size, vect offset)
  requires(!detail::is_cubemap_type<Ty>) {
    gl_trace_full();

    constexpr auto format = detail::texture_format<C, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const vect safe_size = size.isZero() ? m_size : size;

    const size_t size_bytes = C * pixel_size;
    debug::check_expr(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture to be cleared");

    if constexpr (storage_type == detail::StorageType::e1D) {
      glClearTexSubImage(m_object, level, 
        offset.x(), 0, 0, 
        safe_size.x(), 1, 1, 
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e2D
                      || storage_type == detail::StorageType::e2DMSAA) {
      glClearTexSubImage(m_object, level, 
        offset.x(), offset.y(), 0, 
        safe_size.x(), safe_size.y(), 1, 
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D
                      || storage_type == detail::StorageType::e3DMSAA) {
      glClearTexSubImage(m_object, level, 
        offset.x(), offset.y(), offset.z(), 
        safe_size.x(), safe_size.y(), safe_size.z(), 
        format, pixel_format, data.data());
    }
  }

  /* operands for cubemap texture types follow */

  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::get(std::span<T> data, uint face, uint level, vect size, vect offset) const 
  requires(detail::is_cubemap_type<Ty>) {
    gl_trace_full();

    constexpr auto format = detail::texture_format<C, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const vect safe_size = size.isZero() ? m_size : size;

    const size_t size_bytes = (safe_size - offset).prod() * C * pixel_size;
    debug::check_expr(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be written");
    
    if constexpr (storage_type == detail::StorageType::e2D) {
      glGetTextureSubImage(m_object, level, 
        offset.x(), offset.y(), face, 
        safe_size.x(), safe_size.y(), 1, 
        format, pixel_format, data.size_bytes(), data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      glGetTextureSubImage(m_object, level, 
        offset.x(), offset.y(), offset.z() * face, 
        safe_size.x(), safe_size.y(), safe_size.z(), 
        format, pixel_format, data.size_bytes(), data.data());
    }
  }

  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::set(std::span<const T> data, uint face, uint level, vect size, vect offset)
  requires(detail::is_cubemap_type<Ty>) {
    gl_trace_full();

    constexpr auto format = detail::texture_format<C, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const vect safe_size = size.isZero() ? m_size : size;

    const size_t size_bytes = safe_size.prod() * C * pixel_size;
    debug::check_expr(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture region to be read");

    if constexpr (storage_type == detail::StorageType::e2D) {
      glTextureSubImage3D(m_object, level, 
      offset.x(), offset.y(), face, 
      safe_size.x(), safe_size.y(), 1, 
      format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      glTextureSubImage3D(m_object, level, 
      offset.x(), offset.y(), safe_size.z() * face, 
      safe_size.x(), safe_size.y(), safe_size.z(), 
      format, pixel_format, data.data());
    }
  }

  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::clear(std::span<const T> data, uint face, uint level, vect size, vect offset)
  requires(detail::is_cubemap_type<Ty>) {
    gl_trace_full();

    constexpr auto format = detail::texture_format<C, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto pixel_size = detail::texture_pixel_size_bytes<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const vect safe_size = size.isZero() ? m_size : size;

    const size_t size_bytes = C * pixel_size;
    debug::check_expr(!data.data() || data.size_bytes() >= size_bytes,
      "provided data span is too small for requested texture to be cleared");
    
    if constexpr (storage_type == detail::StorageType::e2D) {
      glClearTexSubImage(m_object, level,
        offset.x(), offset.y(), face,
        safe_size.x(), safe_size.y(), 1,
        format, pixel_format, data.data());
    } else if constexpr (storage_type == detail::StorageType::e3D) {
      glClearTexSubImage(m_object, level,
        offset.x(), offset.y(), offset.z() * face,
        safe_size.x(), safe_size.y(), safe_size.z(),
        format, pixel_format, data.data());
    }
  }
  
  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::bind_to(TextureTargetType target, uint index, uint level) const {
    gl_trace_full();

    if (target == TextureTargetType::eTextureUnit) {
      glBindTextureUnit(index, m_object);
    } else {
      constexpr auto internal_format = detail::image_internal_format<C, T>();
      glBindImageTexture(index, m_object, level, GL_FALSE, 0, (uint) target, internal_format);
    }
  }
  
  template <typename T, uint D, uint C, TextureType Ty>
  void Texture<T, D, C, Ty>::generate_mipmaps() {
    gl_trace_full();
    guard(m_levels > 1);
    glGenerateTextureMipmap(m_object);
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

  // Image vect texture explicit template instantiations (1/2d, rgba, all pixel types)
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

  // Depth texture explicit template instantiations (1/2/3d/vect/cubemap/msaa, r, depth component)
  gl_explicit_texture(gl::DepthComponent, 1, 1, TextureType::eImage)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eImage)
  gl_explicit_texture(gl::DepthComponent, 3, 1, TextureType::eImage)
  gl_explicit_texture(gl::DepthComponent, 1, 1, TextureType::eImageArray)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eImageArray)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eCubemap)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eCubemapArray)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eMultisample)
  gl_explicit_texture(gl::DepthComponent, 2, 1, TextureType::eMultisampleArray)

  // Stemcil texture explicit template instantiations (1/2/3d/vect/cubemap/msaa, r, stencil component)
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