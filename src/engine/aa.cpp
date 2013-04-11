#include "engine.h"

extern int tqaamovemask, tqaamovemaskreduce, tqaamovemaskprec;

int tqaaframe = 0;
GLuint tqaaprevtex = 0, tqaacurtex = 0, tqaamasktex = 0, tqaafbo[3] = { 0, 0, 0 };
glmatrix tqaaprevmvp;

void loadtqaashaders()
{
    loadhdrshaders(AA_VELOCITY);

    if(tqaamovemask) useshaderbyname("tqaamaskmovement");
    useshaderbyname(tqaamovemask ? "tqaaresolvemasked" : "tqaaresolve");
}

void setuptqaa(int w, int h)
{
    if(!tqaaprevtex) glGenTextures(1, &tqaaprevtex);
    if(!tqaacurtex) glGenTextures(1, &tqaacurtex);
    createtexture(tqaaprevtex, w, h, NULL, 3, 1, GL_RGBA8, GL_TEXTURE_RECTANGLE);
    createtexture(tqaacurtex, w, h, NULL, 3, 1, GL_RGBA8, GL_TEXTURE_RECTANGLE);
    if(tqaamovemask)
    {
        if(!tqaamasktex) glGenTextures(1, &tqaamasktex);
        int maskw = (w + (1<<tqaamovemaskreduce) - 1) >> tqaamovemaskreduce, maskh = (h + (1<<tqaamovemaskreduce) - 1) >> tqaamovemaskreduce;
        createtexture(tqaamasktex, maskw, maskh, NULL, 3, 1, hasTRG && tqaamovemaskprec ? GL_R8 : GL_RGBA8, GL_TEXTURE_RECTANGLE);
    }
    loopi(tqaamovemask ? 3 : 2)
    {
        if(!tqaafbo[i]) glGenFramebuffers_(1, &tqaafbo[i]);
        glBindFramebuffer_(GL_FRAMEBUFFER, tqaafbo[i]);
        GLuint tex = 0;
        switch(i)
        {
            case 0: tex = tqaacurtex; break;
            case 1: tex = tqaaprevtex; break;
            case 2: tex = tqaamasktex; break;
        }
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, tex, 0);
        if(i <= 1) bindgdepth();
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating TQAA buffer!");
    }
    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    tqaaprevmvp.identity();

    loadtqaashaders();
}

void cleanuptqaa()
{
    if(tqaaprevtex) { glDeleteTextures(1, &tqaaprevtex); tqaaprevtex = 0; }
    if(tqaacurtex) { glDeleteTextures(1, &tqaacurtex); tqaacurtex = 0; }
    if(tqaamasktex) { glDeleteTextures(1, &tqaamasktex); tqaamasktex = 0; }
    loopi(3) if(tqaafbo[i]) { glDeleteFramebuffers_(1, &tqaafbo[i]); tqaafbo[i] = 0; }

    tqaaframe = 0;
}

VARFP(tqaa, 0, 0, 1, cleanupaa());
FVAR(tqaareproject, 0, 300, 1e3f);
FVAR(tqaareprojectscale, 0, 4, 1e3f);
VARFP(tqaamovemask, 0, 1, 1, cleanuptqaa());
VARFP(tqaamovemaskreduce, 0, 0, 2, cleanuptqaa());
VARFP(tqaamovemaskprec, 0, 1, 1, cleanuptqaa());
VARP(tqaaquincunx, 0, 1, 1);

void setaavelocityparams(GLenum tmu)
{
    if(tmu!=GL_TEXTURE0) glActiveTexture_(tmu);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
    glmatrix reproject;
    reproject.mul(tqaaframe ? tqaaprevmvp : screenmatrix, worldmatrix);
    vec2 jitter = tqaaframe&1 ? vec2(0.5f, 0.5f) : vec2(-0.5f, -0.5f);
    if(multisampledaa()) { jitter.x *= 0.5f; jitter.y *= -0.5f; }
    if(tqaaframe) reproject.jitter(jitter.x, jitter.y);
    LOCALPARAM(reprojectmatrix, reproject);
    float maxvel = sqrtf(vieww*vieww + viewh*viewh)/tqaareproject;
    LOCALPARAMF(maxvelocity, (maxvel, 1/maxvel, tqaareprojectscale));
    if(tmu!=GL_TEXTURE0) glActiveTexture_(GL_TEXTURE0);
}

