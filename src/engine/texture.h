extern int renderpath;

enum { R_GLSLANG = 0 };

enum { SHPARAM_LOOKUP = 0, SHPARAM_VERTEX, SHPARAM_PIXEL, SHPARAM_UNIFORM };

struct GlobalShaderParamState
{
    const char *name;
    union
    {
        float fval[32];
        int ival[32];
        uchar buf[32*sizeof(float)];
    };
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
    int loc, size;
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
            case GL_BOOL:
            case GL_FLOAT:      glUniform1fv_(loc, size, param->fval); break;
            case GL_BOOL_VEC2:
            case GL_FLOAT_VEC2: glUniform2fv_(loc, size, param->fval); break;
            case GL_BOOL_VEC3:
            case GL_FLOAT_VEC3: glUniform3fv_(loc, size, param->fval); break;
            case GL_BOOL_VEC4:
            case GL_FLOAT_VEC4: glUniform4fv_(loc, size, param->fval); break;
            case GL_INT:        glUniform1iv_(loc, size, param->ival); break;
            case GL_INT_VEC2:   glUniform2iv_(loc, size, param->ival); break;
            case GL_INT_VEC3:   glUniform3iv_(loc, size, param->ival); break;
            case GL_INT_VEC4:   glUniform4iv_(loc, size, param->ival); break;
            case GL_FLOAT_MAT2: glUniformMatrix2fv_(loc, 1, GL_FALSE, param->fval); break;
            case GL_FLOAT_MAT3: glUniformMatrix3fv_(loc, 1, GL_FALSE, param->fval); break;
            case GL_FLOAT_MAT4: glUniformMatrix4fv_(loc, 1, GL_FALSE, param->fval); break;
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
        size = 1;
        format = GL_FLOAT_VEC4; 
        memcpy(val, p.val, sizeof(val)); 
    }
};

enum 
{ 
    SHADER_DEFAULT    = 0, 
    SHADER_WORLD      = 1<<0, 
    SHADER_ENVMAP     = 1<<1,
    SHADER_REFRACT    = 1<<2,
    SHADER_OPTION     = 1<<3,
    SHADER_DYNAMIC    = 1<<4,

    SHADER_INVALID    = 1<<8,
    SHADER_DEFERRED   = 1<<9
};

#define MAXSHADERDETAIL 3
#define MAXVARIANTROWS 8

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

struct FragDataLoc
{
    const char *name;
    int loc;
    GLenum format;
    FragDataLoc(const char *name = NULL, int loc = -1, GLenum format = GL_FALSE) : name(name), loc(loc), format(format) {}
};

struct Shader
{
    static Shader *lastshader;

    char *name, *vsstr, *psstr, *defer;
    int type;
    GLuint program, vsobj, psobj;
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
    vector<FragDataLoc> fragdatalocs;

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

    bool isdynamic() const { return (type&SHADER_DYNAMIC)!=0; }

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

    void setf(float x = 0, float y = 0, float z = 0, float w = 0)
    {
        GlobalShaderParamState *g = resolve();
        g->fval[0] = x;
        g->fval[1] = y;
        g->fval[2] = z;
        g->fval[3] = w;
    }
    void set(const vec &v, float w = 0) { setf(v.x, v.y, v.z, w); }
    void set(const vec2 &v, float z = 0, float w = 0) { setf(v.x, v.y, z, w); }
    void set(const vec4 &v) { setf(v.x, v.y, v.z, v.w); }
    void set(const plane &p) { setf(p.x, p.y, p.z, p.offset); }
    void set(const matrix3x3 &m) { m.transpose(resolve()->fval); }
    void set(const glmatrix2x2 &m) { memcpy(resolve()->fval, m.a.v, sizeof(m)); }
    void set(const glmatrix3x3 &m) { memcpy(resolve()->fval, m.a.v, sizeof(m)); }
    void set(const glmatrix &m) { memcpy(resolve()->fval, m.a.v, sizeof(m)); }

    template<class T>
    void setv(const T *v, int n = 1) { memcpy(resolve()->buf, v, n*sizeof(T)); }

