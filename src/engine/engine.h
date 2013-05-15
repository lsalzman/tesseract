#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "cube.h"
#include "world.h"

#ifndef STANDALONE

#include "octa.h"
#include "light.h"
#include "bih.h"
#include "texture.h"
#include "model.h"

extern dynent *player;
extern physent *camera1;                // special ent that acts as camera, same object as player1 in FPS mode

extern int worldscale, worldsize;
extern int mapversion;
extern char *maptitle;
extern vector<ushort> texmru;
extern int xtraverts, xtravertsva;
extern const ivec cubecoords[8];
extern const ivec facecoords[6][4];
extern const uchar fv[6][4];
extern const uchar fvmasks[64];
extern const uchar faceedgesidx[6][4];
extern bool inbetweenframes, renderedframe;

extern SDL_Window *screen;
extern int screenw, screenh;

extern vector<int> entgroup;

// rendertext
struct font
{
    struct charinfo
    {
        short x, y, w, h, offsetx, offsety, advance, tex;
    };

    char *name;
    vector<Texture *> texs;
    vector<charinfo> chars;
    int charoffset, defaultw, defaulth, scale;

    font() : name(NULL) {}
    ~font() { DELETEA(name); }
};

#define FONTH (curfont->scale)
#define FONTW (FONTH/2)
#define MINRESW 640
#define MINRESH 480

extern font *curfont;
extern Shader *textshader;
extern const matrix3x4 *textmatrix;

// texture
extern int hwtexsize, hwcubetexsize, hwmaxaniso, maxtexsize, hwtexunits, hwvtexunits;

extern Texture *textureload(const char *name, int clamp = 0, bool mipit = true, bool msg = true);
extern int texalign(const void *data, int w, int bpp);
extern bool floatformat(GLenum format);
extern void cleanuptexture(Texture *t);
extern void loadalphamask(Texture *t);
extern Texture *cubemapload(const char *name, bool mipit = true, bool msg = true, bool transient = false);
extern void drawcubemap(int size, const vec &o, float yaw, float pitch, const cubemapside &side);
extern void loadshaders();
extern void setuptexparameters(int tnum, const void *pixels, int clamp, int filter, GLenum format = GL_RGB, GLenum target = GL_TEXTURE_2D);
extern void createtexture(int tnum, int w, int h, const void *pixels, int clamp, int filter, GLenum component = GL_RGB, GLenum target = GL_TEXTURE_2D, int pw = 0, int ph = 0, int pitch = 0, bool resize = true, GLenum format = GL_FALSE);
extern void create3dtexture(int tnum, int w, int h, int d, const void *pixels, int clamp, int filter, GLenum component = GL_RGB, GLenum target = GL_TEXTURE_3D);
extern void blurtexture(int n, int bpp, int w, int h, uchar *dst, const uchar *src, int margin = 0);
extern void blurnormals(int n, int w, int h, bvec *dst, const bvec *src, int margin = 0);
extern GLuint setuppostfx(int w, int h, GLuint outfbo = 0);
extern void cleanuppostfx(bool fullclean = false);
extern void renderpostfx(GLuint outfbo = 0);
extern void initenvmaps();
extern void genenvmaps();
extern ushort closestenvmap(const vec &o);
extern ushort closestenvmap(int orient, int x, int y, int z, int size);
extern GLuint lookupenvmap(ushort emid);
extern GLuint lookupenvmap(Slot &slot);
extern bool reloadtexture(Texture &tex);
extern bool reloadtexture(const char *name);
extern void setuptexcompress();
extern void clearslots();
extern void compacteditvslots();
extern void compactmruvslots();
extern void compactvslots(cube *c, int n = 8);
extern void compactvslot(int &index);
extern int compactvslots();

// pvs
extern void clearpvs();
extern bool pvsoccluded(const ivec &bborigin, const ivec &bbsize);
extern bool pvsoccludedsphere(const vec &center, float radius);
extern bool waterpvsoccluded(int height);
extern void setviewcell(const vec &p);
extern void savepvs(stream *f);
extern void loadpvs(stream *f, int numpvs);
extern int getnumviewcells();