void packtqaa()
{
    if(tqaamovemask)
    {
        int maskw = (vieww + (1<<tqaamovemaskreduce) - 1) >> tqaamovemaskreduce, maskh = (viewh + (1<<tqaamovemaskreduce) - 1) >> tqaamovemaskreduce;
        glBindFramebuffer_(GL_FRAMEBUFFER, tqaafbo[2]);
        if(!tqaaframe)
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        SETSHADER(tqaamaskmovement);
        if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msnormaltex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gnormaltex);
        if(tqaamovemaskreduce)
        {
            glViewport(0, 0, maskw, maskh);
            if(!msaasamples)
            {
                glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
        }
        glBlendFunc(GL_ONE, GL_SRC_ALPHA);
        glEnable(GL_BLEND);
        screenquad(maskw<<tqaamovemaskreduce, maskh<<tqaamovemaskreduce);
        glDisable(GL_BLEND);
        if(tqaamovemaskreduce)
        {
            if(!msaasamples)
            {
                glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }
            glViewport(0, 0, vieww, viewh);
        }
    }
}

void resolvetqaa(GLuint outfbo)
{
    glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
    if(tqaamovemask)
    {
        SETSHADER(tqaaresolvemasked);
        LOCALPARAMF(movemaskscale, (1/float(1<<tqaamovemaskreduce)));
    }
    else SETSHADER(tqaaresolve);
    glBindTexture(GL_TEXTURE_RECTANGLE, tqaacurtex);
    glActiveTexture_(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE, tqaaframe ? tqaaprevtex : tqaacurtex);
    setaavelocityparams(GL_TEXTURE2);
    if(tqaamovemask)
    {
       glActiveTexture_(GL_TEXTURE3);
       glBindTexture(GL_TEXTURE_RECTANGLE, tqaamasktex);
    }
    glActiveTexture_(GL_TEXTURE0);
    vec4 quincunx(0, 0, 0, 0);
    if(tqaaquincunx) quincunx = tqaaframe&1 ? vec4(0.25f, 0.25f, -0.25f, -0.25f) : vec4(-0.25f, -0.25f, 0.25f, 0.25f);
    if(multisampledaa()) { quincunx.x *= 0.5f; quincunx.y *= -0.5f; quincunx.z *= 0.5f; quincunx.w *= -0.5f; }
    LOCALPARAM(quincunx, quincunx);
    screenquad(vieww, viewh, vieww/float(1<<tqaamovemaskreduce), viewh/float(1<<tqaamovemaskreduce));

    swap(tqaafbo[0], tqaafbo[1]);
    swap(tqaacurtex, tqaaprevtex);
    tqaaprevmvp = screenmatrix;
    tqaaframe++;
}

void dotqaa(GLuint outfbo = 0)
{
    timer *tqaatimer = begintimer("tqaa");

    packtqaa();
    resolvetqaa(outfbo);

    endtimer(tqaatimer);
}

GLuint fxaafbo = 0, fxaatex = 0;

extern int fxaaquality, fxaagreenluma;

static Shader *fxaashader = NULL;

void loadfxaashaders()
{
    loadhdrshaders(tqaa ? AA_VELOCITY : (!fxaagreenluma ? AA_LUMA : AA_UNUSED));

    string opts;
    int optslen = 0;
    if(fxaagreenluma || tqaa) opts[optslen++] = 'g';
    opts[optslen] = '\0';

    defformatstring(fxaaname)("fxaa%d%s", fxaaquality, opts);
    fxaashader = generateshader(fxaaname, "fxaashaders %d \"%s\"", fxaaquality, opts);
}

void clearfxaashaders()
{
    fxaashader = NULL;
}

void setupfxaa(int w, int h)
{
    if(!fxaatex) glGenTextures(1, &fxaatex);
    if(!fxaafbo) glGenFramebuffers_(1, &fxaafbo);
    glBindFramebuffer_(GL_FRAMEBUFFER, fxaafbo);
    createtexture(fxaatex, w, h, NULL, 3, 1, tqaa || !fxaagreenluma ? GL_RGBA8 : GL_RGB, GL_TEXTURE_RECTANGLE);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, fxaatex, 0);
    bindgdepth();
    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating FXAA buffer!");
    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    loadfxaashaders();
}

void cleanupfxaa()
{
    if(fxaafbo) { glDeleteFramebuffers_(1, &fxaafbo); fxaafbo = 0; }
    if(fxaatex) { glDeleteTextures(1, &fxaatex); fxaatex = 0; }

    clearfxaashaders();
}

