#include "engine.h"

struct QuadNode
{
    int x, y, size;
    uint filled;
    QuadNode *child[4];

    QuadNode(int x, int y, int size) : x(x), y(y), size(size), filled(0) { loopi(4) child[i] = 0; }

    void clear() 
    {
        loopi(4) DELETEP(child[i]);
    }
    
    ~QuadNode()
    {
        clear();
    }

    void insert(int mx, int my, int msize)
    {
        if(size == msize)
        {
            filled = 0xF;
            return;
        }
        int csize = size>>1, i = 0;
        if(mx >= x+csize) i |= 1;
        if(my >= y+csize) i |= 2;
        if(csize == msize)
        {
            filled |= (1 << i);
            return;
        }
        if(!child[i]) child[i] = new QuadNode(i&1 ? x+csize : x, i&2 ? y+csize : y, csize);
        child[i]->insert(mx, my, msize);
        loopj(4) if(child[j])
        {
            if(child[j]->filled == 0xF)
            {
                DELETEP(child[j]);
                filled |= (1 << j);
            }
        }
    }

    void genmatsurf(uchar mat, uchar orient, uchar flags, int x, int y, int z, int size, materialsurface *&matbuf)
    {
        materialsurface &m = *matbuf++;
        m.material = mat;
        m.orient = orient;
        m.flags = flags;
        m.csize = size;
        m.rsize = size;
        int dim = dimension(orient);
        m.o[C[dim]] = x;
        m.o[R[dim]] = y;
        m.o[dim] = z;
    }

    void genmatsurfs(uchar mat, uchar orient, uchar flags, int z, materialsurface *&matbuf)
    {
        if(filled == 0xF) genmatsurf(mat, orient, flags, x, y, z, size, matbuf);
        else if(filled)
        {
            int csize = size>>1;
            loopi(4) if(filled & (1 << i))
                genmatsurf(mat, orient, flags, i&1 ? x+csize : x, i&2 ? y+csize : y, z, csize, matbuf);
        }
        loopi(4) if(child[i]) child[i]->genmatsurfs(mat, orient, flags, z, matbuf);
    }
};

static void drawmaterial(const materialsurface &m, float offset)
{
    if(varray::data.empty())
    {
        varray::defattrib(varray::ATTRIB_VERTEX, 3, GL_FLOAT);
        varray::begin(GL_QUADS);
    }
    float x = m.o.x, y = m.o.y, z = m.o.z, csize = m.csize, rsize = m.rsize;
    switch(m.orient)
    {
#define GENFACEORIENT(orient, v0, v1, v2, v3) \
        case orient: v0 v1 v2 v3 break;
#define GENFACEVERT(orient, vert, mx,my,mz, sx,sy,sz) \
            varray::attrib<float>(mx sx, my sy, mz sz); 
        GENFACEVERTS(x, x, y, y, z, z, /**/, + csize, /**/, + rsize, + offset, - offset)
#undef GENFACEORIENT
#undef GENFACEVERT
    }
}

struct material
{
    const char *name;
    uchar id;
} materials[] = 
{
    {"air", MAT_AIR},
    {"water", MAT_WATER},
    {"clip", MAT_CLIP},
    {"glass", MAT_GLASS},
    {"noclip", MAT_NOCLIP},
    {"lava", MAT_LAVA},
    {"gameclip", MAT_GAMECLIP},
    {"death", MAT_DEATH},
    {"alpha", MAT_ALPHA}
};

int findmaterial(const char *name)
{
    loopi(sizeof(materials)/sizeof(material))
    {
        if(!strcmp(materials[i].name, name)) return materials[i].id;
    } 
    return -1;
}  
    