static inline bool pvsoccluded(const ivec &bborigin, int size)
{
    return pvsoccluded(bborigin, ivec(size, size, size));
}

// rendergl
extern bool hasVAO, hasTR, hasTSW, hasFBO, hasAFBO, hasDS, hasTF, hasCBF, hasS3TC, hasFXT1, hasAF, hasFBB, hasFBMS, hasTMS, hasMSS, hasFBMSBS, hasNVFBMSC, hasNVTMS, hasUBO, hasMBR, hasDB, hasTG, hasT4, hasTQ, hasPF, hasTRG, hasDBT, hasDC, hasDBGO, hasGPU4, hasGPU5, hasEAL;
extern int glversion, glslversion;

enum { DRAWTEX_NONE = 0, DRAWTEX_ENVMAP, DRAWTEX_MINIMAP, DRAWTEX_MODELPREVIEW };

extern int vieww, viewh;
extern int fov;
extern float curfov, fovy, aspect, forceaspect;
extern float nearplane;
extern int farplane;
extern int hdr;
extern bool hdrfloat;
extern float ldrscale, ldrscaleb;
extern int drawtex;
extern const glmatrix viewmatrix, invviewmatrix;
extern glmatrix cammatrix, projmatrix, camprojmatrix, invcammatrix, invcamprojmatrix, invprojmatrix;
extern int fog;
extern bvec fogcolor;
extern vec curfogcolor;
extern int wireframe;

extern int glerr;
extern void glerror(const char *file, int line, GLenum error);

#define GLERROR do { if(glerr) { GLenum error = glGetError(); if(error != GL_NO_ERROR) glerror(__FILE__, __LINE__, error); } } while(0)

extern void gl_checkextensions();
extern void gl_init(int w, int h);
extern void gl_resize(int w, int h);
extern void cleangl();
extern void gl_drawframe(int w, int h);
extern void gl_drawmainmenu(int w, int h);
extern void drawminimap();
extern void enablepolygonoffset(GLenum type);
extern void disablepolygonoffset(GLenum type);
extern bool calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2);
extern bool calcbbscissor(const ivec &bbmin, const ivec &bbmax, float &sx1, float &sy1, float &sx2, float &sy2);
extern bool calcspotscissor(const vec &origin, float radius, const vec &dir, int spot, const vec &spotx, const vec &spoty, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2);
extern void screenquad();
extern void screenquad(float sw, float sh);
extern void screenquadflipped(float sw, float sh);
extern void screenquad(float sw, float sh, float sw2, float sh2);
extern void screenquadoffset(float x, float y, float w, float h);
extern void screenquadoffset(float x, float y, float w, float h, float x2, float y2, float w2, float h2);
extern void hudquad(float x, float y, float w, float h, float tx = 0, float ty = 0, float tw = 1, float th = 1);
extern void debugquad(float x, float y, float w, float h, float tx = 0, float ty = 0, float tw = 1, float th = 1);
extern void recomputecamera();
extern float calcfrustumboundsphere(float nearplane, float farplane,  const vec &pos, const vec &view, vec &center);
extern void setfogcolor(const vec &v);
extern void zerofogcolor();
extern void resetfogcolor();
extern void renderavatar();
extern void writecrosshairs(stream *f);

namespace modelpreview
{
    extern void start(int x, int y, int w, int h, bool background = true, bool scissor = false);
    extern void end();
}

struct timer;
extern timer *begintimer(const char *name, bool gpu = true);
extern void endtimer(timer *t);

// renderextras
extern void render3dbox(vec &o, float tofloor, float toceil, float xradius, float yradius = 0);