VARFP(fxaa, 0, 0, 1, cleanupfxaa());
VARFP(fxaaquality, 0, 1, 3, cleanupfxaa());
VARFP(fxaagreenluma, 0, 0, 1, cleanupfxaa());

void dofxaa(GLuint outfbo = 0)
{
    timer *fxaatimer = begintimer("fxaa");

    if(tqaa) packtqaa();

    glBindFramebuffer_(GL_FRAMEBUFFER, tqaa ? tqaafbo[0] : outfbo);
    fxaashader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE, fxaatex);
    screenquad(vieww, viewh);

    if(tqaa) resolvetqaa(outfbo);

    endtimer(fxaatimer);
}
 
GLuint smaaareatex = 0, smaasearchtex = 0, smaafbo[4] = { 0, 0, 0, 0 }, smaatex[5] = { 0, 0, 0, 0, 0 };
int smaasubsampleorder = -1;

extern int smaaquality, smaagreenluma, smaacoloredge, smaadepthmask, smaastencil;

static Shader *smaalumaedgeshader = NULL, *smaacoloredgeshader = NULL, *smaablendweightshader = NULL, *smaaneighborhoodshader = NULL;

void loadsmaashaders(bool split = false)
{
    loadhdrshaders(split ? (tqaa ? AA_SPLIT_VELOCITY : (!smaagreenluma && !smaacoloredge ? AA_SPLIT_LUMA : AA_SPLIT)) :
                           (tqaa ? AA_VELOCITY : (!smaagreenluma && !smaacoloredge ? AA_LUMA : AA_UNUSED)));

    string opts;
    int optslen = 0;
    if(!hasTRG) opts[optslen++] = 'a';
    if(smaadepthmask || smaastencil) opts[optslen++] = 'd';
    if(split) opts[optslen++] = 's';
    if(smaagreenluma || tqaa) opts[optslen++] = 'g';
    if(tqaa) 
    {
        opts[optslen++] = 't';
        if(tqaamovemask) opts[optslen++] = 'm';
    }
    opts[optslen] = '\0';

    defformatstring(lumaedgename)("SMAALumaEdgeDetection%d%s", smaaquality, opts);
    defformatstring(coloredgename)("SMAAColorEdgeDetection%d%s", smaaquality, opts);
    defformatstring(blendweightname)("SMAABlendingWeightCalculation%d%s", smaaquality, opts);
    defformatstring(neighborhoodname)("SMAANeighborhoodBlending%d%s", smaaquality, opts);
    smaalumaedgeshader = lookupshaderbyname(lumaedgename);
    smaacoloredgeshader = lookupshaderbyname(coloredgename);
    smaablendweightshader = lookupshaderbyname(blendweightname);
    smaaneighborhoodshader = lookupshaderbyname(neighborhoodname);

    if(smaalumaedgeshader && smaacoloredgeshader && smaablendweightshader && smaaneighborhoodshader) return;

    generateshader(NULL, "smaashaders %d \"%s\"", smaaquality, opts);
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

#define SMAA_SEARCHTEX_WIDTH 66
#define SMAA_SEARCHTEX_HEIGHT 33
static uchar smaasearchdata[SMAA_SEARCHTEX_WIDTH*SMAA_SEARCHTEX_HEIGHT];
static bool smaasearchdatainited = false;

void gensmaasearchdata()
{
    if(smaasearchdatainited) return;
    int edges[33];
    memset(edges, -1, sizeof(edges));
    loop(a, 2) loop(b, 2) loop(c, 2) loop(d, 2) edges[(a*1 + b*3) + (c*7 + d*21)] = a + (b<<1) + (c<<2) + (d<<3);
    memset(smaasearchdata, 0, sizeof(smaasearchdata));
    loop(y, 33) loop(x, 33)
    {
        int left = edges[x], top = edges[y];
        if(left < 0 || top < 0) continue;
        uchar deltaLeft = 0;
        if(top&(1<<3)) deltaLeft++;
        if(deltaLeft && top&(1<<2) && !(left&(1<<1)) && !(left&(1<<3))) deltaLeft++;
        smaasearchdata[y*66 + x] = deltaLeft;
        uchar deltaRight = 0;
        if(top&(1<<3) && !(left&(1<<1)) && !(left&(1<<3))) deltaRight++;        
        if(deltaRight && top&(1<<2) && !(left&(1<<0)) && !(left&(1<<2))) deltaRight++;
        smaasearchdata[y*66 + 33 + x] = deltaRight;
    }   
    smaasearchdatainited = true;
}

vec2 areaunderortho(const vec2 &p1, const vec2 &p2, float x)
{
    vec2 d(p2.x - p1.x, p2.y - p1.y);
    float y1 = p1.y + (x - p1.x)*d.y/d.x,
          y2 = p1.y + (x+1 - p1.x)*d.y/d.x;
    if((x < p1.x || x >= p2.x) && (x+1 <= p1.x || x+1 > p2.x)) return vec2(0, 0);
    if((y1 < 0) == (y2 < 0) || fabs(y1) < 1e-4f || fabs(y2) < 1e-4f)
    {
        float a = (y1 + y2) / 2;
        return a < 0.0f ? vec2(-a, 0) : vec2(0, a);
    }
    x = -p1.y*d.x/d.y + p1.x;
    float a1 = x > p1.x ? y1*fmod(x, 1.0f)/2 : 0,
          a2 = x < p2.x ? y2*(1-fmod(x, 1.0f))/2 : 0;
    vec2 a(fabs(a1), fabs(a2));
    if((a.x > a.y ? a1 : -a2) >= 0) swap(a.x, a.y);
    return a;
}

static const int edgesortho[][2] =
{
    {0, 0}, {3, 0}, {0, 3}, {3, 3}, {1, 0}, {4, 0}, {1, 3}, {4, 3},
    {0, 1}, {3, 1}, {0, 4}, {3, 4}, {1, 1}, {4, 1}, {1, 4}, {4, 4}
};

static inline vec2 areaortho(float p1x, float p1y, float p2x, float p2y, float left)
{
    return areaunderortho(vec2(p1x, p1y), vec2(p2x, p2y), left);
}

vec2 areaortho(int pattern, float left, float right, float offset)
{
    float d = left + right + 1, o1 = offset + 0.5f, o2 = offset - 0.5f;
    switch(pattern)
    {
        case 0: return vec2(0, 0);
        case 1: return left <= right ? areaortho(0, o2, d/2, 0, left) : vec2(0, 0);
        case 2: return left >= right ? areaortho(d/2, 0, d, o2, left) : vec2(0, 0);
        case 3: return areaortho(0, o2, d/2, 0, left).add(areaortho(d/2, 0, d, o2, left));
        case 4: return left <= right ? areaortho(0, o1, d/2, 0, left) : vec2(0, 0);
        case 5: return vec2(0, 0);
        case 6: 
        {
            vec2 a = areaortho(0, o1, d, o2, left);
            if(fabs(offset) > 0) a.avg(areaortho(0, o1, d/2, 0, left).add(areaortho(d/2, 0, d, o2, left)));
            return a;
        }
        case 7: return areaortho(0, o1, d, o2, left);
        case 8: return left >= right ? areaortho(d/2, 0, d, o1, left) : vec2(0, 0); 
        case 9:
        {
            vec2 a = areaortho(0, o2, d, o1, left);
            if(fabs(offset) > 0) a.avg(areaortho(0, o2, d/2, 0, left).add(areaortho(d/2, 0, d, o1, left)));
            return a;
        }
        case 10: return vec2(0, 0);
        case 11: return areaortho(0, o2, d, o1, left);
        case 12: return areaortho(0, o1, d/2, 0, left).add(areaortho(d/2, 0, d, o1, left));
        case 13: return areaortho(0, o2, d, o1, left);
        case 14: return areaortho(0, o1, d, o2, left);
        case 15: return vec2(0, 0);
    }
    return vec2(0, 0);
}

float areaunderdiag(const vec2 &p1, const vec2 &p2, const vec2 &p)
{
    vec2 d(p2.y - p1.y, p1.x - p2.x);
    float dp = d.dot(vec2(p1).avg(p2).sub(p));
    if(!d.x)
    {
        if(!d.y) return 0 > dp;
        if(d.y < dp) return 0;
        return 1 - dp/d.y;
    }
    if(!d.y)
    {
        if(d.x < dp) return 0;
        return 1 - dp/d.x;
    }
    float l = dp/d.y, r = (dp-d.x)/d.y, b = dp/d.x, t = (dp-d.y)/d.x;
    if(0 <= dp)
    {
        if(d.y <= dp)
        {
            if(d.x <= dp)
            {
                if(d.y+d.x <= dp) return 0;
                return 0.5f*(1-r)*(1-t);
            }
            if(d.y+d.x > dp) return min(1-b, 1-t) + 0.5f*fabs(b-t);
            return 0.5f*(1-b)*r;
        }
        if(d.x <= dp)
        {
            if(d.y+d.x <= dp) return 0.5f*(1-l)*t;
            return min(1-l, 1-r) + 0.5f*fabs(r-l);
        }
        return 1 - 0.5f*l*b;
    }
    if(d.y <= dp)
    {
        if(d.x <= dp) return l*b;
        if(d.y+d.x <= dp) return min(l, r) + 0.5f*fabs(r-l);
        return 1 - 0.5f*(1-l)*t;
    }
    if(d.x <= dp)
    {
        if(d.y+d.x <= dp) return min(b, t) + 0.5f*fabs(b-t);
        return 1 - 0.5f*(1-b)*r;
    }
    if(d.y+d.x <= dp) return 1 - 0.5f*(1-t)*(1-r);
    return 1;
}

static inline vec2 areadiag(const vec2 &p1, const vec2 &p2, float left)
{
    return vec2(1 - areaunderdiag(p1, p2, vec2(1, 0).add(left)), areaunderdiag(p1, p2, vec2(1, 1).add(left)));
}

static const int edgesdiag[][2] =
{
    {0, 0}, {1, 0}, {0, 2}, {1, 2}, {2, 0}, {3, 0}, {2, 2}, {3, 2},
    {0, 1}, {1, 1}, {0, 3}, {1, 3}, {2, 1}, {3, 1}, {2, 3}, {3, 3}
};

static inline vec2 areadiag(float p1x, float p1y, float p2x, float p2y, float d, float left, const vec2 &offset, int pattern)
{
    vec2 p1(p1x, p1y), p2(p2x+d, p2y+d);
    if(edgesdiag[pattern][0]) p1.add(offset);
    if(edgesdiag[pattern][1]) p2.add(offset);
    return areadiag(p1, p2, left);
}

static inline vec2 areadiag2(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y, float d, float left, const vec2 &offset, int pattern)
{
    vec2 p1(p1x, p1y), p2(p2x+d, p2y+d), p3(p3x, p3y), p4(p4x+d, p4y+d);
    if(edgesdiag[pattern][0]) { p1.add(offset); p3.add(offset); }
    if(edgesdiag[pattern][1]) { p2.add(offset); p4.add(offset); }
    return areadiag(p1, p2, left).avg(areadiag(p3, p4, left));
}

vec2 areadiag(int pattern, float left, float right, const vec2 &offset)
{
    float d = left + right + 1;
    switch(pattern)
    {
        case 0: return areadiag2(1, 1, 1, 1, 1, 0, 1, 0, d, left, offset, pattern);
        case 1: return areadiag2(1, 0, 0, 0, 1, 0, 1, 0, d, left, offset, pattern);
        case 2: return areadiag2(0, 0, 1, 0, 1, 0, 1, 0, d, left, offset, pattern);
        case 3: return areadiag(1, 0, 1, 0, d, left, offset, pattern);
        case 4: return areadiag2(1, 1, 0, 0, 1, 1, 1, 0, d, left, offset, pattern);
        case 5: return areadiag2(1, 1, 0, 0, 1, 0, 1, 0, d, left, offset, pattern);
        case 6: return areadiag(1, 1, 1, 0, d, left, offset, pattern);
        case 7: return areadiag2(1, 1, 1, 0, 1, 0, 1, 0, d, left, offset, pattern);
        case 8: return areadiag2(0, 0, 1, 1, 1, 0, 1, 1, d, left, offset, pattern);
        case 9: return areadiag(1, 0, 1, 1, d, left, offset, pattern);
        case 10: return areadiag2(0, 0, 1, 1, 1, 0, 1, 0, d, left, offset, pattern);
        case 11: return areadiag2(1, 0, 1, 1, 1, 0, 1, 0, d, left, offset, pattern);
        case 12: return areadiag(1, 1, 1, 1, d, left, offset, pattern);
        case 13: return areadiag2(1, 1, 1, 1, 1, 0, 1, 1, d, left, offset, pattern);
        case 14: return areadiag2(1, 1, 1, 1, 1, 1, 1, 0, d, left, offset, pattern);
        case 15: return areadiag2(1, 1, 1, 1, 1, 0, 1, 0, d, left, offset, pattern);
    }
    return vec2(0, 0);
}

static const float offsetsortho[] = { 0.0f, -0.25f, 0.25f, -0.125f, 0.125f, -0.375f, 0.375f };
static const float offsetsdiag[][2] = { { 0.0f, 0.0f, }, { 0.25f, -0.25f }, { -0.25f, 0.25f }, { 0.125f, -0.125f }, { -0.125f, 0.125f } };

#define SMAA_AREATEX_WIDTH 160
#define SMAA_AREATEX_HEIGHT 560
static uchar smaaareadata[SMAA_AREATEX_WIDTH*SMAA_AREATEX_HEIGHT*2];
static bool smaaareadatainited = false;

void gensmaaareadata()
{
    if(smaaareadatainited) return;
    memset(smaaareadata, 0, sizeof(smaaareadata));
    loop(offset, sizeof(offsetsortho)/sizeof(offsetsortho[0])) loop(pattern, 16)
    {
        int px = edgesortho[pattern][0]*16, py = (5*offset + edgesortho[pattern][1])*16;
        uchar *dst = &smaaareadata[(py*SMAA_AREATEX_WIDTH + px)*2];
        loop(y, 16) 
        {
            loop(x, 16)
            {
                vec2 a = areaortho(pattern, x*x, y*y, offsetsortho[offset]);
                dst[0] = uchar(255*a.x);
                dst[1] = uchar(255*a.y);
                dst += 2;
            }
            dst += (SMAA_AREATEX_WIDTH-16)*2;
        }
    }
    loop(offset, sizeof(offsetsdiag)/sizeof(offsetsdiag[0])) loop(pattern, 16)
    {
        int px = 5*16 + edgesdiag[pattern][0]*20, py = (4*offset + edgesdiag[pattern][1])*20;
        uchar *dst = &smaaareadata[(py*SMAA_AREATEX_WIDTH + px)*2];
        loop(y, 20)
        {
            loop(x, 20)
            {
                vec2 a = areadiag(pattern, x, y, vec2(offsetsdiag[offset][0], offsetsdiag[offset][1]));
                dst[0] = uchar(255*a.x);
                dst[1] = uchar(255*a.y);
                dst += 2;
            }
            dst += (SMAA_AREATEX_WIDTH-20)*2;
        }
    }
    smaaareadatainited = true;
}

VAR(smaat2x, 1, 0, 0);
VAR(smaas2x, 1, 0, 0);
VAR(smaa4x, 1, 0, 0);

void setupsmaa(int w, int h)
{
    if(!smaaareatex) glGenTextures(1, &smaaareatex);
    if(!smaasearchtex) glGenTextures(1, &smaasearchtex);
    gensmaasearchdata();
    gensmaaareadata();
    createtexture(smaaareatex, SMAA_AREATEX_WIDTH, SMAA_AREATEX_HEIGHT, smaaareadata, 3, 1, hasTRG ? GL_RG8 : GL_LUMINANCE8_ALPHA8, GL_TEXTURE_RECTANGLE, 0, 0, 0, false);
    createtexture(smaasearchtex, SMAA_SEARCHTEX_WIDTH, SMAA_SEARCHTEX_HEIGHT, smaasearchdata, 3, 0, hasTRG ? GL_R8 : GL_LUMINANCE8, GL_TEXTURE_RECTANGLE, 0, 0, 0, false);
    bool split = multisampledaa();
    smaasubsampleorder = split ? (msaapositions[0].x < 0.5f ? 1 : 0) : -1;
    smaat2x = tqaa ? 1 : 0;
    smaas2x = split ? 1 : 0;
    smaa4x = tqaa && split ? 1 : 0;
    loopi(split ? 4 : 3)
    {
        if(!smaatex[i]) glGenTextures(1, &smaatex[i]);
        if(!smaafbo[i]) glGenFramebuffers_(1, &smaafbo[i]);
        glBindFramebuffer_(GL_FRAMEBUFFER, smaafbo[i]);
        GLenum format = GL_RGB;
        switch(i)
        {
            case 0: format = tqaa || (!smaagreenluma && !smaacoloredge) ? GL_RGBA8 : GL_RGB; break;
            case 1: format = hasTRG ? GL_RG8 : GL_RGBA8; break;
            case 2: case 3: format = GL_RGBA8; break;
        }  
        createtexture(smaatex[i], w, h, NULL, 3, 1, format, GL_TEXTURE_RECTANGLE);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, smaatex[i], 0);
        if(!i && split)
        {
            if(!smaatex[4]) glGenTextures(1, &smaatex[4]);
            createtexture(smaatex[4], w, h, NULL, 3, 1, format, GL_TEXTURE_RECTANGLE);
            glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, smaatex[4], 0);
            static const GLenum drawbufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
            glDrawBuffers_(2, drawbufs);
        }
        if(!i || smaadepthmask || smaastencil) bindgdepth();
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating SMAA buffer!");
    }
    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    loadsmaashaders(split);
}

