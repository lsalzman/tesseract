extern int glversion;

namespace varray
{
#ifndef VARRAY_INTERNAL
    enum
    {
        ATTRIB_VERTEX       = 0,
        ATTRIB_COLOR        = 1,
        ATTRIB_TEXCOORD0    = 2,
        ATTRIB_TEXCOORD1    = 3,
        ATTRIB_NORMAL       = 4,
        ATTRIB_TANGENT      = 5,
        ATTRIB_BONEWEIGHT   = 6,
        ATTRIB_BONEINDEX    = 7, 
        MAXATTRIBS          = 8
    };

    extern const char * const attribnames[MAXATTRIBS];
    extern vector<uchar> data;

    extern void begin(GLenum mode);
    extern void defattribs(const char *fmt);
    extern void defattrib(int type, int size, int format);

    #define VARRAY_DEFATTRIB(name, type, defaultsize, defaultformat) \
        static inline void def##name(int size = defaultsize, int format = defaultformat) { defattrib(type, size, format); }

    VARRAY_DEFATTRIB(vertex, ATTRIB_VERTEX, 3, GL_FLOAT)
    VARRAY_DEFATTRIB(color, ATTRIB_COLOR, 3, GL_FLOAT)
    VARRAY_DEFATTRIB(texcoord0, ATTRIB_TEXCOORD0, 2, GL_FLOAT)
    VARRAY_DEFATTRIB(texcoord1, ATTRIB_TEXCOORD1, 2, GL_FLOAT)
    VARRAY_DEFATTRIB(normal, ATTRIB_NORMAL, 3, GL_FLOAT)
    VARRAY_DEFATTRIB(tangent, ATTRIB_TANGENT, 4, GL_FLOAT)
    VARRAY_DEFATTRIB(boneweight, ATTRIB_BONEWEIGHT, 4, GL_UNSIGNED_BYTE)
    VARRAY_DEFATTRIB(boneindex, ATTRIB_BONEINDEX, 4, GL_UNSIGNED_BYTE)

    #define VARRAY_INITATTRIB(name, index, suffix, type) \
        static inline void name##suffix(type x) { glVertexAttrib1##suffix##_(index, x); } \
        static inline void name##suffix(type x, type y) { glVertexAttrib2##suffix##_(index, x, y); } \
        static inline void name##suffix(type x, type y, type z) { glVertexAttrib3##suffix##_(index, x, y, z); } \
        static inline void name##suffix(type x, type y, type z, type w) { glVertexAttrib4##suffix##_(index, x, y, z, w); }
    #define VARRAY_INITATTRIBF(name, index) \
        VARRAY_INITATTRIB(name, index, f, float) \
        static inline void name(const vec &v) { glVertexAttrib3fv_(index, v.v); } \
        static inline void name(const vec &v, float w) { glVertexAttrib4f_(index, v.x, v.y, v.z, w); } \
        static inline void name(const vec2 &v) { glVertexAttrib2fv_(index, v.v); } \
        static inline void name(const vec4 &v) { glVertexAttrib4fv_(index, v.v); }
    #define VARRAY_INITATTRIBN(name, index, suffix, type, defaultw) \
        static inline void name##suffix(type x, type y, type z, type w = defaultw) { glVertexAttrib4N##suffix##_(index, x, y, z, w); }

    VARRAY_INITATTRIBF(vertex, ATTRIB_VERTEX)
    VARRAY_INITATTRIBF(color, ATTRIB_COLOR)
    VARRAY_INITATTRIBN(color, ATTRIB_COLOR, ub, uchar, 255)
    static inline void color(const bvec &v, uchar alpha = 255) { glVertexAttrib4Nub_(ATTRIB_COLOR, v.x, v.y, v.z, alpha); }
    VARRAY_INITATTRIBF(texcoord0, ATTRIB_TEXCOORD0)
    VARRAY_INITATTRIBF(texcoord1, ATTRIB_TEXCOORD1)
    static inline void normal(float x, float y, float z) { glVertexAttrib4f_(ATTRIB_NORMAL, x, y, z, 0.0f); }
    static inline void normal(const vec &v) { glVertexAttrib4f_(ATTRIB_NORMAL, v.x, v.y, v.z, 0.0f); }
    static inline void tangent(float x, float y, float z, float w = 1.0f) { glVertexAttrib4f_(ATTRIB_TANGENT, x, y, z, w); }
    static inline void tangent(const vec &v, float w = 1.0f) { glVertexAttrib4f_(ATTRIB_TANGENT, v.x, v.y, v.z, w); }
    static inline void tangent(const vec4 &v) { glVertexAttrib4fv_(ATTRIB_TANGENT, v.v); }

