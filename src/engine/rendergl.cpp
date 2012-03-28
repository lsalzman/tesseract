// rendergl.cpp: core opengl rendering stuff

#include "engine.h"

bool hasVBO = false, hasDRE = false, hasOQ = false, hasTR = false, hasFBO = false, hasDS = false, hasTF = false, hasBE = false, hasBC = false, hasCM = false, hasNP2 = false, hasTC = false, hasMT = false, hasAF = false, hasMDA = false, hasGLSL = false, hasGM = false, hasNVFB = false, hasSGIDT = false, hasSGISH = false, hasDT = false, hasSH = false, hasNVPCF = false, hasPBO = false, hasFBB = false, hasUBO = false, hasBUE = false, hasDB = false, hasTG = false, hasT4 = false, hasTQ = false;

#define BEN_TEST_SPLITTING 1

int hasstencil = 0;

VAR(renderpath, 1, 0, 0);

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

// GL_ARB_timer_query
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v_  = NULL;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v_ = NULL;
PFNGLQUERYCOUNTERPROC glQueryCounter_ = NULL;

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

void *getprocaddress(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

VARP(ati_skybox_bug, 0, 0, 1);
VAR(ati_oq_bug, 0, 0, 1);
VAR(ati_minmax_bug, 0, 0, 1);
VAR(ati_cubemap_bug, 0, 0, 1);
VAR(ati_ubo_bug, 0, 0, 1);
VAR(nvidia_scissor_bug, 0, 0, 1);
VAR(sdl_backingstore_bug, -1, 0, 1);
VAR(minimizetcusage, 1, 0, 0);
VAR(usetexrect, 1, 0, 0);
VAR(useubo, 1, 0, 0);
VAR(usebue, 1, 0, 0);
VAR(rtscissor, 0, 1, 1);
VAR(blurtile, 0, 1, 1);
VAR(rtsharefb, 0, 1, 1);
VAR(usetexgather, 1, 0, 0);

static bool checkseries(const char *s, int low, int high)
{
    while(*s && !isdigit(*s)) ++s;
    if(!*s) return false;
    int n = 0;
    while(isdigit(*s)) n = n*10 + (*s++ - '0');    
    return n >= low && n < high;
}

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

    bool mesa = false, intel = false, ati = false, nvidia = false;
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

    if(hasext(exts, "GL_ARB_timer_query"))
    {
        glGetQueryiv_ =          (PFNGLGETQUERYIVARBPROC)       getprocaddress("glGetQueryivARB");
        glGenQueries_ =          (PFNGLGENQUERIESARBPROC)       getprocaddress("glGenQueriesARB");
        glDeleteQueries_ =       (PFNGLDELETEQUERIESARBPROC)    getprocaddress("glDeleteQueriesARB");
        glBeginQuery_ =          (PFNGLBEGINQUERYARBPROC)       getprocaddress("glBeginQueryARB");
        glEndQuery_ =            (PFNGLENDQUERYARBPROC)         getprocaddress("glEndQueryARB");
        glGetQueryObjectiv_ =    (PFNGLGETQUERYOBJECTIVARBPROC) getprocaddress("glGetQueryObjectivARB");
        glGetQueryObjectuiv_ =   (PFNGLGETQUERYOBJECTUIVARBPROC)getprocaddress("glGetQueryObjectuivARB");
        glGetQueryObjecti64v_ =  (PFNGLGETQUERYOBJECTI64VPROC)  getprocaddress("glGetQueryObjecti64v");
        glGetQueryObjectui64v_ = (PFNGLGETQUERYOBJECTUI64VPROC) getprocaddress("glGetQueryObjectui64v");
        glQueryCounter_ =        (PFNGLQUERYCOUNTERPROC)        getprocaddress("glQueryCounter");
        hasTQ = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_timer_query extension.");
    }

    extern int batchlightmaps, fpdepthfx;
    if(ati)
    {
        //conoutf(CON_WARN, "WARNING: ATI cards may show garbage in skybox. (use \"/ati_skybox_bug 1\" to fix)");
        minimizetcusage = 1;
		if(hasTF && hasNVFB) fpdepthfx = 1;
    }
    else if(nvidia)
    {
        reservevpparams = 10;
        rtsharefb = 0; // work-around for strange driver stalls involving when using many FBOs
        extern int filltjoints;
        if(!hasext(exts, "GL_EXT_gpu_shader4")) filltjoints = 0; // DX9 or less NV cards seem to not cause many sparklies
        
        if(hasFBO && !hasTF) nvidia_scissor_bug = 1; // 5200 bug, clearing with scissor on an FBO messes up on reflections, may affect lesser cards too 
        extern int fpdepthfx;
        if(hasTF && (!strstr(renderer, "GeForce") || !checkseries(renderer, 6000, 6600)))
            fpdepthfx = 1; // FP filtering causes software fallback on 6200?
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
                batchlightmaps = 0;
            }
        }

        reservevpparams = 20;

        if(!hasOQ) waterrefract = 0;
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
        glBindAttribLocation_ =         (PFNGLBINDATTRIBLOCATIONARBPROC)      getprocaddress("glBindAttribLocationARB");
        glGetActiveUniform_ =           (PFNGLGETACTIVEUNIFORMARBPROC)        getprocaddress("glGetActiveUniformARB");
        glEnableVertexAttribArray_ =    (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) getprocaddress("glEnableVertexAttribArrayARB");
        glDisableVertexAttribArray_ =   (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)getprocaddress("glDisableVertexAttribArrayARB");
        glVertexAttribPointer_ =        (PFNGLVERTEXATTRIBPOINTERARBPROC)     getprocaddress("glVertexAttribPointerARB");

        extern bool checkglslsupport();
        if(checkglslsupport()) hasGLSL = true;
    }
    if(!hasGLSL) fatal("GLSL support is required!");
 
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

    extern int usenp2;
    if(hasext(exts, "GL_ARB_texture_non_power_of_two"))
    {
        hasNP2 = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_non_power_of_two extension.");
    }
    else if(usenp2) conoutf(CON_WARN, "WARNING: Non-power-of-two textures not supported!");

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
        hasT4 = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_AMD_texture_texture4 extension.");
    }
    if(hasTG || hasT4) usetexgather = 1;

    if(hasext(exts, "GL_EXT_gpu_shader4"))
    {
        // on DX10 or above class cards (i.e. GF8 or RadeonHD) enable expensive features
        extern int grass, depthfxsize, depthfxrect, depthfxfilter, blurdepthfx;
        grass = 1;
        if(hasOQ)
        {
            waterfallrefract = 1;
            if(hasTR)
            {
                depthfxsize = 10;
                depthfxrect = 1;
                depthfxfilter = 0;
                blurdepthfx = 0;
            }
        }
    }
}

void glext(char *ext)
{
    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    intret(hasext(exts, ext) ? 1 : 0);
}
COMMAND(glext, "s");

#define CHECKERROR(body) do { body; GLenum error = glGetError(); if(error != GL_NO_ERROR) {printf("%s:%d:%x: %s\n", __FILE__, __LINE__, error, #body); }} while(0)

// GPU-side timing information will use OGL timers
enum {TIMER_SM=0u, TIMER_GBUFFER, TIMER_SHADING, TIMER_SPLITTING, TIMER_MERGING, TIMER_N};
static const char *timer_string[] = {
    "shadow map",
    "gbuffer",
    "deferred shading",
    "buffer splitting",
    "buffer merging"
};
static const int timer_query_n = 4;
static GLuint timers[timer_query_n][TIMER_N];
static bool timer_used[timer_query_n][TIMER_N];
static int timer_curr = 0;
static int timer_last_query = -1;
VAR(gputimer, 0, 0, 1);

