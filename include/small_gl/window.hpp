#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/enum.hpp>
#include <small_gl/utility.hpp>
#include <small_gl/detail/eigen.hpp>
#include <small_gl/detail/handle.hpp>
#include <list>
#include <filesystem>
#include <string>

namespace gl {
  /**
   * Helper object to create window/context in window object.
   */
  struct WindowInfo {
    // Window settings
    eig::Array2u size            = { 1, 1 };
    std::string title            = "";
    uint swap_interval           = 1;
    bool respect_content_scale   = true;

    // OpenGL context settings
    ProfileType profile_type     = ProfileType::eAny;
    uint profile_version_major   = 1;
    uint profile_version_minor   = 0;
    bool is_main_context         = true;
    const Window *shared_context = nullptr;

    // Remainder of settings
    WindowFlags flags      = { };
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
    eig::Array2f mouse_position;
    eig::Array2f mouse_scroll;
    
    // File/dir path, in case a file is dropped into the window
    std::list<fs::path> dropped_paths;
  };

  /**
   * Window object wrapping GLFW window object and OpenGL context
   */
  class Window : public detail::Handle<void *> {
    using Base = Handle<void *>;

    eig::Array2u m_window_pos;
    eig::Array2u m_window_size;
    eig::Array2u m_framebuffer_size;
    float m_content_scale;
    std::string m_title;
    uint m_swap_interval;
    
    bool m_is_visible;
    bool m_is_maximized;
    bool m_is_focused;
    bool m_is_main_context;

    bool m_did_window_resize;  
    bool m_did_framebuffer_resize;
    
    WindowInputInfo m_input_info;

  public:
    using InfoType = WindowInfo;

    /* constr/destr */

    Window() = default;
    Window(WindowInfo info);
    ~Window();

    /* context */    

    void swap_buffers();
    void poll_events();
    void attach_context();
    void detach_context();
    bool is_current_context() const;

    /* getters/setters */

    // window/framebuffer setters/getters/change flags
    inline eig::Array2u window_pos() const { return m_window_pos; }
    inline eig::Array2u window_size() const { return m_window_size; }
    inline eig::Array2u framebuffer_size() const { return m_framebuffer_size; }
    inline float content_scale() const { return m_content_scale; }
    void set_window_pos(const eig::Array2u &window_pos);
    void set_window_size(const eig::Array2u &window_size);
    bool did_window_resize() const { return m_did_window_resize; }
    bool did_framebuffer_resize() const { return m_did_framebuffer_resize; }

    // Swap interval (0 = off, 1 = kinda vsync; may be ignored by driver)
    inline uint swap_interval() const { return m_swap_interval; }
    void set_swap_interval(uint swap_interval);
    
    // window state setters/getters
    inline bool visible() const { return m_is_visible; }
    inline bool maximized() const { return m_is_maximized; }
    inline bool focused() const { return m_is_focused; }
    inline std::string title() const { return m_title; }
    bool should_close() const;
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