    #define VARRAY_ATTRIBPOINTER(name, index, normalized, defaultsize, defaulttype) \
        static inline void enable##name() { glEnableVertexAttribArray_(index); } \
        static inline void disable##name() { glDisableVertexAttribArray_(index); } \
        static inline void name##pointer(int stride, const void *data, GLenum type = defaulttype, int size = defaultsize) { \
            glVertexAttribPointer_(index, size, type, normalized, stride, data); \
        }

    static inline void enableattrib(int index) { glEnableVertexAttribArray_(index); }
    static inline void disableattrib(int index) { glDisableVertexAttribArray_(index); }
    VARRAY_ATTRIBPOINTER(vertex, ATTRIB_VERTEX, GL_FALSE, 3, GL_FLOAT)
    VARRAY_ATTRIBPOINTER(color, ATTRIB_COLOR, GL_TRUE, 4, GL_UNSIGNED_BYTE)
    VARRAY_ATTRIBPOINTER(texcoord0, ATTRIB_TEXCOORD0, GL_FALSE, 2, GL_FLOAT)
    VARRAY_ATTRIBPOINTER(texcoord1, ATTRIB_TEXCOORD1, GL_FALSE, 2, GL_FLOAT)
    VARRAY_ATTRIBPOINTER(normal, ATTRIB_NORMAL, GL_TRUE, 3, GL_FLOAT)
    VARRAY_ATTRIBPOINTER(tangent, ATTRIB_TANGENT, GL_TRUE, 4, GL_FLOAT)
    VARRAY_ATTRIBPOINTER(boneweight, ATTRIB_BONEWEIGHT, GL_TRUE, 4, GL_UNSIGNED_BYTE)
    VARRAY_ATTRIBPOINTER(boneindex, ATTRIB_BONEINDEX, GL_FALSE, 4, GL_UNSIGNED_BYTE)

    template<class T>
    static inline void attrib(T x)
    {
        T *buf = (T *)data.pad(sizeof(T));
        buf[0] = x;
    }

    template<class T>
    static inline void attrib(T x, T y)
    {
        T *buf = (T *)data.pad(2*sizeof(T));
        buf[0] = x;
        buf[1] = y;
    }

    template<class T>
    static inline void attrib(T x, T y, T z)
    {
        T *buf = (T *)data.pad(3*sizeof(T));
        buf[0] = x;
        buf[1] = y;
        buf[2] = z;
    }

    template<class T>
    static inline void attrib(T x, T y, T z, T w)
    {
        T *buf = (T *)data.pad(4*sizeof(T));
        buf[0] = x;
        buf[1] = y;
        buf[2] = z;
        buf[3] = w;
    }

    template<size_t N, class T>
    static inline void attribv(const T *v)
    {
        data.put((const uchar *)v, N*sizeof(T)); 
    }

    #define VARRAY_ATTRIB(suffix, type) \
        static inline void attrib##suffix(type x) { attrib<type>(x); } \
        static inline void attrib##suffix(type x, type y) { attrib<type>(x, y); } \
        static inline void attrib##suffix(type x, type y, type z) { attrib<type>(x, y, z); } \
        static inline void attrib##suffix(type x, type y, type z, type w) { attrib<type>(x, y, z, w); }

    VARRAY_ATTRIB(f, float)
    VARRAY_ATTRIB(d, double)
    VARRAY_ATTRIB(b, char)
    VARRAY_ATTRIB(ub, uchar)
    VARRAY_ATTRIB(s, short)
    VARRAY_ATTRIB(us, ushort)
    VARRAY_ATTRIB(i, int)
    VARRAY_ATTRIB(ui, uint)

    static inline void attrib(const vec &v) { attribf(v.x, v.y, v.z); }
    static inline void attrib(const vec2 &v) { attribf(v.x, v.y); }
    static inline void attrib(const vec4 &v) { attribf(v.x, v.y, v.z, v.w); }
    static inline void attrib(const ivec &v) { attribi(v.x, v.y, v.z); }
    static inline void attrib(const ivec2 &v) { attribi(v.x, v.y); }
    static inline void attrib(const ivec4 &v) { attribi(v.x, v.y, v.z, v.w); }
    static inline void attrib(const bvec &b) { attribub(b.x, b.y, b.z); }
    static inline void attrib(const bvec &b, uchar w) { attribub(b.x, b.y, b.z, w); }

    extern int end();
    extern void disable();

    extern void enablequads();
    extern void disablequads();
    extern void drawquads(int offset, int count);

    extern void setup();
    extern void cleanup();

