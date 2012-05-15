// rendergl.cpp: core opengl rendering stuff

#include "engine.h"

bool hasVBO = false, hasDRE = false, hasOQ = false, hasTR = false, hasFBO = false, hasAFBO = false, hasDS = false, hasTF = false, hasCBF = false, hasBE = false, hasBC = false, hasCM = false, hasNP2 = false, hasTC = false, hasMT = false, hasAF = false, hasMDA = false, hasGLSL = false, hasGM = false, hasNVFB = false, hasSGIDT = false, hasSGISH = false, hasDT = false, hasSH = false, hasNVPCF = false, hasPBO = false, hasFBB = false, hasUBO = false, hasBUE = false, hasDB = false, hasTG = false, hasT4 = false, hasTQ = false, hasPF = false, hasTRG = false, hasDBT = false;
bool mesa = false, intel = false, ati = false, nvidia = false;

int hasstencil = 0;

VAR(renderpath, 1, 0, 0);
VAR(glslversion, 1, 0, 0);

// GL_ARB_vertex_buffer_object, GL_ARB_pixel_buffer_object
PFNGLGENBUFFERSARBPROC       glGenBuffers_       = NULL;
PFNGLBINDBUFFERARBPROC       glBindBuffer_       = NULL;
PFNGLMAPBUFFERARBPROC        glMapBuffer_        = NULL;
PFNGLUNMAPBUFFERARBPROC      glUnmapBuffer_      = NULL;
PFNGLBUFFERDATAARBPROC       glBufferData_       = NULL;
PFNGLBUFFERSUBDATAARBPROC    glBufferSubData_    = NULL;
PFNGLDELETEBUFFERSARBPROC    glDeleteBuffers_    = NULL;
PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubData_ = NULL;

// GL_ARB_multitexture
PFNGLACTIVETEXTUREARBPROC       glActiveTexture_       = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture_ = NULL;
PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2f_     = NULL;
PFNGLMULTITEXCOORD3FARBPROC     glMultiTexCoord3f_     = NULL;
PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4f_     = NULL;

// GL_ARB_occlusion_query
PFNGLGENQUERIESARBPROC        glGenQueries_        = NULL;
PFNGLDELETEQUERIESARBPROC     glDeleteQueries_     = NULL;
PFNGLBEGINQUERYARBPROC        glBeginQuery_        = NULL;
PFNGLENDQUERYARBPROC          glEndQuery_          = NULL;
PFNGLGETQUERYIVARBPROC        glGetQueryiv_        = NULL;
PFNGLGETQUERYOBJECTIVARBPROC  glGetQueryObjectiv_  = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuiv_ = NULL;

// GL_EXT_timer_query
PFNGLGETQUERYOBJECTI64VEXTPROC glGetQueryObjecti64v_  = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v_ = NULL;

// GL_EXT_framebuffer_object
PFNGLBINDRENDERBUFFEREXTPROC        glBindRenderbuffer_        = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC     glDeleteRenderbuffers_     = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC         glGenRenderbuffers_        = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC     glRenderbufferStorage_     = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatus_  = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebuffer_         = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffers_      = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffers_         = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2D_    = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer_ = NULL;
PFNGLGENERATEMIPMAPEXTPROC          glGenerateMipmap_          = NULL;

// GL_ARB_draw_buffers
PFNGLDRAWBUFFERSARBPROC glDrawBuffers_ = NULL;

// GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC         glBlitFramebuffer_         = NULL;

// GL_ARB_shading_language_100, GL_ARB_shader_objects, GL_ARB_fragment_shader, GL_ARB_vertex_shader
PFNGLCREATEPROGRAMOBJECTARBPROC       glCreateProgramObject_      = NULL;
PFNGLDELETEOBJECTARBPROC              glDeleteObject_             = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC          glUseProgramObject_         = NULL; 
PFNGLCREATESHADEROBJECTARBPROC        glCreateShaderObject_       = NULL;
PFNGLSHADERSOURCEARBPROC              glShaderSource_             = NULL;
PFNGLCOMPILESHADERARBPROC             glCompileShader_            = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC      glGetObjectParameteriv_     = NULL;
PFNGLATTACHOBJECTARBPROC              glAttachObject_             = NULL;
PFNGLGETINFOLOGARBPROC                glGetInfoLog_               = NULL;
PFNGLLINKPROGRAMARBPROC               glLinkProgram_              = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC        glGetUniformLocation_       = NULL;
PFNGLUNIFORM1FARBPROC                 glUniform1f_                = NULL;
PFNGLUNIFORM2FARBPROC                 glUniform2f_                = NULL;
PFNGLUNIFORM3FARBPROC                 glUniform3f_                = NULL;
PFNGLUNIFORM4FARBPROC                 glUniform4f_                = NULL;
PFNGLUNIFORM1FVARBPROC                glUniform1fv_               = NULL;
PFNGLUNIFORM2FVARBPROC                glUniform2fv_               = NULL;
PFNGLUNIFORM3FVARBPROC                glUniform3fv_               = NULL;
PFNGLUNIFORM4FVARBPROC                glUniform4fv_               = NULL;
PFNGLUNIFORM1IARBPROC                 glUniform1i_                = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC          glUniformMatrix2fv_         = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC          glUniformMatrix3fv_         = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC          glUniformMatrix4fv_         = NULL;
PFNGLBINDATTRIBLOCATIONARBPROC        glBindAttribLocation_       = NULL;
PFNGLGETACTIVEUNIFORMARBPROC          glGetActiveUniform_         = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC   glEnableVertexAttribArray_  = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC  glDisableVertexAttribArray_ = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC       glVertexAttribPointer_      = NULL;

// GL_EXT_draw_range_elements
PFNGLDRAWRANGEELEMENTSEXTPROC glDrawRangeElements_ = NULL;

// GL_EXT_blend_minmax
PFNGLBLENDEQUATIONEXTPROC glBlendEquation_ = NULL;

// GL_EXT_blend_color
PFNGLBLENDCOLOREXTPROC glBlendColor_ = NULL;

// GL_EXT_multi_draw_arrays
PFNGLMULTIDRAWARRAYSEXTPROC   glMultiDrawArrays_   = NULL;
PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElements_ = NULL;

// GL_ARB_texture_compression
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC    glCompressedTexImage3D_    = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2D_    = NULL;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC    glCompressedTexImage1D_    = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3D_ = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2D_ = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1D_ = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImage_   = NULL;

// GL_ARB_uniform_buffer_object
PFNGLGETUNIFORMINDICESPROC       glGetUniformIndices_       = NULL;
PFNGLGETACTIVEUNIFORMSIVPROC     glGetActiveUniformsiv_     = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC    glGetUniformBlockIndex_    = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv_ = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC     glUniformBlockBinding_     = NULL;
PFNGLBINDBUFFERBASEPROC          glBindBufferBase_          = NULL;
PFNGLBINDBUFFERRANGEPROC         glBindBufferRange_         = NULL;

// GL_EXT_bindable_uniform
PFNGLUNIFORMBUFFEREXTPROC        glUniformBuffer_        = NULL;
PFNGLGETUNIFORMBUFFERSIZEEXTPROC glGetUniformBufferSize_ = NULL;
PFNGLGETUNIFORMOFFSETEXTPROC     glGetUniformOffset_     = NULL;

// GL_EXT_depth_bounds_test
PFNGLDEPTHBOUNDSEXTPROC glDepthBounds_ = NULL;

// GL_ARB_color_buffer_float
PFNGLCLAMPCOLORARBPROC glClampColor_ = NULL;

void *getprocaddress(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

void glerror(const char *file, int line, GLenum error)
{
    const char *desc = "unknown";
    switch(error)
    {
    case GL_NO_ERROR: desc = "no error"; break;
    case GL_INVALID_ENUM: desc = "invalid enum"; break;
    case GL_INVALID_VALUE: desc = "invalid value"; break;
    case GL_INVALID_OPERATION: desc = "invalid operation"; break;
    case GL_STACK_OVERFLOW: desc = "stack overflow"; break;
    case GL_STACK_UNDERFLOW: desc = "stack underflow"; break;
    case GL_OUT_OF_MEMORY: desc = "out of memory"; break;
    }
    printf("GL error: %s:%d: %s (%x)\n", file, line, desc, error);
}

VAR(ati_oq_bug, 0, 0, 1);
VAR(ati_minmax_bug, 0, 0, 1);
VAR(ati_cubemap_bug, 0, 0, 1);
VAR(ati_ubo_bug, 0, 0, 1);
VAR(sdl_backingstore_bug, -1, 0, 1);
VAR(usetexrect, 1, 0, 0);
VAR(useubo, 1, 0, 0);
VAR(usebue, 1, 0, 0);
VAR(usetexgather, 1, 0, 0);

#if 0
static bool checkseries(const char *s, int low, int high)
{
    while(*s && !isdigit(*s)) ++s;
    if(!*s) return false;
    int n = 0;
    while(isdigit(*s)) n = n*10 + (*s++ - '0');    
    return n >= low && n < high;
}
#endif

VAR(dbgexts, 0, 0, 1);

bool hasext(const char *exts, const char *ext)
{
    int len = strlen(ext);
    if(len) for(const char *cur = exts; (cur = strstr(cur, ext)); cur += len)
    {
        if((cur == exts || cur[-1] == ' ') && (cur[len] == ' ' || !cur[len])) return true;
    }
    return false;
}

void gl_checkextensions()
{
    const char *vendor = (const char *)glGetString(GL_VENDOR);
    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    const char *renderer = (const char *)glGetString(GL_RENDERER);
    const char *version = (const char *)glGetString(GL_VERSION);
    conoutf(CON_INIT, "Renderer: %s (%s)", renderer, vendor);
    conoutf(CON_INIT, "Driver: %s", version);

#ifdef __APPLE__
    extern int mac_osversion();
    int osversion = mac_osversion();  /* 0x1050 = 10.5 (Leopard) */
    sdl_backingstore_bug = -1;
#endif

    if(strstr(renderer, "Mesa") || strstr(version, "Mesa"))
        mesa = true;
    else if(strstr(vendor, "NVIDIA"))
        nvidia = true;
    else if(strstr(vendor, "ATI") || strstr(vendor, "Advanced Micro Devices"))
        ati = true;
    else if(strstr(vendor, "Intel"))
        intel = true;

    //extern int shaderprecision;
    // default to low precision shaders on certain cards, can be overridden with -f3
    // char *weakcards[] = { "GeForce FX", "Quadro FX", "6200", "9500", "9550", "9600", "9700", "9800", "X300", "X600", "FireGL", "Intel", "Chrome", NULL } 
    // if(shaderprecision==2) for(char **wc = weakcards; *wc; wc++) if(strstr(renderer, *wc)) shaderprecision = 1;

    GLint val;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
    hwtexsize = val;

    if(hasext(exts, "GL_ARB_multitexture"))
    {
        glActiveTexture_       = (PFNGLACTIVETEXTUREARBPROC)      getprocaddress("glActiveTextureARB");
        glClientActiveTexture_ = (PFNGLCLIENTACTIVETEXTUREARBPROC)getprocaddress("glClientActiveTextureARB");
        glMultiTexCoord2f_     = (PFNGLMULTITEXCOORD2FARBPROC)    getprocaddress("glMultiTexCoord2fARB");
        glMultiTexCoord3f_     = (PFNGLMULTITEXCOORD3FARBPROC)    getprocaddress("glMultiTexCoord3fARB");
        glMultiTexCoord4f_     = (PFNGLMULTITEXCOORD4FARBPROC)    getprocaddress("glMultiTexCoord4fARB");
        hasMT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_multitexture extension.");
    }
    else conoutf(CON_WARN, "WARNING: No multitexture extension!");


    if(hasext(exts, "GL_ARB_vertex_buffer_object")) 
    {
        hasVBO = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_vertex_buffer_object extension.");
    }
    else conoutf(CON_WARN, "WARNING: No vertex_buffer_object extension! (geometry heavy maps will be SLOW)");
#ifdef __APPLE__
    /* VBOs over 256KB seem to destroy performance on 10.5, but not in 10.6 */
    extern int maxvbosize;
    if(osversion < 0x1060) maxvbosize = min(maxvbosize, 8192);  
#endif

    if(hasext(exts, "GL_ARB_pixel_buffer_object"))
    {
        hasPBO = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_pixel_buffer_object extension.");
    }

    if(hasVBO || hasPBO)
    {
        glGenBuffers_       = (PFNGLGENBUFFERSARBPROC)      getprocaddress("glGenBuffersARB");
        glBindBuffer_       = (PFNGLBINDBUFFERARBPROC)      getprocaddress("glBindBufferARB");
        glMapBuffer_        = (PFNGLMAPBUFFERARBPROC)       getprocaddress("glMapBufferARB");
        glUnmapBuffer_      = (PFNGLUNMAPBUFFERARBPROC)     getprocaddress("glUnmapBufferARB");
        glBufferData_       = (PFNGLBUFFERDATAARBPROC)      getprocaddress("glBufferDataARB");
        glBufferSubData_    = (PFNGLBUFFERSUBDATAARBPROC)   getprocaddress("glBufferSubDataARB");
        glDeleteBuffers_    = (PFNGLDELETEBUFFERSARBPROC)   getprocaddress("glDeleteBuffersARB");
        glGetBufferSubData_ = (PFNGLGETBUFFERSUBDATAARBPROC)getprocaddress("glGetBufferSubDataARB");
    }

    if(hasext(exts, "GL_EXT_draw_range_elements"))
    {
        glDrawRangeElements_ = (PFNGLDRAWRANGEELEMENTSEXTPROC)getprocaddress("glDrawRangeElementsEXT");
        hasDRE = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_draw_range_elements extension.");
    }

    if(hasext(exts, "GL_EXT_multi_draw_arrays"))
    {
        glMultiDrawArrays_   = (PFNGLMULTIDRAWARRAYSEXTPROC)  getprocaddress("glMultiDrawArraysEXT");
        glMultiDrawElements_ = (PFNGLMULTIDRAWELEMENTSEXTPROC)getprocaddress("glMultiDrawElementsEXT");
        hasMDA = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_multi_draw_arrays extension.");
    }

#ifdef __APPLE__
    // floating point FBOs not fully supported until 10.5
    if(osversion>=0x1050)
#endif
    if(hasext(exts, "GL_ARB_texture_float") || hasext(exts, "GL_ATI_texture_float"))
    {
        hasTF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_float extension.");
    }

    if(hasext(exts, "GL_NV_float_buffer")) 
    {
        hasNVFB = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_NV_float_buffer extension.");
    }

    if(hasext(exts, "GL_ARB_texture_rg"))
    {
        hasTRG = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_rg extension.");
    }

    if(hasext(exts, "GL_EXT_framebuffer_object"))
    {
        glBindRenderbuffer_        = (PFNGLBINDRENDERBUFFEREXTPROC)       getprocaddress("glBindRenderbufferEXT");
        glDeleteRenderbuffers_     = (PFNGLDELETERENDERBUFFERSEXTPROC)    getprocaddress("glDeleteRenderbuffersEXT");
        glGenRenderbuffers_        = (PFNGLGENFRAMEBUFFERSEXTPROC)        getprocaddress("glGenRenderbuffersEXT");
        glRenderbufferStorage_     = (PFNGLRENDERBUFFERSTORAGEEXTPROC)    getprocaddress("glRenderbufferStorageEXT");
        glCheckFramebufferStatus_  = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) getprocaddress("glCheckFramebufferStatusEXT");
        glBindFramebuffer_         = (PFNGLBINDFRAMEBUFFEREXTPROC)        getprocaddress("glBindFramebufferEXT");
        glDeleteFramebuffers_      = (PFNGLDELETEFRAMEBUFFERSEXTPROC)     getprocaddress("glDeleteFramebuffersEXT");
        glGenFramebuffers_         = (PFNGLGENFRAMEBUFFERSEXTPROC)        getprocaddress("glGenFramebuffersEXT");
        glFramebufferTexture2D_    = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)   getprocaddress("glFramebufferTexture2DEXT");
        glFramebufferRenderbuffer_ = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)getprocaddress("glFramebufferRenderbufferEXT");
        glGenerateMipmap_          = (PFNGLGENERATEMIPMAPEXTPROC)         getprocaddress("glGenerateMipmapEXT");
        hasFBO = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_framebuffer_object extension.");

        if(hasext(exts, "GL_ARB_framebuffer_object"))
        {
            hasAFBO = true;
            if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_framebuffer_object extension.");
        } 
        if(hasext(exts, "GL_EXT_framebuffer_blit"))
        {
            glBlitFramebuffer_     = (PFNGLBLITFRAMEBUFFEREXTPROC)        getprocaddress("glBlitFramebufferEXT");
            hasFBB = true;
            if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_framebuffer_blit extension.");
        }
    }
    //else conoutf(CON_WARN, "WARNING: No framebuffer object support. (reflective water may be slow)");
    else fatal("Framebuffer object support is required!");

    if(hasext(exts, "GL_ARB_draw_buffers"))
    {
        glDrawBuffers_ = (PFNGLDRAWBUFFERSARBPROC)getprocaddress("glDrawBuffersARB");
        hasDB = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_draw_buffers extension.");
        GLint maxbufs = 0;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &maxbufs);
        if(maxbufs < 3) fatal("Hardware does not support at least 3 draw buffers.");
    }
    else fatal("Draw buffers support is required!");

    if(hasext(exts, "GL_ARB_color_buffer_float"))
    {
        glClampColor_ = (PFNGLCLAMPCOLORARBPROC)getprocaddress("glClampColorARB");
        hasCBF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_color_buffer_float extension.");
    }

#ifdef __APPLE__
    // Intel HD3000 broke occlusion queries - either causing software fallback, or returning wrong results
    if(!intel)
