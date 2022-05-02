#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/dispatch.hpp>
#include <filesystem>

namespace gl {
  namespace fs = std::filesystem;
  
  // Load shader binary or char data from the given filepath
  std::vector<std::byte> load_shader_binary(fs::path path);
  
  // ...
} // namespace gl