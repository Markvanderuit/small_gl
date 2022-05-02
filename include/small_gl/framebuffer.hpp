#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/handle.hpp>
#include <initializer_list>
#include <span>

namespace gl {
  struct FramebufferCreateInfo {
    FramebufferType type;
    const Handle<> *texture;
    uint index = 0;
    uint level = 0;
  };
  
  
  struct Framebuffer : public Handle<> {
    /* constr/destr */

    Framebuffer() = default;
    Framebuffer(FramebufferCreateInfo info);
    Framebuffer(std::initializer_list<FramebufferCreateInfo> info);
    ~Framebuffer();

    /* state */

    template <typename T>
    void clear(FramebufferType type, T t = { }, uint index = 0);

    void bind() const;
    void unbind() const;

    /* miscellaneous */  

    // Return an uninitialized object masking as 
    // a "placeholder" for a default framebuffer
    static Framebuffer make_default();

    // Assume lifetime ownership over a provided buffer
    static Framebuffer make_from(uint object);

  private:
    using Base = Handle<>;
  
  public:
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