void cleanupsmaa()
{
    if(smaaareatex) { glDeleteTextures(1, &smaaareatex); smaaareatex = 0; }
    if(smaasearchtex) { glDeleteTextures(1, &smaasearchtex); smaasearchtex = 0; }
    loopi(4) if(smaafbo[i]) { glDeleteFramebuffers_(1, &smaafbo[i]); smaafbo[i] = 0; }
    loopi(5) if(smaatex[i]) { glDeleteTextures(1, &smaatex[i]); smaatex[i] = 0; }
    smaasubsampleorder = -1;
    smaat2x = smaas2x = smaa4x = 0;

    clearsmaashaders();
}

VARFP(smaa, 0, 0, 1, cleanupgbuffer());
VARFP(smaaspatial, 0, 1, 1, cleanupgbuffer());
VARFP(smaaquality, 0, 2, 3, cleanupsmaa());
VARFP(smaacoloredge, 0, 0, 1, cleanupsmaa());
VARFP(smaagreenluma, 0, 0, 1, cleanupsmaa());
VARF(smaadepthmask, 0, 1, 1, cleanupsmaa());
VARF(smaastencil, 0, 1, 1, cleanupsmaa());
VAR(debugsmaa, 0, 0, 5);

void viewsmaa()
{
    int w = min(screenw, screenh)*1.0f, h = (w*screenh)/screenw, tw = gw, th = gh;
    SETSHADER(hudrect);
    gle::colorf(1, 1, 1);
    switch(debugsmaa)
    {
        case 1: glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[0]); break;
        case 2: glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[1]); break;
        case 3: glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[2]); break;
        case 4: glBindTexture(GL_TEXTURE_RECTANGLE, smaaareatex); tw = SMAA_AREATEX_WIDTH; th = SMAA_AREATEX_HEIGHT; break;
        case 5: glBindTexture(GL_TEXTURE_RECTANGLE, smaasearchtex); tw = SMAA_SEARCHTEX_WIDTH; th = SMAA_SEARCHTEX_HEIGHT; break;
    }
    debugquad(0, 0, w, h, 0, 0, tw, th);
}

