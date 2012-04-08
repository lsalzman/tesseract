// GL_ARB_shading_language_100, GL_ARB_shader_objects, GL_ARB_fragment_shader, GL_ARB_vertex_shader
extern PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgramObject_;
extern PFNGLDELETEOBJECTARBPROC         glDeleteObject_;
extern PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgramObject_;
extern PFNGLCREATESHADEROBJECTARBPROC   glCreateShaderObject_;
extern PFNGLSHADERSOURCEARBPROC         glShaderSource_;
extern PFNGLCOMPILESHADERARBPROC        glCompileShader_;
extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv_;
extern PFNGLATTACHOBJECTARBPROC         glAttachObject_;
extern PFNGLGETINFOLOGARBPROC           glGetInfoLog_;
extern PFNGLLINKPROGRAMARBPROC          glLinkProgram_;
extern PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocation_;
extern PFNGLUNIFORM1FARBPROC            glUniform1f_;
extern PFNGLUNIFORM2FARBPROC            glUniform2f_;
extern PFNGLUNIFORM3FARBPROC            glUniform3f_;
extern PFNGLUNIFORM4FARBPROC            glUniform4f_;
extern PFNGLUNIFORM1FVARBPROC           glUniform1fv_;
extern PFNGLUNIFORM2FVARBPROC           glUniform2fv_;
extern PFNGLUNIFORM3FVARBPROC           glUniform3fv_;
extern PFNGLUNIFORM4FVARBPROC           glUniform4fv_;
extern PFNGLUNIFORM1IARBPROC            glUniform1i_;
extern PFNGLUNIFORMMATRIX2FVARBPROC     glUniformMatrix2fv_;
extern PFNGLUNIFORMMATRIX3FVARBPROC     glUniformMatrix3fv_;
extern PFNGLUNIFORMMATRIX4FVARBPROC     glUniformMatrix4fv_;
extern PFNGLBINDATTRIBLOCATIONARBPROC   glBindAttribLocation_;
extern PFNGLGETACTIVEUNIFORMARBPROC     glGetActiveUniform_;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArray_;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray_;
extern PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointer_;

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

extern int renderpath;

enum { R_GLSLANG = 0 };

enum { SHPARAM_LOOKUP = 0, SHPARAM_VERTEX, SHPARAM_PIXEL, SHPARAM_UNIFORM };

struct GlobalShaderParamState
{
    const char *name;
    float val[16];
    int version;

    static int nextversion;

    void resetversions();

    void changed()
    {
        if(++nextversion < 0) resetversions();
        version = nextversion;
    }
};

struct ShaderParamBinding
{
    int loc;
    GLenum format;
};

struct GlobalShaderParamUse : ShaderParamBinding
{

    GlobalShaderParamState *param;
    int version;

    void flush()
    {
        if(version == param->version) return;
        switch(format)
        {
            case GL_FLOAT:          glUniform1fv_(loc, 1, param->val); break;
            case GL_FLOAT_VEC2_ARB: glUniform2fv_(loc, 1, param->val); break;
            case GL_FLOAT_VEC3_ARB: glUniform3fv_(loc, 1, param->val); break;
            case GL_FLOAT_VEC4_ARB: glUniform4fv_(loc, 1, param->val); break;
            case GL_FLOAT_MAT2_ARB: glUniformMatrix2fv_(loc, 1, GL_TRUE, param->val); break;
            case GL_FLOAT_MAT3_ARB: glUniformMatrix3fv_(loc, 1, GL_TRUE, param->val); break;
            case GL_FLOAT_MAT4_ARB: glUniformMatrix4fv_(loc, 1, GL_FALSE, param->val); break;
        }
        version = param->version;
    }
};

struct LocalShaderParamState : ShaderParamBinding
{
    const char *name;
};

struct SlotShaderParam
{
    const char *name;
    int loc;
    float val[4];
};

struct SlotShaderParamState : LocalShaderParamState
{
    float val[4];

    SlotShaderParamState() {}
    SlotShaderParamState(const SlotShaderParam &p) 
    { 
        name = p.name; 
        loc = -1; 
        format = GL_FLOAT_VEC4_ARB; 
        memcpy(val, p.val, sizeof(val)); 
    }
};

