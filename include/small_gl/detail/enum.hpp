#pragma once

#include <glad/glad.h>

// For enum class T, declare bitflag operators and has_flag(T, T) boolean operator
#define gl_declare_bitflag(T)\
  constexpr T operator~(T a) { return (T) (~ (uint) a); }\
  constexpr T operator|(T a, T b) { return (T) ((uint) a | (uint) b); }\
  constexpr T operator&(T a, T b) { return (T) ((uint) a & (uint) b); }\
  constexpr T operator^(T a, T b) { return (T) ((uint) a ^ (uint) b); }\
  constexpr T& operator|=(T &a, T b) { return a = a | b; }\
  constexpr T& operator&=(T &a, T b) { return a = a & b; }\
  constexpr T& operator^=(T &a, T b) { return a = a ^ b; }\
  constexpr bool has_flag(T flags, T t) { return (uint) (flags & t) != 0u; }

namespace gl {
  using uint = unsigned int;

  /* Buffer enums */

  // Binding target for gl::Buffer::bind_to(...)
  enum class BufferTargetType : uint {
    eAtomicCounter      = GL_ATOMIC_COUNTER_BUFFER,
    eStorage            = GL_SHADER_STORAGE_BUFFER,
    eTransformFeedback  = GL_TRANSFORM_FEEDBACK_BUFFER,
    eUniform            = GL_UNIFORM_BUFFER
  };

  // Storage flags for gl::Buffer(...) construction
  enum class BufferCreateFlags : uint {
    eNone               = 0,

    eStorageDynamic     = GL_DYNAMIC_STORAGE_BIT,
    eStorageClient      = GL_CLIENT_STORAGE_BIT,
    eMapRead            = GL_MAP_READ_BIT,
    eMapWrite           = GL_MAP_WRITE_BIT,
    eMapPersistent      = GL_MAP_PERSISTENT_BIT,  
    eMapCoherent        = GL_MAP_COHERENT_BIT,
    
    // Special assembled types
    eMapReadWrite       = (uint) BufferCreateFlags::eMapRead 
                        | (uint) BufferCreateFlags::eMapWrite,
    eMapReadPersistent  = (uint) BufferCreateFlags::eMapRead
                        | (uint) BufferCreateFlags::eMapPersistent,
    eMapWritePersistent = (uint) BufferCreateFlags::eMapWrite
                        | (uint) BufferCreateFlags::eMapPersistent,
    eMapFull            = (uint) BufferCreateFlags::eMapReadWrite
                        | (uint) BufferCreateFlags::eMapPersistent
                        | (uint) BufferCreateFlags::eMapCoherent,
  };
  gl_declare_bitflag(BufferCreateFlags);

  // Access flags for gl::Buffer::map(...)
  enum class BufferAccessFlags : uint {
    eNone               = 0,
    
    eMapRead            = GL_MAP_READ_BIT,
    eMapWrite           = GL_MAP_WRITE_BIT,
    eMapInvalidate      = GL_MAP_INVALIDATE_RANGE_BIT,
    eMapPersistent      = GL_MAP_PERSISTENT_BIT,  
    eMapCoherent        = GL_MAP_COHERENT_BIT,
    eMapFlush           = GL_MAP_FLUSH_EXPLICIT_BIT,

    // Special assembled types
    eMapReadWrite       = (uint) BufferAccessFlags::eMapRead 
                        | (uint) BufferAccessFlags::eMapWrite,
    eMapReadPersistent  = (uint) BufferAccessFlags::eMapRead
                        | (uint) BufferAccessFlags::eMapPersistent,
    eMapWritePersistent = (uint) BufferAccessFlags::eMapWrite
                        | (uint) BufferAccessFlags::eMapPersistent,
    eMapFull            = (uint) BufferAccessFlags::eMapReadWrite
                        | (uint) BufferAccessFlags::eMapPersistent
                        | (uint) BufferAccessFlags::eMapCoherent
                        | (uint) BufferAccessFlags::eMapFlush,
  };
  gl_declare_bitflag(BufferAccessFlags);