void dosmaa(GLuint outfbo = 0, bool split = false)
{
    timer *smaatimer = begintimer("smaa");

    if(tqaa) packtqaa();

    int cleardepth = msaasamples ? GL_DEPTH_BUFFER_BIT | ((gdepthstencil && hasDS) || gstencil ? GL_STENCIL_BUFFER_BIT : 0) : 0;
    loop(pass, split ? 2 : 1)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, smaafbo[1]);
        if(smaadepthmask || smaastencil)
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | (!pass ? cleardepth : 0));
        }
        if(smaadepthmask)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            float depthval = cleardepth ? 0.25f*(pass+1) : 1;
            glDepthRange(depthval, depthval);
        }
        else if(smaastencil && ((gdepthstencil && hasDS) || gstencil))
        {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 0x10*(pass+1), ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        if(smaacoloredge) smaacoloredgeshader->set();
        else smaalumaedgeshader->set();
        glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[pass ? 4 : 0]);
        screenquad(vieww, viewh);

        glBindFramebuffer_(GL_FRAMEBUFFER, smaafbo[2 + pass]);
        if(smaadepthmask)
        {
            glDepthFunc(GL_EQUAL);
            glDepthMask(GL_FALSE);
        }
        else if(smaastencil && ((gdepthstencil && hasDS) || gstencil))
        {
            glStencilFunc(GL_EQUAL, 0x10*(pass+1), ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }
        if(smaadepthmask || smaastencil) glClear(GL_COLOR_BUFFER_BIT);
        smaablendweightshader->set();
        vec4 subsamples(0, 0, 0, 0);
        if(tqaa && split) subsamples = tqaaframe&1 ? (pass != smaasubsampleorder ? vec4(6, 4, 2, 4) : vec4(3, 5, 1, 4)) : (pass != smaasubsampleorder ? vec4(4, 6, 2, 3) : vec4(5, 3, 1, 3));
        else if(tqaa) subsamples = tqaaframe&1 ? vec4(2, 2, 2, 0) : vec4(1, 1, 1, 0);
        else if(split) subsamples = pass != smaasubsampleorder ? vec4(2, 2, 2, 0) : vec4(1, 1, 1, 0); 
        LOCALPARAM(subsamples, subsamples);
        glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[1]);
        glActiveTexture_(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_RECTANGLE, smaaareatex);
        glActiveTexture_(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_RECTANGLE, smaasearchtex);
        if(tqaa && tqaamovemask)
        {
            glActiveTexture_(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_RECTANGLE, tqaamasktex);
        }
        glActiveTexture_(GL_TEXTURE0);
        screenquadoffset(0, 0, vieww, viewh, -0.5f/(1<<tqaamovemaskreduce), -0.5f/(1<<tqaamovemaskreduce), vieww>>tqaamovemaskreduce, viewh>>tqaamovemaskreduce);
        if(smaadepthmask)
        {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glDepthRange(0, 1);
        }
        else if(smaastencil && ((gdepthstencil && hasDS) || gstencil)) glDisable(GL_STENCIL_TEST);
    }
        
    glBindFramebuffer_(GL_FRAMEBUFFER, tqaa ? tqaafbo[0] : outfbo);
    smaaneighborhoodshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[0]);
    glActiveTexture_(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[2]);
    if(split)
    {
        glActiveTexture_(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[4]);
        glActiveTexture_(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_RECTANGLE, smaatex[3]);
    }
    glActiveTexture_(GL_TEXTURE0);
    screenquad(vieww, viewh);

    if(tqaa) resolvetqaa(outfbo);

    endtimer(smaatimer);
}