static void timer_sync()
{
    if(timer_curr - timer_query_n < 0) return;
    if(!gputimer || !hasTQ) return;
    const int curr = timer_curr % timer_query_n;
    loopi(TIMER_N)
    {
        GLint available = 0;
        if(timer_used[curr][i])
            while(!available)
                glGetQueryObjectiv_(timers[curr][i], GL_QUERY_RESULT_AVAILABLE, &available);
    }
}
static inline void timer_begin(int whichone)
{
    if(!gputimer || !hasTQ) return;
    glBeginQuery_(GL_TIME_ELAPSED, timers[timer_curr % timer_query_n][whichone]);
    timer_last_query = whichone;
}
static inline void timer_end()
{
    if(!gputimer || !hasTQ) return;
    assert(timer_last_query != -1);
    timer_used[timer_curr % timer_query_n][timer_last_query] = true;
    glEndQuery_(GL_TIME_ELAPSED);
}
static inline void timer_nextframe()
{
    if (gputimer && hasTQ) timer_curr++;
}
static void timer_print(int conw, int conh)
{
    if(!gputimer || !hasTQ) return;
    if(timer_curr - timer_query_n < 0) return;
    const int curr = timer_curr % timer_query_n;
    loopi(TIMER_N)
    {
        if(!timer_used[curr][i]) continue;
        GLuint64 elapsed;
        glGetQueryObjectui64v_(timers[curr][i], GL_QUERY_RESULT, &elapsed);
        draw_textf("%s %3.2f ms", conw-30*FONTH, conh-FONTH*3/2-i*9*FONTH/8, timer_string[i], float(elapsed) * 1e-6f);
    }
}
static void timer_setup() {
    if(!hasTQ) return;
    loopi(timer_query_n) glGenQueries_(TIMER_N, timers[i]);
    loopi(timer_query_n) loopj(TIMER_N) timer_used[i][j] = false;
}
static void cleanuptimer() { if(hasTQ) loopi(timer_query_n) glDeleteQueries_(TIMER_N, timers[i]); }

void gl_init(int w, int h, int bpp, int depth, int fsaa)
{
    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 0);
    glClearDepth(1);
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
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

    setuptexcompress();
    timer_setup();
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

    extern void cleanupshadowatlas();
    cleanupshadowatlas();

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
bool isthirdperson() { return player!=camera1 || detachedcamera || reflecting; }

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
glmatrixf mvmatrix, projmatrix, mvpmatrix, invmvmatrix, invmvpmatrix;

void readmatrices()
{
    glGetFloatv(GL_MODELVIEW_MATRIX, mvmatrix.v);
    glGetFloatv(GL_PROJECTION_MATRIX, projmatrix.v);

    mvpmatrix.mul(projmatrix, mvmatrix);
    invmvmatrix.invert(mvmatrix);
    invmvpmatrix.invert(mvpmatrix);
}

FVAR(nearplane, 1e-3f, 0.54f, 1e3f);

void project(float fovy, float aspect, int farplane, bool flipx = false, bool flipy = false, bool swapxy = false, float zscale = 1)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(swapxy) glRotatef(90, 0, 0, 1);
    if(flipx || flipy!=swapxy || zscale!=1) glScalef(flipx ? -1 : 1, flipy!=swapxy ? -1 : 1, zscale);
    GLdouble ydist = nearplane * tan(fovy/2*RAD), xdist = ydist * aspect;
    glFrustum(-xdist, xdist, -ydist, ydist, nearplane, farplane);
    glMatrixMode(GL_MODELVIEW);
}

static const uint maxsplitn = 16;
VAR(csmsplitn, 1, 3, maxsplitn);
FVAR(csmsplitweight, 0.20f, 0.75f, 0.95f);

// The csm code is partly taken from the nVidia OpenGL SDK
struct frustum
{
    float near, far, fov, ratio;
    vec point[8];
};

static void updatefrustumpoints(frustum &f, vec &center, vec &viewdir)
{
    vec up(0.0f, 1.0f, 0.0f);
    vec right; right.cross(viewdir,up);

    vec fc = center + viewdir*f.far;
    vec nc = center + viewdir*f.near;

    right.normalize();
    up.cross(right, viewdir);
    up.normalize();

    float near_height = tan(f.fov/2.0f) * f.near;
    float near_width = near_height * f.ratio;
    float far_height = tan(f.fov/2.0f) * f.far;
    float far_width = far_height * f.ratio;

    f.point[0] = nc - up*near_height - right*near_width;
    f.point[1] = nc + up*near_height - right*near_width;
    f.point[2] = nc + up*near_height + right*near_width;
    f.point[3] = nc - up*near_height + right*near_width;
    f.point[4] = fc - up*far_height - right*far_width;
    f.point[5] = fc + up*far_height - right*far_width;
    f.point[6] = fc + up*far_height + right*far_width;
    f.point[7] = fc - up*far_height + right*far_width;
}

