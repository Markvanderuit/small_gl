#include <small_gl/sampler.hpp>
#include <small_gl/utility.hpp>

namespace gl {
  Sampler::Sampler(SamplerCreateInfo info)
  : Base(true), m_min_filter(info.min_filter), m_mag_filter(info.mag_filter), 
    m_wrap(info.wrap), m_compare_func(info.compare_func), m_compare_mode(info.compare_mode) {
    
    glCreateSamplers(1, &m_object);

    glSamplerParameteri(m_object, GL_TEXTURE_MIN_FILTER, (uint) info.min_filter);
    glSamplerParameteri(m_object, GL_TEXTURE_MAG_FILTER, (uint) info.mag_filter);
    glSamplerParameteri(m_object, GL_TEXTURE_WRAP_R, (uint) info.wrap);
    glSamplerParameteri(m_object, GL_TEXTURE_WRAP_S, (uint) info.wrap);
    glSamplerParameteri(m_object, GL_TEXTURE_WRAP_T, (uint) info.wrap);
    glSamplerParameteri(m_object, GL_TEXTURE_COMPARE_FUNC, (uint) info.compare_func);
    glSamplerParameteri(m_object, GL_TEXTURE_COMPARE_MODE, (uint) info.compare_mode);
  }

  Sampler::~Sampler() {
    guard(m_is_init);
    glDeleteSamplers(1, &m_object);
  }

  void Sampler::set_min_filter(SamplerMinFilter min_filter) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");
    
    m_min_filter = min_filter;
    glSamplerParameteri(m_object, GL_TEXTURE_MIN_FILTER, (uint) min_filter);
  }

  void Sampler::set_mag_filter(SamplerMagFilter mag_filter) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    m_mag_filter = mag_filter;
    glSamplerParameteri(m_object, GL_TEXTURE_MAG_FILTER, (uint) mag_filter);
  }

  void Sampler::set_wrap(SamplerWrap wrap) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    m_wrap = wrap;
    glSamplerParameteri(m_object, GL_TEXTURE_WRAP_R, (uint) wrap);
    glSamplerParameteri(m_object, GL_TEXTURE_WRAP_S, (uint) wrap);
    glSamplerParameteri(m_object, GL_TEXTURE_WRAP_T, (uint) wrap);
  }

  void Sampler::set_depth_compare_func(SamplerCompareFunc compare_func) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    m_compare_func = compare_func;
    glSamplerParameteri(m_object, GL_TEXTURE_COMPARE_FUNC, (uint) compare_func);
  }

  void Sampler::set_depth_compare_mode(SamplerCompareMode compare_mode) {
    debug::check_expr_dbg(m_is_init, "attempt to use an uninitialized object");

    m_compare_mode = compare_mode;
    glSamplerParameteri(m_object, GL_TEXTURE_COMPARE_MODE, (uint) compare_mode);
  }

  void Sampler::bind_to(uint index) const {
    glBindSampler(index, m_object);
  }
} // namespace gl