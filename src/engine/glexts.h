// GL_ARB_multitexture
extern PFNGLACTIVETEXTUREARBPROC       glActiveTexture_;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture_;
extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2f_;
extern PFNGLMULTITEXCOORD3FARBPROC     glMultiTexCoord3f_;
extern PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4f_;

// GL_ARB_vertex_buffer_object
extern PFNGLGENBUFFERSARBPROC       glGenBuffers_;
extern PFNGLBINDBUFFERARBPROC       glBindBuffer_;
extern PFNGLMAPBUFFERARBPROC        glMapBuffer_;
extern PFNGLUNMAPBUFFERARBPROC      glUnmapBuffer_;
extern PFNGLBUFFERDATAARBPROC       glBufferData_;
extern PFNGLBUFFERSUBDATAARBPROC    glBufferSubData_;
extern PFNGLDELETEBUFFERSARBPROC    glDeleteBuffers_;
extern PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubData_;

// GL_ARB_occlusion_query
extern PFNGLGENQUERIESARBPROC        glGenQueries_;
extern PFNGLDELETEQUERIESARBPROC     glDeleteQueries_;
extern PFNGLBEGINQUERYARBPROC        glBeginQuery_;
extern PFNGLENDQUERYARBPROC          glEndQuery_;
extern PFNGLGETQUERYIVARBPROC        glGetQueryiv_;
extern PFNGLGETQUERYOBJECTIVARBPROC  glGetQueryObjectiv_;
extern PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuiv_;

// GL_EXT_timer_query
#ifndef GL_EXT_timer_query
#define GL_TIME_ELAPSED_EXT               0x88BF
typedef llong GLint64EXT;
typedef ullong GLuint64EXT;
typedef void (APIENTRYP PFNGLGETQUERYOBJECTI64VEXTPROC) (GLuint id, GLenum pname, GLint64EXT *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTUI64VEXTPROC) (GLuint id, GLenum pname, GLuint64EXT *params);
#endif
extern PFNGLGETQUERYOBJECTI64VEXTPROC glGetQueryObjecti64v_;
extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v_;

// GL_EXT_framebuffer_object
extern PFNGLBINDRENDERBUFFEREXTPROC        glBindRenderbuffer_;
extern PFNGLDELETERENDERBUFFERSEXTPROC     glDeleteRenderbuffers_;
extern PFNGLGENFRAMEBUFFERSEXTPROC         glGenRenderbuffers_;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC     glRenderbufferStorage_;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatus_;
extern PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebuffer_;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffers_;
extern PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffers_;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC    glFramebufferTexture1D_;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2D_;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC    glFramebufferTexture3D_;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer_;
extern PFNGLGENERATEMIPMAPEXTPROC          glGenerateMipmap_;

// GL_ARB_draw_buffers
extern PFNGLDRAWBUFFERSARBPROC glDrawBuffers_;

// GL_EXT_framebuffer_blit
#ifndef GL_EXT_framebuffer_blit
#define GL_READ_FRAMEBUFFER_EXT           0x8CA8
#define GL_DRAW_FRAMEBUFFER_EXT           0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT   0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_EXT   0x8CAA
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFEREXTPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif
extern PFNGLBLITFRAMEBUFFEREXTPROC         glBlitFramebuffer_;

// GL_EXT_framebuffer_multisample
#ifndef GL_EXT_framebuffer_multisample
#define GL_RENDERBUFFER_SAMPLES_EXT               0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT 0x8D56
#define GL_MAX_SAMPLES_EXT                        0x8D57
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisample_;

// GL_ARB_texture_multisample
#ifndef GL_ARB_texture_multisample
#define GL_SAMPLE_POSITION                0x8E50
#define GL_SAMPLE_MASK                    0x8E51
#define GL_SAMPLE_MASK_VALUE              0x8E52
#define GL_MAX_SAMPLE_MASK_WORDS          0x8E59
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE   0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#define GL_TEXTURE_SAMPLES                0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY   0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
#define GL_MAX_INTEGER_SAMPLES            0x9110
typedef void (APIENTRYP PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXIMAGE3DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLGETMULTISAMPLEFVPROC) (GLenum pname, GLuint index, GLfloat *val);
typedef void (APIENTRYP PFNGLSAMPLEMASKIPROC) (GLuint index, GLbitfield mask);
#endif
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample_;
extern PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample_;
extern PFNGLGETMULTISAMPLEFVPROC      glGetMultisamplefv_;
extern PFNGLSAMPLEMASKIPROC           glSampleMaski_;

// GL_ARB_sample_shading
#ifndef GL_ARB_sample_shading
#define GL_SAMPLE_SHADING_ARB             0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE_ARB   0x8C37
typedef void (APIENTRYP PFNGLMINSAMPLESHADINGARBPROC) (GLclampf value);
#endif
extern PFNGLMINSAMPLESHADINGARBPROC glMinSampleShading_;

