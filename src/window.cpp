#include <small_gl/window.hpp>
#include <small_gl/exception.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace gl {
  void attach_callbacks(Window &window) {
    guard( window._is_init);
    GLFWwindow *object = (GLFWwindow *) window.object();

    glfwSetWindowUserPointer(object, &window);
    glfwSetWindowCloseCallback(object, [](GLFWwindow *object) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window._should_close = true;
    });
    glfwSetWindowFocusCallback(object, [](GLFWwindow *object, int focused) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window._is_focused = focused != 0;
    });
    glfwSetWindowMaximizeCallback(object, [](GLFWwindow *object, int maximized) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window._is_maximized = maximized != 0;
    });
    glfwSetWindowPosCallback(object, [](GLFWwindow *object, int x, int y) {
    gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window._window_pos = { x, y };
    });
    glfwSetWindowSizeCallback(object, [](GLFWwindow *object, int x, int y) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window._window_size = { x, y };
      window._did_window_resize = true;
    });
    glfwSetFramebufferSizeCallback(object, [](GLFWwindow *object, int x, int y) {
      gl::Window &window = *((gl::Window *) glfwGetWindowUserPointer(object));
      window._framebuffer_size = { x, y };
      window._did_framebuffer_resize = true;
    });
  }

  void detach_callbacks(Window &window) {
    guard(window._is_init);
    GLFWwindow *object = (GLFWwindow *) window.object();

    glfwSetWindowUserPointer(object, nullptr);
    glfwSetWindowCloseCallback(object, nullptr);
    glfwSetWindowFocusCallback(object, nullptr);
    glfwSetWindowMaximizeCallback(object, nullptr);
    glfwSetWindowPosCallback(object, nullptr);
    glfwSetWindowSizeCallback(object, nullptr);
    glfwSetFramebufferSizeCallback(object, nullptr);
  }
  
  Window::Window(WindowCreateInfo info)
  : Handle<void *>(true),
    _window_size(info.size),
    _window_pos(0, 0),
    _title(info.title),
    _swap_interval(info.swap_interval),
    _is_visible(has_flag(info.flags, WindowFlags::eVisible)),
    _is_maximized(has_flag(info.flags, WindowFlags::eMaximized)),
    _is_focused(has_flag(info.flags, WindowFlags::eFocused)),
    _should_close(false),
    _is_main_context(info.is_main_context),
    _did_window_resize(false),
    _did_framebuffer_resize(false) {

    // Initialize the GLFW library before any function calls can be made
    if (_is_main_context) {
      expr_check(glfwInit(), "glfwInit() failed");
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
    glfwWindowHint(GLFW_DECORATED,    has_flag(info.flags, WindowFlags::eDecorated));
    glfwWindowHint(GLFW_FLOATING,     has_flag(info.flags, WindowFlags::eFloating));
    glfwWindowHint(GLFW_FOCUSED,      has_flag(info.flags, WindowFlags::eFocused));
    glfwWindowHint(GLFW_MAXIMIZED,    has_flag(info.flags, WindowFlags::eMaximized));
    glfwWindowHint(GLFW_VISIBLE,      has_flag(info.flags, WindowFlags::eVisible));
    glfwWindowHint(GLFW_RESIZABLE,    has_flag(info.flags, WindowFlags::eResizable));
    glfwWindowHint(GLFW_SRGB_CAPABLE, has_flag(info.flags, WindowFlags::eSRGB));
    glfwWindowHint(GLFW_SAMPLES, has_flag(info.flags, WindowFlags::eMSAA) ? 4 : 0); 
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 
                                      has_flag(info.flags, WindowFlags::eDebug));

    // Obtain and configure main monitor for fullscreen, if requested
    GLFWmonitor *mon = nullptr;
    if (has_flag(info.flags, WindowFlags::eFullscreen)
     && has_flag(info.flags, WindowFlags::eVisible)) {
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
    _object = (void *) glfwCreateWindow(_window_size.x(), _window_size.y(), _title.c_str(), mon, shared);
    expr_check(_object, "glfwCreateWindow(...) failed");
    
    // Finally, load GLAD bindings
    if (_is_main_context) {
      attach_context();
      expr_check(gladLoadGL(), "gladLoadGL() failed");
    }

    // Instantiate miscellaneous window properties
    glfwSwapInterval(_swap_interval);
    attach_callbacks(*this);
    glfwGetFramebufferSize((GLFWwindow *) _object, &_framebuffer_size[0], &_framebuffer_size[1]);
  }

  Window::~Window() {
    guard(_is_init);
    detach_callbacks(*this);
    glfwDestroyWindow((GLFWwindow *) _object);
    if (_is_main_context) {
      glfwTerminate();
    }
  }

  void Window::swap_buffers() {
    glfwSwapBuffers((GLFWwindow *) _object);
  }

  void Window::poll_events() {
    _did_window_resize = false;
    _did_framebuffer_resize = false;
    if (_is_main_context) {
      glfwPollEvents();
    }
  }

  void Window::attach_context() {
    guard(!is_current_context());
    glfwMakeContextCurrent((GLFWwindow *) _object);
  }

  void Window::detach_context() {
    guard(is_current_context());
    glfwMakeContextCurrent( nullptr);
  }

  bool Window::is_current_context() const {
    return glfwGetCurrentContext() == (GLFWwindow *) _object;
  }

  void Window::set_window_pos(Array2i window_pos) {
    glfwSetWindowPos((GLFWwindow *) _object, window_pos[0], window_pos[1]);
  }

  void Window::set_window_size(Array2i window_size) {
    glfwSetWindowSize((GLFWwindow *) _object, window_size[0], window_size[1]);
  }
  
  void Window::set_swap_interval(uint swap_interval) {
    is_current_context(true);
    _swap_interval = swap_interval;
    glfwSwapInterval(_swap_interval);
  }

  void Window::set_visible(bool visible) {
    _is_visible = visible;
    if (_is_visible) {
      glfwShowWindow((GLFWwindow *) _object);
    } else {
      glfwHideWindow((GLFWwindow *) _object);
    }
  }

  void Window::set_maximized() {
    glfwMaximizeWindow((GLFWwindow *) _object);
  }

  void Window::set_focused() {
    glfwFocusWindow((GLFWwindow *) _object);
  }

  void Window::set_should_close() {
    glfwSetWindowShouldClose((GLFWwindow *) _object, GLFW_TRUE);
  }

  void Window::set_title(const std::string &title) {
    _title = title;
    glfwSetWindowTitle((GLFWwindow *) _object, _title.c_str());
  }

  void Window::request_attention() const {
    glfwRequestWindowAttention((GLFWwindow *) _object);
  }

  void Window::swap(Window &o) {
    using std::swap;

    detach_callbacks(*this);
    detach_callbacks(o);

    Base::swap(o);
    swap(_window_pos, o._window_pos);
    swap(_window_size, o._window_size);
    swap(_framebuffer_size, o._framebuffer_size);
    swap(_title, o._title);
    swap(_swap_interval, o._swap_interval);
    swap(_is_visible, o._is_visible);
    swap(_is_maximized, o._is_maximized);
    swap(_is_focused, o._is_focused);
    swap(_should_close, o._should_close);
    swap(_is_main_context, o._is_main_context);
    swap(_did_window_resize, o._did_window_resize);
    swap(_did_framebuffer_resize, o._did_framebuffer_resize);
    
    attach_callbacks(*this);
    attach_callbacks(o);
  }
  
  bool Window::operator==(const Window &o) const {
    using std::tie;
    return Base::operator==(o)
      && _window_pos.isApprox(o._window_pos) 
      && _window_size.isApprox(o._window_size)
      && _framebuffer_size.isApprox(o._framebuffer_size)
      && std::tie(_title, _swap_interval, _is_visible, 
                  _is_maximized, _is_focused, _should_close,
                  _is_main_context, _did_window_resize, _did_framebuffer_resize)
      == std::tie(o._title, o._swap_interval, o._is_visible, 
                  o._is_maximized, o._is_focused, o._should_close,
                  o._is_main_context, o._did_window_resize, o._did_framebuffer_resize);
  }

} // namespace gl