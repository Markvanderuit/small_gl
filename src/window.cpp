#include <small_gl/window.hpp>
#include <small_gl/utility.hpp>
#include <small_gl/detail/trace.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace gl {
  void attach_callbacks(Window &window) {
    guard(window.m_is_init);
    GLFWwindow *object = (GLFWwindow *) window.object();
    glfwSetWindowUserPointer(object, &window);

    glfwSetWindowCloseCallback(object, [](GLFWwindow *object) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      glfwSetWindowShouldClose(object, GLFW_TRUE);
    });

    glfwSetWindowFocusCallback(object, [](GLFWwindow *object, int focused) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_is_focused = focused != 0;
    });

    glfwSetWindowMaximizeCallback(object, [](GLFWwindow *object, int maximized) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_is_maximized = maximized != 0;
    });

    glfwSetWindowPosCallback(object, [](GLFWwindow *object, int x, int y) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_window_pos = { x, y };
    });

    glfwSetWindowSizeCallback(object, [](GLFWwindow *object, int x, int y) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_window_size = { x, y };
      window.m_did_window_resize = true;
    });

    glfwSetFramebufferSizeCallback(object, [](GLFWwindow *object, int x, int y) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_framebuffer_size = { x, y };
      window.m_did_framebuffer_resize = true;
    });

    glfwSetKeyCallback(object, [](GLFWwindow *object, int key, int scan, int action, int mods) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_input_info.keybd_button_actions.push_back({key, action});
    });

    glfwSetMouseButtonCallback(object, [](GLFWwindow *object, int button, int action, int mods) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_input_info.mouse_button_actions.push_back({button, action});
    });

    glfwSetCursorPosCallback(object, [](GLFWwindow *object, double x_pos, double y_pos) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_input_info.mouse_position = { static_cast<float>(x_pos), static_cast<float>(y_pos) };
    });

    glfwSetScrollCallback(object, [](GLFWwindow *object, double x_pos, double y_pos) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window.m_input_info.mouse_scroll = { static_cast<float>(x_pos), static_cast<float>(y_pos) };
    });

    glfwSetDropCallback(object, [](GLFWwindow *object, int count, const char **paths) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      for (int i = 0; i < count; ++i) {
        window.m_input_info.dropped_paths.push_back(paths[i]);
      } 
    });
  }

  void detach_callbacks(Window &window) {
    guard(window.m_is_init);
    GLFWwindow *object = (GLFWwindow *) window.object();

    glfwSetWindowUserPointer(object, nullptr);
    glfwSetWindowCloseCallback(object, nullptr);
    glfwSetWindowFocusCallback(object, nullptr);
    glfwSetWindowMaximizeCallback(object, nullptr);
    glfwSetWindowPosCallback(object, nullptr);
    glfwSetWindowSizeCallback(object, nullptr);
    glfwSetFramebufferSizeCallback(object, nullptr);
    glfwSetKeyCallback(object, nullptr);
    glfwSetMouseButtonCallback(object, nullptr);
    glfwSetCursorPosCallback(object, nullptr);
    glfwSetScrollCallback(object, nullptr);
    glfwSetDropCallback(object, nullptr);
  }

  Window::Window(WindowCreateInfo info)
  : Handle<void *>(true),
    m_window_size(info.size),
    m_window_pos(0, 0),
    m_title(info.title),
    m_swap_interval(info.swap_interval),
    m_is_visible(has_flag(info.flags, WindowCreateFlags::eVisible)),
    m_is_maximized(has_flag(info.flags, WindowCreateFlags::eMaximized)),
    m_is_focused(has_flag(info.flags, WindowCreateFlags::eFocused)),
    m_is_main_context(info.is_main_context),
    m_did_window_resize(false),
    m_did_framebuffer_resize(false) {

    // Initialize the GLFW library before any function calls can be made
    if (m_is_main_context) {
      debug::check_expr_dbg(glfwInit(), "glfwInit() failed");
    }

    // Determine correct profile flag to pass to GLFW
    uint profile;
    switch (info.profile_type) {
      case ProfileType::eAny:
        profile = GLFW_OPENGL_ANY_PROFILE;
        break;
      case ProfileType::eCore:
        profile = GLFW_OPENGL_CORE_PROFILE;
        break;
      case ProfileType::eCompatibility:
        profile = GLFW_OPENGL_COMPAT_PROFILE;
        break;
    }
    
    // Set window/framebuffer/context construction hints
    glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.profile_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.profile_version_minor);
    glfwWindowHint(GLFW_DECORATED,    has_flag(info.flags, WindowCreateFlags::eDecorated));
    glfwWindowHint(GLFW_FLOATING,     has_flag(info.flags, WindowCreateFlags::eFloating));
    glfwWindowHint(GLFW_FOCUSED,      has_flag(info.flags, WindowCreateFlags::eFocused));
    glfwWindowHint(GLFW_MAXIMIZED,    has_flag(info.flags, WindowCreateFlags::eMaximized));
    glfwWindowHint(GLFW_VISIBLE,      has_flag(info.flags, WindowCreateFlags::eVisible));
    glfwWindowHint(GLFW_RESIZABLE,    has_flag(info.flags, WindowCreateFlags::eResizable));
    glfwWindowHint(GLFW_SRGB_CAPABLE, has_flag(info.flags, WindowCreateFlags::eSRGB));
    glfwWindowHint(GLFW_SAMPLES,      has_flag(info.flags, WindowCreateFlags::eMSAA) ? 4 : 0); 
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 
                                      has_flag(info.flags, WindowCreateFlags::eDebug));

    // Obtain and configure main monitor for fullscreen, if requested
    GLFWmonitor *mon = nullptr;
    if (has_flag(info.flags, WindowCreateFlags::eFullscreen)
     && has_flag(info.flags, WindowCreateFlags::eVisible)) {
      GLFWmonitor *monitor = glfwGetPrimaryMonitor();
      const GLFWvidmode *mode = glfwGetVideoMode(monitor);
      glfwWindowHint(GLFW_RED_BITS,     mode->redBits);
      glfwWindowHint(GLFW_GREEN_BITS,   mode->greenBits);
      glfwWindowHint(GLFW_BLUE_BITS,    mode->blueBits);
      glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    // Pointer to shared window for context sharing
    GLFWwindow *shared = info.shared_context ? (GLFWwindow *) info.shared_context->object()
                                             : nullptr;

    // Initialize a GLFW window 
    m_object = (void *) glfwCreateWindow(m_window_size.x(), m_window_size.y(), m_title.c_str(), mon, shared);
    debug::check_expr_dbg(m_object, "glfwCreateWindow(...) failed");
    
    // Finally, load GLAD bindings
    if (m_is_main_context) {
      attach_context();
      debug::check_expr_dbg(gladLoadGL(), "gladLoadGL() failed");
      gl_trace_init_context();
    }

    // Instantiate miscellaneous window properties
    glfwSwapInterval(m_swap_interval);
    attach_callbacks(*this);
    eig::Array2i _framebuffer_size;
    glfwGetFramebufferSize((GLFWwindow *) m_object, &_framebuffer_size.x(), &_framebuffer_size.y());
    m_framebuffer_size = _framebuffer_size.cast<uint>();

    // Instantiate (and apply) window scale
    if (info.respect_content_scale) {
      // Assume horizontal scale is sufficient?
      glfwGetWindowContentScale((GLFWwindow *) m_object, &m_content_scale, nullptr);
      set_window_size((m_content_scale * window_size().cast<float>()).cast<uint>());
    } else {
      m_content_scale = 1.f;
    }
  }

  Window::~Window() {
    guard(m_is_init);
    detach_callbacks(*this);
    glfwDestroyWindow((GLFWwindow *) m_object);
    if (m_is_main_context) {
      glfwTerminate();
    }
  }

  void Window::swap_buffers() {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwSwapBuffers((GLFWwindow *) m_object);
  }

  void Window::poll_events() {
    gl_trace_full();
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    m_did_window_resize = false;
    m_did_framebuffer_resize = false;
    m_input_info.keybd_button_actions.clear();
    m_input_info.mouse_button_actions.clear();
    m_input_info.dropped_paths.clear();
    
    if (m_is_main_context) {
      glfwPollEvents();
    }
  }

  void Window::attach_context() {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    guard(!is_current_context());
    glfwMakeContextCurrent((GLFWwindow *) m_object);
  }

  void Window::detach_context() {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    guard(is_current_context());
    glfwMakeContextCurrent( nullptr);
  }

  bool Window::is_current_context() const {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    return glfwGetCurrentContext() == (GLFWwindow *) m_object;
  }

  void Window::set_window_pos(const eig::Array2u &window_pos) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwSetWindowPos((GLFWwindow *) m_object, window_pos.x(), window_pos.y());
  }

  void Window::set_window_size(const eig::Array2u &window_size) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwSetWindowSize((GLFWwindow *) m_object, window_size.x(), window_size.y());
  }
  
  void Window::set_swap_interval(uint swap_interval) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    attach_context();
    m_swap_interval = swap_interval;
    glfwSwapInterval(m_swap_interval);
  }

  void Window::set_visible(bool visible) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    m_is_visible = visible;
    if (m_is_visible) {
      glfwShowWindow((GLFWwindow *) m_object);
    } else {
      glfwHideWindow((GLFWwindow *) m_object);
    }
  }

  void Window::set_maximized() {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwMaximizeWindow((GLFWwindow *) m_object);
  }

  void Window::set_focused() {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwFocusWindow((GLFWwindow *) m_object);
  }

  void Window::set_should_close() {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwSetWindowShouldClose((GLFWwindow *) m_object, GLFW_TRUE);
  }

  bool Window::should_close() const {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    return glfwWindowShouldClose((GLFWwindow *) m_object);
  }

  void Window::set_title(const std::string &title) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    m_title = title;
    glfwSetWindowTitle((GLFWwindow *) m_object, m_title.c_str());
  }

  void Window::request_attention() const {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    glfwRequestWindowAttention((GLFWwindow *) m_object);
  }

  void Window::swap(Window &o) {
    using std::swap;

    detach_callbacks(*this);
    detach_callbacks(o);

    Base::swap(o);

    swap(m_window_pos, o.m_window_pos);
    swap(m_window_size, o.m_window_size);
    swap(m_framebuffer_size, o.m_framebuffer_size);
    swap(m_content_scale, o.m_content_scale);
    swap(m_title, o.m_title);
    swap(m_swap_interval, o.m_swap_interval);

    swap(m_is_visible, o.m_is_visible);
    swap(m_is_maximized, o.m_is_maximized);
    swap(m_is_focused, o.m_is_focused);
    swap(m_is_main_context, o.m_is_main_context);

    swap(m_did_window_resize, o.m_did_window_resize);
    swap(m_did_framebuffer_resize, o.m_did_framebuffer_resize);

    swap(m_input_info, o.m_input_info);

    attach_callbacks(*this);
    attach_callbacks(o);
  }
  
  bool Window::operator==(const Window &o) const {
    using std::tie;
    return Base::operator==(o)
      && m_window_pos.isApprox(o.m_window_pos)
      && m_window_size.isApprox(o.m_window_size)
      && m_framebuffer_size.isApprox(o.m_framebuffer_size)
      && std::tie(m_content_scale,
                  m_title, m_swap_interval, m_is_visible, 
                  m_is_maximized, m_is_focused,
                  m_is_main_context, m_did_window_resize, m_did_framebuffer_resize)
      == std::tie(o.m_content_scale,
                  o.m_title, o.m_swap_interval, o.m_is_visible, 
                  o.m_is_maximized, o.m_is_focused,
                  o.m_is_main_context, o.m_did_window_resize, o.m_did_framebuffer_resize);
  }

} // namespace gl