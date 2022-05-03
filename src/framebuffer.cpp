#include <small_gl/framebuffer.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/detail/exception.hpp>

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
    detail::expr_check(is_complete, "framebuffer is not complete");
    detail::gl_check();
  }

  Framebuffer::~Framebuffer() {
    guard(_is_init);
    guard(glIsFramebuffer(_object)); // default framebuffer makes this a special case
    glDeleteFramebuffers(1, &_object);
    detail::gl_check();
  }

  void Framebuffer::bind() const {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");
    glBindFramebuffer(GL_FRAMEBUFFER, _object);
    detail::gl_check();
  }

  void Framebuffer::unbind() const {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    detail::gl_check();
  }
  
  Framebuffer Framebuffer::make_default() {
    Framebuffer framebuffer;
    framebuffer._is_init = true;
    return framebuffer;
  }

  Framebuffer Framebuffer::make_from(uint object) {
    auto is_framebuffer = glIsFramebuffer(object);
    detail::expr_check(is_framebuffer, "attempt to take ownership over a non-framebuffer handle");

    auto is_complete = glCheckNamedFramebufferStatus(object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    detail::expr_check(is_complete, "attempt to take ownership of an incomplete framebuffer");
    
    Framebuffer framebuffer;
    framebuffer._is_init = true;
    framebuffer._object = object;

    return framebuffer;
  }

  /* Explicit template instantiations of gl::Framebuffer::clear<...>(...) */

  #define gl_explicit_clear_template(type, type_short)\
    template <> void Framebuffer::clear<type>\
    (FramebufferType t, type v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, &v);\
      detail::gl_check(); }\
    template <> void Framebuffer::clear<eig::Array<type, 2, 1>>\
    (FramebufferType t, eig::Array<type, 2, 1> v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      detail::gl_check(); }\
    template <> void Framebuffer::clear<eig::Array<type, 3, 1>>\
    (FramebufferType t, eig::Array<type, 3, 1> v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      detail::gl_check(); }\
    template <> void Framebuffer::clear<eig::Array<type, 4, 1>>\
    (FramebufferType t, eig::Array<type, 4, 1> v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      detail::gl_check(); }\
    template <> void Framebuffer::clear<eig::Vector<type, 2>>\
    (FramebufferType t, eig::Vector<type, 2> v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      detail::gl_check(); }\
    template <> void Framebuffer::clear<eig::Vector<type, 3>>\
    (FramebufferType t, eig::Vector<type, 3> v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      detail::gl_check(); }\
    template <> void Framebuffer::clear<eig::Vector<type, 4>>\
    (FramebufferType t, eig::Vector<type, 4> v, uint i)\
    { detail::expr_check(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data());\
      detail::gl_check(); }

  // Explicit template specializations
  gl_explicit_clear_template(float, f)
  gl_explicit_clear_template(uint, ui)
  gl_explicit_clear_template(int, i)
} // namespace gl