#endif	   
    if(hasext(exts, "GL_ARB_occlusion_query"))
    {
        GLint bits;
        glGetQueryiv_ = (PFNGLGETQUERYIVARBPROC)getprocaddress("glGetQueryivARB");
        glGetQueryiv_(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bits);
        if(bits)
        {
            glGenQueries_ =        (PFNGLGENQUERIESARBPROC)       getprocaddress("glGenQueriesARB");
            glDeleteQueries_ =     (PFNGLDELETEQUERIESARBPROC)    getprocaddress("glDeleteQueriesARB");
            glBeginQuery_ =        (PFNGLBEGINQUERYARBPROC)       getprocaddress("glBeginQueryARB");
            glEndQuery_ =          (PFNGLENDQUERYARBPROC)         getprocaddress("glEndQueryARB");
            glGetQueryObjectiv_ =  (PFNGLGETQUERYOBJECTIVARBPROC) getprocaddress("glGetQueryObjectivARB");
            glGetQueryObjectuiv_ = (PFNGLGETQUERYOBJECTUIVARBPROC)getprocaddress("glGetQueryObjectuivARB");
            hasOQ = true;
            if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_occlusion_query extension.");
#if defined(__APPLE__) && SDL_BYTEORDER == SDL_BIG_ENDIAN
            if(ati && (osversion<0x1050)) ati_oq_bug = 1;
#endif
            //if(ati_oq_bug) conoutf(CON_WARN, "WARNING: Using ATI occlusion query bug workaround. (use \"/ati_oq_bug 0\" to disable if unnecessary)");
        }
    }
    if(!hasOQ)
    {
        conoutf(CON_WARN, "WARNING: No occlusion query support! (large maps may be SLOW)");
        extern int vacubesize;
        vacubesize = 64;
        waterreflect = 0;
    }

    if(hasext(exts, "GL_EXT_timer_query"))
    {
        glGetQueryiv_ =          (PFNGLGETQUERYIVARBPROC)       getprocaddress("glGetQueryivARB");
        glGenQueries_ =          (PFNGLGENQUERIESARBPROC)       getprocaddress("glGenQueriesARB");
        glDeleteQueries_ =       (PFNGLDELETEQUERIESARBPROC)    getprocaddress("glDeleteQueriesARB");
        glBeginQuery_ =          (PFNGLBEGINQUERYARBPROC)       getprocaddress("glBeginQueryARB");
        glEndQuery_ =            (PFNGLENDQUERYARBPROC)         getprocaddress("glEndQueryARB");
        glGetQueryObjectiv_ =    (PFNGLGETQUERYOBJECTIVARBPROC) getprocaddress("glGetQueryObjectivARB");
        glGetQueryObjectuiv_ =   (PFNGLGETQUERYOBJECTUIVARBPROC)getprocaddress("glGetQueryObjectuivARB");
        glGetQueryObjecti64v_ =  (PFNGLGETQUERYOBJECTI64VEXTPROC)  getprocaddress("glGetQueryObjecti64vEXT");
        glGetQueryObjectui64v_ = (PFNGLGETQUERYOBJECTUI64VEXTPROC) getprocaddress("glGetQueryObjectui64vEXT");
        hasTQ = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_timer_query extension.");
    }

    extern int gdepthstencil;
    if(ati)
    {
        //conoutf(CON_WARN, "WARNING: ATI cards may show garbage in skybox. (use \"/ati_skybox_bug 1\" to fix)");
        if(!hasext(exts, "GL_ARB_gpu_shader5")) gdepthstencil = 0; // some older ATI GPUs do not support reading from depth-stencil textures, so only use depth-stencil renderbuffer for now
    }
    else if(nvidia)
    {
    }
    else
    {
        // silence warnings
        (void)intel;
        (void)mesa;

        if(!hasext(exts, "GL_EXT_gpu_shader4"))
        {
            if(hwtexsize < 4096) 
            {
                maxtexsize = hwtexsize >= 2048 ? 512 : 256;
            }
        }
    }

    if(hasext(exts, "GL_ARB_shading_language_100") && hasext(exts, "GL_ARB_shader_objects") && hasext(exts, "GL_ARB_vertex_shader") && hasext(exts, "GL_ARB_fragment_shader"))
    {
        glCreateProgramObject_ =        (PFNGLCREATEPROGRAMOBJECTARBPROC)     getprocaddress("glCreateProgramObjectARB");
        glDeleteObject_ =               (PFNGLDELETEOBJECTARBPROC)            getprocaddress("glDeleteObjectARB");
        glUseProgramObject_ =           (PFNGLUSEPROGRAMOBJECTARBPROC)        getprocaddress("glUseProgramObjectARB");
        glCreateShaderObject_ =         (PFNGLCREATESHADEROBJECTARBPROC)      getprocaddress("glCreateShaderObjectARB");
        glShaderSource_ =               (PFNGLSHADERSOURCEARBPROC)            getprocaddress("glShaderSourceARB");
        glCompileShader_ =              (PFNGLCOMPILESHADERARBPROC)           getprocaddress("glCompileShaderARB");
        glGetObjectParameteriv_ =       (PFNGLGETOBJECTPARAMETERIVARBPROC)    getprocaddress("glGetObjectParameterivARB");
        glAttachObject_ =               (PFNGLATTACHOBJECTARBPROC)            getprocaddress("glAttachObjectARB");
        glGetInfoLog_ =                 (PFNGLGETINFOLOGARBPROC)              getprocaddress("glGetInfoLogARB");
        glLinkProgram_ =                (PFNGLLINKPROGRAMARBPROC)             getprocaddress("glLinkProgramARB");
        glGetUniformLocation_ =         (PFNGLGETUNIFORMLOCATIONARBPROC)      getprocaddress("glGetUniformLocationARB");
        glUniform1f_ =                  (PFNGLUNIFORM1FARBPROC)               getprocaddress("glUniform1fARB");
        glUniform2f_ =                  (PFNGLUNIFORM2FARBPROC)               getprocaddress("glUniform2fARB");
        glUniform3f_ =                  (PFNGLUNIFORM3FARBPROC)               getprocaddress("glUniform3fARB");
        glUniform4f_ =                  (PFNGLUNIFORM4FARBPROC)               getprocaddress("glUniform4fARB");
        glUniform1fv_ =                 (PFNGLUNIFORM1FVARBPROC)              getprocaddress("glUniform1fvARB");
        glUniform2fv_ =                 (PFNGLUNIFORM2FVARBPROC)              getprocaddress("glUniform2fvARB");
        glUniform3fv_ =                 (PFNGLUNIFORM3FVARBPROC)              getprocaddress("glUniform3fvARB");
        glUniform4fv_ =                 (PFNGLUNIFORM4FVARBPROC)              getprocaddress("glUniform4fvARB");
        glUniform1i_ =                  (PFNGLUNIFORM1IARBPROC)               getprocaddress("glUniform1iARB");
        glUniformMatrix2fv_ =           (PFNGLUNIFORMMATRIX2FVARBPROC)        getprocaddress("glUniformMatrix2fvARB");
        glUniformMatrix3fv_ =           (PFNGLUNIFORMMATRIX3FVARBPROC)        getprocaddress("glUniformMatrix3fvARB");          
        glUniformMatrix4fv_ =           (PFNGLUNIFORMMATRIX4FVARBPROC)        getprocaddress("glUniformMatrix4fvARB");          
        glBindAttribLocation_ =         (PFNGLBINDATTRIBLOCATIONARBPROC)      getprocaddress("glBindAttribLocationARB");
        glGetActiveUniform_ =           (PFNGLGETACTIVEUNIFORMARBPROC)        getprocaddress("glGetActiveUniformARB");
        glEnableVertexAttribArray_ =    (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) getprocaddress("glEnableVertexAttribArrayARB");
        glDisableVertexAttribArray_ =   (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)getprocaddress("glDisableVertexAttribArrayARB");
        glVertexAttribPointer_ =        (PFNGLVERTEXATTRIBPOINTERARBPROC)     getprocaddress("glVertexAttribPointerARB");

        extern bool checkglslsupport();
        if(checkglslsupport()) hasGLSL = true;
        
        const char *str = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
        conoutf(CON_INIT, "GLSL: %s", str ? str : "unknown");

        uint majorversion, minorversion;
        if(!str || sscanf(str, " %u.%u", &majorversion, &minorversion) != 2) glslversion = 100;
        else glslversion = majorversion*100 + minorversion; 
        
    }
    if(!hasGLSL || glslversion < 100) fatal("GLSL support is required!");
 
    if(hasext(exts, "GL_ARB_uniform_buffer_object"))
    {
        glGetUniformIndices_       = (PFNGLGETUNIFORMINDICESPROC)      getprocaddress("glGetUniformIndices");
        glGetActiveUniformsiv_     = (PFNGLGETACTIVEUNIFORMSIVPROC)    getprocaddress("glGetActiveUniformsiv");
        glGetUniformBlockIndex_    = (PFNGLGETUNIFORMBLOCKINDEXPROC)   getprocaddress("glGetUniformBlockIndex");
        glGetActiveUniformBlockiv_ = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)getprocaddress("glGetActiveUniformBlockiv");
        glUniformBlockBinding_     = (PFNGLUNIFORMBLOCKBINDINGPROC)    getprocaddress("glUniformBlockBinding");
        glBindBufferBase_          = (PFNGLBINDBUFFERBASEPROC)         getprocaddress("glBindBufferBase");
        glBindBufferRange_         = (PFNGLBINDBUFFERRANGEPROC)        getprocaddress("glBindBufferRange");

        useubo = 1;
        hasUBO = true;
        if(ati) ati_ubo_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_uniform_buffer_object extension.");
    }
    else if(hasext(exts, "GL_EXT_bindable_uniform"))
    {
        glUniformBuffer_        = (PFNGLUNIFORMBUFFEREXTPROC)       getprocaddress("glUniformBufferEXT");
        glGetUniformBufferSize_ = (PFNGLGETUNIFORMBUFFERSIZEEXTPROC)getprocaddress("glGetUniformBufferSizeEXT");
        glGetUniformOffset_     = (PFNGLGETUNIFORMOFFSETEXTPROC)    getprocaddress("glGetUniformOffsetEXT");

        usebue = 1;
        hasBUE = true;
        if(ati) ati_ubo_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_bindable_uniform extension.");
    }

    if(hasext(exts, "GL_EXT_texture_rectangle") || hasext(exts, "GL_ARB_texture_rectangle"))
    {
        usetexrect = 1;
        hasTR = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_rectangle extension.");
    }
    //else if(hasMT) conoutf(CON_WARN, "WARNING: No texture rectangle support. (no full screen shaders)");
    else fatal("Texture rectangle support is required!");

    if(hasext(exts, "GL_EXT_packed_depth_stencil") || hasext(exts, "GL_NV_packed_depth_stencil"))
    {
        hasDS = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_packed_depth_stencil extension.");
    }

    if(hasext(exts, "GL_EXT_packed_float"))
    {
        hasPF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_packed_float extension.");
    }

    if(hasext(exts, "GL_EXT_blend_minmax"))
    {
        glBlendEquation_ = (PFNGLBLENDEQUATIONEXTPROC) getprocaddress("glBlendEquationEXT");
        hasBE = true;
        if(ati) ati_minmax_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_blend_minmax extension.");
    }

    if(hasext(exts, "GL_EXT_blend_color"))
    {
        glBlendColor_ = (PFNGLBLENDCOLOREXTPROC) getprocaddress("glBlendColorEXT");
        hasBC = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_blend_color extension.");
    }

    if(hasext(exts, "GL_ARB_texture_cube_map"))
    {
        GLint val;
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &val);
        hwcubetexsize = val;
        hasCM = true;
        // On Catalyst 10.2, issuing an occlusion query on the first draw using a given cubemap texture causes a nasty crash
        if(ati) ati_cubemap_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_cube_map extension.");
    }
    else conoutf(CON_WARN, "WARNING: No cube map texture support. (no reflective glass)");

    if(hasext(exts, "GL_ARB_texture_non_power_of_two"))
    {
        hasNP2 = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_non_power_of_two extension.");
    }
    else fatal("Non-power-of-two texture support is required!");

    if(hasext(exts, "GL_ARB_texture_compression") && hasext(exts, "GL_EXT_texture_compression_s3tc"))
    {
        glCompressedTexImage3D_ =    (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)   getprocaddress("glCompressedTexImage3DARB");
        glCompressedTexImage2D_ =    (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)   getprocaddress("glCompressedTexImage2DARB");
        glCompressedTexImage1D_ =    (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)   getprocaddress("glCompressedTexImage1DARB");
        glCompressedTexSubImage3D_ = (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)getprocaddress("glCompressedTexSubImage3DARB");
        glCompressedTexSubImage2D_ = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)getprocaddress("glCompressedTexSubImage2DARB");
        glCompressedTexSubImage1D_ = (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)getprocaddress("glCompressedTexSubImage1DARB");
        glGetCompressedTexImage_ =   (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)  getprocaddress("glGetCompressedTexImageARB");

        hasTC = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_texture_compression_s3tc extension.");
    }

    if(hasext(exts, "GL_EXT_texture_filter_anisotropic"))
    {
       GLint val;
       glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &val);
       hwmaxaniso = val;
       hasAF = true;
       if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_texture_filter_anisotropic extension.");
    }

    if(hasext(exts, "GL_SGIS_generate_mipmap"))
    {
        hasGM = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_SGIS_generate_mipmap extension.");
    }

    if(hasext(exts, "GL_ARB_depth_texture"))
    {
        hasSGIDT = hasDT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_depth_texture extension.");
    }
    else if(hasext(exts, "GL_SGIX_depth_texture"))
    {
        hasSGIDT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_SGIX_depth_texture extension.");
    }

    if(hasext(exts, "GL_ARB_shadow"))
    {
        hasSGISH = hasSH = true;
        if(nvidia || (ati && strstr(renderer, "Radeon HD"))) hasNVPCF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_shadow extension.");
    }
    else if(hasext(exts, "GL_SGIX_shadow"))
    {
        hasSGISH = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_SGIX_shadow extension.");
    }

    if(hasext(exts, "GL_ARB_texture_gather"))
    {
        hasTG = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_gather extension.");
    }
    else if(hasext(exts, "GL_AMD_texture_texture4"))
    {
        if(dbgexts) conoutf(CON_INIT, "Using GL_AMD_texture_texture4 extension.");
    }
    if(hasTG || hasT4) usetexgather = 1;

    if(hasext(exts, "GL_EXT_depth_bounds_test"))
    {
        glDepthBounds_ = (PFNGLDEPTHBOUNDSEXTPROC) getprocaddress("glDepthBoundsEXT");
        hasDBT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_depth_bounds_test extension.");
    }
}

void glext(char *ext)
{
    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    intret(hasext(exts, ext) ? 1 : 0);
}
COMMAND(glext, "s");

// GPU-side timing information will use OGL timers
enum { TIMER_SM = 0, TIMER_GBUFFER, TIMER_SHADING, TIMER_HDR, TIMER_AO, TIMER_TRANSPARENT, TIMER_SMAA, TIMER_SPLITTING, TIMER_MERGING, TIMER_CPU_SM, TIMER_CPU_GBUFFER, TIMER_CPU_SHADING, TIMER_N };
#define TIMER_GPU_N TIMER_CPU_SM
static const char *timer_string[] = {
    "shadow map",
    "g-buffer",
    "deferred shading",
    "hdr processing",
    "ambient obscurance",
    "transparent",
    "smaa",
    "buffer splitting",
    "buffer merging",

    "shadow map",
    "g-buffer",
    "deferred shading"
};
// gpu
static const int timer_query_n = 4;
static int timer_curr = 0;
static GLuint timers[timer_query_n][TIMER_GPU_N];
static uint timer_used[timer_query_n];
// cpu
static int timer_began[TIMER_N];

static uint timer_available = 0;
static float timer_results[TIMER_N];
static float timer_prints[TIMER_N];
static int timer_last_print = 0;
VAR(timer, 0, 0, 1);

static void timer_sync()
{
    if(!timer) return;
    if(hasTQ) 
    {
        timer_curr = (timer_curr+1) % timer_query_n;
        loopi(TIMER_GPU_N)
        {
            if(timer_used[timer_curr]&(1<<i))
            {
                GLint available = 0;
                while(!available)
                    glGetQueryObjectiv_(timers[timer_curr][i], GL_QUERY_RESULT_AVAILABLE_ARB, &available);
                GLuint64EXT result = 0;
                glGetQueryObjectui64v_(timers[timer_curr][i], GL_QUERY_RESULT_ARB, &result);
                timer_results[i] = float(result) * 1e-6f;
                timer_available |= 1<<i;
            }
        }
        timer_used[timer_curr] = 0;
    }
}
static inline void timer_begin(int whichone)
{
    if(!timer || inbetweenframes) return;
    if(whichone < TIMER_GPU_N)
    {
        if(!hasTQ) return;
        glBeginQuery_(GL_TIME_ELAPSED_EXT, timers[timer_curr][whichone]);
    }
    else timer_began[whichone] = getclockmillis();
}
static inline void timer_end(int whichone)
{
    if(!timer || inbetweenframes) return;
    if(whichone < TIMER_GPU_N)
    {
        if(!hasTQ) return; 
        glEndQuery_(GL_TIME_ELAPSED_EXT);
        timer_used[timer_curr] |= 1<<whichone;
    }
    else 
    {
        timer_results[whichone] = float(getclockmillis() - timer_began[whichone]);
        timer_available |= 1<<whichone;
    }
}
static inline void timer_nextframe()
{
    timer_available = 0;
}
static void timer_print(int conw, int conh)
{
    if(!timer || !timer_available) return;
    if(totalmillis - timer_last_print >= 200) // keep the timer read-outs from looking spastic on the hud
    {
        memcpy(timer_prints, timer_results, sizeof(timer_prints));
        timer_last_print = totalmillis;
    }        
    int offset = 0;
    loopi(TIMER_N)
    {
        if(!(timer_available&(1<<i))) continue;
        draw_textf("%s%s %5.2f ms", conw-20*FONTH, conh-FONTH*3/2-offset*9*FONTH/8, timer_string[i], (i < TIMER_GPU_N)?"":" (cpu)", timer_prints[i]);
        offset++;
    }
}
static void timer_setup()
{
    memset(timer_used, 0, sizeof(timer_used));
    memset(timer_results, 0, sizeof(timer_results));
    memset(timer_prints, 0, sizeof(timer_prints));
    if(hasTQ) loopi(timer_query_n) glGenQueries_(TIMER_GPU_N, timers[i]);
}
void cleanuptimer() 
{ 
    if(hasTQ) loopi(timer_query_n) glDeleteQueries_(TIMER_GPU_N, timers[i]); 
}
static void frametimer_print(int conw, int conh)
{
    extern int framemillis, frametimer;
    if(!frametimer) return;
    static int lastprint = 0, printmillis = 0;
    if(totalmillis - lastprint >= 200) 
    {
        printmillis = framemillis;
        lastprint = totalmillis;
    }
    draw_textf("frame time %i ms", conw-40*FONTH, conh-FONTH*3/2, printmillis);
}

void gl_init(int w, int h, int bpp, int depth, int fsaa)
{
    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 0);
    glClearDepth(1);
    glClearStencil(0);
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glShadeModel(GL_SMOOTH);
    
    
    glDisable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    //glHint(GL_FOG_HINT, GL_NICEST);
    GLfloat fogcolor[4] = { 0, 0, 0, 0 };
    glFogfv(GL_FOG_COLOR, fogcolor);
    

    glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

#ifdef __APPLE__
    if(sdl_backingstore_bug)
    {
        if(fsaa)
        {
            sdl_backingstore_bug = 1;
            // since SDL doesn't add kCGLPFABackingStore to the pixelformat and so it isn't guaranteed to be preserved - only manifests when using fsaa?
            //conoutf(CON_WARN, "WARNING: Using SDL backingstore workaround. (use \"/sdl_backingstore_bug 0\" to disable if unnecessary)");
        }
        else sdl_backingstore_bug = -1;
    }
#endif

    renderpath = R_GLSLANG;

    static const char * const rpnames[1] = { "GLSL shader" };
    conoutf(CON_INIT, "Rendering using the OpenGL %s path.", rpnames[renderpath]);

    extern void initgbuffer();
    initgbuffer();

    setuptexcompress();
    timer_setup();

    vieww = w;
    viewh = h;
}

void cleanupgl()
{
    extern void cleanupmotionblur();
    cleanupmotionblur();

    extern void clearminimap();
    clearminimap();

    extern void cleanupgbuffer();
    cleanupgbuffer();

    extern void cleanupbloom();
    cleanupbloom();

    extern void cleanupao();
    cleanupao();

    extern void cleanupsmaa();
    cleanupsmaa();

    extern void cleanupshadowatlas();
    cleanupshadowatlas();

    extern void cleanuptimer();
    cleanuptimer();
}

#define VARRAY_INTERNAL
#include "varray.h"

VAR(wireframe, 0, 0, 1);

ICOMMAND(getcamyaw, "", (), floatret(camera1->yaw));
ICOMMAND(getcampitch, "", (), floatret(camera1->pitch));
ICOMMAND(getcamroll, "", (), floatret(camera1->roll));
ICOMMAND(getcampos, "", (), 
{
    defformatstring(pos)("%s %s %s", floatstr(camera1->o.x), floatstr(camera1->o.y), floatstr(camera1->o.z));
    result(pos);
});

vec worldpos, camdir, camright, camup;

void findorientation()
{
    vecfromyawpitch(camera1->yaw, camera1->pitch, 1, 0, camdir);
    vecfromyawpitch(camera1->yaw, 0, 0, -1, camright);
    vecfromyawpitch(camera1->yaw, camera1->pitch+90, 1, 0, camup);

    if(raycubepos(camera1->o, camdir, worldpos, 0, RAY_CLIPMAT|RAY_SKIPFIRST) == -1)
        worldpos = vec(camdir).mul(2*worldsize).add(camera1->o); //otherwise 3dgui won't work when outside of map
}

void transplayer()
{
    // move from RH to Z-up LH quake style worldspace
    glLoadMatrixf(viewmatrix.v);

    glRotatef(camera1->roll, 0, 1, 0);
    glRotatef(camera1->pitch, -1, 0, 0);
    glRotatef(camera1->yaw, 0, 0, -1);

    glTranslatef(-camera1->o.x, -camera1->o.y, -camera1->o.z);   
}

int vieww = -1, viewh = -1;
float curfov = 100, curavatarfov = 65, fovy, aspect;
int farplane;
VARP(zoominvel, 0, 250, 5000);
VARP(zoomoutvel, 0, 100, 5000);
VARP(zoomfov, 10, 35, 60);
VARP(fov, 10, 100, 150);
VAR(avatarzoomfov, 10, 25, 60);
VAR(avatarfov, 10, 65, 150);
FVAR(avatardepth, 0, 0.5f, 1);

static int zoommillis = 0;
VARF(zoom, -1, 0, 1,
    if(zoom) zoommillis = totalmillis;
);

void disablezoom()
{
    zoom = 0;
    zoommillis = totalmillis;
}

void computezoom()
{
    if(!zoom) { curfov = fov; curavatarfov = avatarfov; return; }
    if(zoom < 0 && curfov >= fov) { zoom = 0; curfov = fov; curavatarfov = avatarfov; return; } // don't zoom-out if not zoomed-in
    int zoomvel = zoom > 0 ? zoominvel : zoomoutvel,
        oldfov = zoom > 0 ? fov : zoomfov,
        newfov = zoom > 0 ? zoomfov : fov,
        oldavatarfov = zoom > 0 ? avatarfov : avatarzoomfov,
        newavatarfov = zoom > 0 ? avatarzoomfov : avatarfov;
    float t = zoomvel ? float(zoomvel - (totalmillis - zoommillis)) / zoomvel : 0;
    if(t <= 0) 
    {
        if(!zoomvel && fabs(newfov - curfov) >= 1) 
        {
            curfov = newfov;
            curavatarfov = newavatarfov;
        }
        zoom = max(zoom, 0);
    }
    else 
    {
        curfov = oldfov*t + newfov*(1 - t);
        curavatarfov = oldavatarfov*t + newavatarfov*(1 - t);
    }
}

