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
    using InfoType = std::variant<ShaderLoadSPIRVInfo, ShaderLoadGLSLInfo>;
    using InfoList = std::variant<std::initializer_list<ShaderLoadSPIRVInfo>, 
                                  std::initializer_list<ShaderLoadGLSLInfo>>;
    
    // Data caches
    std::unordered_map<KeyType, std::vector<InfoType>> m_info_cache;
    std::unordered_map<KeyType, Program>               m_prog_cache;

    // Initialize-and-return a reference to a program object
    // Exposed in overloads below for variant types to support 
    // aggregate initialization from the front-end
    std::pair<KeyType, gl::Program&> set(InfoType &&info); 
    std::pair<KeyType, gl::Program&> set(InfoList &&info); 

  public:
    // Default constructor
    ProgramCache() = default;

    // File path constructor; construct cache internals from file, if file exists
    ProgramCache(fs::path cache_file_path);

  public:
    // Return an existing program for a given key
    gl::Program& at(const KeyType &k);

    // Clear out program cache
    void clear();

    // Save cache internals to file
    void save(fs::path cache_file_path) const;
    
    // Overwrite cache internals from file, if file exists
    void load(fs::path cache_file_path);

  public:
    // Forward to private variant constructor
    std::pair<KeyType, gl::Program&> set(ShaderLoadSPIRVInfo &&info) {
      return set(std::forward<InfoType>(std::forward<ShaderLoadSPIRVInfo>(info)));
    }
    
    // Forward to private variant constructor
    std::pair<KeyType, gl::Program&> set(ShaderLoadGLSLInfo &&info) {
      return set(std::forward<InfoType>(std::forward<ShaderLoadGLSLInfo>(info)));
    }

    // Forward to private variant constructor
    std::pair<KeyType, gl::Program&> set(std::initializer_list<ShaderLoadSPIRVInfo> &&info) {
      return set(std::forward<InfoList>(std::forward<std::initializer_list<ShaderLoadSPIRVInfo>>(info)));
    }
    
    // Forward to private variant constructor
    std::pair<KeyType, gl::Program&> set(std::initializer_list<ShaderLoadGLSLInfo> &&info) {
      return set(std::forward<InfoList>(std::forward<std::initializer_list<ShaderLoadGLSLInfo>>(info)));
    }
  };
} // namespace gl::detail