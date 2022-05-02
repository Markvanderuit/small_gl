#pragma once

// Simple guard statement syntactic sugar
#define guard(expr,...) if (!(expr)) { return __VA_ARGS__ ; }
#define guard_continue(expr) if (!(expr)) { continue; }
#define guard_break(expr) if (!(expr)) { break; }

// Declare swap-based move constr/operators for object T
// and delete copy constr/operators, making T non-copyable
#define gl_declare_noncopyable(T)\
  T(const T &) = delete;\
  T & operator= (const T &) = delete;\
  T(T &&o) noexcept { swap(o); }\
  inline T & operator= (T &&o) noexcept { swap(o); return *this; }

// Declare bitflag operators and has_flag(T, T) for enum class T
#define gl_declare_bitflag(T)\
  constexpr T operator~(T a) { return (T) (~ (uint) a); }\
  constexpr T operator|(T a, T b) { return (T) ((uint) a | (uint) b); }\
  constexpr T operator&(T a, T b) { return (T) ((uint) a & (uint) b); }\
  constexpr T operator^(T a, T b) { return (T) ((uint) a ^ (uint) b); }\
  constexpr T& operator|=(T &a, T b) { return a = a | b; }\
  constexpr T& operator&=(T &a, T b) { return a = a & b; }\
  constexpr T& operator^=(T &a, T b) { return a = a ^ b; }\
  constexpr bool has_flag(T flags, T t) { return (uint) (flags & t) != 0u; }