#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <glm/vec2.hpp>
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
    uint msaa_samples = 0;

    // OpenGL context settings
    ProfileType profile_type = ProfileType::eAny;
    uint profile_version_major = 1;
    uint profile_version_minor = 0;
    bool is_main_context = true;
    const Window *shared_context = nullptr;

    // Remainder of settings
    WindowFlags flags = { };
  };

  /**
   * Window object wrapping GLFW window object and OpenGL context
   */
  struct Window : public detail::Handle<void *> {
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
    inline glm::ivec2 window_pos() const { return _window_pos; }
    inline glm::ivec2 window_size() const { return _window_size; }
    inline glm::ivec2 framebuffer_size() const { return _framebuffer_size; }
    void set_window_pos(glm::ivec2 window_pos);
    void set_window_size(glm::ivec2 window_size);
    bool did_window_resize() const { return _did_window_resize; }
    bool did_framebuffer_resize() const { return _did_framebuffer_resize; }

    // Swap interval (0 = off, 1 = kinda vsync; may be ignored by driver)
    inline uint swap_interval() const { return _swap_interval; }
    void set_swap_interval(uint swap_interval);
    
    // window state setters/getters
    inline bool visible() const { return _is_visible; }
    inline bool maximized() const { return _is_maximized; }
    inline bool focused() const { return _is_focused; }
    inline bool should_close() const { return _should_close; }
    inline std::string title() const { return _title; }
    void set_visible(bool visible);
    void set_maximized();
    void set_focused();
    void set_should_close();
    void set_title(const std::string &title);

    /* miscellaneous */
    
    void request_attention() const; // Notify user of an event without a hard focus

  private:
    using Base = Handle<void *>;

    glm::ivec2 _window_pos;
    glm::ivec2 _window_size;
    glm::ivec2 _framebuffer_size;

    std::string _title;
    uint _swap_interval;
    
    bool _is_visible;
    bool _is_maximized;
    bool _is_focused;

    bool _should_close;
    bool _is_main_context;
    
    bool _did_window_resize;  
    bool _did_framebuffer_resize;

    friend void attach_callbacks(Window &);
    friend void detach_callbacks(Window &);

  public:
    void swap(Window &o);

    bool operator==(const Window &o) const;

    gl_declare_noncopyable(Window)
  };
} // namespace gl