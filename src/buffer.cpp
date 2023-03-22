#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/utility.hpp>
#include <array>
#include <string>

namespace gl {
  namespace detail {
    GLint get_buffer_param_iv(GLuint object, GLenum name) {
      GLint value;
      glGetNamedBufferParameteriv(object, name, &value);
      return value;
    }
  } // namespace detail

  Buffer::Buffer(BufferCreateInfo info)
  : Base(true), 
    m_is_mapped(false),
    m_flags(info.flags),
    m_size(info.size > 0 ? info.size : info.data.size_bytes()) {
    gl_trace_full();
    debug::check_expr_dbg(m_size >= info.data.size_bytes(), "buffer size is smaller than data size");
    
    glCreateBuffers(1, &m_object);
    glNamedBufferStorage(object(), m_size, info.data.data(), (uint) info.flags);
    gl_trace_gpu_alloc("gl::Buffer", object(), m_size);
  }

  Buffer::~Buffer() {  
    gl_trace_full();
    guard(m_is_init);
  
    if (m_is_mapped) 
      unmap();
    
    gl_trace_gpu_free("gl::Buffer", object());
    glDeleteBuffers(1, &object());
  }

  void Buffer::get(std::span<std::byte> data, size_t size, size_t offset) const {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? m_size : size;
    glGetNamedBufferSubData(m_object, offset, safe_size, data.data());
  }

  void Buffer::set(std::span<const std::byte> data, size_t size, size_t offset) {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? m_size : size;
    glNamedBufferSubData(m_object, offset, safe_size, data.data());
  }
  
  void Buffer::clear(std::span<const std::byte> data, size_t stride, size_t size, size_t offset) {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    int intr_fmt, fmt;
    switch (stride) {
      case 1: intr_fmt = GL_R32UI, fmt = GL_RED_INTEGER; break;
      case 2: intr_fmt = GL_RG32UI; fmt = GL_RG_INTEGER; break;
      case 3: intr_fmt = GL_RGB32UI; fmt = GL_RGB_INTEGER; break;
      case 4: intr_fmt = GL_RGBA32UI; fmt = GL_RGBA_INTEGER; break;
    }
    
    size_t safe_size = (size == 0) ? m_size : size;
    glClearNamedBufferSubData(m_object, intr_fmt, offset, safe_size, fmt, GL_UNSIGNED_INT, data.data());
  }

  void Buffer::bind_to(BufferTargetType target, uint index, size_t size, size_t offset) const {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? m_size : size;
    glBindBufferRange((uint) target, index, m_object, offset, safe_size);
  }

  void Buffer::copy_to(gl::Buffer &dst, size_t size, size_t src_offset, size_t dst_offset) const {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? m_size : size;
    glCopyNamedBufferSubData(m_object, dst.object(), src_offset, dst_offset, size);
  }

  std::span<std::byte> Buffer::map(BufferAccessFlags flags, size_t size, size_t offset) {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    debug::check_expr_dbg(!m_is_mapped, "attempt to map a previously mapped buffer");
    debug::check_expr_dbg((uint) flags != 0, "Buffer::map() requires at least some access flags as an argument");

    // Check if buffer create flags at least superseed buffer access flags
    debug::check_expr_dbg(!has_flag(flags, BufferAccessFlags::eMapRead)
                    || has_flag(m_flags, BufferCreateFlags::eMapRead)
                    == has_flag(flags, BufferAccessFlags::eMapRead),
      "Buffer::map() requested read access; this was not specified during buffer creation");
    debug::check_expr_dbg(!has_flag(flags, BufferAccessFlags::eMapWrite)
                    || has_flag(m_flags, BufferCreateFlags::eMapWrite)
                    == has_flag(flags, BufferAccessFlags::eMapWrite),
      "Buffer::map() requested write access; this was not specified during buffer creation");
    debug::check_expr_dbg(!has_flag(flags, BufferAccessFlags::eMapCoherent)
                    || has_flag(m_flags, BufferCreateFlags::eMapCoherent)
                    == has_flag(flags, BufferAccessFlags::eMapCoherent),
      "Buffer::map() requested coherent access; this was not specified during buffer creation");
    debug::check_expr_dbg(!has_flag(flags, BufferAccessFlags::eMapPersistent)
                    || has_flag(m_flags, BufferCreateFlags::eMapPersistent)
                    == has_flag(flags, BufferAccessFlags::eMapPersistent),
      "Buffer::map() requested persistent access; this was not specified during buffer creation");

    m_is_mapped  = true;
    size_t safe_size = (size == 0) ? m_size : size;

    // Obtain a pointer to a mapped ranger, and return this as a std::span object 
    void *data = glMapNamedBufferRange(m_object, offset, safe_size, (uint) flags);
    return { reinterpret_cast<std::byte *>(data), safe_size };
  }

  void Buffer::flush(size_t size, size_t offset) {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    debug::check_expr_dbg(m_is_mapped, "attempt to flush a unmapped buffer");

    size_t safe_size = (size == 0) ? m_size : size;
    glFlushMappedNamedBufferRange(m_object, offset, safe_size);
  }

  void Buffer::unmap() {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    debug::check_expr_dbg(m_is_mapped, "attempt to unmap a unmapped buffer");

    m_is_mapped  = false;
    glUnmapNamedBuffer(m_object);
  }

  Buffer Buffer::make_from(uint object) {
    gl_trace_full();
    debug::check_expr_dbg(glIsBuffer(object), "attempt to take ownership over a non-buffer handle");
    
    // Fill in object details manually
    Buffer buffer;
    buffer.m_is_init = true;
    buffer.m_object = object;
    buffer.m_is_mapped = detail::get_buffer_param_iv(object, GL_BUFFER_MAPPED) != GL_FALSE;
    buffer.m_size = detail::get_buffer_param_iv(object, GL_BUFFER_SIZE);
    buffer.m_flags = (BufferCreateFlags) detail::get_buffer_param_iv(object, GL_BUFFER_STORAGE_FLAGS);

    gl_trace_gpu_alloc("gl::Buffer", object, buffer.m_size);
    
    return buffer;
  }

  Buffer Buffer::make_indirect(DrawInfo info, BufferCreateFlags flags) {
    gl_trace_full();
    debug::check_expr_dbg(info.bindable_array, "DrawInfo submitted without bindable array object");

    if (info.bindable_array->has_elements()) {
      std::array<uint, 5> data = { info.vertex_count, info.instance_count, 
                                   info.vertex_first, info.vertex_base,
                                   info.instance_base };
      return Buffer({ .size = data.size() * sizeof(uint),
                      .data = std::as_bytes(std::span(data)),
                      .flags = flags });
    } else {
      std::array<uint, 4> data = { info.vertex_count, info.instance_count, 
                                   info.vertex_first, info.instance_base };
      return Buffer({ .size = data.size() * sizeof(uint),
                      .data = std::as_bytes(std::span(data)),
                      .flags = flags });
    }
  }

  Buffer Buffer::make_indirect(ComputeInfo info, BufferCreateFlags flags) {
    gl_trace_full();
    std::array<uint, 3> data = { info.groups_x, info.groups_y, info.groups_z };
    return Buffer({ .size = data.size() * sizeof(uint),
                    .data = std::as_bytes(std::span(data)),
                    .flags = flags });
  }
} // namespace gl