#include <small_gl/renderbuffer.hpp>
#include <small_gl/utility.hpp>

namespace gl {
  template <typename T, uint C, RenderbufferType Ty>
  Renderbuffer<T, C, Ty>::Renderbuffer(RenderbufferCreateInfo info)
  : Base(true), m_size(info.size) {
    gl_trace_full();
    debug::check_expr((m_size >= eig::Array2u(1u)).all(), "renderbuffer size must be all >= 1");

    glCreateRenderbuffers(1, &m_object);

    constexpr auto internal_format = detail::texture_internal_format<C, T>();

    if constexpr (Ty == RenderbufferType::eImage) {
      glNamedRenderbufferStorage(m_object, internal_format, m_size.x(), m_size.y());
    } else if constexpr (Ty == RenderbufferType::eMultisample) {
      glNamedRenderbufferStorageMultisample(m_object, 4, internal_format, m_size.x(), m_size.y());
    }

    // Poorly estimate renderbuffer size in bytes
    size_t alloc_size = m_size.prod() * C * detail::texture_pixel_size_bytes<T>();
    gl_trace_gpu_alloc("gl::Renderbuffer", object(), alloc_size);
  }

  template <typename T, uint C, RenderbufferType Ty>
  Renderbuffer<T, C, Ty>::~Renderbuffer() {
    gl_trace_full();
    guard(m_is_init);
    glDeleteRenderbuffers(1, &m_object);
    gl_trace_gpu_free("gl::Renderbuffer", object());
  }

  /* Explicit template instantiations of gl::Renderbuffer<...> */
  
  #define gl_explicit_rbuffer(type, components, rbuffer_type)\
    template class Renderbuffer<type, components, rbuffer_type>;

  #define gl_explicit_rbuffer_components_1234(type, rbuffer_type)\
    gl_explicit_rbuffer(type, 1, rbuffer_type)\
    gl_explicit_rbuffer(type, 2, rbuffer_type)\
    gl_explicit_rbuffer(type, 3, rbuffer_type)\
    gl_explicit_rbuffer(type, 4, rbuffer_type)

  #define gl_explicit_rbuffer_all_types(rbuffer_type)\
    gl_explicit_rbuffer_components_1234(ushort, rbuffer_type)\
    gl_explicit_rbuffer_components_1234(short, rbuffer_type)\
    gl_explicit_rbuffer_components_1234(uint, rbuffer_type)\
    gl_explicit_rbuffer_components_1234(int, rbuffer_type)\
    gl_explicit_rbuffer_components_1234(float, rbuffer_type)
  
  gl_explicit_rbuffer_all_types(RenderbufferType::eImage)
  gl_explicit_rbuffer_all_types(RenderbufferType::eMultisample)

  gl_explicit_rbuffer(DepthComponent, 1, RenderbufferType::eImage);
  gl_explicit_rbuffer(DepthComponent, 1, RenderbufferType::eMultisample);
  gl_explicit_rbuffer(StencilComponent, 1, RenderbufferType::eImage);
  gl_explicit_rbuffer(StencilComponent, 1, RenderbufferType::eMultisample);
} // namespace gl