enum 
{ 
    SHADER_DEFAULT    = 0, 
    SHADER_NORMALSLMS = 1<<0, 
    SHADER_ENVMAP     = 1<<1,
    SHADER_OPTION     = 1<<3,

    SHADER_INVALID    = 1<<8,
    SHADER_DEFERRED   = 1<<9
};

#define MAXSHADERDETAIL 3
#define MAXVARIANTROWS 5

extern int shaderdetail;

struct Slot;
struct VSlot;

struct UniformLoc
{
    const char *name, *blockname;
    int loc, version, binding, stride, offset, size;
    void *data;
    UniformLoc(const char *name = NULL, const char *blockname = NULL, int binding = -1, int stride = -1) : name(name), blockname(blockname), loc(-1), version(-1), binding(binding), stride(stride), offset(-1), size(-1), data(NULL) {}
};

struct AttribLoc
{
    const char *name;
    int loc;
    AttribLoc(const char *name = NULL, int loc = -1) : name(name), loc(loc) {}
};

struct Shader
{
    static Shader *lastshader;

    char *name, *vsstr, *psstr, *defer;
    int type;
    GLhandleARB program, vsobj, psobj;
    vector<SlotShaderParamState> defaultparams;
    vector<GlobalShaderParamUse> globalparams;
    vector<LocalShaderParamState> localparams;
    vector<uchar> localparamremap;
    Shader *detailshader, *variantshader, *altshader, *fastshader[MAXSHADERDETAIL];
    vector<Shader *> variants[MAXVARIANTROWS];
    bool standard, forced, used, native;
    Shader *reusevs, *reuseps;
    vector<UniformLoc> uniformlocs;
    vector<AttribLoc> attriblocs;

    Shader() : name(NULL), vsstr(NULL), psstr(NULL), defer(NULL), type(SHADER_DEFAULT), program(0), vsobj(0), psobj(0), detailshader(NULL), variantshader(NULL), altshader(NULL), standard(false), forced(false), used(false), native(true), reusevs(NULL), reuseps(NULL) 
    {
        loopi(MAXSHADERDETAIL) fastshader[i] = this;
    }

    ~Shader()
    {
        DELETEA(name);
        DELETEA(vsstr);
        DELETEA(psstr);
        DELETEA(defer);
    }

    void fixdetailshader(bool force = true, bool recurse = true);
    void allocparams(Slot *slot = NULL);
    void setslotparams(Slot &slot, VSlot &vslot);
    void bindprograms();

    void flushparams(Slot *slot = NULL)
    {
        if(!used) { allocparams(slot); used = true; }
        loopv(globalparams) globalparams[i].flush();
    }
     
    bool hasoption(int row)
    {
        if(!detailshader || detailshader->variants[row].empty()) return false;
        return (detailshader->variants[row][0]->type&SHADER_OPTION)!=0;
    }

    void setvariant_(int col, int row, Shader *fallbackshader)
    {
        Shader *s = fallbackshader;
        for(col = min(col, detailshader->variants[row].length()-1); col >= 0; col--) 
            if(!(detailshader->variants[row][col]->type&SHADER_INVALID)) 
            { 
                s = detailshader->variants[row][col]; 
                break; 
            }
        if(lastshader!=s) s->bindprograms();
    }

    void setvariant(int col, int row, Shader *fallbackshader)
    {
        if(!this || !detailshader) return;
        setvariant_(col, row, fallbackshader);
        lastshader->flushparams();
    }

    void setvariant(int col, int row)
    {
        if(!this || !detailshader) return;
        setvariant_(col, row, detailshader);
        lastshader->flushparams();
    }

    void setvariant(int col, int row, Slot &slot, VSlot &vslot, Shader *fallbackshader)
    {
        if(!this || !detailshader) return;
        setvariant_(col, row, fallbackshader);
        lastshader->flushparams(&slot);
        lastshader->setslotparams(slot, vslot);
    }

    void setvariant(int col, int row, Slot &slot, VSlot &vslot)
    {
        if(!this || !detailshader) return;
        setvariant_(col, row, detailshader);
        lastshader->flushparams(&slot);
        lastshader->setslotparams(slot, vslot);
    }