// octa
extern cube *newcubes(uint face = F_EMPTY, int mat = MAT_AIR);
extern cubeext *growcubeext(cubeext *ext, int maxverts);
extern void setcubeext(cube &c, cubeext *ext);
extern cubeext *newcubeext(cube &c, int maxverts = 0, bool init = true);
extern void getcubevector(cube &c, int d, int x, int y, int z, ivec &p);
extern void setcubevector(cube &c, int d, int x, int y, int z, const ivec &p);
extern int familysize(const cube &c);
extern void freeocta(cube *c);
extern void discardchildren(cube &c, bool fixtex = false, int depth = 0);
extern void optiface(uchar *p, cube &c);
extern void validatec(cube *c, int size = 0);
extern bool isvalidcube(const cube &c);
extern ivec lu;
extern int lusize;
extern cube &lookupcube(int tx, int ty, int tz, int tsize = 0, ivec &ro = lu, int &rsize = lusize);
extern const cube *neighbourstack[32];
extern int neighbourdepth;
extern const cube &neighbourcube(const cube &c, int orient, int x, int y, int z, int size, ivec &ro = lu, int &rsize = lusize);
extern void resetclipplanes();
extern int getmippedtexture(const cube &p, int orient);
extern void forcemip(cube &c, bool fixtex = true);
extern bool subdividecube(cube &c, bool fullcheck=true, bool brighten=true);
extern int faceconvexity(const ivec v[4]);
extern int faceconvexity(const ivec v[4], int &vis);
extern int faceconvexity(const vertinfo *verts, int numverts, int size);
extern int faceconvexity(const cube &c, int orient);
extern void calcvert(const cube &c, int x, int y, int z, int size, ivec &vert, int i, bool solid = false);
extern void calcvert(const cube &c, int x, int y, int z, int size, vec &vert, int i, bool solid = false);
extern uint faceedges(const cube &c, int orient);
extern bool collapsedface(const cube &c, int orient);
extern bool touchingface(const cube &c, int orient);
extern bool flataxisface(const cube &c, int orient);
extern bool collideface(const cube &c, int orient);
extern int genclipplane(const cube &c, int i, vec *v, plane *clip);
extern void genclipplanes(const cube &c, int x, int y, int z, int size, clipplanes &p, bool collide = true);
extern bool visibleface(const cube &c, int orient, int x, int y, int z, int size, ushort mat = MAT_AIR, ushort nmat = MAT_AIR, ushort matmask = MATF_VOLUME);
extern int classifyface(const cube &c, int orient, int x, int y, int z, int size);
extern int visibletris(const cube &c, int orient, int x, int y, int z, int size, ushort nmat = MAT_ALPHA, ushort matmask = MAT_ALPHA);
extern int visibleorient(const cube &c, int orient);
extern void genfaceverts(const cube &c, int orient, ivec v[4]);
extern int calcmergedsize(int orient, const ivec &co, int size, const vertinfo *verts, int numverts);
extern void invalidatemerges(cube &c, const ivec &co, int size, bool msg);
extern void calcmerges();

extern int mergefaces(int orient, facebounds *m, int sz);
extern void mincubeface(const cube &cu, int orient, const ivec &o, int size, const facebounds &orig, facebounds &cf, ushort nmat = MAT_AIR, ushort matmask = MATF_VOLUME);

static inline uchar octantrectangleoverlap(const ivec &c, int size, const ivec &o, const ivec &s)
{
    uchar p = 0xFF; // bitmask of possible collisions with octants. 0 bit = 0 octant, etc
    ivec v(c);
    v.add(size);
    if(v.z <= o.z)     p &= 0xF0; // not in a -ve Z octant
    else if(v.z >= o.z+s.z) p &= 0x0F; // not in a +ve Z octant
    if(v.y <= o.y)     p &= 0xCC; // not in a -ve Y octant
    else if(v.y >= o.y+s.y) p &= 0x33; // etc..
    if(v.x <= o.x)     p &= 0xAA;
    else if(v.x >= o.x+s.x) p &= 0x55;
    return p;
}

static inline cubeext &ext(cube &c)
{
    return *(c.ext ? c.ext : newcubeext(c));
}

// renderlights

