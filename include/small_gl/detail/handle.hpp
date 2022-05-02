#pragma once

#include <small_gl/detail/fwd.hpp>

namespace gl {
  template <typename T = uint>
  struct Handle {
    /* public getters/setters */

    T Object() const { return _object; }
    T& Object() { return _object; }
    bool is_init() const { return _is_init; }

  protected:
    /* protected objects */

    bool _is_init = false;
    T _object = 0;

    /* protected constr/virtual destr */

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
} // namespace gl