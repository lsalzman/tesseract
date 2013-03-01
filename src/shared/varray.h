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
    #define VARRAY_INITATTRIBN(name, index, suffix, type, defaultw) \
        static inline void name##suffix(type x, type y, type z, type w = defaultw) { glVertexAttrib4N##suffix##_(index, x, y, z, w); }

    VARRAY_INITATTRIB(vertex, ATTRIB_VERTEX, f, float)
    VARRAY_INITATTRIB(color, ATTRIB_COLOR, f, float)
    VARRAY_INITATTRIBN(color, ATTRIB_COLOR, ub, uchar, 255)
    VARRAY_INITATTRIB(texcoord0, ATTRIB_TEXCOORD0, f, float)
    VARRAY_INITATTRIB(texcoord1, ATTRIB_TEXCOORD1, f, float)

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
                if(!(enabled&(1<<ATTRIB_VERTEX))) glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(a.size, a.format, vertexsize, buf);
                break;
            case ATTRIB_COLOR:
                if(!(enabled&(1<<ATTRIB_COLOR))) glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(a.size, a.format, vertexsize, buf);
                break;
            case ATTRIB_TEXCOORD0:
                if(!(enabled&(1<<ATTRIB_TEXCOORD0))) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(a.size, a.format, vertexsize, buf);
                break;
            case ATTRIB_TEXCOORD1:
                glClientActiveTexture_(GL_TEXTURE1_ARB);
                if(!(enabled&(1<<ATTRIB_TEXCOORD1))) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(a.size, a.format, vertexsize, buf);
                glClientActiveTexture_(GL_TEXTURE0_ARB);
                break;
            case ATTRIB_NORMAL:
                if(!(enabled&(1<<ATTRIB_NORMAL))) glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(a.format, vertexsize, buf);
                break;
            case ATTRIB_TANGENT:
                if(!(enabled&(1<<ATTRIB_TANGENT))) glEnableVertexAttribArray_(ATTRIB_TANGENT);
                glVertexAttribPointer_(ATTRIB_TANGENT, a.size, a.format, GL_TRUE, vertexsize, buf);
                break;
            case ATTRIB_BONEWEIGHT:
                if(!(enabled&(1<<ATTRIB_BONEWEIGHT))) glEnableVertexAttribArray_(ATTRIB_BONEWEIGHT);
                glVertexAttribPointer_(ATTRIB_BONEWEIGHT, a.size, a.format, GL_TRUE, vertexsize, buf);
                break;
            case ATTRIB_BONEINDEX:
                if(!(enabled&(1<<ATTRIB_BONEINDEX))) glEnableVertexAttribArray_(ATTRIB_BONEINDEX);
                glVertexAttribPointer_(ATTRIB_BONEINDEX, a.size, a.format, GL_FALSE, vertexsize, buf);
                break;
        }
        enabled |= 1<<a.type;
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
            case ATTRIB_TEXCOORD0:
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                break;
            case ATTRIB_TEXCOORD1:
                glClientActiveTexture_(GL_TEXTURE1_ARB);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glClientActiveTexture_(GL_TEXTURE0_ARB);
                break;
            case ATTRIB_NORMAL:
                glDisableClientState(GL_NORMAL_ARRAY);
                break;
            case ATTRIB_TANGENT:
            case ATTRIB_BONEWEIGHT:
            case ATTRIB_BONEINDEX:
                glDisableVertexAttribArray_(a.type);
                break;
        }
        enabled &= ~(1<<a.type);
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
        glDrawArrays(primtype, 0, numvertexes);
        data.setsize(0);
        return numvertexes;
    }

    void disable()
    {
        if(!enabled) return;
        if(enabled&(1<<ATTRIB_VERTEX)) glDisableClientState(GL_VERTEX_ARRAY);
        if(enabled&(1<<ATTRIB_COLOR)) glDisableClientState(GL_COLOR_ARRAY);
        if(enabled&(1<<ATTRIB_TEXCOORD0)) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(enabled&(1<<ATTRIB_TEXCOORD1))
        {
            glClientActiveTexture_(GL_TEXTURE1_ARB);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glClientActiveTexture_(GL_TEXTURE0_ARB);
        }
        if(enabled&(1<<ATTRIB_NORMAL)) glDisableClientState(GL_NORMAL_ARRAY);
        if(enabled&(1<<ATTRIB_TANGENT)) glDisableVertexAttribArray_(ATTRIB_TANGENT);
        if(enabled&(1<<ATTRIB_BONEWEIGHT)) glDisableVertexAttribArray_(ATTRIB_BONEWEIGHT);
        if(enabled&(1<<ATTRIB_BONEINDEX)) glDisableVertexAttribArray_(ATTRIB_BONEINDEX);
        enabled = 0;
        numlastattribs = lastattribmask = lastvertexsize = 0;
        lastbuf = NULL;
    }
#endif
}