#define LIGHTTILE_MAXW 16
#define LIGHTTILE_MAXH 16

extern int lighttilealignw, lighttilealignh, lighttilevieww, lighttileviewh, lighttilew, lighttileh;

static inline void calctilebounds(float sx1, float sy1, float sx2, float sy2, int &bx1, int &by1, int &bx2, int &by2)
{
    int tx1 = max(int(floor(((sx1 + 1)*0.5f*vieww)/lighttilealignw)), 0),
        ty1 = max(int(floor(((sy1 + 1)*0.5f*viewh)/lighttilealignh)), 0),
        tx2 = min(int(ceil(((sx2 + 1)*0.5f*vieww)/lighttilealignw)), lighttilevieww),
        ty2 = min(int(ceil(((sy2 + 1)*0.5f*viewh)/lighttilealignh)), lighttileviewh);
    bx1 = ((tx1 + 1) * lighttilew - 1) / lighttilevieww;
    by1 = ((ty1 + 1) * lighttileh - 1) / lighttileviewh;
    bx2 = (tx2 * lighttilew + lighttilevieww - 1) / lighttilevieww;
    by2 = (ty2 * lighttileh + lighttileviewh - 1) / lighttileviewh;
}

static inline void masktiles(uint *tiles, float sx1, float sy1, float sx2, float sy2)
{
    int tx1, ty1, tx2, ty2;
    calctilebounds(sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
    for(int ty = ty1; ty < ty2; ty++) tiles[ty] |= ((1<<(tx2-tx1))-1)<<tx1;
}

enum { SM_NONE = 0, SM_REFLECT, SM_CUBEMAP, SM_CASCADE, SM_SPOT };

enum { L_NOSHADOW = 1<<0, L_NODYNSHADOW = 1<<1 };
 
extern int shadowmapping;

extern vec shadoworigin, shadowdir;
extern float shadowradius, shadowbias;
extern int shadowside, shadowspot;
extern glmatrix shadowmatrix;

extern void resetlights();
extern void collectlights();
extern void loaddeferredlightshaders();
extern void cleardeferredlightshaders();
extern void clearshadowcache();

extern void findshadowvas();
extern void findshadowmms();

extern int calcshadowinfo(const extentity &e, vec &origin, float &radius, vec &spotloc, int &spotangle, float &bias);
extern int dynamicshadowvabounds(int mask, vec &bbmin, vec &bbmax);
extern void rendershadowmapworld();
extern void batchshadowmapmodels();
extern void rendershadowatlas();
extern void renderrsmgeom(bool dyntex = false);
extern void renderradiancehints();
extern void clearradiancehintscache();

extern int calcbbsidemask(const vec &bbmin, const vec &bbmax, const vec &lightpos, float lightradius, float bias);
extern int calcspheresidemask(const vec &p, float radius, float bias);
extern int calctrisidemask(const vec &p1, const vec &p2, const vec &p3, float bias);
extern int cullfrustumsides(const vec &lightpos, float lightradius, float size, float border);
extern int calcbbcsmsplits(const ivec &bbmin, const ivec &bbmax);
extern int calcspherecsmsplits(const vec &center, float radius);
extern int calcbbrsmsplits(const ivec &bbmin, const ivec &bbmax);
extern int calcspherersmsplits(const vec &center, float radius);

static inline bool sphereinsidespot(const vec &dir, int spot, const vec &center, float radius)
{
    const vec2 &sc = sincos360[spot];
    float cdist = dir.dot(center), cradius = radius + sc.y*cdist;
    return sc.x*sc.x*(center.dot(center) - cdist*cdist) <= cradius*cradius;
}
static inline bool bbinsidespot(const vec &origin, const vec &dir, int spot, const ivec &bbmin, const ivec &bbmax)
{
    vec radius = ivec(bbmax).sub(bbmin).tovec().mul(0.5f), center = bbmin.tovec().add(radius);
    return sphereinsidespot(dir, spot, center.sub(origin), radius.magnitude());
}

extern glmatrix worldmatrix, screenmatrix;

extern int gw, gh, gdepthformat, gstencil, gdepthstencil;
extern GLuint gdepthtex, gcolortex, gnormaltex, gglowtex, gdepthrb, gstencilrb;
extern int msaasamples;
extern GLuint msdepthtex, mscolortex, msnormaltex, msglowtex, msdepthrb, msstencilrb;
extern vec2 msaapositions[16];
enum { AA_UNUSED = 0, AA_RESERVED, AA_LUMA, AA_VELOCITY, AA_SPLIT, AA_SPLIT_LUMA, AA_SPLIT_VELOCITY };

extern void cleanupgbuffer();
extern void initgbuffer();
extern void maskgbuffer(const char *mask);
extern void bindgdepth();
extern void preparegbuffer(bool depthclear = true);
extern void rendergbuffer(bool depthclear = true);
extern void shadegbuffer();
extern void shademinimap(const vec &color = vec(-1, -1, -1));
extern void shademodelpreview(int x, int y, int w, int h, bool background = true, bool scissor = false);
extern void rendertransparent();
extern void renderao();
extern void loadhdrshaders(int aa = AA_UNUSED);
extern void processhdr(GLuint outfbo = 0, int aa = AA_UNUSED);
extern void readhdr(int w, int h, GLenum format, GLenum type, void *dst, GLenum target = 0, GLuint tex = 0);
extern void setupframe(int w, int h);
extern void setupgbuffer(int w, int h);
extern GLuint shouldscale();
extern void doscale(int w, int h);
extern bool debuglights();
extern void cleanuplights();

// aa
extern glmatrix nojittermatrix, aamaskmatrix;

extern void setupaa(int w, int h);
extern void jitteraa(bool init = true);
extern bool maskedaa();
extern bool multisampledaa();
extern void setaavelocityparams(GLenum tmu = GL_TEXTURE0);
extern void setaamask(bool val);
extern void doaa(GLuint outfbo, void (*resolve)(GLuint, int));
extern bool debugaa();
extern void cleanupaa();

// ents
extern char *entname(entity &e);
extern bool haveselent();
extern undoblock *copyundoents(undoblock *u);
extern void pasteundoents(undoblock *u);

// octaedit
extern void cancelsel();
extern void rendertexturepanel(int w, int h);
extern void addundo(undoblock *u);
extern void commitchanges(bool force = false);
extern void rendereditcursor();
extern void tryedit();

// octarender
extern ivec worldmin, worldmax;
extern vector<tjoint> tjoints;

extern ushort encodenormal(const vec &n);
extern vec decodenormal(ushort norm);
extern void reduceslope(ivec &n);
extern void findtjoints();
extern void octarender();
extern void allchanged(bool load = false);
extern void clearvas(cube *c);
extern vtxarray *newva(int x, int y, int z, int size);
extern void destroyva(vtxarray *va, bool reparent = true);
extern void updatevabb(vtxarray *va, bool force = false);
extern void updatevabbs(bool force = false);

// renderva

extern int oqfrags;
extern float alphafrontsx1, alphafrontsx2, alphafrontsy1, alphafrontsy2, alphabacksx1, alphabacksx2, alphabacksy1, alphabacksy2, alpharefractsx1, alpharefractsx2, alpharefractsy1, alpharefractsy2;
extern uint alphatiles[LIGHTTILE_MAXH];

extern void visiblecubes(bool cull = true);
extern void setvfcP(const vec &bbmin = vec(-1, -1, -1), const vec &bbmax = vec(1, 1, 1));
extern void savevfcP();
extern void restorevfcP();
extern void rendergeom();
extern int findalphavas();
extern void renderrefractmask();
extern void renderalphageom(int side);
extern void rendermapmodels();
extern void renderoutline();

extern bool isfoggedsphere(float rad, const vec &cv);
extern int isvisiblesphere(float rad, const vec &cv);
extern bool bboccluded(const ivec &bo, const ivec &br);

extern int deferquery;
extern void flipqueries();
extern occludequery *newquery(void *owner);
extern bool checkquery(occludequery *query, bool nowait = false);
extern void resetqueries();
extern int getnumqueries();
extern void drawbb(const ivec &bo, const ivec &br, const vec &camera = camera1->o);

#define startquery(query) do { glBeginQuery_(GL_SAMPLES_PASSED, ((occludequery *)(query))->id); } while(0)
#define endquery(query) do { glEndQuery_(GL_SAMPLES_PASSED); } while(0)

struct shadowmesh;
extern void clearshadowmeshes();
extern void genshadowmeshes();
extern shadowmesh *findshadowmesh(int idx, extentity &e);
extern void rendershadowmesh(shadowmesh *m);

// dynlight

extern void updatedynlights();
extern int finddynlights();
extern bool getdynlight(int n, vec &o, float &radius, vec &color, vec &dir, int &spot);

// material

extern float matliquidsx1, matliquidsy1, matliquidsx2, matliquidsy2;
extern float matsolidsx1, matsolidsy1, matsolidsx2, matsolidsy2;
extern float matrefractsx1, matrefractsy1, matrefractsx2, matrefractsy2;
extern uint matliquidtiles[LIGHTTILE_MAXH], matsolidtiles[LIGHTTILE_MAXH];
extern vector<materialsurface> editsurfs, glasssurfs[4], watersurfs[4], waterfallsurfs[4], lavasurfs[4], lavafallsurfs[4];
extern const vec matnormals[6];

extern int showmat;

extern int findmaterial(const char *name);
extern const char *findmaterialname(int mat);
extern const char *getmaterialdesc(int mat, const char *prefix = "");
extern void genmatsurfs(const cube &c, int cx, int cy, int cz, int size, vector<materialsurface> &matsurfs);
extern void calcmatbb(vtxarray *va, int cx, int cy, int cz, int size, vector<materialsurface> &matsurfs);
extern int optimizematsurfs(materialsurface *matbuf, int matsurfs);
extern void setupmaterials(int start = 0, int len = 0);
extern int findmaterials();
extern void rendermaterialmask();
extern void renderliquidmaterials();
extern void rendersolidmaterials();
extern void rendereditmaterials();
extern void renderminimapmaterials();
extern int visiblematerial(const cube &c, int orient, int x, int y, int z, int size, ushort matmask = MATF_VOLUME);

// water
extern int vertwater, waterreflect, caustics;
extern float watersx1, watersy1, watersx2, watersy2;

#define GETMATIDXVAR(name, var, type) \
    type get##name##var(int mat) \
    { \
        switch(mat&MATF_INDEX) \
        { \
            default: case 0: return name##var; \
            case 1: return name##2##var; \
            case 2: return name##3##var; \
            case 3: return name##4##var; \
        } \
    }

extern const bvec &getwatercolor(int mat);
extern const bvec &getwaterdeepcolor(int mat);
extern const bvec &getwaterdeepfadecolor(int mat);
extern const bvec &getwaterrefractcolor(int mat);
extern const bvec &getwaterfallcolor(int mat);
extern const bvec &getwaterfallrefractcolor(int mat);
extern int getwaterfog(int mat);
extern int getwaterdeep(int mat);
extern int getwaterspec(int mat);
extern float getwaterrefract(int mat);
extern int getwaterfallspec(int mat);
extern float getwaterfallrefract(int mat);

extern const bvec &getlavacolor(int mat);
extern int getlavafog(int mat);
extern float getlavaglowmin(int mat);
extern float getlavaglowmax(int mat);
extern int getlavaspec(int mat);

extern const bvec &getglasscolor(int mat);
extern float getglassrefract(int mat);
extern int getglassspec(int mat);

extern void renderwater();
extern void renderwaterfalls();
extern void renderlava();
extern void renderlava(const materialsurface &m, Texture *tex, float scale);
extern void loadcaustics(bool force = false);
extern void renderwaterfog(int mat, float blend);
extern void preloadwatershaders(bool force = false);

// server
extern vector<const char *> gameargs;

extern void initserver(bool listen, bool dedicated);
extern void cleanupserver();
extern void serverslice(bool dedicated, uint timeout);
extern void updatetime();

extern ENetSocket connectmaster();
extern void localclienttoserver(int chan, ENetPacket *);
extern void localconnect();
extern bool serveroption(char *opt);

// serverbrowser
extern bool resolverwait(const char *name, ENetAddress *address);
extern int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &address);
extern void addserver(const char *name, int port = 0, const char *password = NULL, bool keep = false);
extern void writeservercfg();

