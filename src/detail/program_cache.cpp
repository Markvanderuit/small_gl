#include <small_gl/detail/program_cache.hpp>
#include <small_gl/program.hpp>
#include <zstr.hpp>
#include <algorithm>
#include <ranges>
#include <sstream>

namespace gl {
  // Ranges shorthands
  namespace rng = std::ranges;
  namespace vws = std::views;
  
  namespace io {
    // Serialization for std::unordered_map<std::string,...>
    template <typename Ty> /* requires(!is_serializable<Ty>) */
    void to_stream(const std::unordered_map<std::string, Ty> &v, std::ostream &str) {
      gl_trace();
      
      to_stream(v.size(), str);
      for (const auto &[key, value] : v) {
        to_stream(key, str);
        to_stream(value, str);
      }
    }

    // Serialization for std::unordered_map<std::string,...>
    template <typename Ty> /* requires (!is_serializable<Ty>) */
    void from_stream(std::unordered_map<std::string, Ty> &v, std::istream &str) {
      gl_trace();

      size_t n;
      from_stream(n, str);
      for (size_t i = 0; i < n; ++i) {
        std::string key;
        Ty value;
        from_stream(key, str);
        from_stream(value, str);
        v.insert({ key, std::move(value) });
      }
    }
  } // namespace io

  namespace detail {
    std::pair<std::string, gl::Program &> ProgramCache::set(InfoType &&info) {
      gl_trace();

      // Visitor generates key, and tests if program exists in cache
      auto key = std::visit([](const auto &i) { return i.to_string(); }, info);
      auto it  = m_prog_cache.find(key);

      // If program is not in cache
      if (it == m_prog_cache.end()) {
        // Visitor generates program
        auto prog = std::visit([](const auto &i) { return gl::Program(i); }, info);
        
        // Program is newly cached
        it = m_prog_cache.emplace(key, std::move(prog)).first;
      }

      return { key, it->second };
    }

    std::pair<std::string, gl::Program &> ProgramCache::set(InfoList &&info) {
      gl_trace();

      // Visitor generates key, and tests if program exists in cache,
      // by joining consecutive info objects as keys
      auto key = std::visit([](const auto &l) {
        auto in = l | vws::transform([](const auto &i) { return i.to_string(); }) | vws::join;
        std::string out;
        rng::copy(in, std::back_inserter(out));
        return out;
      }, info);
      auto it  = m_prog_cache.find(key);

      // If program is not in cache
      if (it == m_prog_cache.end()) {
        // Visitor generates program from list
        auto prog = std::visit([](const auto &l) { return gl::Program(l); }, info);
        
        // Program is are newly cached
        auto cval = std::visit([](const auto &l) { 
          auto in = l | vws::transform([](const auto &t) { return InfoType(t); });
          std::vector<InfoType> out;
          rng::copy(in, std::back_inserter(out));
          return out;
        }, info);
        it = m_prog_cache.emplace(key, std::move(prog)).first;
      }

      return { key, it->second };
    }

    gl::Program & ProgramCache::at(const KeyType &k) {
      gl_trace();
      auto f = m_prog_cache.find(k);
      debug::check_expr(f != m_prog_cache.end(),
        fmt::format("ProgramCache::at(...) failed with key lookup for key: \"{}\"", k));
      return f->second;
    }

    void ProgramCache::clear() {
      gl_trace();
      m_prog_cache.clear();
    }

    ProgramCache::ProgramCache(fs::path cache_file_path) { load(cache_file_path); }

    void ProgramCache::save(fs::path cache_file_path) const {
      gl_trace();

      // Attempt opening zlib compressed stream
      constexpr auto str_flags = std::ios::out | std::ios::binary | std::ios::trunc;
      auto str = zstr::ofstream(cache_file_path.string(), str_flags, Z_BEST_SPEED);
      debug::check_expr(str.good());

      // Serialize program cache to stream
      io::to_stream(m_prog_cache, str);

      // Output OpenGL debug message to warn of cache save
      debug::insert_message(
        fmt::format("Program cache saved to: {}", cache_file_path.string()), 
        gl::DebugMessageSeverity::eLow);
    }

    void ProgramCache::load(fs::path cache_file_path) {
      gl_trace();
      
      // Sanity check file path
      debug::check_expr(fs::exists(cache_file_path),
        fmt::format("Program cache cannot load; cache does not exist at: {}", cache_file_path.string()));

      // Clear out cache first
      *this = { };

      // Next, attempt opening zlib compressed stream, and deserialize to scene object
      constexpr auto str_flags = std::ios::in  | std::ios::binary;
      auto str = zstr::ifstream(cache_file_path.string(), str_flags);
      debug::check_expr(str.good());

      // Deserialize program cache from stream
      io::from_stream(m_prog_cache, str);

      // Output OpenGL debug message to warn of cache load
      debug::insert_message(
        fmt::format("Program cache loaded from: {}", cache_file_path.string()), 
        gl::DebugMessageSeverity::eLow);
    }
  } // namespace detail
} // namespace gl