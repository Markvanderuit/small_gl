#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/enum.hpp>

namespace gl::detail {
  // Concept to evaluate whether a cubemap type is used
  template <TextureType Ty>
  concept is_cubemap_type = Ty == TextureType::eCubemap
                         || Ty == TextureType::eCubemapArray;

  // Consteval function determining a texture's underlying dimensions at compile time
  template <uint D, TextureType Ty>
  consteval uint texture_dims();
  
  // Consteval function determining a texture format
  template <uint Components, typename T>
  consteval uint texture_format();

  // Consteval function determining a texture's internal_format
  template <uint Components, typename T>
  consteval uint texture_internal_format();

  // Consteval function determinig a texture's correct binding target
  template <uint D, TextureType Ty>
  consteval uint texture_target();

  // Consteval function determining a texture's correct pixel format
  template <typename T>
  consteval uint texture_pixel_format();

  // Consteval function determining a texture's pixel format size in machine units
  template <typename T>
  consteval uint texture_pixel_size_bytes() { return sizeof(T); }
  
  // Enum types matching the five kinds of glTextureStorage*(...)
  enum class StorageType { e1D, e2D, e3D, e2DMSAA, e3DMSAA };

  // Consteval function determining a texture's correct storage type
  template <uint D, TextureType Ty> 
  consteval StorageType texture_storage_type();

  /* Explicit template specializations */

  // Template specializations for the above declared texture_dims();
  template <> consteval uint texture_dims<1, TextureType::eImage>()            { return 1; }
  template <> consteval uint texture_dims<2, TextureType::eImage>()            { return 2; }
  template <> consteval uint texture_dims<3, TextureType::eImage>()            { return 3; }
  template <> consteval uint texture_dims<1, TextureType::eImageArray>()       { return 2; }
  template <> consteval uint texture_dims<2, TextureType::eImageArray>()       { return 3; }
  template <> consteval uint texture_dims<2, TextureType::eCubemap>()          { return 2; }
  template <> consteval uint texture_dims<2, TextureType::eCubemapArray>()     { return 3; }
  template <> consteval uint texture_dims<2, TextureType::eMultisample>()      { return 2; }
  template <> consteval uint texture_dims<2, TextureType::eMultisampleArray>() { return 3; }

  // Template specializations for the above declared texture_format();
  template <> consteval uint texture_format<1, ushort>()           { return GL_RED_INTEGER; }
  template <> consteval uint texture_format<1, short>()            { return GL_RED_INTEGER; }
  template <> consteval uint texture_format<1, uint>()             { return GL_RED_INTEGER; }
  template <> consteval uint texture_format<1, int>()              { return GL_RED_INTEGER; }
  template <> consteval uint texture_format<1, float>()            { return GL_RED; }
  template <> consteval uint texture_format<2, ushort>()           { return GL_RG_INTEGER; }
  template <> consteval uint texture_format<2, short>()            { return GL_RG_INTEGER; }
  template <> consteval uint texture_format<2, uint>()             { return GL_RG_INTEGER; }
  template <> consteval uint texture_format<2, int>()              { return GL_RG_INTEGER; }
  template <> consteval uint texture_format<2, float>()            { return GL_RG; }
  template <> consteval uint texture_format<3, ushort>()           { return GL_RGB_INTEGER; }
  template <> consteval uint texture_format<3, short>()            { return GL_RGB_INTEGER; }
  template <> consteval uint texture_format<3, uint>()             { return GL_RGB_INTEGER; }
  template <> consteval uint texture_format<3, int>()              { return GL_RGB_INTEGER; }
  template <> consteval uint texture_format<3, float>()            { return GL_RGB; }
  template <> consteval uint texture_format<4, ushort>()           { return GL_RGBA_INTEGER; }
  template <> consteval uint texture_format<4, short>()            { return GL_RGBA_INTEGER; }
  template <> consteval uint texture_format<4, uint>()             { return GL_RGBA_INTEGER; }
  template <> consteval uint texture_format<4, int>()              { return GL_RGBA_INTEGER; }
  template <> consteval uint texture_format<4, float>()            { return GL_RGBA; }
  template <> consteval uint texture_format<1, DepthComponent>()   { return GL_DEPTH_COMPONENT; }
  template <> consteval uint texture_format<1, StencilComponent>() { return GL_STENCIL_INDEX; }

