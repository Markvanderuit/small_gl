#pragma once

#include <small_gl/detail/fwd.hpp>
#include <Eigen/Dense>

namespace gl {
  using Array1ui = Eigen::Array<uint, 1, 1>;
  using Array2ui = Eigen::Array<uint, 2, 1>;
  using Array3ui = Eigen::Array<uint, 3, 1>;
  using Array4ui = Eigen::Array<uint, 4, 1>;
  using ArrayXui = Eigen::Array<uint, -1, 1>;

  using Vector1ui = Eigen::Matrix<uint, 1, 1>;
  using Vector2ui = Eigen::Matrix<uint, 2, 1>;
  using Vector3ui = Eigen::Matrix<uint, 3, 1>;
  using Vector4ui = Eigen::Matrix<uint, 4, 1>;
  using VectorXui = Eigen::Matrix<uint, -1, 1>;

  using Array1i = Eigen::Array<int, 1, 1>;
  using Eigen::Array2i;
  using Eigen::Array3i;
  using Eigen::Array4i;
  using Eigen::ArrayXi;

  using Eigen::Vector2i;
  using Eigen::Vector3i;
  using Eigen::Vector4i;
  using Eigen::VectorXi;

  using Array1f = Eigen::Array<float, 1, 1>;
  using Eigen::Array2f;
  using Eigen::Array3f;
  using Eigen::Array4f;
  using Eigen::ArrayXf;

  using Eigen::Vector2f;
  using Eigen::Vector3f;
  using Eigen::Vector4f;
  using Eigen::VectorXf;

  using Eigen::Array22i;
  using Eigen::Array33i;
  using Eigen::Array44i;
  using Eigen::ArrayXXi;

  using Eigen::Matrix2i;
  using Eigen::Matrix3i;
  using Eigen::Matrix4i;
  using Eigen::MatrixXi;

  using Eigen::Array22f;
  using Eigen::Array33f;
  using Eigen::Array44f;
  using Eigen::ArrayXXf;

  using Eigen::Matrix2f;
  using Eigen::Matrix3f;
  using Eigen::Matrix4f;
  using Eigen::MatrixXf;

  /* Default initializer values for some functions; avoid evaluating Eigen's UnaryOp */

  template <typename T> inline T zero_value();

#define gl_explicit_zero_value(array_type, short_type, value)\
  template <> inline array_type ## 2 ## short_type zero_value<array_type ## 2 ## short_type>()\
  { return array_type ## 2 ## short_type::Zero(); }\
  template <> inline array_type ## 3 ## short_type zero_value<array_type ## 3 ## short_type>()\
  { return array_type ## 3 ## short_type::Zero(); }\
  template <> inline array_type ## 4 ## short_type zero_value<array_type ## 4 ## short_type>()\
  { return array_type ## 4 ## short_type::Zero(); }

  template <> inline float zero_value<float>() { return 0.f; }
  template <> inline int zero_value<int>() { return 0; }
  template <> inline uint zero_value<uint>() { return 0; }
  template <> inline short zero_value<short>() { return 0; }
  template <> inline ushort zero_value<ushort>() { return 0; }
  template <> inline char zero_value<char>() { return 0; }
  template <> inline uchar zero_value<uchar>() { return 0; }

  gl_explicit_zero_value(Array, i, 0)
  gl_explicit_zero_value(Vector, i, 0)
  gl_explicit_zero_value(Array, f, 0.f)
  gl_explicit_zero_value(Vector, f, 0.f)
} // namespace gl