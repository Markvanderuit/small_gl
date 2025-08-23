#pragma once

#include <small_gl/utility.hpp>
#include <small_gl/detail/serialization.hpp>

namespace gl::detail {
  // Wrapper to encapsulate efsw file watcher library
  class FileWatcher {
    fs::file_time_type mutable m_file_time;
    fs::path                   m_file_path;

  public:
    FileWatcher() = default;
    
    FileWatcher(const fs::path &file_path)
    : m_file_path(file_path) {
      gl_trace();
      debug::check_expr(fs::exists(m_file_path));
      m_file_time = fs::last_write_time(m_file_path);
    }

    bool update() const {
      gl_trace();
      auto new_file_time = fs::last_write_time(m_file_path);
      std::swap(m_file_time, new_file_time);
      return new_file_time != m_file_time;
    }

    operator bool() const { return update(); }
  
  public: // Binary data serialization
    void to_stream(std::ostream &str) const {
      gl_trace();
      io::to_stream(m_file_time, str);
      io::to_stream(m_file_path, str);
    }

    void from_stream(std::istream &str) {
      gl_trace();
      io::from_stream(m_file_time, str);
      io::from_stream(m_file_path, str);
    }
  };
} // namespace gl::detail