int visiblematerial(cube &c, int orient, int x, int y, int z, int size, uchar matmask)
{   
    uchar mat = c.material&matmask;
    switch(mat)
    {
    case MAT_AIR:
         break;

    case MAT_LAVA:
    case MAT_WATER:
        if(visibleface(c, orient, x, y, z, size, mat, MAT_AIR, matmask))
            return (orient != O_BOTTOM ? MATSURF_VISIBLE : MATSURF_EDIT_ONLY);
        break;

    case MAT_GLASS:
        if(visibleface(c, orient, x, y, z, size, MAT_GLASS, MAT_AIR, matmask))
            return MATSURF_VISIBLE;
        break;

    default:
        if(visibleface(c, orient, x, y, z, size, mat, MAT_AIR, matmask))
            return MATSURF_EDIT_ONLY;
        break;
    }
    return MATSURF_NOT_VISIBLE;
}

void genmatsurfs(cube &c, int cx, int cy, int cz, int size, vector<materialsurface> &matsurfs)
{
    loopi(6)
    {
        static uchar matmasks[] = { MATF_VOLUME, MATF_CLIP, MAT_DEATH, MAT_ALPHA };
        loopj(sizeof(matmasks)/sizeof(matmasks[0]))
        {
            int matmask = matmasks[j];
            int vis = visiblematerial(c, i, cx, cy, cz, size, matmask);
            if(vis != MATSURF_NOT_VISIBLE) 
            {
                materialsurface m;
                m.material = c.material&matmask;
                m.orient = i;
                m.flags = vis == MATSURF_EDIT_ONLY ? materialsurface::F_EDIT : 0;
                m.o = ivec(cx, cy, cz);
                m.csize = m.rsize = size;
                if(dimcoord(i)) m.o[dimension(i)] += size;
                matsurfs.add(m);
                break;
            }
        }
    }
}

void calcmatbb(vtxarray *va, int cx, int cy, int cz, int size, vector<materialsurface> &matsurfs)
{
    va->matmax = ivec(cx, cy, cz);
    va->matmin = ivec(va->matmax).add(size);
    loopv(matsurfs)
    {
        materialsurface &m = matsurfs[i];
        switch(m.material)
        {
            case MAT_WATER:
            case MAT_GLASS:
            case MAT_LAVA:
                if(m.flags&materialsurface::F_EDIT) continue;
                break;

            default:
                continue;
        }

        int dim = dimension(m.orient),
            r = R[dim],
            c = C[dim];
        va->matmin[dim] = min(va->matmin[dim], m.o[dim]);
        va->matmax[dim] = max(va->matmax[dim], m.o[dim]);

        va->matmin[r] = min(va->matmin[r], m.o[r]);
        va->matmax[r] = max(va->matmax[r], m.o[r] + m.rsize);

        va->matmin[c] = min(va->matmin[c], m.o[c]);
        va->matmax[c] = max(va->matmax[c], m.o[c] + m.csize);
    }
}

static inline bool mergematcmp(const materialsurface &x, const materialsurface &y)
{
    int dim = dimension(x.orient), c = C[dim], r = R[dim];
    if(x.o[r] + x.rsize < y.o[r] + y.rsize) return true;
    if(x.o[r] + x.rsize > y.o[r] + y.rsize) return false;
    return x.o[c] < y.o[c];
}

static int mergematr(materialsurface *m, int sz, materialsurface &n)
{
    int dim = dimension(n.orient), c = C[dim], r = R[dim];
    for(int i = sz-1; i >= 0; --i)
    {
        if(m[i].o[r] + m[i].rsize < n.o[r]) break;
        if(m[i].o[r] + m[i].rsize == n.o[r] && m[i].o[c] == n.o[c] && m[i].csize == n.csize)
        {
            n.o[r] = m[i].o[r];
            n.rsize += m[i].rsize;
            memmove(&m[i], &m[i+1], (sz - (i+1)) * sizeof(materialsurface));
            return 1;
        }
    }
    return 0;
}

static int mergematc(materialsurface &m, materialsurface &n)
{
    int dim = dimension(n.orient), c = C[dim], r = R[dim];
    if(m.o[r] == n.o[r] && m.rsize == n.rsize && m.o[c] + m.csize == n.o[c])
    {
        n.o[c] = m.o[c];
        n.csize += m.csize;
        return 1;
    }
    return 0;
}