// client
extern void localdisconnect(bool cleanup = true);
extern void localservertoclient(int chan, ENetPacket *packet);
extern void connectserv(const char *servername, int port, const char *serverpassword);
extern void abortconnect();
extern void clientkeepalive();

// command
extern hashset<ident> idents;
extern int identflags;

extern void clearoverrides();
extern void writecfg(const char *name = NULL);

extern void checksleep(int millis);
extern void clearsleep(bool clearoverrides = true);

// console
extern void processkey(int code, bool isdown);
extern void processtextinput(const char *str, int len);
extern int rendercommand(int x, int y, int w);
extern int renderconsole(int w, int h, int abovehud);
extern void conoutf(const char *s, ...) PRINTFARGS(1, 2);
extern void conoutf(int type, const char *s, ...) PRINTFARGS(2, 3);
extern void resetcomplete();
extern void complete(char *s, const char *cmdprefix);
const char *getkeyname(int code);
extern const char *addreleaseaction(char *s);
extern void writebinds(stream *f);
extern void writecompletions(stream *f);

// main
enum
{
    NOT_INITING = 0,
    INIT_GAME,
    INIT_LOAD,
    INIT_RESET
};
extern int initing, numcpus;

enum
{
    CHANGE_GFX     = 1<<0,
    CHANGE_SOUND   = 1<<1,
    CHANGE_SHADERS = 1<<2
};
extern bool initwarning(const char *desc, int level = INIT_RESET, int type = CHANGE_GFX);

