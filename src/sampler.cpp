#include <small_gl/sampler.hpp>
#include <small_gl/exception.hpp>

namespace gl {
  Sampler::Sampler(SamplerCreateInfo info)
  : Base(true), _min_filter(info.min_filter), _mag_filter(info.mag_filter), 
    _wrap(info.wrap), _compare_func(info.compare_func), _compare_mode(info.compare_mode) {
    
    glCreateSamplers(1, &_object);

    glSamplerParameteri(_object, GL_TEXTURE_MIN_FILTER, (uint) info.min_filter);
    glSamplerParameteri(_object, GL_TEXTURE_MAG_FILTER, (uint) info.mag_filter);
    glSamplerParameteri(_object, GL_TEXTURE_WRAP_R, (uint) info.wrap);
    glSamplerParameteri(_object, GL_TEXTURE_WRAP_S, (uint) info.wrap);
    glSamplerParameteri(_object, GL_TEXTURE_WRAP_T, (uint) info.wrap);
    glSamplerParameteri(_object, GL_TEXTURE_COMPARE_FUNC, (uint) info.compare_func);
    glSamplerParameteri(_object, GL_TEXTURE_COMPARE_MODE, (uint) info.compare_mode);

    gl_check();
  }

  Sampler::~Sampler() {
    guard(_is_init);
    glDeleteSamplers(1, &_object);
    gl_check();
  }

  void Sampler::set_min_filter(SamplerMinFilter min_filter) {
    expr_check(_is_init, "attempt to use an uninitialized object");
    
    _min_filter = min_filter;
    glSamplerParameteri(_object, GL_TEXTURE_MIN_FILTER, (uint) min_filter);
    
    gl_check();
  }

  void Sampler::set_mag_filter(SamplerMagFilter mag_filter) {
    expr_check(_is_init, "attempt to use an uninitialized object");

    _mag_filter = mag_filter;
    glSamplerParameteri(_object, GL_TEXTURE_MAG_FILTER, (uint) mag_filter);

    gl_check();
  }

  void Sampler::set_wrap(SamplerWrap wrap) {
    expr_check(_is_init, "attempt to use an uninitialized object");

    _wrap = wrap;
    glSamplerParameteri(_object, GL_TEXTURE_WRAP_R, (uint) wrap);
    glSamplerParameteri(_object, GL_TEXTURE_WRAP_S, (uint) wrap);
    glSamplerParameteri(_object, GL_TEXTURE_WRAP_T, (uint) wrap);

    gl_check();
  }

  void Sampler::set_depth_compare_func(SamplerCompareFunc compare_func) {
    expr_check(_is_init, "attempt to use an uninitialized object");

    _compare_func = compare_func;
    glSamplerParameteri(_object, GL_TEXTURE_COMPARE_FUNC, (uint) compare_func);

    gl_check();
  }

  void Sampler::set_depth_compare_mode(SamplerCompareMode compare_mode) {
    expr_check(_is_init, "attempt to use an uninitialized object");

    _compare_mode = compare_mode;
    glSamplerParameteri(_object, GL_TEXTURE_COMPARE_MODE, (uint) compare_mode);

    gl_check();
  }
} // namespace gl