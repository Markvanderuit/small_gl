#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/handle.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/dispatch.hpp>
#include <span>

namespace gl {
  /**
   * Helper object to create buffer object.
   */
  struct BufferCreateInfo {
    // Size of the buffer, in bytes
    size_t size = 0;

    // Non-owning span to data passed into buffer
    std::span<const std::byte> data = { };

    // Remainder of settings
    BufferCreateFlags flags = { };
  };

  /**
   * Buffer object wrapping OpenGL buffer object.
   */
  class Buffer : public detail::Handle<> {
    using Base = detail::Handle<>;

    bool m_is_mapped;
    size_t m_size;
    BufferCreateFlags m_flags;

  public:
    /* constr/destr */

    Buffer() = default;
    Buffer(BufferCreateInfo info);
    ~Buffer();

    /* getters/setters */

    inline size_t size() const { return m_size; }
    inline bool is_mapped() const { return m_is_mapped; }
    inline BufferCreateFlags flags() const { return m_flags; }

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

    // Map a region of the buffer; returns a non-owning span over the mapped region
    std::span<std::byte> map(size_t size = 0, size_t offset = 0, BufferAccessFlags flags = { });

    // Unmap (all mapped regions of) the buffer
    void unmap();

    // Flush a mapped region of the buffer
    void flush(size_t size = 0, size_t offset = 0);
    
    /* miscellaneous */

    // Assume lifetime ownership over a provided buffer handle
    static Buffer make_from(uint object);

    // Create a indirect buffer object from a gl::DrawInfo object
    static Buffer make_indirect(DrawInfo info, BufferCreateFlags flags = { });

    // Create a indirect buffer object from a gl::ComputeInfo object
    static Buffer make_indirect(ComputeInfo info, BufferCreateFlags flags = { });
  
    inline constexpr void swap(Buffer &o) {
      using std::swap;
      Base::swap(o);
      swap(m_size, o.m_size);
      swap(m_is_mapped, o.m_is_mapped);
      swap(m_flags, o.m_flags);
    }

    inline constexpr bool operator==(const Buffer &o) const {
      using std::tie;
      return Base::operator==(o)
        && tie(m_size, m_is_mapped, m_flags)
        == tie(o.m_size, o.m_is_mapped, o.m_flags);
    }

    gl_declare_noncopyable(Buffer)
  };
} // namespace gl