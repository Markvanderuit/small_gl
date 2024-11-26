#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/enum.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/texture.hpp>
#include <small_gl/detail/trace.hpp>

namespace gl {
  struct RenderBufferInfo {
    // Multi-dimensional size of the rendebuffer; not in bytes
    eig::Array2u size;
  };

  template <typename T,
            uint C,
            RenderbufferType Ty
            = RenderbufferType::eImage>
  class Renderbuffer : public AbstractRenderbuffer {
    using Base = detail::Handle<>;

    eig::Array2u m_size;

  public:
    /* constr/destr */

    Renderbuffer() = default;
    Renderbuffer(RenderBufferInfo info);
    ~Renderbuffer();

    /* getters */

    inline eig::Array2u size() const { return m_size; }

    /* miscellaneous */

    inline void swap(Renderbuffer &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_size, o.m_size);
    }

    inline bool operator==(const Renderbuffer &o) {
      return Base::operator==(o) && (m_size == o.m_size).all();
    }

    gl_declare_noncopyable(Renderbuffer)
  };
} // namespace gl