static int mergemat(materialsurface *m, int sz, materialsurface &n)
{
    for(bool merged = false; sz; merged = true)
    {
        int rmerged = mergematr(m, sz, n);
        sz -= rmerged;
        if(!rmerged && merged) break;
        if(!sz) break;
        int cmerged = mergematc(m[sz-1], n);
        sz -= cmerged;
        if(!cmerged) break;
    }
    m[sz++] = n;
    return sz;
}

static int mergemats(materialsurface *m, int sz)
{
    quicksort(m, sz, mergematcmp);

    int nsz = 0;
    loopi(sz) nsz = mergemat(m, nsz, m[i]);
    return nsz;
}

static inline bool optmatcmp(const materialsurface &x, const materialsurface &y)
{
    if(x.material < y.material) return true;
    if(x.material > y.material) return false;
    if(x.orient > y.orient) return true;
    if(x.orient < y.orient) return false;
    int dim = dimension(x.orient);
    return x.o[dim] < y.o[dim];
}

VARF(optmats, 0, 1, 1, allchanged());

int optimizematsurfs(materialsurface *matbuf, int matsurfs)
{
    quicksort(matbuf, matsurfs, optmatcmp);
    if(!optmats) return matsurfs;
    materialsurface *cur = matbuf, *end = matbuf+matsurfs;
    while(cur < end)
    {
         materialsurface *start = cur++;
         int dim = dimension(start->orient);
         while(cur < end &&
               cur->material == start->material &&
               cur->orient == start->orient &&
               cur->flags == start->flags && 
               cur->o[dim] == start->o[dim])
            ++cur;
         if(!isliquid(start->material) || start->orient != O_TOP || !vertwater)
         {
            if(start!=matbuf) memmove(matbuf, start, (cur-start)*sizeof(materialsurface));
            matbuf += mergemats(matbuf, cur-start);
         }
         else if(cur-start>=4)
         {
            QuadNode vmats(0, 0, worldsize);
            loopi(cur-start) vmats.insert(start[i].o[C[dim]], start[i].o[R[dim]], start[i].csize);
            vmats.genmatsurfs(start->material, start->orient, start->flags, start->o[dim], matbuf);
         }
         else
         {
            if(start!=matbuf) memmove(matbuf, start, (cur-start)*sizeof(materialsurface));
            matbuf += cur-start;
         }
    }
    return matsurfs - (end-matbuf);
}

void preloadglassshaders(bool force = false)
{
    static bool needglass = false;
    if(force) needglass = true;
    if(!needglass) return;

    useshaderbyname("glass");

    extern int glassenv;
    if(glassenv) useshaderbyname("glassenv");
}

extern vector<vtxarray *> valist;

