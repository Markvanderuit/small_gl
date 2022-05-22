#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <glm/vec2.hpp>
#include <list>
#include <filesystem>
#include <string>

namespace gl {
  /**
   * Helper object to create window/context in window object.
   */
  struct WindowCreateInfo {
    // Window settings
    glm::ivec2 size = { 1, 1 };
    std::string title = "";
    uint swap_interval = 1;
    bool respect_content_scale = true;

    // OpenGL context settings
    ProfileType profile_type = ProfileType::eAny;
    uint profile_version_major = 1;
    uint profile_version_minor = 0;
    bool is_main_context = true;
    const Window *shared_context = nullptr;

    // Remainder of settings
    WindowCreateFlags flags = { };
  };

  /**
   * Helper object to encapsulate all received input events for
   * mouse, keyboard, and file drop.
   */
  struct WindowInputInfo {
    // Keyboard/mouse button actions, format is <id, action>
    std::list<std::pair<int, int>> keybd_button_actions;
    std::list<std::pair<int, int>> mouse_button_actions;
    
    // Mouse cursor/wheel positions
    glm::vec2 mouse_position;
    glm::vec2 mouse_scroll;
    
    // File/dir path, in case a file is dropped into the window
    std::list<std::filesystem::path> dropped_paths;
  };

  /**
   * Window object wrapping GLFW window object and OpenGL context
   */
  class Window : public detail::Handle<void *> {
    using Base = Handle<void *>;

    glm::ivec2 m_window_pos;
    glm::ivec2 m_window_size;
    glm::ivec2 m_framebuffer_size;
    float m_content_scale;
    std::string m_title;
    uint m_swap_interval;
    
    bool m_is_visible;
    bool m_is_maximized;
    bool m_is_focused;
    bool m_is_main_context;

    bool m_should_close;  
    bool m_did_window_resize;  
    bool m_did_framebuffer_resize;
    WindowInputInfo m_input_info;

  public:
    /* constr/destr */

    Window() = default;
    Window(WindowCreateInfo info);
    ~Window();

    /* context */    

    void swap_buffers();
    void poll_events();
    void attach_context();
    void detach_context();
    bool is_current_context() const;

    /* getters/setters */

    // window/framebuffer setters/getters/change flags
    inline glm::ivec2 window_pos() const { return m_window_pos; }
    inline glm::ivec2 window_size() const { return m_window_size; }
    inline glm::ivec2 framebuffer_size() const { return m_framebuffer_size; }
    inline float content_scale() const { return m_content_scale; }
    void set_window_pos(glm::ivec2 window_pos);
    void set_window_size(glm::ivec2 window_size);
    bool did_window_resize() const { return m_did_window_resize; }
    bool did_framebuffer_resize() const { return m_did_framebuffer_resize; }

    // Swap interval (0 = off, 1 = kinda vsync; may be ignored by driver)
    inline uint swap_interval() const { return m_swap_interval; }
    void set_swap_interval(uint swap_interval);
    
    // window state setters/getters
    inline bool visible() const { return m_is_visible; }
    inline bool maximized() const { return m_is_maximized; }
    inline bool focused() const { return m_is_focused; }
    inline bool should_close() const { return m_should_close; }
    inline std::string title() const { return m_title; }
    void set_visible(bool visible);
    void set_maximized();
    void set_focused();
    void set_should_close();
    void set_title(const std::string &title);

    // window input cannot be modified from the application r.n.
    inline const WindowInputInfo & input_info() const { return m_input_info; }

    /* miscellaneous */
    
    void request_attention() const; // Notify user of an event without a hard focus

    void swap(Window &o);

    bool operator==(const Window &o) const;

    gl_declare_noncopyable(Window)

  private:
    // Friend functions to setup/teardown GLFW's callback system
    friend void attach_callbacks(Window &);
    friend void detach_callbacks(Window &);
  };
} // namespace gl