void setupaa(int w, int h)
{
    if(smaa) { if(!smaafbo[0]) setupsmaa(w, h); }
    else if(fxaa) { if(!fxaafbo) setupfxaa(w, h); }

    if(tqaa && !tqaafbo[0]) setuptqaa(w, h);
}

glmatrix nojittermatrix, aamaskmatrix;
int aamask = -1;

void jitteraa(bool init)
{
    nojittermatrix = projmatrix;

    if(!drawtex && tqaa)
    {
        vec2 jitter = tqaaframe&1 ? vec2(0.25f, 0.25f) : vec2(-0.25f, -0.25f);
        if(multisampledaa()) { jitter.x *= 0.5f; jitter.y *= -0.5f; }
        projmatrix.jitter(jitter.x*2.0f/vieww, jitter.y*2.0f/viewh);
    }

    if(init) aamask = -1;
    else if(aamask >= 0) aamaskmatrix.mul(aamask ? nojittermatrix : projmatrix, cammatrix);
}

void setaamask(bool on)
{
    int val = on && !drawtex && tqaa && tqaamovemask ? 1 : 0;
    if(aamask == val) return;

    aamask = val;

    if(aamask) aamaskmatrix.mul(nojittermatrix, cammatrix);
    else aamaskmatrix = camprojmatrix;

    GLOBALPARAMF(aamask, (aamask ? 1.0f : 0.0f));
}