#else
    struct attribinfo
    {
        int type, size, formatsize, offset;
        GLenum format;

        attribinfo() : type(0), size(0), formatsize(0), offset(0), format(GL_FALSE) {}

        bool operator==(const attribinfo &a) const
        {
            return type == a.type && size == a.size && format == a.format && offset == a.offset;
        }
        bool operator!=(const attribinfo &a) const
        {
            return type != a.type || size != a.size || format != a.format || offset != a.offset;
        }
    };

    extern const char * const attribnames[MAXATTRIBS] = { "vvertex", "vcolor", "vtexcoord0", "vtexcoord1", "vnormal", "vtangent", "vboneweight", "vboneindex" };
    vector<uchar> data;
    static attribinfo attribdefs[MAXATTRIBS], lastattribs[MAXATTRIBS];
    static int enabled = 0, numattribs = 0, attribmask = 0, numlastattribs = 0, lastattribmask = 0, vertexsize = 0, lastvertexsize = 0;
    static GLenum primtype = GL_TRIANGLES;
    static uchar *lastbuf = NULL;
    static bool changedattribs = false;

    #define MAXQUADS (0x10000/4)
    static GLuint quadindexes = 0;
    static bool quadsenabled = false;

    #define MAXVBOSIZE (4*1024*1024)
    static GLuint vbo = 0;
    static int vbooffset = MAXVBOSIZE;

    static GLuint defaultvao = 0;

    void enablequads()
    {
        quadsenabled = true;

        if(quadindexes)
        {
            glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, quadindexes);
            return;
        }

        glGenBuffers_(1, &quadindexes);
        ushort *data = new ushort[MAXQUADS*6], *dst = data;
        for(int idx = 0; idx < MAXQUADS*4; idx += 4, dst += 6)
        {
            dst[0] = idx;
            dst[1] = idx + 1;
            dst[2] = idx + 2;
            dst[3] = idx + 0;
            dst[4] = idx + 2;
            dst[5] = idx + 3;
        }
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, quadindexes);
        glBufferData_(GL_ELEMENT_ARRAY_BUFFER, MAXQUADS*6*sizeof(ushort), data, GL_STATIC_DRAW);
        delete[] data;
    }

    void disablequads()
    {
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

        quadsenabled = false;
    }

    void drawquads(int offset, int count)
    {
        if(count <= 0) return;
        if(offset + count > MAXQUADS) 
        {
            if(offset >= MAXQUADS) return;
            count = MAXQUADS - offset;
        }
        glDrawRangeElements_(GL_TRIANGLES, offset*4, (offset + count)*4-1, count*6, GL_UNSIGNED_SHORT, (ushort *)0 + offset*6); 
    }

    void begin(GLenum mode)
    {
        primtype = mode;
    }

    void defattrib(int type, int size, int format)
    {
        if(type == ATTRIB_VERTEX)
        {
            numattribs = attribmask = 0;
            vertexsize = 0;
        }
        changedattribs = true;
        attribmask |= type;
        attribinfo &a = attribdefs[numattribs++];
        a.type = type;
        a.size = size;
        a.format = format;
        switch(format)
        {
            case 'B': case GL_UNSIGNED_BYTE:  a.formatsize = 1; a.format = GL_UNSIGNED_BYTE; break;
            case 'b': case GL_BYTE:           a.formatsize = 1; a.format = GL_BYTE; break;
            case 'S': case GL_UNSIGNED_SHORT: a.formatsize = 2; a.format = GL_UNSIGNED_SHORT; break;
            case 's': case GL_SHORT:          a.formatsize = 2; a.format = GL_SHORT; break;
            case 'I': case GL_UNSIGNED_INT:   a.formatsize = 4; a.format = GL_UNSIGNED_INT; break;
            case 'i': case GL_INT:            a.formatsize = 4; a.format = GL_INT; break;
            case 'f': case GL_FLOAT:          a.formatsize = 4; a.format = GL_FLOAT; break;
            case 'd': case GL_DOUBLE:         a.formatsize = 8; a.format = GL_DOUBLE; break;
            default:                          a.formatsize = 0; a.format = GL_FALSE; break;
        }
        a.formatsize *= size;
        a.offset = vertexsize;
        vertexsize += a.formatsize;
    }

    void defattribs(const char *fmt)
    {
        for(;; fmt += 3) 
        {
            GLenum format;
            switch(fmt[0])
            {
                case 'v': format = ATTRIB_VERTEX; break;
                case 'c': format = ATTRIB_COLOR; break;
                case 't': format = ATTRIB_TEXCOORD0; break;
                case 'T': format = ATTRIB_TEXCOORD1; break; 
                case 'n': format = ATTRIB_NORMAL; break;
                case 'x': format = ATTRIB_TANGENT; break;
                case 'w': format = ATTRIB_BONEWEIGHT; break;
                case 'i': format = ATTRIB_BONEINDEX; break;
                default: return;
            }
            defattrib(format, fmt[1]-'0', fmt[2]);
        }
    }

    static inline void setattrib(const attribinfo &a, uchar *buf)
    {
        switch(a.type)
        {
            case ATTRIB_VERTEX:
            case ATTRIB_TEXCOORD0:
            case ATTRIB_TEXCOORD1:
            case ATTRIB_BONEINDEX:
                glVertexAttribPointer_(a.type, a.size, a.format, GL_FALSE, vertexsize, buf);
                break;
            case ATTRIB_COLOR:
            case ATTRIB_NORMAL:
            case ATTRIB_TANGENT:
            case ATTRIB_BONEWEIGHT:
                glVertexAttribPointer_(a.type, a.size, a.format, GL_TRUE, vertexsize, buf);
                break;
        }
        if(!(enabled&(1<<a.type))) 
        {
            glEnableVertexAttribArray_(a.type);
            enabled |= 1<<a.type;
        }
    }

    static inline void unsetattrib(const attribinfo &a)
    {
        glDisableVertexAttribArray_(a.type);
        enabled &= ~(1<<a.type);
    }

    int end()
    {
        if(data.empty()) return 0;
        uchar *buf = data.getbuf();
        int start = 0;
        if(glversion >= 300)
        {
            if(vbooffset + data.length() >= MAXVBOSIZE)
            {
                if(!vbo) glGenBuffers_(1, &vbo);
                glBindBuffer_(GL_ARRAY_BUFFER, vbo);
                glBufferData_(GL_ARRAY_BUFFER, MAXVBOSIZE, NULL, GL_STREAM_DRAW);
                vbooffset = 0;
            }
            else if(!lastvertexsize) glBindBuffer_(GL_ARRAY_BUFFER, vbo);
            glBufferSubData_(GL_ARRAY_BUFFER, vbooffset, data.length(), data.getbuf());    
            buf = (uchar *)0 + vbooffset;
            if(vertexsize == lastvertexsize && buf >= lastbuf)
            {
                start = int(buf - lastbuf)/vertexsize;
                if(primtype == GL_QUADS && (start%4 || start + data.length()/vertexsize >= 4*MAXQUADS))
                    start = 0;
                else buf = lastbuf; 
            }
            vbooffset += data.length();
        }
        bool forceattribs = numattribs != numlastattribs || vertexsize != lastvertexsize || buf != lastbuf;
        if(forceattribs || changedattribs)
        {
            int diffmask = enabled & lastattribmask & ~attribmask;
            if(diffmask) loopi(numlastattribs)
            {
                const attribinfo &a = lastattribs[i];
                if(diffmask & (1<<a.type)) unsetattrib(a);
            }
            uchar *src = buf;
            loopi(numattribs)
            {
                const attribinfo &a = attribdefs[i];
                if(forceattribs || a != lastattribs[i])
                {
                    setattrib(a, src);
                    lastattribs[i] = a;
                }
                src += a.formatsize;
            }
            lastbuf = buf;
            numlastattribs = numattribs;
            lastattribmask = attribmask;
            lastvertexsize = vertexsize;
            changedattribs = false;
        }
        int numvertexes = data.length()/vertexsize;
        if(primtype == GL_QUADS) 
        {
            if(!quadsenabled) enablequads();
            drawquads(start/4, numvertexes/4);
        }
        else 
        {
            glDrawArrays(primtype, start, numvertexes);
        }
        data.setsize(0);
        return numvertexes;
    }

    void disable()
    {
        if(!enabled) return;
        for(int i = 0; enabled; i++) if(enabled&(1<<i)) { glDisableVertexAttribArray_(i); enabled &= ~(1<<i); }
        numlastattribs = lastattribmask = lastvertexsize = 0;
        lastbuf = NULL;
        if(quadsenabled) disablequads();
        if(glversion >= 300) glBindBuffer_(GL_ARRAY_BUFFER, 0);
    }

    void setup()
    {
        if(glversion >= 300)
        {
            if(!defaultvao) glGenVertexArrays_(1, &defaultvao);
            glBindVertexArray_(defaultvao);
        }
    }

    void cleanup()
    {
        if(quadindexes) { glDeleteBuffers_(1, &quadindexes); quadindexes = 0; }

        if(vbo) { glDeleteBuffers_(1, &vbo); vbo = 0; }
        vbooffset = MAXVBOSIZE;

        if(defaultvao) { glDeleteVertexArrays_(1, &defaultvao); defaultvao = 0; }
    }
#endif
}

