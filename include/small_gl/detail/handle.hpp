#pragma once

#include <small_gl/detail/fwd.hpp>
#include <utility>
#include <tuple>

// For class T, declare swap-based move constr/operator
// and delete copy cosntr/operators, making T non-copyable
#define gl_declare_noncopyable(T)\
  T(const T &) = delete;\
  T & operator= (const T &) = delete;\
  T(T &&o) noexcept { swap(o); }\
  inline T & operator= (T &&o) noexcept { swap(o); return *this; }

namespace gl::detail {
  /**
   * Virtual handle class for OpenGL objects. Does not manage lifetime of the underlying object, 
   * but allows for querying of its state. Implementing classes must still init/destroy this 
   * object. The class is non-copyable, as are any implementing classes.
   */
  template <typename T = uint>
  struct Handle {
    /* public getters/setters */

    T object() const { return _object; }
    T& object() { return _object; }
    bool is_init() const { return _is_init; }

  protected:
    /* protected object data */

    bool _is_init = false;
    T _object = 0;

    /* protected constr and _virtual_ destr */

    constexpr Handle() = default;
    constexpr Handle(bool init) noexcept : _is_init(init) { }
    constexpr virtual ~Handle() = default;

    /* miscellaneous */

    // Assume lifetime ownership over a provided object
    static inline constexpr Handle make_from(T object) {
      return { true, object };
    }

    inline constexpr void swap(Handle &o) {
      using std::swap;
      swap(_is_init, o._is_init);
      swap(_object, o._object);
    }

    inline constexpr bool operator==(const Handle &o) const {
      using std::tie;
      return tie(_is_init, _object) 
          == tie(o._is_init, o._object);
    }

    gl_declare_noncopyable(Handle)
  };
} // namespace gl::detail