  /* Draw and state enums */

  // Draw capabilities for gl::state::set(...)/get(...)
  enum class DrawCapability : uint {
    // Misc capabilities
    eFramebufferSRGB    = GL_FRAMEBUFFER_SRGB,
    eMSAA               = GL_MULTISAMPLE,
    eCullOp             = GL_CULL_FACE,
    eDither             = GL_DITHER,

    // Blending capabilities
    eBlendOp            = GL_BLEND,
    eLogicOp            = GL_COLOR_LOGIC_OP,

    // Depth/stencil/scissor test capabilities
    eDepthClamp         = GL_DEPTH_CLAMP,
    eDepthTest          = GL_DEPTH_TEST,
    eStencilTest        = GL_STENCIL_TEST,
    eScissorTest        = GL_SCISSOR_TEST,

    // Smoothing capabilities
    eLineSmooth         = GL_LINE_SMOOTH,
    ePolySmooth         = GL_POLYGON_SMOOTH,
  };

  // Blend operations for gl::state::set_op(...)
  enum class BlendOp : uint {
    eZero               = GL_ZERO,
    eOne                = GL_ONE,
    eSrcColor           = GL_SRC_COLOR,
    eOneMinusSrcColor   = GL_ONE_MINUS_SRC_COLOR,
    eDstColor           = GL_DST_COLOR,
    eOneMinusDstColor   = GL_ONE_MINUS_DST_COLOR,
    eSrcAlpha           = GL_SRC_ALPHA,
    eOneMinusSrcAlpha   = GL_ONE_MINUS_SRC_ALPHA,
    eDstAlpha           = GL_DST_ALPHA,
    eOneMinusDstAlpha   = GL_ONE_MINUS_DST_ALPHA,
    eConstColor         = GL_CONSTANT_COLOR,
    eOneMinusConstColor = GL_ONE_MINUS_CONSTANT_COLOR,
    eConstAlpha         = GL_CONSTANT_ALPHA,
    eOneMinusConstAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
    eSrcAlphaSaturate   = GL_SRC_ALPHA_SATURATE,
    eSrc1Color          = GL_SRC1_COLOR,
    eOneMinusSrc1Color  = GL_ONE_MINUS_SRC1_COLOR,
    eSrc1Alpha          = GL_SRC1_ALPHA,
    eOneMinusSrc1Alpha  = GL_ONE_MINUS_SRC1_ALPHA
  };

  // Culling operations for gl::state::set_op(...)
  enum class CullOp : uint {
    eBack               = GL_BACK,
    eFront              = GL_FRONT,
    eBoth               = GL_FRONT_AND_BACK 
  };

  // Drawing operations for gl::state::set_op(...)
  enum class DrawOp : uint {
    ePoint              = GL_POINT,
    eLine               = GL_LINE,
    eFill               = GL_FILL
  };

  // Logic operations for gl::state::set_op(...)
  enum class LogicOp : uint {
    eClear              = GL_CLEAR,
    eSet                = GL_SET,
    eCopy               = GL_COPY,
    eCopyInverted       = GL_COPY_INVERTED,
    eNoop               = GL_NOOP,
    eInvert             = GL_INVERT,
    eAnd                = GL_AND,
    eNand               = GL_NAND,
    eOr                 = GL_OR,
    eNor                = GL_NOR,
    eXor                = GL_XOR,
    eEquiv              = GL_EQUIV,
    eAndReverse         = GL_AND_REVERSE,
    eAndInverse         = GL_AND_INVERTED,
    eOrReverse          = GL_OR_REVERSE,
    eOrInverse          = GL_OR_INVERTED,
  };