FVARP(zoomsens, 1e-3f, 1, 1000);
FVARP(zoomaccel, 0, 0, 1000);
VARP(zoomautosens, 0, 1, 1);
FVARP(sensitivity, 1e-3f, 3, 1000);
FVARP(sensitivityscale, 1e-3f, 1, 1000);
VARP(invmouse, 0, 0, 1);
FVARP(mouseaccel, 0, 0, 1000);
 
VAR(thirdperson, 0, 0, 2);
FVAR(thirdpersondistance, 0, 20, 1000);
physent *camera1 = NULL;
bool detachedcamera = false;
bool isthirdperson() { return player!=camera1 || detachedcamera; }

void fixcamerarange()
{
    const float MAXPITCH = 90.0f;
    if(camera1->pitch>MAXPITCH) camera1->pitch = MAXPITCH;
    if(camera1->pitch<-MAXPITCH) camera1->pitch = -MAXPITCH;
    while(camera1->yaw<0.0f) camera1->yaw += 360.0f;
    while(camera1->yaw>=360.0f) camera1->yaw -= 360.0f;
}

void mousemove(int dx, int dy)
{
    float cursens = sensitivity, curaccel = mouseaccel;
    if(zoom)
    {
        if(zoomautosens) 
        {
            cursens = float(sensitivity*zoomfov)/fov;
            curaccel = float(mouseaccel*zoomfov)/fov;
        }
        else 
        {
            cursens = zoomsens;
            curaccel = zoomaccel;
        }
    }
    if(curaccel && curtime && (dx || dy)) cursens += curaccel * sqrtf(dx*dx + dy*dy)/curtime;
    cursens /= 33.0f*sensitivityscale;
    camera1->yaw += dx*cursens;
    camera1->pitch -= dy*cursens*(invmouse ? -1 : 1);
    fixcamerarange();
    if(camera1!=player && !detachedcamera)
    {
        player->yaw = camera1->yaw;
        player->pitch = camera1->pitch;
    }
}

void recomputecamera()
{
    game::setupcamera();
    computezoom();

    bool shoulddetach = thirdperson > 1 || game::detachcamera();
    if(!thirdperson && !shoulddetach)
    {
        camera1 = player;
        detachedcamera = false;
    }
    else
    {
        static physent tempcamera;
        camera1 = &tempcamera;
        if(detachedcamera && shoulddetach) camera1->o = player->o;
        else
        {
            *camera1 = *player;
            detachedcamera = shoulddetach;
        }
        camera1->reset();
        camera1->type = ENT_CAMERA;
        camera1->collidetype = COLLIDE_AABB;
        camera1->move = -1;
        camera1->eyeheight = camera1->aboveeye = camera1->radius = camera1->xradius = camera1->yradius = 2;
        
        vec dir;
        vecfromyawpitch(camera1->yaw, camera1->pitch, -1, 0, dir);
        if(game::collidecamera()) 
        {
            movecamera(camera1, dir, thirdpersondistance, 1);
            movecamera(camera1, dir, clamp(thirdpersondistance - camera1->o.dist(player->o), 0.0f, 1.0f), 0.1f);
        }
        else camera1->o.add(vec(dir).mul(thirdpersondistance));
    }

    setviewcell(camera1->o);
}

extern const glmatrixf viewmatrix(vec4(-1, 0, 0, 0), vec4(0, 0, 1, 0), vec4(0, -1, 0, 0));
extern const glmatrixf invviewmatrix(vec4(-1, 0, 0, 0), vec4(0, 0, -1, 0), vec4(0, 1, 0, 0));
glmatrixf mvmatrix, projmatrix, mvpmatrix, invmvmatrix, invmvpmatrix, invprojmatrix, eyematrix, worldmatrix, linearworldmatrix;

void readmatrices()
{
    glGetFloatv(GL_MODELVIEW_MATRIX, mvmatrix.v);
    glGetFloatv(GL_PROJECTION_MATRIX, projmatrix.v);

    mvpmatrix.mul(projmatrix, mvmatrix);
    invmvmatrix.invert(mvmatrix);
    invmvpmatrix.invert(mvpmatrix);
    invprojmatrix.invert(projmatrix);
}

FVAR(nearplane, 1e-3f, 0.54f, 1e3f);

void project(float fovy, float aspect, int farplane, float zscale = 1)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(zscale!=1) glScalef(1, 1, zscale);
    GLdouble ydist = nearplane * tan(fovy/2*RAD), xdist = ydist * aspect;
    glFrustum(-xdist, xdist, -ydist, ydist, nearplane, farplane);
    glMatrixMode(GL_MODELVIEW);
}


vec calcavatarpos(const vec &pos, float dist)
{
    vec eyepos;
    mvmatrix.transform(pos, eyepos);
    GLdouble ydist = nearplane * tan(curavatarfov/2*RAD), xdist = ydist * aspect;
    vec4 scrpos;
    scrpos.x = eyepos.x*nearplane/xdist;
    scrpos.y = eyepos.y*nearplane/ydist;
    scrpos.z = (eyepos.z*(farplane + nearplane) - 2*nearplane*farplane) / (farplane - nearplane);
    scrpos.w = -eyepos.z;

    vec worldpos = invmvpmatrix.perspectivetransform(scrpos);
    vec dir = vec(worldpos).sub(camera1->o).rescale(dist);
    return dir.add(camera1->o);
}

VAR(reflectclip, 0, 6, 64);
VAR(reflectclipavatar, -64, 0, 64);

static const glmatrixf dummymatrix;
static int projectioncount = 0;
void pushprojection(const glmatrixf &m = dummymatrix)
{
    glMatrixMode(GL_PROJECTION);
    if(projectioncount <= 0) glPushMatrix();
    if(&m != &dummymatrix) glLoadMatrixf(m.v);
    glMatrixMode(GL_MODELVIEW);
    projectioncount++;
}

void popprojection()
{
    --projectioncount;
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    if(projectioncount > 0)
    {
        glPushMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

FVAR(polygonoffsetfactor, -1e4f, -3.0f, 1e4f);
FVAR(polygonoffsetunits, -1e4f, -3.0f, 1e4f);
FVAR(depthoffset, -1e4f, 0.01f, 1e4f);

void enablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glPolygonOffset(polygonoffsetfactor, polygonoffsetunits);
        glEnable(type);
        return;
    }
    
    glmatrixf offsetmatrix = projmatrix;
    offsetmatrix[14] += depthoffset * projmatrix[10];

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(offsetmatrix.v);
    glMatrixMode(GL_MODELVIEW);
}

void disablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glDisable(type);
        return;
    }
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

bool calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2)
{
    vec e(mvmatrix.transformx(center),
          mvmatrix.transformy(center),
          mvmatrix.transformz(center));
    if(e.z > 2*size) { sx1 = sy1 = 1; sx2 = sy2 = -1; return false; }
    float zzrr = e.z*e.z - size*size,
          dx = e.x*e.x + zzrr, dy = e.y*e.y + zzrr,
          focaldist = 1.0f/tan(fovy*0.5f*RAD);
    sx1 = sy1 = -1;
    sx2 = sy2 = 1;
    #define CHECKPLANE(c, dir, focaldist, low, high) \
    do { \
        float nzc = (cz*cz + 1) / (cz dir drt) - cz, \
              pz = (d##c)/(nzc*e.c - e.z); \
        if(pz > 0) \
        { \
            float c = (focaldist)*nzc, \
                  pc = pz*nzc; \
            if(pc < e.c) low = c; \
            else if(pc > e.c) high = c; \
        } \
    } while(0)
    if(dx > 0)
    {
        float cz = e.x/e.z, drt = sqrtf(dx)/size;
        CHECKPLANE(x, -, focaldist/aspect, sx1, sx2);
        CHECKPLANE(x, +, focaldist/aspect, sx1, sx2);
    }
    if(dy > 0)
    {
        float cz = e.y/e.z, drt = sqrtf(dy)/size;
        CHECKPLANE(y, -, focaldist, sy1, sy2);
        CHECKPLANE(y, +, focaldist, sy1, sy2);
    }
    return sx1 < sx2 && sy1 < sy2;
}

bool calcbbscissor(const ivec &bbmin, const ivec &bbmax, float &sx1, float &sy1, float &sx2, float &sy2)
{
    vec4 v[8];
    sx1 = sy1 = 1;
    sx2 = sy2 = -1;
    loopi(8)
    {
        vec4 &p = v[i];
        mvpmatrix.transform(vec(i&1 ? bbmax.x : bbmin.x, i&2 ? bbmax.y : bbmin.y, i&4 ? bbmax.z : bbmin.z), p);
        if(p.z >= -p.w)
        {
            float x = p.x / p.w, y = p.y / p.w;
            sx1 = min(sx1, x);
            sy1 = min(sy1, y);
            sx2 = max(sx2, x);
            sy2 = max(sy2, y);
        }
    }
    if(sx1 > sx2 || sy1 > sy2) return false;
    loopi(8)
    {
        const vec4 &p = v[i];
        if(p.z >= -p.w) continue;
        loopj(3)
        {
            const vec4 &o = v[i^(1<<j)];
            if(o.z <= -o.w) continue;
            float t = (p.z + p.w)/(p.z + p.w - o.z - o.w),
                  w = p.w + t*(o.w - p.w),
                  x = (p.x + t*(o.x - p.x))/w,
                  y = (p.y + t*(o.y - p.y))/w;
            sx1 = min(sx1, x);
            sy1 = min(sy1, y);
            sx2 = max(sx2, x);
            sy2 = max(sy2, y);
        }
    }
    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);
    return true;
}

void calcspheredepth(const vec &center, float size, float &sz1, float &sz2)
{
    float z = mvmatrix.transformz(center),
          z1 = min(z + size, -1e-3f - nearplane), z2 = min(z - size, -1e-3f - nearplane);
    sz1 = (z1*projmatrix.v[10] + projmatrix.v[14]) / (z1*projmatrix.v[11] + projmatrix.v[15]);
    sz2 = (z2*projmatrix.v[10] + projmatrix.v[14]) / (z2*projmatrix.v[11] + projmatrix.v[15]);
}

static int scissoring = 0;
static GLint oldscissor[4];

int pushscissor(float sx1, float sy1, float sx2, float sy2)
{
    scissoring = 0;

    if(sx1 <= -1 && sy1 <= -1 && sx2 >= 1 && sy2 >= 1) return 0;

    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int sx = viewport[0] + int(floor((sx1+1)*0.5f*viewport[2])),
        sy = viewport[1] + int(floor((sy1+1)*0.5f*viewport[3])),
        sw = viewport[0] + int(ceil((sx2+1)*0.5f*viewport[2])) - sx,
        sh = viewport[1] + int(ceil((sy2+1)*0.5f*viewport[3])) - sy;
    if(sw <= 0 || sh <= 0) return 0;

    if(glIsEnabled(GL_SCISSOR_TEST))
    {
        glGetIntegerv(GL_SCISSOR_BOX, oldscissor);
        sw += sx;
        sh += sy;
        sx = max(sx, int(oldscissor[0]));
        sy = max(sy, int(oldscissor[1]));
        sw = min(sw, int(oldscissor[0] + oldscissor[2])) - sx;
        sh = min(sh, int(oldscissor[1] + oldscissor[3])) - sy;
        if(sw <= 0 || sh <= 0) return 0;
        scissoring = 2;
    }
    else scissoring = 1;

    glScissor(sx, sy, sw, sh);
    if(scissoring<=1) glEnable(GL_SCISSOR_TEST);
    
    return scissoring;
}

void popscissor()
{
    if(scissoring>1) glScissor(oldscissor[0], oldscissor[1], oldscissor[2], oldscissor[3]);
    else if(scissoring) glDisable(GL_SCISSOR_TEST);
    scissoring = 0;
}

VARR(fog, 16, 4000, 1000024);
bvec fogcolor(0x80, 0x99, 0xB3);
HVARFR(fogcolour, 0, 0x8099B3, 0xFFFFFF,
{
    fogcolor = bvec((fogcolour>>16)&0xFF, (fogcolour>>8)&0xFF, fogcolour&0xFF);
});

static float findsurface(int fogmat, const vec &v, int &abovemat)
{
    ivec o(v), co;
    int csize;
    do
    {
        cube &c = lookupcube(o.x, o.y, o.z, 0, co, csize);
        int mat = c.material&MATF_VOLUME;
        if(mat != fogmat)
        {
            abovemat = isliquid(mat) ? mat : MAT_AIR;
            return o.z;
        }
        o.z = co.z + csize;
    }
    while(o.z < worldsize);
    abovemat = MAT_AIR;
    return worldsize;
}

static void blendfog(int fogmat, float below, float blend, float logblend, float &start, float &end, float *fogc)
{
    switch(fogmat)
    {
        case MAT_WATER:
        {
            float deepfade = clamp(below/max(waterdeep, waterfog), 0.0f, 1.0f);
            vec color;
            loopk(3) color[k] = watercolor[k]*(1-deepfade) + waterdeepcolor[k]*deepfade;
            loopk(3) fogc[k] += blend*color[k]/255.0f;
            end += logblend*min(fog, max(waterfog*4, 32));
            break;
        }

        case MAT_LAVA:
            loopk(3) fogc[k] += blend*lavacolor[k]/255.0f;
            end += logblend*min(fog, max(lavafog*4, 32));
            break;

        default:
            loopk(3) fogc[k] += blend*fogcolor[k]/255.0f;
            start += logblend*(fog+64)/8;
            end += logblend*fog;
            break;
    }
}

static void setfog(int fogmat, float below = 0, float blend = 1, int abovemat = MAT_AIR)
{
    float fogc[4] = { 0, 0, 0, 1 };
    float start = 0, end = 0;
    float logscale = 256, logblend = log(1 + (logscale - 1)*blend) / log(logscale);

    blendfog(fogmat, below, blend, logblend, start, end, fogc);
    if(blend < 1) blendfog(abovemat, 0, 1-blend, 1-logblend, start, end, fogc);

    loopk(3) fogc[k] *= ldrscale;

    glFogf(GL_FOG_START, start);
    glFogf(GL_FOG_END, end);
    glFogfv(GL_FOG_COLOR, fogc);
    //glClearColor(fogc[0], fogc[1], fogc[2], 1.0f);
}

static void blendfogoverlay(int fogmat, float below, float blend, float *overlay)
{
    float maxc;
    switch(fogmat)
    {
        case MAT_WATER:
        {
            float deepfade = clamp(below/max(waterdeep, waterfog), 0.0f, 1.0f);
            vec color;
            loopk(3) color[k] = watercolor[k]*(1-deepfade) + waterdeepcolor[k]*deepfade;
            maxc = max(color[0], max(color[1], color[2]));
            loopk(3) overlay[k] += blend*max(0.4f, color[k]/min(32.0f + maxc*7.0f/8.0f, 255.0f));
            break;
        }

        case MAT_LAVA:
            maxc = max(lavacolor[0], max(lavacolor[1], lavacolor[2]));
            loopk(3) overlay[k] += blend*max(0.4f, lavacolor[k]/min(32.0f + maxc*7.0f/8.0f, 255.0f));
            break;

        default:
            loopk(3) overlay[k] += blend;
            break;
    }
}

void drawfogoverlay(int fogmat, float fogbelow, float fogblend, int abovemat)
{
    notextureshader->set();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    float overlay[3] = { 0, 0, 0 };
    blendfogoverlay(fogmat, fogbelow, fogblend, overlay);
    blendfogoverlay(abovemat, 0, 1-fogblend, overlay);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3fv(overlay);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(-1, 1);
    glVertex2f(1, 1);
    glEnd();
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    defaultshader->set();
}

bool envmapping = false;
int minimapping = 0;

GLuint minimaptex = 0;
vec minimapcenter(0, 0, 0), minimapradius(0, 0, 0), minimapscale(0, 0, 0);

void clearminimap()
{
    if(minimaptex) { glDeleteTextures(1, &minimaptex); minimaptex = 0; }
}

VARR(minimapheight, 0, 0, 2<<16);
bvec minimapcolor(0, 0, 0);
HVARFR(minimapcolour, 0, 0, 0xFFFFFF,
{
    minimapcolor = bvec((minimapcolour>>16)&0xFF, (minimapcolour>>8)&0xFF, minimapcolour&0xFF);
});
VARR(minimapclip, 0, 0, 1);
VARFP(minimapsize, 7, 8, 10, { if(minimaptex) drawminimap(); });

void bindminimap()
{
    glBindTexture(GL_TEXTURE_2D, minimaptex);
}

void clipminimap(ivec &bbmin, ivec &bbmax, cube *c = worldroot, int x = 0, int y = 0, int z = 0, int size = worldsize>>1)
{
    loopi(8)
    {
        ivec o(i, x, y, z, size);
        if(c[i].children) clipminimap(bbmin, bbmax, c[i].children, o.x, o.y, o.z, size>>1);
        else if(!isentirelysolid(c[i]) && (c[i].material&MATF_CLIP)!=MAT_CLIP)
        {
            loopk(3) bbmin[k] = min(bbmin[k], o[k]);
            loopk(3) bbmax[k] = max(bbmax[k], o[k] + size);
        }
    }
}

GLuint motiontex = 0;
int motionw = 0, motionh = 0, lastmotion = 0;

void cleanupmotionblur()
{
    if(motiontex) { glDeleteTextures(1, &motiontex); motiontex = 0; }
    motionw = motionh = 0;
    lastmotion = 0;
}

VARFP(motionblur, 0, 0, 1, { if(!motionblur) cleanupmotionblur(); });
VARP(motionblurmillis, 1, 5, 1000);
FVARP(motionblurscale, 0, 0.5f, 1);

void addmotionblur()
{
    if(!motionblur || !hasTR || max(screen->w, screen->h) > hwtexsize) return;

    if(paused || game::ispaused()) { lastmotion = 0; return; }

    if(!motiontex || motionw != screen->w || motionh != screen->h)
    {
        if(!motiontex) glGenTextures(1, &motiontex);
        motionw = screen->w;
        motionh = screen->h;
        lastmotion = 0;
        createtexture(motiontex, motionw, motionh, NULL, 3, 0, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    }

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, motiontex);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    rectshader->set();

    glColor4f(1, 1, 1, lastmotion ? pow(motionblurscale, max(float(lastmillis - lastmotion)/motionblurmillis, 1.0f)) : 0);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(      0,       0); glVertex2f(-1, -1);
    glTexCoord2f(motionw,       0); glVertex2f( 1, -1);
    glTexCoord2f(      0, motionh); glVertex2f(-1,  1);
    glTexCoord2f(motionw, motionh); glVertex2f( 1,  1);
    glEnd();

    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glEnable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
 
    if(lastmillis - lastmotion >= motionblurmillis)
    {
        lastmotion = lastmillis - lastmillis%motionblurmillis;

        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, screen->w, screen->h);
    }
}

bool dopostfx = false;

void invalidatepostfx()
{
    dopostfx = false;
}

void gl_drawhud(int w, int h);

int xtraverts, xtravertsva;

void screenquad(float sw, float sh)
{
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(sw, 0); glVertex2f(1, -1);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(sw, sh); glVertex2f(1, 1);
    glTexCoord2f(0, sh); glVertex2f(-1, 1);
    glEnd();
}

void screenquad(float sw, float sh, float sw2, float sh2)
{
    glBegin(GL_TRIANGLE_STRIP);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, sw, 0); glMultiTexCoord2f_(GL_TEXTURE1_ARB, sw2, 0); glVertex2f(1, -1);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, 0, 0); glMultiTexCoord2f_(GL_TEXTURE1_ARB, 0, 0); glVertex2f(-1, -1);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, sw, sh); glMultiTexCoord2f_(GL_TEXTURE1_ARB, sw2, sh2); glVertex2f(1, 1);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, 0, sh); glMultiTexCoord2f_(GL_TEXTURE1_ARB, 0, sh2); glVertex2f(-1, 1);
    glEnd();
}

int gw = -1, gh = -1, bloomw = -1, bloomh = -1, lasthdraccum = 0;
GLuint gfbo = 0, gdepthtex = 0, gcolortex = 0, gnormaltex = 0, gglowtex = 0, gdepthrb = 0, gstencilrb = 0;
GLuint smaaareatex = 0, smaasearchtex = 0, smaafbo[3] = { 0, 0, 0 };
GLuint hdrfbo = 0, hdrtex = 0, bloomfbo[5] = { 0, 0, 0, 0, 0 }, bloomtex[5] = { 0, 0, 0, 0, 0 };
int hdrclear = 0;
GLuint refractfbo = 0, refracttex = 0;
GLenum bloomformat = 0, hdrformat = 0;
bool hdrfloat = false;