    void set_()
    {
        if(lastshader!=detailshader) detailshader->bindprograms();
    }
 
    void set()
    {
        if(!this || !detailshader) return;
        set_();
        lastshader->flushparams();
    }

    void set(Slot &slot, VSlot &vslot)
    {
        if(!this || !detailshader) return;
        set_();
        lastshader->flushparams(&slot);
        lastshader->setslotparams(slot, vslot);
    }

    bool compile();
    void cleanup(bool invalid = false);
    
    static int uniformlocversion();
};

struct GlobalShaderParam
{
    const char *name;
    GlobalShaderParamState *param;

    GlobalShaderParam(const char *name) : name(name), param(NULL) {}

    GlobalShaderParamState *resolve()
    {
        extern GlobalShaderParamState *getglobalparam(const char *name);
        if(!param) param = getglobalparam(name);
        param->changed();
        return param;
    }

    void set(float x = 0, float y = 0, float z = 0, float w = 0)
    {
        GlobalShaderParamState *g = resolve();
        g->val[0] = x;
        g->val[1] = y;
        g->val[2] = z;
        g->val[3] = w;
    }
    void set(const vec &v) { set(v.x, v.y, v.z); }
    void set(const vec2 &v) { set(v.x, v.y); }
    void set(const vec4 &v) { set(v.x, v.y, v.z, v.w); }
    void set(const plane &p) { set(p.x, p.y, p.z, p.offset); }
    void set(const matrix3x3 &m) { memcpy(resolve()->val, m.a.v, sizeof(m.a.v)); }
    void set(const glmatrixf &m) { memcpy(resolve()->val, m.v, sizeof(m.v)); }
};

struct LocalShaderParam
{
    const char *name;
    int loc;

    LocalShaderParam(const char *name) : name(name), loc(-1) {}

    LocalShaderParamState *resolve()
    {
        Shader *s = Shader::lastshader;
        if(!s) return NULL;
        if(!s->localparamremap.inrange(loc))
        {
            extern int getlocalparam(const char *name);
            if(loc == -1) loc = getlocalparam(name);
            if(!s->localparamremap.inrange(loc)) return NULL;
        }
        return &s->localparams[s->localparamremap[loc]];
    }

    void set(float x = 0, float y = 0, float z = 0, float w = 0)
    {
        ShaderParamBinding *b = resolve();
        if(b) switch(b->format)
        {
            case GL_FLOAT:          glUniform1f_(b->loc, x); break;
            case GL_FLOAT_VEC2_ARB: glUniform2f_(b->loc, x, y); break;
            case GL_FLOAT_VEC3_ARB: glUniform3f_(b->loc, x, y, z); break;
            case GL_FLOAT_VEC4_ARB: glUniform4f_(b->loc, x, y, z, w); break;
        }
    }
    void set(const vec &v) { set(v.x, v.y, v.z); }
    void set(const vec2 &v) { set(v.x, v.y); }
    void set(const vec4 &v) { set(v.x, v.y, v.z, v.w); }
    void set(const plane &p) { set(p.x, p.y, p.z, p.offset); }
    void set(const float *f, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform1fv_(b->loc, n, f); }
    void set(const vec *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform3fv_(b->loc, n, v->v); }
    void set(const vec2 *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform2fv_(b->loc, n, v->v); }
    void set(const vec4 *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform4fv_(b->loc, n, v->v); }
    void set(const plane *p, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform4fv_(b->loc, n, p->v); }
    void set(const matrix3x3 *m, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniformMatrix3fv_(b->loc, n, GL_TRUE, m->a.v); }
    void set(const glmatrixf *m, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniformMatrix4fv_(b->loc, n, GL_FALSE, m->v); }
    void set(const matrix3x3 &m) { set(&m); }
    void set(const glmatrixf &m) { set(&m); }
};

