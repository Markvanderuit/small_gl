#include <small_gl/detail/exception.hpp>
#include <small_gl/array.hpp>
#include <small_gl/dispatch.hpp>
#include <small_gl/program.hpp>
#include <small_gl/utility.hpp>

namespace gl {
  void dispatch(DrawInfo info) {
    detail::expr_check(info.array, "DrawInfo submitted without array object");
    info.array->bind();
    if (info.program) info.program->bind();

    if (info.array->has_elements()) {
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

  void dispatch(DrawIndirectInfo info) {
    detail::expr_check(info.array, "DrawIndirectInfo submitted without array object");
    info.array->bind();
    if (info.program) info.program->bind();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, info.buffer->object());
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

    if (info.array->has_elements()) {
      glDrawElementsIndirect((uint) info.type, GL_UNSIGNED_INT, nullptr);
    } else {
      glDrawArraysIndirect((uint) info.type, nullptr);
    }
  }

  void dispatch(ComputeInfo info) {
    if (info.program) info.program->bind();
    glDispatchCompute(info.groups_x, info.groups_y, info.groups_z);
  }

  void dispatch(ComputeIndirectInfo info) {
    if (info.program) info.program->bind();
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, info.buffer->object());
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
    glDispatchComputeIndirect(0);
  }
} // namespace gl