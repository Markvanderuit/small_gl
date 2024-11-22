#pragma once

// Extensions to Eigen's existing classes are inserted through header files
#ifndef EIGEN_ARRAYBASE_PLUGIN
  #define EIGEN_ARRAYBASE_PLUGIN  "small_gl/detail/eigen_arraybase.ext"
#endif

#ifndef EIGEN_MATRIXBASE_PLUGIN
  #define EIGEN_MATRIXBASE_PLUGIN "small_gl/detail/eigen_matrixbase.ext"
#endif

#ifndef EIGEN_ARRAY_PLUGIN
  #define EIGEN_ARRAY_PLUGIN "small_gl/detail/eigen_array.ext"
#endif

#ifndef EIGEN_MATRIX_PLUGIN
  #define EIGEN_MATRIX_PLUGIN "small_gl/detail/eigen_matrix.ext"
#endif

#include <Eigen/Dense>

namespace gl {
  namespace eig = Eigen; // namespace shorthand
} // namespace gl

namespace Eigen {
  using Array2u = Array<unsigned int, 2, 1>;
  using Array3u = Array<unsigned int, 3, 1>;
  using Array4u = Array<unsigned int, 4, 1>;

  using Vector2u = Matrix<unsigned int, 2, 1>;
  using Vector3u = Matrix<unsigned int, 3, 1>;
  using Vector4u = Matrix<unsigned int, 4, 1>;
} // namespace Eigen