int aow = -1, aoh = -1;
GLuint aofbo[3] = { 0, 0, 0 }, aotex[3] = { 0, 0, 0 }, aonoisetex = 0;

extern int bloomsize, bloomprec;

void setupbloom(int w, int h)
{
    int maxsize = ((1<<bloomsize)*5)/4;
    while(w >= maxsize) w /= 2;
    while(h >= maxsize) h /= 2; 
    if(w == bloomw && h == bloomh) return;

    bloomw = w;
    bloomh = h;

    loopi(5) if(!bloomtex[i]) glGenTextures(1, &bloomtex[i]);

    loopi(5) if(!bloomfbo[i]) glGenFramebuffers_(1, &bloomfbo[i]);

    bloomformat = bloomprec >= 3 && hasTF ? GL_RGB16F_ARB : (bloomprec >= 2 && hasPF ? GL_R11F_G11F_B10F_EXT : (bloomprec >= 1 ? GL_RGB10 : GL_RGB));
    createtexture(bloomtex[0], max(gw/2, bloomw), max(gh/2, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[1], max(gw/4, bloomw), max(gh/4, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[2], bloomw, bloomh, NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[3], bloomw, bloomh, NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);

    static uchar gray[3] = { 32, 32, 32 };
    static float grayf[3] = { 0.125f, 0.125f, 0.125f };
    createtexture(bloomtex[4], 1, 1, hasTF ? (void *)grayf : (void *)gray, 3, 1, hasTF ? GL_RGB16F_ARB : GL_RGB16, GL_TEXTURE_RECTANGLE_ARB);

    loopi(5)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, bloomfbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, bloomtex[i], 0);

        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating bloom buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void cleanupbloom()
{
    loopi(5) if(bloomfbo[i]) { glDeleteFramebuffers_(1, &bloomfbo[i]); bloomfbo[i] = 0; }
    loopi(5) if(bloomtex[i]) { glDeleteTextures(1, &bloomtex[i]); bloomtex[i] = 0; }
    bloomw = bloomh = -1;
    lasthdraccum = 0;
}

extern int ao, aotaps, aoreduce, aoreducedepth, aonoise, aobilateral, aopackdepth;

static Shader *bilateralshader[2] = { NULL, NULL };

Shader *loadbilateralshader(int pass)
{
    if(!aobilateral) return nullshader;

    string opts;
    int optslen = 0;

    if(aoreduce) opts[optslen++] = 'r';
    bool linear = hasTRG && hasTF && (aopackdepth || (aoreducedepth && (aoreduce || aoreducedepth > 1)));
    if(linear)
    {
        opts[optslen++] = 'l';
        if(aopackdepth) opts[optslen++] = 'p';
    }
    opts[optslen] = '\0';

    defformatstring(name)("bilateral%c%s%d", 'x' + pass, opts, aobilateral);
    return generateshader(name, "bilateralshader \"%s\" %d", opts, aobilateral);
}

void loadbilateralshaders()
{
    loopk(2) bilateralshader[k] = loadbilateralshader(k);
}

void clearbilateralshaders()
{
    loopk(2) bilateralshader[k] = NULL;
}

static Shader *ambientobscuranceshader = NULL;

Shader *loadambientobscuranceshader()
{
    string opts;
    int optslen = 0;

    if(aoreduce) opts[optslen++] = 'r';
    bool linear = hasTRG && hasTF && (aoreducedepth && (aoreduce || aoreducedepth > 1));
    if(linear) opts[optslen++] = 'l';
    opts[optslen] = '\0';

    defformatstring(name)("ambientobscurance%s%d", opts, aotaps);
    return generateshader(name, "ambientobscuranceshader \"%s\" %d", opts, aotaps);
}

void loadaoshaders()
{
    ambientobscuranceshader = loadambientobscuranceshader();
}

void clearaoshaders()
{
    ambientobscuranceshader = NULL;
}

void setupao(int w, int h)
{
    w >>= aoreduce;
    h >>= aoreduce;

    if(w == aow && h == aoh) return;

    aow = w;
    aoh = h;

    if(!aonoisetex) glGenTextures(1, &aonoisetex);
    bvec *noise = new bvec[(1<<aonoise)*(1<<aonoise)];
    loopk((1<<aonoise)*(1<<aonoise)) noise[k] = bvec(vec(rndscale(2)-1, rndscale(2)-1, 0).normalize());
    createtexture(aonoisetex, 1<<aonoise, 1<<aonoise, noise, 0, 0, GL_RGB, GL_TEXTURE_2D);
    delete[] noise;

    loopi(2)
    {
        if(!aotex[i]) glGenTextures(1, &aotex[i]);
        if(!aofbo[i]) glGenFramebuffers_(1, &aofbo[i]);
        createtexture(aotex[i], w, h, NULL, 3, 1, aobilateral && aopackdepth && hasTRG && hasTF ? GL_RG16F : GL_RGB , GL_TEXTURE_RECTANGLE_ARB);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, aotex[i], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating AO buffer!");
    }

    if(hasTRG && hasTF && aoreducedepth && (aoreduce || aoreducedepth > 1))
    {
        if(!aotex[2]) glGenTextures(1, &aotex[2]);
        if(!aofbo[2]) glGenFramebuffers_(1, &aofbo[2]);
        createtexture(aotex[2], w, h, NULL, 3, 0, GL_R16F, GL_TEXTURE_RECTANGLE_ARB);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[2]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, aotex[2], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating AO buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadaoshaders();
    loadbilateralshaders();
}

void cleanupao()
{
    loopi(3) if(aofbo[i]) { glDeleteFramebuffers_(1, &aofbo[i]); aofbo[i] = 0; }
    loopi(3) if(aotex[i]) { glDeleteTextures(1, &aotex[i]); aotex[i] = 0; }
    if(aonoisetex) { glDeleteTextures(1, &aonoisetex); aonoisetex = 0; }
    aow = bloomh = -1;
    
    clearaoshaders();
    clearbilateralshaders();
}

void viewao()
{
    if(!ao) return;
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[0]);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, aoh); glVertex2i(0, 0);
    glTexCoord2f(aow, aoh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(aow, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

#include "AreaTex.h"
#include "SearchTex.h"

extern int smaaquality, hdrprec, gdepthformat, gstencil, gdepthstencil, glineardepth;

static Shader *smaalumaedgeshader = NULL, *smaacoloredgeshader = NULL, *smaablendweightshader = NULL, *smaaneighborhoodshader = NULL;

void loadsmaashaders()
{
    defformatstring(lumaedgename)("SMAALumaEdgeDetection%d", smaaquality);
    defformatstring(coloredgename)("SMAAColorEdgeDetection%d", smaaquality);
    defformatstring(blendweightname)("SMAABlendingWeightCalculation%d", smaaquality);
    defformatstring(neighborhoodname)("SMAANeighborhoodBlending%d", smaaquality);
    smaalumaedgeshader = lookupshaderbyname(lumaedgename);
    smaacoloredgeshader = lookupshaderbyname(coloredgename);
    smaablendweightshader = lookupshaderbyname(blendweightname);
    smaaneighborhoodshader = lookupshaderbyname(neighborhoodname);
    if(smaalumaedgeshader && smaacoloredgeshader && smaablendweightshader && smaaneighborhoodshader) return;
    generateshader(NULL, "smaashaders %d", smaaquality);
    smaalumaedgeshader = lookupshaderbyname(lumaedgename);
    if(!smaalumaedgeshader) smaalumaedgeshader = nullshader;
    smaacoloredgeshader = lookupshaderbyname(coloredgename);
    if(!smaacoloredgeshader) smaacoloredgeshader = nullshader;
    smaablendweightshader = lookupshaderbyname(blendweightname);
    if(!smaablendweightshader) smaablendweightshader = nullshader;
    smaaneighborhoodshader = lookupshaderbyname(neighborhoodname);
    if(!smaaneighborhoodshader) smaaneighborhoodshader = nullshader;
}

void clearsmaashaders()
{
    smaalumaedgeshader = NULL;
    smaacoloredgeshader = NULL;
    smaablendweightshader = NULL;
    smaaneighborhoodshader = NULL;
}

void setupsmaa()
{
    if(!smaaareatex) glGenTextures(1, &smaaareatex);
    if(!smaasearchtex) glGenTextures(1, &smaasearchtex);
    createtexture(smaaareatex, AREATEX_WIDTH, AREATEX_HEIGHT, areaTexBytes, 3, 1, GL_LUMINANCE_ALPHA, GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, false);
    createtexture(smaasearchtex, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, searchTexBytes, 3, 0, GL_LUMINANCE, GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, false);
    loopi(3) 
    {
        if(!smaafbo[i]) glGenFramebuffers_(1, &smaafbo[i]);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[i]);
        GLuint tex = 0;
        switch(i)
        {
            case 0: tex = gcolortex; break;
            case 1: tex = gnormaltex; break;
            case 2: tex = gglowtex; break;
        }
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, tex, 0);
        if(i > 0)
        {
            if(gdepthformat)
            {
                glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
                if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
            }
            else
            {
                glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
                if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
                else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
            }
        }
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating SMAA buffer!");
    }
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadsmaashaders();
}

void cleanupsmaa()
{
    if(smaaareatex) { glDeleteTextures(1, &smaaareatex); smaaareatex = 0; }
    if(smaasearchtex) { glDeleteTextures(1, &smaasearchtex); smaasearchtex = 0; }
    loopi(3) if(smaafbo[i]) { glDeleteFramebuffers_(1, &smaafbo[i]); smaafbo[i] = 0; }

    clearsmaashaders();
}

VARFP(smaa, 0, 0, 1, cleanupsmaa());
VARFP(smaaquality, 0, 2, 3, cleanupsmaa());
VARP(smaacoloredge, 0, 0, 1);
VAR(smaadepthmask, 0, 1, 1);
VAR(smaastencil, 0, 1, 1);
VAR(debugsmaa, 0, 0, 5);

void viewsmaa()
{
    int w = min(screen->w, screen->h)*1.0f, h = (w*screen->h)/screen->w, tw = gw, th = gh;
    rectshader->set();
    glColor3f(1, 1, 1);
    switch(debugsmaa)
    {
        case 1: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex); break;
        case 2: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex); break;
        case 3: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex); break;
        case 4: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaaareatex); tw = AREATEX_WIDTH; th = AREATEX_HEIGHT; break;
        case 5: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaasearchtex); tw = SEARCHTEX_WIDTH; th = SEARCHTEX_HEIGHT; break;
        
    }
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, th); glVertex2i(0, 0);
    glTexCoord2f(tw, th); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(tw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VAR(gdepthformat, 1, 0, 0);

void maskgbuffer(const char *mask)
{
    GLenum drawbufs[4];
    int numbufs = 0;
    while(*mask) switch(*mask++)
    {
        case 'c': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT0_EXT; break;
        case 'n': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT1_EXT; break;
        case 'g': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT2_EXT; break;
        case 'd': if(gdepthformat) drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT3_EXT; break;
    }
    glDrawBuffers_(numbufs, drawbufs);
}

void initgbuffer()
{
    if(glineardepth >= 2 && (!hasAFBO || !hasTF || !hasTRG)) gdepthformat = 1;
    else gdepthformat = glineardepth;
}

void setupgbuffer(int w, int h)
{
    if(gw == w && gh == h) return;

    gw = w;
    gh = h;

    if(!gdepthtex) glGenTextures(1, &gdepthtex);
    if(!gcolortex) glGenTextures(1, &gcolortex);
    if(!gnormaltex) glGenTextures(1, &gnormaltex);
    if(!gglowtex) glGenTextures(1, &gglowtex);
    if(!gfbo) glGenFramebuffers_(1, &gfbo);
    
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);

    maskgbuffer("cngd");

    static const GLenum depthformats[] = { GL_RGBA8, GL_R16F, GL_R32F };
    GLenum depthformat = gdepthformat ? depthformats[gdepthformat-1] : (gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT);
    createtexture(gdepthtex, gw, gh, NULL, 3, 0, depthformat, GL_TEXTURE_RECTANGLE_ARB);

    createtexture(gcolortex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gnormaltex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gglowtex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
 
    if(gdepthformat)
    {
        if(!gdepthrb) glGenRenderbuffers_(1, &gdepthrb);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, gdepthrb);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
    } 
    else if(gstencil && (!gdepthstencil || !hasDS))
    {
        if(!gstencilrb) glGenRenderbuffers_(1, &gstencilrb);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, gstencilrb);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX8_EXT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
    }
 
    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, gcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, gnormaltex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_RECTANGLE_ARB, gglowtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) 
        fatal("failed allocating g-buffer!");

    if(!hdrtex) glGenTextures(1, &hdrtex);

    if(!hdrfbo) glGenFramebuffers_(1, &hdrfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);

    hdrformat = hdr ? (hdrprec >= 3 && hasTF ? GL_RGB16F_ARB : (hdrprec >= 2 && hasPF ? GL_R11F_G11F_B10F_EXT : (hdrprec >= 1 ? GL_RGB10 : GL_RGB))) : GL_RGB;
    hdrfloat = floatformat(hdrformat);
    createtexture(hdrtex, gw, gh, NULL, 3, 1, hdrformat, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, hdrtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating HDR buffer!");

    hdrclear = 3;

    if(!refracttex) glGenTextures(1, &refracttex);
    if(!refractfbo) glGenFramebuffers_(1, &refractfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, refractfbo);

    createtexture(refracttex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, refracttex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating refraction buffer!");
 
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    cleanupbloom();
    cleanupao();
    cleanupsmaa();
}

void cleanupgbuffer()
{
    if(gfbo) { glDeleteFramebuffers_(1, &gfbo); gfbo = 0; }
    if(gdepthtex) { glDeleteTextures(1, &gdepthtex); gdepthtex = 0; }
    if(gcolortex) { glDeleteTextures(1, &gcolortex); gcolortex = 0; }
    if(gnormaltex) { glDeleteTextures(1, &gnormaltex); gnormaltex = 0; }
    if(gglowtex) { glDeleteTextures(1, &gglowtex); gglowtex = 0; }
    if(gstencilrb) { glDeleteRenderbuffers_(1, &gstencilrb); gstencilrb = 0; }
    if(gdepthrb) { glDeleteRenderbuffers_(1, &gdepthrb); gdepthrb = 0; }
    if(hdrfbo) { glDeleteFramebuffers_(1, &hdrfbo); hdrfbo = 0; }
    if(hdrtex) { glDeleteTextures(1, &hdrtex); hdrtex = 0; }
    if(refractfbo) { glDeleteFramebuffers_(1, &refractfbo); refractfbo = 0; }
    if(refracttex) { glDeleteTextures(1, &refracttex); refracttex = 0; }
    gw = gh = -1;
}

VAR(debugdepth, 0, 0, 1);

void viewdepth()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, gh); glVertex2i(0, 0);
    glTexCoord2f(gw, gh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(gw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VAR(debugrefract, 0, 0, 1);

void viewrefract()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, refracttex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, gh); glVertex2i(0, 0);
    glTexCoord2f(gw, gh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(gw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VARF(gstencil, 0, 0, 1, cleanupgbuffer());
VARF(gdepthstencil, 0, 1, 1, cleanupgbuffer());
VARF(glineardepth, 0, 0, 3, initwarning("g-buffer setup"))
VARFP(hdr, 0, 1, 1, cleanupgbuffer());
VARFP(hdrprec, 0, 2, 3, cleanupgbuffer());
FVAR(bloomthreshold, 0, 1.5f, 10.0f);
FVARP(bloomscale, 0, 1.0f, 10.0f);
VARP(bloomblur, 0, 7, 7);
VARP(bloomiter, 0, 0, 4);
FVARP(bloomsigma, 0.005f, 0.5f, 2.0f);
VARFP(bloomsize, 6, 8, 10, cleanupbloom());
VARFP(bloomprec, 0, 2, 3, cleanupbloom());
FVAR(hdraccumscale, 0, 0.98f, 1);
VAR(hdraccummillis, 1, 33, 1000);
VAR(hdrreduce, 0, 2, 2);

FVARR(hdrbright, 1e-3f, 0.18f, 1e3f);
FVAR(hdrtonemin, 1e-3f, 0.75f, 1e3f);
FVAR(hdrtonemax, 1e-3f, 2.0f, 1e3f);
FVARR(bloombright, 1e-3f, 1.5f, 1e3f);
FVAR(bloomlummin, 1e-3f, 0.125f, 1e3f);
FVAR(bloomlummax, 1e-3f, 8.0f, 1e3f);

float ldrscale = 1.0f, ldrscaleb = 1.0f/255;

#define SHADOWATLAS_SIZE 4096

PackNode shadowatlaspacker(0, 0, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE),
         shadowcachepacker(0, 0, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE);

enum { L_NOSHADOW = 1<<0, L_NODYNSHADOW = 1<<1 };

struct lightinfo
{
    float sx1, sy1, sx2, sy2, sz1, sz2;
    int shadowmap, flags;
    vec o, color;
    int radius;
    float dist;
    occludequery *query;
};

struct shadowcachekey
{
    vec o;
    float radius;

    shadowcachekey() {}
    shadowcachekey(const lightinfo &l) : o(l.o), radius(l.radius) {}
};

static inline uint hthash(const shadowcachekey &k)
{
    return hthash(k.o);
}

static inline bool htcmp(const shadowcachekey &x, const shadowcachekey &y)
{
    return x.o == y.o && x.radius == y.radius;
}

struct shadowcacheval;

struct shadowmapinfo
{
    ushort x, y, size, sidemask;
    int light;
    shadowcacheval *cached;
};

struct shadowcacheval
{
    ushort x, y, size, sidemask;

    shadowcacheval() {}
    shadowcacheval(const shadowmapinfo &sm) : x(sm.x), y(sm.y), size(sm.size), sidemask(sm.sidemask) {}
};

struct shadowcache : hashtable<shadowcachekey, shadowcacheval>
{
    shadowcache() : hashtable<shadowcachekey, shadowcacheval>(256) {}

    void reset()
    {
        clear();
    } 
};

extern int smcache;

#define SHADOWCACHE_EVICT 2

GLuint shadowatlastex = 0, shadowatlasfbo = 0;
shadowcache shadowcache[2];
bool shadowcachefull = false;
int evictshadowcache = 0;

void setupshadowatlas()
{
    if(!shadowatlastex) glGenTextures(1, &shadowatlastex);

    createtexture(shadowatlastex, smcache ? 2*SHADOWATLAS_SIZE : SHADOWATLAS_SIZE, SHADOWATLAS_SIZE, NULL, 3, 1, GL_DEPTH_COMPONENT16, GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);

    if(!shadowatlasfbo) glGenFramebuffers_(1, &shadowatlasfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, shadowatlastex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating shadow atlas!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void cleanupshadowatlas()
{
    if(shadowatlastex) { glDeleteTextures(1, &shadowatlastex); shadowatlastex = 0; }
    if(shadowatlasfbo) { glDeleteFramebuffers_(1, &shadowatlasfbo); shadowatlasfbo = 0; }
    clearshadowcache();
}

const float cubeshadowviewmatrix[6][16] =
{
    // sign-preserving cubemap projections
    { // +X
         0, 0,-1, 0,
         0, 1, 0, 0,
         1, 0, 0, 0,
         0, 0, 0, 1,
    },
    { // -X
         0, 0, 1, 0,
         0, 1, 0, 0,
         1, 0, 0, 0,
         0, 0, 0, 1,
    },
    { // +Y
         1, 0, 0, 0,
         0, 0,-1, 0,
         0, 1, 0, 0,
         0, 0, 0, 1,
    },
    { // -Y
         1, 0, 0, 0,
         0, 0, 1, 0,
         0, 1, 0, 0,
         0, 0, 0, 1,
    },
    { // +Z
         1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0,-1, 0,
         0, 0, 0, 1,
    },
    { // -Z
         1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1,
    },
};

const float tetrashadowviewmatrix[4][16] =
{
    { // +1, +1, +1
        1, 0,-1, 0,
        0, 1,-1, 0,
        0, 0,-1, 0,
        0, 0, 0, 1
    },
    { // -1, -1, +1
        1, 0, 1, 0,
        0, 1, 1, 0,
        0, 0,-1, 0,
        0, 0, 0, 1
    },
    { // +1, -1, -1
        1, 0,-1, 0,
        0, 1, 1, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    },
    { // -1, +1, -1
        1, 0, 1, 0,
        0, 1,-1, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }
};

FVAR(smpolyfactor, -1e3f, 1, 1e3f);
FVAR(smpolyoffset, -1e3f, 0, 1e3f);
FVAR(smbias, -1e6f, 0.01f, 1e6f);
FVAR(smprec, 1e-3f, 1, 1e3f);
FVAR(smtetraprec, 1e-3f, SQRT3, 1e3f);
FVAR(smcubeprec, 1e-3f, 1, 1e3f);

VAR(smsidecull, 0, 1, 1);
VAR(smviscull, 0, 1, 1);
VAR(smborder, 0, 2, 16);
VAR(smminradius, 0, 16, 10000);
VAR(smminsize, 1, 96, 1024);
VAR(smmaxsize, 1, 384, 1024);
//VAR(smmaxsize, 1, 4096, 4096);
VAR(smused, 1, 0, 0);
VAR(smquery, 0, 1, 1);
VARFP(smtetra, 0, 0, 1, { cleardeferredlightshaders(); clearshadowcache(); });
VAR(smtetraclip, 0, 1, 1);
VAR(smtetraclear, 0, 1, 1);
FVAR(smtetraborder, 0, 0, 1e3f);
VAR(smcullside, 0, 1, 1);
VARF(smcache, 0, 1, 2, cleanupshadowatlas());
VARFP(smgather, 0, 0, 1, cleardeferredlightshaders());
VAR(smnoshadow, 0, 0, 2);
VAR(lighttilesused, 1, 0, 0);

int shadowmapping = 0;

plane smtetraclipplane;

static inline void setsmnoncomparemode() // use texture gather
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static inline void setsmcomparemode() // use embedded shadow cmp
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static inline bool usegatherforsm() { return smgather && (hasTG || hasT4); }

void viewshadowatlas()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    defaultshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, shadowatlastex);
    if(!usegatherforsm()) setsmnoncomparemode(); // "normal" mode
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord2f(1, 0); glVertex2i(screen->w, screen->h-h);
    glTexCoord2f(0, 1); glVertex2i(screen->w-w, screen->h);
    glTexCoord2f(1, 1); glVertex2i(screen->w, screen->h);
    glEnd();
    if(!usegatherforsm()) setsmcomparemode(); // "gather" mode basically
    notextureshader->set();
}
VAR(debugshadowatlas, 0, 0, 1);

vector<lightinfo> lights;
vector<int> lightorder;
vector<int> lighttiles[LIGHTTILE_H][LIGHTTILE_W];
vector<shadowmapinfo> shadowmaps;

void clearshadowcache()
{
    shadowmaps.setsize(0);
}

static shadowmapinfo *addshadowmap(ushort x, ushort y, int size, int &idx)
{
    idx = shadowmaps.length();
    shadowmapinfo *sm = &shadowmaps.add();
    sm->x = x;
    sm->y = y;
    sm->size = size;
    sm->light = -1;
    sm->sidemask = 0;
    sm->cached = NULL;
    return sm;
}

static const uint csmmaxsplitn = 8, csmminsplitn = 1;
VAR(csmmaxsize, 256, 768, 2048);
VARF(csmsplitn, csmminsplitn, 3, csmmaxsplitn, cleardeferredlightshaders());
FVAR(csmsplitweight, 0.20f, 0.75f, 0.95f);
VARF(csmshadowmap, 0, 1, 1, cleardeferredlightshaders());

// cascaded shadow maps
struct cascaded_shadow_map
{
    struct splitinfo
    {
        float nearplane;  // split distance to near plane
        float farplane;   // split distance to farplane
        glmatrixf proj;   // one projection per split
        int idx;          // shadowmapinfo indices
        vec bbmin, bbmax; // max extents of shadowmap in sunlight model space
        plane cull[4];    // world space culling planes of the split's projected sides
    };
    glmatrixf model;                // model view is shared by all splits
    splitinfo splits[csmmaxsplitn]; // per-split parameters
    vec lightview;                  // view vector for light
    void setup();                   // insert shadowmaps for each split frustum if there is sunlight
    void updatesplitdist();         // compute split frustum distances
    void getmodelmatrix();          // compute the shared model matrix
    void getprojmatrix();           // compute each cropped projection matrix
    void gencullplanes();           // generate culling planes for the mvp matrix
    void bindparams();              // bind any shader params necessary for lighting
};

void cascaded_shadow_map::setup()
{
    loopi(csmsplitn)
    {
        ushort smx = USHRT_MAX, smy = USHRT_MAX;
        splits[i].idx = -1;
        if(shadowatlaspacker.insert(smx, smy, csmmaxsize, csmmaxsize))
            addshadowmap(smx, smy, csmmaxsize, splits[i].idx);
    }
    getmodelmatrix();
    getprojmatrix();
    gencullplanes();
}

static float calcfrustumboundsphere(float nearplane, float farplane,  const vec &pos, const vec &view, vec &center)
{
    if(minimapping)
    {
        center = minimapcenter;
        return minimapradius.magnitude();
    }

    float height = tan(fovy/2.0f*RAD), width = height * aspect,
          cdist = ((nearplane + farplane)/2)*(1 + width*width + height*height);
    if(cdist <= farplane)
    {
        center = vec(view).mul(cdist).add(pos);
        return vec(width*nearplane, height*nearplane, cdist-nearplane).magnitude();
    }
    else
    {
        center = vec(view).mul(farplane).add(pos);
        return vec(width*farplane, height*farplane, 0).magnitude();
    }
}

VAR(csmfarplane, 64, 1024, 16384);
FVAR(csmpradiustweak, 1e-3f, 1, 1e3f);
FVAR(csmdepthmargin, 0, 0.1f, 1e3f);
VAR(debugcsm, 0, 0, csmmaxsplitn);
FVAR(csmpolyfactor, -1e3f, 2, 1e3f);
FVAR(csmpolyoffset, -1e4f, 0, 1e4f);
FVAR(csmbias, -1e6f, 1e-4f, 1e6f);
VAR(csmcull, 0, 1, 1);

void cascaded_shadow_map::updatesplitdist()
{
    float lambda = csmsplitweight, nd = nearplane, fd = csmfarplane, ratio = fd/nd;
    splits[0].nearplane = nd;
    for(int i = 1; i < csmsplitn; ++i)
    {
        float si = i / float(csmsplitn);
        splits[i].nearplane = lambda*(nd*pow(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
        splits[i-1].farplane = splits[i].nearplane * 1.005f;
    }
    splits[csmsplitn-1].farplane = fd;
}

void cascaded_shadow_map::getmodelmatrix()
{
    model = viewmatrix;
    model.rotate_around_x(sunlightpitch*RAD);
    model.rotate_around_z((180-sunlightyaw)*RAD);
}

void cascaded_shadow_map::getprojmatrix()
{
    vec lightup, lightright;
    lightview = vec(sunlightdir).neg();
    lightup.orthogonal(lightview);
    lightup.normalize();
    lightright.cross(lightview, lightup);
    lightup.cross(lightright, lightview);

    // compute the split frustums
    updatesplitdist();

    // find z extent
    float minz = lightview.project_bb(worldmin, worldmax), maxz = lightview.project_bb(worldmax, worldmin), zmargin = (maxz - minz)*csmdepthmargin;
    minz -= zmargin;
    maxz += zmargin;

    // compute each split projection matrix
    loopi(csmsplitn)
    {
        splitinfo &split = this->splits[i];
        if(split.idx < 0) continue;

        vec c;
        float radius = calcfrustumboundsphere(split.nearplane, split.farplane, camera1->o, camdir, c);

        // compute the projected bounding box of the sphere
        const vec p = c + radius * lightright;
        vec tp, tc;
        this->model.transform(p, tp);
        this->model.transform(c, tc);
        const float pradius = tp.dist2(tc) * csmpradiustweak, step = (2.0f*pradius) / shadowmaps[split.idx].size;
        vec2 minv = vec2(tc).sub(pradius), maxv = vec2(tc).add(pradius);
        minv.x -= fmod(minv.x, step);
        minv.y -= fmod(minv.y, step);
        maxv.x -= fmod(maxv.x, step);
        maxv.y -= fmod(maxv.y, step);
        split.bbmin = vec(minv, -maxz);
        split.bbmax = vec(maxv, -minz);

        // modify mvp with a scale and offset
        // now compute the update model view matrix for this split
        split.proj.ortho(minv.x, maxv.x, minv.y, maxv.y, minz, maxz);
    }
}

void cascaded_shadow_map::gencullplanes()
{
    loopi(csmsplitn)
    {
        splitinfo &split = splits[i];
        glmatrixf mvp;
        mvp.mul(split.proj, model);
        vec4 px = mvp.getrow(0), py = mvp.getrow(1), pw = mvp.getrow(3);
        split.cull[0] = plane(vec4(pw).add(px)).normalize(); // left plane
        split.cull[1] = plane(vec4(pw).sub(px)).normalize(); // right plane
        split.cull[2] = plane(vec4(pw).add(py)).normalize(); // bottom plane
        split.cull[3] = plane(vec4(pw).sub(py)).normalize(); // top plane
    }        
}

cascaded_shadow_map csm;

int calcbbcsmsplits(const ivec &bbmin, const ivec &bbmax)
{
    int mask = (1<<csmsplitn)-1;
    if(!csmcull) return mask;
    loopi(csmsplitn)
    {
        const cascaded_shadow_map::splitinfo &split = csm.splits[i];
        int k; 
        for(k = 0; k < 4; k++)
        {
            const plane &p = split.cull[k];
            ivec omin, omax;
            if(p.x > 0) { omin.x = bbmin.x; omax.x = bbmax.x; } else { omin.x = bbmax.x; omax.x = bbmin.x; }
            if(p.y > 0) { omin.y = bbmin.y; omax.y = bbmax.y; } else { omin.y = bbmax.y; omax.y = bbmin.y; }
            if(p.z > 0) { omin.z = bbmin.z; omax.z = bbmax.z; } else { omin.z = bbmax.z; omax.z = bbmin.z; }
            if(omax.dist(p) < 0) { mask &= ~(1<<i); goto nextsplit; }
            if(omin.dist(p) < 0) goto notinside;
        }
        mask &= (2<<i)-1;
        break;
    notinside:
        while(++k < 4)
        {
            const plane &p = split.cull[k];
            ivec omax(p.x > 0 ? bbmax.x : bbmin.x, p.y > 0 ? bbmax.y : bbmin.y, p.z > 0 ? bbmax.z : bbmin.z);
            if(omax.dist(p) < 0) { mask &= ~(1<<i); break; }
        }
    nextsplit:;
    }
    return mask;
}

int calcspherecsmsplits(const vec &center, float radius)
{
    int mask = (1<<csmsplitn)-1;
    if(!csmcull) return mask;
    loopi(csmsplitn)
    {
        const cascaded_shadow_map::splitinfo &split = csm.splits[i];
        int k;
        for(k = 0; k < 4; k++)
        {
            const plane &p = split.cull[k];
            float dist = p.dist(center);
            if(dist < -radius) { mask &= ~(1<<i); goto nextsplit; }
            if(dist < radius) goto notinside;
        }
        mask &= (2<<i)-1;
        break;
    notinside:
        while(++k < 4)
        {
            const plane &p = split.cull[k];
            if(p.dist(center) < -radius) { mask &= ~(1<<i); break; }
        }
    nextsplit:;
    }
    return mask;
}

void viewbuffersplitmerge()
{
    const int w = screen->w, h = screen->h;
#if 1
    Shader *splitshader = lookupshaderbyname("buffersplit");
    splitshader->set();
    const float split[] = {4.f, 4.f};
    const float tiledim[] = {float(w)/split[0], float(h)/split[1]};
    LOCALPARAM(tiledim, (tiledim[0], tiledim[1]));
    LOCALPARAM(rcptiledim, (1.f/tiledim[0], 1.f/tiledim[1]));
    LOCALPARAM(split, (split[0], split[1]));
    timer_begin(TIMER_SPLITTING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,        0);        glVertex2i(-w, -h);
    glTexCoord2f(float(w), 0);        glVertex2i(w, -h);
    glTexCoord2f(0,        float(h)); glVertex2i(-w, h);
    glTexCoord2f(float(w), float(h)); glVertex2i(w, h);
    glEnd();
    timer_end(TIMER_SPLITTING);
    notextureshader->set();
#else
    Shader *mergeshader = lookupshaderbyname("buffermerge");
    mergeshader->set();
    const float split[] = {4.f, 4.f};
    const float tiledim[] = {float(w)/split[0], float(h)/split[1]};
    LOCALPARAM(tiledim, (tiledim[0], tiledim[1]));
    LOCALPARAM(rcptiledim, (1.f/tiledim[0], 1.f/tiledim[1]));
    LOCALPARAM(split, (split[0], split[1]));
    timer_begin(TIMER_MERGING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,        0);        glVertex2i(-w, -h);
    glTexCoord2f(float(w), 0);        glVertex2i(w, -h);
    glTexCoord2f(0,        float(h)); glVertex2i(-w, h);
    glTexCoord2f(float(w), float(h)); glVertex2i(w, h);
    glEnd();
    timer_end(TIMER_MERGING);
    notextureshader->set();

#endif
}
VAR(debugbuffersplit, 0, 0, 1);

VARFP(ao, 0, 1, 1, { cleanupao(); cleardeferredlightshaders(); });
FVARR(aoradius, 0, 3, 256);
FVARR(aodark, 1e-3f, 3, 1e3f);
FVARR(aosharp, 1e-3f, 1, 1e3f);
FVARR(aomin, 0, 0.25f, 1);
VARFR(aosun, 0, 1, 1, cleardeferredlightshaders());
FVARR(aosunmin, 0, 0.5f, 1);
VARP(aoblur, 0, 4, 7);
FVARP(aosigma, 0.005f, 0.5f, 2.0f);
VARP(aoiter, 0, 0, 4);
VARFP(aoreduce, 0, 1, 2, cleanupao());
VARF(aoreducedepth, 0, 1, 2, cleanupao());
VARF(aonoise, 0, 5, 8, cleanupao());
VARFP(aobilateral, 0, 7, 10, cleanupao());
FVARP(aobilateralsigma, 0, 0.5f, 1e3);
FVARP(aobilateraldepth, 0, 4, 1e3);
VARF(aopackdepth, 0, 1, 1, cleanupao());
VARFP(aotaps, 1, 5, 12, cleanupao());
VAR(debugao, 0, 0, 1);

void cascaded_shadow_map::bindparams()
{
    static GlobalShaderParam splitcenter("splitcenter"), splitbounds("splitbounds"), splitscale("splitscale"), splitoffset("splitoffset");
    vec *splitcenterv = splitcenter.reserve<vec>(csmsplitn),
        *splitboundsv = splitbounds.reserve<vec>(csmsplitn),
        *splitscalev = splitscale.reserve<vec>(csmsplitn),
        *splitoffsetv = splitoffset.reserve<vec>(csmsplitn);
    loopi(csmsplitn)
    {
        cascaded_shadow_map::splitinfo &split = csm.splits[i];
        if(split.idx < 0) continue;
        const shadowmapinfo &sm = shadowmaps[split.idx];
        const float bias = csmbias * (1024.0f / sm.size) * (split.farplane - split.nearplane) / (csm.splits[0].farplane - csm.splits[0].nearplane);
        splitcenterv[i] = vec(split.bbmin).add(split.bbmax).mul(0.5f);
        splitboundsv[i] = vec(split.bbmax).sub(split.bbmin).mul(0.5f*(sm.size - 2*smborder)/sm.size);
        splitscalev[i] = vec(0.5f*sm.size*split.proj.v[0], 0.5f*sm.size*split.proj.v[5], 0.5f*split.proj.v[10]);
        splitoffsetv[i] = vec(0.5f*sm.size*(split.proj.v[12] + 1) + sm.x, 0.5f*sm.size*(split.proj.v[13] + 1) + sm.y, 0.5f*(split.proj.v[14] + 1 - bias));
    }
    glMatrixMode(GL_TEXTURE);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glLoadMatrixf(csm.model.v);
    glActiveTexture_(GL_TEXTURE0_ARB);
    glMatrixMode(GL_MODELVIEW);
}

static Shader *deferredlightshader = NULL, *deferredminimapshader = NULL;

void cleardeferredlightshaders()
{
    deferredlightshader = NULL;
    deferredminimapshader = NULL;
}

Shader *loaddeferredlightshader(const char *prefix = NULL)
{
    string common, shadow, sun;
    int commonlen = 0, shadowlen = 0, sunlen = 0;

    if(prefix) { commonlen = strlen(prefix); memcpy(common, prefix, commonlen); }
    if(usegatherforsm()) common[commonlen++] = 'g';
    common[commonlen] = '\0';
 
    shadow[shadowlen++] = 'p';
    if(smtetra && glslversion >= 130) shadow[shadowlen++] = 't';
    shadow[shadowlen] = '\0';

    if(ao) sun[sunlen++] = 'a';
    if(sunlight && csmshadowmap)
    {
        sun[sunlen++] = 'c';
        sun[sunlen++] = '0' + csmsplitn;
        if(ao && aosun) sun[sunlen++] = 'A';
    }
    sun[sunlen] = '\0';

    defformatstring(name)("deferredlight%s%s%s", common, shadow, sun);
    return generateshader(name, "deferredlightshader \"%s\" \"%s\" \"%s\" %d", common, shadow, sun, csmsplitn);
}

void loaddeferredlightshaders()
{
    deferredlightshader = loaddeferredlightshader();
}

static inline bool sortlights(int x, int y)
{
    const lightinfo &xl = lights[x], &yl = lights[y];
    return xl.dist - xl.radius < yl.dist - yl.radius;
}

void resetlights()
{
    if(smcache)
    {
        loopk(2) shadowcache[k].reset();
        int evictx = SHADOWATLAS_SIZE + ((evictshadowcache%SHADOWCACHE_EVICT)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT, 
            evicty = ((evictshadowcache/SHADOWCACHE_EVICT)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT,
            evictx2 = SHADOWATLAS_SIZE + (((evictshadowcache%SHADOWCACHE_EVICT)+1)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT,
            evicty2 = (((evictshadowcache/SHADOWCACHE_EVICT)+1)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT; 
        bool tetra = smtetra && glslversion >= 130;
        loopv(shadowmaps)
        {
            shadowmapinfo &sm = shadowmaps[i];
            if(sm.light < 0) continue;
            lightinfo &l = lights[sm.light];
            if(sm.x >= SHADOWATLAS_SIZE)
            {
                if(shadowcachefull)
                {
                    int smw = tetra ? sm.size*2 : sm.size*3, smh = tetra ? sm.size : sm.size*2;
                    if(sm.x < evictx2 && sm.x + smw > evictx && sm.y < evicty2 && sm.y + smh > evicty) continue;
                }
                shadowcache[1][l] = sm;
            }
            else shadowcache[0][l] = sm;
        }
        shadowcachepacker.reset();
        if(shadowcachefull)
        {
            evictshadowcache = (evictshadowcache + 1)%(SHADOWCACHE_EVICT*SHADOWCACHE_EVICT);
            shadowcachefull = false;
        }
    }

    lights.setsize(0);
    lightorder.setsize(0);
    loopi(LIGHTTILE_H) loopj(LIGHTTILE_W) lighttiles[i][j].setsize(0);

    shadowmaps.setsize(0);
    shadowatlaspacker.reset();
}

VAR(depthtestlights, 0, 1, 2);
VAR(lighttilebatch, 1, 8, 8);

void renderlights(float bsx1 = -1, float bsy1 = -1, float bsx2 = 1, float bsy2 = 1, const uint *tilemask = NULL)
{
    Shader *s = minimapping ? deferredminimapshader : deferredlightshader;
    if(!s || s == nullshader) return;

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);

    if(!depthtestlights) glDisable(GL_DEPTH_TEST);
    else
    {
        glDepthMask(GL_FALSE);
        if(hasDBT && depthtestlights > 1) glEnable(GL_DEPTH_BOUNDS_TEST_EXT);
    }

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE4_ARB);
    glBindTexture(GL_TEXTURE_2D, shadowatlastex);
    if(usegatherforsm()) setsmnoncomparemode(); else setsmcomparemode();
    if(ao)
    {
        glActiveTexture_(GL_TEXTURE5_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[0]);
    }
    glActiveTexture_(GL_TEXTURE0_ARB);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(worldmatrix.v);
    glMatrixMode(GL_MODELVIEW);

    GLOBALPARAM(shadowatlasscale, (1.0f/(smcache ? 2*SHADOWATLAS_SIZE : SHADOWATLAS_SIZE), 1.0f/SHADOWATLAS_SIZE));
    if(ao) 
    {
        if((editmode && fullbright) || envmapping)
        {
            GLOBALPARAM(aoscale, (0.0f, 0.0f));
            GLOBALPARAM(aoparams, (1.0f, 0.0f, 1.0f, 0.0f));
        }
        else
        {
            GLOBALPARAM(aoscale, (float(aow)/vieww, float(aoh)/viewh));
            GLOBALPARAM(aoparams, (aomin, 1.0f-aomin, aosunmin, 1.0f-aosunmin));
        }
    }
    float lightscale = 2.0f*ldrscaleb;
    if(editmode && fullbright)
        GLOBALPARAM(lightscale, (fullbrightlevel*lightscale, fullbrightlevel*lightscale, fullbrightlevel*lightscale, 255*lightscale));
    else
        GLOBALPARAM(lightscale, (ambientcolor.x*lightscale, ambientcolor.y*lightscale, ambientcolor.z*lightscale, 255*lightscale));

    if(sunlight && csmshadowmap)
    {
        if(editmode && fullbright)
        {
            GLOBALPARAM(sunlightdir, (0, 0, 0));
            GLOBALPARAM(sunlightcolor, (0, 0, 0));
        }
        else
        {
            extern float sunlightscale;
            GLOBALPARAM(sunlightdir, (sunlightdir));
            GLOBALPARAM(sunlightcolor, (sunlightcolor.x*lightscale*sunlightscale, sunlightcolor.y*lightscale*sunlightscale, sunlightcolor.z*lightscale*sunlightscale));
        }
        csm.bindparams();
    }

    int btx1 = max(int(floor((bsx1 + 1)*0.5f*LIGHTTILE_W)), 0), bty1 = max(int(floor((bsy1 + 1)*0.5f*LIGHTTILE_H)), 0),
        btx2 = min(int(ceil((bsx2 + 1)*0.5f*LIGHTTILE_W)), LIGHTTILE_W), bty2 = min(int(ceil((bsy2 + 1)*0.5f*LIGHTTILE_H)), LIGHTTILE_H);
    for(int y = bty1; y < bty2; y++) if(!tilemask || tilemask[y]) for(int x = btx1; x < btx2; x++) if(!tilemask || tilemask[y]&(1<<x))
    {
        vector<int> &tile = lighttiles[y][x];

        static LocalShaderParam lightpos("lightpos"), lightcolor("lightcolor"), shadowparams("shadowparams"), shadowoffset("shadowoffset");
        static vec4 lightposv[8], shadowparamsv[8];
        static vec lightcolorv[8];
        static vec2 shadowoffsetv[8];

        for(int i = 0;;)
        {
            int n = min(tile.length() - i, lighttilebatch);

            bool shadowmap = n > 0 && lights[tile[i]].shadowmap >= 0;
            loopj(n)
            {
                if((lights[tile[i+j]].shadowmap >= 0) != shadowmap) { n = j; break; }
            }

            float sx1 = 1, sy1 = 1, sx2 = -1, sy2 = -1, sz1 = 1, sz2 = -1;
            loopj(n)
            {
                lightinfo &l = lights[tile[i+j]];
                lightposv[j] = vec4(l.o.x, l.o.y, l.o.z, 1.0f/l.radius);
                lightcolorv[j] = vec(l.color.x*lightscale, l.color.y*lightscale, l.color.z*lightscale);
                if(shadowmap)
                {
                    shadowmapinfo &sm = shadowmaps[l.shadowmap];
                    float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3,
                          bias = (smcullside ? smbias : -smbias) * smnearclip * (1024.0f / sm.size);
                    shadowparamsv[j] = vec4(
                        0.5f * (sm.size - smborder),
                        -smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias,
                        sm.size,
                        0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                    shadowoffsetv[j] = vec2(sm.x + 0.5f*sm.size, sm.y + 0.5f*sm.size);
                }
                sx1 = min(sx1, l.sx1);
                sy1 = min(sy1, l.sy1);
                sx2 = max(sx2, l.sx2);
                sy2 = max(sy2, l.sy2);
                sz1 = min(sz1, l.sz1);
                sz2 = max(sz2, l.sz2);
            }
            if(!i) { sx1 = sy1 = sz1 = -1; sx2 = sy2 = sz2 = 1; }
            else if(sx1 >= sx2 || sy1 >= sy2 || sz1 >= sz2) continue;

            if(n) 
            {
                s->setvariant(n-1, (shadowmap ? 1 : 0) + (i ? 2 : 0));
                lightpos.set(lightposv, n);
                lightcolor.set(lightcolorv, n);
                if(shadowmap)   
                {
                    shadowparams.set(shadowparamsv, n);
                    shadowoffset.set(shadowoffsetv, n);
                }
            } 
            else s->set();

            sx1 = max(sx1, bsx1);
            sy1 = max(sy1, bsy1);
            sx2 = min(sx2, bsx2);
            sy2 = min(sy2, bsy2);
            if(sx1 < sx2 && sy1 < sy2)
            { 
                int tx1 = max(int(floor((sx1*0.5f+0.5f)*vieww)), (x*vieww)/LIGHTTILE_W), ty1 = max(int(floor((sy1*0.5f+0.5f)*viewh)), (y*viewh)/LIGHTTILE_H),
                    tx2 = min(int(ceil((sx2*0.5f+0.5f)*vieww)), ((x+1)*vieww)/LIGHTTILE_W), ty2 = min(int(ceil((sy2*0.5f+0.5f)*viewh)), ((y+1)*viewh)/LIGHTTILE_H);

                glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

                if(hasDBT && depthtestlights > 1) glDepthBounds_(sz1*0.5f + 0.5f, sz2*0.5f + 0.5f);

                // FIXME: render light geometry here
                glBegin(GL_TRIANGLE_STRIP);
                glVertex3f( 1, -1, sz1);
                glVertex3f(-1, -1, sz1);
                glVertex3f( 1,  1, sz1);
                glVertex3f(-1,  1, sz1);
                glEnd();
            }

            i += n;
            if(i >= tile.length()) break;
        }
    }

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);

    if(!depthtestlights) glEnable(GL_DEPTH_TEST);
    else
    {
        glDepthMask(GL_TRUE);
        if(hasDBT && depthtestlights > 1) glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

VAR(oqlights, 0, 1, 1);

void collectlights()
{
    // point lights processed here
    const vector<extentity *> &ents = entities::getents();
    if(!editmode || !fullbright) loopv(ents)
    {
        extentity *e = ents[i];
        if(e->type != ET_LIGHT) continue;

        float sx1 = -1, sy1 = -1, sx2 = 1, sy2 = 1, sz1 = -1, sz2 = 1;
        if(e->attr1 > 0)
        {
            if(smviscull)
            {
                if(isfoggedsphere(e->attr1, e->o)) continue;
                if(pvsoccludedsphere(e->o, e->attr1)) continue;
            }
            calcspherescissor(e->o, e->attr1, sx1, sy1, sx2, sy2);
            calcspheredepth(e->o, e->attr1, sz1, sz2);
        }
        if(sx1 >= sx2 || sy1 >= sy2 || sz1 >= sz2) { sx1 = sy1 = sz1 = -1; sx2 = sy2 = sz2 = 1; }

        lightorder.add(lights.length());
        lightinfo &l = lights.add();
        l.sx1 = sx1;
        l.sx2 = sx2;
        l.sy1 = sy1;
        l.sy2 = sy2;
        l.sz1 = sz1;
        l.sz2 = sz2;
        l.shadowmap = -1;
        l.flags = e->attr5;
        l.query = NULL;
        l.o = e->o;
        l.color = vec(e->attr2, e->attr3, e->attr4);
        l.radius = e->attr1 > 0 ? e->attr1 : 2*worldsize;
        l.dist = camera1->o.dist(e->o);
    }

    updatedynlights();
    int numdynlights = finddynlights();
    loopi(numdynlights)
    {
        vec o, color;
        float radius;
        if(!getdynlight(i, o, radius, color)) continue;

        float sx1 = -1, sy1 = -1, sx2 = 1, sy2 = 1, sz1 = -1, sz2 = 1;
        calcspherescissor(o, radius, sx1, sy1, sx2, sy2);
        calcspheredepth(o, radius, sz1, sz2);
        if(sx1 >= sx2 || sy1 >= sy2 || sz1 >= sz2) { sx1 = sy1 = sz1 = -1; sx2 = sy2 = sz2 = 1; }

        lightorder.add(lights.length());
        lightinfo &l = lights.add();
        l.sx1 = sx1;
        l.sx2 = sx2;
        l.sy1 = sy1;
        l.sy2 = sy2;
        l.sz1 = sz1;
        l.sz2 = sz2;
        l.shadowmap = -1;
        l.flags = 0;
        l.query = NULL;
        l.o = o;
        l.color = vec(color).mul(255);
        l.radius = radius;
        l.dist = camera1->o.dist(o);
    }

    lightorder.sort(sortlights);

    if(!envmapping && smquery && hasOQ && oqfrags && oqlights) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(!(l.flags&L_NOSHADOW) && l.radius > smminradius && l.radius < worldsize &&
           (camera1->o.x < l.o.x - l.radius - 2 || camera1->o.x > l.o.x + l.radius + 2 ||
            camera1->o.y < l.o.y - l.radius - 2 || camera1->o.y > l.o.y + l.radius + 2 ||
            camera1->o.z < l.o.z - l.radius - 2 || camera1->o.z > l.o.z + l.radius + 2))
        {
            l.query = newquery(&l);
            if(l.query)
            {
                startquery(l.query);
                ivec bo = vec(l.o).sub(l.radius), br = vec(l.o).add(l.radius+1);
                br.sub(bo);
                drawbb(bo, br);
                endquery(l.query);
            }
        }
    }
}

static inline void addlighttiles(const lightinfo &l, int idx)
{
    int tx1 = max(int(floor((l.sx1 + 1)*0.5f*LIGHTTILE_W)), 0), ty1 = max(int(floor((l.sy1 + 1)*0.5f*LIGHTTILE_H)), 0),
        tx2 = min(int(ceil((l.sx2 + 1)*0.5f*LIGHTTILE_W)), LIGHTTILE_W), ty2 = min(int(ceil((l.sy2 + 1)*0.5f*LIGHTTILE_H)), LIGHTTILE_H);
    for(int y = ty1; y < ty2; y++) for(int x = tx1; x < tx2; x++) { lighttiles[y][x].add(idx); lighttilesused++; }
}
 
VAR(lightsvisible, 1, 0, 0);
VAR(lightsoccluded, 1, 0, 0);
 
void packlights()
{ 
    lightsvisible = lightsoccluded = 0;
    lighttilesused = 0;
    smused = 0;

    bool tetra = smtetra && glslversion >= 130;
    if(smcache && smnoshadow <= 1 && shadowcache[1].numelems) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.flags&L_NOSHADOW || l.radius <= smminradius) continue;
        if(l.query && l.query->owner == &l && checkquery(l.query)) continue; 

        float smlod = clamp(l.radius * smprec / sqrtf(max(1.0f, l.dist/l.radius)), float(smminsize), float(smmaxsize));
        int smsize = clamp(int(ceil(smlod * (tetra ? smtetraprec : smcubeprec))), 1, SHADOWATLAS_SIZE / (tetra ? 2 : 3)),
            smw = tetra ? smsize*2 : smsize*3, smh = tetra ? smsize : smsize*2;
        ushort smx = USHRT_MAX, smy = USHRT_MAX;
        shadowmapinfo *sm = NULL;
        int smidx = -1;
        shadowcacheval *cached = shadowcache[1].access(l);
        if(!cached || cached->size != smsize) continue;
        smx = cached->x;
        smy = cached->y;
        shadowcachepacker.reserve(smx - SHADOWATLAS_SIZE, smy, smw, smh);
        sm = addshadowmap(smx, smy, smsize, smidx);
        sm->light = idx;
        sm->cached = cached;
        l.shadowmap = smidx;
        smused += smsize*smsize*(tetra ? 2 : 6);

        addlighttiles(l, idx);
    }
    loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.shadowmap >= 0) continue;

        if(!(l.flags&L_NOSHADOW) && smnoshadow <= 1 && l.radius > smminradius)
        {
            if(l.query && l.query->owner == &l && checkquery(l.query)) { lightsoccluded++; continue; }
            float smlod = clamp(l.radius * smprec / sqrtf(max(1.0f, l.dist/l.radius)), float(smminsize), float(smmaxsize));
            int smsize = clamp(int(ceil(smlod * (tetra ? smtetraprec : smcubeprec))), 1, SHADOWATLAS_SIZE / (tetra ? 2 : 3)),
                smw = tetra ? smsize*2 : smsize*3, smh = tetra ? smsize : smsize*2;
            ushort smx = USHRT_MAX, smy = USHRT_MAX;
            shadowmapinfo *sm = NULL;
            int smidx = -1;
            if(smcache)
            {
                shadowcacheval *prev = shadowcache[0].access(l);
                if(prev && prev->size == smsize)
                {
                    if(shadowcachepacker.insert(smx, smy, smw, smh))
                    {
                        smx += SHADOWATLAS_SIZE;
                        sm = addshadowmap(smx, smy, smsize, smidx); 
                        sm->light = idx;
                        l.shadowmap = smidx;
                        smused += smsize*smsize*(tetra ? 2 : 6);

                        addlighttiles(l, idx);
                        continue;
                    }
                    shadowcachefull = true;
                }
            }
            if(shadowatlaspacker.insert(smx, smy, smw, smh))
            {
                sm = addshadowmap(smx, smy, smsize, smidx);
                sm->light = idx;
                l.shadowmap = smidx;
                smused += smsize*smsize*(tetra ? 2 : 6);
            }
        }
   
        addlighttiles(l, idx); 
    }

    lightsvisible = lightorder.length() - lightsoccluded;
}

void rendercsmshadowmaps()
{
    shadowmapping = SM_CASCADE;
    shadoworigin = vec(0, 0, 0);
    shadowradius = 2*worldsize;
    shadowbias = 0;

    if(csmpolyfactor || csmpolyoffset)
    {
        glPolygonOffset(csmpolyfactor, csmpolyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }

    findcsmshadowvas(); // no culling here
    findcsmshadowmms(); // no culling here

    shadowmaskbatchedmodels();
    batchshadowmapmodels();

    loopi(csmsplitn) if(csm.splits[i].idx >= 0)
    {
        const shadowmapinfo &sm = shadowmaps[csm.splits[i].idx];
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(csm.splits[i].proj.v);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(csm.model.v);
        glViewport(sm.x, sm.y, sm.size, sm.size);
        glScissor(sm.x, sm.y, sm.size, sm.size);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowside = i;

        rendershadowmapworld();
        rendermodelbatches();
    }

    clearbatchedmapmodels();

    if(csmpolyfactor || csmpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowmaps()
{
    shadowmapping = smtetra && glslversion >= 130 ? SM_TETRA : SM_CUBEMAP;

    if(smpolyfactor || smpolyoffset)
    {
        glPolygonOffset(smpolyfactor, smpolyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    if(shadowmapping == SM_TETRA && smtetraclip) glEnable(GL_CLIP_PLANE0);

    loopv(shadowmaps)
    {
        shadowmapinfo &sm = shadowmaps[i];
        if(sm.light < 0) continue;

        lightinfo &l = lights[sm.light];

        int sidemask;
        if(shadowmapping == SM_TETRA) sidemask = smsidecull ? cullfrustumtetra(l.o, l.radius, sm.size, smborder) : 0xF;
        else sidemask = smsidecull ? cullfrustumsides(l.o, l.radius, sm.size, smborder) : 0x3F;

        sm.sidemask = sidemask;

        shadoworigin = l.o;
        shadowradius = l.radius;
        shadowbias = smborder / float(sm.size - smborder);

        findshadowvas();
        findshadowmms();

        shadowmaskbatchedmodels(!(l.flags&L_NODYNSHADOW));
        batchshadowmapmodels();

        shadowcacheval *cached = NULL;
        int cachemask = 0;
        if(smcache)
        {
            int dynmask = smcache <= 1 ? batcheddynamicmodels() : 0;
            if(shadowmapping == SM_TETRA && !smtetraclear)
            {
                if(dynmask&0x3) dynmask |= 0x3;
                if(dynmask&0xC) dynmask |= 0xC;
            }
            cached = sm.cached;
            if(cached)
            {
                if(!debugshadowatlas) cachemask = cached->sidemask & ~dynmask;
                sm.sidemask |= cachemask;
            }
            sm.sidemask &= ~dynmask;

            sidemask &= ~cachemask;
            if(!sidemask) { clearbatchedmapmodels(); continue; }
        }

        float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3;
        GLfloat smprojmatrix[16] =
        {
            float(sm.size - smborder) / sm.size, 0, 0, 0,
            0, float(sm.size - smborder) / sm.size, 0, 0,
            0, 0, -(smfarclip + smnearclip) / (smfarclip - smnearclip), -1,
            0, 0, -2*smnearclip*smfarclip / (smfarclip - smnearclip), 0
        };
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(smprojmatrix);
        glMatrixMode(GL_MODELVIEW);

        GLfloat lightmatrix[16] =
        {
            1.0f/l.radius, 0, 0, 0,
            0, 1.0f/l.radius, 0, 0,
            0, 0, 1.0f/l.radius, 0,
            -l.o.x/l.radius, -l.o.y/l.radius, -l.o.z/l.radius, 1
        };

        glmatrixf smviewmatrix;

        if(shadowmapping == SM_TETRA)
        {
            if(!cachemask)
            {
                glScissor(sm.x + (sidemask & 0x3 ? 0 : sm.size), sm.y, sidemask & 0x3 && sidemask & 0xC ? 2*sm.size : sm.size, sm.size);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
            loop(side, 4) if(sidemask&(1<<side))
            {
                int sidex = (side>>1)*sm.size;
                if(!(side&1) || !(sidemask&(1<<(side-1))))
                {
                    glViewport(sm.x + sidex, sm.y, sm.size, sm.size);
                    glScissor(sm.x + sidex, sm.y, sm.size, sm.size);
                    if(cachemask && (!smtetraclear || (sidemask&(3<<(side&~1))) == (3<<(side&~1)))) glClear(GL_DEPTH_BUFFER_BIT);
                }

                if(cachemask && smtetraclear && (sidemask&(3<<(side&~1))) != (3<<(side&~1)))
                {
                    if(smtetraclip) glDisable(GL_CLIP_PLANE0);
                    if(smpolyfactor || smpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);
                    glDisable(GL_CULL_FACE);
                    glDepthFunc(GL_ALWAYS);

                    SETSHADER(tetraclear);
                    glBegin(GL_TRIANGLES);
                    switch(side)
                    {
                    case 0: glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1); glVertex3f(1, -1, 1); break;
                    case 1: glVertex3f(-1, -1, 1); glVertex3f(-1, 1, 1); glVertex3f(1, -1, 1); break;
                    case 2: glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1); glVertex3f(1, 1, 1); break;
                    case 3: glVertex3f(-1, 1, 1); glVertex3f(-1, -1, 1); glVertex3f(1, 1, 1); break;
                    }
                    glEnd();

                    glDepthFunc(GL_LESS);
                    glEnable(GL_CULL_FACE);
                    if(smpolyfactor || smpolyoffset) glEnable(GL_POLYGON_OFFSET_FILL);
                    if(smtetraclip) glEnable(GL_CLIP_PLANE0);
                }

                smviewmatrix.mul(tetrashadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side>>1) ^ smcullside ? GL_BACK : GL_FRONT);

                if(smtetraclip)
                {
                    smtetraclipplane.toplane(vec(-smviewmatrix.v[2], -smviewmatrix.v[6], 0), l.o);
                    smtetraclipplane.offset += smtetraborder/(0.5f*sm.size);
                    GLOBALPARAM(tetraclip, (smtetraclipplane));
                }

                shadowside = side;

                rendershadowmapworld();
                rendermodelbatches();
            }
        }
        else
        {
            if(!cachemask)
            {
                int cx1 = sidemask & 0x03 ? 0 : (sidemask & 0xC ? sm.size : 2 * sm.size),
                    cx2 = sidemask & 0x30 ? 3 * sm.size : (sidemask & 0xC ? 2 * sm.size : sm.size),
                    cy1 = sidemask & 0x15 ? 0 : sm.size,
                    cy2 = sidemask & 0x2A ? 2 * sm.size : sm.size;
                glScissor(sm.x + cx1, sm.y + cy1, cx2 - cx1, cy2 - cy1);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
            loop(side, 6) if(sidemask&(1<<side))
            {
                int sidex = (side>>1)*sm.size, sidey = (side&1)*sm.size;
                glViewport(sm.x + sidex, sm.y + sidey, sm.size, sm.size);
                glScissor(sm.x + sidex, sm.y + sidey, sm.size, sm.size);
                if(cachemask) glClear(GL_DEPTH_BUFFER_BIT);

                smviewmatrix.mul(cubeshadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side & 1) ^ (side >> 2) ^ smcullside ? GL_FRONT : GL_BACK);

                shadowside = side;

                rendershadowmapworld();
                rendermodelbatches();
            }
        }

        clearbatchedmapmodels();
    }

    if(shadowmapping == SM_TETRA && smtetraclip) glDisable(GL_CLIP_PLANE0);
    if(smpolyfactor || smpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowatlas()
{
    timer_begin(TIMER_CPU_SM);
    timer_begin(TIMER_SM);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    if(debugshadowatlas)
    {
        glClearDepth(0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearDepth(1);
    }

    glEnable(GL_SCISSOR_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // sun light
    if(sunlight && csmshadowmap) 
    {
        csm.setup();
        rendercsmshadowmaps();
    }

    packlights();

    // point lights
    rendershadowmaps();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    timer_end(TIMER_SM);
    timer_end(TIMER_CPU_SM);
}

void setbilateralshader(int radius, int pass, float sigma, float depth, bool linear, bool packed)
{
    float stepx = linear ? 1 : float(vieww)/aow, stepy = linear ? 1 : float(viewh)/aoh;
    bilateralshader[pass]->set();
    sigma *= 2*radius;
    LOCALPARAM(bilateralparams, (1.0f/(2*sigma*sigma), 1.0f/(depth*depth), pass==0 ? stepx : 0, pass==1 ? stepy : 0));
}

void renderao()
{
    timer_begin(TIMER_AO);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);

    bool linear = hasTRG && hasTF && aoreducedepth && (aoreduce || aoreducedepth > 1);
    float xscale = eyematrix.v[0], yscale = eyematrix.v[5];
    if(linear)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[2]);
        glViewport(0, 0, aow, aoh);
        SETSHADER(linearizedepth);
        screenquad(vieww, viewh);

        xscale *= float(vieww)/aow;
        yscale *= float(viewh)/aoh;

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[2]);
    }

    ambientobscuranceshader->set();

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[0]);
    glViewport(0, 0, aow, aoh);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, aonoisetex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    LOCALPARAM(tapparams, (aoradius*eyematrix.v[14]/xscale, aoradius*eyematrix.v[14]/yscale, (2.0f*M_PI*aodark)/aotaps, aosharp));
    LOCALPARAM(offsetscale, (xscale/eyematrix.v[14], yscale/eyematrix.v[14], eyematrix.v[12]/eyematrix.v[14], eyematrix.v[13]/eyematrix.v[14]));
    screenquad(vieww, viewh, aow/float(1<<aonoise), aoh/float(1<<aonoise));

    if(aobilateral)
    {
        if(!linear && aopackdepth && hasTRG && hasTF) linear = true;
        loopi(2 + 2*aoiter)
        {
            setbilateralshader(aobilateral, i%2, aobilateralsigma, aobilateraldepth, linear, linear && aopackdepth);
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[i%2]);
            if(!linear || !aopackdepth)
            {
                glActiveTexture_(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, linear ? aotex[2] : gdepthtex);
                glActiveTexture_(GL_TEXTURE0_ARB);
            }
            screenquad(vieww, viewh);
        }
    }
    else if(aoblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(aoblur, aosigma, blurweights, bluroffsets);
        loopi(2 + 2*aoiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            setblurshader(i%2, 1, aoblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[i%2]);
            screenquad(aow, aoh);
        }
    }

    timer_end(TIMER_AO);
}

void processhdr(GLuint outfbo = 0)
{
    if(!hdr)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
        glViewport(0, 0, vieww, viewh);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(vieww, viewh);
        return;
    }

    timer_begin(TIMER_HDR);
    GLuint b0fbo = bloomfbo[1], b0tex = bloomtex[1], b1fbo =  bloomfbo[0], b1tex = bloomtex[0], ptex = hdrtex;
    int b0w = max(vieww/4, bloomw), b0h = max(viewh/4, bloomh), b1w = max(vieww/2, bloomw), b1h = max(viewh/2, bloomh),
        pw = vieww, ph = viewh;
    if(hdrreduce) while(pw > bloomw || ph > bloomh)
    {
        GLuint cfbo = b1fbo, ctex = b1tex;
        int cw = max(pw/2, bloomw), ch = max(ph/2, bloomh);

        if(hdrreduce > 1 && cw/2 >= bloomw && ch/2 >= bloomh)
        {
            cw /= 2;
            ch /= 2;
            SETSHADER(hdrreduce2);
        }
        else SETSHADER(hdrreduce);
        if(cw == bloomw && ch == bloomh) { cfbo = bloomfbo[2]; ctex = bloomtex[2]; }
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, cfbo);
        glViewport(0, 0, cw, ch);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ptex);
        screenquad(pw, ph);

        ptex = ctex;
        pw = cw;
        ph = ch;
        swap(b0fbo, b1fbo);
        swap(b0tex, b1tex);
        swap(b0w, b1w);
        swap(b0h, b1h);
    }

    if(!lasthdraccum || lastmillis - lasthdraccum >= hdraccummillis)
    {
        GLuint ltex = ptex;
        int lw = pw, lh = ph;
        for(int i = 0; lw > 2 || lh > 2; i++)
        {
            int cw = max(lw/2, 2), ch = max(lh/2, 2);

            if(hdrreduce > 1 && cw/2 >= 2 && ch/2 >= 2)
            {
                cw /= 2;
                ch /= 2;
                if(i) SETSHADER(hdrreduce2); else SETSHADER(hdrluminance2);
            }
            else if(i) SETSHADER(hdrreduce); else SETSHADER(hdrluminance);
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
            glViewport(0, 0, cw, ch);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ltex);
            screenquad(lw, lh);

            ltex = b1tex;
            lw = cw;
            lh = ch;
            swap(b0fbo, b1fbo);
            swap(b0tex, b1tex);
            swap(b0w, b1w);
            swap(b0h, b1h);
        }

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, bloomfbo[4]);
        glViewport(0, 0, 1, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        SETSHADER(hdraccum);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
        LOCALPARAM(accumscale, (lasthdraccum ? pow(hdraccumscale, float(lastmillis - lasthdraccum)/hdraccummillis) : 0));
        screenquad(2, 2);
        glDisable(GL_BLEND);

        lasthdraccum = lastmillis;
    }

    b0fbo = bloomfbo[3];
    b0tex = bloomtex[3];
    b1fbo = bloomfbo[2];
    b1tex = bloomtex[2];
    b0w = b1w = bloomw;
    b0h = b1h = bloomh;

    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomtex[4]);
    glActiveTexture_(GL_TEXTURE0_ARB);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b0fbo);
    glViewport(0, 0, b0w, b0h);
    SETSHADER(hdrbloom);
    LOCALPARAM(bloomparams, (-bloombright, bloomthreshold*bloombright/hdrbright, bloomlummin, bloomlummax));
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ptex);
    screenquad(pw, ph);

    if(bloomblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(bloomblur, bloomsigma, blurweights, bluroffsets);
        loopi(2 + 2*bloomiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
            glViewport(0, 0, b1w, b1h);
            setblurshader(i%2, 1, bloomblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
            screenquad(b0w, b0h);
            swap(b0w, b1w);
            swap(b0h, b1h);
            swap(b0tex, b1tex);
            swap(b0fbo, b1fbo);
        }
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
    glViewport(0, 0, vieww, viewh);
    SETSHADER(hdrtonemap);
    LOCALPARAM(bloomsize, (b0w, b0h));
    LOCALPARAM(hdrparams, (-hdrbright, bloomscale, hdrtonemin, hdrtonemax));
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    timer_end(TIMER_HDR);
}

void dosmaa()
{
    timer_begin(TIMER_SMAA);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if(smaadepthmask) 
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthRange(1, 1);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil)) 
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 8, 8);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    if(smaacoloredge) smaacoloredgeshader->set();
    else smaalumaedgeshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    screenquad(vieww, viewh);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[2]);
    if(smaadepthmask)
    {
        glDepthFunc(GL_EQUAL);
        glDepthMask(GL_FALSE);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil))
    {
        glStencilFunc(GL_EQUAL, 8, 8);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    smaablendweightshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaaareatex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaasearchtex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if(smaadepthmask) 
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDepthRange(0, 1);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil)) glDisable(GL_STENCIL_TEST);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
    smaaneighborhoodshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    timer_end(TIMER_SMAA);
}

