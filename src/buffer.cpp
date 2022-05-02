#include <small_gl/buffer.hpp>
#include <small_gl/detail/exception.hpp>

namespace gl {
  namespace detail {
    GLint get_buffer_param_iv(GLuint object, GLenum name) {
      GLint value;
      glGetNamedBufferParameteriv(object, name, &value);
      return value;
    }
  } // namespace detail

  Buffer::Buffer(BufferCreateInfo info)
  : Handle<>(true), 
    _is_mapped(false),
    _flags(info.flags),
    _size(info.size > 0 ? info.size : info.data.size_bytes()) {
    detail::expr_check(_size >= info.data.size_bytes(), "buffer size is smaller than data size");

    glCreateBuffers(1, &_object);
    glNamedBufferStorage(object(), _size,info.data.data(),(uint) info.flags);

    detail::gl_check();
  }

  Buffer::~Buffer() {  
    guard(_is_init);
    glDeleteBuffers(1, &object());
  }

  void Buffer::get(std::span<std::byte> data, size_t size, size_t offset) const {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? _size : size;
    glGetNamedBufferSubData(_object, offset, safe_size, data.data());
  }

  void Buffer::set(std::span<const std::byte> data, size_t size, size_t offset) {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? _size : size;
    glNamedBufferSubData(_object, offset, safe_size, data.data());
  }
  
  void Buffer::clear(std::span<const std::byte> data, size_t stride, size_t size, size_t offset) {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");

    int intr_fmt, fmt;
    switch (stride) {
      case 1: intr_fmt = GL_R32UI, fmt = GL_RED_INTEGER; break;
      case 2: intr_fmt = GL_RG32UI; fmt = GL_RG_INTEGER; break;
      case 3: intr_fmt = GL_RGB32UI; fmt = GL_RGB_INTEGER; break;
      case 4: intr_fmt = GL_RGBA32UI; fmt = GL_RGBA_INTEGER; break;
    }
    
    size_t safe_size = (size == 0) ? _size : size;
    glClearNamedBufferSubData(_object, intr_fmt, offset, safe_size, fmt, GL_UNSIGNED_INT, data.data());
  }

  void Buffer::bind_to(BufferTargetType target, uint index, size_t size, size_t offset) const {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");

    size_t safe_size = (size == 0) ? _size : size;
    glBindBufferRange((uint) target, index, _object, offset, safe_size);
  }
  
  std::span<std::byte> Buffer::map(size_t size, size_t offset, BufferAccessFlags flags) {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");
    detail::expr_check(!_is_mapped, "attempt to map a previously mapped buffer");

    _is_mapped  = true;
    size_t safe_size = (size == 0) ? _size : size;

    // Obtain a pointer to a mapped ranger, and return this as a std::span object 
    void *data = glMapNamedBufferRange(_object, offset, safe_size, (uint) flags);
    return { reinterpret_cast<std::byte *>(data), safe_size };
  }

  void Buffer::flush(size_t size, size_t offset) {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");
    detail::expr_check(_is_mapped, "attempt to flush a unmapped buffer");

    size_t safe_size = (size == 0) ? _size : size;
    glFlushMappedNamedBufferRange(_object, offset, safe_size);
  }

  void Buffer::unmap() {
    detail::expr_check(_is_init, "attempt to use an uninitialized object");
    detail::expr_check(_is_mapped, "attempt to unmap a unmapped buffer");

    _is_mapped  = false;
    glUnmapNamedBuffer(_object);
  }

  Buffer Buffer::make_from(uint object) {
    detail::expr_check(glIsBuffer(object), "attempt to take ownership over a non-buffer handle");
    
    // Fill in object details manually
    Buffer buffer;
    buffer._is_init = true;
    buffer._object = object;
    buffer._is_mapped = detail::get_buffer_param_iv(object, GL_BUFFER_MAPPED) != GL_FALSE;
    buffer._size = detail::get_buffer_param_iv(object, GL_BUFFER_SIZE);
    buffer._flags = (BufferStorageFlags) detail::get_buffer_param_iv(object, GL_BUFFER_STORAGE_FLAGS);
    
    return buffer;
  }
} // namespace gl