extern bool grabinput, minimized;

extern void pushevent(const SDL_Event &e);
extern bool interceptkey(int sym);

extern float loadprogress;
extern void renderbackground(const char *caption = NULL, Texture *mapshot = NULL, const char *mapname = NULL, const char *mapinfo = NULL, bool restore = false, bool force = false);
extern void renderprogress(float bar, const char *text, GLuint tex = 0, bool background = false);

extern void getframemillis(float &avg, float &best, float &worst);
extern void getfps(int &fps, int &bestdiff, int &worstdiff);
extern void swapbuffers();
extern int getclockmillis();

enum { KR_CONSOLE = 1<<0, KR_GUI = 1<<1, KR_EDITMODE = 1<<2 };

extern void keyrepeat(bool on, int mask = ~0);

enum { TI_CONSOLE = 1<<0, TI_GUI = 1<<1 };

extern void textinput(bool on, int mask = ~0);

// menu
extern void menuprocess();
extern void addchange(const char *desc, int type);
extern void clearchanges(int type);

// physics
extern void mousemove(int dx, int dy);
extern bool pointincube(const clipplanes &p, const vec &v);
extern bool overlapsdynent(const vec &o, float radius);
extern void rotatebb(vec &center, vec &radius, int yaw, int pitch, int roll = 0);
extern float shadowray(const vec &o, const vec &ray, float radius, int mode, extentity *t = NULL);
struct ShadowRayCache;
extern ShadowRayCache *newshadowraycache();
extern void freeshadowraycache(ShadowRayCache *&cache);
extern void resetshadowraycache(ShadowRayCache *cache);
extern float shadowray(ShadowRayCache *cache, const vec &o, const vec &ray, float radius, int mode, extentity *t = NULL);

