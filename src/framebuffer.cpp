#include <small_gl/detail/glm.hpp>
#include <small_gl/framebuffer.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/utility.hpp>

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
    debug::check_expr(is_complete, "framebuffer is not complete");
  }

  Framebuffer::~Framebuffer() {
    guard(_is_init);
    guard(_object != 0); // Default framebuffer 0 makes this a special case
    glDeleteFramebuffers(1, &_object);
  }

  void Framebuffer::bind() const {
    debug::check_expr(_is_init, "attempt to use an uninitialized object");
    glBindFramebuffer(GL_FRAMEBUFFER, _object);
  }

  void Framebuffer::unbind() const {
    debug::check_expr(_is_init, "attempt to use an uninitialized object");
    guard(_object != 0); // Default framebuffer 0 makes this a special case
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void Framebuffer::blit_to(gl::Framebuffer &dst,
                            glm::ivec2 src_size,
                            glm::ivec2 src_offset,
                            glm::ivec2 dst_size,
                            glm::ivec2 dst_offset,
                            FramebufferMaskFlags flags,
                            SamplerMagFilter filter) const {
    
    glBlitNamedFramebuffer(_object, dst.object(),
      src_offset[0], src_offset[1],
      src_size[0], src_size[1],
      dst_offset[0], dst_offset[1],
      dst_size[0], dst_size[1],
      (uint) flags, (uint) filter);
  }
  
  Framebuffer Framebuffer::make_default() {
    Framebuffer framebuffer;
    framebuffer._is_init = true;
    framebuffer._object = 0;
    return framebuffer;
  }

  Framebuffer Framebuffer::make_from(uint object) {
    auto is_framebuffer = glIsFramebuffer(object);
    debug::check_expr(is_framebuffer, "attempt to take ownership over a non-framebuffer handle");

    auto is_complete = glCheckNamedFramebufferStatus(object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    debug::check_expr(is_complete, "attempt to take ownership of an incomplete framebuffer");
    
    Framebuffer framebuffer;
    framebuffer._is_init = true;
    framebuffer._object = object;

    return framebuffer;
  }

  /* Explicit template instantiations of gl::Framebuffer::clear<...>(...) */

  #define gl_explicit_clear_template(type, type_short)\
    template <> void Framebuffer::clear<type>\
    (FramebufferType t, type v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, &v); }\
    template <> void Framebuffer::clear<glm::vec<2, type, glm::defaultp>>\
    (FramebufferType t, glm::vec<2, type, glm::defaultp> v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, glm::value_ptr(v)); }\
    template <> void Framebuffer::clear<glm::vec<3, type, glm::defaultp>>\
    (FramebufferType t, glm::vec<3, type, glm::defaultp> v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, glm::value_ptr(v)); }\
    template <> void Framebuffer::clear<glm::vec<4, type, glm::defaultp>>\
    (FramebufferType t, glm::vec<4, type, glm::defaultp> v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, glm::value_ptr(v)); }

   /*  template <> void Framebuffer::clear<Eigen::Vector<type, 2>>\
    (FramebufferType t, Eigen::Vector<type, 2> v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data()); }\
    template <> void Framebuffer::clear<Eigen::Vector<type, 3>>\
    (FramebufferType t, Eigen::Vector<type, 3> v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data()); }\
    template <> void Framebuffer::clear<Eigen::Vector<type, 4>>\
    (FramebufferType t, Eigen::Vector<type, 4> v, uint i)\
    { debug::check_expr(_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(_object, (uint) t, i, v.data()); } */

  // Explicit template specializations
  gl_explicit_clear_template(float, f)
  gl_explicit_clear_template(uint, ui)
  gl_explicit_clear_template(int, i)
} // namespace gl