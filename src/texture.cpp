#include <small_gl/texture.hpp>
#include <small_gl/detail/exception.hpp>

namespace gl {
  template <typename T, uint D, uint Components, TextureType Ty>
  Texture<T, D, Components, Ty>::Texture(TextureCreateInfo info)
  : Base(true), _size(info.size), _levels(info.levels) {
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

    // If prior data was provided, process this or return early
    guard(info.data.data());
    set(info.data);
    generate_mipmaps();
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  Texture<T, D, Components, Ty>::~Texture() {
    guard(_is_init);
    glDeleteTextures(1, &_object);
  }

  /* operands for most texture types follow */

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::get(std::span<T> data, uint level, Array size, Array offset) const
  requires(!detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

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
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::set(std::span<const T> data, uint level, Array size, Array offset)
  requires(!detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

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
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::clear(std::span<const T> data, uint level, Array size, Array offset)
  requires(!detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

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
  }

  /* operands for cubemap texture types follow */

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::get(std::span<T> data, uint face, uint level, Array size, Array offset) const 
  requires(detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;
    
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
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::set(std::span<const T> data, uint face, uint level, Array size, Array offset)
  requires(detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;

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
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::clear(std::span<const T> data, uint face, uint level, Array size, Array offset)
  requires(detail::is_cubemap_type<Ty>) {
    constexpr auto format = detail::texture_format<Components, T>();
    constexpr auto pixel_format = detail::texture_pixel_format<T>();
    constexpr auto storage_type = detail::texture_storage_type<D, Ty>();
    const Array safe_size = (size == 0).all() ? _size : size;
    
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
  }
  
  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::bind_to(uint index) const {
    glBindTextureUnit(index, _object);
  }

  template <typename T, uint D, uint Components, TextureType Ty>
  void Texture<T, D, Components, Ty>::generate_mipmaps() {
    guard(_levels > 1);
    glGenerateTextureMipmap(_object);
  }

  /* Explicit template instantiations of gl::Texture<...> */

  #define MET_TEXTURE_INST_FOR(type, dims, components, texture_type)\
    template class Texture<type, dims, components, texture_type>;

  #define MET_TEXTURE_INST_COMPONENTS_1(type, dims, texture_type)\
    MET_TEXTURE_INST_FOR(type, dims, 1, texture_type)

  #define MET_TEXTURE_INST_COMPONENTS_1_2(type, dims, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1(type, dims, texture_type)\
    MET_TEXTURE_INST_FOR(type, dims, 2, texture_type)

  #define MET_TEXTURE_INST_COMPONENTS_1_2_3(type, dims, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2(type, dims, texture_type)\
    MET_TEXTURE_INST_FOR(type, dims, 3, texture_type)

  #define MET_TEXTURE_INST_COMPONENTS_1_2_3_4(type, dims, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3(type, dims, texture_type)\
    MET_TEXTURE_INST_FOR(type, dims, 4, texture_type)

  #define MET_TEXTURE_INST_DIMS_1_2_3_special(type, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1(type, 1, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1(type, 2, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1(type, 3, texture_type)

  #define MET_TEXTURE_INST_DIMS_1_2_rgba(type, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(type, 1, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(type, 2, texture_type)

  #define MET_TEXTURE_INST_DIMS_1_2_3_rgba(type, texture_type)\
    MET_TEXTURE_INST_DIMS_1_2_rgba(type, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(type, 3, texture_type)

  #define MET_TEXTURE_INST_TYPES_2d_rgba(texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(ushort, 2, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(short, 2, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(uint, 2, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(int, 2, texture_type)\
    MET_TEXTURE_INST_COMPONENTS_1_2_3_4(float, 2, texture_type)

  MET_TEXTURE_INST_DIMS_1_2_3_rgba(ushort, TextureType::eImage)
  MET_TEXTURE_INST_DIMS_1_2_3_rgba(short, TextureType::eImage)
  MET_TEXTURE_INST_DIMS_1_2_3_rgba(uint, TextureType::eImage)
  MET_TEXTURE_INST_DIMS_1_2_3_rgba(int, TextureType::eImage)
  MET_TEXTURE_INST_DIMS_1_2_3_rgba(float, TextureType::eImage)

  MET_TEXTURE_INST_DIMS_1_2_rgba(ushort, TextureType::eImageArray)
  MET_TEXTURE_INST_DIMS_1_2_rgba(short, TextureType::eImageArray)
  MET_TEXTURE_INST_DIMS_1_2_rgba(uint, TextureType::eImageArray)
  MET_TEXTURE_INST_DIMS_1_2_rgba(int, TextureType::eImageArray)
  MET_TEXTURE_INST_DIMS_1_2_rgba(float, TextureType::eImageArray)

  MET_TEXTURE_INST_TYPES_2d_rgba(TextureType::eCubemap)
  MET_TEXTURE_INST_TYPES_2d_rgba(TextureType::eCubemapArray)
  MET_TEXTURE_INST_TYPES_2d_rgba(TextureType::eMultisample)
  MET_TEXTURE_INST_TYPES_2d_rgba(TextureType::eMultisampleArray)

  MET_TEXTURE_INST_DIMS_1_2_3_special(gl::DepthComponent, TextureType::eImage)
  MET_TEXTURE_INST_COMPONENTS_1(gl::DepthComponent, 1, TextureType::eImageArray)
  MET_TEXTURE_INST_COMPONENTS_1(gl::DepthComponent, 2, TextureType::eImageArray)
  MET_TEXTURE_INST_COMPONENTS_1(gl::DepthComponent, 2, TextureType::eCubemap)
  MET_TEXTURE_INST_COMPONENTS_1(gl::DepthComponent, 2, TextureType::eCubemapArray)
  MET_TEXTURE_INST_COMPONENTS_1(gl::DepthComponent, 2, TextureType::eMultisample)
  MET_TEXTURE_INST_COMPONENTS_1(gl::DepthComponent, 2, TextureType::eMultisampleArray)

  MET_TEXTURE_INST_DIMS_1_2_3_special(gl::StencilComponent, TextureType::eImage)
  MET_TEXTURE_INST_COMPONENTS_1(gl::StencilComponent, 1, TextureType::eImageArray)
  MET_TEXTURE_INST_COMPONENTS_1(gl::StencilComponent, 2, TextureType::eImageArray)
  MET_TEXTURE_INST_COMPONENTS_1(gl::StencilComponent, 2, TextureType::eCubemap)
  MET_TEXTURE_INST_COMPONENTS_1(gl::StencilComponent, 2, TextureType::eCubemapArray)
  MET_TEXTURE_INST_COMPONENTS_1(gl::StencilComponent, 2, TextureType::eMultisample)
  MET_TEXTURE_INST_COMPONENTS_1(gl::StencilComponent, 2, TextureType::eMultisampleArray)
} // namespace gl