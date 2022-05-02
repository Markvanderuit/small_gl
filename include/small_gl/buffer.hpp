#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/handle.hpp>
#include <span>

namespace gl {
  /**
   * Helper object to create buffer object.
   */
  struct BufferCreateInfo {
    size_t size = 0;
    std::span<const std::byte> data = { };
    BufferStorageFlags flags = { };
  };

  /**
   * Buffer object wrapping OpenGL buffer object.
   */
  struct Buffer : public detail::Handle<> {
    /* constr/destr */

    Buffer() = default;
    Buffer(BufferCreateInfo info);
    ~Buffer();

    /* getters/setters */

    inline size_t size() const { return _size; }
    inline bool is_mapped() const { return _is_mapped; }
    inline BufferStorageFlags flags() const { return _flags; }

    /* data operands */

    void get(std::span<std::byte> data, 
             size_t size = 0,
             size_t offset = 0) const;

    void set(std::span<const std::byte> data,
             size_t size = 0,
             size_t offset = 0);

    void clear(std::span<const std::byte> data = { },
               size_t stride = 1,
               size_t size = 0,
               size_t offset = 0);

    /* state */

    void bind_to(BufferTargetType target, 
                 uint index,
                 size_t size = 0,
                 size_t offset = 0) const;

    /* mapping */

    std::span<std::byte> map(size_t size = 0, 
                             size_t offset = 0,
                             BufferAccessFlags flags = { });
    void flush(size_t size = 0, size_t offset = 0);
    void unmap();
    
    /* miscellaneous */

    // Assume lifetime ownership over a provided buffer
    static Buffer make_from(uint object);

  private:
    using Base = detail::Handle<>;

    bool _is_mapped;
    size_t _size;
    BufferStorageFlags _flags;
  
  public:
    inline constexpr void swap(Buffer &o) {
      using std::swap;
      Base::swap(o);
      swap(_size, o._size);
      swap(_is_mapped, o._is_mapped);
      swap(_flags, o._flags);
    }

    inline constexpr bool operator==(const Buffer &o) const {
      using std::tie;
      return Base::operator==(o)
        && tie(_size, _is_mapped, _flags)
        == tie(o._size, o._is_mapped, o._flags);
    }

    gl_declare_noncopyable(Buffer)
  };
} // namespace gl