#pragma once

// Insert Tracy scope statements if tracing is enabled
#ifndef GL_ENABLE_TRACY
  #define gl_trace()
  #define gl_trace_n(name)
  #define gl_trace_gpu()
  #define gl_trace_gpu_n(name)
  #define gl_trace_full()
  #define gl_trace_full_n(name)
  #define gl_trace_gpu_alloc(alloc_name, handle, size)
  #define gl_trace_gpu_free(alloc_name, handle)
  #define gl_trace_init_context()
#else // GL_ENABLE_TRACY
  #include <glad/glad.h>
  #include <tracy/Tracy.hpp>
  #include <tracy/TracyOpenGL.hpp>
  
  // Insert CPU event trace
  #define gl_trace()                    ZoneScoped;
  #define gl_trace_n(name)              ZoneScopedN(name)

  // Insert GPU event trace
  #define gl_trace_gpu()                TracyGpuZone(__FUNCTION__)      
  #define gl_trace_gpu_n(name)          TracyGpuZone(name)      

  // Insert CPU+GPU event traces
  #define gl_trace_full()               gl_trace(); gl_trace_gpu();
  #define gl_trace_full_n(name)         gl_trace_n(name); gl_trace_gpu_n(name);

  // Insert gpu memory event trace
  #define gl_trace_gpu_alloc(alloc_name, handle, size)\
    TracyAllocN(reinterpret_cast<void *>(static_cast<std::uintptr_t>(handle)), size, alloc_name)
  #define gl_trace_gpu_free(alloc_name, handle)\
    TracyFreeN(reinterpret_cast<void *>(static_cast<std::uintptr_t>(handle)), alloc_name)

  // Signal active gpu context
  #define gl_trace_init_context() TracyGpuContext;
  

  #ifndef TRACY_ENABLE
  #define TRACY_ENABLE
  #endif // TRACY_ENABLE
  #ifndef TRACY_ON_DEMAND
  #define TRACY_ON_DEMAND
  #endif // TRACY_ON_DEMAND
#endif // GL_ENABLE_TRACY