  // Depth function operations for gl::state::set_op(...)
  enum class DepthOp : uint {
    eLessOrEqual        = GL_LEQUAL,
    eGreaterOrEqual     = GL_GEQUAL,
    eLess               = GL_LESS,
    eGreater            = GL_GREATER,
    eEqual              = GL_EQUAL,
    eNotEqual           = GL_NOTEQUAL,
    eAlways             = GL_ALWAYS,
    eNever              = GL_NEVER
  };

  // Primitive types for gl::draw(...)
  enum class PrimitiveType : uint {
    ePoints             = GL_POINTS,
    eLines              = GL_LINES,
    eTriangles          = GL_TRIANGLES,
    ePatches            = GL_PATCHES,

    eLineStrip          = GL_LINE_STRIP,
    eLineLoop           = GL_LINE_LOOP,
    eLinesAdj           = GL_LINES_ADJACENCY,
    eLineStripAdj       = GL_LINE_STRIP_ADJACENCY,

    eTriangleStrip      = GL_TRIANGLE_STRIP,
    eTriangleFan        = GL_TRIANGLE_FAN,
    eTrianglesAdj       = GL_TRIANGLES_ADJACENCY,
    eTriangleStripAdj   = GL_TRIANGLE_STRIP_ADJACENCY,
  };

  // State variable names for gl::get_variable(...)
  // TODO: fill as needed
  enum class VariableName : uint {
    // GL_KHR_shader_subgroup variables
    eSubgroupSize           = GL_SUBGROUP_SIZE_KHR,
    eSharedMemorySize       = GL_MAX_COMPUTE_SHARED_MEMORY_SIZE,
    eFramebufferBinding     = GL_DRAW_FRAMEBUFFER_BINDING,
    eProgramBinding         = GL_CURRENT_PROGRAM,
    
    eMaxTextureSize         = GL_MAX_TEXTURE_SIZE,
    eMaxArrayTextureLayers  = GL_MAX_ARRAY_TEXTURE_LAYERS,

    eUBOOffsetAlignment     = GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
    eSSBOOffsetAlignment    = GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
    // eMaxComputeShaderStorageBlocks
    //                     = GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS,
    // eMaxCombinedShaderStorageBlocks
    //                     = GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS,
    // eMaxComputeUniformBlocks
    //                     = GL_MAX_COMPUTE_UNIFORM_BLOCKS,
    // eMaxComputeImageUnits
    //                     = GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS,
    // eMaxComputeUniformComponents
    //                     = GL_MAX_COMPUTE_UNIFORM_COMPONENTS,
  };

  /* Debug enums */

  enum class DebugMessageTypeFlags : uint {
    // Base OpenGL debug message types
    eError              = 0x001u,
    eDeprecated         = 0x002u,
    eUndefinedBehavior  = 0x004u,
    ePortability        = 0x008u,
    ePerformance        = 0x010u,
    eMarker             = 0x020u,
    ePushGroup          = 0x040u,
    ePopGroup           = 0x080u,
    eOther              = 0x100u,

    // Special assembled types
    eThrowType          = (uint) DebugMessageTypeFlags::eError
                        | (uint) DebugMessageTypeFlags::eUndefinedBehavior,
    eWarnType           = (uint) DebugMessageTypeFlags::eDeprecated
                        | (uint) DebugMessageTypeFlags::ePortability
                        | (uint) DebugMessageTypeFlags::ePerformance
                        | (uint) DebugMessageTypeFlags::eMarker,
    eGroupType          = (uint) DebugMessageTypeFlags::ePushGroup
                        | (uint) DebugMessageTypeFlags::ePopGroup,
    eAll                = (uint) DebugMessageTypeFlags::eThrowType 
                        | (uint) DebugMessageTypeFlags::eWarnType 
                        | (uint) DebugMessageTypeFlags::eGroupType
                        | (uint) DebugMessageTypeFlags::eOther
  };
  gl_declare_bitflag(DebugMessageTypeFlags)

