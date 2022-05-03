#include <small_gl/framebuffer.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/exception.hpp>

namespace gl {
  namespace detail {
    GLenum framebuffer_attachment(gl::FramebufferType type) {
      switch (type) {
        case gl::FramebufferType::eDepth: return GL_DEPTH_ATTACHMENT;
        case gl::FramebufferType::eStencil: return GL_STENCIL_ATTACHMENT;
        default: return GL_COLOR_ATTACHMENT0;
      }
    }
  } // namespace detail

  Framebuffer::Framebuffer(FramebufferCreateInfo info)
  : Framebuffer({info}) { }

  Framebuffer::Framebuffer(std::initializer_list<FramebufferCreateInfo> info)
  : Base(true) {
    glCreateFramebuffers(1, &_object);

    for (const auto &info : info) {
      glNamedFramebufferTexture(_object, 
        detail::framebuffer_attachment(info.type) + info.index, 
        info.texture->object(), 
        info.level);
    }

    auto is_complete = glCheckNamedFramebufferStatus(_object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    expr_check(is_complete, "framebuffer is not complete");
    gl_check();
  }

  Framebuffer::~Framebuffer() {
    guard(_is_init);
    guard(_object != 0); // Default framebuffer 0 makes this a special case
    glDeleteFramebuffers(1, &_object);
    gl_check();
  }

  void Framebuffer::bind() const {
    expr_check(_is_init, "attempt to use an uninitialized object");
    glBindFramebuffer(GL_FRAMEBUFFER, _object);
    gl_check();
  }

  void Framebuffer::unbind() const {
    expr_check(_is_init, "attempt to use an uninitialized object");
    guard(_object != 0); // Default framebuffer 0 makes this a special case
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_check();
  }

  void Framebuffer::blit_to(gl::Framebuffer &dst,
                            Array2i src_size,
                            Array2i src_offset,
                            Array2i dst_size,
                            Array2i dst_offset,
                            FramebufferMaskFlags flags,
                            SamplerMagFilter filter) const {
    
    glBlitNamedFramebuffer(_object, dst.object(),
      src_offset[0], src_offset[1],
      src_size[0], src_size[1],
      dst_offset[0], dst_offset[1],
      dst_size[0], dst_size[1],
      (uint) flags, (uint) filter);
    gl_check();
  }
  
  Framebuffer Framebuffer::make_default() {
    Framebuffer framebuffer;
    framebuffer._is_init = true;
    framebuffer._object = 0;
    return framebuffer;
  }

  Framebuffer Framebuffer::make_from(uint object) {
    auto is_framebuffer = glIsFramebuffer(object);
    expr_check(is_framebuffer, "attempt to take ownership over a non-framebuffer handle");

    auto is_complete = glCheckNamedFramebufferStatus(object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    expr_check(is_complete, "attempt to take ownership of an incomplete framebuffer");
    
    Framebuffer framebuffer;
    framebuffer._is_init = true;
    framebuffer._object = object;

    return framebuffer;
  }

  /* Explicit template instantiations of gl::Framebuffer::clear<...>(...) */

  #define gl_explicit_clear_template(type, type_short)\
    template <> void Framebuffer::clear<type>\
    (FramebufferType t, type v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, &v);\
      gl_check(); }\
    template <> void Framebuffer::clear<Eigen::Array<type, 2, 1>>\
    (FramebufferType t, Eigen::Array<type, 2, 1> v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      gl_check(); }\
    template <> void Framebuffer::clear<Eigen::Array<type, 3, 1>>\
    (FramebufferType t, Eigen::Array<type, 3, 1> v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      gl_check(); }\
    template <> void Framebuffer::clear<Eigen::Array<type, 4, 1>>\
    (FramebufferType t, Eigen::Array<type, 4, 1> v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      gl_check(); }\
    template <> void Framebuffer::clear<Eigen::Vector<type, 2>>\
    (FramebufferType t, Eigen::Vector<type, 2> v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      gl_check(); }\
    template <> void Framebuffer::clear<Eigen::Vector<type, 3>>\
    (FramebufferType t, Eigen::Vector<type, 3> v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      gl_check(); }\
    template <> void Framebuffer::clear<Eigen::Vector<type, 4>>\
    (FramebufferType t, Eigen::Vector<type, 4> v, uint i)\
    { expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      gl_check(); }

  // Explicit template specializations
  gl_explicit_clear_template(float, f)
  gl_explicit_clear_template(uint, ui)
  gl_explicit_clear_template(int, i)
} // namespace gl