// world

extern vector<int> outsideents;

extern void entcancel();
extern void entitiesinoctanodes();
extern void attachentities();
extern void freeoctaentities(cube &c);
extern bool pointinsel(selinfo &sel, vec &o);

extern void resetmap();
extern void startmap(const char *name);

// rendermodel
struct mapmodelinfo { string name; model *m; };

extern float transmdlsx1, transmdlsy1, transmdlsx2, transmdlsy2;
extern uint transmdltiles[LIGHTTILE_MAXH];

extern void findanims(const char *pattern, vector<int> &anims);
extern void loadskin(const char *dir, const char *altdir, Texture *&skin, Texture *&masks);
extern mapmodelinfo *getmminfo(int i);
extern void resetmodelbatches();
extern void startmodelquery(occludequery *query);
extern void endmodelquery();
extern void rendershadowmodelbatches(bool dynmodel = true);
extern void shadowmaskbatchedmodels(bool dynshadow = true);
extern void rendermapmodelbatches();
extern void rendermodelbatches();
extern void rendertransparentmodelbatches();
extern void rendermapmodel(int idx, int anim, const vec &o, float yaw = 0, float pitch = 0, float roll = 0, int flags = MDL_CULL_VFC | MDL_CULL_DIST, int basetime = 0, float size = 1);
extern void clearbatchedmapmodels();
extern void preloadusedmapmodels(bool msg = false, bool bih = false);
extern int batcheddynamicmodels();
extern int batcheddynamicmodelbounds(int mask, vec &bbmin, vec &bbmax);
extern void cleanupmodels();