void setupmaterials(int start, int len)
{
    int hasmat = 0;
    if(!len) len = valist.length();
    for(int i = start; i < len; i++) 
    {
        vtxarray *va = valist[i];
        materialsurface *skip = NULL;
        loopj(va->matsurfs)
        {
            materialsurface &m = va->matbuf[j];
            if(isliquid(m.material) && m.orient!=O_BOTTOM && m.orient!=O_TOP)
            {
                m.ends = 0;
                int dim = dimension(m.orient), coord = dimcoord(m.orient);
                ivec o(m.o);
                o.z -= 1;
                o[dim] += coord ? 1 : -1;
                int minc = o[dim^1], maxc = minc + (C[dim]==2 ? m.rsize : m.csize);
                ivec co;
                int csize;
                while(o[dim^1] < maxc)
                {
                    cube &c = lookupcube(o.x, o.y, o.z, 0, co, csize);
                    if(isliquid(c.material&MATF_VOLUME)) { m.ends |= 1; break; }
                    o[dim^1] += csize;
                }
                o[dim^1] = minc;
                o.z += R[dim]==2 ? m.rsize : m.csize;
                o[dim] -= coord ? 2 : -2;
                while(o[dim^1] < maxc)
                {
                    cube &c = lookupcube(o.x, o.y, o.z, 0, co, csize);
                    if(visiblematerial(c, O_TOP, co.x, co.y, co.z, csize)) { m.ends |= 2; break; }
                    o[dim^1] += csize;
                }
            }
            else if(m.material==MAT_GLASS)
            {
                if(!hasCM) m.envmap = EMID_NONE;
                else
                {
                    int dim = dimension(m.orient);
                    vec center(m.o.tovec());
                    center[R[dim]] += m.rsize/2;
                    center[C[dim]] += m.csize/2;
                    m.envmap = closestenvmap(center);
                }
            }
            if(m.material&MATF_VOLUME) hasmat |= 1<<m.material;
            m.skip = 0;
            if(skip && m.material == skip->material && m.orient == skip->orient && skip->skip < 0xFF)
                skip->skip++;
            else 
                skip = &m;
        }
    }
    if(hasmat&(1<<MAT_WATER))
    {
        loadcaustics(true);
        preloadwatershaders(true);
        lookupmaterialslot(MAT_WATER);
    }
    if(hasmat&(1<<MAT_LAVA)) 
    {
        useshaderbyname("lava");
        lookupmaterialslot(MAT_LAVA);
    }
    if(hasmat&(1<<MAT_GLASS)) 
    {
        preloadglassshaders(true);
        lookupmaterialslot(MAT_GLASS);
    }
}

VARP(showmat, 0, 1, 1);

static int sortdim[3];
static ivec sortorigin;

static inline bool editmatcmp(const materialsurface &x, const materialsurface &y)
{
    int xdim = dimension(x.orient), ydim = dimension(y.orient);
    loopi(3)
    {
        int dim = sortdim[i], xmin, xmax, ymin, ymax;
        xmin = xmax = x.o[dim];
        if(dim==C[xdim]) xmax += x.csize;
        else if(dim==R[xdim]) xmax += x.rsize;
        ymin = ymax = y.o[dim];
        if(dim==C[ydim]) ymax += y.csize;
        else if(dim==R[ydim]) ymax += y.rsize;
        if(xmax > ymin && ymax > xmin) continue;
        int c = sortorigin[dim];
        if(c > xmin && c < xmax) return true;
        if(c > ymin && c < ymax) return false;
        xmin = abs(xmin - c);
        xmax = abs(xmax - c);
        ymin = abs(ymin - c);
        ymax = abs(ymax - c);
        if(max(xmin, xmax) <= min(ymin, ymax)) return true;
        else if(max(ymin, ymax) <= min(xmin, xmax)) return false;
    }
    if(x.material < y.material) return true;
    //if(x.material > y.material) return false;
    return false;
}

void sorteditmaterials()
{
    sortorigin = ivec(camera1->o);
    vec dir(fabs(camdir.x), fabs(camdir.y), fabs(camdir.z));
    loopi(3) sortdim[i] = i;
    if(dir[sortdim[2]] > dir[sortdim[1]]) swap(sortdim[2], sortdim[1]);
    if(dir[sortdim[1]] > dir[sortdim[0]]) swap(sortdim[1], sortdim[0]);
    if(dir[sortdim[2]] > dir[sortdim[1]]) swap(sortdim[2], sortdim[1]);
    editsurfs.sort(editmatcmp);
}