#define SETSHADER(name) \
    do { \
        static Shader *name##shader = NULL; \
        if(!name##shader) name##shader = lookupshaderbyname(#name); \
        name##shader->set(); \
    } while(0)

struct ImageData
{
    int w, h, bpp, levels, align, pitch;
    GLenum compressed;
    uchar *data;
    void *owner;
    void (*freefunc)(void *);

    ImageData()
        : data(NULL), owner(NULL), freefunc(NULL)
    {}

    
    ImageData(int nw, int nh, int nbpp, int nlevels = 1, int nalign = 0, GLenum ncompressed = GL_FALSE) 
    { 
        setdata(NULL, nw, nh, nbpp, nlevels, nalign, ncompressed); 
    }

    ImageData(int nw, int nh, int nbpp, uchar *data)
        : owner(NULL), freefunc(NULL)
    { 
        setdata(data, nw, nh, nbpp); 
    }

    ImageData(SDL_Surface *s) { wrap(s); }
    ~ImageData() { cleanup(); }

    void setdata(uchar *ndata, int nw, int nh, int nbpp, int nlevels = 1, int nalign = 0, GLenum ncompressed = GL_FALSE)
    {
        w = nw;
        h = nh;
        bpp = nbpp;
        levels = nlevels;
        align = nalign;
        pitch = align ? 0 : w*bpp;
        compressed = ncompressed;
        data = ndata ? ndata : new uchar[calcsize()];
        if(!ndata) { owner = this; freefunc = NULL; }
    }
  
    int calclevelsize(int level) const { return ((max(w>>level, 1)+align-1)/align)*((max(h>>level, 1)+align-1)/align)*bpp; }
 
    int calcsize() const
    {
        if(!align) return w*h*bpp;
        int lw = w, lh = h,
            size = 0;
        loopi(levels)
        {
            if(lw<=0) lw = 1;
            if(lh<=0) lh = 1;
            size += ((lw+align-1)/align)*((lh+align-1)/align)*bpp;
            if(lw*lh==1) break;
            lw >>= 1;
            lh >>= 1;
        }
        return size;
    }

    void disown()
    {
        data = NULL;
        owner = NULL;
        freefunc = NULL;
    }

    void cleanup()
    {
        if(owner==this) delete[] data;
        else if(freefunc) (*freefunc)(owner);
        disown();
    }

    void replace(ImageData &d)
    {
        cleanup();
        *this = d;
        if(owner == &d) owner = this;
        d.disown();
    }

    void wrap(SDL_Surface *s)
    {
        setdata((uchar *)s->pixels, s->w, s->h, s->format->BytesPerPixel);
        pitch = s->pitch;
        owner = s;
        freefunc = (void (*)(void *))SDL_FreeSurface;
    }
};

// management of texture slots
// each texture slot can have multiple texture frames, of which currently only the first is used
// additional frames can be used for various shaders

struct Texture
{
    enum
    {
        IMAGE      = 0,
        CUBEMAP    = 1,
        TYPE       = 0xFF,
        
        STUB       = 1<<8,
        TRANSIENT  = 1<<9,
        COMPRESSED = 1<<10, 
        ALPHA      = 1<<11,
        FLAGS      = 0xFF00
    };

    char *name;
    int type, w, h, xs, ys, bpp, clamp;
    bool mipmap, canreduce;
    GLuint id;
    uchar *alphamask;

    Texture() : alphamask(NULL) {}
};

enum
{
    TEX_DIFFUSE = 0,
    TEX_UNKNOWN,
    TEX_DECAL,
    TEX_NORMAL,
    TEX_GLOW,
    TEX_SPEC,
    TEX_DEPTH,
    TEX_ENVMAP
};

enum 
{ 
    VSLOT_SHPARAM = 0, 
    VSLOT_SCALE, 
    VSLOT_ROTATION, 
    VSLOT_OFFSET, 
    VSLOT_SCROLL, 
    VSLOT_LAYER, 
    VSLOT_ALPHA,
    VSLOT_COLOR,
    VSLOT_NUM 
};
   
struct VSlot
{
    Slot *slot;
    VSlot *next;
    int index, changed;
    vector<SlotShaderParam> params;
    bool linked;
    float scale;
    int rotation, xoffset, yoffset;
    float scrollS, scrollT;
    int layer;
    float alphafront, alphaback;
    vec colorscale;
    vec glowcolor;

    VSlot(Slot *slot = NULL, int index = -1) : slot(slot), next(NULL), index(index), changed(0)
    { 
        reset();
        if(slot) addvariant(slot); 
    }

    void addvariant(Slot *slot);

    void reset()
    {
        params.shrink(0);
        linked = false;
        scale = 1;
        rotation = xoffset = yoffset = 0;
        scrollS = scrollT = 0;
        layer = 0;
        alphafront = 0.5f;
        alphaback = 0;
        colorscale = vec(1, 1, 1);
        glowcolor = vec(1, 1, 1);
    }

    void cleanup()
    {
        linked = false;
    }
};

struct Slot
{
    struct Tex
    {
        int type;
        Texture *t;
        string name;
        int combined;
    };

    int index;
    vector<Tex> sts;
    Shader *shader;
    vector<SlotShaderParam> params;
    VSlot *variants;
    bool loaded;
    uint texmask;
    char *autograss;
    Texture *grasstex, *thumbnail;

    Slot(int index = -1) : index(index), variants(NULL), autograss(NULL) { reset(); }
    
    void reset()
    {
        sts.shrink(0);
        shader = NULL;
        params.shrink(0);
        loaded = false;
        texmask = 0;
        DELETEA(autograss);
        grasstex = NULL;
        thumbnail = NULL;
    }

    void cleanup()
    {
        loaded = false;
        grasstex = NULL;
        thumbnail = NULL;
        loopv(sts) 
        {
            Tex &t = sts[i];
            t.t = NULL;
            t.combined = -1;
        }
    }
};

inline void VSlot::addvariant(Slot *slot)
{
    if(!slot->variants) slot->variants = this;
    else
    {
        VSlot *prev = slot->variants;
        while(prev->next) prev = prev->next;
        prev->next = this;
    }
}

struct MSlot : Slot, VSlot
{
    MSlot() : VSlot(this) {}

    void reset()
    {
        Slot::reset();
        VSlot::reset();
    }

    void cleanup()
    {
        Slot::cleanup();
        VSlot::cleanup();
    }
};

struct cubemapside
{
    GLenum target;
    const char *name;
    bool flipx, flipy, swapxy;
};

extern cubemapside cubemapsides[6];
extern Texture *notexture;
extern Shader *defaultshader, *rectshader, *cubemapshader, *notextureshader, *nocolorshader, *foggedshader, *foggednotextureshader, *stdworldshader, *lineshader, *foggedlineshader;
extern int reservevpparams, maxvsuniforms, maxfsuniforms;

extern Shader *lookupshaderbyname(const char *name);
extern Shader *useshaderbyname(const char *name);
extern Texture *loadthumbnail(Slot &slot);
extern void resetslotshader();
extern void setslotshader(Slot &s);
extern void linkslotshader(Slot &s, bool load = true);
extern void linkvslotshader(VSlot &s, bool load = true);
extern void linkslotshaders();
extern void setenvparamf(const char *name, int type, int index, float x = 0, float y = 0, float z = 0, float w = 0);
extern void setenvparamfv(const char *name, int type, int index, const float *v);
extern void setlocalparamf(const char *name, int type, int index, float x = 0, float y = 0, float z = 0, float w = 0);
extern void setlocalparamfv(const char *name, int type, int index, const float *v);
extern const char *getshaderparamname(const char *name);

#define MAXDYNLIGHTS 5
#define DYNLIGHTBITS 6
#define DYNLIGHTMASK ((1<<DYNLIGHTBITS)-1)

#define MAXBLURRADIUS 7

extern void setupblurkernel(int radius, float sigma, float *weights, float *offsets);
extern void setblurshader(int pass, int size, int radius, float *weights, float *offsets, GLenum target = GL_TEXTURE_2D);

extern void savepng(const char *filename, ImageData &image, bool flip = false);
extern void savetga(const char *filename, ImageData &image, bool flip = false);
extern bool loaddds(const char *filename, ImageData &image);
extern bool loadimage(const char *filename, ImageData &image);

extern MSlot &lookupmaterialslot(int slot, bool load = true);
extern Slot &lookupslot(int slot, bool load = true);
extern VSlot &lookupvslot(int slot, bool load = true);
extern VSlot *findvslot(Slot &slot, const VSlot &src, const VSlot &delta);
extern VSlot *editvslot(const VSlot &src, const VSlot &delta);
extern void mergevslot(VSlot &dst, const VSlot &src, const VSlot &delta);

extern vector<Slot *> slots;
extern vector<VSlot *> vslots;