static void updatesplitdist(frustum *f, float nd, float fd)
{
    float lambda = csmsplitweight;
    float ratio = fd/nd;
    f[0].near = nd;

    loopi(csmsplitn)
    {
        float si = i / (float)csmsplitn;
        f[i].near = lambda*(nd*powf(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
        f[i-1].far = f[i].near * 1.005f;
    }
    f[csmsplitn-1].far = fd;
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

glmatrixf clipmatrix;

static const glmatrixf dummymatrix;
static int projectioncount = 0;
void pushprojection(const glmatrixf &m = dummymatrix)
{
    glMatrixMode(GL_PROJECTION);
    if(projectioncount <= 0) glPushMatrix();
    if(&m != &dummymatrix) glLoadMatrixf(m.v);
    if(fogging)
    {
        glMultMatrixf(mvmatrix.v);
        glMultMatrixf(invfogmatrix.v);
    }
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
        if(fogging)
        {
            glMultMatrixf(mvmatrix.v);
            glMultMatrixf(invfogmatrix.v);
        }
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
    
    bool clipped = reflectz < 1e15f && reflectclip;

    glmatrixf offsetmatrix = clipped ? clipmatrix : projmatrix;
    offsetmatrix[14] += depthoffset * projmatrix[10];

    glMatrixMode(GL_PROJECTION);
    if(!clipped) glPushMatrix();
    glLoadMatrixf(offsetmatrix.v);
    if(fogging)
    {
        glMultMatrixf(mvmatrix.v);
        glMultMatrixf(invfogmatrix.v);
    }
    glMatrixMode(GL_MODELVIEW);
}

void disablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glDisable(type);
        return;
    }
    
    bool clipped = reflectz < 1e15f && reflectclip;

    glMatrixMode(GL_PROJECTION);
    if(clipped) 
    {
        glLoadMatrixf(clipmatrix.v);
        if(fogging)
        {
            glMultMatrixf(mvmatrix.v);
            glMultMatrixf(invfogmatrix.v);
        }
    }
    else glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2)
{
    vec worldpos(center);
    if(reflecting) worldpos.z = 2*reflectz - worldpos.z; 
    vec e(mvmatrix.transformx(worldpos),
          mvmatrix.transformy(worldpos),
          mvmatrix.transformz(worldpos));
    if(e.z > 2*size) { sx1 = sy1 = 1; sx2 = sy2 = -1; return; }
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

glmatrixf envmatrix;

void setenvmatrix()
{
    envmatrix = fogging ? fogmatrix : mvmatrix;
    if(reflecting) envmatrix.reflectz(reflectz);
    envmatrix.transpose();
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

static void blendfog(int fogmat, float blend, float logblend, float &start, float &end, float *fogc)
{
    switch(fogmat)
    {
        case MAT_WATER:
            loopk(3) fogc[k] += blend*watercolor[k]/255.0f;
            end += logblend*min(fog, max(waterfog*4, 32));
            break;

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

static void setfog(int fogmat, float below = 1, int abovemat = MAT_AIR)
{
    float fogc[4] = { 0, 0, 0, 1 };
    float start = 0, end = 0;
    float logscale = 256, logblend = log(1 + (logscale - 1)*below) / log(logscale);

    blendfog(fogmat, below, logblend, start, end, fogc);
    if(below < 1) blendfog(abovemat, 1-below, 1-logblend, start, end, fogc);

    glFogf(GL_FOG_START, start);
    glFogf(GL_FOG_END, end);
    glFogfv(GL_FOG_COLOR, fogc);
    glClearColor(fogc[0], fogc[1], fogc[2], 1.0f);
}

static void blendfogoverlay(int fogmat, float blend, float *overlay)
{
    float maxc;
    switch(fogmat)
    {
        case MAT_WATER:
            maxc = max(watercolor[0], max(watercolor[1], watercolor[2]));
            loopk(3) overlay[k] += blend*max(0.4f, watercolor[k]/min(32.0f + maxc*7.0f/8.0f, 255.0f));
            break;

        case MAT_LAVA:
            maxc = max(lavacolor[0], max(lavacolor[1], lavacolor[2]));
            loopk(3) overlay[k] += blend*max(0.4f, lavacolor[k]/min(32.0f + maxc*7.0f/8.0f, 255.0f));
            break;

        default:
            loopk(3) overlay[k] += blend;
            break;
    }
}

void drawfogoverlay(int fogmat, float fogblend, int abovemat)
{
    notextureshader->set();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    float overlay[3] = { 0, 0, 0 };
    blendfogoverlay(fogmat, fogblend, overlay);
    blendfogoverlay(abovemat, 1-fogblend, overlay);

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

bool renderedgame = false;

void rendergame(bool mainpass)
{
    game::rendergame(mainpass);
    if(!shadowmapping) renderedgame = true;
}

VARP(reflectmms, 0, 1, 1);
VARR(refractsky, 0, 0, 1);

glmatrixf fogmatrix, invfogmatrix;

void drawreflection(float z, bool refract)
{
    reflectz = z < 0 ? 1e16f : z;
    reflecting = !refract;
    refracting = refract ? (z < 0 || camera1->o.z >= z ? -1 : 1) : 0;
    fading = waterrefract && waterfade && hasFBO && z>=0;
    fogging = refracting<0 && z>=0;

    float oldfogstart, oldfogend, oldfogcolor[4];
    glGetFloatv(GL_FOG_START, &oldfogstart);
    glGetFloatv(GL_FOG_END, &oldfogend);
    glGetFloatv(GL_FOG_COLOR, oldfogcolor);

    if(fogging)
    {
        glFogf(GL_FOG_START, camera1->o.z - z);
        glFogf(GL_FOG_END, camera1->o.z - (z-waterfog));
        GLfloat m[16] =
        {
             1,   0,  0, 0,
             0,   1,  0, 0,
             0,   0,  1, 0,
            -camera1->o.x, -camera1->o.y, -camera1->o.z, 1
        };
        memcpy(fogmatrix.v, m, sizeof(m));
        invfogmatrix.invert(fogmatrix);
        pushprojection();
        glPushMatrix();
        glLoadMatrixf(fogmatrix.v);
        float fogc[4] = { watercolor.x/255.0f, watercolor.y/255.0f, watercolor.z/255.0f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogc);
    }
    else
    {
        glFogf(GL_FOG_START, (fog+64)/8);
        glFogf(GL_FOG_END, fog);
        float fogc[4] = { fogcolor.x/255.0f, fogcolor.y/255.0f, fogcolor.z/255.0f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogc);
    }

    if(fading)
    {
        float scale = fogging ? -0.25f : 0.25f, offset = 2*fabs(scale) - scale*z;
        setenvparamf("waterfadeparams", SHPARAM_VERTEX, 8, scale, offset, -scale, offset + camera1->o.z*scale);
        setenvparamf("waterfadeparams", SHPARAM_PIXEL, 8, scale, offset, -scale, offset + camera1->o.z*scale);
    }

    if(reflecting)
    {
        glPushMatrix();
        glTranslatef(0, 0, 2*z);
        glScalef(1, 1, -1);

        glFrontFace(GL_CCW);
    }

    setenvmatrix();

    if(reflectclip && z>=0)
    {
        float zoffset = reflectclip/4.0f, zclip;
        if(refracting<0)
        {
            zclip = z+zoffset;
            if(camera1->o.z<=zclip) zclip = z;
        }
        else
        {
            zclip = z-zoffset;
            if(camera1->o.z>=zclip && camera1->o.z<=z+4.0f) zclip = z;
            if(reflecting) zclip = 2*z - zclip;
        }
        plane clipplane;
        invmvmatrix.transposedtransform(plane(0, 0, refracting>0 ? 1 : -1, refracting>0 ? -zclip : zclip), clipplane);
        clipmatrix.clip(clipplane, projmatrix);
        pushprojection(clipmatrix);
    }

    renderreflectedgeom(refracting<0 && z>=0 && caustics, fogging);

    if(reflecting || refracting>0 || (refracting<0 && refractsky) || z<0)
    {
        if(fading) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        if(reflectclip && z>=0) popprojection();
        if(fogging) 
        {
            popprojection();
            glPopMatrix();
        }
        drawskybox(farplane, false);
        if(fogging) 
        {
            pushprojection();
            glPushMatrix();
            glLoadMatrixf(fogmatrix.v);
        }
        if(reflectclip && z>=0) pushprojection(clipmatrix);
        if(fading) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    }
    else if(fading) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

    renderdecals();

    if(reflectmms) renderreflectedmapmodels();
    rendergame();

    if(refracting && z>=0 && !isthirdperson() && fabs(camera1->o.z-z) <= 0.5f*(player->eyeheight + player->aboveeye))
    {   
        glmatrixf avatarproj;
        avatarproj.perspective(curavatarfov, aspect, nearplane, farplane);
        if(reflectclip)
        {
            popprojection();
            glmatrixf avatarclip;
            plane clipplane;
            invmvmatrix.transposedtransform(plane(0, 0, refracting, reflectclipavatar/4.0f - refracting*z), clipplane);
            avatarclip.clip(clipplane, avatarproj);
            pushprojection(avatarclip);
        }
        else pushprojection(avatarproj);
        game::renderavatar();
        popprojection();
        if(reflectclip) pushprojection(clipmatrix);
    }

    if(refracting) rendergrass();
    rendermaterials();
    renderalphageom(fogging);
    renderparticles();

    if(fading) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if(reflectclip && z>=0) popprojection();

    if(reflecting)
    {
        glPopMatrix();

        glFrontFace(GL_CW);
    }

    if(fogging) 
    {
        popprojection();
        glPopMatrix();
    }
    glFogf(GL_FOG_START, oldfogstart);
    glFogf(GL_FOG_END, oldfogend);
    glFogfv(GL_FOG_COLOR, oldfogcolor);
    
    reflectz = 1e16f;
    refracting = 0;
    reflecting = fading = fogging = false;

    setenvmatrix();
}

bool envmapping = false;

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
   
    defaultshader->set();

    int fogmat = lookupmaterial(o)&MATF_VOLUME;
    if(fogmat!=MAT_WATER && fogmat!=MAT_LAVA) fogmat = MAT_AIR;

    setfog(fogmat);

    glClear(GL_DEPTH_BUFFER_BIT);

    int farplane = worldsize*2;

    project(90.0f, 1.0f, farplane, !side.flipx, !side.flipy, side.swapxy);

    transplayer();
    readmatrices();
    findorientation();
    setenvmatrix();

    glEnable(GL_FOG);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    xtravertsva = xtraverts = glde = gbatches = 0;

    visiblecubes();

    if(limitsky()) drawskybox(farplane, true);

    rendergeom();

    if(!limitsky()) drawskybox(farplane, false);

//    queryreflections();

    rendermapmodels();
    renderalphageom();

//    drawreflections();

//    renderwater();
//    rendermaterials();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FOG);

    camera1 = oldcamera;
    envmapping = false;
}

bool minimapping = false;

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

void drawminimap()
{
    if(!game::needminimap()) { clearminimap(); return; }

    renderprogress(0, "generating mini-map...", 0, !renderedframe);

    int size = 1<<minimapsize, sizelimit = min(hwtexsize, min(screen->w, screen->h));
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

    envmapping = minimapping = true;

    physent *oldcamera = camera1;
    static physent cmcamera;
    cmcamera = *player;
    cmcamera.reset();
    cmcamera.type = ENT_CAMERA;
    cmcamera.o = vec(minimapcenter.x, minimapcenter.y, max(minimapcenter.z + minimapradius.z + 1, float(minimapheight)));
    cmcamera.yaw = 0;
    cmcamera.pitch = -90;
    cmcamera.roll = 0;
    camera1 = &cmcamera;
    setviewcell(vec(-1, -1, -1));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, 0, camera1->o.z + 1);
    glScalef(-1, 1, 1);
    glMatrixMode(GL_MODELVIEW);

    transplayer();

    defaultshader->set();

    GLfloat fogc[4] = { minimapcolor.x/255.0f, minimapcolor.y/255.0f, minimapcolor.z/255.0f, 1.0f };
    glFogf(GL_FOG_START, 0);
    glFogf(GL_FOG_END, 1000000);
    glFogfv(GL_FOG_COLOR, fogc);

    glClearColor(fogc[0], fogc[1], fogc[2], fogc[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glViewport(1, 1, size-2, size-2);
    glScissor(1, 1, size-2, size-2);
    glEnable(GL_SCISSOR_TEST);

    glDisable(GL_FOG);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glFrontFace(GL_CCW);

    xtravertsva = xtraverts = glde = gbatches = 0;

    visiblecubes(false);
    queryreflections();
    drawreflections();

    loopi(minimapheight > 0 && minimapheight < minimapcenter.z + minimapradius.z ? 2 : 1)
    {
        if(i)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            camera1->o.z = minimapheight;
            transplayer();
        }
        rendergeom();
        rendermapmodels();
        renderwater();
        rendermaterials();
        renderalphageom();
    }

    glFrontFace(GL_CW);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FOG);

    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, screen->w, screen->h);

    camera1 = oldcamera;
    envmapping = minimapping = false;

    glBindTexture(GL_TEXTURE_2D, minimaptex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, 0, 0, size, size, 0);
    setuptexparameters(minimaptex, NULL, 3, 1, GL_RGB5, GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
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

int gw = -1, gh = -1, bloomw = -1, bloomh = -1;
GLuint gfbo = 0, gdepthtex = 0, gcolortex = 0, gnormaltex = 0, gglowtex = 0, hdrfbo = 0, hdrtex = 0, bloomfbo[5] = { 0, 0, 0, 0, 0 }, bloomtex[5] = { 0, 0, 0, 0, 0 };

extern int bloomsize, bloomhdr;

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

    GLenum format = bloomhdr ? GL_RGB10 : GL_RGB;
    createtexture(bloomtex[0], max(gw/2, bloomw), max(gh/2, bloomh), NULL, 3, 1, format, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[1], max(gw/4, bloomw), max(gh/4, bloomh), NULL, 3, 1, format, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[2], bloomw, bloomh, NULL, 3, 1, format, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[3], bloomw, bloomh, NULL, 3, 1, format, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[4], 1, 1, NULL, 3, 1, GL_RGB16, GL_TEXTURE_RECTANGLE_ARB);

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

    static const GLenum drawbufs[3] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
    glDrawBuffers_(3, drawbufs);

    createtexture(gdepthtex, gw, gh, NULL, 3, 0, GL_DEPTH_COMPONENT24, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gcolortex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gnormaltex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gglowtex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
 
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, gcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, gnormaltex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_RECTANGLE_ARB, gglowtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) 
        fatal("failed allocating g-buffer!");

    if(!hdrtex) glGenTextures(1, &hdrtex);

    if(!hdrfbo) glGenFramebuffers_(1, &hdrfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);

    createtexture(hdrtex, gw, gh, NULL, 3, 1, GL_RGB10, GL_TEXTURE_RECTANGLE_ARB);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, hdrtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating HDR buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    cleanupbloom();
}

void cleanupgbuffer()
{
    if(gfbo) { glDeleteFramebuffers_(1, &gfbo); gfbo = 0; }
    if(gdepthtex) { glDeleteTextures(1, &gdepthtex); gdepthtex = 0; }
    if(gcolortex) { glDeleteTextures(1, &gcolortex); gcolortex = 0; }
    if(gnormaltex) { glDeleteTextures(1, &gnormaltex); gnormaltex = 0; }
    if(gglowtex) { glDeleteTextures(1, &gglowtex); gglowtex = 0; }
    if(hdrfbo) { glDeleteFramebuffers_(1, &hdrfbo); hdrfbo = 0; }
    if(hdrtex) { glDeleteTextures(1, &hdrtex); hdrtex = 0; }
    gw = gh = -1;
}

VAR(hdr, 0, 1, 1);
FVAR(hdrscale, 0.1f, 4.0f, 10.0f);
FVAR(bloomthreshold, 0, 0.75f, 10.0f);
FVAR(bloomscale, 0, 2.0f, 10.0f);
FVAR(bloomprec, 0.1f, 1.0f, 10.0f);
VAR(bloomreduce, 0, 1, 1);
VAR(bloomblit, 0, 1, 1);
VAR(bloomblur, 0, 7, 7);
VAR(bloomiter, 0, 0, 4);
FVAR(bloomsigma, 0.005f, 0.5f, 2.0f);
VARF(bloomsize, 6, 8, 10, cleanupbloom());
VARF(bloomhdr, 0, 1, 1, cleanupbloom());
VAR(bloominit, 0, 1, 1);
FVAR(hdraccumscale, 0, 0.98f, 1);
VAR(hdraccummillis, 1, 33, 1000);
FVAR(hdrtonemin, 0, 0.5f, 1000); 
FVAR(hdrtonemax, 0, 1.5f, 1000);

void bloomquad(int sw, int sh)
{
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(sw, 0), glVertex2f(1, -1);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(sw, sh); glVertex2f(1, 1);
    glTexCoord2f(0, sh); glVertex2f(-1, 1);
    glEnd();
}

#define SHADOWATLAS_SIZE 4096

PackNode shadowatlaspacker(0, 0, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE);

GLuint shadowatlastex = 0, shadowatlasfbo = 0;

void setupshadowatlas()
{
    if(!shadowatlastex) glGenTextures(1, &shadowatlastex);

    createtexture(shadowatlastex, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE, NULL, 3, 1, GL_DEPTH_COMPONENT16, GL_TEXTURE_2D);
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

struct lighttile
{
    float sx1, sy1, sx2, sy2;
    extentity *ent;
    int shadowmap;
};

struct shadowmapinfo
{
    ushort x, y, size;
    extentity *ent;
    occludequery *query;
};

FVAR(smpolyfactor, -1e3, 0, 1e3);
FVAR(smpolyoffset, -1e3, 0, 1e3);
FVAR(smbias, -1e3, 0, 1e3);
FVAR(smprec, 1e-3f, 1, 1e3);
FVAR(smtetraprec, 1e-3f, SQRT3, 1e3);
FVAR(smcubeprec, 1e-3f, 1, 1e3);

#define LIGHTTILE_W 10
#define LIGHTTILE_H 10

VAR(smsidecull, 0, 1, 1);
VAR(smviscull, 0, 1, 1);
VAR(smborder, 0, 4, 16);
VAR(smminradius, 0, 16, 10000);
VAR(smminsize, 1, 96, 1024);
VAR(smmaxsize, 1, 384, 1024);
VAR(smused, 1, 0, 0);
VAR(smquery, 0, 1, 1);
VAR(smtetra, 0, 0, 1);
VAR(smtetraclip, 0, 1, 1);
FVAR(smtetraborder, 0, 0, 1e3);
VAR(smcullside, 0, 0, 1);
VAR(smgather, 0, 0, 1);
VAR(smnoshadow, 0, 0, 2);

bool shadowmapping = false;

plane smtetraclipplane;

static inline void setsmgathermode() // use texture gather
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static inline void setsmnongathermode() // use embedded shadow cmp
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
    if(!usegatherforsm()) setsmgathermode(); // "normal" mode basically
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2i(0, 0);
    glTexCoord2f(1, 0); glVertex2i(w, 0);
    glTexCoord2f(0, 1); glVertex2i(0, h);
    glTexCoord2f(1, 1); glVertex2i(w, h);
    glEnd();
    if(!usegatherforsm()) setsmnongathermode(); // "gather" mode basically
    notextureshader->set();
}
VAR(debugshadowatlas, 0, 0, 1);

static int playing_around_with_buffer_splitting;
void viewbuffersplitmerge()
{
    const int w = screen->w, h = screen->h;
#if 1
    Shader *splitshader = lookupshaderbyname("buffersplit");
    splitshader->set();
    const float split[] = {4.f, 4.f};
    const float tiledim[] = {float(w)/split[0], float(h)/split[1]};
    setlocalparamf("tiledim", SHPARAM_PIXEL, 0, tiledim[0], tiledim[1]);
    setlocalparamf("rcptiledim", SHPARAM_PIXEL, 2, 1.f/tiledim[0], 1.f/tiledim[1]);
    setlocalparamf("split", SHPARAM_PIXEL, 4, split[0], split[1]);
    timer_begin(TIMER_SPLITTING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,        0);        glVertex2i(-w, -h);
    glTexCoord2f(float(w), 0);        glVertex2i(w, -h);
    glTexCoord2f(0,        float(h)); glVertex2i(-w, h);
    glTexCoord2f(float(w), float(h)); glVertex2i(w, h);
    glEnd();
    timer_end();
    notextureshader->set();
#else
    Shader *mergeshader = lookupshaderbyname("buffermerge");
    mergeshader->set();
    const float split[] = {4.f, 4.f};
    const float tiledim[] = {float(w)/split[0], float(h)/split[1]};
    setlocalparamf("tiledim", SHPARAM_PIXEL, 0, tiledim[0], tiledim[1]);
    setlocalparamf("split", SHPARAM_PIXEL, 2, split[0], split[1]);
    setlocalparamf("rcpsplit", SHPARAM_PIXEL, 4, 1.f / split[0], 1.f / split[1]);
    timer_begin(TIMER_MERGING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,        0);        glVertex2i(-w, -h);
    glTexCoord2f(float(w), 0);        glVertex2i(w, -h);
    glTexCoord2f(0,        float(h)); glVertex2i(-w, h);
    glTexCoord2f(float(w), float(h)); glVertex2i(w, h);
    glEnd();
    timer_end();
    notextureshader->set();

#endif
}
VAR(debugbuffersplit, 0, 0, 1);

static int playsing_around_with_timer_queries_here;

void gl_drawframe(int w, int h)
{
    timer_sync();

    setupgbuffer(w, h);
    if(!bloomfbo[0] || !bloomfbo[1]) setupbloom(w, h);
    if(!shadowatlasfbo) setupshadowatlas();

    defaultshader->set();

    aspect = w/float(h);
    fovy = 2*atan2(tan(curfov/2*RAD), aspect)/RAD;
    
    int fogmat = lookupmaterial(camera1->o)&MATF_VOLUME, abovemat = MAT_AIR;
    float fogblend = 1.0f, causticspass = 0.0f;
    if(fogmat==MAT_WATER || fogmat==MAT_LAVA)
    {
        float z = findsurface(fogmat, camera1->o, abovemat) - WATER_OFFSET;
        if(camera1->o.z < z + 1) fogblend = min(z + 1 - camera1->o.z, 1.0f);
        else fogmat = abovemat;
        if(caustics && fogmat==MAT_WATER && camera1->o.z < z)
            causticspass = min(z - camera1->o.z, 1.0f);
    }
    else fogmat = MAT_AIR;    
    setfog(fogmat, fogblend, abovemat);
    if(fogmat!=MAT_AIR)
    {
        float blend = abovemat==MAT_AIR ? fogblend : 1.0f;
        fovy += blend*sinf(lastmillis/1000.0)*2.0f;
        aspect += blend*sinf(lastmillis/1000.0+PI)*0.1f;
    }

    farplane = worldsize*2;

    project(fovy, aspect, farplane);
    transplayer();
    readmatrices();
    findorientation();
    setenvmatrix();

    glEnable(GL_FOG);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    xtravertsva = xtraverts = glde = gbatches = 0;

    if(!hasFBO)
    {
        if(dopostfx)
        {
#if 0
            drawglaretex();
            drawdepthfxtex();
            drawreflections();
#endif
        }
        else dopostfx = true;
    }

    visiblecubes();
    
    glClear(GL_DEPTH_BUFFER_BIT|(wireframe && editmode ? GL_COLOR_BUFFER_BIT : 0)|(hasstencil ? GL_STENCIL_BUFFER_BIT : 0));

    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

    if(limitsky()) drawskybox(farplane, true);

    int deferred_weirdness_starts_here;

    // just temporarily render world geometry into the g-buffer so we can slowly grow the g-buffers tendrils into the rendering pipeline

    timer_begin(TIMER_GBUFFER);
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);

    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

    rendergeom(causticspass);
    rendermapmodels();
    rendergame(true);
    timer_end();

    int deferred_weirdness_ends_here;
    
    //extern int outline;
    //if(!wireframe && editmode && outline) renderoutline();

    //queryreflections();

    //generategrass();

    //if(!limitsky()) drawskybox(farplane, false);

#ifndef MORE_DEFERRED_WEIRDNESS
    timer_begin(TIMER_SM);

    vector<lighttile> tiles[LIGHTTILE_H][LIGHTTILE_W];
    vector<shadowmapinfo> shadowmaps;
 
    shadowatlaspacker.reset();
  
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    nocolorshader->set();

    smused = 0;
    const vector<extentity *> &ents = entities::getents();
    loopv(ents)
    {
        extentity *l = ents[i];
        if(l->type != ET_LIGHT) continue;

        float sx1 = -1, sy1 = -1, sx2 = 1, sy2 = 1;
        if(l->attr1 > 0) 
        {
            if(smviscull)
            {
                if(isvisiblesphere(l->attr1, l->o) >= VFC_NOT_VISIBLE) continue;
                ivec bo = ivec(l->o).sub(l->attr1), br = ivec(l->attr1*2+1, l->attr1*2+1, l->attr1*2+1);
                if(pvsoccluded(bo, br) || bboccluded(bo, br)) continue;
            }
            calcspherescissor(l->o, l->attr1, sx1, sy1, sx2, sy2);
        }
        if(sx1 >= sx2 || sy1 >= sy2) { sx1 = sy1 = -1; sx2 = sy2 = 1; }

        int smradius = l->attr1 > 0 ? l->attr1 : 2*worldsize;
        float smlod = clamp(smradius * smprec / sqrtf(max(1.0f, camera1->o.dist(l->o)/smradius)), float(smminsize), float(smmaxsize));
        int smsize = clamp(int(ceil(smlod * (smtetra ? smtetraprec : smcubeprec))), 1, SHADOWATLAS_SIZE / (smtetra ? 2 : 3)),
            smw = smtetra ? smsize*2 : smsize*3, smh = smtetra ? smsize : smsize*2;
        ushort smx = USHRT_MAX, smy = USHRT_MAX;
        shadowmapinfo *sm = NULL;
        int smidx = -1;
        if(smnoshadow <= 1 && smradius > smminradius && shadowatlaspacker.insert(smx, smy, smw, smh))
        {
            smidx = shadowmaps.length();
            sm = &shadowmaps.add();
            sm->x = smx;
            sm->y = smy;
            sm->size = smsize;
            sm->ent = l;
            sm->query = NULL;
            if(smquery && l->attr1 > 0)
            {
                ivec bo = ivec(l->o).sub(l->attr1), br(l->attr1*2+1, l->attr1*2+1, l->attr1*2+1);
                if(camera1->o.x < bo.x || camera1->o.x > bo.x + br.x ||
                   camera1->o.y < bo.y || camera1->o.y > bo.y + br.y ||
                   camera1->o.z < bo.z || camera1->o.z > bo.z + br.z)
                { 
                    sm->query = newquery(l);
                    if(sm->query)
                    {
                        startquery(sm->query);
                        drawbb(bo, br);
                        endquery(sm->query);
                    }
                }
            }
            smused += smsize*smsize*(smtetra ? 2 : 6);
        }
        
        int tx1 = max(int(floor((sx1 + 1)*0.5f*LIGHTTILE_W)), 0), ty1 = max(int(floor((sy1 + 1)*0.5f*LIGHTTILE_H)), 0),
            tx2 = min(int(ceil((sx2 + 1)*0.5f*LIGHTTILE_W)), LIGHTTILE_W), ty2 = min(int(ceil((sy2 + 1)*0.5f*LIGHTTILE_H)), LIGHTTILE_H); 
        for(int y = ty1; y < ty2; y++)
        {
            for(int x = tx1; x < tx2; x++)
            {
                lighttile &t = tiles[y][x].add();
                t.sx1 = sx1;
                t.sy1 = sy1;
                t.sx2 = sx2;
                t.sy2 = sy2;
                t.ent = l;
                t.shadowmap = smidx;
            }
        }
    }

    glDepthMask(GL_TRUE);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);

    glEnable(GL_SCISSOR_TEST);

    if(smpolyfactor || smpolyoffset)
    {
        glPolygonOffset(smpolyfactor, smpolyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    if(smtetra && smtetraclip) glEnable(GL_CLIP_PLANE0);

    shadowmapping = true;

    loopv(shadowmaps)
    {
        shadowmapinfo &sm = shadowmaps[i];
        extentity *l = sm.ent;

        int smradius = l->attr1 > 0 ? l->attr1 : 2*worldsize, sidemask;
        if(smtetra)
        {
            extern int cullfrustumtetra(const vec &lightpos, float lightradius, float size, float border);
            sidemask = smsidecull ? cullfrustumtetra(l->o, smradius, sm.size, smborder) : 0xF;
        }
        else
        {
            extern int cullfrustumsides(const vec &lightpos, float lightradius, float size, float border);
            sidemask = smsidecull ? cullfrustumsides(l->o, smradius, sm.size, smborder) : 0x3F;
        }
        if(!sidemask || (sm.query && sm.query->owner == l && checkquery(sm.query))) continue;

        float smnearclip = SQRT3 / smradius, smfarclip = SQRT3;
        GLfloat smprojmatrix[16] =
        {
            float(sm.size - smborder) / sm.size, 0, 0, 0,
            0, float(sm.size - smborder) / sm.size, 0, 0,
            0, 0, -(smfarclip + smnearclip) / (smfarclip - smnearclip), -1,
            0, 0, -2*smnearclip*smfarclip / (smfarclip - smnearclip), 0
        };
        GLfloat lightmatrix[16] =
        {
            1.0f/smradius, 0, 0, 0,
            0, 1.0f/smradius, 0, 0,
            0, 0, 1.0f/smradius, 0,
            -l->o.x/smradius, -l->o.y/smradius, -l->o.z/smradius, 1
        };

        shadoworigin = l->o;
        shadowradius = smradius;
        shadowbias = smborder / float(sm.size - smborder);

        findshadowvas();
        findshadowmms();

        lockmodelbatches();
        extern void rendershadowmapmodels();
        rendershadowmapmodels();
        rendergame();
        unlockmodelbatches();

        if(smtetra)
        {
            int smw = sm.size*2, smh = sm.size;
            glScissor(sm.x + (sidemask & 0x3 ? 0 : sm.size), sm.y, smw + (sidemask & 0x3 && sidemask & 0xC ? 2*sm.size : sm.size), smh);
            glClear(GL_DEPTH_BUFFER_BIT);

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(smprojmatrix);
            glMatrixMode(GL_MODELVIEW);

            glmatrixf smviewmatrix;
            loop(side, 4) if(sidemask&(1<<side))
            {
                int sidex = (side>>1)*sm.size;
                if(!(side&1) || !(sidemask&(1<<(side-1))))
                {
                    glViewport(sm.x + sidex, sm.y, sm.size, sm.size);
                    glScissor(sm.x + sidex, sm.y, sm.size, sm.size);
                }

                smviewmatrix.mul(tetrashadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side>>1) ^ smcullside ? GL_BACK : GL_FRONT);
   
                if(smtetraclip)
                {
                    smtetraclipplane.toplane(vec(-smviewmatrix.v[2], -smviewmatrix.v[6], 0), l->o);
                    smtetraclipplane.offset += smtetraborder/(0.5f*sm.size);
                    setenvparamf("tetraclip", SHPARAM_VERTEX, 1, smtetraclipplane.x, smtetraclipplane.y, smtetraclipplane.z, smtetraclipplane.offset);
                }

                shadowside = side;

                extern void rendershadowmapworld();
                rendershadowmapworld();
                rendermodelbatches();
            }

            continue; 
        }

        int cx1 = sidemask & 0x03 ? 0 : (sidemask & 0xC ? sm.size : 2 * sm.size),
            cx2 = sidemask & 0x30 ? 3 * sm.size : (sidemask & 0xC ? 2 * sm.size : sm.size),
            cy1 = sidemask & 0x15 ? 0 : sm.size,
            cy2 = sidemask & 0x2A ? 2 * sm.size : sm.size;
        glScissor(sm.x + cx1, sm.y + cy1, cx2 - cx1, cy2 - cy1);
        glClear(GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(smprojmatrix);
        glMatrixMode(GL_MODELVIEW);

        glmatrixf smviewmatrix;
        loop(side, 6) if(sidemask&(1<<side))
        {
            int sidex = (side>>1)*sm.size, sidey = (side&1)*sm.size;
            glViewport(sm.x + sidex, sm.y + sidey, sm.size, sm.size);
            glScissor(sm.x + sidex, sm.y + sidey, sm.size, sm.size);

            smviewmatrix.mul(cubeshadowviewmatrix[side], lightmatrix);
            glLoadMatrixf(smviewmatrix.v);

            glCullFace((side & 1) ^ (side >> 2) ^ smcullside ? GL_FRONT : GL_BACK);

            shadowside = side;

            extern void rendershadowmapworld();
            rendershadowmapworld();
            rendermodelbatches();
        }   
    }
    timer_end();

    shadowmapping = false;

    if(smtetra && smtetraclip) glDisable(GL_CLIP_PLANE0);

    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glViewport(0, 0, w, h);

    if(smpolyfactor || smpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_BACK);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdr ? hdrfbo : 0);

    CHECKERROR();
    timer_begin(TIMER_SHADING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE4_ARB);
    glBindTexture(GL_TEXTURE_2D, shadowatlastex);
    if(usegatherforsm()) setsmgathermode(); else setsmnongathermode();
    glActiveTexture_(GL_TEXTURE0_ARB);

    glMatrixMode(GL_TEXTURE);
    glmatrixf screenmatrix, worldmatrix;
    screenmatrix.identity();
    screenmatrix.scale(2.0f/w, 2.0f/h, 2.0f);
    screenmatrix.translate(-1.0f, -1.0f, -1.0f);
    worldmatrix.mul(invmvpmatrix, screenmatrix);
    glLoadMatrixf(worldmatrix.v);
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);

    static Shader *deferredlightshader = NULL, *deferredshadowshader = NULL;
    if(!deferredlightshader) deferredlightshader = lookupshaderbyname("deferredlight");
    if(!deferredshadowshader) deferredshadowshader = lookupshaderbyname("deferredshadow");

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);

    setenvparamf("camera", SHPARAM_PIXEL, 0, camera1->o.x, camera1->o.y, camera1->o.z);
    setenvparamf("shadowatlasscale", SHPARAM_PIXEL, 1, 1.0f/SHADOWATLAS_SIZE, 1.0f/SHADOWATLAS_SIZE);

#if BEN_TEST_SPLITTING == 0 // Lee's original code ggoe
    loop(y, LIGHTTILE_H) loop(x, LIGHTTILE_W)
    {
        vector<lighttile> &lights = tiles[y][x];
        
        static const char * const lightpos[] = { "light0pos", "light1pos", "light2pos", "light3pos", "light4pos", "light5pos", "light6pos", "light7pos" };
        static const char * const lightcolor[] = { "light0color", "light1color", "light2color", "light3color", "light4color", "light5color", "light6color", "light7color" };
        static const char * const shadowparams[] = { "shadow0params", "shadow1params", "shadow2params", "shadow3params", "shadow4params", "shadow5params", "shadow6params", "shadow7params" };
        static const char * const shadowoffset[] = { "shadow0offset", "shadow1offset", "shadow2offset", "shadow3offset", "shadow4offset", "shadow5offset", "shadow6offset", "shadow7offset" };

        for(int i = 0;;)
        {
            int n = min(lights.length() - i, 7);

            bool shadowmap = n > 0 && lights[i].shadowmap >= 0;
            loopj(n)
            {
                if((lights[i+j].shadowmap >= 0) != shadowmap) { n = j; break; }
            }
            
            if(shadowmap && !smnoshadow) deferredshadowshader->setvariant(n-1, (smtetra ? 1 : 0) + (smgather && (hasTG || hasT4) ? 2 : 0));
            else if(n > 0) deferredlightshader->setvariant(n-1, 0);
            else deferredlightshader->set();

            float lightscale = (hdr ? 1/hdrscale : 1)/255.0f;
            if(!i)
            {
                extern bvec ambientcolor;
                setlocalparamf("lightscale", SHPARAM_PIXEL, 2, ambientcolor.x*lightscale, ambientcolor.y*lightscale, ambientcolor.z*lightscale, 255*lightscale);
            }
            else
            {
                setlocalparamf("lightscale", SHPARAM_PIXEL, 2, 0, 0, 0, 0);
            }

            float sx1 = 1, sy1 = 1, sx2 = -1, sy2 = -1;
            loopj(n)
            {
                lighttile &t = lights[i+j];
                extentity *l = t.ent;
                setlocalparamf(lightpos[j], SHPARAM_PIXEL, 3 + 4*j, l->o.x, l->o.y, l->o.z, l->attr1 > 0 ? 1.0f/l->attr1 : 0.5f/worldsize);
                setlocalparamf(lightcolor[j], SHPARAM_PIXEL, 4 + 4*j, l->attr2*lightscale, l->attr3*lightscale, l->attr4*lightscale);
                if(shadowmap)
                {
                    shadowmapinfo &sm = shadowmaps[lights[i+j].shadowmap];
                    int smradius = l->attr1 > 0 ? l->attr1 : 2*worldsize;
                    float smnearclip = SQRT3 / smradius, smfarclip = SQRT3, 
                          bias = smbias * smnearclip * (1024.0f / sm.size);
                    setlocalparamf(shadowparams[j], SHPARAM_PIXEL, 5 + 4*j, 
                        0.5f * (sm.size - smborder), 
                        -smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias, 
                        sm.size, 
                        0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                    setlocalparamf(shadowoffset[j], SHPARAM_PIXEL, 6 + 4*j, sm.x + 0.5f*sm.size, sm.y + 0.5f*sm.size);
                }
                sx1 = min(sx1, t.sx1);
                sy1 = min(sy1, t.sy1);
                sx2 = max(sx2, t.sx2);
                sy2 = max(sy2, t.sy2);
            }
            if(!i || sx1 >= sx2 || sy1 >= sy2) { sx1 = sy1 = -1; sx2 = sy2 = 1; }

            int tx1 = max(int(floor((sx1*0.5f+0.5f)*w)), (x*w)/LIGHTTILE_W), ty1 = max(int(floor((sy1*0.5f+0.5f)*h)), (y*h)/LIGHTTILE_H),
                tx2 = min(int(ceil((sx2*0.5f+0.5f)*w)), ((x+1)*w)/LIGHTTILE_W), ty2 = min(int(ceil((sy2*0.5f+0.5f)*h)), ((y+1)*h)/LIGHTTILE_H);

            // FIXME: use depth bounds here in addition to scissor
            glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

            // FIXME: render light geometry here
            glBegin(GL_TRIANGLE_STRIP);
            glVertex2f( 1, -1);
            glVertex2f(-1, -1);
            glVertex2f( 1,  1);
            glVertex2f(-1,  1);
            glEnd();
    
            i += n;
            if(i >= lights.length()) break;
        }
    }
#else


#endif

    glDisable(GL_SCISSOR_TEST);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    timer_end();

    defaultshader->set();
    CHECKERROR();

#endif

    renderdecals(true);

    if(!isthirdperson())
    {
        project(curavatarfov, aspect, farplane, false, false, false, avatardepth);
        game::renderavatar();
        project(fovy, aspect, farplane);
    }

    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#if 0
    if(hasFBO) 
    {
        drawglaretex();
        drawdepthfxtex();
        drawreflections();
    }
#endif

    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //renderwater();
    //rendergrass();

    //rendermaterials();
    //renderalphageom();

    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    renderparticles(true);

    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

#ifndef HDR_STUFF
    if(hdr)
    {
        GLuint b0fbo = bloomfbo[0], b0tex = bloomtex[0], b1fbo =  bloomfbo[1], b1tex = bloomtex[1];
        int b0w = bloomreduce ? max(gw/2, bloomw) : bloomw, b0h = bloomreduce ? max(gh/2, bloomh) : bloomh, b1w = max(gw/4, bloomw), b1h = max(gh/4, bloomh),
            pw = bloomreduce ? b0w : bloomw, ph = bloomreduce ? b0h : bloomh;
        CHECKERROR();
        bool reduced = false;
        if(bloomreduce && (b1w < b0w || b1h < b0h))
        {
            if(hasFBB && bloomblit)
            {
                glBindFramebuffer_(GL_READ_FRAMEBUFFER_EXT, hdrfbo);
                glBindFramebuffer_(GL_DRAW_FRAMEBUFFER_EXT, b0fbo);
                glBlitFramebuffer_(0, 0, gw, gh, 0, 0, pw, ph, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            }
            else
            {
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b0fbo);
                glViewport(0, 0, pw, ph);
                SETSHADER(bloomreduce);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
                bloomquad(gw, gh);
            }
            reduced = true;
        }

        if(bloomreduce) for(;; reduced = true)
        {
            int cw = max(pw/2, bloomw), ch = max(ph/2, bloomh);
            if(cw == bloomw && ch == bloomh) break; 
            if(hasFBB && bloomblit)
            {
                glBindFramebuffer_(GL_READ_FRAMEBUFFER_EXT, b0fbo);
                glBindFramebuffer_(GL_DRAW_FRAMEBUFFER_EXT, b1fbo);
                glBlitFramebuffer_(0, 0, pw, ph, 0, 0, cw, ch, GL_COLOR_BUFFER_BIT, GL_LINEAR); 
            }
            else
            {
                glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
                glViewport(0, 0, cw, ch);
                SETSHADER(bloomreduce);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
                bloomquad(pw, ph);
            }
            pw = cw;
            ph = ch;
            swap(b0fbo, b1fbo);
            swap(b0tex, b1tex);
            swap(b0w, b1w);
            swap(b0h, b1h);
        }
  
        GLuint h0fbo = b0fbo, h0tex = b0tex, h1fbo = b1fbo, h1tex = b1tex;
        int h0w = b0w, h0h = b0h, h1w = b1w, h1h = b1h;

        b1fbo = bloomfbo[2];
        b1tex = bloomtex[2];
        b1w = bloomw;
        b1h = bloomh;
 
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
        glViewport(0, 0, b1w, b1h);
        SETSHADER(bloominit);
        setlocalparamf("bloomparams", SHPARAM_PIXEL, 0, hdrscale*bloomprec, bloomthreshold*bloomprec);
        if(reduced)
        {
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
            bloomquad(b0w, b0h);
        }
        else
        {
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
            bloomquad(gw, gh);
        }
        b0fbo = b1fbo;
        b0tex = b1tex;
        b0w = b1w;
        b0h = b1h;
        b1fbo = bloomfbo[3];
        b1tex = bloomtex[3];
        b1w = bloomw;
        b1h = bloomh;

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
                bloomquad(b0w, b0h);
                swap(b0w, b1w);
                swap(b0h, b1h);
                swap(b0tex, b1tex);
                swap(b0fbo, b1fbo);
            }  
        }

        
        static int lastaccum = 0;
        if(!lastaccum || lastmillis - lastaccum >= hdraccummillis)
        {
            while(pw > 2 || ph > 2) 
            {
                int cw = max(pw/2, 2), ch = max(ph/2, 2);
                if(hasFBB && bloomblit)
                {
                    glBindFramebuffer_(GL_READ_FRAMEBUFFER_EXT, h0fbo);
                    glBindFramebuffer_(GL_DRAW_FRAMEBUFFER_EXT, h1fbo);
                    glBlitFramebuffer_(0, 0, pw, ph, 0, 0, cw, ch, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                }
                else
                {
                    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, h1fbo);
                    glViewport(0, 0, cw, ch);
                    SETSHADER(bloomreduce);
                    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, h0tex);
                    bloomquad(pw, ph);
                }
                pw = cw;
                ph = ch;
                swap(h0fbo, h1fbo);
                swap(h0tex, h1tex);
                swap(h0w, h1w);
                swap(h0h, h1h);
            }

            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, bloomfbo[4]);
            glViewport(0, 0, 1, 1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
            SETSHADER(hdraccum);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, h0tex);
            setlocalparamf("accumscale", SHPARAM_PIXEL, 0, lastaccum ? pow(hdraccumscale, float(lastmillis - lastaccum)/hdraccummillis) : 0);
            bloomquad(2, 2);
            glDisable(GL_BLEND);

            lastaccum = lastmillis;
        }

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        glViewport(0, 0, w, h);
        SETSHADER(hdrbloom);
        setlocalparamf("bloomsize", SHPARAM_VERTEX, 0, b0w, b0h);
        setlocalparamf("hdrparams", SHPARAM_PIXEL, 1, hdrscale, bloomscale/bloomprec, hdrtonemin, hdrtonemax);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        glActiveTexture(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
        glActiveTexture(GL_TEXTURE2_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomtex[4]);
        glActiveTexture(GL_TEXTURE0_ARB); 
        bloomquad(gw, gh);
    }
