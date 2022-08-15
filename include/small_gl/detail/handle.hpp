#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/trace.hpp>
#include <utility>
#include <tuple>

// For class T, declare swap-based move constr/operator
// and delete copy constr/operators, making T non-copyable
#define gl_declare_noncopyable(T)\
  T(const T &) = delete;\
  T & operator= (const T &) = delete;\
  T(T &&o) noexcept { gl_trace(); swap(o); }\
  inline T & operator= (T &&o) noexcept { gl_trace(); swap(o); return *this; }

namespace gl::detail {
  /**
   * Virtual handle class for OpenGL objects. Does not manage lifetime of the underlying object, 
   * but allows for querying of its state. Implementing classes must still init/destroy this 
   * object. The class is non-copyable, as are any implementing classes.
   */
  template <typename T = uint>
  struct Handle {
    /* public getters/setters */

    T object() const { return m_object; }
    T& object() { return m_object; }
    bool is_init() const { return m_is_init; }

  protected:
    /* protected object data */

    bool m_is_init = false;
    T m_object = 0;

    /* protected constr and _virtual_ destr */

    constexpr Handle() = default;
    constexpr Handle(bool init) noexcept : m_is_init(init) { }
    constexpr virtual ~Handle() = default;

    /* miscellaneous */

    // Assume lifetime ownership over a provided object
    static inline constexpr Handle make_from(T object) {
      return { true, object };
    }

    inline constexpr void swap(Handle &o) {
      using std::swap;
      swap(m_is_init, o.m_is_init);
      swap(m_object, o.m_object);
    }

    inline constexpr bool operator==(const Handle &o) const {
      using std::tie;
      return tie(m_is_init, m_object) 
          == tie(o.m_is_init, o.m_object);
    }

    gl_declare_noncopyable(Handle)
  };
} // namespace gl::detail