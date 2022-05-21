#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/texture.hpp>
#include <glm/glm.hpp>

namespace gl {
  struct RenderbufferCreateInfo {
    // Multi-dimensional size of the rendebuffer; not in bytes
    glm::ivec2 size;
  };

  template <typename T,
            uint C,
            RenderbufferType Ty
            = RenderbufferType::eImage>
  class Renderbuffer : public AbstractRenderbuffer {
    using Base = detail::Handle<>;

    glm::ivec2 m_size;

  public:
    /* constr/destr */

    Renderbuffer() = default;
    Renderbuffer(RenderbufferCreateInfo info);
    ~Renderbuffer();

    /* getters */

    inline glm::ivec2 size() const { return m_size; }

    /* miscellaneous */

    inline void swap(Renderbuffer &o) {
      using std::swap;
      Base::swap(o);
      swap(m_size, o.m_size);
    }

    inline bool operator==(const Renderbuffer &o) {
      return Base::operator==(o) && m_size == o.m_size;
    }

    gl_declare_noncopyable(Renderbuffer)
  };
} // namespace gl