void rendermatgrid()
{
    enablepolygonoffset(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int lastmat = -1;
    float colorscale = (hdr ? 0.5f : 1)/255.0f;
    loopvrev(editsurfs)
    {
        materialsurface &m = editsurfs[i];
        if(m.material != lastmat)
        {
            xtraverts += varray::end();
            bvec color;
            switch(m.material)
            {
                case MAT_WATER:    color = bvec( 0,  0, 85); break; // blue
                case MAT_CLIP:     color = bvec(85,  0,  0); break; // red
                case MAT_GLASS:    color = bvec( 0, 85, 85); break; // cyan
                case MAT_NOCLIP:   color = bvec( 0, 85,  0); break; // green
                case MAT_LAVA:     color = bvec(85, 40,  0); break; // orange
                case MAT_GAMECLIP: color = bvec(85, 85,  0); break; // yellow
                case MAT_DEATH:    color = bvec(40, 40, 40); break; // black
                case MAT_ALPHA:    color = bvec(85,  0, 85); break; // pink
                default: continue;
            }
            glColor3f(color.x*colorscale, color.y*colorscale, color.z*colorscale);
            lastmat = m.material;
        }
        drawmaterial(m, -0.1f);
    }
    xtraverts += varray::end();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    disablepolygonoffset(GL_POLYGON_OFFSET_LINE);
}

static float glassxscale = 0, glassyscale = 0;

static void drawglass(const materialsurface &m, float offset, const vec *normal = NULL)
{
    if(varray::data.empty())
    {
        varray::defattrib(varray::ATTRIB_VERTEX, 3, GL_FLOAT);
        if(normal) varray::defattrib(varray::ATTRIB_NORMAL, 3, GL_FLOAT);
        varray::defattrib(varray::ATTRIB_TEXCOORD0, 2, GL_FLOAT);
        varray::begin(GL_QUADS);
    }
    #define GENFACEORIENT(orient, v0, v1, v2, v3) \
        case orient: v0 v1 v2 v3 break;
    #undef GENFACEVERTX
    #define GENFACEVERTX(orient, vert, mx,my,mz, sx,sy,sz) \
        { \
            vec v(mx sx, my sy, mz sz); \
            varray::attrib<float>(v.x, v.y, v.z); \
            GENFACENORMAL \
            varray::attrib<float>(glassxscale*v.y, -glassyscale*v.z); \
        }
    #undef GENFACEVERTY
    #define GENFACEVERTY(orient, vert, mx,my,mz, sx,sy,sz) \
        { \
            vec v(mx sx, my sy, mz sz); \
            varray::attrib<float>(v.x, v.y, v.z); \
            GENFACENORMAL \
            varray::attrib<float>(glassxscale*v.x, -glassyscale*v.z); \
        }
    #undef GENFACEVERTZ
    #define GENFACEVERTZ(orient, vert, mx,my,mz, sx,sy,sz) \
        { \
            vec v(mx sx, my sy, mz sz); \
            varray::attrib<float>(v.x, v.y, v.z); \
            GENFACENORMAL \
            varray::attrib<float>(glassxscale*v.x, glassyscale*v.y); \
        }
    #define GENFACENORMAL varray::attrib<float>(n.x, n.y, n.z);
    float x = m.o.x, y = m.o.y, z = m.o.z, csize = m.csize, rsize = m.rsize;
    if(normal)
    {
        vec n = *normal;
        switch(m.orient) { GENFACEVERTS(x, x, y, y, z, z, /**/, + csize, /**/, + rsize, + offset, - offset) }     
    }
    #undef GENFACENORMAL
    #define GENFACENORMAL
    else switch(m.orient) { GENFACEVERTS(x, x, y, y, z, z, /**/, + csize, /**/, + rsize, + offset, - offset) }
    #undef GENFACENORMAL
    #undef GENFACEORIENT
    #undef GENFACEVERTX
    #define GENFACEVERTX(o,n, x,y,z, xv,yv,zv) GENFACEVERT(o,n, x,y,z, xv,yv,zv)
    #undef GENFACEVERTY
    #define GENFACEVERTY(o,n, x,y,z, xv,yv,zv) GENFACEVERT(o,n, x,y,z, xv,yv,zv)
    #undef GENFACEVERTZ
    #define GENFACEVERTZ(o,n, x,y,z, xv,yv,zv) GENFACEVERT(o,n, x,y,z, xv,yv,zv)
}

vector<materialsurface> editsurfs, glasssurfs, watersurfs, waterfallsurfs, lavasurfs, lavafallsurfs;

float matsx1 = -1, matsy1 = -1, matsx2 = 1, matsy2 = 1, matrefractsx1 = -1, matrefractsy1 = -1, matrefractsx2 = 1, matrefractsy2 = 1;
uint mattiles[LIGHTTILE_H];

extern vtxarray *visibleva;

bool findmaterials()
{
    editsurfs.setsize(0);
    glasssurfs.setsize(0);
    watersurfs.setsize(0);
    waterfallsurfs.setsize(0);
    lavasurfs.setsize(0);
    lavafallsurfs.setsize(0);
    matsx1 = matsy1 = matrefractsx1 = matrefractsy1 = 1;
    matsx2 = matsy2 = matrefractsx2 = matrefractsy2 = -1;
    memset(mattiles, 0, sizeof(mattiles));
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(!va->matsurfs || va->occluded >= OCCLUDE_BB || va->curvfc >= VFC_FOGGED) continue;
        if(editmode && showmat && !envmapping) 
        {
            loopi(va->matsurfs) editsurfs.add(va->matbuf[i]);
            continue;
        }
        loopi(va->matsurfs)
        {
            materialsurface &m = va->matbuf[i];
            if(m.flags&materialsurface::F_EDIT) continue;
            switch(m.material)
            {
                case MAT_GLASS: glasssurfs.add(m); break;
                case MAT_LAVA: if(m.orient == O_TOP) lavasurfs.add(m); else lavafallsurfs.add(m); break;
                case MAT_WATER: if(m.orient == O_TOP) watersurfs.add(m); else waterfallsurfs.add(m); break;
            }
        }
        float sx1, sy1, sx2, sy2;
        if(!calcbbscissor(va->matmin, va->matmax, sx1, sy1, sx2, sy2)) continue;
        matsx1 = min(matsx1, sx1);
        matsy1 = min(matsy1, sy1);
        matsx2 = max(matsx2, sx2);
        matsy2 = max(matsy2, sy2);
        if(va->matmask&((1<<MAT_GLASS)|(1<<MAT_WATER)))
        {
            matrefractsx1 = min(matrefractsx1, sx1);
            matrefractsy1 = min(matrefractsy1, sy1);
            matrefractsx2 = max(matrefractsx2, sx2);
            matrefractsy2 = max(matrefractsy2, sy2);
        }
        masktiles(mattiles, sx1, sy1, sx2, sy2);
    }
    return glasssurfs.length() || watersurfs.length() || waterfallsurfs.length() || lavasurfs.length() || lavafallsurfs.length();
}

