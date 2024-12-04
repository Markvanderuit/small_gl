#pragma once

#include <small_gl/program.hpp>
#include <variant>
#include <unordered_map>

namespace gl::detail {
  /**
   * Helper object to initialize, store and load program objects
   * based on their construction objects, to avoid unnecessary
   * recreation of some heavier program objects.
   */
  class ProgramCache {
    using KeyType  = std::string;
    using InfoType = ShaderLoadFileInfo; 
    using InfoList = std::initializer_list<ShaderLoadFileInfo>;
    
    // Data caches
    std::unordered_map<KeyType, std::vector<InfoType>> m_info_cache;
    std::unordered_map<KeyType, Program>               m_prog_cache;

  public:
    // Default constructor
    ProgramCache() = default;

    // File path constructor; construct cache internals from file, if file exists
    ProgramCache(fs::path cache_file_path);

  public:
    // Initialize-and-return a reference to a program object
    std::pair<KeyType, gl::Program&> set(InfoType &&info); 
    std::pair<KeyType, gl::Program&> set(InfoList &&info); 

    // Return an existing program for a given key
    gl::Program& at(const KeyType &k);

    // Clear out program cache
    void clear();

    // Save cache internals to file
    void save(fs::path cache_file_path) const;
    
    // Overwrite cache internals from file, if file exists
    void load(fs::path cache_file_path);
  };
} // namespace gl::detail