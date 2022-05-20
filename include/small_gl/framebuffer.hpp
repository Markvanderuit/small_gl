#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <glm/vec2.hpp>
#include <initializer_list>
#include <span>

namespace gl {
  /**
   * Helper object to create framebuffer object by defining a single
   * framebuffer attachment.
   */
  struct FramebufferCreateInfo {
    // Framebuffer attachment type (color, depth, stencil)
    FramebufferType type;

    // Binding index to attach to (only applies for color)
    uint index = 0;

    // Texture handle to attach to framebuffer
    const AbstractTexture *texture;

    // Mipmap level of texture to attach
    uint level = 0;
  };
  
  /**
   * Framebuffer object wrapping OpenGL framebuffer object.
   */
  class Framebuffer : public detail::Handle<> {
    using Base = detail::Handle<>;

  public:
    /* constr/destr */

    Framebuffer() = default;
    Framebuffer(FramebufferCreateInfo info);
    Framebuffer(std::initializer_list<FramebufferCreateInfo> info);
    ~Framebuffer();

    /* state */

    // Clear one of the framebuffer attachments (color + index, depth, stencil)
    template <typename T>
    void clear(FramebufferType type, T t = T(0), uint index = 0);

    void bind() const;
    void unbind() const;

    /* miscellaneous */  

    void blit_to(gl::Framebuffer &dst,
                 glm::ivec2 src_size,
                 glm::ivec2 src_offset,
                 glm::ivec2 dst_size,
                 glm::ivec2 dst_offset,
                 FramebufferMaskFlags flags,
                 SamplerMagFilter filter = SamplerMagFilter::eNearest) const;

    // Return a special object acting as a placeholder for the default framebuffer
    static Framebuffer make_default();

    // Assume lifetime ownership over a provided framebuffer handle
    static Framebuffer make_from(uint object);

    inline void swap(Framebuffer &o) {
      using std::swap;
      Base::swap(o);
    }

    inline bool operator==(const Framebuffer &o) const {
      return Base::operator==(o);
    }

    gl_declare_noncopyable(Framebuffer)
  };
} // namespace gl