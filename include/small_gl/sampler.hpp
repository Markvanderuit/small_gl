#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>

namespace gl {
  /**
   * Helper object to create sampler object.
   */
  struct SamplerCreateInfo {
    SamplerMinFilter min_filter     = SamplerMinFilter::eNearest;
    SamplerMagFilter mag_filter     = SamplerMagFilter::eNearest;
    SamplerWrap wrap                = SamplerWrap::eClampToEdge;
    SamplerCompareFunc compare_func = SamplerCompareFunc::eLessOrEqual;
    SamplerCompareMode compare_mode = SamplerCompareMode::eNone;
  };

  /**
   * Sampler object wrapping OpenGL sampler object.
   */
  class Sampler : public detail::Handle<> {
    using Base = detail::Handle<>;

    SamplerMinFilter m_min_filter;
    SamplerMagFilter m_mag_filter;
    SamplerWrap m_wrap;
    SamplerCompareFunc m_compare_func;
    SamplerCompareMode m_compare_mode;

  public:
    using InfoType = SamplerCreateInfo;

    /* constr/destr */

    Sampler() = default;
    Sampler(SamplerCreateInfo info);
    ~Sampler();

    /* getters/setters */

    inline SamplerMinFilter min_filter() const { return m_min_filter; }
    inline SamplerMagFilter mag_filter() const { return m_mag_filter; }
    inline SamplerWrap wrap() const { return m_wrap; }
    inline SamplerCompareFunc compare_func() const { return m_compare_func; }
    inline SamplerCompareMode compare_mode() const { return m_compare_mode; }

    void set_min_filter(SamplerMinFilter min_filter);
    void set_mag_filter(SamplerMagFilter mag_filter);
    void set_wrap(SamplerWrap wrap);
    void set_depth_compare_func(SamplerCompareFunc compare_func);
    void set_depth_compare_mode(SamplerCompareMode compare_mode);

    /* state */

    void bind_to(uint index) const;
    
    /* miscellaneous */

    inline void swap(Sampler &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_min_filter, o.m_min_filter);
      swap(m_mag_filter, o.m_mag_filter);
      swap(m_wrap, o.m_wrap);
      swap(m_compare_func, o.m_compare_func);
      swap(m_compare_mode, o.m_compare_mode);
    }

    inline bool operator==(const Sampler &o) const {
      using std::tie;
      return Base::operator==(o)
          && std::tie(m_min_filter, m_mag_filter, m_wrap, m_compare_func, m_compare_mode)
          == std::tie(o.m_min_filter, o.m_mag_filter, o.m_wrap, o.m_compare_func, o.m_compare_mode);
    }

    gl_declare_noncopyable(Sampler)
  };
} // namespace gl