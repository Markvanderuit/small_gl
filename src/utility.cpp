#include <small_gl/utility.hpp>
#include <small_gl/detail/exception.hpp>
#include <fmt/format.h>
#include <fstream>

namespace gl {
  std::vector<std::byte> load_shader_binary(fs::path path) {
    // Check that file path exists
    detail::expr_check(fs::exists(path),
      fmt::format("failed to resolve path \"{}\"", path.string()));

    // Attempt to open file stream
    std::ifstream ifs(path, std::ios::ate | std::ios::binary);
    detail::expr_check(ifs.is_open(),
      fmt::format("failed to open file \"{}\"", path.string()));

    // Read file size and construct vector to hold data
    size_t file_size = static_cast<size_t>(ifs.tellg());
    std::vector<std::byte> buffer(file_size);
    
    // Set inpout position to start, then read full file into vector
    ifs.seekg(0);
    ifs.read((char *) buffer.data(), file_size);
    ifs.close();
    
    return buffer;
  }
} // namespace gl