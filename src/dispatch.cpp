#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/dispatch.hpp>
#include <small_gl/framebuffer.hpp>
#include <small_gl/program.hpp>
#include <small_gl/utility.hpp>
#include <ranges>

namespace gl {
  namespace detail {
    void handle_info_binds(const auto &info) {
      gl_trace_full();

      // Bind array object, which is required
      debug::check_expr(info.bindable_array, 
        "DrawInfo submitted without bindable array object");
      info.bindable_array->bind();

      // Bind optional program and target framebuffer
      if (info.bindable_program)     info.bindable_program->bind();
      if (info.bindable_framebuffer) info.bindable_framebuffer->bind();
    }

    void handle_info_ops(const auto &info) {
      gl_trace_full();

      // Pass through provided state data
      if (info.draw_op)  state::set_op(*info.draw_op);
      if (info.logic_op) state::set_op(*info.logic_op);
      if (info.cull_op)  state::set_op(*info.cull_op);
      if (info.depth_op) state::set_op(*info.depth_op);
      if (info.blend_op) state::set_op(info.blend_op->first, info.blend_op->second);
    }
  } // namespace detail

  void dispatch_draw(const DrawInfo &info) {
    gl_trace_full();

    // Pass through provided data
    detail::handle_info_binds(info);
    detail::handle_info_ops(info);

    // Set scoped state capabilities
    std::vector<state::ScopedSet> scoped_state;
    for (auto [key, value] : info.capabilities)
      scoped_state.emplace_back(key, value);

    // Dispatch relevant draw call given array object's situation
    if (info.bindable_array->has_elements()) {
      if (info.instance_count > 0) {
        glDrawElementsInstancedBaseVertexBaseInstance(
          (uint) info.type, info.vertex_count, GL_UNSIGNED_INT, 
          (void *) (sizeof(uint) * info.vertex_first), 
          info.instance_count,  info.vertex_base, info.instance_base);
      } else {
        glDrawElementsBaseVertex(
          (uint) info.type, info.vertex_count, GL_UNSIGNED_INT,
          (void *) (sizeof(uint) * info.vertex_first), info.vertex_base);
      }
    } else {
      if (info.instance_count > 0) {
        glDrawArraysInstancedBaseInstance(
          (uint) info.type, info.vertex_first, info.vertex_count, 
          info.instance_count, info.instance_base);
      } else {
        glDrawArrays((uint) info.type, info.vertex_first, info.vertex_count);
      }
    }
  }

  void dispatch_draw(const DrawIndirectInfo &info) {
    gl_trace_full();

    // Pass through provided data
    detail::handle_info_binds(info);
    detail::handle_info_ops(info);

    // Set scoped state capabilities
    std::vector<state::ScopedSet> scoped_state;
    for (auto [key, value] : info.capabilities)
      scoped_state.emplace_back(key, value);

    // Bind supplied buffer object to indirect handle
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, info.buffer->object());
    sync::memory_barrier(BarrierFlags::eIndirectBuffer);

    // Dispatch relevant draw call given array object's situation
    if (info.bindable_array->has_elements()) {
      glDrawElementsIndirect((uint) info.type, GL_UNSIGNED_INT, nullptr);
    } else {
      glDrawArraysIndirect((uint) info.type, nullptr);
    }
  }

  void dispatch_multidraw(const MultiDrawInfo &info) {
    gl_trace_full();

    // Pass through provided data
    detail::handle_info_binds(info);
    detail::handle_info_ops(info);

    // Set scoped state capabilities
    std::vector<state::ScopedSet> scoped_state;
    for (auto [key, value] : info.capabilities)
      scoped_state.emplace_back(key, value);

    if (info.bindable_array->has_elements()) {
      struct IndirectElementCommand {
        uint count, instanceCount, first;
        int baseVertex;
        uint baseInstance;
      };
      std::vector<IndirectElementCommand> data(info.commands.size());
      std::ranges::transform(info.commands, data.begin(), [](auto i) { return IndirectElementCommand {
          .count         = i.vertex_count,
          .instanceCount = i.instance_count,
          .first         = i.vertex_first,
          .baseVertex    = (int) (i.vertex_base),
          .baseInstance  = i.instance_base
      }; });
      glMultiDrawElementsIndirect((uint) info.type, GL_UNSIGNED_INT, data.data(), data.size(), 0);
    } else {
      struct IndirectArrayCommand {
        uint count, instanceCount, first, baseInstance;
      };
      std::vector<IndirectArrayCommand> data(info.commands.size());
      std::ranges::transform(info.commands, data.begin(), [](auto i) { return IndirectArrayCommand {
          .count         = i.vertex_count,
          .instanceCount = i.instance_count,
          .first         = i.vertex_first,
          .baseInstance  = i.instance_base
      }; });
      glMultiDrawArraysIndirect((uint) info.type, data.data(), data.size(), 0);
    }
  }

  void dispatch_compute(const ComputeInfo &info) {
    gl_trace_full();
    if (info.bindable_program) info.bindable_program->bind();
    glDispatchCompute(info.groups_x, info.groups_y, info.groups_z);
  }

  void dispatch_compute(const ComputeIndirectInfo &info) {
    gl_trace_full();
    if (info.bindable_program) info.bindable_program->bind();
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, info.buffer->object());
    sync::memory_barrier(BarrierFlags::eIndirectBuffer);
    glDispatchComputeIndirect(0);
  }
} // namespace gl