  enum class DebugMessageSeverity : uint {
    eNotification       = 0u,
    eLow                = 1u,
    eMedium             = 2u,
    eHigh               = 3u
  };

  /* Framebuffer enums */

  // Attachment types for gl::Framebuffer(...) construction
  // and for gl::Framebuffer::clear(...) targets
  enum class FramebufferType : uint {
    eColor               = GL_COLOR,
    eDepth               = GL_DEPTH,
    eStencil             = GL_STENCIL
  };

  // Mask types for gl::Framebuffer()::blit_to(...)
  enum class FramebufferMaskFlags : uint {
    eColor               = GL_COLOR_BUFFER_BIT,
    eDepth               = GL_DEPTH_BUFFER_BIT,
    eStencil             = GL_STENCIL_BUFFER_BIT
  };
  gl_declare_bitflag(FramebufferMaskFlags)

  /* Renderbuffer enums */

  // Created type for gl::Renderbuffer<...>(...) construction
  enum class RenderbufferType {
    eImage, // default type
    eMultisample
  };

  /* Shader enums */

  // Created type for gl::Program(...) internal construction
  enum class ShaderType : uint {
    eCompute            = GL_COMPUTE_SHADER,
    eVertex             = GL_VERTEX_SHADER,
    eGeometry           = GL_GEOMETRY_SHADER,
    eFragment           = GL_FRAGMENT_SHADER,
    eTesselationEval    = GL_TESS_EVALUATION_SHADER,
    eTesselationCtrl    = GL_TESS_CONTROL_SHADER
  };

  /* Texture enums */
  
  // Created type for gl::Texture<...>(...) construction
  enum class TextureType {
    eImage, // default type
    eImageArray,
    eCubemap,
    eCubemapArray,
    eMultisample,
    eMultisampleArray
  };

  enum class TextureTargetType {
    eTextureUnit            = 0,
    eImageReadOnly          = GL_READ_ONLY,
    eImageWriteOnly         = GL_WRITE_ONLY,
    eImageReadWrite         = GL_READ_WRITE
  };

  /* Sampler enums */
  
  // Filter used for minimization in gl::Sampler
  enum class SamplerMinFilter : uint {
    eNearest                = GL_NEAREST,
    eLinear                 = GL_LINEAR,
    eNearestMipmapNearest   = GL_NEAREST_MIPMAP_NEAREST,
    eLinearMipmapNearest    = GL_LINEAR_MIPMAP_NEAREST,
    eNearestMipmapLinear    = GL_NEAREST_MIPMAP_LINEAR,
    eLinearMipmapLinear     = GL_LINEAR_MIPMAP_LINEAR
  };
  
  // Filter used for magnification in gl::Sampler
  enum class SamplerMagFilter : uint {
    eNearest                = GL_NEAREST,
    eLinear                 = GL_LINEAR
  };

  // Technique used for wrapping in gl::Sampler
  enum class SamplerWrap : uint {
    eRepeat                 = GL_REPEAT,
    eMirroredRepeat         = GL_MIRRORED_REPEAT,
    eClampToEdge            = GL_CLAMP_TO_EDGE,
    eClampToBorder          = GL_CLAMP_TO_BORDER
  };

  enum class SamplerCompareFunc : uint {
    eLessOrEqual            = GL_LEQUAL,
    eGreaterOrEqual         = GL_GEQUAL,
    eLess                   = GL_LESS,
    eGreater                = GL_GREATER,
    eEqual                  = GL_EQUAL,
    eNotEqual               = GL_NOTEQUAL,
    eAlways                 = GL_ALWAYS,
    eNever                  = GL_NEVER
  };

  enum class SamplerCompareMode : uint {
    eNone                   = GL_NONE,
    eCompare                = GL_COMPARE_REF_TO_TEXTURE
  };

  /* Sync enums */