bool multisampledaa()
{
    return smaa && smaaspatial && msaasamples == 2 && hasMSS;
}
 
bool maskedaa()
{
    return tqaa && tqaamovemask;
}
   
void doaa(GLuint outfbo, void (*resolve)(GLuint, int))
{
    if(smaa) 
    { 
        bool split = multisampledaa();
        resolve(smaafbo[0], split ? (tqaa ? AA_SPLIT_VELOCITY : (!smaagreenluma && !smaacoloredge ? AA_SPLIT_LUMA : AA_SPLIT)) : 
                                    (tqaa ? AA_VELOCITY : (!smaagreenluma && !smaacoloredge ? AA_LUMA : AA_UNUSED))); 
        dosmaa(outfbo, split); 
    }
    else if(fxaa) { resolve(fxaafbo, tqaa ? AA_VELOCITY : (!fxaagreenluma ? AA_LUMA : AA_UNUSED)); dofxaa(outfbo); }
    else if(tqaa) { resolve(tqaafbo[0], AA_VELOCITY); dotqaa(outfbo); }
    else resolve(outfbo, AA_UNUSED);
}

bool debugaa()
{
    if(debugsmaa) viewsmaa();
    else return false;
    return true;
}

void cleanupaa()
{
    cleanupsmaa();
    cleanupfxaa();
    cleanuptqaa();
}