#endif

    //addmotionblur();
    //addglare();
    if(fogmat==MAT_WATER || fogmat==MAT_LAVA) drawfogoverlay(fogmat, fogblend, abovemat);
    //renderpostfx();

    defaultshader->set();
    g3d_render();

    glDisable(GL_TEXTURE_2D);
    notextureshader->set();

    gl_drawhud(w, h);

    renderedgame = false;
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
    glEnable(GL_TEXTURE_2D);
    g3d_render();

    notextureshader->set();
    glDisable(GL_TEXTURE_2D);

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
    if(editmode && !hidehud && !mainmenu)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        renderblendbrush();

        rendereditcursor();

        glDepthMask(GL_TRUE);
        glDisable(GL_DEPTH_TEST);
    }

    gettextres(w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glColor3f(1, 1, 1);

    extern int debugdepthfx;
    if(debugdepthfx)
    {
        extern void viewdepthfxtex();
        viewdepthfxtex();
    }

    if(debugshadowatlas) viewshadowatlas();
    else if(debugbuffersplit) viewbuffersplitmerge();

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

            if(gputimer) timer_print(conw, conh);

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
                static int laststats = 0, prevstats[8] = { 0, 0, 0, 0, 0, 0, 0 }, curstats[8] = { 0, 0, 0, 0, 0, 0, 0 };
                if(totalmillis - laststats >= statrate)
                {
                    memcpy(prevstats, curstats, sizeof(prevstats));
                    laststats = totalmillis - (totalmillis%statrate);
                }
                int nextstats[8] =
                {
                    vtris*100/max(wtris, 1),
                    vverts*100/max(wverts, 1),
                    xtraverts/1024,
                    xtravertsva/1024,
                    glde,
                    gbatches,
                    getnumqueries(),
                    rplanes
                };
                loopi(8) if(prevstats[i]==curstats[i]) curstats[i] = nextstats[i];

                abovehud -= 2*FONTH;
                draw_textf("wtr:%dk(%d%%) wvt:%dk(%d%%) evt:%dk eva:%dk", FONTH/2, abovehud, wtris/1024, curstats[0], wverts/1024, curstats[1], curstats[2], curstats[3]);
                draw_textf("ond:%d va:%d gl:%d(%d) oq:%d lm:%d rp:%d pvs:%d", FONTH/2, abovehud+FONTH, allocnodes*8, allocva, curstats[4], curstats[5], curstats[6], lightmaps.length(), curstats[7], getnumviewcells());
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


