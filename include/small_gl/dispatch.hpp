#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/enum.hpp>
#include <optional>
#include <vector>

namespace gl {
  /**
   * Helper object to dispatch a draw operation for the current context.
   */
  struct DrawInfo {
    // Draw information
    PrimitiveType type;

    // Vertex range data
    uint vertex_count = 0;
    uint vertex_first = 0;

    // Instancing data
    uint instance_count = 0;
    uint vertex_base    = 0;
    uint instance_base  = 0;

    // Specific state data; will affect active state before draw
    std::vector<std::pair<DrawCapability, bool>> capabilities = { };
    std::optional<DrawOp>                        draw_op      = { };
    std::optional<LogicOp>                       logic_op     = { };
    std::optional<CullOp>                        cull_op      = { };
    std::optional<DepthOp>                       depth_op     = { };
    std::optional<std::pair<BlendOp, BlendOp>>   blend_op     = { }; // { src, dst }
    
    // Bindables; will be bound before draw
    const Array       *bindable_array       = nullptr; // required
    const Program     *bindable_program     = nullptr; // optional
    const Framebuffer *bindable_framebuffer = nullptr; // optional
  };

  /**
   * Helper object to dispatch a draw operation using an indirect buffer object.
   */
  struct DrawIndirectInfo {
    // Draw information
    PrimitiveType type;

    // Indirect buffer
    const Buffer *buffer;

    // Specific state data; will override active state before draw
    std::vector<std::pair<DrawCapability, bool>> capabilities = { };
    std::optional<DrawOp>                        draw_op      = { };
    std::optional<LogicOp>                       logic_op     = { };
    std::optional<CullOp>                        cull_op      = { };
    std::optional<DepthOp>                       depth_op     = { };
    std::optional<std::pair<BlendOp, BlendOp>>   blend_op     = { }; // { src, dst }
    
    // Bindables; will be bound before draw
    const Array       *bindable_array       = nullptr; // required
    const Program     *bindable_program     = nullptr; // optional
    const Framebuffer *bindable_framebuffer = nullptr; // optional
  };

  /**
   * Helper object to dispatch several draw operations for the current context.
   */
  struct MultiDrawInfo {
    struct DrawCommand {
      // Vertex range data
      uint vertex_count = 0;
      uint vertex_first = 0;

      // Instancing data
      uint instance_count = 1;
      uint vertex_base    = 0;
      uint instance_base  = 0;
    };

    // Draw information
    PrimitiveType type;

    // List of draw commands passed to MultiDrawElements
    std::vector<DrawCommand> commands = { };

    // Specific state data; will affect active state before draw
    std::vector<std::pair<DrawCapability, bool>> capabilities = { };
    std::optional<DrawOp>                        draw_op      = { };
    std::optional<LogicOp>                       logic_op     = { };
    std::optional<CullOp>                        cull_op      = { };
    std::optional<DepthOp>                       depth_op     = { };
    std::optional<std::pair<BlendOp, BlendOp>>   blend_op     = { }; // { src, dst }
    
    // Bindables; will be bound before draw
    const Array       *bindable_array       = nullptr; // required
    const Program     *bindable_program     = nullptr; // optional
    const Framebuffer *bindable_framebuffer = nullptr; // optional
  };

  /**
   * Helper object to dispatch a compute operation for the current context.
   */
  struct ComputeInfo {
    // Dispatch dimensions
    uint groups_x = 1;
    uint groups_y = 1;
    uint groups_z = 1;
    
    // Optional bindables will be bound before draw
    const Program *bindable_program = nullptr;
  };

  /**
   * Helper object to dispatch a compute operation using an indirect buffer object.
   */
  struct ComputeIndirectInfo {
    // Indirect buffer
    const Buffer *buffer;

    // Optional bindable; will be bound before draw
    const Program *bindable_program = nullptr;
  };
  
  // Dispatch a draw/compute operation
  void dispatch_draw(const DrawInfo               &info);
  void dispatch_draw(const DrawIndirectInfo       &info);
  void dispatch_multidraw(const MultiDrawInfo     &info);
  void dispatch_compute(const ComputeInfo         &info);
  void dispatch_compute(const ComputeIndirectInfo &info); 
} // namespace gl