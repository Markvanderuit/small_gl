#pragma once

#include <small_gl/detail/fwd.hpp>

namespace gl {
  /**
   * Helper object to dispatch a draw operation.
   */
  struct DrawInfo {
    // Draw information
    PrimitiveType type;
    const Array *array;

    // Vertex range data
    uint vertex_count;
    uint vertex_first = 0;

    // Instancing data
    uint instance_count = 0;
    uint vertex_base = 0;
    uint instance_base = 0;

    // Optional bindable program
    const Program *program = nullptr;
  };

  /**
   * Helper object to dispatch a draw operation using an indirect buffer object.
   */
  struct DrawIndirectInfo {
    // Draw information
    PrimitiveType type;
    const Array *array;

    // Indirect buffer
    const Buffer *buffer;

    // Optional bindable program
    const Program *program = nullptr; 
  };

  /**
   * Helper object to dispatch a compute operation.
   */
  struct ComputeInfo {
    // Dispatch dimensions
    uint groups_x = 1;
    uint groups_y = 1;
    uint groups_z = 1;
    
    // Optional bindable program
    const Program *program = nullptr;
  };

  /**
   * Helper object to dispatch a compute operation using an indirect buffer object.
   */
  struct ComputeIndirectInfo {
    // Indirect buffer
    const Buffer *buffer;

    // Optional bindable program
    const Program *program = nullptr;
  };

  
  // Dispatch a draw/compute operation
  void dispatch(DrawInfo info);
  void dispatch(DrawIndirectInfo info);
  void dispatch(ComputeInfo info);
  void dispatch(ComputeIndirectInfo info); 
} // namespace gl