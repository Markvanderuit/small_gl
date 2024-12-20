#include <small_gl/framebuffer.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/utility.hpp>

namespace gl {
  namespace detail {
    GLenum framebuffer_attachment(gl::FramebufferType type) {
      switch (type) {
        case gl::FramebufferType::eDepth:   return GL_DEPTH_ATTACHMENT;
        case gl::FramebufferType::eStencil: return GL_STENCIL_ATTACHMENT;
        default:                            return GL_COLOR_ATTACHMENT0;
      }
    }
  } // namespace detail

  Framebuffer::Framebuffer(FramebufferAttachmentInfo info)
  : Framebuffer({info}) { }

  Framebuffer::Framebuffer(std::initializer_list<FramebufferAttachmentInfo> info)
  : Base(true) {
    gl_trace_full();

    glCreateFramebuffers(1, &m_object);

    uint n_color_targets = 0;
    std::vector<uint> color_targets;

    for (const auto &info : info) {
      auto attachment_type = detail::framebuffer_attachment(info.type);
      if (info.type == gl::FramebufferType::eColor)
        color_targets.push_back(attachment_type + info.index);

      if (auto attachment = dynamic_cast<const AbstractTexture *>(info.attachment)) {
        if (attachment->layers() > 0) {
          glNamedFramebufferTexture3DEXT(m_object, 
            attachment_type + info.index,
            attachment->target(),
            attachment->object(),
            info.level,
            info.layer);
        } else {
          glNamedFramebufferTexture(m_object, 
            attachment_type + info.index, 
            attachment->object(), 
            info.level);
        }
      } else if (auto attachment = dynamic_cast<const AbstractRenderbuffer *>(info.attachment)) {
        glNamedFramebufferRenderbuffer(m_object,
          attachment_type + info.index, 
          GL_RENDERBUFFER,
          attachment->object());
      }
    }
    glNamedFramebufferDrawBuffers(m_object, color_targets.size(), color_targets.data());

    auto is_complete = glCheckNamedFramebufferStatus(m_object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    debug::check_expr(is_complete, "framebuffer is not complete");
  }

  Framebuffer::~Framebuffer() {
    gl_trace_full();
    guard(m_is_init);
    guard(m_object != 0); // Default framebuffer 0 makes this a special case
    glDeleteFramebuffers(1, &m_object);
  }

  void Framebuffer::bind() const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glBindFramebuffer(GL_FRAMEBUFFER, m_object);
  }

  void Framebuffer::unbind() const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    guard(m_object != 0); // Default framebuffer 0 makes this a special case
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void Framebuffer::blit_to(gl::Framebuffer &dst,
                            eig::Array2u src_size,
                            eig::Array2u src_offset,
                            eig::Array2u dst_size,
                            eig::Array2u dst_offset,
                            FramebufferMaskFlags flags,
                            SamplerMagFilter filter) const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    debug::check_expr(dst.m_is_init, "attempt to use an uninitialized object");
    
    glBlitNamedFramebuffer(m_object, dst.object(),
      src_offset[0], src_offset[1],
      src_size[0], src_size[1],
      dst_offset[0], dst_offset[1],
      dst_size[0], dst_size[1],
      (uint) flags, (uint) filter);
  }
  
  Framebuffer Framebuffer::make_default() {
    gl_trace_full();
    Framebuffer framebuffer;
    framebuffer.m_is_init = true;
    framebuffer.m_object = 0;
    return framebuffer;
  }

  Framebuffer Framebuffer::make_from(uint object) {
    gl_trace_full();
    auto is_framebuffer = glIsFramebuffer(object);
    debug::check_expr(is_framebuffer, "attempt to take ownership over a non-framebuffer handle");

    auto is_complete = glCheckNamedFramebufferStatus(object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    debug::check_expr(is_complete, "attempt to take ownership of an incomplete framebuffer");
    
    Framebuffer framebuffer;
    framebuffer.m_is_init = true;
    framebuffer.m_object = object;

    return framebuffer;
  }

  /* Explicit template instantiations of gl::Framebuffer::clear<...>(...) */

  #define gl_explicit_clear_template(type, type_short)\
    template <> void Framebuffer::clear<type>\
    (FramebufferType t, type v, uint i)\
    { debug::check_expr(m_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(m_object, (uint) t, i, &v); }\
    template <> void Framebuffer::clear<eig::Array<type, 2, 1>>\
    (FramebufferType t, eig::Array<type, 2, 1> v, uint i)\
    { debug::check_expr(m_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(m_object, (uint) t, i, v.data()); }\
    template <> void Framebuffer::clear<eig::Array<type, 3, 1>>\
    (FramebufferType t, eig::Array<type, 3, 1> v, uint i)\
    { debug::check_expr(m_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(m_object, (uint) t, i, v.data()); }\
    template <> void Framebuffer::clear<eig::Array<type, 4, 1>>\
    (FramebufferType t, eig::Array<type, 4, 1> v, uint i)\
    { debug::check_expr(m_is_init, "attempt to use an uninitialized object");\
      glClearNamedFramebuffer ## type_short ## v(m_object, (uint) t, i, v.data()); }

  // Explicit template specializations
  gl_explicit_clear_template(float, f)
  gl_explicit_clear_template(uint, ui)
  gl_explicit_clear_template(int, i)
} // namespace gl