FVAR(refractmargin, 0, 0.1f, 1);
FVAR(refractdepth, 1e-3f, 16, 1e3f);

void rendertransparent()
{
    int hasalphavas = findalphavas();
    int hasmats = findmaterials();
    if(!hasalphavas && !hasmats) return;
        
    timer_begin(TIMER_TRANSPARENT);

    if(hasalphavas&4 || hasmats&4)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, refractfbo);
        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
        float sx1 = min(alpharefractsx1, matrefractsx1), sy1 = min(alpharefractsy1, matrefractsy1),
              sx2 = max(alpharefractsx2, matrefractsx2), sy2 = max(alpharefractsy2, matrefractsy2);
        bool scissor = sx1 > -1 || sy1 > -1 || sx2 < 1 || sy2 < 1;
        if(scissor)
        {
            int x1 = int(floor(max(sx1*0.5f+0.5f-refractmargin*viewh/vieww, 0.0f)*vieww)),
                y1 = int(floor(max(sy1*0.5f+0.5f-refractmargin, 0.0f)*viewh)),
                x2 = int(ceil(min(sx2*0.5f+0.5f+refractmargin*viewh/vieww, 1.0f)*vieww)),
                y2 = int(ceil(min(sy2*0.5f+0.5f+refractmargin, 1.0f)*viewh));
            glEnable(GL_SCISSOR_TEST);
            glScissor(x1, y1, x2 - x1, y2 - y1);
        }
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        if(scissor) glDisable(GL_SCISSOR_TEST);
        SETSHADER(refractmask);
		LOCALPARAM(refractdepth, (1.0f/refractdepth));
        if(hasalphavas&4) renderrefractmask();
        if(hasmats&4) rendermaterialmask();
        glDepthMask(GL_TRUE);
    }

    glActiveTexture_(GL_TEXTURE7_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, refracttex);
    glActiveTexture_(GL_TEXTURE8_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
    glActiveTexture_(GL_TEXTURE9_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    if((gdepthstencil && hasDS) || gstencil) glEnable(GL_STENCIL_TEST);

    glmatrixf raymatrix = mvmatrix.v;
    loopk(4)
    {
        raymatrix.v[0 + k*4] = 0.5f*vieww*(raymatrix.v[2 + k*4] - raymatrix.v[0 + k*4]*projmatrix.v[0]);
        raymatrix.v[1 + k*4] = 0.5f*viewh*(raymatrix.v[2 + k*4] - raymatrix.v[1 + k*4]*projmatrix.v[5]);
    }

    loop(layer, 3)
    {
        uint tiles[LIGHTTILE_H];
        float sx1, sy1, sx2, sy2;
        switch(layer)
        {
        case 0:
            if(!(hasmats&1)) continue;
            sx1 = matliquidsx1; sy1 = matliquidsy1; sx2 = matliquidsx2; sy2 = matliquidsy2;
            memcpy(tiles, matliquidtiles, sizeof(tiles));
            break;
        case 1:
            if(!(hasalphavas&1)) continue;
            sx1 = alphabacksx1; sy1 = alphabacksy1; sx2 = alphabacksx2; sy2 = alphabacksy2;
            memcpy(tiles, alphatiles, sizeof(tiles));
            break;
        case 2:
            if(!(hasalphavas&2) && !(hasmats&2)) continue;
            sx1 = alphafrontsx1; sy1 = alphafrontsy1; sx2 = alphafrontsx2; sy2 = alphafrontsy2;
            memcpy(tiles, alphatiles, sizeof(tiles));
            if(hasmats&2)
            {
                sx1 = min(sx1, matsolidsx1);
                sy1 = min(sy1, matsolidsy1);
                sx2 = max(sx2, matsolidsx2);
                sy2 = max(sy2, matsolidsy2);
                loopj(LIGHTTILE_H) tiles[j] |= matsolidtiles[j];
            }
            break;
        default:
            continue;
        }

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
        if((gdepthstencil && hasDS) || gstencil)
        {
            glStencilFunc(GL_ALWAYS, 1<<layer, 1<<layer);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            maskgbuffer("cg");
            bool scissor = sx1 > -1 || sy1 > -1 || sx2 < 1 || sy2 < 1;
            if(scissor)
            {
                int x1 = int(floor((sx1*0.5f+0.5f)*vieww)), y1 = int(floor((sy1*0.5f+0.5f)*viewh)),
                    x2 = int(ceil((sx2*0.5f+0.5f)*vieww)), y2 = int(ceil((sy2*0.5f+0.5f)*viewh));
                glEnable(GL_SCISSOR_TEST);
                glScissor(x1, y1, x2 - x1, y2 - y1);
            }
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            if(scissor) glDisable(GL_SCISSOR_TEST);
            maskgbuffer("cngd");
        }

        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(linearworldmatrix.v);
        glActiveTexture_(GL_TEXTURE1_ARB);
        glLoadMatrixf(raymatrix.v);
        glActiveTexture_(GL_TEXTURE0_ARB);

        switch(layer)
        {
        case 0:
            if(hasmats&1) renderliquidmaterials();
            break;
        case 1:
            if(hasalphavas&1) renderalphageom(1);
            break;
        case 2:
            if(hasalphavas&2) renderalphageom(2);
            if(hasmats&2) rendersolidmaterials();
            break;
        }

        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
        if((gdepthstencil && hasDS) || gstencil)
        {
            glStencilFunc(GL_EQUAL, 1<<layer, 1<<layer);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }

        renderlights(sx1, sy1, sx2, sy2, tiles);
    }

    if((gdepthstencil && hasDS) || gstencil) glDisable(GL_STENCIL_TEST);

    timer_end(TIMER_TRANSPARENT);
}

VAR(gdepthclear, 0, 1, 1);
VAR(gcolorclear, 0, 1, 1);

void rendergbuffer()
{
    timer_begin(TIMER_CPU_GBUFFER);
    timer_begin(TIMER_GBUFFER);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
    glViewport(0, 0, vieww, viewh);

    if(gdepthformat && gdepthclear)
    {
        maskgbuffer("d");
        if(gdepthformat == 1) glClearColor(1, 1, 1, 1);
        else glClearColor(-farplane, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 0);
        maskgbuffer("cng");
    }
    glClear((minimapping < 2 ? GL_DEPTH_BUFFER_BIT : 0)|(gcolorclear ? GL_COLOR_BUFFER_BIT : 0)|(minimapping < 2 && ((gdepthstencil && hasDS) || gstencil) ? GL_STENCIL_BUFFER_BIT : 0));
    if(gdepthformat && gdepthclear) maskgbuffer("cngd");

    glmatrixf screenmatrix;
    screenmatrix.identity();
    screenmatrix.scale(2.0f/vieww, 2.0f/viewh, 2.0f);
    screenmatrix.translate(-1.0f, -1.0f, -1.0f);
    eyematrix.mul(invprojmatrix, screenmatrix);
    if(minimapping)
    {
        linearworldmatrix.mul(invmvpmatrix, screenmatrix);
        worldmatrix = linearworldmatrix;
        glScissor(1, 1, vieww-2, viewh-2);
        glEnable(GL_SCISSOR_TEST);
    }
    else
    {
        linearworldmatrix.mul(invprojmatrix, screenmatrix);
        float xscale = linearworldmatrix.v[0], yscale = linearworldmatrix.v[5], xoffset = linearworldmatrix.v[12], yoffset = linearworldmatrix.v[13], zscale = linearworldmatrix.v[14];
        GLfloat depthmatrix[16] =
        {
            xscale/zscale,  0,              0, 0,
            0,              yscale/zscale,  0, 0,
            xoffset/zscale, yoffset/zscale, 1, 0,
            0,              0,              0, 1
        };
        linearworldmatrix.mul(invmvmatrix.v, depthmatrix);
        if(gdepthformat) worldmatrix = linearworldmatrix;
        else worldmatrix.mul(invmvpmatrix, screenmatrix);
    }

    GLOBALPARAM(viewsize, (vieww, viewh, 1.0f/vieww, 1.0f/viewh));
    GLOBALPARAM(gdepthscale, (eyematrix.v[14], eyematrix.v[11], eyematrix.v[15]));
    GLOBALPARAM(gdepthpackparams, (-1.0f/farplane, -255.0f/farplane, -(255.0f*255.0f)/farplane));
    GLOBALPARAM(gdepthunpackparams, (-farplane, -farplane/255.0f, -farplane/(255.0f*255.0f)));

    GLERROR;
    if(limitsky())
    {
        renderexplicitsky();
        GLERROR;
    }
    rendergeom();
    GLERROR;
    resetmodelbatches();
    rendermapmodels();
    GLERROR;
    if(minimapping)
    {
        renderminimapmaterials();
        glDisable(GL_SCISSOR_TEST);
    }
    else if(!envmapping)
    {
        game::rendergame();
        rendermodelbatches();
        GLERROR;
        maskgbuffer("c");
        renderdecals(true);
        maskgbuffer("cngd");
        GLERROR;
        if(!isthirdperson())
        {
            project(curavatarfov, aspect, farplane, avatardepth);
            game::renderavatar();
            project(fovy, aspect, farplane);
        }
        GLERROR;
    }

    timer_end(TIMER_GBUFFER);
    timer_end(TIMER_CPU_GBUFFER);
}

void shadegbuffer()
{
    GLERROR;
    timer_begin(TIMER_CPU_SHADING);
    timer_begin(TIMER_SHADING);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    glViewport(0, 0, vieww, viewh);

    if(!minimapping) 
    {
        if(hdrclear > 0)
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            hdrclear--;
        }
        drawskybox(farplane);
    }

    if(minimapping) renderlights(2.0f/vieww - 1, 2.0f/viewh - 1, 1 - 2.0f/vieww, 1 - 2.0f/viewh);
    else renderlights();
    GLERROR;

    timer_end(TIMER_SHADING);
    timer_end(TIMER_CPU_SHADING);
}