  // Barrier types for gl::memory_barrier(...)
  enum class BarrierFlags : uint {
    // Data sourced from element buffers reflects shader writes prior to barrier
    eElementArray           = GL_ELEMENT_ARRAY_BARRIER_BIT,

    // Data sourced from attrib buffers reflects shader writes prior to barrier
    eVertexAttribArray      = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,

    // Operations on framebuffer attachments reflect shader writes prior to barrier
    eFramebuffer            = GL_FRAMEBUFFER_BARRIER_BIT,

    // Writes via transform feedback reflect shader writes prior to barrier
    eTransformFeedback      = GL_TRANSFORM_FEEDBACK_BARRIER_BIT,

    // Data sourced from textures reflects shader writes prior to barrier
    eTextureFetch           = GL_TEXTURE_FETCH_BARRIER_BIT,

    // Operations using set*/get*/copy* reflect shader writes prior to barrier
    eTextureUpdate          = GL_TEXTURE_UPDATE_BARRIER_BIT,

    // Shader image load/store/atomics reflect shader writes prior to barrier
    eShaderImageAccess      = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT,

    // Operations using set*/get*/copy*/mapped memory reflect shader writes prior to barrier
    eBufferUpdate           = GL_BUFFER_UPDATE_BARRIER_BIT,

    // Client operations on persistent mapped buffers reflect shader writes prior to barrier
    eClientMappedBuffer     = GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT,

    // Data sourced from bound buffer targets reflects shader writes prior to barrier
    eAtomicCounterBuffer    = GL_ATOMIC_COUNTER_BARRIER_BIT,
    eIndirectBuffer         = GL_COMMAND_BARRIER_BIT,
    eStorageBuffer          = GL_SHADER_STORAGE_BARRIER_BIT,
    ePixelBuffer            = GL_PIXEL_BUFFER_BARRIER_BIT,
    eUniformBuffer          = GL_UNIFORM_BARRIER_BIT,
    eQueryBuffer            = GL_QUERY_BUFFER_BARRIER_BIT,
  };
  gl_declare_bitflag(BarrierFlags);

  /* Vertexarray enums */

  // Format used for gl::VertexArray(...) in gl::VertexAttribInfo(...) object
  enum class VertexAttribType : uint {
    eByte                   = GL_BYTE,
    eUByte                  = GL_UNSIGNED_BYTE,
    eShort                  = GL_SHORT,
    eUShort                 = GL_UNSIGNED_SHORT,
    eInt                    = GL_INT,
    eUInt                   = GL_UNSIGNED_INT,
    eHalf                   = GL_HALF_FLOAT,
    eFloat                  = GL_FLOAT,
    eDouble                 = GL_DOUBLE,
  };

  // Size used for gl::VertexArray(...) in gl::VertexAttribInfo(...) object
  enum class VertexAttribSize : uint {
    e1                      = 1,
    e2                      = 2,
    e3                      = 3,
    e4                      = 4
  };

  // Format used for internal element buffer in gl::ArrayCreateInfo(...) object
  enum class VertexElemType : uint {
    eUByte  = GL_UNSIGNED_BYTE,
    eUShort = GL_UNSIGNED_SHORT,
    eUInt   = GL_UNSIGNED_INT,
  };

  /* Window/context enums */

  // Window hint flags to pass to GLFW
  enum class WindowCreateFlags : uint {
    eDebug                  = 0x001u,
    eDecorated              = 0x002u,
    eFloating               = 0x004u,
    eFullscreen             = 0x008u,
    eFocused                = 0x010u,
    eMaximized              = 0x020u,
    eVisible                = 0x040u,
    eResizable              = 0x080u,
    eSRGB                   = 0x100u,
    eMSAA                   = 0x200u
  };  
  gl_declare_bitflag(WindowCreateFlags);

  // Preferred OpenGL profile for GLFW to support
  enum class ProfileType {
    eAny,
    eCore,
    eCompatibility,
  };
} // namespace gl