  // Template specializations for the above declared texture_internal_format();
  template <> consteval uint texture_internal_format<1, ushort>()           { return GL_R16UI; }
  template <> consteval uint texture_internal_format<1, short>()            { return GL_R16I; }
  template <> consteval uint texture_internal_format<1, uint>()             { return GL_R32UI; }
  template <> consteval uint texture_internal_format<1, int>()              { return GL_R32I; }
  template <> consteval uint texture_internal_format<1, float>()            { return GL_R32F; }
  template <> consteval uint texture_internal_format<2, ushort>()           { return GL_RG16UI; }
  template <> consteval uint texture_internal_format<2, short>()            { return GL_RG16I; }
  template <> consteval uint texture_internal_format<2, uint>()             { return GL_RG32UI; }
  template <> consteval uint texture_internal_format<2, int>()              { return GL_RG32I; }
  template <> consteval uint texture_internal_format<2, float>()            { return GL_RG32F; }
  template <> consteval uint texture_internal_format<3, ushort>()           { return GL_RGB16UI; }
  template <> consteval uint texture_internal_format<3, short>()            { return GL_RGB16I; }
  template <> consteval uint texture_internal_format<3, uint>()             { return GL_RGB32UI; }
  template <> consteval uint texture_internal_format<3, int>()              { return GL_RGB32I; }
  template <> consteval uint texture_internal_format<3, float>()            { return GL_RGB32F; }
  template <> consteval uint texture_internal_format<4, ushort>()           { return GL_RGBA16UI; }
  template <> consteval uint texture_internal_format<4, short>()            { return GL_RGBA16I; }
  template <> consteval uint texture_internal_format<4, uint>()             { return GL_RGBA32UI; }
  template <> consteval uint texture_internal_format<4, int>()              { return GL_RGBA32I; }
  template <> consteval uint texture_internal_format<4, float>()            { return GL_RGBA32F; }
  template <> consteval uint texture_internal_format<1, DepthComponent>()   { return GL_DEPTH_COMPONENT32F; }
  template <> consteval uint texture_internal_format<1, StencilComponent>() { return GL_STENCIL_INDEX8; }

  // Template specializations for the above declared texture_target();
  template <> consteval uint texture_target<1, TextureType::eImage>()            { return GL_TEXTURE_1D; }
  template <> consteval uint texture_target<2, TextureType::eImage>()            { return GL_TEXTURE_2D; }
  template <> consteval uint texture_target<3, TextureType::eImage>()            { return GL_TEXTURE_3D; }
  template <> consteval uint texture_target<1, TextureType::eImageArray>()       { return GL_TEXTURE_1D_ARRAY; }
  template <> consteval uint texture_target<2, TextureType::eImageArray>()       { return GL_TEXTURE_2D_ARRAY; }
  template <> consteval uint texture_target<2, TextureType::eCubemap>()          { return GL_TEXTURE_CUBE_MAP; }
  template <> consteval uint texture_target<2, TextureType::eCubemapArray>()     { return GL_TEXTURE_CUBE_MAP_ARRAY; }
  template <> consteval uint texture_target<2, TextureType::eMultisample>()      { return GL_TEXTURE_2D_MULTISAMPLE; }
  template <> consteval uint texture_target<2, TextureType::eMultisampleArray>() { return GL_TEXTURE_2D_MULTISAMPLE_ARRAY; }

  // Template specializations for the above declared texture_pixel_format();
  template <> consteval uint texture_pixel_format<ushort>()           { return GL_UNSIGNED_SHORT; }
  template <> consteval uint texture_pixel_format<short>()            { return GL_SHORT; }
  template <> consteval uint texture_pixel_format<uint>()             { return GL_UNSIGNED_INT; }
  template <> consteval uint texture_pixel_format<int>()              { return GL_INT; }
  template <> consteval uint texture_pixel_format<float>()            { return GL_FLOAT; }
  template <> consteval uint texture_pixel_format<DepthComponent>()   { return GL_FLOAT; }
  template <> consteval uint texture_pixel_format<StencilComponent>() { return GL_UNSIGNED_BYTE; }

  // Template specializations for the above declared texture_pixel_size_bytes() for special types
  template <> consteval uint texture_pixel_size_bytes<DepthComponent>()   { return sizeof(float); }
  template <> consteval uint texture_pixel_size_bytes<StencilComponent>() { return sizeof(std::byte); }

  // Template specializations for the above declared texture_storage_type();
  template <> consteval StorageType texture_storage_type<1, TextureType::eImage>()            { return StorageType::e1D; }
  template <> consteval StorageType texture_storage_type<2, TextureType::eImage>()            { return StorageType::e2D; }
  template <> consteval StorageType texture_storage_type<3, TextureType::eImage>()            { return StorageType::e3D; }
  template <> consteval StorageType texture_storage_type<1, TextureType::eImageArray>()       { return StorageType::e2D; }
  template <> consteval StorageType texture_storage_type<2, TextureType::eImageArray>()       { return StorageType::e3D; }
  template <> consteval StorageType texture_storage_type<2, TextureType::eCubemap>()          { return StorageType::e2D; }
  template <> consteval StorageType texture_storage_type<2, TextureType::eCubemapArray>()     { return StorageType::e3D; }
  template <> consteval StorageType texture_storage_type<2, TextureType::eMultisample>()      { return StorageType::e2DMSAA; }
  template <> consteval StorageType texture_storage_type<2, TextureType::eMultisampleArray>() { return StorageType::e3DMSAA; }
} // namespace gl::detail