void rendermaterialmask()
{
    if(glasssurfs.empty() && watersurfs.empty() && waterfallsurfs.empty()) return;
    glDisable(GL_CULL_FACE);
    varray::enable();
    loopv(glasssurfs) drawmaterial(glasssurfs[i], 0.1f);
    loopv(watersurfs) drawmaterial(watersurfs[i], WATER_OFFSET);
    loopv(waterfallsurfs) drawmaterial(waterfallsurfs[i], 0.1f);
    xtraverts += varray::end();
    varray::disable();
    glEnable(GL_CULL_FACE);
}

extern const vec matnormals[6] =
{
    vec(-1, 0, 0),
    vec( 1, 0, 0),
    vec(0, -1, 0),
    vec(0,  1, 0),
    vec(0, 0, -1),
    vec(0, 0,  1)
};

bvec glasscolor(0xB0, 0xD8, 0xFF);
HVARFR(glasscolour, 0, 0xB0D8FF, 0xFFFFFF,
{
    if(!glasscolour) glasscolour = 0xB0D8FF;
    glasscolor = bvec((glasscolour>>16)&0xFF, (glasscolour>>8)&0xFF, glasscolour&0xFF);
});
FVARR(glassrefract, 0, 0.1f, 1e3f);
VARR(glassspec, 0, 150, 200);
VARFP(glassenv, 0, 1, 1, preloadglassshaders());