    void seti(int x = 0, int y = 0, int z = 0, int w = 0)
    {
        GlobalShaderParamState *g = resolve();
        g->ival[0] = x;
        g->ival[1] = y;
        g->ival[2] = z;
        g->ival[3] = w;
    }
    void set(const ivec &v, int w = 0) { seti(v.x, v.y, v.z, w); }
    void set(const ivec2 &v, int z = 0, int w = 0) { seti(v.x, v.y, z, w); }
    void set(const ivec4 &v) { seti(v.x, v.y, v.z, v.w); }
 
    template<class T>
    T *reserve(int n = 1) { return (T *)resolve()->buf; }
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

    void setf(float x = 0, float y = 0, float z = 0, float w = 0)
    {
        ShaderParamBinding *b = resolve();
        if(b) switch(b->format)
        {
            case GL_BOOL:
            case GL_FLOAT:      glUniform1f_(b->loc, x); break;
            case GL_BOOL_VEC2:
            case GL_FLOAT_VEC2: glUniform2f_(b->loc, x, y); break;
            case GL_BOOL_VEC3:
            case GL_FLOAT_VEC3: glUniform3f_(b->loc, x, y, z); break;
            case GL_BOOL_VEC4:
            case GL_FLOAT_VEC4: glUniform4f_(b->loc, x, y, z, w); break;
            case GL_INT:        glUniform1i_(b->loc, int(x)); break;
            case GL_INT_VEC2:   glUniform2i_(b->loc, int(x), int(y)); break;
            case GL_INT_VEC3:   glUniform3i_(b->loc, int(x), int(y), int(z)); break;
            case GL_INT_VEC4:   glUniform4i_(b->loc, int(x), int(y), int(z), int(w)); break;
        }
    }
    void set(const vec &v, float w = 0) { setf(v.x, v.y, v.z, w); }
    void set(const vec2 &v, float z = 0, float w = 0) { setf(v.x, v.y, z, w); }
    void set(const vec4 &v) { setf(v.x, v.y, v.z, v.w); }
    void set(const plane &p) { setf(p.x, p.y, p.z, p.offset); }
    void setv(const float *f, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform1fv_(b->loc, n, f); }
    void setv(const vec *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform3fv_(b->loc, n, v->v); }
    void setv(const vec2 *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform2fv_(b->loc, n, v->v); }
    void setv(const vec4 *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform4fv_(b->loc, n, v->v); }
    void setv(const plane *p, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform4fv_(b->loc, n, p->v); }
    void setv(const matrix3x3 *m, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniformMatrix3fv_(b->loc, n, GL_TRUE, m->a.v); }
    void setv(const glmatrix2x2 *m, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniformMatrix2fv_(b->loc, n, GL_FALSE, m->a.v); }
    void setv(const glmatrix3x3 *m, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniformMatrix3fv_(b->loc, n, GL_FALSE, m->a.v); }
    void setv(const glmatrix *m, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniformMatrix4fv_(b->loc, n, GL_FALSE, m->a.v); }
    void set(const matrix3x3 &m) { setv(&m); }
    void set(const glmatrix2x2 &m) { setv(&m); }
    void set(const glmatrix3x3 &m) { setv(&m); }
    void set(const glmatrix &m) { setv(&m); }

    void seti(int x = 0, int y = 0, int z = 0, int w = 0)
    {
        ShaderParamBinding *b = resolve();
        if(b) switch(b->format)
        {
            case GL_FLOAT:      glUniform1f_(b->loc, x); break;
            case GL_FLOAT_VEC2: glUniform2f_(b->loc, x, y); break;
            case GL_FLOAT_VEC3: glUniform3f_(b->loc, x, y, z); break;
            case GL_FLOAT_VEC4: glUniform4f_(b->loc, x, y, z, w); break;
            case GL_BOOL:
            case GL_INT:        glUniform1i_(b->loc, x); break;
            case GL_BOOL_VEC2:
            case GL_INT_VEC2:   glUniform2i_(b->loc, x, y); break;
            case GL_BOOL_VEC3:
            case GL_INT_VEC3:   glUniform3i_(b->loc, x, y, z); break;
            case GL_BOOL_VEC4:
            case GL_INT_VEC4:   glUniform4i_(b->loc, x, y, z, w); break;
        }
    }
    void set(const ivec &v, int w = 0) { seti(v.x, v.y, v.z, w); }
    void set(const ivec2 &v, int z = 0, int w = 0) { seti(v.x, v.y, z, w); }
    void set(const ivec4 &v) { seti(v.x, v.y, v.z, v.w); }
    void setv(const int *i, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform1iv_(b->loc, n, i); }
    void setv(const ivec *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform3iv_(b->loc, n, v->v); }
    void setv(const ivec2 *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform2iv_(b->loc, n, v->v); }
    void setv(const ivec4 *v, int n = 1) { ShaderParamBinding *b = resolve(); if(b) glUniform4iv_(b->loc, n, v->v); }
};

