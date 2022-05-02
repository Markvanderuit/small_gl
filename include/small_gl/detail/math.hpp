#pragma once

#include <Eigen/Dense>

namespace gl {
  using uint = unsigned int;
  using uchar = unsigned char;
  using ushort = unsigned short;

  /* 
    Map common vector/matrix math types into program's namespace
  */

  namespace eig = Eigen;

  using Array1ui = eig::Array<uint, 1, 1>;
  using Array2ui = eig::Array<uint, 2, 1>;
  using Array3ui = eig::Array<uint, 3, 1>;
  using Array4ui = eig::Array<uint, 4, 1>;
  using ArrayXui = eig::Array<uint, -1, 1>;

  using Vector1ui = eig::Matrix<uint, 1, 1>;
  using Vector2ui = eig::Matrix<uint, 2, 1>;
  using Vector3ui = eig::Matrix<uint, 3, 1>;
  using Vector4ui = eig::Matrix<uint, 4, 1>;
  using VectorXui = eig::Matrix<uint, -1, 1>;

  using Array1i = eig::Array<int, 1, 1>;
  using eig::Array2i;
  using eig::Array3i;
  using eig::Array4i;
  using eig::ArrayXi;

  using eig::Vector2i;
  using eig::Vector3i;
  using eig::Vector4i;
  using eig::VectorXi;

  using Array1f = eig::Array<float, 1, 1>;
  using eig::Array2f;
  using eig::Array3f;
  using eig::Array4f;
  using eig::ArrayXf;

  using eig::Vector2f;
  using eig::Vector3f;
  using eig::Vector4f;
  using eig::VectorXf;

  using eig::Array22i;
  using eig::Array33i;
  using eig::Array44i;
  using eig::ArrayXXi;

  using eig::Matrix2i;
  using eig::Matrix3i;
  using eig::Matrix4i;
  using eig::MatrixXi;

  using eig::Array22f;
  using eig::Array33f;
  using eig::Array44f;
  using eig::ArrayXXf;

  using eig::Matrix2f;
  using eig::Matrix3f;
  using eig::Matrix4f;
  using eig::MatrixXf;
} // namespace gl