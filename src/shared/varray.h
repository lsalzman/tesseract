namespace varray
{
#ifndef VARRAY_INTERNAL
    enum
    {
        ATTRIB_VERTEX    = 1<<0,
        ATTRIB_COLOR     = 1<<1,
        ATTRIB_NORMAL    = 1<<2,
        ATTRIB_TEXCOORD0 = 1<<3,
        ATTRIB_TEXCOORD1 = 1<<4,
        MAXATTRIBS       = 5
    };

    extern vector<uchar> data;

    extern void begin(GLenum mode);
    extern void defattribs(const char *fmt);
    extern void defattrib(int type, int size, int format);

    #define VARRAY_DEFATTRIBALIAS(name, type, defaultsize, defaultformat) \
        static inline void def##name(int size = defaultsize, int format = defaultformat) { defattrib(type, size, format); }

    VARRAY_DEFATTRIBALIAS(vertex, ATTRIB_VERTEX, 3, GL_FLOAT)
    VARRAY_DEFATTRIBALIAS(color, ATTRIB_COLOR, 3, GL_FLOAT)
    VARRAY_DEFATTRIBALIAS(normal, ATTRIB_NORMAL, 3, GL_FLOAT)
    VARRAY_DEFATTRIBALIAS(texcoord0, ATTRIB_TEXCOORD0, 2, GL_FLOAT)
    VARRAY_DEFATTRIBALIAS(texcoord1, ATTRIB_TEXCOORD1, 2, GL_FLOAT)
    
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

    #define VARRAY_ATTRIBALIAS(suffix, type) \
        static inline void attrib##suffix(type x) { attrib<type>(x); } \
        static inline void attrib##suffix(type x, type y) { attrib<type>(x, y); } \
        static inline void attrib##suffix(type x, type y, type z) { attrib<type>(x, y, z); } \
        static inline void attrib##suffix(type x, type y, type z, type w) { attrib<type>(x, y, z, w); }

    VARRAY_ATTRIBALIAS(f, float)
    VARRAY_ATTRIBALIAS(d, double)
    VARRAY_ATTRIBALIAS(b, char)
    VARRAY_ATTRIBALIAS(ub, uchar)
    VARRAY_ATTRIBALIAS(s, short)
    VARRAY_ATTRIBALIAS(us, ushort)
    VARRAY_ATTRIBALIAS(i, int)
    VARRAY_ATTRIBALIAS(ui, uint)

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

    vector<uchar> data;
    static attribinfo attribdefs[MAXATTRIBS], lastattribs[MAXATTRIBS];
    static int enabled = 0, numattribs = 0, attribmask = 0, numlastattribs = 0, lastattribmask = 0, vertexsize = 0, lastvertexsize = 0;
    static GLenum primtype = GL_TRIANGLES;
    static uchar *lastbuf = NULL;
    static bool changedattribs = false;

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
        for(;; fmt += 3) switch(*fmt)
        {
            case 'v': defattrib(ATTRIB_VERTEX, fmt[1]-'0', fmt[2]); break;
            case 'c': defattrib(ATTRIB_COLOR, fmt[1]-'0', fmt[2]); break;
            case 'n': defattrib(ATTRIB_NORMAL, fmt[1]-'0', fmt[2]); break;
            case 't': defattrib(ATTRIB_TEXCOORD0, fmt[1]-'0', fmt[2]); break;
            case 'T': defattrib(ATTRIB_TEXCOORD1, fmt[2]-'0', fmt[2]); break; 
            default: return;
        }
    }

    static inline void setattrib(const attribinfo &a, uchar *buf)
    {
        switch(a.type)
        {
            case ATTRIB_VERTEX:
                if(!(enabled&a.type)) glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(a.size, a.format, vertexsize, buf);
                break;
            case ATTRIB_COLOR:
                if(!(enabled&a.type)) glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(a.size, a.format, vertexsize, buf);
                break;
            case ATTRIB_NORMAL:
                if(!(enabled&a.type)) glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(a.format, vertexsize, buf);
                break;
            case ATTRIB_TEXCOORD0:
                if(!(enabled&a.type)) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(a.size, a.format, vertexsize, buf);
                break;
            case ATTRIB_TEXCOORD1:
                glClientActiveTexture_(GL_TEXTURE1_ARB);
                if(!(enabled&a.type)) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(a.size, a.format, vertexsize, buf);
                glClientActiveTexture_(GL_TEXTURE0_ARB);
                break;
        }
        enabled |= a.type;
    }

    static inline void unsetattrib(const attribinfo &a)
    {
        switch(a.type)
        {
            case ATTRIB_VERTEX:
                glDisableClientState(GL_VERTEX_ARRAY);
                break;
            case ATTRIB_COLOR:
                glDisableClientState(GL_COLOR_ARRAY);
                break;
            case ATTRIB_NORMAL:
                glDisableClientState(GL_NORMAL_ARRAY);
                break;
            case ATTRIB_TEXCOORD0:
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                break;
            case ATTRIB_TEXCOORD1:
                glClientActiveTexture_(GL_TEXTURE1_ARB);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glClientActiveTexture_(GL_TEXTURE0_ARB);
                break;
        }
        enabled &= ~a.type;
    }

    int end()
    {
        if(data.empty()) return 0;
        uchar *buf = data.getbuf();
        bool forceattribs = numattribs != numlastattribs || vertexsize != lastvertexsize || buf != lastbuf;
        if(forceattribs || changedattribs)
        {
            int diffmask = enabled & lastattribmask & ~attribmask;
            if(diffmask) loopi(numlastattribs)
            {
                const attribinfo &a = lastattribs[i];
                if(diffmask & a.type) unsetattrib(a);
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
        glDrawArrays(primtype, 0, numvertexes);
        data.setsize(0);
        return numvertexes;
    }

    void disable()
    {
        if(!enabled) return;
        if(enabled&ATTRIB_VERTEX) glDisableClientState(GL_VERTEX_ARRAY);
        if(enabled&ATTRIB_COLOR) glDisableClientState(GL_COLOR_ARRAY);
        if(enabled&ATTRIB_NORMAL) glDisableClientState(GL_NORMAL_ARRAY);
        if(enabled&ATTRIB_TEXCOORD0) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(enabled&ATTRIB_TEXCOORD1)
        {
            glClientActiveTexture_(GL_TEXTURE1_ARB);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glClientActiveTexture_(GL_TEXTURE0_ARB);
        }
        enabled = 0;
        numlastattribs = lastattribmask = lastvertexsize = 0;
        lastbuf = NULL;
    }
#endif
}