// renderparticles
extern void particleinit();
extern void clearparticles();
extern void clearparticleemitters();
extern void seedparticles();
extern void updateparticles();
extern void renderparticles();
extern bool printparticles(extentity &e, char *buf);

// decal
extern void initdecals();
extern void cleardecals();
extern void renderdecals();

// rendersky
extern int skytexture, skyshadow, explicitsky;

extern void drawskybox(int farplane);
extern bool limitsky();
extern bool renderexplicitsky(bool outline = false);

// 3dgui
extern void g3d_render();
extern bool g3d_windowhit(bool on, bool act);
extern bool g3d_key(int code, bool isdown);
extern bool g3d_input(const char *str, int len);

// menus
extern int mainmenu;

extern void clearmainmenu();
extern void g3d_mainmenu();

// sound
extern void clearmapsounds();
extern void checkmapsounds();
extern void updatesounds();
extern void preloadmapsounds();

extern void initmumble();
extern void closemumble();
extern void updatemumble();

// grass
extern void generategrass();
extern void rendergrass();
extern void cleanupgrass();

// blendmap
extern int blendpaintmode;

struct BlendMapCache;
extern BlendMapCache *newblendmapcache();
extern void freeblendmapcache(BlendMapCache *&cache);
extern bool setblendmaporigin(BlendMapCache *cache, const ivec &o, int size);
extern bool hasblendmap(BlendMapCache *cache);
extern uchar lookupblendmap(BlendMapCache *cache, const vec &pos);
extern void resetblendmap();
extern void enlargeblendmap();
extern void shrinkblendmap(int octant);
extern void optimizeblendmap();
extern void stoppaintblendmap();
extern void trypaintblendmap();
extern void renderblendbrush(GLuint tex, float x, float y, float w, float h);
extern void renderblendbrush();
extern bool loadblendmap(stream *f, int info);
extern void saveblendmap(stream *f);
extern uchar shouldsaveblendmap();
extern bool usesblendmap(int x1 = 0, int y1 = 0, int x2 = worldsize, int y2 = worldsize);
extern int calcblendlayer(int x1, int y1, int x2, int y2);
extern void updateblendtextures(int x1 = 0, int y1 = 0, int x2 = worldsize, int y2 = worldsize);
extern void bindblendtexture(const ivec &p);
extern void clearblendtextures();

// recorder

namespace recorder
{
    extern void stop();
    extern void capture();
    extern void cleanup();
}

#endif

#endif