// GL_EXT_framebuffer_multisample_blit_scaled
#ifndef GL_EXT_framebuffer_multisample_blit_scaled
#define GL_SCALED_RESOLVE_FASTEST_EXT     0x90BA
#define GL_SCALED_RESOLVE_NICEST_EXT      0x90BB
#endif

// GL_NV_framebuffer_multisample_coverage
#ifndef GL_NV_framebuffer_multisample_coverage
#define GL_RENDERBUFFER_COVERAGE_SAMPLES_NV  0x8CAB
#define GL_RENDERBUFFER_COLOR_SAMPLES_NV     0x8E10
#define GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV 0x8E11
#define GL_MULTISAMPLE_COVERAGE_MODES_NV     0x8E12
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
#endif
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC glRenderbufferStorageMultisampleCoverageNV_;

// GL_NV_texture_multisample
#ifndef GL_NV_texture_multisample
#define GL_TEXTURE_COVERAGE_SAMPLES_NV    0x9045
#define GL_TEXTURE_COLOR_SAMPLES_NV       0x9046
typedef void (APIENTRYP PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
typedef void (APIENTRYP PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
typedef void (APIENTRYP PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC) (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
typedef void (APIENTRYP PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC) (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
typedef void (APIENTRYP PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC) (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
typedef void (APIENTRYP PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC) (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
#endif
extern PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC     glTexImage2DMultisampleCoverageNV_;
extern PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC     glTexImage3DMultisampleCoverageNV_;
extern PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC         glTextureImage2DMultisampleNV_;
extern PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC         glTextureImage3DMultisampleNV_;
extern PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC glTextureImage2DMultisampleCoverageNV_;
extern PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC glTextureImage3DMultisampleCoverageNV_;

// GL_EXT_draw_range_elements
extern PFNGLDRAWRANGEELEMENTSEXTPROC glDrawRangeElements_;

// GL_EXT_blend_minmax
extern PFNGLBLENDEQUATIONEXTPROC glBlendEquation_;

// GL_EXT_blend_color
extern PFNGLBLENDCOLOREXTPROC glBlendColor_;

// GL_EXT_multi_draw_arrays
extern PFNGLMULTIDRAWARRAYSEXTPROC   glMultiDrawArrays_;
extern PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElements_;

// GL_EXT_packed_depth_stencil
#ifndef GL_DEPTH_STENCIL_EXT
#define GL_DEPTH_STENCIL_EXT 0x84F9
#endif
#ifndef GL_DEPTH24_STENCIL8_EXT
#define GL_DEPTH24_STENCIL8_EXT 0x88F0
#endif
#ifndef GL_UNSIGNED_INT_24_8_EXT
#define GL_UNSIGNED_INT_24_8_EXT 0x84FA
#endif

// GL_EXT_packed_float
#ifndef GL_R11F_G11F_B10F_EXT
#define GL_R11F_G11F_B10F_EXT 0x8C3A
#endif

// GL_ARB_texture_compression
extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC    glCompressedTexImage3D_;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2D_;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC    glCompressedTexImage1D_;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3D_;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2D_;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1D_;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImage_;

// GL_ARB_texture_rg
#ifndef GL_ARB_texture_rg
#define GL_RG                             0x8227
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#endif

// GL_EXT_depth_bounds_test
extern PFNGLDEPTHBOUNDSEXTPROC glDepthBounds_;

// GL_ARB_color_buffer_float
extern PFNGLCLAMPCOLORARBPROC glClampColor_;

// GL_EXT_texture3D
extern PFNGLTEXIMAGE3DEXTPROC        glTexImage3D_;
extern PFNGLTEXSUBIMAGE3DEXTPROC     glTexSubImage3D_;
extern PFNGLCOPYTEXSUBIMAGE3DEXTPROC glCopyTexSubImage3D_;

// GL_ARB_debug_output
#ifndef GL_ARB_debug_output
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB   0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB    0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB  0x8245
#define GL_DEBUG_SOURCE_API_ARB           0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB   0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB   0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB         0x824B
#define GL_DEBUG_TYPE_ERROR_ARB           0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB 0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB     0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB     0x8250
#define GL_DEBUG_TYPE_OTHER_ARB           0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB   0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB  0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB      0x9145
#define GL_DEBUG_SEVERITY_HIGH_ARB        0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB      0x9147
#define GL_DEBUG_SEVERITY_LOW_ARB         0x9148
typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLARBPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (APIENTRYP PFNGLDEBUGMESSAGEINSERTARBPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKARBPROC) (GLDEBUGPROCARB callback, const GLvoid *userParam);
typedef GLuint (APIENTRYP PFNGLGETDEBUGMESSAGELOGARBPROC) (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
#endif
extern PFNGLDEBUGMESSAGECONTROLARBPROC glDebugMessageControl_;
extern PFNGLDEBUGMESSAGEINSERTARBPROC glDebugMessageInsert_;
extern PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallback_;
extern PFNGLGETDEBUGMESSAGELOGARBPROC glGetDebugMessageLog_;

// GL_ARB_map_buffer_range
#ifndef GL_ARB_map_buffer_range
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRYP PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);
#endif
extern PFNGLMAPBUFFERRANGEPROC         glMapBufferRange_;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange_;

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif

// OpenGL 2.0: GL_ARB_shading_language_100, GL_ARB_shader_objects, GL_ARB_fragment_shader, GL_ARB_vertex_shader
#ifdef __APPLE__
#define glCreateProgram_ glCreateProgram
#define glDeleteProgram_ glDeleteProgram
#define glUseProgram_ glUseProgram
#define glCreateShader_ glCreateShader
#define glDeleteShader_ glDeleteShader
#define glShaderSource_ glShaderSource
#define glCompileShader_ glCompileShader
#define glGetShaderiv_ glGetShaderiv
#define glGetProgramiv_ glGetProgramiv
#define glAttachShader_ glAttachShader
#define glGetProgramInfoLog_ glGetProgramInfoLog
#define glGetShaderInfoLog_ glGetShaderInfoLog
#define glLinkProgram_ glLinkProgram
#define glGetUniformLocation_ glGetUniformLocation
#define glUniform1f_ glUniform1f
#define glUniform2f_ glUniform2f
#define glUniform3f_ glUniform3f
#define glUniform4f_ glUniform4f
#define glUniform1fv_ glUniform1fv
#define glUniform2fv_ glUniform2fv
#define glUniform3fv_ glUniform3fv
#define glUniform4fv_ glUniform4fv
#define glUniform1i_ glUniform1i
#define glUniform2i_ glUniform2i
#define glUniform3i_ glUniform3i
#define glUniform4i_ glUniform4i
#define glUniform1iv_ glUniform1iv
#define glUniform2iv_ glUniform2iv
#define glUniform3iv_ glUniform3iv
#define glUniform4iv_ glUniform4iv
#define glUniformMatrix2fv_ glUniformMatrix2fv
#define glUniformMatrix3fv_ glUniformMatrix3fv
#define glUniformMatrix4fv_ glUniformMatrix4fv
#define glBindAttribLocation_ glBindAttribLocation
#define glGetActiveUniform_ glGetActiveUniform
#define glEnableVertexAttribArray_ glEnableVertexAttribArray
#define glDisableVertexAttribArray_ glDisableVertexAttribArray
#define glVertexAttribPointer_ glVertexAttribPointer

#define glUniformMatrix2x3fv_ glUniformMatrix2x3fv
#define glUniformMatrix3x2fv_ glUniformMatrix3x2fv
#define glUniformMatrix2x4fv_ glUniformMatrix2x4fv
#define glUniformMatrix4x2fv_ glUniformMatrix4x2fv
#define glUniformMatrix3x4fv_ glUniformMatrix3x4fv
#define glUniformMatrix4x3fv_ glUniformMatrix4x3fv
#else
extern PFNGLCREATEPROGRAMPROC            glCreateProgram_;
extern PFNGLDELETEPROGRAMPROC            glDeleteProgram_;
extern PFNGLUSEPROGRAMPROC               glUseProgram_;
extern PFNGLCREATESHADERPROC             glCreateShader_;
extern PFNGLDELETESHADERPROC             glDeleteShader_;
extern PFNGLSHADERSOURCEPROC             glShaderSource_;
extern PFNGLCOMPILESHADERPROC            glCompileShader_;
extern PFNGLGETSHADERIVPROC              glGetShaderiv_;
extern PFNGLGETPROGRAMIVPROC             glGetProgramiv_;
extern PFNGLATTACHSHADERPROC             glAttachShader_;
extern PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog_;
extern PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog_;
extern PFNGLLINKPROGRAMPROC              glLinkProgram_;
extern PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation_;
extern PFNGLUNIFORM1FPROC                glUniform1f_;
extern PFNGLUNIFORM2FPROC                glUniform2f_;
extern PFNGLUNIFORM3FPROC                glUniform3f_;
extern PFNGLUNIFORM4FPROC                glUniform4f_;
extern PFNGLUNIFORM1FVPROC               glUniform1fv_;
extern PFNGLUNIFORM2FVPROC               glUniform2fv_;
extern PFNGLUNIFORM3FVPROC               glUniform3fv_;
extern PFNGLUNIFORM4FVPROC               glUniform4fv_;
extern PFNGLUNIFORM1IPROC                glUniform1i_;
extern PFNGLUNIFORM2IPROC                glUniform2i_;
extern PFNGLUNIFORM3IPROC                glUniform3i_;
extern PFNGLUNIFORM4IPROC                glUniform4i_;
extern PFNGLUNIFORM1IVPROC               glUniform1iv_;
extern PFNGLUNIFORM2IVPROC               glUniform2iv_;
extern PFNGLUNIFORM3IVPROC               glUniform3iv_;
extern PFNGLUNIFORM4IVPROC               glUniform4iv_;
extern PFNGLUNIFORMMATRIX2FVPROC         glUniformMatrix2fv_;
extern PFNGLUNIFORMMATRIX3FVPROC         glUniformMatrix3fv_;
extern PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv_;
extern PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation_;
extern PFNGLGETACTIVEUNIFORMPROC         glGetActiveUniform_;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray_;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_;
extern PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer_;

extern PFNGLUNIFORMMATRIX2X3FVPROC       glUniformMatrix2x3fv_;
extern PFNGLUNIFORMMATRIX3X2FVPROC       glUniformMatrix3x2fv_;
extern PFNGLUNIFORMMATRIX2X4FVPROC       glUniformMatrix2x4fv_;
extern PFNGLUNIFORMMATRIX4X2FVPROC       glUniformMatrix4x2fv_;
extern PFNGLUNIFORMMATRIX3X4FVPROC       glUniformMatrix3x4fv_;
extern PFNGLUNIFORMMATRIX4X3FVPROC       glUniformMatrix4x3fv_;
#endif

#ifndef GL_ARB_uniform_buffer_object
#define GL_ARB_uniform_buffer_object 1
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_UNIFORM_BUFFER_BINDING         0x8A28
#define GL_UNIFORM_BUFFER_START           0x8A29
#define GL_UNIFORM_BUFFER_SIZE            0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS      0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS    0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS    0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS    0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS    0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE         0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS          0x8A36
#define GL_UNIFORM_TYPE                   0x8A37
#define GL_UNIFORM_SIZE                   0x8A38
#define GL_UNIFORM_NAME_LENGTH            0x8A39
#define GL_UNIFORM_BLOCK_INDEX            0x8A3A
#define GL_UNIFORM_OFFSET                 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE           0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE          0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR           0x8A3E
#define GL_UNIFORM_BLOCK_BINDING          0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH      0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS  0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX                  0xFFFFFFFFu

typedef void (APIENTRYP PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar* *uniformNames, GLuint *uniformIndices);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
typedef GLuint (APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
#endif

#ifndef GL_INVALID_INDEX
#define GL_INVALID_INDEX                  0xFFFFFFFFu
#endif

#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1
typedef void (APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
#elif GL_GLEXT_VERSION < 43
typedef void (APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
#endif

// GL_ARB_uniform_buffer_object
extern PFNGLGETUNIFORMINDICESPROC       glGetUniformIndices_;
extern PFNGLGETACTIVEUNIFORMSIVPROC     glGetActiveUniformsiv_;
extern PFNGLGETUNIFORMBLOCKINDEXPROC    glGetUniformBlockIndex_;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv_;
extern PFNGLUNIFORMBLOCKBINDINGPROC     glUniformBlockBinding_; 
extern PFNGLBINDBUFFERBASEPROC          glBindBufferBase_;
extern PFNGLBINDBUFFERRANGEPROC         glBindBufferRange_;

#ifndef GL_EXT_bindable_uniform
#define GL_EXT_bindable_uniform 1
#define GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT 0x8DE2
#define GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT 0x8DE3
#define GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT 0x8DE4
#define GL_MAX_BINDABLE_UNIFORM_SIZE_EXT  0x8DED
#define GL_UNIFORM_BUFFER_EXT             0x8DEE
#define GL_UNIFORM_BUFFER_BINDING_EXT     0x8DEF

typedef void (APIENTRYP PFNGLUNIFORMBUFFEREXTPROC) (GLuint program, GLint location, GLuint buffer);
typedef GLint (APIENTRYP PFNGLGETUNIFORMBUFFERSIZEEXTPROC) (GLuint program, GLint location);
typedef GLintptr (APIENTRYP PFNGLGETUNIFORMOFFSETEXTPROC) (GLuint program, GLint location);
#endif

// GL_EXT_bindable_uniform
extern PFNGLUNIFORMBUFFEREXTPROC        glUniformBuffer_;
extern PFNGLGETUNIFORMBUFFERSIZEEXTPROC glGetUniformBufferSize_;
extern PFNGLGETUNIFORMOFFSETEXTPROC     glGetUniformOffset_;

