#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/dispatch.hpp>
#include <small_gl/framebuffer.hpp>
#include <small_gl/program.hpp>
#include <small_gl/utility.hpp>

namespace gl {
  void dispatch_draw(DrawInfo info) {
    debug::check_expr(info.bindable_array, "DrawInfo submitted without bindable array object");
    info.bindable_array->bind();
    if (info.bindable_program) info.bindable_program->bind();
    if (info.bindable_framebuffer) info.bindable_framebuffer->bind();

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

  void dispatch_draw(DrawIndirectInfo info) {
    debug::check_expr(info.bindable_array, "DrawIndirectInfo submitted without bindable array object");
    info.bindable_array->bind();
    if (info.bindable_program) info.bindable_program->bind();
    if (info.bindable_framebuffer) info.bindable_framebuffer->bind();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, info.buffer->object());
    sync::memory_barrier(BarrierFlags::eIndirectBuffer);

    if (info.bindable_array->has_elements()) {
      glDrawElementsIndirect((uint) info.type, GL_UNSIGNED_INT, nullptr);
    } else {
      glDrawArraysIndirect((uint) info.type, nullptr);
    }
  }

  void dispatch_compute(ComputeInfo info) {
    if (info.bindable_program) info.bindable_program->bind();
    glDispatchCompute(info.groups_x, info.groups_y, info.groups_z);
  }

  void dispatch_compute(ComputeIndirectInfo info) {
    if (info.bindable_program) info.bindable_program->bind();
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, info.buffer->object());
    sync::memory_barrier(BarrierFlags::eIndirectBuffer);
    glDispatchComputeIndirect(0);
  }
} // namespace gl