#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/trace.hpp>
#include <initializer_list>
#include <span>

namespace gl {
  /**
   * Helper object to create framebuffer object by defining a single
   * framebuffer attachment.
   */
  struct FramebufferAttachmentInfo {
    // Framebuffer attachment type (color, depth, stencil)
    FramebufferType type;

    // Binding index to attach to (only applies for color)
    uint index = 0;

    // Object handle to attach to framebuffer
    // Either a texture, texture view, or render buffer
    const AbstractFramebufferAttachment *attachment;

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
    Framebuffer(FramebufferAttachmentInfo info);
    Framebuffer(std::initializer_list<FramebufferAttachmentInfo> info);
    ~Framebuffer();

    /* state */

    // Clear one of the framebuffer attachments (color + index, depth, stencil)
    template <typename T>
    void clear(FramebufferType type, T t = T(0), uint index = 0);

    void bind() const;
    void unbind() const;

    /* miscellaneous */  

    void blit_to(gl::Framebuffer &dst,
                 eig::Array2u src_size,
                 eig::Array2u src_offset,
                 eig::Array2u dst_size,
                 eig::Array2u dst_offset,
                 FramebufferMaskFlags flags,
                 SamplerMagFilter filter = SamplerMagFilter::eNearest) const;

    // Return a special object acting as a placeholder for the default framebuffer
    static Framebuffer make_default();

    // Assume lifetime ownership over a provided framebuffer handle
    static Framebuffer make_from(uint object);

    inline void swap(Framebuffer &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
    }

    inline bool operator==(const Framebuffer &o) const {
      return Base::operator==(o);
    }

    gl_declare_noncopyable(Framebuffer)
  };
} // namespace gl