void drawminimap()
{
    if(!game::needminimap()) { clearminimap(); return; }

    renderprogress(0, "generating mini-map...", 0, !renderedframe);

    gl_setupframe(screen->w, screen->h);

    if(!deferredminimapshader) deferredminimapshader = loaddeferredlightshader("m");

    int size = 1<<minimapsize, sizelimit = min(hwtexsize, min(vieww, viewh));
    while(size > sizelimit) size /= 2;
    if(!minimaptex) glGenTextures(1, &minimaptex);

    extern vector<vtxarray *> valist;
    ivec bbmin(worldsize, worldsize, worldsize), bbmax(0, 0, 0);
    loopv(valist)
    {
        vtxarray *va = valist[i];
        loopk(3)
        {
            if(va->geommin[k]>va->geommax[k]) continue;
            bbmin[k] = min(bbmin[k], va->geommin[k]);
            bbmax[k] = max(bbmax[k], va->geommax[k]);
        }
    }
    if(minimapclip)
    {
        ivec clipmin(worldsize, worldsize, worldsize), clipmax(0, 0, 0);
        clipminimap(clipmin, clipmax);
        loopk(2) bbmin[k] = max(bbmin[k], clipmin[k]);
        loopk(2) bbmax[k] = min(bbmax[k], clipmax[k]);
    }

    minimapradius = bbmax.tovec().sub(bbmin.tovec()).mul(0.5f);
    minimapcenter = bbmin.tovec().add(minimapradius);
    minimapradius.x = minimapradius.y = max(minimapradius.x, minimapradius.y);
    minimapscale = vec((0.5f - 1.0f/size)/minimapradius.x, (0.5f - 1.0f/size)/minimapradius.y, 1.0f);

    envmapping = true;
    minimapping = 1;

    physent *oldcamera = camera1;
    static physent cmcamera;
    cmcamera = *player;
    cmcamera.reset();
    cmcamera.type = ENT_CAMERA;
    cmcamera.o = vec(minimapcenter.x, minimapcenter.y, minimapheight > 0 ? minimapheight : minimapcenter.z + minimapradius.z + 1);
    cmcamera.yaw = 0;
    cmcamera.pitch = -90;
    cmcamera.roll = 0;
    camera1 = &cmcamera;
    setviewcell(vec(-1, -1, -1));

    float oldldrscale = ldrscale, oldldrscaleb = ldrscaleb;
    int oldvieww = vieww, oldviewh = viewh;
    vieww = viewh = size;

    float zscale = max(float(minimapheight), minimapcenter.z + minimapradius.z + 1) + 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, 0, 2*zscale);
    glMatrixMode(GL_MODELVIEW);

    transplayer();
    readmatrices();

    resetlights();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    xtravertsva = xtraverts = glde = gbatches = 0;

    ldrscale = 1;
    ldrscaleb = ldrscale/255;
    GLOBALPARAM(ldrscale, (ldrscale));
    GLOBALPARAM(camera, (camera1->o.x, camera1->o.y, camera1->o.z, 1));
    GLOBALPARAM(millis, (lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f));

    visiblecubes(false);
    collectlights();
    rendershadowatlas();
    findmaterials();

    rendergbuffer();

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    glViewport(0, 0, vieww, viewh);
    glClearColor(minimapcolor.x*ldrscaleb, minimapcolor.y*ldrscaleb, minimapcolor.z*ldrscaleb, 0); 
    glClear(GL_COLOR_BUFFER_BIT);

    shadegbuffer();

    if(minimapheight > 0 && minimapheight < minimapcenter.z + minimapradius.z)
    {
        minimapping = 2;
        camera1->o.z = minimapcenter.z + minimapradius.z + 1;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glTranslatef(0, 0, 1);
        glOrtho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, 0, 2*zscale);
        glMatrixMode(GL_MODELVIEW);
        transplayer();
        readmatrices();
        rendergbuffer();
        shadegbuffer();
    }
        
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    vieww = oldvieww;
    viewh = oldviewh;
    ldrscale = oldldrscale;
    ldrscaleb = oldldrscaleb;

    camera1 = oldcamera;
    envmapping = false;
    minimapping = 0;

    if(hasCBF || !hdrfloat) glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(size, size);
    }

    glViewport(0, 0, vieww, viewh);

    glBindTexture(GL_TEXTURE_2D, minimaptex);
    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR_ARB, GL_TRUE);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, 0, 0, size, size, 0);
    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR_ARB, GL_FIXED_ONLY_ARB);
    setuptexparameters(minimaptex, NULL, 3, 1, GL_RGB5, GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    hdrclear = 3;

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void drawcubemap(int size, const vec &o, float yaw, float pitch, const cubemapside &side)
{
    envmapping = true;

    physent *oldcamera = camera1;
    static physent cmcamera;
    cmcamera = *player;
    cmcamera.reset();
    cmcamera.type = ENT_CAMERA;
    cmcamera.o = o;
    cmcamera.yaw = yaw;
    cmcamera.pitch = pitch;
    cmcamera.roll = 0;
    camera1 = &cmcamera;
    setviewcell(camera1->o);

    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
    int fogmat = lookupmaterial(vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin))&MATF_VOLUME, abovemat = MAT_AIR;
    float fogbelow = 0;
    if(fogmat==MAT_WATER || fogmat==MAT_LAVA)
    {
        float z = findsurface(fogmat, vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin), abovemat) - WATER_OFFSET;
        if(camera1->o.z < z + fogmargin)
        {
            fogbelow = z - camera1->o.z;
        }
        else fogmat = abovemat;
    }
    else fogmat = MAT_AIR;
    setfog(abovemat);
    
    float oldaspect = aspect, oldfovy = fovy, oldfov = curfov, oldldrscale = ldrscale, oldldrscaleb = ldrscaleb;
    int oldfarplane = farplane, oldvieww = vieww, oldviewh = viewh;
    curfov = fovy = 90;
    aspect = 1;
    farplane = worldsize*2;
    vieww = viewh = size;
    project(90.0f, 1.0f, farplane);

    transplayer();
    readmatrices();
    findorientation();

    resetlights();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    xtravertsva = xtraverts = glde = gbatches = 0;

    ldrscale = 1;
    ldrscaleb = ldrscale/255;
    GLOBALPARAM(ldrscale, (ldrscale));
    GLOBALPARAM(camera, (camera1->o.x, camera1->o.y, camera1->o.z, 1));
    GLOBALPARAM(millis, (lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f));

    visiblecubes();

    GLERROR;

    rendergbuffer();
    GLERROR;

    rendershadowatlas();
    GLERROR;

    shadegbuffer();
    GLERROR;

    if(fogmat)
    {
        setfog(fogmat, fogbelow, 1, abovemat);

        glActiveTexture_(GL_TEXTURE9_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
        glActiveTexture_(GL_TEXTURE0_ARB);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
        renderwaterfog(fogmat, fogbelow);
    
        setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    }

    rendertransparent();
    GLERROR;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    aspect = oldaspect;
    fovy = oldfovy;
    curfov = oldfov;
    farplane = oldfarplane;
    vieww = oldvieww;
    viewh = oldviewh;
    ldrscale = oldldrscale;
    ldrscaleb = oldldrscaleb;

    camera1 = oldcamera;
    envmapping = false;

    if(hasCBF) glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(size, size);
    }

    hdrclear = 3;
}