#define LOCALPARAM(name, vals) do { static LocalShaderParam param( #name ); param.set(vals); } while(0)
#define LOCALPARAMF(name, vals) do { static LocalShaderParam param( #name ); param.setf vals ; } while(0)
#define LOCALPARAMI(name, vals) do { static LocalShaderParam param( #name ); param.seti vals ; } while(0)
#define LOCALPARAMV(name, vals, num) do { static LocalShaderParam param( #name ); param.setv(vals, num); } while(0)
#define GLOBALPARAM(name, vals) do { static GlobalShaderParam param( #name ); param.set(vals); } while(0) 
#define GLOBALPARAMF(name, vals) do { static GlobalShaderParam param( #name ); param.setf vals ; } while(0) 
#define GLOBALPARAMI(name, vals) do { static GlobalShaderParam param( #name ); param.seti vals ; } while(0) 
#define GLOBALPARAMV(name, vals, num) do { static GlobalShaderParam param( #name ); param.setv(vals, num); } while(0) 

#define SETSHADER(name) \
    do { \
        static Shader *name##shader = NULL; \
        if(!name##shader) name##shader = lookupshaderbyname(#name); \
        name##shader->set(); \
    } while(0)
#define SETVARIANT(name, col, row) \
    do { \
        static Shader *name##shader = NULL; \
        if(!name##shader) name##shader = lookupshaderbyname(#name); \
        name##shader->setvariant(col, row); \
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
    TEX_NORMAL,
    TEX_GLOW,
    TEX_ENVMAP,
    TEX_DECAL,
    
    TEX_SPEC,
    TEX_DEPTH,
    TEX_UNKNOWN
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
    VSLOT_RESERVED, // used by RE
    VSLOT_REFRACT,
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
    int rotation;
    ivec2 offset;
    vec2 scroll;
    int layer;
    float alphafront, alphaback;
    vec colorscale;
    vec glowcolor;
    float refractscale;
    vec refractcolor;

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
        rotation = 0;
        offset = ivec2(0, 0);
        scroll = vec2(0, 0);
        layer = 0;
        alphafront = 0.5f;
        alphaback = 0;
        colorscale = vec(1, 1, 1);
        glowcolor = vec(1, 1, 1);
        refractscale = 0;
        refractcolor = vec(1, 1, 1);
    }

    void cleanup()
    {
        linked = false;
    }

    bool isdynamic() const;
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

    int index, smooth;
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
        smooth = -1;
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

inline bool VSlot::isdynamic() const
{
    return !scroll.iszero() || slot->shader->isdynamic();
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
extern Shader *nullshader, *hudshader, *hudnotextureshader, *nocolorshader, *foggedshader, *foggednotextureshader, *ldrshader, *ldrnotextureshader, *stdworldshader, *rsmworldshader;
extern int maxvsuniforms, maxfsuniforms;

extern Shader *lookupshaderbyname(const char *name);
extern Shader *useshaderbyname(const char *name);
extern Shader *generateshader(const char *name, const char *cmd, ...);
extern Texture *loadthumbnail(Slot &slot);
extern void resetslotshader();
extern void setslotshader(Slot &s);
extern void linkslotshader(Slot &s, bool load = true);
extern void linkvslotshader(VSlot &s, bool load = true);
extern void linkslotshaders();
extern const char *getshaderparamname(const char *name);

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

extern Slot dummyslot;
extern VSlot dummyvslot;
extern vector<Slot *> slots;
extern vector<VSlot *> vslots;