void renderglass()
{
    if(glasssurfs.empty()) return;

    MSlot &gslot = lookupmaterialslot(MAT_GLASS);

    Texture *tex = gslot.sts.inrange(0) ? gslot.sts[0].t : notexture;
    glassxscale = TEX_SCALE/(tex->xs*gslot.scale);
    glassyscale = TEX_SCALE/(tex->ys*gslot.scale);

    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glActiveTexture_(GL_TEXTURE0_ARB);

    float colorscale = 1/255.0f;
    GLOBALPARAM(glasscolor, (glasscolor.x*colorscale, glasscolor.y*colorscale, glasscolor.z*colorscale));
    GLOBALPARAM(glassrefract, (glassrefract*viewh));
    GLOBALPARAM(glassspec, (0.5f*glassspec/100.0f));

    short envmap = EMID_NONE;
    if(!glassenv) SETSHADER(glass);
    loopv(glasssurfs)
    {
        materialsurface &m = glasssurfs[i];
        if(m.envmap != envmap && glassenv)
        {
            xtraverts += varray::end();
            if(m.envmap != EMID_NONE && glassenv) SETSHADER(glassenv);
            else SETSHADER(glass);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, lookupenvmap(m.envmap));
            envmap = m.envmap;
        }
        drawglass(m, 0.1f, &matnormals[m.orient]);
    }    
    xtraverts += varray::end();
}

void rendermaterials()
{
    glDisable(GL_CULL_FACE);
    varray::enable();

    GLOBALPARAM(camera, (camera1->o));
    GLOBALPARAM(millis, (lastmillis/1000.0f));

    renderlava();
    renderwater();
    renderwaterfalls();
    renderglass();

    varray::disable();
    glEnable(GL_CULL_FACE);
}

void rendereditmaterials()
{
    if(editsurfs.empty()) return;

    sorteditmaterials();

    glDisable(GL_CULL_FACE);
    varray::enable();

    foggednotextureshader->set();

    static const float zerofog[4] = { 0, 0, 0, 1 };
    float oldfogc[4];
    glGetFloatv(GL_FOG_COLOR, oldfogc);
    glFogfv(GL_FOG_COLOR, zerofog);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    glEnable(GL_BLEND);

    int lastmat = -1;
    loopv(editsurfs)
    {
        const materialsurface &m = editsurfs[i];
        if(lastmat!=m.material)
        {
            xtraverts += varray::end();
            bvec color;
            switch(m.material)
            {
                case MAT_WATER:    color = bvec(255, 128,   0); break; // blue
                case MAT_CLIP:     color = bvec(  0, 255, 255); break; // red
                case MAT_GLASS:    color = bvec(255,   0,   0); break; // cyan
                case MAT_NOCLIP:   color = bvec(255,   0, 255); break; // green
                case MAT_LAVA:     color = bvec(  0, 128, 255); break; // orange
                case MAT_GAMECLIP: color = bvec(  0,   0, 255); break; // yellow
                case MAT_DEATH:    color = bvec(192, 192, 192); break; // black
                case MAT_ALPHA:    color = bvec(  0, 255,   0); break; // pink
                default: continue;
            }
            glColor3ub(color.x, color.y, color.z);
            lastmat = m.material;
        }
        drawmaterial(m, -0.1f);
    }

    xtraverts += varray::end();

    glDisable(GL_BLEND);
    glFogfv(GL_FOG_COLOR, oldfogc);

    foggedlineshader->set();

    rendermatgrid();
    
    varray::disable();
    glEnable(GL_CULL_FACE);
}

void renderminimapmaterials()
{
    glDisable(GL_CULL_FACE);
    varray::enable();

    GLOBALPARAM(camera, (camera1->o));
    GLOBALPARAM(millis, (lastmillis/1000.0f));

    renderlava();
    renderwater();

    varray::disable();
    glEnable(GL_CULL_FACE);
}