void gl_setupframe(int w, int h)
{
    setupgbuffer(w, h);
    if(hdr && (bloomw < 0 || bloomh < 0)) setupbloom(w, h);
    if(ao && (aow < 0 || aoh < 0)) setupao(w, h);
    if(smaa && !smaafbo[0]) setupsmaa();
    if(!shadowatlasfbo) setupshadowatlas();
    if(!deferredlightshader) loaddeferredlightshaders();
}

void gl_drawframe(int w, int h)
{
    timer_sync();

    defaultshader->set();

    vieww = w;
    viewh = h;
    aspect = w/float(h);
    fovy = 2*atan2(tan(curfov/2*RAD), aspect)/RAD;
    
    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
    int fogmat = lookupmaterial(vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin))&MATF_VOLUME, abovemat = MAT_AIR;
    float fogbelow = 0;
    if(fogmat==MAT_WATER || fogmat==MAT_LAVA)
    {
        float z = findsurface(fogmat, vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin), abovemat) - WATER_OFFSET;
        if(camera1->o.z < z + fogmargin)
        {
            fogbelow = z - camera1->o.z;
        }
        else fogmat = abovemat;
    }
    else fogmat = MAT_AIR;    
    setfog(abovemat);
    //setfog(fogmat, fogbelow, 1, abovemat);

    farplane = worldsize*2;

    resetlights();

    if(!debugcsm)
    {
        project(fovy, aspect, farplane);
        transplayer();
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        const int splitid = min(debugcsm-1, csmsplitn-1);
        glLoadMatrixf(csm.splits[splitid].proj.v);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(csm.model.v);
    }

    readmatrices();
    findorientation();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    xtravertsva = xtraverts = glde = gbatches = 0;

    ldrscale = hdr ? 0.5f : 1;
    ldrscaleb = ldrscale/255;
    GLOBALPARAM(ldrscale, (ldrscale));
    GLOBALPARAM(camera, (camera1->o.x, camera1->o.y, camera1->o.z, 1));
    GLOBALPARAM(millis, (lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f));

    visiblecubes();
  
    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 
    rendergbuffer();

    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else if(limitsky() && editmode) renderexplicitsky(true);

    if(ao) 
    {
        renderao();
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
        glViewport(0, 0, vieww, viewh);
        GLERROR;
    }
 
    // render grass after AO to avoid disturbing shimmering patterns
    generategrass();
    rendergrass();
    GLERROR;

    rendershadowatlas();
    GLERROR;

    shadegbuffer();
    GLERROR;

    if(fogmat)
    {
        setfog(fogmat, fogbelow, 1, abovemat);

        glActiveTexture_(GL_TEXTURE9_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
        glActiveTexture_(GL_TEXTURE0_ARB);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
        renderwaterfog(fogmat, fogbelow);

        setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    }

    rendertransparent();
    GLERROR;

    if(fogmat) setfog(fogmat, fogbelow, 1, abovemat);

    defaultshader->set();
    GLERROR;

    extern int outline;
    if(!wireframe && editmode && outline) renderoutline();
    GLERROR;

    rendereditmaterials();
    GLERROR;

    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    renderparticles(true);
    GLERROR;

    extern int hidehud;
    if(editmode && !hidehud)
    {
        glDepthMask(GL_FALSE);
        renderblendbrush();
        rendereditcursor();
        glDepthMask(GL_TRUE);
    }
        
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    processhdr(smaa ? smaafbo[0] : 0);
    if(smaa) dosmaa();

    addmotionblur();
    if(fogmat != MAT_AIR) drawfogoverlay(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    renderpostfx();

    defaultshader->set();
    g3d_render();

    notextureshader->set();

    gl_drawhud(w, h);

    timer_nextframe();
}

void gl_drawmainmenu(int w, int h)
{
    xtravertsva = xtraverts = glde = gbatches = 0;

    renderbackground(NULL, NULL, NULL, NULL, true, true);
    renderpostfx();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    defaultshader->set();
    g3d_render();

    notextureshader->set();

    gl_drawhud(w, h);
}

VARNP(damagecompass, usedamagecompass, 0, 1, 1);
VARP(damagecompassfade, 1, 1000, 10000);
VARP(damagecompasssize, 1, 30, 100);
VARP(damagecompassalpha, 1, 25, 100);
VARP(damagecompassmin, 1, 25, 1000);
VARP(damagecompassmax, 1, 200, 1000);

float dcompass[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
void damagecompass(int n, const vec &loc)
{
    if(!usedamagecompass) return;
    vec delta(loc);
    delta.sub(camera1->o); 
    float yaw, pitch;
    if(delta.magnitude()<4) yaw = camera1->yaw;
    else vectoyawpitch(delta, yaw, pitch);
    yaw -= camera1->yaw;
    if(yaw >= 360) yaw = fmod(yaw, 360);
    else if(yaw < 0) yaw = 360 - fmod(-yaw, 360);
    int dir = (int(yaw+22.5f)%360)/45;
    dcompass[dir] += max(n, damagecompassmin)/float(damagecompassmax);
    if(dcompass[dir]>1) dcompass[dir] = 1;

}
void drawdamagecompass(int w, int h)
{
    int dirs = 0;
    float size = damagecompasssize/100.0f*min(h, w)/2.0f;
    loopi(8) if(dcompass[i]>0)
    {
        if(!dirs)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1, 0, 0, damagecompassalpha/100.0f);
        }
        dirs++;

        glPushMatrix();
        glTranslatef(w/2, h/2, 0);
        glRotatef(i*45, 0, 0, 1);
        glTranslatef(0, -size/2.0f-min(h, w)/4.0f, 0);
        float logscale = 32,
              scale = log(1 + (logscale - 1)*dcompass[i]) / log(logscale);
        glScalef(size*scale, size*scale, 0);

        glBegin(GL_TRIANGLES);
        glVertex3f(1, 1, 0);
        glVertex3f(-1, 1, 0);
        glVertex3f(0, 0, 0);
        glEnd();
        glPopMatrix();

        // fade in log space so short blips don't disappear too quickly
        scale -= float(curtime)/damagecompassfade;
        dcompass[i] = scale > 0 ? (pow(logscale, scale) - 1) / (logscale - 1) : 0;
    }
}

int damageblendmillis = 0;

VARFP(damagescreen, 0, 1, 1, { if(!damagescreen) damageblendmillis = 0; });
VARP(damagescreenfactor, 1, 7, 100);
VARP(damagescreenalpha, 1, 45, 100);
VARP(damagescreenfade, 0, 125, 1000);
VARP(damagescreenmin, 1, 10, 1000);
VARP(damagescreenmax, 1, 100, 1000);

void damageblend(int n)
{
    if(!damagescreen) return;
    if(lastmillis > damageblendmillis) damageblendmillis = lastmillis;
    damageblendmillis += clamp(n, damagescreenmin, damagescreenmax)*damagescreenfactor;
}

void drawdamagescreen(int w, int h)
{
    if(lastmillis >= damageblendmillis) return;

    defaultshader->set();
    glEnable(GL_TEXTURE_2D);

    static Texture *damagetex = NULL;
    if(!damagetex) damagetex = textureload("packages/hud/damage.png", 3);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, damagetex->id);
    float fade = damagescreenalpha/100.0f;
    if(damageblendmillis - lastmillis < damagescreenfade)
        fade *= float(damageblendmillis - lastmillis)/damagescreenfade;
    glColor4f(fade, fade, fade, fade);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(w, 0);
    glTexCoord2f(0, 1); glVertex2f(0, h);
    glTexCoord2f(1, 1); glVertex2f(w, h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    notextureshader->set();
}

VAR(hidestats, 0, 0, 1);
VAR(hidehud, 0, 0, 1);

VARP(crosshairsize, 0, 15, 50);
VARP(cursorsize, 0, 30, 50);
VARP(crosshairfx, 0, 1, 1);

#define MAXCROSSHAIRS 4
static Texture *crosshairs[MAXCROSSHAIRS] = { NULL, NULL, NULL, NULL };

void loadcrosshair(const char *name, int i)
{
    if(i < 0 || i >= MAXCROSSHAIRS) return;
	crosshairs[i] = name ? textureload(name, 3, true) : notexture;
    if(crosshairs[i] == notexture) 
    {
        name = game::defaultcrosshair(i);
        if(!name) name = "data/crosshair.png";
        crosshairs[i] = textureload(name, 3, true);
    }
}

void loadcrosshair_(const char *name, int *i)
{
	loadcrosshair(name, *i);
}

COMMANDN(loadcrosshair, loadcrosshair_, "si");

ICOMMAND(getcrosshair, "i", (int *i), 
{
    const char *name = "";
    if(*i >= 0 && *i < MAXCROSSHAIRS)
    {
        name = crosshairs[*i] ? crosshairs[*i]->name : game::defaultcrosshair(*i);
        if(!name) name = "data/crosshair.png";
    }
    result(name);
});
 
void writecrosshairs(stream *f)
{
    loopi(MAXCROSSHAIRS) if(crosshairs[i] && crosshairs[i]!=notexture)
        f->printf("loadcrosshair %s %d\n", escapestring(crosshairs[i]->name), i);
    f->printf("\n");
}

void drawcrosshair(int w, int h)
{
    bool windowhit = g3d_windowhit(true, false);
    if(!windowhit && (hidehud || mainmenu)) return; //(hidehud || player->state==CS_SPECTATOR || player->state==CS_DEAD)) return;

    float r = 1, g = 1, b = 1, cx = 0.5f, cy = 0.5f, chsize;
    Texture *crosshair;
    if(windowhit)
    {
        static Texture *cursor = NULL;
        if(!cursor) cursor = textureload("data/guicursor.png", 3, true);
        crosshair = cursor;
        chsize = cursorsize*w/900.0f;
        g3d_cursorpos(cx, cy);
    }
    else
    { 
        int index = game::selectcrosshair(r, g, b);
        if(index < 0) return;
        if(!crosshairfx)
        {
            index = 0;
            r = g = b = 1;
        }
        crosshair = crosshairs[index];
        if(!crosshair) 
        {
            loadcrosshair(NULL, index);
            crosshair = crosshairs[index];
        }
        chsize = crosshairsize*w/900.0f;
    }
    if(crosshair->type&Texture::ALPHA) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else glBlendFunc(GL_ONE, GL_ONE);
    glColor3f(r, g, b);
    float x = cx*w - (windowhit ? 0 : chsize/2.0f);
    float y = cy*h - (windowhit ? 0 : chsize/2.0f);
    glBindTexture(GL_TEXTURE_2D, crosshair->id);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2f(x,          y);
    glTexCoord2f(1, 0); glVertex2f(x + chsize, y);
    glTexCoord2f(0, 1); glVertex2f(x,          y + chsize);
    glTexCoord2f(1, 1); glVertex2f(x + chsize, y + chsize);
    glEnd();
}

VARP(wallclock, 0, 0, 1);
VARP(wallclock24, 0, 0, 1);
VARP(wallclocksecs, 0, 0, 1);

static time_t walltime = 0;

VARP(showfps, 0, 1, 1);
VARP(showfpsrange, 0, 0, 1);
VAR(showeditstats, 0, 0, 1);
VAR(statrate, 1, 200, 1000);

FVARP(conscale, 1e-3f, 0.33f, 1e3f);

void gl_drawhud(int w, int h)
{
    gettextres(w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glColor3f(1, 1, 1);

    if(debugshadowatlas) viewshadowatlas();
    else if(debugbuffersplit) viewbuffersplitmerge();
    else if(debugao) viewao(); 
    else if(debugdepth) viewdepth();
    else if(debugrefract) viewrefract();
    else if(debugsmaa) viewsmaa();

    glEnable(GL_BLEND);
    
    if(!mainmenu)
    {
        drawdamagescreen(w, h);
        drawdamagecompass(w, h);
    }

    glEnable(GL_TEXTURE_2D);
    defaultshader->set();

    int conw = int(w/conscale), conh = int(h/conscale), abovehud = conh - FONTH, limitgui = abovehud;
    if(!hidehud && !mainmenu)
    {
        if(!hidestats)
        {
            glPushMatrix();
            glScalef(conscale, conscale, 1);

            int roffset = 0;
            if(showfps)
            {
                static int lastfps = 0, prevfps[3] = { 0, 0, 0 }, curfps[3] = { 0, 0, 0 };
                if(totalmillis - lastfps >= statrate)
                {
                    memcpy(prevfps, curfps, sizeof(prevfps));
                    lastfps = totalmillis - (totalmillis%statrate);
                }
                int nextfps[3];
                getfps(nextfps[0], nextfps[1], nextfps[2]);
                loopi(3) if(prevfps[i]==curfps[i]) curfps[i] = nextfps[i];
                if(showfpsrange) draw_textf("fps %d+%d-%d", conw-7*FONTH, conh-FONTH*3/2, curfps[0], curfps[1], curfps[2]);
                else draw_textf("fps %d", conw-5*FONTH, conh-FONTH*3/2, curfps[0]);
                roffset += FONTH;
            }

            timer_print(conw, conh);
            frametimer_print(conw, conh);

            if(wallclock)
            {
                if(!walltime) { walltime = time(NULL); walltime -= totalmillis/1000; if(!walltime) walltime++; }
                time_t walloffset = walltime + totalmillis/1000;
                struct tm *localvals = localtime(&walloffset);
                static string buf;
                if(localvals && strftime(buf, sizeof(buf), wallclocksecs ? (wallclock24 ? "%H:%M:%S" : "%I:%M:%S%p") : (wallclock24 ? "%H:%M" : "%I:%M%p"), localvals))
                {
                    // hack because not all platforms (windows) support %P lowercase option
                    // also strip leading 0 from 12 hour time
                    char *dst = buf;
                    const char *src = &buf[!wallclock24 && buf[0]=='0' ? 1 : 0];
                    while(*src) *dst++ = tolower(*src++);
                    *dst++ = '\0'; 
                    draw_text(buf, conw-5*FONTH, conh-FONTH*3/2-roffset);
                    roffset += FONTH;
                }
            }
                       
            if(editmode || showeditstats)
            {
                static int laststats = 0, prevstats[7] = { 0, 0, 0, 0, 0, 0, 0 }, curstats[7] = { 0, 0, 0, 0, 0, 0, 0 };
                if(totalmillis - laststats >= statrate)
                {
                    memcpy(prevstats, curstats, sizeof(prevstats));
                    laststats = totalmillis - (totalmillis%statrate);
                }
                int nextstats[7] =
                {
                    vtris*100/max(wtris, 1),
                    vverts*100/max(wverts, 1),
                    xtraverts/1024,
                    xtravertsva/1024,
                    glde,
                    gbatches,
                    getnumqueries()
                };
                loopi(7) if(prevstats[i]==curstats[i]) curstats[i] = nextstats[i];

                abovehud -= 2*FONTH;
                draw_textf("wtr:%dk(%d%%) wvt:%dk(%d%%) evt:%dk eva:%dk", FONTH/2, abovehud, wtris/1024, curstats[0], wverts/1024, curstats[1], curstats[2], curstats[3]);
                draw_textf("ond:%d va:%d gl:%d(%d) oq:%d pvs:%d", FONTH/2, abovehud+FONTH, allocnodes*8, allocva, curstats[4], curstats[5], curstats[6], getnumviewcells());
                limitgui = abovehud;
            }

            if(editmode)
            {
                abovehud -= FONTH;
                draw_textf("cube %s%d", FONTH/2, abovehud, selchildcount<0 ? "1/" : "", abs(selchildcount));

                char *editinfo = executestr("edithud");
                if(editinfo)
                {
                    if(editinfo[0])
                    {
                        int tw, th;
                        text_bounds(editinfo, tw, th);
                        th += FONTH-1; th -= th%FONTH;
                        abovehud -= max(th, FONTH);
                        draw_text(editinfo, FONTH/2, abovehud);
                    }
                    DELETEA(editinfo);
                }
            }
            else if(identexists("gamehud"))
            {
                char *gameinfo = executestr("gamehud");
                if(gameinfo)
                {
                    if(gameinfo[0])
                    {
                        int tw, th;
                        text_bounds(gameinfo, tw, th);
                        th += FONTH-1; th -= th%FONTH;
                        roffset += max(th, FONTH);    
                        draw_text(gameinfo, conw-max(5*FONTH, 2*FONTH+tw), conh-FONTH/2-roffset);
                    }
                    DELETEA(gameinfo);
                }
            } 
            
            glPopMatrix();
        }

        if(hidestats || (!editmode && !showeditstats))
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            game::gameplayhud(w, h);
            limitgui = abovehud = min(abovehud, int(conh*game::abovegameplayhud(w, h)));
        }

        rendertexturepanel(w, h);
    }
    
    g3d_limitscale((2*limitgui - conh) / float(conh));

    glPushMatrix();
    glScalef(conscale, conscale, 1);
    abovehud -= rendercommand(FONTH/2, abovehud - FONTH/2, conw-FONTH);
    extern int fullconsole;
    if(!hidehud || fullconsole) renderconsole(conw, conh, abovehud - FONTH/2);
    glPopMatrix();

    drawcrosshair(w, h);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}


