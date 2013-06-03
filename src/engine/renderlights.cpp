#include "engine.h"

int gw = -1, gh = -1, bloomw = -1, bloomh = -1, lasthdraccum = 0;
GLuint gfbo = 0, gdepthtex = 0, gcolortex = 0, gnormaltex = 0, gglowtex = 0, gdepthrb = 0, gstencilrb = 0;
int scalew = -1, scaleh = -1;
GLuint scalefbo[2] = { 0, 0 }, scaletex[2] = { 0, 0 };
GLuint hdrfbo = 0, hdrtex = 0, bloomfbo[6] = { 0, 0, 0, 0, 0, 0 }, bloomtex[6] = { 0, 0, 0, 0, 0, 0 };
int hdrclear = 0;
GLuint refractfbo = 0, refracttex = 0;
GLenum bloomformat = 0, hdrformat = 0, stencilformat = 0;
bool hdrfloat = false;
GLuint msfbo = 0, msdepthtex = 0, mscolortex = 0, msnormaltex = 0, msglowtex = 0, msdepthrb = 0, msstencilrb = 0, mshdrfbo = 0, mshdrtex = 0, msrefractfbo = 0, msrefracttex = 0;
vec2 msaapositions[16];
int aow = -1, aoh = -1;
GLuint aofbo[4] = { 0, 0, 0, 0 }, aotex[4] = { 0, 0, 0, 0 }, aonoisetex = 0;
glmatrix eyematrix, worldmatrix, linearworldmatrix, screenmatrix;

extern int ati_pf_bug;

int gethdrformat(int prec, int fallback = GL_RGB)
{
    if(prec >= 3 && hasTF) return GL_RGB16F;
    if(prec >= 2 && hasPF && !ati_pf_bug) return GL_R11F_G11F_B10F;
    if(prec >= 1) return GL_RGB10;
    return fallback;
}

extern int bloomsize, bloomprec;

void setupbloom(int w, int h)
{
    int maxsize = ((1<<bloomsize)*5)/4;
    while(w >= maxsize || h >= maxsize)
    {
        w /= 2;
        h /= 2;
    }
    if(w == bloomw && h == bloomh) return;
    bloomw = w;
    bloomh = h;

    loopi(5) if(!bloomtex[i]) glGenTextures(1, &bloomtex[i]);

    loopi(5) if(!bloomfbo[i]) glGenFramebuffers_(1, &bloomfbo[i]);

    bloomformat = gethdrformat(bloomprec);
    createtexture(bloomtex[0], max(gw/2, bloomw), max(gh/2, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE);
    createtexture(bloomtex[1], max(gw/4, bloomw), max(gh/4, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE);
    createtexture(bloomtex[2], bloomw, bloomh, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE);
    createtexture(bloomtex[3], bloomw, bloomh, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE);
    if(bloomformat != GL_RGB)
    {
        if(!bloomtex[5]) glGenTextures(1, &bloomtex[5]);
        if(!bloomfbo[5]) glGenFramebuffers_(1, &bloomfbo[5]);
        createtexture(bloomtex[5], bloomw, bloomh, NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE);
    }

    static uchar gray[3] = { 32, 32, 32 };
    static float grayf[3] = { 0.125f, 0.125f, 0.125f };
    createtexture(bloomtex[4], 1, 1, hasTF ? (void *)grayf : (void *)gray, 3, 1, hasTF ? GL_RGB16F : GL_RGB16);

    loopi(5 + (bloomformat != GL_RGB ? 1 : 0))
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, bloomfbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, i==4 ? GL_TEXTURE_2D : GL_TEXTURE_RECTANGLE, bloomtex[i], 0);

        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating bloom buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);
}

void cleanupbloom()
{
    loopi(6) if(bloomfbo[i]) { glDeleteFramebuffers_(1, &bloomfbo[i]); bloomfbo[i] = 0; }
    loopi(6) if(bloomtex[i]) { glDeleteTextures(1, &bloomtex[i]); bloomtex[i] = 0; }
    bloomw = bloomh = -1;
    lasthdraccum = 0;
}

extern int ao, aotaps, aoreduce, aoreducedepth, aonoise, aobilateral, aobilateralupscale, aopackdepth, aodepthformat, aoprec, aoderivnormal;

static Shader *bilateralshader[2] = { NULL, NULL };

Shader *loadbilateralshader(int pass)
{
    if(!aobilateral) return nullshader;

    string opts;
    int optslen = 0;

    bool linear = aoreducedepth && (aoreduce || aoreducedepth > 1), upscale = aoreduce && aobilateralupscale;
    if(aoreduce && (upscale || (!linear && !aopackdepth))) opts[optslen++] = 'r';
    if(upscale) opts[optslen++] = 'u';
    else if(linear) opts[optslen++] = 'l';
    if(aopackdepth) opts[optslen++] = 'p';
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

void setbilateralshader(int radius, int pass, float sigma, float depth)
{
    bilateralshader[pass]->set();
    sigma *= 2*radius;
    float step = pass ? float(viewh)/aoh : float(vieww)/aow;
    LOCALPARAMF(bilateralparams, (1.0f/(2*sigma*sigma), 1.0f/(depth*depth), step));
}

static Shader *ambientobscuranceshader = NULL;

Shader *loadambientobscuranceshader()
{
    string opts;
    int optslen = 0;

    bool linear = aoreducedepth && (aoreduce || aoreducedepth > 1);
    if(linear) opts[optslen++] = 'l';
    if(aoderivnormal) opts[optslen++] = 'd';
    if(aobilateral && aopackdepth) opts[optslen++] = 'p';
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
    int sw = w>>aoreduce, sh = h>>aoreduce;

    if(sw == aow && sh == aoh) return;

    aow = sw;
    aoh = sh;

    if(!aonoisetex) glGenTextures(1, &aonoisetex);
    bvec *noise = new bvec[(1<<aonoise)*(1<<aonoise)];
    loopk((1<<aonoise)*(1<<aonoise)) noise[k] = bvec(vec(rndscale(2)-1, rndscale(2)-1, 0).normalize());
    createtexture(aonoisetex, 1<<aonoise, 1<<aonoise, noise, 0, 0, GL_RGB, GL_TEXTURE_2D);
    delete[] noise;

    bool upscale = aoreduce && aobilateral && aobilateralupscale;
    GLenum format = aoprec && hasTRG ? GL_R8 : GL_RGBA8,
           packformat = aobilateral && aopackdepth ? (aodepthformat ? GL_RG16F : GL_RGBA8) : format;
    int packfilter = upscale && aopackdepth && !aodepthformat ? 0 : 1;
    loopi(upscale ? 3 : 2)
    {
        if(!aotex[i]) glGenTextures(1, &aotex[i]);
        if(!aofbo[i]) glGenFramebuffers_(1, &aofbo[i]);
        createtexture(aotex[i], upscale && i ? w : aow, upscale && i >= 2 ? h : aoh, NULL, 3, i < 2 ? packfilter : 1, i < 2 ? packformat : format, GL_TEXTURE_RECTANGLE);
        glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, aotex[i], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating AO buffer!");
    }

    if(aoreducedepth && (aoreduce || aoreducedepth > 1))
    {
        if(!aotex[3]) glGenTextures(1, &aotex[3]);
        if(!aofbo[3]) glGenFramebuffers_(1, &aofbo[3]);
        createtexture(aotex[3], aow, aoh, NULL, 3, 0, aodepthformat > 1 ? GL_R32F : (aodepthformat ? GL_R16F : GL_RGBA8), GL_TEXTURE_RECTANGLE);
        glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[3]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, aotex[3], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating AO buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    loadaoshaders();
    loadbilateralshaders();
}

void cleanupao()
{
    loopi(4) if(aofbo[i]) { glDeleteFramebuffers_(1, &aofbo[i]); aofbo[i] = 0; }
    loopi(4) if(aotex[i]) { glDeleteTextures(1, &aotex[i]); aotex[i] = 0; }
    if(aonoisetex) { glDeleteTextures(1, &aonoisetex); aonoisetex = 0; }
    aow = bloomh = -1;

    clearaoshaders();
    clearbilateralshaders();
}

VARFP(ao, 0, 1, 1, { cleanupao(); cleardeferredlightshaders(); });
FVARR(aoradius, 0, 5, 256);
FVAR(aocutoff, 0, 2.0f, 1e3f);
FVARR(aodark, 1e-3f, 11.0f, 1e3f);
FVARR(aosharp, 1e-3f, 1, 1e3f);
FVAR(aoprefilterdepth, 0, 1, 1e3);
FVARR(aomin, 0, 0.25f, 1);
VARFR(aosun, 0, 1, 1, cleardeferredlightshaders());
FVARR(aosunmin, 0, 0.5f, 1);
VARP(aoblur, 0, 4, 7);
FVARP(aosigma, 0.005f, 0.5f, 2.0f);
VARP(aoiter, 0, 0, 4);
VARFP(aoreduce, 0, 1, 2, cleanupao());
VARF(aoreducedepth, 0, 1, 2, cleanupao());
VARFP(aofloatdepth, 0, 1, 2, initwarning("AO setup", INIT_LOAD, CHANGE_SHADERS));
VARFP(aoprec, 0, 1, 1, cleanupao());
VAR(aodepthformat, 1, 0, 0);
VARF(aonoise, 0, 5, 8, cleanupao());
VARFP(aobilateral, 0, 3, 10, cleanupao());
FVARP(aobilateralsigma, 0, 0.5f, 1e3f);
FVARP(aobilateraldepth, 0, 4, 1e3f);
VARFP(aobilateralupscale, 0, 0, 1, cleanupao());
VARF(aopackdepth, 0, 1, 1, cleanupao());
VARFP(aotaps, 1, 5, 12, cleanupao());
VARF(aoderivnormal, 0, 0, 1, cleanupao());
VAR(debugao, 0, 0, 1);

void initao()
{
    aodepthformat = aofloatdepth && hasTRG && hasTF ? aofloatdepth : 0;
}

void viewao()
{
    if(!ao) return;
    int w = min(screenw, screenh)/2, h = (w*screenh)/screenw;
    SETSHADER(hudrect);
    gle::colorf(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE, aotex[2] ? aotex[2] : aotex[0]);
    int tw = aotex[2] ? gw : aow, th = aotex[2] ? gh : aoh;
    debugquad(0, 0, w, h, 0, 0, tw, th);
}

void renderao()
{
    if(!ao) return;

    timer *aotimer = begintimer("ambient obscurance");

    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);

    bool linear = aoreducedepth && (aoreduce || aoreducedepth > 1);
    float xscale = eyematrix.a.x, yscale = eyematrix.b.y;
    if(linear)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[3]);
        glViewport(0, 0, aow, aoh);
        SETSHADER(linearizedepth);
        screenquad(vieww, viewh);

        xscale *= float(vieww)/aow;
        yscale *= float(viewh)/aoh;

        glBindTexture(GL_TEXTURE_RECTANGLE, aotex[3]);
    }

    ambientobscuranceshader->set();

    glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[0]);
    glViewport(0, 0, aow, aoh);
    glActiveTexture_(GL_TEXTURE1);
    if(aoderivnormal)
    {
        if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
    }
    else 
    {
        if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msnormaltex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gnormaltex);
        LOCALPARAM(normalmatrix, glmatrix3x3(cammatrix));
    }
    glActiveTexture_(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, aonoisetex);
    glActiveTexture_(GL_TEXTURE0);

    LOCALPARAMF(tapparams, (aoradius*eyematrix.d.z/xscale, aoradius*eyematrix.d.z/yscale, aoradius*aoradius*aocutoff*aocutoff));
    LOCALPARAMF(contrastparams, ((2.0f*aodark)/aotaps, aosharp));
    LOCALPARAMF(offsetscale, (xscale/eyematrix.d.z, yscale/eyematrix.d.z, eyematrix.d.x/eyematrix.d.z, eyematrix.d.y/eyematrix.d.z));
    LOCALPARAMF(prefilterdepth, (aoprefilterdepth));
    screenquad(vieww, viewh, aow/float(1<<aonoise), aoh/float(1<<aonoise));

    if(aobilateral)
    {
        if(aoreduce && aobilateralupscale) loopi(2)
        {
            setbilateralshader(aobilateral, i, aobilateralsigma, aobilateraldepth);
            glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[i+1]);
            glViewport(0, 0, vieww, i ? viewh : aoh);
            glBindTexture(GL_TEXTURE_RECTANGLE, aotex[i]);
            glActiveTexture_(GL_TEXTURE1);
            if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
            else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
            glActiveTexture_(GL_TEXTURE0);
            screenquad(vieww, viewh, i ? vieww : aow, aoh);
        }
        else loopi(2 + 2*aoiter)
        {
            setbilateralshader(aobilateral, i%2, aobilateralsigma, aobilateraldepth);
            glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            glBindTexture(GL_TEXTURE_RECTANGLE, aotex[i%2]);
            glActiveTexture_(GL_TEXTURE1);
            if(linear) glBindTexture(GL_TEXTURE_RECTANGLE, aotex[3]);
            else if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
            else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
            glActiveTexture_(GL_TEXTURE0);
            screenquad(vieww, viewh);
        }
    }
    else if(aoblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(aoblur, aosigma, blurweights, bluroffsets);
        loopi(2 + 2*aoiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            setblurshader(i%2, 1, aoblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE);
            glBindTexture(GL_TEXTURE_RECTANGLE, aotex[i%2]);
            screenquad(aow, aoh);
        }
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, msaasamples ? msfbo : gfbo);
    glViewport(0, 0, vieww, viewh);

    endtimer(aotimer);
}

void cleanupscale()
{
    loopi(2) if(scalefbo[i]) { glDeleteFramebuffers_(1, &scalefbo[i]); scalefbo[i] = 0; }
    loopi(2) if(scaletex[i]) { glDeleteTextures(1, &scaletex[i]); scaletex[i] = 0; }
    scalew = scaleh = -1;
}

extern int gscalecubic, gscalenearest;

void setupscale(int sw, int sh, int w, int h)
{
    scalew = w;
    scaleh = h;

    loopi(gscalecubic ? 2 : 1) 
    {
        if(!scaletex[i]) glGenTextures(1, &scaletex[i]);
        if(!scalefbo[i]) glGenFramebuffers_(1, &scalefbo[i]);

        glBindFramebuffer_(GL_FRAMEBUFFER, scalefbo[i]);

        createtexture(scaletex[i], sw, i ? h : sh, NULL, 3, gscalecubic || !gscalenearest ? 1 : 0, GL_RGB, GL_TEXTURE_RECTANGLE);

        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, scaletex[i], 0);
        if(!i) bindgdepth();

        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating scale buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    useshaderbyname("scalelinear");
    if(gscalecubic) 
    {
        useshaderbyname("scalecubicx");
        useshaderbyname("scalecubicy");
    }
}

GLuint shouldscale()
{
    return scalefbo[0];
}
     
void doscale(int w, int h)
{
    if(!scaletex[0]) return;

    timer *scaletimer = begintimer("scaling");

    if(gscalecubic)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, scalefbo[1]);
        glViewport(0, 0, gw, h);
        glBindTexture(GL_TEXTURE_RECTANGLE, scaletex[0]);
        SETSHADER(scalecubicy);
        screenquad(gw, gh);
        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glBindTexture(GL_TEXTURE_RECTANGLE, scaletex[1]);
        SETSHADER(scalecubicx);
        screenquad(gw, h);
    }
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glBindTexture(GL_TEXTURE_RECTANGLE, scaletex[0]);
        SETSHADER(scalelinear);
        screenquad(gw, gh);
    }

    endtimer(scaletimer);
}
   
VARFP(glineardepth, 0, 0, 3, initwarning("g-buffer setup", INIT_LOAD, CHANGE_SHADERS));
VAR(gdepthformat, 1, 0, 0);
VARFP(msaa, 0, 0, 16, initwarning("MSAA setup", INIT_LOAD, CHANGE_SHADERS));
VARFP(csaa, 0, 0, 16, initwarning("MSAA setup", INIT_LOAD, CHANGE_SHADERS));
VARF(msaadepthstencil, 0, 1, 1, cleanupgbuffer());
VARF(msaastencil, 0, 0, 1, cleanupgbuffer());
VARF(msaaedgedetect, 0, 1, 1, cleanupgbuffer());
VARFP(msaalineardepth, -1, -1, 3, initwarning("MSAA setup", INIT_LOAD, CHANGE_SHADERS));
VARFP(msaatonemap, 0, 0, 1, cleanupgbuffer());
VARF(msaatonemapblit, 0, 0, 1, cleanupgbuffer());
VARF(msaatonemapstencil, 0, 0, 1, cleanupgbuffer());
VAR(msaamaxsamples, 1, 0, 0);
VAR(msaamaxdepthtexsamples, 1, 0, 0);
VAR(msaamaxcolortexsamples, 1, 0, 0);
VAR(msaasamples, 1, 0, 0);
VAR(msaamincolorsamples, 1, 0, 0);
VAR(msaacolorsamples, 1, 0, 0);

void initgbuffer()
{
    msaamaxsamples = msaamaxdepthtexsamples = msaamaxcolortexsamples = msaasamples = msaamincolorsamples = msaacolorsamples = 0;
    if(hasFBMS && hasFBB && hasTMS)
    {
        GLint val;
        glGetIntegerv(GL_MAX_SAMPLES, &val);
        msaamaxsamples = val;
        glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &val);
        msaamaxdepthtexsamples = val;
        glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &val);
        msaamaxcolortexsamples = val;
    }

    int maxsamples = min(msaamaxsamples, msaamaxcolortexsamples), samples = min(max(msaa, csaa), maxsamples);
    if(samples >= 2)
    {
        msaasamples = 2;
        while(msaasamples*2 <= samples) msaasamples *= 2;
        if(hasNVFBMSC && hasNVTMS)
        {
            if(msaa)
            {
                int colorsamples = min(msaa, maxsamples);
                msaamincolorsamples = 2;
                while(msaamincolorsamples*2 <= colorsamples) msaamincolorsamples *= 2;
            }
        }
        else msaamincolorsamples = msaasamples;
    }

    int lineardepth = glineardepth;
    if(msaasamples)
    {
        if(msaamaxdepthtexsamples < msaasamples) 
        {
            if(msaalineardepth > 0) lineardepth = msaalineardepth;
            else if(!lineardepth) lineardepth = 1;
        }
        else if(msaalineardepth >= 0) lineardepth = msaalineardepth;
    }

    if(lineardepth > 1 && (!hasAFBO || !hasTF || !hasTRG)) gdepthformat = 1;
    else gdepthformat = lineardepth;

    initao();
}

void maskgbuffer(const char *mask)
{
    GLenum drawbufs[4];
    int numbufs = 0;
    while(*mask) switch(*mask++)
    {
        case 'c': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT0; break;
        case 'n': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT1; break;
        case 'g': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT2; break;
        case 'd': if(gdepthformat) drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT3; break;
    }
    glDrawBuffers_(numbufs, drawbufs);
}

extern int hdrprec, gscale;

void cleanupmsbuffer()
{
    if(msfbo) { glDeleteFramebuffers_(1, &msfbo); msfbo = 0; }
    if(msdepthtex) { glDeleteTextures(1, &msdepthtex); msdepthtex = 0; }
    if(mscolortex) { glDeleteTextures(1, &mscolortex); mscolortex = 0; }
    if(msnormaltex) { glDeleteTextures(1, &msnormaltex); msnormaltex = 0; }
    if(msglowtex) { glDeleteTextures(1, &msglowtex); msglowtex = 0; }
    if(msstencilrb) { glDeleteRenderbuffers_(1, &msstencilrb); msstencilrb = 0; }
    if(msdepthrb) { glDeleteRenderbuffers_(1, &msdepthrb); msdepthrb = 0; }
    if(mshdrfbo) { glDeleteFramebuffers_(1, &mshdrfbo); mshdrfbo = 0; }
    if(mshdrtex) { glDeleteTextures(1, &mshdrtex); mshdrtex = 0; }
    if(msrefractfbo) { glDeleteFramebuffers_(1, &msrefractfbo); msrefractfbo = 0; }
    if(msrefracttex) { glDeleteTextures(1, &msrefracttex); msrefracttex = 0; }
    msaacolorsamples = 0;
}

void bindmsdepth()
{
    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msdepthrb);
        if(msaadepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msdepthrb);
        else if(msaastencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msstencilrb);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msdepthtex, 0);
        if(msaadepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msdepthtex, 0);
        else if(msaastencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msstencilrb);
    }
}

static void texms(GLenum format, int w, int h, GLenum fixed)
{
    if(msaamincolorsamples < msaasamples)
        glTexImage2DMultisampleCoverageNV_(GL_TEXTURE_2D_MULTISAMPLE, msaasamples, msaamincolorsamples, format, w, h, fixed);
    else
        glTexImage2DMultisample_(GL_TEXTURE_2D_MULTISAMPLE, msaasamples, format, w, h, fixed);
}

static void rbms(GLenum format, int w, int h)
{
    if(msaamincolorsamples < msaasamples)
        glRenderbufferStorageMultisampleCoverageNV_(GL_RENDERBUFFER, msaasamples, msaamincolorsamples, format, w, h);
    else
        glRenderbufferStorageMultisample_(GL_RENDERBUFFER, msaasamples, format, w, h);
}

void setupmsbuffer(int w, int h)
{
    if(!msdepthtex) glGenTextures(1, &msdepthtex);
    if(!mscolortex) glGenTextures(1, &mscolortex);
    if(!msnormaltex) glGenTextures(1, &msnormaltex);
    if(!msglowtex) glGenTextures(1, &msglowtex);
    if(!msfbo) glGenFramebuffers_(1, &msfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, msfbo);

    maskgbuffer("cngd");

    if(msaadepthstencil && hasDS) stencilformat = GL_DEPTH24_STENCIL8;
    else if(msaastencil) stencilformat = GL_STENCIL_INDEX8;

    GLenum fixed = hasMSS && multisampledaa() ? GL_TRUE : GL_FALSE;

    if(gdepthformat)
    {
        if(!msdepthrb) glGenRenderbuffers_(1, &msdepthrb);
        glBindRenderbuffer_(GL_RENDERBUFFER, msdepthrb);
        rbms(msaadepthstencil && hasDS ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT, w, h);
        glBindRenderbuffer_(GL_RENDERBUFFER, 0);
        fixed = GL_TRUE; // spec requires fixed sample locations if renderbuffers are used with textures
    }
    if(msaastencil && (!msaadepthstencil || !hasDS))
    {
        if(!msstencilrb) glGenRenderbuffers_(1, &msstencilrb);
        glBindRenderbuffer_(GL_RENDERBUFFER, msstencilrb);
        rbms(GL_STENCIL_INDEX8, w, h);
        glBindRenderbuffer_(GL_RENDERBUFFER, 0);
        fixed = GL_TRUE; // spec requires fixed sample locations if renderbuffers are used with textures
    }

    static const GLenum depthformats[] = { GL_RGBA8, GL_R16F, GL_R32F };
    GLenum depthformat = gdepthformat ? depthformats[gdepthformat-1] : (msaadepthstencil && hasDS ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
    texms(depthformat, w, h, fixed);
 
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mscolortex);
    texms(GL_RGBA8, w, h, fixed);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msnormaltex);
    texms(GL_RGBA8, w, h, fixed);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msglowtex);
    texms(GL_RGBA8, w, h, fixed);

    bindmsdepth();
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mscolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, msnormaltex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, msglowtex, 0);
    if(gdepthformat) glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D_MULTISAMPLE, msdepthtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating MSAA g-buffer!");

    GLint colorsamples = msaasamples;
    if(msaamincolorsamples < msaasamples) glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_COLOR_SAMPLES_NV, &colorsamples);
    msaacolorsamples = colorsamples;

    memset(msaapositions, 0, sizeof(msaapositions));
    if(fixed) loopi(msaasamples) 
    {
        GLfloat vals[2];
        glGetMultisamplefv_(GL_SAMPLE_POSITION, i, vals);
        msaapositions[i] = vec2(vals[0], vals[1]);
    }

    if(!mshdrtex) glGenTextures(1, &mshdrtex);
    if(!mshdrfbo) glGenFramebuffers_(1, &mshdrfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, mshdrfbo);

    bindmsdepth();

    hdrformat = 0;
    for(int prec = hdr ? hdrprec : 0; prec >= 0; prec--)
    {
        GLenum format = gethdrformat(prec);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mshdrtex);
        glGetError();
        texms(format, w, h, fixed);
        if(glGetError() == GL_NO_ERROR)
        {
            glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mshdrtex, 0);
            if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
            {
                hdrformat = format;
                break;
            }
        }
    }

    if(!hdrformat || glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating MSAA HDR buffer!");

    if(!msrefracttex) glGenTextures(1, &msrefracttex);
    if(!msrefractfbo) glGenFramebuffers_(1, &msrefractfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, msrefractfbo);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msrefracttex);
    texms(GL_RGB, w, h, fixed);

    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msrefracttex, 0);
    bindmsdepth();

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating MSAA refraction buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    useshaderbyname("msaaedgedetect");
    useshaderbyname("msaaresolve");
    useshaderbyname("msaareducew");
    useshaderbyname("msaareduce");
    if(hasMSS && msaatonemap)
    {
        useshaderbyname("msaatonemap");
        useshaderbyname("msaatonemapsample");
    }
}

void bindgdepth()
{
    if(gdepthformat || msaasamples)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gdepthrb);
        if((msaasamples ? msaadepthstencil : gdepthstencil) && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gdepthrb);
        else if(msaasamples ? hasMSS && msaatonemap && msaatonemapstencil && msaastencil : gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gstencilrb);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gstencilrb);
    }
}

void setupgbuffer(int w, int h)
{
    int sw = w, sh = h;
    if(gscale != 100)
    {
        sw = max((w*gscale + 99)/100, 1);
        sh = max((h*gscale + 99)/100, 1);
    }

    if(gw == sw && gh == sh && (gscale == 100 || (scalew == w && scaleh == h))) return;

    cleanupscale();
    cleanupbloom();
    cleanupao();
    cleanupaa();
    cleanuppostfx();

    gw = sw;
    gh = sh;

    hdrformat = gethdrformat(hdr ? hdrprec : 0);
    stencilformat = 0;

    if(msaasamples) setupmsbuffer(gw, gh);

    hdrfloat = floatformat(hdrformat);
    hdrclear = 3;

    if(gdepthformat || msaasamples)
    {
        if(!gdepthrb) glGenRenderbuffers_(1, &gdepthrb);
        glBindRenderbuffer_(GL_RENDERBUFFER, gdepthrb);
        glRenderbufferStorage_(GL_RENDERBUFFER, (msaasamples ? msaadepthstencil : gdepthstencil) && hasDS ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER, 0);
    }
    if(msaasamples ? hasMSS && msaatonemap && msaatonemapstencil && msaastencil && (!hasDS || !msaadepthstencil) : gstencil && (!hasDS || !gdepthstencil))
    {
        if(!gstencilrb) glGenRenderbuffers_(1, &gstencilrb);
        glBindRenderbuffer_(GL_RENDERBUFFER, gstencilrb);
        glRenderbufferStorage_(GL_RENDERBUFFER, GL_STENCIL_INDEX8, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER, 0);
    }

    if(!msaasamples)
    {
        if(!gdepthtex) glGenTextures(1, &gdepthtex);
        if(!gcolortex) glGenTextures(1, &gcolortex);
        if(!gnormaltex) glGenTextures(1, &gnormaltex);
        if(!gglowtex) glGenTextures(1, &gglowtex);
        if(!gfbo) glGenFramebuffers_(1, &gfbo);

        glBindFramebuffer_(GL_FRAMEBUFFER, gfbo);

        maskgbuffer("cngd");

        if(gdepthstencil && hasDS) stencilformat = GL_DEPTH24_STENCIL8;
        else if(gstencil) stencilformat = GL_STENCIL_INDEX8;

        static const GLenum depthformats[] = { GL_RGBA8, GL_R16F, GL_R32F };
        GLenum depthformat = gdepthformat ? depthformats[gdepthformat-1] : (gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT);
        createtexture(gdepthtex, gw, gh, NULL, 3, 0, depthformat, GL_TEXTURE_RECTANGLE);

        createtexture(gcolortex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE);
        createtexture(gnormaltex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE);
        createtexture(gglowtex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE);

        bindgdepth();
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, gcolortex, 0);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, gnormaltex, 0);
        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, gglowtex, 0);
        if(gdepthformat) glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_RECTANGLE, gdepthtex, 0);

        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating g-buffer!");
    }

    if(!hdrtex) glGenTextures(1, &hdrtex);
    if(!hdrfbo) glGenFramebuffers_(1, &hdrfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, hdrfbo);

    createtexture(hdrtex, gw, gh, NULL, 3, 1, hdrformat, GL_TEXTURE_RECTANGLE);

    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, hdrtex, 0);
    bindgdepth();

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating HDR buffer!");

    if(!msaasamples || (hasMSS && msaatonemap && msaatonemapblit))
    {
        if(!refracttex) glGenTextures(1, &refracttex);
        if(!refractfbo) glGenFramebuffers_(1, &refractfbo);

        glBindFramebuffer_(GL_FRAMEBUFFER, refractfbo);

        createtexture(refracttex, gw, gh, NULL, 3, 0, GL_RGB, GL_TEXTURE_RECTANGLE);

        glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, refracttex, 0);
        bindgdepth();

        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("failed allocating refraction buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    if(gscale != 100) setupscale(sw, sh, w, h);
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
    cleanupscale();
    cleanupmsbuffer();
    cleardeferredlightshaders();
}

void resolvemsaacolor(int w = vieww, int h = viewh)
{
    if(!msaasamples) return;

    timer *resolvetimer = drawtex ? NULL : begintimer("msaa resolve");

    glBindFramebuffer_(GL_READ_FRAMEBUFFER, mshdrfbo);
    glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, hdrfbo);
    glBlitFramebuffer_(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer_(GL_FRAMEBUFFER, hdrfbo);

    endtimer(resolvetimer);
}

VARF(gstencil, 0, 0, 1, cleanupgbuffer());
VARF(gdepthstencil, 0, 1, 1, cleanupgbuffer());
FVAR(bloomthreshold, 1e-3f, 0.8f, 1e3f);
FVARP(bloomscale, 0, 1.0f, 1e3f);
VARP(bloomblur, 0, 7, 7);
VARP(bloomiter, 0, 0, 4);
FVARP(bloomsigma, 0.005f, 0.5f, 2.0f);
VARFP(bloomsize, 6, 9, 11, cleanupbloom());
VARFP(bloomprec, 0, 2, 3, cleanupbloom());
FVAR(hdraccumscale, 0, 0.98f, 1);
VAR(hdraccummillis, 1, 33, 1000);
VAR(hdrreduce, 0, 2, 2);
VARFP(hdr, 0, 1, 1, cleanupgbuffer());
VARFP(hdrprec, 0, 2, 3, cleanupgbuffer());
FVARFP(hdrgamma, 1e-3f, 2, 1e3f, initwarning("HDR setup", INIT_LOAD, CHANGE_SHADERS));
FVARR(hdrbright, 1e-4f, 1.0f, 1e4f);
FVAR(hdrsaturate, 1e-3f, 0.8f, 1e3f);
VARFP(gscale, 25, 100, 100, cleanupgbuffer());
VARFP(gscalecubic, 0, 0, 1, cleanupgbuffer());
VARFP(gscalenearest, 0, 0, 1, cleanupgbuffer());
FVARFP(gscalecubicsoft, 0, 0, 1, initwarning("scaling setup", INIT_LOAD, CHANGE_SHADERS));

float ldrscale = 1.0f, ldrscaleb = 1.0f/255;

void readhdr(int w, int h, GLenum format, GLenum type, void *dst, GLenum target, GLuint tex)
{
    if(msaasamples) resolvemsaacolor(w, h);
    GLERROR;

    if(hasCBF || !hdrfloat) glBindFramebuffer_(GL_FRAMEBUFFER, hdrfbo);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE, hdrtex);
        screenquad(w, h);
    }

    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR, GL_TRUE);

    if(dst) glReadPixels(0, 0, w, h, format, type, dst);
    else if(target)
    {
        glBindTexture(target, tex);
        GLERROR;
        glCopyTexImage2D(target, 0, format, 0, 0, w, h, 0);
    }

    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR, GL_FIXED_ONLY);

    hdrclear = 3;
}

void loadhdrshaders(int aa)
{
    switch(aa)
    {
        case AA_LUMA:
            useshaderbyname("hdrtonemapluma");
            useshaderbyname("hdrnopluma");
            if(msaasamples && hasMSS && msaatonemap) useshaderbyname("msaatonemapluma");
            break;
        case AA_VELOCITY:
            useshaderbyname("hdrtonemapvelocity");
            useshaderbyname("hdrnopvelocity");
            if(msaasamples && hasMSS && msaatonemap) useshaderbyname("msaatonemapvelocity");
            break;
        case AA_SPLIT:
            useshaderbyname("msaasplit");
            useshaderbyname("msaatonemapsplit");
            break;
        case AA_SPLIT_LUMA:
            useshaderbyname("msaasplitluma");
            useshaderbyname("msaatonemapsplitluma");
            break;
        case AA_SPLIT_VELOCITY:
            useshaderbyname("msaasplitvelocity");
            useshaderbyname("msaatonemapsplitvelocity");
            break;
        default:
            break;
    }
}

void processldr(GLuint outfbo, int aa)
{
    timer *ldrtimer = begintimer("ldr processing");

    if(aa >= AA_SPLIT)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
        glViewport(0, 0, vieww, viewh);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mshdrtex);
        switch(aa)
        {
            case AA_SPLIT_LUMA: SETSHADER(msaasplitluma); break;
            case AA_SPLIT_VELOCITY:
                SETSHADER(msaasplitvelocity);
                setaavelocityparams(GL_TEXTURE3);
                break;
            default: SETSHADER(msaasplit); break;
        }
        screenquad(vieww, viewh);

        endtimer(ldrtimer);
        return;
    }

    if(msaasamples)
    {
        glBindFramebuffer_(GL_READ_FRAMEBUFFER, mshdrfbo);
        glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, outfbo && !aa ? outfbo : hdrfbo);
        glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, vieww, viewh, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        if(outfbo && !aa) return;
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
    glViewport(0, 0, vieww, viewh);
    switch(aa)
    {
        case AA_LUMA: SETSHADER(hdrnopluma); break;
        case AA_VELOCITY:
            SETSHADER(hdrnopvelocity);
            setaavelocityparams(GL_TEXTURE3);
            break;
        default: SETSHADER(hdrnop); break;
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, hdrtex);
    screenquad(vieww, viewh);
    
    endtimer(ldrtimer);
}

void processhdr(GLuint outfbo, int aa)
{
    if(!hdr) { processldr(outfbo, aa); return; }

    timer *hdrtimer = begintimer("hdr processing");

    GLOBALPARAMF(hdrparams, (hdrbright, hdrsaturate, bloomthreshold, bloomscale));
    GLOBALPARAMF(hdrgamma, (hdrgamma, 1.0f/hdrgamma));

    GLuint b0fbo = bloomfbo[1], b0tex = bloomtex[1], b1fbo =  bloomfbo[0], b1tex = bloomtex[0], ptex = hdrtex;
    int b0w = max(vieww/4, bloomw), b0h = max(viewh/4, bloomh), b1w = max(vieww/2, bloomw), b1h = max(viewh/2, bloomh),
        pw = vieww, ph = viewh;
    if(msaasamples)
    {
        if(!hasMSS || (!msaatonemap && aa < AA_SPLIT))
        {
            glBindFramebuffer_(GL_READ_FRAMEBUFFER, mshdrfbo);
            glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, hdrfbo);
            glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, vieww, viewh, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        else if(hasFBMSBS && (vieww > bloomw || viewh > bloomh))
        {
            int cw = max(vieww/2, bloomw), ch = max(viewh/2, bloomh);
            glBindFramebuffer_(GL_READ_FRAMEBUFFER, mshdrfbo);
            glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, hdrfbo);
            glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, cw, ch, GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_FASTEST_EXT);
            pw = cw;
            ph = ch;
        }
        else
        {
            glBindFramebuffer_(GL_FRAMEBUFFER, hdrfbo);
            if(vieww/2 >= bloomw)
            {
                pw = vieww/2;
                if(viewh/2 >= bloomh)
                {
                    ph = viewh/2;
                    glViewport(0, 0, pw, ph);
                    SETSHADER(msaareduce);
                }
                else
                {
                    glViewport(0, 0, pw, viewh);
                    SETSHADER(msaareducew);
                }
            }
            else
            {
                glViewport(0, 0, vieww, viewh);
                SETSHADER(msaaresolve);
            }
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mshdrtex);
            screenquad(vieww, viewh);
        }
    }
    if(hdrreduce) while(pw > bloomw || ph > bloomh)
    {
        GLuint cfbo = b1fbo, ctex = b1tex;
        int cw = max(pw/2, bloomw), ch = max(ph/2, bloomh);

        if(hdrreduce > 1 && cw/2 >= bloomw)
        {
            cw /= 2;
            if(ch/2 >= bloomh)
            {
                ch /= 2;
                SETSHADER(hdrreduce2);
            }
            else SETSHADER(hdrreduce2w);
        }
        else SETSHADER(hdrreduce);
        if(cw == bloomw && ch == bloomh) { if(bloomfbo[5]) { cfbo = bloomfbo[5]; ctex = bloomtex[5]; } else { cfbo = bloomfbo[2]; ctex = bloomtex[2]; } }
        glBindFramebuffer_(GL_FRAMEBUFFER, cfbo);
        glViewport(0, 0, cw, ch);
        glBindTexture(GL_TEXTURE_RECTANGLE, ptex);
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

            if(hdrreduce > 1 && cw/2 >= 2)
            {
                cw /= 2;
                if(ch/2 >= 2)
                {
                    ch /= 2;
                    if(i) SETSHADER(hdrreduce2); else SETSHADER(hdrluminance2);
                }
                else if(i) SETSHADER(hdrreduce2w); else SETSHADER(hdrluminance2w);
            }
            else if(i) SETSHADER(hdrreduce); else SETSHADER(hdrluminance);
            glBindFramebuffer_(GL_FRAMEBUFFER, b1fbo);
            glViewport(0, 0, cw, ch);
            glBindTexture(GL_TEXTURE_RECTANGLE, ltex);
            screenquad(lw, lh);

            ltex = b1tex;
            lw = cw;
            lh = ch;
            swap(b0fbo, b1fbo);
            swap(b0tex, b1tex);
            swap(b0w, b1w);
            swap(b0h, b1h);
        }

        glBindFramebuffer_(GL_FRAMEBUFFER, bloomfbo[4]);
        glViewport(0, 0, 1, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        SETSHADER(hdraccum);
        glBindTexture(GL_TEXTURE_RECTANGLE, b0tex);
        LOCALPARAMF(accumscale, (lasthdraccum ? pow(hdraccumscale, float(lastmillis - lasthdraccum)/hdraccummillis) : 0));
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

    glActiveTexture_(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bloomtex[4]);
    glActiveTexture_(GL_TEXTURE0);

    glBindFramebuffer_(GL_FRAMEBUFFER, b0fbo);
    glViewport(0, 0, b0w, b0h);
    SETSHADER(hdrbloom);
    glBindTexture(GL_TEXTURE_RECTANGLE, ptex);
    screenquad(pw, ph);

    if(bloomblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(bloomblur, bloomsigma, blurweights, bluroffsets);
        loopi(2 + 2*bloomiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER, b1fbo);
            glViewport(0, 0, b1w, b1h);
            setblurshader(i%2, 1, bloomblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE);
            glBindTexture(GL_TEXTURE_RECTANGLE, b0tex);
            screenquad(b0w, b0h);
            swap(b0w, b1w);
            swap(b0h, b1h);
            swap(b0tex, b1tex);
            swap(b0fbo, b1fbo);
        }
    }

    if(aa >= AA_SPLIT)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
        glViewport(0, 0, vieww, viewh);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mshdrtex);
        glActiveTexture_(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_RECTANGLE, b0tex);
        glActiveTexture_(GL_TEXTURE0);
        switch(aa)
        {
            case AA_SPLIT_LUMA: SETSHADER(msaatonemapsplitluma); break;
            case AA_SPLIT_VELOCITY:
                SETSHADER(msaatonemapsplitvelocity);
                setaavelocityparams(GL_TEXTURE3);
                break;
            default: SETSHADER(msaatonemapsplit); break;
        }
        screenquad(vieww, viewh, b0w, b0h);
    }
    else if(!msaasamples || !hasMSS || !msaatonemap)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
        glViewport(0, 0, vieww, viewh);
        glBindTexture(GL_TEXTURE_RECTANGLE, hdrtex);
        glActiveTexture_(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_RECTANGLE, b0tex);
        glActiveTexture_(GL_TEXTURE0);
        switch(aa)
        {
            case AA_LUMA: SETSHADER(hdrtonemapluma); break;
            case AA_VELOCITY:
                SETSHADER(hdrtonemapvelocity);
                setaavelocityparams(GL_TEXTURE3);
                break;
            default: SETSHADER(hdrtonemap); break;
        }
        screenquad(vieww, viewh, b0w, b0h);
    }
    else
    {
        bool blit = false, stencil = false;
        if(msaatonemapblit && (!aa || !outfbo)) 
        { 
            blit = true; 
            if(msaatonemapstencil && stencilformat) stencil = true; 
        }
        else if(msaatonemapstencil && stencilformat)
        {
            stencil = true;
            glBindFramebuffer_(GL_READ_FRAMEBUFFER, mshdrfbo);
            glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, hdrfbo);
            glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, vieww, viewh, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }

        glBindFramebuffer_(GL_FRAMEBUFFER, blit ? msrefractfbo : (stencil && !outfbo ? hdrfbo : outfbo));
        glViewport(0, 0, vieww, viewh);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mshdrtex);
        glActiveTexture_(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_RECTANGLE, b0tex);
        glActiveTexture_(GL_TEXTURE0);

        if(stencil) 
        {
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_EQUAL, 0, 0x08);
            switch(aa)
            {
                case AA_LUMA: SETSHADER(msaatonemapluma); break;
                case AA_VELOCITY:
                    SETSHADER(msaatonemapvelocity);
                    setaavelocityparams(GL_TEXTURE3);
                    break;
                default: SETSHADER(msaatonemap); break;
            }
            screenquad(vieww, viewh, b0w, b0h);

            glStencilFunc(GL_EQUAL, 0x08, 0x08);
        }

        if(blit) SETSHADER(msaatonemapsample);
        else switch(aa)
        {
            case AA_LUMA: SETVARIANT(msaatonemapluma, 0, 0); break;
            case AA_VELOCITY: 
                SETVARIANT(msaatonemapvelocity, 0, 0);
                setaavelocityparams(GL_TEXTURE3);
                break;
            default: SETVARIANT(msaatonemap, 0, 0); break;
        }
        screenquad(vieww, viewh, b0w, b0h);
   
        if(stencil) glDisable(GL_STENCIL_TEST);

        if(blit)
        {
            glBindFramebuffer_(GL_READ_FRAMEBUFFER, msrefractfbo);
            glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, aa || !outfbo ? refractfbo : outfbo);
            glBlitFramebuffer_(0, 0, vieww, viewh, 0, 0, vieww, viewh, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        if((blit || stencil) && !outfbo)
        { 
            glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
            glViewport(0, 0, vieww, viewh);
            if(!blit) SETSHADER(hdrnop);
            else switch(aa)
            {
                case AA_LUMA: SETSHADER(hdrnopluma); break;
                case AA_VELOCITY: 
                    SETSHADER(hdrnopvelocity); 
                    setaavelocityparams(GL_TEXTURE3);
                    break;
                default: SETSHADER(hdrnop); break;
            }    
            glBindTexture(GL_TEXTURE_RECTANGLE, blit ? refracttex : hdrtex);
            screenquad(vieww, viewh);
        }
    }

    endtimer(hdrtimer);
}

VAR(debugdepth, 0, 0, 1);

void viewdepth()
{
    int w = min(screenw, screenh)/2, h = (w*screenh)/screenw;
    SETSHADER(hudrect);
    gle::colorf(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
    debugquad(0, 0, w, h, 0, 0, gw, gh);
}

VAR(debugrefract, 0, 0, 1);

void viewrefract()
{
    int w = min(screenw, screenh)/2, h = (w*screenh)/screenw;
    SETSHADER(hudrect);
    gle::colorf(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE, refracttex);
    debugquad(0, 0, w, h, 0, 0, gw, gh);
}

#define RH_MAXSPLITS 4

GLuint rhtex[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }, rhfbo = 0;
GLuint rsmdepthtex = 0, rsmcolortex = 0, rsmnormaltex = 0, rsmfbo = 0;

extern int rhgrid, rhsplits, rhborder, rhprec, rhtaps, rhcache, rsmprec, rsmsize;

static Shader *radiancehintsshader = NULL;

Shader *loadradiancehintsshader()
{
    defformatstring(name)("radiancehints%d", rhtaps);
    return generateshader(name, "radiancehintsshader %d", rhtaps);
}

void loadrhshaders()
{
    if(rhborder) useshaderbyname("radiancehintsborder");
    if(rhcache) useshaderbyname("radiancehintscached");
    radiancehintsshader = loadradiancehintsshader();
}

void clearrhshaders()
{
    radiancehintsshader = NULL;
}

void setupradiancehints()
{
    loopi(rhcache ? 8 : 4) if(!rhtex[i]) glGenTextures(1, &rhtex[i]);

    if(!rhfbo) glGenFramebuffers_(1, &rhfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, rhfbo);

    GLenum rhformat = hasTF && rhprec >= 1 ? GL_RGBA16F : GL_RGBA8;

    loopi(rhcache ? 8 : 4)
    {
        create3dtexture(rhtex[i], rhgrid+2*rhborder, rhgrid+2*rhborder, (rhgrid+2*rhborder)*rhsplits, NULL, 7, 1, rhformat);
        if(rhborder)
        {
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            GLfloat border[4] = { 0.5f, 0.5f, 0.5f, 0 };
            glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border);
        }
        glFramebufferTexture3D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_3D, rhtex[i], 0, 0);
    }

    static const GLenum drawbufs[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers_(4, drawbufs);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating radiance hints buffer!");

    if(!rsmdepthtex) glGenTextures(1, &rsmdepthtex);
    if(!rsmcolortex) glGenTextures(1, &rsmcolortex);
    if(!rsmnormaltex) glGenTextures(1, &rsmnormaltex);

    if(!rsmfbo) glGenFramebuffers_(1, &rsmfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, rsmfbo);

    GLenum rsmformat = gethdrformat(rsmprec, GL_RGBA8);

    createtexture(rsmdepthtex, rsmsize, rsmsize, NULL, 3, 0, GL_DEPTH_COMPONENT16, GL_TEXTURE_RECTANGLE);
    createtexture(rsmcolortex, rsmsize, rsmsize, NULL, 3, 0, rsmformat, GL_TEXTURE_RECTANGLE);
    createtexture(rsmnormaltex, rsmsize, rsmsize, NULL, 3, 0, rsmformat, GL_TEXTURE_RECTANGLE);

    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, rsmdepthtex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rsmcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, rsmnormaltex, 0);

    glDrawBuffers_(2, drawbufs);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating RSM buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);

    loadrhshaders();
}

void cleanupradiancehints()
{
    clearradiancehintscache();

    loopi(8) if(rhtex[i]) { glDeleteTextures(1, &rhtex[i]); rhtex[i] = 0; }
    if(rhfbo) { glDeleteFramebuffers_(1, &rhfbo); rhfbo = 0; }
    if(rsmdepthtex) { glDeleteTextures(1, &rsmdepthtex); rsmdepthtex = 0; }
    if(rsmcolortex) { glDeleteTextures(1, &rsmcolortex); rsmcolortex = 0; }
    if(rsmnormaltex) { glDeleteTextures(1, &rsmnormaltex); rsmnormaltex = 0; }
    if(rsmfbo) { glDeleteFramebuffers_(1, &rsmfbo); rsmfbo = 0; }

    clearrhshaders();
}

VARF(rhsplits, 1, 2, RH_MAXSPLITS, { cleardeferredlightshaders(); cleanupradiancehints(); });
VARF(rhborder, 0, 1, 1, cleanupradiancehints());
VARF(rsmsize, 64, 384, 2048, cleanupradiancehints());
VARF(rhnearplane, 1, 1, 16, clearradiancehintscache());
VARF(rhfarplane, 64, 1024, 16384, clearradiancehintscache());
FVARF(rsmpradiustweak, 1e-3f, 1, 1e3f, clearradiancehintscache());
FVARF(rhpradiustweak, 1e-3f, 1, 1e3f, clearradiancehintscache());
FVARF(rsmdepthrange, 0, 1024, 1e6f, clearradiancehintscache());
FVARF(rsmdepthmargin, 0, 0.1f, 1e3f, clearradiancehintscache());
VARF(rhprec, 0, 0, 1, cleanupradiancehints());
VARF(rsmprec, 0, 0, 3, cleanupradiancehints());
FVAR(rhnudge, 0, 0.5f, 4);
FVARF(rhsplitweight, 0.20f, 0.6f, 0.95f, clearradiancehintscache());
VARF(rhgrid, 3, 27, 128, cleanupradiancehints());
FVARF(rsmspread, 0, 0.15f, 1, clearradiancehintscache());
VAR(rhclipgrid, 0, 1, 1);
VARF(rhcache, 0, 1, 1, cleanupradiancehints());
VAR(rsmcull, 0, 1, 1);
VARFP(rhtaps, 0, 20, 32, cleanupradiancehints());
VAR(rhdyntex, 0, 0, 1);
VAR(rhdynmm, 0, 0, 1);
VARFR(gidist, 0, 384, 1024, { clearradiancehintscache(); cleardeferredlightshaders(); if(!gidist) cleanupradiancehints(); });
FVARFR(giscale, 0, 1.5f, 1e3f, { cleardeferredlightshaders(); if(!giscale) cleanupradiancehints(); });
FVARR(giaoscale, 0, 3, 1e3f);
VARFP(gi, 0, 1, 1, { cleardeferredlightshaders(); cleanupradiancehints(); });

VAR(debugrsm, 0, 0, 2);
void viewrsm()
{
    int w = min(screenw, screenh)/2, h = (w*screenh)/screenw, x = screenw-w, y = screenh-h;
    SETSHADER(hudrect);
    gle::colorf(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE, debugrsm == 2 ? rsmnormaltex : rsmcolortex);
    debugquad(x, y, w, h, 0, 0, rsmsize, rsmsize);
}

VAR(debugrh, 0, 0, RH_MAXSPLITS*(128 + 2));
void viewrh()
{
    int w = min(screenw, screenh)/2, h = (w*screenh)/screenw, x = screenw-w, y = screenh-h;
    SETSHADER(hud3d);
    gle::colorf(1, 1, 1);
    glBindTexture(GL_TEXTURE_3D, rhtex[1]);
    float z = (debugrh-1+0.5f)/float((rhgrid+2*rhborder)*rhsplits);
    gle::defvertex(2);
    gle::deftexcoord0(3);
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(x,   y);   gle::attribf(0, 0, z);
    gle::attribf(x+w, y);   gle::attribf(1, 0, z);
    gle::attribf(x,   y+h); gle::attribf(0, 1, z);
    gle::attribf(x+w, y+h); gle::attribf(1, 1, z);
    gle::end();
    gle::disable();
}

#define SHADOWATLAS_SIZE 4096

PackNode shadowatlaspacker(0, 0, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE);

struct lightinfo
{
    float sx1, sy1, sx2, sy2, sz1, sz2;
    int ent, shadowmap, flags;
    vec o, color;
    float radius;
    vec dir, spotx, spoty;
    int spot;
    float dist;
    occludequery *query;

    void calcspot(const vec &spotdir, int spotangle)
    {
        dir = spotdir;
        spot = spotangle;
        vec adir(fabs(dir.x), fabs(dir.y), fabs(dir.z));
        spotx = vec(1, 0, 0);
        spoty = vec(0, 1, 0);
        if(adir.x > adir.y) { if(adir.x > adir.z) spotx = vec(0, 0, 1); }
        else if(adir.y > adir.z) spoty = vec(0, 0, 1);
        dir.orthonormalize(spotx, spoty);
        const vec2 &sc = sincos360[spot];
        float spotscale = sc.x/sc.y;
        spotx.rescale(spotscale);
        spoty.rescale(spotscale);
    }
};

struct shadowcachekey
{
    vec o;
    float radius;
    vec dir;
    int spot;

    shadowcachekey() {}
    shadowcachekey(const lightinfo &l) : o(l.o), radius(l.radius), dir(l.dir), spot(l.spot) {}
};

static inline uint hthash(const shadowcachekey &k)
{
    return hthash(k.o);
}

static inline bool htcmp(const shadowcachekey &x, const shadowcachekey &y)
{
    return x.o == y.o && x.radius == y.radius && x.dir == y.dir && x.spot == y.spot;
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

extern int smcache, smfilter, smgather;

#define SHADOWCACHE_EVICT 2

GLuint shadowatlastex = 0, shadowatlasfbo = 0;
GLenum shadowatlastarget = GL_NONE;
shadowcache shadowcache;
bool shadowcachefull = false;
int evictshadowcache = 0;

static inline void setsmnoncomparemode() // use texture gather
{
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static inline void setsmcomparemode() // use embedded shadow cmp
{
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

extern int usetexgather;
static inline bool usegatherforsm() { return smfilter > 1 && smgather && (hasTG || hasT4) && usetexgather; }
static inline bool usesmcomparemode() { return !usegatherforsm() || (hasTG && hasGPU5 && usetexgather > 1); }

void viewshadowatlas()
{
    int w = min(screenw, screenh)/2, h = (w*screenh)/screenw, x = screenw-w, y = screenh-h;
    float tw = 1, th = 1;
    if(shadowatlastarget == GL_TEXTURE_RECTANGLE)
    {
        tw = shadowatlaspacker.w;
        th = shadowatlaspacker.h;
        SETSHADER(hudrect);
    }
    else hudshader->set();
    gle::colorf(1, 1, 1);
    glBindTexture(shadowatlastarget, shadowatlastex);
    if(usesmcomparemode()) setsmnoncomparemode();
    debugquad(x, y, w, h, 0, 0, tw, th);
    if(usesmcomparemode()) setsmcomparemode();
}
VAR(debugshadowatlas, 0, 0, 1);

extern int smsize;

void setupshadowatlas()
{
    int size = min((1<<smsize), hwtexsize);
    shadowatlaspacker.resize(size, size);

    if(!shadowatlastex) glGenTextures(1, &shadowatlastex);

    shadowatlastarget = usegatherforsm() ? GL_TEXTURE_2D : GL_TEXTURE_RECTANGLE;
    createtexture(shadowatlastex, shadowatlaspacker.w, shadowatlaspacker.h, NULL, 3, 1, GL_DEPTH_COMPONENT16, shadowatlastarget);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    if(!shadowatlasfbo) glGenFramebuffers_(1, &shadowatlasfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER, shadowatlasfbo);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowatlastarget, shadowatlastex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal("failed allocating shadow atlas!");

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);
}

void cleanupshadowatlas()
{
    if(shadowatlastex) { glDeleteTextures(1, &shadowatlastex); shadowatlastex = 0; }
    if(shadowatlasfbo) { glDeleteFramebuffers_(1, &shadowatlasfbo); shadowatlasfbo = 0; }
    clearshadowcache();
}

const glmatrix cubeshadowviewmatrix[6] =
{
    // sign-preserving cubemap projections
    glmatrix(vec(0, 0, 1), vec(0, 1, 0), vec(-1, 0, 0)), // +X
    glmatrix(vec(0, 0, 1), vec(0, 1, 0), vec( 1, 0, 0)), // -X
    glmatrix(vec(1, 0, 0), vec(0, 0, 1), vec(0, -1, 0)), // +Y
    glmatrix(vec(1, 0, 0), vec(0, 0, 1), vec(0,  1, 0)), // -Y
    glmatrix(vec(1, 0, 0), vec(0, 1, 0), vec(0, 0, -1)), // +Z
    glmatrix(vec(1, 0, 0), vec(0, 1, 0), vec(0, 0,  1))  // -Z
};

FVAR(smpolyfactor, -1e3f, 1, 1e3f);
FVAR(smpolyoffset, -1e3f, 0, 1e3f);
FVAR(smbias, -1e6f, 0.01f, 1e6f);
FVAR(smpolyfactor2, -1e3f, 1.5f, 1e3f);
FVAR(smpolyoffset2, -1e3f, 0, 1e3f);
FVAR(smbias2, -1e6f, 0.02f, 1e6f);
FVAR(smprec, 1e-3f, 1, 1e3f);
FVAR(smcubeprec, 1e-3f, 1, 1e3f);
FVAR(smspotprec, 1e-3f, 1, 1e3f);

VARFP(smsize, 10, 12, 14, cleanupshadowatlas());
VAR(smsidecull, 0, 1, 1);
VAR(smviscull, 0, 1, 1);
VAR(smborder, 0, 3, 16);
VAR(smborder2, 0, 4, 16);
VAR(smminradius, 0, 16, 10000);
VAR(smminsize, 1, 96, 1024);
VAR(smmaxsize, 1, 384, 1024);
//VAR(smmaxsize, 1, 4096, 4096);
VAR(smused, 1, 0, 0);
VAR(smquery, 0, 1, 1);
VARF(smcullside, 0, 1, 1, cleanupshadowatlas());
VARF(smcache, 0, 1, 2, cleanupshadowatlas());
VARFP(smfilter, 0, 2, 3, { cleardeferredlightshaders(); cleanupshadowatlas(); });
VARFP(smgather, 0, 0, 1, { cleardeferredlightshaders(); cleanupshadowatlas(); });
VAR(smnoshadow, 0, 0, 1);
VAR(smdynshadow, 0, 1, 1);
VAR(lighttilesused, 1, 0, 0);
VAR(lightpassesused, 1, 0, 0);

int shadowmapping = 0;

vector<lightinfo> lights;
vector<int> lightorder;
vector<int> lighttiles[LIGHTTILE_MAXH][LIGHTTILE_MAXW];
vector<shadowmapinfo> shadowmaps;

void clearshadowcache()
{
    shadowmaps.setsize(0);

    clearradiancehintscache();
    clearshadowmeshes();
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

#define CSM_MAXSPLITS 8

VARF(csmmaxsize, 256, 768, 2048, clearshadowcache());
VARF(csmsplits, 1, 3, CSM_MAXSPLITS, { cleardeferredlightshaders(); clearshadowcache(); });
FVAR(csmsplitweight, 0.20f, 0.75f, 0.95f);
VARF(csmshadowmap, 0, 1, 1, cleardeferredlightshaders());

// cascaded shadow maps
struct cascadedshadowmap
{
    struct splitinfo
    {
        float nearplane;     // split distance to near plane
        float farplane;      // split distance to farplane
        glmatrix proj;      // one projection per split
        vec scale, offset;   // scale and offset of the projection
        int idx;             // shadowmapinfo indices
        vec center, bounds;  // max extents of shadowmap in sunlight model space
        plane cull[4];       // world space culling planes of the split's projected sides
    };
    glmatrix model;                // model view is shared by all splits
    splitinfo splits[CSM_MAXSPLITS]; // per-split parameters
    vec lightview;                  // view vector for light
    void setup();                   // insert shadowmaps for each split frustum if there is sunlight
    void updatesplitdist();         // compute split frustum distances
    void getmodelmatrix();          // compute the shared model matrix
    void getprojmatrix();           // compute each cropped projection matrix
    void gencullplanes();           // generate culling planes for the mvp matrix
    void bindparams();              // bind any shader params necessary for lighting
};

void cascadedshadowmap::setup()
{
    int size = (csmmaxsize * shadowatlaspacker.w) / SHADOWATLAS_SIZE;
    loopi(csmsplits)
    {
        ushort smx = USHRT_MAX, smy = USHRT_MAX;
        splits[i].idx = -1;
        if(shadowatlaspacker.insert(smx, smy, size, size))
            addshadowmap(smx, smy, size, splits[i].idx);
    }
    getmodelmatrix();
    getprojmatrix();
    gencullplanes();
}

VAR(csmnearplane, 1, 1, 16);
VAR(csmfarplane, 64, 1024, 16384);
FVAR(csmpradiustweak, 1e-3f, 1, 1e3f);
FVAR(csmdepthrange, 0, 1024, 1e6f);
FVAR(csmdepthmargin, 0, 0.1f, 1e3f);
FVAR(csmpolyfactor, -1e3f, 2, 1e3f);
FVAR(csmpolyoffset, -1e4f, 0, 1e4f);
FVAR(csmbias, -1e6f, 1e-4f, 1e6f);
FVAR(csmpolyfactor2, -1e3f, 3, 1e3f);
FVAR(csmpolyoffset2, -1e4f, 0, 1e4f);
FVAR(csmbias2, -1e16f, 2e-4f, 1e6f);
VAR(csmcull, 0, 1, 1);

void cascadedshadowmap::updatesplitdist()
{
    float lambda = csmsplitweight, nd = csmnearplane, fd = csmfarplane, ratio = fd/nd;
    splits[0].nearplane = nd;
    for(int i = 1; i < csmsplits; ++i)
    {
        float si = i / float(csmsplits);
        splits[i].nearplane = lambda*(nd*pow(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
        splits[i-1].farplane = splits[i].nearplane * 1.005f;
    }
    splits[csmsplits-1].farplane = fd;
}

void cascadedshadowmap::getmodelmatrix()
{
    model = viewmatrix;
    model.rotate_around_x(sunlightpitch*RAD);
    model.rotate_around_z((180-sunlightyaw)*RAD);
}

void cascadedshadowmap::getprojmatrix()
{
    lightview = vec(sunlightdir).neg();

    // compute the split frustums
    updatesplitdist();

    // find z extent
    float minz = lightview.project_bb(worldmin, worldmax), maxz = lightview.project_bb(worldmax, worldmin),
          zmargin = max((maxz - minz)*csmdepthmargin, 0.5f*(csmdepthrange - (maxz - minz)));
    minz -= zmargin;
    maxz += zmargin;

    // compute each split projection matrix
    loopi(csmsplits)
    {
        splitinfo &split = splits[i];
        if(split.idx < 0) continue;
        const shadowmapinfo &sm = shadowmaps[split.idx];

        vec c;
        float radius = calcfrustumboundsphere(split.nearplane, split.farplane, camera1->o, camdir, c);

        // compute the projected bounding box of the sphere
        vec tc;
        model.transform(c, tc);
        int border = smfilter > 2 ? smborder2 : smborder;
        const float pradius = ceil(radius * csmpradiustweak), step = (2*pradius) / (sm.size - 2*border);
        vec2 offset = vec2(tc).sub(pradius).div(step);
        offset.x = floor(offset.x);
        offset.y = floor(offset.y);
        split.center = vec(vec2(offset).mul(step).add(pradius), -0.5f*(minz + maxz));
        split.bounds = vec(pradius, pradius, 0.5f*(maxz - minz));

        // modify mvp with a scale and offset
        // now compute the update model view matrix for this split
        split.scale = vec(1/step, 1/step, -1/(maxz - minz));
        split.offset = vec(border - offset.x, border - offset.y, -minz/(maxz - minz));

        split.proj.identity();
        split.proj.settranslation(2*split.offset.x/sm.size - 1, 2*split.offset.y/sm.size - 1, 2*split.offset.z - 1);
        split.proj.setscale(2*split.scale.x/sm.size, 2*split.scale.y/sm.size, 2*split.scale.z);

        const float bias = (smfilter > 2 ? csmbias2 : csmbias) * (-512.0f / sm.size) * (split.farplane - split.nearplane) / (splits[0].farplane - splits[0].nearplane);
        split.offset.add(vec(sm.x, sm.y, bias));
    }
}

void cascadedshadowmap::gencullplanes()
{
    loopi(csmsplits)
    {
        splitinfo &split = splits[i];
        glmatrix mvp;
        mvp.mul(split.proj, model);
        vec4 px = mvp.getrow(0), py = mvp.getrow(1), pw = mvp.getrow(3);
        split.cull[0] = plane(vec4(pw).add(px)).normalize(); // left plane
        split.cull[1] = plane(vec4(pw).sub(px)).normalize(); // right plane
        split.cull[2] = plane(vec4(pw).add(py)).normalize(); // bottom plane
        split.cull[3] = plane(vec4(pw).sub(py)).normalize(); // top plane
    }
}

void cascadedshadowmap::bindparams()
{
    static GlobalShaderParam splitcenter("splitcenter"), splitbounds("splitbounds"), splitscale("splitscale"), splitoffset("splitoffset");
    vec *splitcenterv = splitcenter.reserve<vec>(csmsplits),
        *splitboundsv = splitbounds.reserve<vec>(csmsplits),
        *splitscalev = splitscale.reserve<vec>(csmsplits),
        *splitoffsetv = splitoffset.reserve<vec>(csmsplits);
    loopi(csmsplits)
    {
        cascadedshadowmap::splitinfo &split = splits[i];
        if(split.idx < 0) continue;
        splitcenterv[i] = split.center;
        splitboundsv[i] = split.bounds;
        splitscalev[i] = split.scale;
        splitoffsetv[i] = split.offset;
    }
    GLOBALPARAM(csmmatrix, glmatrix3x3(model));
}

cascadedshadowmap csm;

int calcbbcsmsplits(const ivec &bbmin, const ivec &bbmax)
{
    int mask = (1<<csmsplits)-1;
    if(!csmcull) return mask;
    loopi(csmsplits)
    {
        const cascadedshadowmap::splitinfo &split = csm.splits[i];
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
    int mask = (1<<csmsplits)-1;
    if(!csmcull) return mask;
    loopi(csmsplits)
    {
        const cascadedshadowmap::splitinfo &split = csm.splits[i];
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

struct reflectiveshadowmap
{
    glmatrix model, proj;
    vec lightview;
    plane cull[4];
    vec scale, offset;
    vec center, bounds;
    void setup();
    void getmodelmatrix();
    void getprojmatrix();
    void gencullplanes();
} rsm;

void reflectiveshadowmap::setup()
{
    getmodelmatrix();
    getprojmatrix();
    gencullplanes();
}

void reflectiveshadowmap::getmodelmatrix()
{
    model = viewmatrix;
    model.rotate_around_x(sunlightpitch*RAD);
    model.rotate_around_z((180-sunlightyaw)*RAD);
}

void reflectiveshadowmap::getprojmatrix()
{
    lightview = vec(sunlightdir).neg();

    // find z extent
    float minz = lightview.project_bb(worldmin, worldmax), maxz = lightview.project_bb(worldmax, worldmin),
          zmargin = max((maxz - minz)*rsmdepthmargin, 0.5f*(rsmdepthrange - (maxz - minz)));
    minz -= zmargin;
    maxz += zmargin;

    vec c;
    float radius = calcfrustumboundsphere(rhnearplane, rhfarplane, camera1->o, camdir, c);

    // compute the projected bounding box of the sphere
    vec tc;
    model.transform(c, tc);
    const float pradius = ceil((radius + gidist) * rsmpradiustweak), step = (2*pradius) / rsmsize;
    vec2 tcoff = vec2(tc).sub(pradius).div(step);
    tcoff.x = floor(tcoff.x);
    tcoff.y = floor(tcoff.y);
    center = vec(vec2(tcoff).mul(step).add(pradius), -0.5f*(minz + maxz));
    bounds = vec(pradius, pradius, 0.5f*(maxz - minz));

    scale = vec(1/step, 1/step, -1/(maxz - minz));
    offset = vec(-tcoff.x, -tcoff.y, -minz/(maxz - minz));

    proj.identity();
    proj.settranslation(2*offset.x/rsmsize - 1, 2*offset.y/rsmsize - 1, 2*offset.z - 1);
    proj.setscale(2*scale.x/rsmsize, 2*scale.y/rsmsize, 2*scale.z);
}

void reflectiveshadowmap::gencullplanes()
{
    glmatrix mvp;
    mvp.mul(proj, model);
    vec4 px = mvp.getrow(0), py = mvp.getrow(1), pw = mvp.getrow(3);
    cull[0] = plane(vec4(pw).add(px)).normalize(); // left plane
    cull[1] = plane(vec4(pw).sub(px)).normalize(); // right plane
    cull[2] = plane(vec4(pw).add(py)).normalize(); // bottom plane
    cull[3] = plane(vec4(pw).sub(py)).normalize(); // top plane
}

int calcbbrsmsplits(const ivec &bbmin, const ivec &bbmax)
{
    if(!rsmcull) return 1;
    loopk(4)
    {
        const plane &p = rsm.cull[k];
        ivec omin, omax;
        if(p.x > 0) { omin.x = bbmin.x; omax.x = bbmax.x; } else { omin.x = bbmax.x; omax.x = bbmin.x; }
        if(p.y > 0) { omin.y = bbmin.y; omax.y = bbmax.y; } else { omin.y = bbmax.y; omax.y = bbmin.y; }
        if(p.z > 0) { omin.z = bbmin.z; omax.z = bbmax.z; } else { omin.z = bbmax.z; omax.z = bbmin.z; }
        if(omax.dist(p) < 0) return 0;
        if(omin.dist(p) < 0) while(++k < 4)
        {
            const plane &p = rsm.cull[k];
            ivec omax(p.x > 0 ? bbmax.x : bbmin.x, p.y > 0 ? bbmax.y : bbmin.y, p.z > 0 ? bbmax.z : bbmin.z);
            if(omax.dist(p) < 0) return 0;
        }
    }
    return 1;
}

int calcspherersmsplits(const vec &center, float radius)
{
    if(!rsmcull) return 1;
    loopk(4)
    {
        const plane &p = rsm.cull[k];
        float dist = p.dist(center);
        if(dist < -radius) return 0;
        if(dist < radius) while(++k < 4)
        {
            const plane &p = rsm.cull[k];
            if(p.dist(center) < -radius) return 0;
        }
    }
    return 1;
}

struct radiancehints
{
    struct splitinfo
    {
        float nearplane, farplane;
        vec offset, scale;
        vec center; float bounds;
        vec cached;

        splitinfo() : center(-1e16f, -1e16f, -1e16f), bounds(-1e16f), cached(-1e16f, -1e16f, -1e16f) {}

        void clearcache() { bounds = -1e16f; }
    } splits[RH_MAXSPLITS];

    vec dynmin, dynmax, prevdynmin, prevdynmax;

    radiancehints() : dynmin(1e16f, 1e16f, 1e16f), dynmax(-1e16f, -1e16f, -1e16f), prevdynmin(1e16f, 1e16f, 1e16f), prevdynmax(-1e16f, -1e16f, -1e16f) {}

    void setup();
    void updatesplitdist();
    void bindparams();
    void renderslices();

    void clearcache() { loopi(RH_MAXSPLITS) splits[i].clearcache(); }
    bool allcached() const { loopi(rhsplits) if(splits[i].cached != splits[i].center) return false; return true; }
} rh;

void clearradiancehintscache()
{
    rh.clearcache();
}

void radiancehints::updatesplitdist()
{
    float lambda = rhsplitweight, nd = rhnearplane, fd = rhfarplane, ratio = fd/nd;
    splits[0].nearplane = nd;
    for(int i = 1; i < rhsplits; ++i)
    {
        float si = i / float(rhsplits);
        splits[i].nearplane = lambda*(nd*pow(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
        splits[i-1].farplane = splits[i].nearplane * 1.005f;
    }
    splits[rhsplits-1].farplane = fd;
}

void radiancehints::setup()
{
    updatesplitdist();

    loopi(rhsplits)
    {
        splitinfo &split = splits[i];

        vec c;
        float radius = calcfrustumboundsphere(split.nearplane, split.farplane, camera1->o, camdir, c);

        // compute the projected bounding box of the sphere
        const float pradius = ceil(radius * rhpradiustweak), step = (2*pradius) / rhgrid;
        vec offset = vec(c).sub(pradius).div(step);
        offset.x = floor(offset.x);
        offset.y = floor(offset.y);
        offset.z = floor(offset.z);
        split.cached = split.bounds == pradius ? split.center : vec(-1e16f, -1e16f, -1e16f);
        split.center = vec(offset).mul(step).add(pradius);
        split.bounds = pradius;

        // modify mvp with a scale and offset
        // now compute the update model view matrix for this split
        split.scale = vec(1/(step*(rhgrid+2*rhborder)), 1/(step*(rhgrid+2*rhborder)), 1/(step*(rhgrid+2*rhborder)*rhsplits));
        split.offset = vec(-(offset.x-rhborder)/(rhgrid+2*rhborder), -(offset.y-rhborder)/(rhgrid+2*rhborder), (i - (offset.z-rhborder)/(rhgrid+2*rhborder))/float(rhsplits));
    }
}

void radiancehints::bindparams()
{
    static GlobalShaderParam rhbb("rhbb"), rhscale("rhscale"), rhoffset("rhoffset");
    vec4 *rhbbv = rhbb.reserve<vec4>(rhsplits);
    vec *rhscalev = rhscale.reserve<vec>(rhsplits),
        *rhoffsetv = rhoffset.reserve<vec>(rhsplits);
    loopi(rhsplits)
    {
        splitinfo &split = splits[i];
        rhbbv[i] = vec4(split.center, split.bounds*(1 + rhborder*2*0.5f/rhgrid));
        rhscalev[i] = split.scale;
        rhoffsetv[i] = split.offset;
    }
    float step = 2*splits[0].bounds/rhgrid;
    GLOBALPARAMF(rhnudge, (rhnudge*step));
}

static Shader *deferredlightshader = NULL, *deferredminimapshader = NULL, *deferredmsaapixelshader = NULL, *deferredmsaasampleshader = NULL;

void cleardeferredlightshaders()
{
    deferredlightshader = NULL;
    deferredminimapshader = NULL;
    deferredmsaapixelshader = NULL;
    deferredmsaasampleshader = NULL;
}

Shader *loaddeferredlightshader(const char *type = NULL)
{
    string common, shadow, sun;
    int commonlen = 0, shadowlen = 0, sunlen = 0;

    if(type)
    {
        copystring(common, type);
        commonlen = strlen(common);
    }
    if(usegatherforsm()) common[commonlen++] = smfilter > 2 ? 'G' : 'g';
    else if(smfilter) common[commonlen++] = smfilter > 2 ? 'E' : (smfilter > 1 ? 'F' : 'f');
    common[commonlen] = '\0';

    shadow[shadowlen++] = 'p';
    shadow[shadowlen] = '\0';

    int usecsm = 0, userh = 0;
    if(common[0] != 'm' && ao) sun[sunlen++] = 'a';
    if(sunlight && csmshadowmap)
    {
        usecsm = csmsplits;
        sun[sunlen++] = 'c';
        sun[sunlen++] = '0' + csmsplits;
        if(common[0] != 'm')
        {
            if(ao && aosun) sun[sunlen++] = 'A';
            if(gi && giscale && gidist)
            {
                userh = rhsplits;
                sun[sunlen++] = 'r';
                sun[sunlen++] = '0' + rhsplits;
            }
        }
    }
    sun[sunlen] = '\0';

    defformatstring(name)("deferredlight%s%s%s", common, shadow, sun);
    return generateshader(name, "deferredlightshader \"%s\" \"%s\" \"%s\" %d %d", common, shadow, sun, usecsm, userh);
}

void loaddeferredlightshaders()
{
    if(msaasamples) 
    {
        string opts;
        if(hasMSS) copystring(opts, "MS");
        else formatstring(opts)((msaadepthstencil && hasDS) || msaastencil || !msaaedgedetect ? "MR%d" : "MRT%d", msaasamples);
        deferredmsaasampleshader = loaddeferredlightshader(opts);
        deferredmsaapixelshader = loaddeferredlightshader("M");
        deferredlightshader = deferredmsaapixelshader;
    }
    else deferredlightshader = loaddeferredlightshader();
}

static inline bool sortlights(int x, int y)
{
    const lightinfo &xl = lights[x], &yl = lights[y];
    if(!xl.spot) { if(yl.spot) return true; }
    else if(!yl.spot) return false;
    if(xl.sz1 < yl.sz1) return true;
    else if(xl.sz1 > yl.sz1) return false;
    return xl.dist - xl.radius < yl.dist - yl.radius;
}

VAR(lighttilealignw, 1, 16, 256);
VAR(lighttilealignh, 1, 16, 256);
VARN(lighttilew, lighttilemaxw, 1, 10, LIGHTTILE_MAXW);
VARN(lighttileh, lighttilemaxh, 1, 10, LIGHTTILE_MAXH);

int lighttilew = 0, lighttileh = 0, lighttilevieww = 0, lighttileviewh = 0;

void calctilesize()
{
    lighttilevieww = (vieww + lighttilealignw - 1)/lighttilealignw;
    lighttileviewh = (viewh + lighttilealignh - 1)/lighttilealignh;
    lighttilew = min(lighttilevieww, lighttilemaxw);
    lighttileh = min(lighttileviewh, lighttilemaxh);
}

void resetlights()
{
    shadowcache.reset();
    if(smcache)
    {
        int evictx = ((evictshadowcache%SHADOWCACHE_EVICT)*shadowatlaspacker.w)/SHADOWCACHE_EVICT,
            evicty = ((evictshadowcache/SHADOWCACHE_EVICT)*shadowatlaspacker.h)/SHADOWCACHE_EVICT,
            evictx2 = (((evictshadowcache%SHADOWCACHE_EVICT)+1)*shadowatlaspacker.w)/SHADOWCACHE_EVICT,
            evicty2 = (((evictshadowcache/SHADOWCACHE_EVICT)+1)*shadowatlaspacker.h)/SHADOWCACHE_EVICT;
        loopv(shadowmaps)
        {
            shadowmapinfo &sm = shadowmaps[i];
            if(sm.light < 0) continue;
            lightinfo &l = lights[sm.light];
            if(sm.cached && shadowcachefull)
            {
                int w = l.spot ? sm.size : sm.size*3, h = l.spot ? sm.size : sm.size*2;
                if(sm.x < evictx2 && sm.x + w > evictx && sm.y < evicty2 && sm.y + h > evicty) continue;
            }
            shadowcache[l] = sm;
        }
        if(shadowcachefull)
        {
            evictshadowcache = (evictshadowcache + 1)%(SHADOWCACHE_EVICT*SHADOWCACHE_EVICT);
            shadowcachefull = false;
        }
    }

    lights.setsize(0);
    lightorder.setsize(0);
    loopi(LIGHTTILE_MAXH) loopj(LIGHTTILE_MAXW) lighttiles[i][j].setsize(0);

    shadowmaps.setsize(0);
    shadowatlaspacker.reset();

    calctilesize();
}

static vec *lightsphereverts = NULL;
static GLushort *lightsphereindices = NULL;
static int lightspherenumverts = 0, lightspherenumindices = 0;
static GLuint lightspherevbuf = 0, lightsphereebuf = 0;

static void initlightsphere(int slices, int stacks)
{
    lightspherenumverts = (stacks+1)*(slices+1);
    lightsphereverts = new vec[lightspherenumverts];
    float ds = 1.0f/slices, dt = 1.0f/stacks, t = 1.0f;
    loopi(stacks+1)
    {
        float rho = M_PI*(1-t), s = 0.0f;
        loopj(slices+1)
        {
            float theta = j==slices ? 0 : 2*M_PI*s;
            lightsphereverts[i*(slices+1) + j] = vec(-sin(theta)*sin(rho), -cos(theta)*sin(rho), cos(rho));
            s += ds;
        }
        t -= dt;
    }

    lightspherenumindices = stacks*slices*3*2;
    lightsphereindices = new ushort[lightspherenumindices];
    GLushort *curindex = lightsphereindices;
    loopi(stacks)
    {
        loopk(slices)
        {
            int j = i%2 ? slices-k-1 : k;

            *curindex++ = i*(slices+1)+j;
            *curindex++ = i*(slices+1)+j+1;
            *curindex++ = (i+1)*(slices+1)+j;

            *curindex++ = i*(slices+1)+j+1;
            *curindex++ = (i+1)*(slices+1)+j+1;
            *curindex++ = (i+1)*(slices+1)+j;
        }
    }

    if(!lightspherevbuf) glGenBuffers_(1, &lightspherevbuf);
    glBindBuffer_(GL_ARRAY_BUFFER, lightspherevbuf);
    glBufferData_(GL_ARRAY_BUFFER, lightspherenumverts*sizeof(vec), lightsphereverts, GL_STATIC_DRAW);
    DELETEA(lightsphereverts);

    if(!lightsphereebuf) glGenBuffers_(1, &lightsphereebuf);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, lightsphereebuf);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER, lightspherenumindices*sizeof(GLushort), lightsphereindices, GL_STATIC_DRAW);
    DELETEA(lightsphereindices);
}

void cleanuplightsphere()
{
    if(lightspherevbuf) { glDeleteBuffers_(1, &lightspherevbuf); lightspherevbuf = 0; }
    if(lightsphereebuf) { glDeleteBuffers_(1, &lightsphereebuf); lightsphereebuf = 0; }
}

VAR(depthtestlights, 0, 2, 2);
VAR(depthclamplights, 0, 0, 1);
VAR(depthfaillights, 0, 1, 1);
VAR(lighttilebatch, 0, 8, 8);
VAR(batchsunlight, 0, 1, 1);
FVAR(lightradiustweak, 1, 1.11f, 2);

VAR(lighttilestrip, 0, 1, 1);
VAR(lighttilestripthreshold, 1, 8, 8);

static inline void lightquad(float z = -1)
{
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf( 1, -1, z);
    gle::attribf(-1, -1, z);
    gle::attribf( 1,  1, z);
    gle::attribf(-1,  1, z);
    gle::end();
}

void renderlights(float bsx1 = -1, float bsy1 = -1, float bsx2 = 1, float bsy2 = 1, const uint *tilemask = NULL, int stencilmask = 0, int msaapass = 0)
{
    Shader *s = drawtex == DRAWTEX_MINIMAP ? deferredminimapshader : (msaapass <= 0 ? deferredlightshader : (msaapass > 1 ? deferredmsaasampleshader : deferredmsaapixelshader));
    if(!s || s == nullshader) return;

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);

    bool depth = true;
    if(!depthtestlights) { glDisable(GL_DEPTH_TEST); depth = false; }
    else glDepthMask(GL_FALSE);

    if(msaapass) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mscolortex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gcolortex);
    glActiveTexture_(GL_TEXTURE1);
    if(msaapass) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msnormaltex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gnormaltex);
    glActiveTexture_(GL_TEXTURE2);
    if(msaapass) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msglowtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gglowtex);
    glActiveTexture_(GL_TEXTURE3);
    if(msaapass) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
    glActiveTexture_(GL_TEXTURE4);
    glBindTexture(shadowatlastarget, shadowatlastex);
    if(usesmcomparemode()) setsmcomparemode(); else setsmnoncomparemode();
    if(ao)
    {
        glActiveTexture_(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_RECTANGLE, aotex[2] ? aotex[2] : aotex[0]);
    }
    if(sunlight && csmshadowmap && gi && giscale && gidist) loopi(4)
    {
        glActiveTexture_(GL_TEXTURE6 + i);
        glBindTexture(GL_TEXTURE_3D, rhtex[i]);
    }
    glActiveTexture_(GL_TEXTURE0);

    GLOBALPARAM(worldmatrix, worldmatrix);
    GLOBALPARAM(fogdir, cammatrix.getrow(2));
    GLOBALPARAMF(shadowatlasscale, (1.0f/shadowatlaspacker.w, 1.0f/shadowatlaspacker.h));
    if(ao)
    {
        if(tilemask || (editmode && fullbright) || drawtex)
        {
            GLOBALPARAMF(aoscale, (0.0f, 0.0f));
            GLOBALPARAMF(aoparams, (1.0f, 0.0f, 1.0f, 0.0f));
        }
        else
        {
            GLOBALPARAM(aoscale, aotex[2] ? vec2(1, 1) : vec2(float(aow)/vieww, float(aoh)/viewh));
            GLOBALPARAMF(aoparams, (aomin, 1.0f-aomin, aosunmin, 1.0f-aosunmin));
        }
    }
    float lightscale = 2.0f*ldrscaleb;
    if(editmode && fullbright)
        GLOBALPARAMF(lightscale, (fullbrightlevel*lightscale, fullbrightlevel*lightscale, fullbrightlevel*lightscale, 255*lightscale));
    else
        GLOBALPARAMF(lightscale, (ambientcolor.x*lightscale*ambientscale, ambientcolor.y*lightscale*ambientscale, ambientcolor.z*lightscale*ambientscale, 255*lightscale));

    bool sunpass = !lighttilebatch;
    if(sunlight && csmshadowmap)
    {
        csm.bindparams();
        rh.bindparams();
        if(editmode && fullbright)
        {
            GLOBALPARAMF(sunlightdir, (0, 0, 0));
            GLOBALPARAMF(sunlightcolor, (0, 0, 0));
            GLOBALPARAMF(giscale, (0));
            GLOBALPARAMF(skylightcolor, (0, 0, 0));
        }
        else
        {
            GLOBALPARAM(sunlightdir, sunlightdir);
            GLOBALPARAMF(sunlightcolor, (sunlightcolor.x*lightscale*sunlightscale, sunlightcolor.y*lightscale*sunlightscale, sunlightcolor.z*lightscale*sunlightscale));
            GLOBALPARAMF(giscale, (2*giscale));
            GLOBALPARAMF(skylightcolor, (2*giaoscale*skylightcolor.x*lightscale*skylightscale, 2*giaoscale*skylightcolor.y*lightscale*skylightscale, 2*giaoscale*skylightcolor.z*lightscale*skylightscale));
        }
        if(!batchsunlight) sunpass = true;
    }

    gle::defvertex(3);

    int btx1, bty1, btx2, bty2;
    calctilebounds(bsx1, bsy1, bsx2, bsy2, btx1, bty1, btx2, bty2);
    if(msaapass == 1)
    {
        int tx1 = max(int(floor((bsx1*0.5f+0.5f)*vieww)), 0), ty1 = max(int(floor((bsy1*0.5f+0.5f)*viewh)), 0),
            tx2 = min(int(ceil((bsx2*0.5f+0.5f)*vieww)), vieww), ty2 = min(int(ceil((bsy2*0.5f+0.5f)*viewh)), viewh);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        if(stencilmask) glStencilFunc(GL_EQUAL, stencilmask|0x08, stencilmask);
        else glStencilFunc(GL_ALWAYS, 0x08, ~0);
        if(depthtestlights && depth) { glDisable(GL_DEPTH_TEST); depth = false; }
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glScissor(tx1, ty1, tx2-tx1, ty2-ty1);
        SETSHADER(msaaedgedetect);
        lightquad();
        glStencilFunc(GL_EQUAL, stencilmask, ~0);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    else if(msaapass == 2)
    {
        glStencilFunc(GL_EQUAL, stencilmask|0x08, ~0);
    }

    glmatrix lightmatrix;
    lightmatrix.identity();
    GLOBALPARAM(lightmatrix, lightmatrix);

    if(sunpass)
    {
        int tx1 = max(int(floor((bsx1*0.5f+0.5f)*vieww)), 0), ty1 = max(int(floor((bsy1*0.5f+0.5f)*viewh)), 0),
            tx2 = min(int(ceil((bsx2*0.5f+0.5f)*vieww)), vieww), ty2 = min(int(ceil((bsy2*0.5f+0.5f)*viewh)), viewh);
        s->set();
        glScissor(tx1, ty1, tx2-tx1, ty2-ty1);
        if(depthtestlights && depth) { glDisable(GL_DEPTH_TEST); depth = false; }
        lightquad();
        lightpassesused++;
    }

    if(depthtestlights && !depth) { glEnable(GL_DEPTH_TEST); depth = true; }
    if(hasDBT && depthtestlights > 1) glEnable(GL_DEPTH_BOUNDS_TEST_EXT);

    static LocalShaderParam lightpos("lightpos"), lightcolor("lightcolor"), spotparams("spotparams"), spotx("spotx"), spoty("spoty"), shadowparams("shadowparams"), shadowoffset("shadowoffset");
    static vec4 lightposv[8], spotparamsv[8], shadowparamsv[8];
    static vec lightcolorv[8], spotxv[8], spotyv[8];
    static vec2 shadowoffsetv[8];

    if(!lighttilebatch)
    {
        gle::disable();
        if(!lightspherevbuf) initlightsphere(10, 5);
        glBindBuffer_(GL_ARRAY_BUFFER, lightspherevbuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, lightsphereebuf);
        gle::vertexpointer(sizeof(vec), lightsphereverts);
        gle::enablevertex();

        if(hasDC && depthclamplights) glEnable(GL_DEPTH_CLAMP_NV);

        bool outside = true;
        loopv(lightorder)
        {
            lightinfo &l = lights[lightorder[i]];
            float sx1 = max(bsx1, l.sx1), sy1 = max(bsy1, l.sy1),
                  sx2 = min(bsx2, l.sx2), sy2 = min(bsy2, l.sy2);
            if(sx1 >= sx2 || sy1 >= sy2 || l.sz1 >= l.sz2) continue;

            bool shadowmap = l.shadowmap >= 0, spotlight = l.spot > 0;

            lightposv[0] = vec4(l.o.x, l.o.y, l.o.z, 1.0f/l.radius);
            lightcolorv[0] = vec(l.color.x*lightscale, l.color.y*lightscale, l.color.z*lightscale);
            if(spotlight)
            {
                float maxatten = sincos360[l.spot].x;
                spotparamsv[0] = vec4(l.dir, maxatten).div(1 - maxatten);
            }
            if(shadowmap)
            {
                shadowmapinfo &sm = shadowmaps[l.shadowmap];
                float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3,
                      bias = (smfilter > 2 ? smbias2 : smbias) * (smcullside ? 1 : -1) * smnearclip * (1024.0f / sm.size);
                int border = smfilter > 2 ? smborder2 : smborder;
                if(spotlight)
                {
                    spotxv[0] = l.spotx;
                    spotyv[0] = l.spoty;
                    float maxatten = sincos360[l.spot].x;
                    shadowparamsv[0] = vec4(
                        0.5f * sm.size / (1 - maxatten),
                        (-smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias) / (1 - maxatten),
                        sm.size,
                        0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                }
                else
                {
                    shadowparamsv[0] = vec4(
                        0.5f * (sm.size - border),
                        -smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias,
                        sm.size,
                        0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                }
                shadowoffsetv[0] = vec2(sm.x + 0.5f*sm.size, sm.y + 0.5f*sm.size);
            }

            lightmatrix = camprojmatrix;
            lightmatrix.translate(l.o);
            lightmatrix.scale(l.radius*lightradiustweak);
            GLOBALPARAM(lightmatrix, lightmatrix);

            s->setvariant(0, (shadowmap ? 1 : 0) + 2 + (spotlight ? 4 : 0));
            lightpos.setv(lightposv, 1);
            lightcolor.setv(lightcolorv, 1);
            if(spotlight) spotparams.setv(spotparamsv, 1);
            if(shadowmap)
            {
                if(spotlight)
                {
                    spotx.setv(spotxv, 1);
                    spoty.setv(spotyv, 1);
                }
                shadowparams.setv(shadowparamsv, 1);
                shadowoffset.setv(shadowoffsetv, 1);
            }

            int tx1 = int(floor((sx1*0.5f+0.5f)*vieww)), ty1 = int(floor((sy1*0.5f+0.5f)*viewh)),
                tx2 = int(ceil((sx2*0.5f+0.5f)*vieww)), ty2 = int(ceil((sy2*0.5f+0.5f)*viewh));
            glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

            if(hasDBT && depthtestlights > 1) glDepthBounds_(l.sz1*0.5f + 0.5f, l.sz2*0.5f + 0.5f);

            if(camera1->o.dist(l.o) <= l.radius*lightradiustweak + nearplane + 1 && depthfaillights)
            {
                if(outside)
                {
                    outside = false;
                    glDepthFunc(GL_GEQUAL);
                    glCullFace(GL_FRONT);
                }
            }
            else if(!outside)
            {
                outside = true;
                glDepthFunc(GL_LESS);
                glCullFace(GL_BACK);
            }

            glDrawRangeElements_(GL_TRIANGLES, 0, lightspherenumverts-1, lightspherenumindices, GL_UNSIGNED_SHORT, lightsphereindices);
            xtraverts += lightspherenumindices;
            glde++;

            lightpassesused++;
        }

        if(!outside)
        {
            outside = true;
            glDepthFunc(GL_LESS);
            glCullFace(GL_BACK);
        }

        if(hasDC && depthclamplights) glDisable(GL_DEPTH_CLAMP_NV);

        gle::disablevertex();
        glBindBuffer_(GL_ARRAY_BUFFER, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else for(int y = bty1; y < bty2; y++) if(!tilemask || tilemask[y])
    {
        int skips[LIGHTTILE_MAXW] = {0};
        for(int x = btx1; x < btx2; x++) if(!tilemask || tilemask[y]&(1<<x))
        {
            vector<int> &tile = lighttiles[y][x];
            if(tile.empty() ? sunpass || skips[x] : skips[x] >= tile.length()) continue;
            for(int i = skips[x];;)
            {
                int n = min(tile.length() - i, lighttilebatch);
                bool shadowmap = false, spotlight = false;
                if(n > 0)
                {
                    lightinfo &l = lights[tile[i]];
                    shadowmap = l.shadowmap >= 0;
                    spotlight = l.spot > 0;
                }
                loopj(n)
                {
                    lightinfo &l = lights[tile[i+j]];
                    if((l.shadowmap >= 0) != shadowmap || (l.spot > 0) != spotlight) { n = j; break; }
                }

                int striplength = 1;
                if(lighttilestrip && n >= min(min(lighttilebatch, lighttilestripthreshold), tile.length() - i))
                {
                    for(int k = x+1; k < btx2; k++)
                    {
                        vector<int> &striptile = lighttiles[y][k];
                        if((tilemask && !(tilemask[y]&(1<<k))) ||
                           skips[k] != i ||
                           striptile.length() < i+n ||
                           memcmp(&tile[i], &striptile[i], n*sizeof(int)) ||
                           n < min(min(lighttilebatch, lighttilestripthreshold), striptile.length()-i))
                            break;
                        skips[k] = max(i+n, 1);
                        striplength++;
                    }
                }

                float sx1 = 1, sy1 = 1, sx2 = -1, sy2 = -1, sz1 = 1, sz2 = -1;
                loopj(n)
                {
                    lightinfo &l = lights[tile[i+j]];
                    lightposv[j] = vec4(l.o.x, l.o.y, l.o.z, 1.0f/l.radius);
                    lightcolorv[j] = vec(l.color.x*lightscale, l.color.y*lightscale, l.color.z*lightscale);
                    if(spotlight)
                    {
                        float maxatten = sincos360[l.spot].x;
                        spotparamsv[j] = vec4(l.dir, maxatten).div(1 - maxatten);
                    }
                    if(shadowmap)
                    {
                        shadowmapinfo &sm = shadowmaps[l.shadowmap];
                        float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3,
                              bias = (smfilter > 2 ? smbias2 : smbias) * (smcullside ? 1 : -1) * smnearclip * (1024.0f / sm.size);
                        int border = smfilter > 2 ? smborder2 : smborder;
                        if(spotlight)
                        {
                            spotxv[j] = l.spotx;
                            spotyv[j] = l.spoty;
                            float maxatten = sincos360[l.spot].x;
                            shadowparamsv[j] = vec4(
                                0.5f * sm.size / (1 - maxatten),
                                (-smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias) / (1 - maxatten),
                                sm.size,
                                0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                        }
                        else
                        {
                            shadowparamsv[j] = vec4(
                                0.5f * (sm.size - border),
                                -smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias,
                                sm.size,
                                0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                        }
                        shadowoffsetv[j] = vec2(sm.x + 0.5f*sm.size, sm.y + 0.5f*sm.size);
                    }
                    sx1 = min(sx1, l.sx1);
                    sy1 = min(sy1, l.sy1);
                    sx2 = max(sx2, l.sx2);
                    sy2 = max(sy2, l.sy2);
                    sz1 = min(sz1, l.sz1);
                    sz2 = max(sz2, l.sz2);
                }
                if(!i && !sunpass) { sx1 = sy1 = sz1 = -1; sx2 = sy2 = sz2 = 1; }
                else if(sx1 >= sx2 || sy1 >= sy2 || sz1 >= sz2)
                {
                    i += n;
                    if(i >= tile.length()) break;
                    continue;
                }

                if(n)
                {
                    s->setvariant(n-1, (shadowmap ? 1 : 0) + (i || sunpass ? 2 : 0) + (spotlight ? 4 : 0));
                    lightpos.setv(lightposv, n);
                    lightcolor.setv(lightcolorv, n);
                    if(spotlight) spotparams.setv(spotparamsv, n);
                    if(shadowmap)
                    {
                        if(spotlight)
                        {
                            spotx.setv(spotxv, n);
                            spoty.setv(spotyv, n);
                        }
                        shadowparams.setv(shadowparamsv, n);
                        shadowoffset.setv(shadowoffsetv, n);
                    }
                }
                else s->set();

                sx1 = max(sx1, bsx1);
                sy1 = max(sy1, bsy1);
                sx2 = min(sx2, bsx2);
                sy2 = min(sy2, bsy2);
                if(sx1 < sx2 && sy1 < sy2)
                {
                    int tx1 = max(int(floor((sx1*0.5f+0.5f)*vieww)), ((x*lighttilevieww)/lighttilew)*lighttilealignw),
                        ty1 = max(int(floor((sy1*0.5f+0.5f)*viewh)), ((y*lighttileviewh)/lighttileh)*lighttilealignh),
                        tx2 = min(int(ceil((sx2*0.5f+0.5f)*vieww)), min((((x+striplength)*lighttilevieww)/lighttilew)*lighttilealignw, vieww)),
                        ty2 = min(int(ceil((sy2*0.5f+0.5f)*viewh)), min((((y+1)*lighttileviewh)/lighttileh)*lighttilealignh, viewh));
                    glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

                    if(hasDBT && depthtestlights > 1) glDepthBounds_(sz1*0.5f + 0.5f, sz2*0.5f + 0.5f);

                    // FIXME: render light geometry here
                    lightquad(sz1);

                    lightpassesused++;
                }

                i += n;
                if(i >= tile.length()) break;
            }
        }
    }

    gle::disable();

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    if(!depthtestlights) glEnable(GL_DEPTH_TEST);
    else
    {
        glDepthMask(GL_TRUE);
        if(hasDBT && depthtestlights > 1) glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
    }
}

VAR(oqlights, 0, 1, 1);
VAR(debuglightscissor, 0, 0, 1);

void viewlightscissor()
{
    vector<extentity *> &ents = entities::getents();
    gle::defvertex(2);
    loopv(entgroup)
    {
        int idx = entgroup[i];
        if(ents.inrange(idx) && ents[idx]->type == ET_LIGHT)
        {
            extentity &e = *ents[idx];
            loopvj(lights) if(lights[j].o == e.o)
            {
                lightinfo &l = lights[j];
                if(l.sx1 >= l.sx2 || l.sy1 >= l.sy2 || l.sz1 >= l.sz2) break;
                gle::colorf(l.color.x/255, l.color.y/255, l.color.z/255);
                float x1 = (l.sx1+1)/2*screenw, x2 = (l.sx2+1)/2*screenw,
                      y1 = (1-l.sy1)/2*screenh, y2 = (1-l.sy2)/2*screenh;
                gle::begin(GL_TRIANGLE_STRIP);
                gle::attribf(x1, y1);
                gle::attribf(x2, y1);
                gle::attribf(x1, y2);
                gle::attribf(x2, y2);
                gle::end();
            }
        }
    }
    gle::disable();
}

static inline bool calclightscissor(lightinfo &l)
{
    float sx1 = -1, sy1 = -1, sx2 = 1, sy2 = 1, sz1 = -1, sz2 = 1;
    if(l.radius > 0)
    {
        if(l.spot > 0) calcspotscissor(l.o, l.radius, l.dir, l.spot, l.spotx, l.spoty, sx1, sy1, sx2, sy2, sz1, sz2);
        else calcspherescissor(l.o, l.radius, sx1, sy1, sx2, sy2, sz1, sz2);
    }
    l.sx1 = sx1;
    l.sx2 = sx2;
    l.sy1 = sy1;
    l.sy2 = sy2;
    l.sz1 = sz1;
    l.sz2 = sz2;
    return sx1 < sx2 && sy1 < sy2 && sz1 < sz2;
}

void collectlights()
{
    // point lights processed here
    const vector<extentity *> &ents = entities::getents();
    if(!editmode || !fullbright) loopv(ents)
    {
        extentity *e = ents[i];
        if(e->type != ET_LIGHT) continue;

        if(e->attr1 > 0 && smviscull)
        {
            if(isfoggedsphere(e->attr1, e->o)) continue;
            if(pvsoccludedsphere(e->o, e->attr1)) continue;
        }

        lightinfo &l = lights.add();
        l.ent = i;
        l.shadowmap = -1;
        l.flags = e->attr5;
        l.query = NULL;
        l.o = e->o;
        l.color = vec(e->attr2, e->attr3, e->attr4);
        l.radius = e->attr1 > 0 ? e->attr1 : 2*worldsize;
        if(e->attached && e->attached->type == ET_SPOTLIGHT)
        {
            l.calcspot(vec(e->attached->o).sub(e->o).normalize(), clamp(int(e->attached->attr1), 1, 89));
        }
        else
        {
            l.dir = vec(0, 0, 0);
            l.spot = 0;
        }
        l.dist = camera1->o.dist(e->o);

        if(calclightscissor(l)) lightorder.add(lights.length()-1);
    }

    updatedynlights();
    int numdynlights = finddynlights();
    loopi(numdynlights)
    {
        vec o, color, dir;
        float radius;
        int spot;
        if(!getdynlight(i, o, radius, color, dir, spot)) continue;

        lightinfo &l = lights.add();
        l.ent = -1;
        l.shadowmap = -1;
        l.flags = 0;
        l.query = NULL;
        l.o = o;
        l.color = vec(color).mul(255);
        l.radius = radius;
        if(spot > 0)
        {
            l.calcspot(dir, spot);
        }
        else
        {
            l.dir = vec(0, 0, 0);
            l.spot = 0;
        }
        l.dist = camera1->o.dist(o);

        if(calclightscissor(l)) lightorder.add(lights.length()-1);
    }

    lightorder.sort(sortlights);

    bool queried = false;
    if(!drawtex && smquery && oqfrags && oqlights) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.flags&L_NOSHADOW || l.radius <= smminradius || l.radius >= worldsize) continue;
        vec bbmin, bbmax;
        if(l.spot > 0)
        {
            const vec2 &sc = sincos360[l.spot];
            float spotscale = l.radius*sc.y/sc.x;
            vec up = vec(l.spotx).rescale(spotscale), right = vec(l.spoty).rescale(spotscale), center = vec(l.dir).mul(l.radius).add(l.o);
            up.x = fabs(up.x); up.y = fabs(up.y); up.z = fabs(up.z);
            right.x = fabs(right.x); right.y = fabs(right.y); right.z = fabs(right.z);
            bbmin = bbmax = center;
            bbmin.sub(up).sub(right);
            bbmax.add(up).add(right);
            bbmin.min(l.o);
            bbmax.max(l.o);
        }
        else
        {
            bbmin = vec(l.o).sub(l.radius);
            bbmax = vec(l.o).add(l.radius);
        }
        if(camera1->o.x < bbmin.x - 2 || camera1->o.x > bbmax.x + 2 ||
           camera1->o.y < bbmin.y - 2 || camera1->o.y > bbmax.y + 2 ||
           camera1->o.z < bbmin.z - 2 || camera1->o.z > bbmax.z + 2)
        {
            l.query = newquery(&l);
            if(l.query)
            {
                if(!queried)
                {
                    gle::defvertex();
                    queried = true;
                }
                startquery(l.query);
                ivec bo = bbmin, br = bbmax;
                br.sub(bo).add(1);
                drawbb(bo, br);
                endquery(l.query);
            }
        }
    }
    if(queried) gle::disable();
}

static inline void addlighttiles(const lightinfo &l, int idx)
{
    int tx1, ty1, tx2, ty2;
    calctilebounds(l.sx1, l.sy1, l.sx2, l.sy2, tx1, ty1, tx2, ty2);
    for(int y = ty1; y < ty2; y++) for(int x = tx1; x < tx2; x++) { lighttiles[y][x].add(idx); lighttilesused++; }
}

VAR(lightsvisible, 1, 0, 0);
VAR(lightsoccluded, 1, 0, 0);

void packlights()
{
    lightsvisible = lightsoccluded = 0;
    lighttilesused = lightpassesused = 0;
    smused = 0;

    if(smcache && !smnoshadow && shadowcache.numelems) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.flags&L_NOSHADOW || l.radius <= smminradius) continue;
        if(l.query && l.query->owner == &l && checkquery(l.query)) continue;

        float prec = smprec, lod;
        int w, h;
        if(l.spot) { w = 1; h = 1; const vec2 &sc = sincos360[l.spot]; prec = sc.y/sc.x; lod = smspotprec; }
        else { w = 3; h = 2; lod = smcubeprec; }
        lod *= clamp(l.radius * prec / sqrtf(max(1.0f, l.dist/l.radius)), float(smminsize), float(smmaxsize));
        int size = clamp(int(ceil((lod * shadowatlaspacker.w) / SHADOWATLAS_SIZE)), 1, shadowatlaspacker.w / w);
        w *= size;
        h *= size;
        ushort x = USHRT_MAX, y = USHRT_MAX;
        shadowmapinfo *sm = NULL;
        int smidx = -1;
        shadowcacheval *cached = shadowcache.access(l);
        if(!cached || cached->size != size) continue;
        x = cached->x;
        y = cached->y;
        shadowatlaspacker.reserve(x, y, w, h);
        sm = addshadowmap(x, y, size, smidx);
        sm->light = idx;
        sm->cached = cached;
        l.shadowmap = smidx;
        smused += w*h;

        addlighttiles(l, idx);
    }
    loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.shadowmap >= 0) continue;

        if(!(l.flags&L_NOSHADOW) && !smnoshadow && l.radius > smminradius)
        {
            if(l.query && l.query->owner == &l && checkquery(l.query)) { lightsoccluded++; continue; }
            float prec = smprec, lod;
            int w, h;
            if(l.spot) { w = 1; h = 1; const vec2 &sc = sincos360[l.spot]; prec = sc.y/sc.x; lod = smspotprec; }
            else { w = 3; h = 2; lod = smcubeprec; }
            lod *= clamp(l.radius * prec / sqrtf(max(1.0f, l.dist/l.radius)), float(smminsize), float(smmaxsize));
            int size = clamp(int(ceil((lod * shadowatlaspacker.w) / SHADOWATLAS_SIZE)), 1, shadowatlaspacker.w / w);
            w *= size;
            h *= size;
            ushort x = USHRT_MAX, y = USHRT_MAX;
            shadowmapinfo *sm = NULL;
            int smidx = -1;
            if(shadowatlaspacker.insert(x, y, w, h))
            {
                sm = addshadowmap(x, y, size, smidx);
                sm->light = idx;
                l.shadowmap = smidx;
                smused += w*h;
            }
            else if(smcache) shadowcachefull = true;
        }

        addlighttiles(l, idx);
    }

    lightsvisible = lightorder.length() - lightsoccluded;
}

static inline void rhquad(float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2, float tz)
{
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(x2, y1); gle::attribf(tx2, ty1, tz);
    gle::attribf(x1, y1); gle::attribf(tx1, ty1, tz);
    gle::attribf(x2, y2); gle::attribf(tx2, ty2, tz);
    gle::attribf(x1, y2); gle::attribf(tx1, ty2, tz);
    gle::end();
}

static inline void rhquad(float dx1, float dy1, float dx2, float dy2, float dtx1, float dty1, float dtx2, float dty2, float dtz,
                          float px1, float py1, float px2, float py2, float ptx1, float pty1, float ptx2, float pty2, float ptz)
{
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(dx2, dy1); gle::attribf(dtx2, dty1, dtz);
        gle::attribf(px2, py1); gle::attribf(ptx2, pty1, ptz);
    gle::attribf(dx1, dy1); gle::attribf(dtx1, dty1, dtz);
        gle::attribf(px1, py1); gle::attribf(ptx1, pty1, ptz);
    gle::attribf(dx1, dy2); gle::attribf(dtx1, dty2, dtz);
        gle::attribf(px1, py2); gle::attribf(ptx1, pty2, ptz);
    gle::attribf(dx2, dy2); gle::attribf(dtx2, dty2, dtz);
        gle::attribf(px2, py2); gle::attribf(ptx2, pty2, ptz);
    gle::attribf(dx2, dy1); gle::attribf(dtx2, dty1, dtz);
        gle::attribf(px2, py1); gle::attribf(ptx2, pty1, ptz);
    gle::end();
}

void radiancehints::renderslices()
{
    if(rhcache) loopi(4) swap(rhtex[i], rhtex[i+4]);

    glBindFramebuffer_(GL_FRAMEBUFFER, rhfbo);
    glViewport(0, 0, rhgrid+2*rhborder, rhgrid+2*rhborder);

    GLOBALPARAMF(rhatten, (1.0f/(gidist*gidist)));
    GLOBALPARAMF(rsmspread, (gidist*rsmspread*rsm.scale.x, gidist*rsmspread*rsm.scale.y));
    GLOBALPARAMF(rhaothreshold, (splits[0].bounds/rhgrid));
    GLOBALPARAMF(rhaoatten, (1.0f/(gidist*rsmspread)));
    GLOBALPARAMF(rhaoheight, (gidist*rsmspread));

    glmatrix rsmtcmatrix;
    rsmtcmatrix.identity();
    rsmtcmatrix.settranslation(rsm.offset);
    rsmtcmatrix.setscale(rsm.scale);
    rsmtcmatrix.mul(rsm.model);
    GLOBALPARAM(rsmtcmatrix, rsmtcmatrix);

    glmatrix rsmworldmatrix;
    rsmworldmatrix.invert(rsmtcmatrix);
    GLOBALPARAM(rsmworldmatrix, rsmworldmatrix);

    glBindTexture(GL_TEXTURE_RECTANGLE, rsmdepthtex);
    glActiveTexture_(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE, rsmcolortex);
    glActiveTexture_(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_RECTANGLE, rsmnormaltex);
    loopi(rhcache ? 8 : 4)
    {
        glActiveTexture_(GL_TEXTURE3 + i);
        glBindTexture(GL_TEXTURE_3D, rhtex[i]);
    }
    glActiveTexture_(GL_TEXTURE0);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    gle::defvertex(2);
    gle::deftexcoord0(3);

    loopirev(rhsplits)
    {
        splitinfo &split = splits[i];

        float cellradius = split.bounds/rhgrid, step = 2*cellradius;
        GLOBALPARAMF(rhspread, (cellradius));

        vec cmin, cmax, bmin(1e16f, 1e16f, 1e16f), bmax(-1e16f, -1e16f, -1e16f), dmin(1e16f, 1e16f, 1e16f), dmax(-1e16f, -1e16f, -1e16f);
        loopk(3)
        {
            cmin[k] = floor((worldmin[k] - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
            cmax[k] = ceil((worldmax[k] - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
        }
        if(prevdynmin.z < prevdynmax.z) loopk(3)
        {
            dmin[k] = min(dmin[k], (float)floor((prevdynmin[k] - gidist - cellradius - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds);
            dmax[k] = max(dmax[k], (float)ceil((prevdynmax[k] + gidist + cellradius - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds);
        }
        if(dynmin.z < dynmax.z) loopk(3)
        {
            dmin[k] = min(dmin[k], (float)floor((dynmin[k] - gidist - cellradius - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds);
            dmax[k] = max(dmax[k], (float)ceil((dynmax[k] + gidist + cellradius - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds);
        }

        if(rhborder && i + 1 < rhsplits)
        {
            GLOBALPARAMF(bordercenter, (0.5f, 0.5f, float(i+1 + 0.5f)/rhsplits));
            GLOBALPARAMF(borderrange, (0.5f - 0.5f/(rhgrid+2), 0.5f - 0.5f/(rhgrid+2), (0.5f - 0.5f/(rhgrid+2))/rhsplits));
            GLOBALPARAMF(borderscale, (rhgrid+2, rhgrid+2, (rhgrid+2)*rhsplits));
            splitinfo &next = splits[i+1];
            loopk(3)
            {
                bmin[k] = floor((max(float(worldmin[k]), next.center[k] - next.bounds) - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
                bmax[k] = ceil((min(float(worldmax[k]), next.center[k] + next.bounds) - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
            }
        }

        loopjrev(rhgrid+2*rhborder)
        {
            glFramebufferTexture3D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, rhtex[0], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_3D, rhtex[1], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_3D, rhtex[2], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_3D, rhtex[3], 0, i*(rhgrid+2*rhborder) + j);

            float x1 = split.center.x - split.bounds, x2 = split.center.x + split.bounds,
                  y1 = split.center.y - split.bounds, y2 = split.center.y + split.bounds,
                  z = split.center.z - split.bounds + (j-rhborder+0.5f)*step,
                  vx1 = -1 + rhborder*2.0f/(rhgrid+2), vx2 = 1 - rhborder*2.0f/(rhgrid+2), vy1 = -1 + rhborder*2.0f/(rhgrid+2), vy2 = 1 - rhborder*2.0f/(rhgrid+2),
                  tx1 = x1, tx2 = x2, ty1 = y1, ty2 = y2;

            if(rhborder && i + 1 < rhsplits)
            {
                splitinfo &next = splits[i+1];
                float bx1 = x1-step, bx2 = x2+step, by1 = y1-step, by2 = y2+step, bz = z,
                      bvx1 = -1, bvx2 = 1, bvy1 = -1, bvy2 = 1,
                      btx1 = bx1, btx2 = bx2, bty1 = by1, bty2 = by2;

                if(rhclipgrid)
                {
                    if(bz < bmin.z || bz > bmax.z) goto noborder;
                    if(bx1 < bmin.x || bx2 > bmax.x || by1 < bmin.y || by2 > bmax.y)
                    {
                        btx1 = max(bx1, bmin.x);
                        btx2 = min(bx2, bmax.x);
                        bty1 = max(by1, bmin.y);
                        bty2 = min(by2, bmax.y);
                        if(btx1 > tx2 || bty1 > bty2) goto noborder;
                        bvx1 += 2*(btx1 - bx1)/(bx2 - bx1);
                        bvx2 += 2*(btx2 - bx2)/(bx2 - bx1);
                        bvy1 += 2*(bty1 - by1)/(by2 - by1);
                        bvy2 += 2*(bty2 - by2)/(by2 - by1);

                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                }

                btx1 = btx1*next.scale.x + next.offset.x;
                btx2 = btx2*next.scale.x + next.offset.x;
                bty1 = bty1*next.scale.y + next.offset.y;
                bty2 = bty2*next.scale.y + next.offset.y;
                bz = bz*next.scale.z + next.offset.z;

                SETSHADER(radiancehintsborder);
                rhquad(bvx1, bvy1, bvx2, bvy2, btx1, bty1, btx2, bty2, bz);
            }
            else
            {
            noborder:
                if(rhborder) glClear(GL_COLOR_BUFFER_BIT);
            }

            if(j < rhborder || j >= rhgrid + rhborder) continue;

            if(rhclipgrid)
            {
                if(z < cmin.z || z > cmax.z)
                {
                    if(!rhborder) glClear(GL_COLOR_BUFFER_BIT);
                    continue;
                }
                if(x1 < cmin.x || x2 > cmax.x || y1 < cmin.y || y2 > cmax.y)
                {
                    if(!rhborder) glClear(GL_COLOR_BUFFER_BIT);
                    tx1 = max(x1, cmin.x);
                    tx2 = min(x2, cmax.x);
                    ty1 = max(y1, cmin.y);
                    ty2 = min(y2, cmax.y);
                    if(tx1 > tx2 || ty1 > ty2) continue;
                    vx1 += 2*rhgrid/float(rhgrid+2*rhborder)*(tx1 - x1)/(x2 - x1);
                    vx2 += 2*rhgrid/float(rhgrid+2*rhborder)*(tx2 - x2)/(x2 - x1);
                    vy1 += 2*rhgrid/float(rhgrid+2*rhborder)*(ty1 - y1)/(y2 - y1);
                    vy2 += 2*rhgrid/float(rhgrid+2*rhborder)*(ty2 - y2)/(y2 - y1);
                }
            }

            if(rhcache && z > split.cached.z - split.bounds && z < split.cached.z + split.bounds)
            {
                float px1 = max(tx1, split.cached.x - split.bounds), px2 = min(tx2, split.cached.x + split.bounds),
                      py1 = max(ty1, split.cached.y - split.bounds), py2 = min(ty2, split.cached.y + split.bounds);
                if(px1 < px2 && py1 < py2)
                {
                    float pvx1 = -1 + rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(px1 - x1)/(x2 - x1),
                          pvx2 = 1 - rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(px2 - x2)/(x2 - x1),
                          pvy1 = -1 + rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(py1 - y1)/(y2 - y1),
                          pvy2 = 1 - rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(py2 - y2)/(y2 - y1),
                          ptx1 = (px1 + split.center.x - split.cached.x)*split.scale.x + split.offset.x,
                          ptx2 = (px2 + split.center.x - split.cached.x)*split.scale.x + split.offset.x,
                          pty1 = (py1 + split.center.y - split.cached.y)*split.scale.y + split.offset.y,
                          pty2 = (py2 + split.center.y - split.cached.y)*split.scale.y + split.offset.y,
                          pz = (z + split.center.z - split.cached.z)*split.scale.z + split.offset.z;

                    if(px1 != tx1 || px2 != tx2 || py1 != ty1 || py2 != ty2)
                    {
                        radiancehintsshader->set();
                        rhquad(pvx1, pvy1, pvx2, pvy2, px1, py1, px2, py2, z,
                                vx1,  vy1,  vx2,  vy2, tx1, ty1, tx2, ty2, z);
                    }

                    if(z > dmin.z && z < dmax.z)
                    {
                        float dx1 = max(px1, dmin.x), dx2 = min(px2, dmax.x),
                              dy1 = max(py1, dmin.y), dy2 = min(py2, dmax.y);
                        if(dx1 < dx2 && dy1 < dy2)
                        {
                            float dvx1 = -1 + rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(dx1 - x1)/(x2 - x1),
                                  dvx2 = 1 - rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(dx2 - x2)/(x2 - x1),
                                  dvy1 = -1 + rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(dy1 - y1)/(y2 - y1),
                                  dvy2 = 1 - rhborder*2.0f/(rhgrid+2) + 2*rhgrid/float(rhgrid+2*rhborder)*(dy2 - y2)/(y2 - y1),
                                  dtx1 = (dx1 + split.center.x - split.cached.x)*split.scale.x + split.offset.x,
                                  dtx2 = (dx2 + split.center.x - split.cached.x)*split.scale.x + split.offset.x,
                                  dty1 = (dy1 + split.center.y - split.cached.y)*split.scale.y + split.offset.y,
                                  dty2 = (dy2 + split.center.y - split.cached.y)*split.scale.y + split.offset.y,
                                  dz = (z + split.center.z - split.cached.z)*split.scale.z + split.offset.z;

                            if(dx1 != px1 || dx2 != px2 || dy1 != py1 || dy2 != py2)
                            {
                                SETSHADER(radiancehintscached);
                                rhquad(dvx1, dvy1, dvx2, dvy2, dtx1, dty1, dtx2, dty2, dz,
                                       pvx1, pvy1, pvx2, pvy2, ptx1, pty1, ptx2, pty2, pz);
                            }

                            radiancehintsshader->set();
                            rhquad(dvx1, dvy1, dvx2, dvy2, dx1, dy1, dx2, dy2, z);
                            continue;
                        }
                    }

                    SETSHADER(radiancehintscached);
                    rhquad(pvx1, pvy1, pvx2, pvy2, ptx1, pty1, ptx2, pty2, pz);
                    continue;
                }
            }

            radiancehintsshader->set();
            rhquad(vx1, vy1, vx2, vy2, tx1, ty1, tx2, ty2, z);
        }
    }

    gle::disable();
}

void renderradiancehints()
{
    if(!sunlight || !csmshadowmap || !gi || !giscale || !gidist) return;

    timer *rhtimer = begintimer("radiance hints");

    rh.setup();
    rsm.setup();

    shadowmapping = SM_REFLECT;
    shadowside = 0;
    shadoworigin = vec(0, 0, 0);
    shadowdir = rsm.lightview;
    shadowbias = rsm.lightview.project_bb(worldmin, worldmax);
    shadowradius = fabs(rsm.lightview.project_bb(worldmax, worldmin));

    findshadowvas();
    findshadowmms();

    shadowmaskbatchedmodels(false);
    batchshadowmapmodels();

    rh.prevdynmin = rh.dynmin;
    rh.prevdynmax = rh.dynmax;
    rh.dynmin = vec(1e16f, 1e16f, 1e16f);
    rh.dynmax = vec(-1e16f, -1e16f, -1e16f);
    if(rhdyntex) dynamicshadowvabounds(1<<shadowside, rh.dynmin, rh.dynmax);
    if(rhdynmm) batcheddynamicmodelbounds(1<<shadowside, rh.dynmin, rh.dynmax);

    if(!rhcache || rh.prevdynmin.z < rh.prevdynmax.z || rh.dynmin.z < rh.dynmax.z || !rh.allcached())
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, rsmfbo);

        shadowmatrix.mul(rsm.proj, rsm.model);
        GLOBALPARAM(rsmmatrix, shadowmatrix);
        GLOBALPARAMF(rsmdir, (-rsm.lightview.x, -rsm.lightview.y, -rsm.lightview.z));

        glViewport(0, 0, rsmsize, rsmsize);
        glClearColor(0, 0, 0, 0);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

        renderrsmgeom(rhdyntex!=0);
        rendershadowmodelbatches(rhdynmm!=0);

        rh.renderslices();
    }

    clearbatchedmapmodels();

    shadowmapping = 0;

    endtimer(rhtimer);
}

void rendercsmshadowmaps()
{
    shadowmapping = SM_CASCADE;
    shadoworigin = vec(0, 0, 0);
    shadowdir = csm.lightview;
    shadowbias = csm.lightview.project_bb(worldmin, worldmax);
    shadowradius = fabs(csm.lightview.project_bb(worldmax, worldmin));

    float polyfactor = csmpolyfactor, polyoffset = csmpolyoffset;
    if(smfilter > 2) { polyfactor = csmpolyfactor2; polyoffset = csmpolyoffset2; } 
    if(polyfactor || polyoffset)
    {
        glPolygonOffset(polyfactor, polyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }

    findshadowvas();
    findshadowmms();

    shadowmaskbatchedmodels(smdynshadow!=0);
    batchshadowmapmodels();

    loopi(csmsplits) if(csm.splits[i].idx >= 0)
    {
        const shadowmapinfo &sm = shadowmaps[csm.splits[i].idx];

        shadowmatrix.mul(csm.splits[i].proj, csm.model);
        GLOBALPARAM(shadowmatrix, shadowmatrix);

        glViewport(sm.x, sm.y, sm.size, sm.size);
        glScissor(sm.x, sm.y, sm.size, sm.size);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowside = i;

        rendershadowmapworld();
        rendershadowmodelbatches();
    }

    clearbatchedmapmodels();

    if(polyfactor || polyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

int calcshadowinfo(const extentity &e, vec &origin, float &radius, vec &spotloc, int &spotangle, float &bias)
{
    if(e.attr5&L_NOSHADOW || (e.attr1 > 0 && e.attr1 <= smminradius)) return SM_NONE;

    origin = e.o;
    radius = e.attr1 > 0 ? e.attr1 : 2*worldsize;
    int type, w, border;
    float lod;
    if(e.attached && e.attached->type == ET_SPOTLIGHT)
    {
        type = SM_SPOT;
        w = 1;
        border = 0;
        lod = smspotprec;
        spotloc = e.attached->o;
        spotangle = clamp(int(e.attached->attr1), 1, 89);
    }
    else
    {
        type = SM_CUBEMAP;
        w = 3;
        lod = smcubeprec;
        border = smfilter > 2 ? smborder2 : smborder;
        spotloc = e.o;
        spotangle = 0;
    }
   
    lod *= smminsize;
    int size = clamp(int(ceil((lod * shadowatlaspacker.w) / SHADOWATLAS_SIZE)), 1, shadowatlaspacker.w / w);
    bias = border / float(size - border);

    return type;
}
   
glmatrix shadowmatrix;
 
void rendershadowmaps()
{
    float polyfactor = smpolyfactor, polyoffset = smpolyoffset;
    if(smfilter > 2) { polyfactor = smpolyfactor2; polyoffset = smpolyoffset2; }
    if(polyfactor || polyoffset)
    {
        glPolygonOffset(polyfactor, polyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }

    const vector<extentity *> &ents = entities::getents();
    loopv(shadowmaps)
    {
        shadowmapinfo &sm = shadowmaps[i];
        if(sm.light < 0) continue;

        lightinfo &l = lights[sm.light];
        extentity *e = l.ent >= 0 ? ents[l.ent] : NULL;

        int border, sidemask;
        if(l.spot)
        {
            shadowmapping = SM_SPOT;
            border = 0;
            sidemask = 1;
        }
        else
        {
            shadowmapping = SM_CUBEMAP;
            border = smfilter > 2 ? smborder2 : smborder;
            sidemask = smsidecull ? cullfrustumsides(l.o, l.radius, sm.size, border) : 0x3F;
        }

        sm.sidemask = sidemask;

        shadoworigin = l.o;
        shadowradius = l.radius;
        shadowbias = border / float(sm.size - border);
        shadowdir = l.dir;
        shadowspot = l.spot;

        shadowmesh *mesh = e ? findshadowmesh(l.ent, *e) : NULL;

        findshadowvas();
        findshadowmms();

        shadowmaskbatchedmodels(!(l.flags&L_NODYNSHADOW) && smdynshadow);
        batchshadowmapmodels();

        shadowcacheval *cached = NULL;
        int cachemask = 0;
        if(smcache)
        {
            int dynmask = smcache <= 1 ? batcheddynamicmodels() : 0;
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
        glmatrix smprojmatrix(vec4(float(sm.size - border) / sm.size, 0, 0, 0),
                              vec4(0, float(sm.size - border) / sm.size, 0, 0),
                              vec4(0, 0, -(smfarclip + smnearclip) / (smfarclip - smnearclip), -2*smnearclip*smfarclip / (smfarclip - smnearclip)),
                              vec4(0, 0, -1, 0));

        if(shadowmapping == SM_SPOT)
        {
            glViewport(sm.x, sm.y, sm.size, sm.size);
            glScissor(sm.x, sm.y, sm.size, sm.size);
            glClear(GL_DEPTH_BUFFER_BIT);

            glmatrix spotmatrix(l.spotx, l.spoty, vec(l.dir).neg());
            spotmatrix.scale(1.0f/l.radius);
            spotmatrix.translate(vec(l.o).neg());
            shadowmatrix.mul(smprojmatrix, spotmatrix);
            GLOBALPARAM(shadowmatrix, shadowmatrix);

            glCullFace((l.dir.scalartriple(l.spoty, l.spotx) < 0) == (smcullside != 0) ? GL_BACK : GL_FRONT);

            shadowside = 0;

            if(mesh) rendershadowmesh(mesh); else rendershadowmapworld();
            rendershadowmodelbatches();
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

                glmatrix cubematrix(cubeshadowviewmatrix[side]);
                cubematrix.scale(1.0f/l.radius);
                cubematrix.translate(vec(l.o).neg());
                shadowmatrix.mul(smprojmatrix, cubematrix);
                GLOBALPARAM(shadowmatrix, shadowmatrix);

                glCullFace((side & 1) ^ (side >> 2) ^ smcullside ? GL_FRONT : GL_BACK);

                shadowside = side;

                if(mesh) rendershadowmesh(mesh); else rendershadowmapworld();
                rendershadowmodelbatches();
            }
        }

        clearbatchedmapmodels();
    }

    if(polyfactor || polyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowatlas()
{
    timer *smcputimer = begintimer("shadow map", false);
    timer *smtimer = begintimer("shadow map");

    glBindFramebuffer_(GL_FRAMEBUFFER, shadowatlasfbo);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    if(debugshadowatlas)
    {
        glClearDepth(0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearDepth(1);
    }

    glEnable(GL_SCISSOR_TEST);

    // sun light
    if(sunlight && csmshadowmap)
    {
        csm.setup();
        rendercsmshadowmaps();
    }

    packlights();

    // point lights
    rendershadowmaps();

    glCullFace(GL_BACK);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    endtimer(smtimer);
    endtimer(smcputimer);
}

FVAR(refractmargin, 0, 0.1f, 1);
FVAR(refractdepth, 1e-3f, 16, 1e3f);

void rendertransparent()
{
    int hasalphavas = findalphavas();
    int hasmats = findmaterials();
    bool hasmodels = transmdlsx1 < transmdlsx2 && transmdlsy1 < transmdlsy2;
    if(!hasalphavas && !hasmats && !hasmodels) return;

    timer *transtimer = begintimer("transparent");

    if(hasalphavas&4 || hasmats&4)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER, msaasamples ? msrefractfbo : refractfbo);
        glDepthMask(GL_FALSE);
        if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
        else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
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
        GLOBALPARAMF(refractdepth, (1.0f/refractdepth));
        SETSHADER(refractmask);
        if(hasalphavas&4) renderrefractmask();
        if(hasmats&4) rendermaterialmask();

        glDepthMask(GL_TRUE);
    }

    glActiveTexture_(GL_TEXTURE7);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msrefracttex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, refracttex);
    glActiveTexture_(GL_TEXTURE8);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mshdrtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, hdrtex);
    glActiveTexture_(GL_TEXTURE9);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
    glActiveTexture_(GL_TEXTURE0);

    if(stencilformat) glEnable(GL_STENCIL_TEST);

    glmatrix raymatrix(vec(-0.5f*vieww*projmatrix.a.x, 0, 0.5f*vieww), 
                       vec(0, -0.5f*viewh*projmatrix.b.y, 0.5f*viewh));
    raymatrix.mul(cammatrix);
    GLOBALPARAM(raymatrix, raymatrix);
    GLOBALPARAM(linearworldmatrix, linearworldmatrix);

    uint tiles[LIGHTTILE_MAXH];
    float sx1, sy1, sx2, sy2;

    loop(layer, 4)
    {
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
                loopj(LIGHTTILE_MAXH) tiles[j] |= matsolidtiles[j];
            }
            break;
        case 3:
            if(!hasmodels) continue;
            sx1 = transmdlsx1; sy1 = transmdlsy1; sx2 = transmdlsx2; sy2 = transmdlsy2;
            memcpy(tiles, transmdltiles, sizeof(tiles));
            break;

        default:
            continue;
        }

        glBindFramebuffer_(GL_FRAMEBUFFER, msaasamples ? msfbo : gfbo);
        if(stencilformat)
        {
            glStencilFunc(GL_ALWAYS, layer+1, ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            maskgbuffer(maskedaa() || msaasamples ? "cg" : "cng");
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
        }
        maskgbuffer("cngd"); // workaround for strange Radeon HD 7340 bug, if not here (instead of inside the glClear branch where it should be), stencil doesn't work!

        extern int wireframe;
        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
        case 3:
            if(hasmodels) rendertransparentmodelbatches();
            break;
        }

        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if(msaasamples)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER, mshdrfbo);
            if(stencilformat)
            {
                if(!msaaedgedetect)
                {
                    glStencilFunc(GL_EQUAL, layer+1, ~0);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    renderlights(sx1, sy1, sx2, sy2, tiles, 0, 3);
                }
                else loopi(2) renderlights(sx1, sy1, sx2, sy2, tiles, layer+1, i+1);
            }
            else renderlights(sx1, sy1, sx2, sy2, tiles, 0, 3);
        }
        else
        {
            glBindFramebuffer_(GL_FRAMEBUFFER, hdrfbo);
            if(stencilformat)
            {
                glStencilFunc(GL_EQUAL, layer+1, ~0);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            }
            renderlights(sx1, sy1, sx2, sy2, tiles);
        }
    }

    if(stencilformat) glDisable(GL_STENCIL_TEST);

    endtimer(transtimer);
}

VAR(gdepthclear, 0, 1, 1);
VAR(gcolorclear, 0, 1, 1);

void preparegbuffer(bool depthclear)
{
    glBindFramebuffer_(GL_FRAMEBUFFER, msaasamples ? msfbo : gfbo);
    glViewport(0, 0, vieww, viewh);

    if(drawtex)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, vieww, viewh);
    }
    if(gdepthformat && gdepthclear)
    {
        maskgbuffer("d");
        if(gdepthformat == 1) glClearColor(1, 1, 1, 1);
        else glClearColor(-farplane, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        maskgbuffer("cng");
    }
    if(gcolorclear) glClearColor(0, 0, 0, 0);
    glClear((depthclear ? GL_DEPTH_BUFFER_BIT : 0)|(gcolorclear ? GL_COLOR_BUFFER_BIT : 0)|(depthclear && stencilformat ? GL_STENCIL_BUFFER_BIT : 0));
    if(gdepthformat && gdepthclear) maskgbuffer("cngd");
    if(drawtex) glDisable(GL_SCISSOR_TEST);

    glmatrix invscreenmatrix;
    invscreenmatrix.identity();
    invscreenmatrix.settranslation(-1.0f, -1.0f, -1.0f);
    invscreenmatrix.setscale(2.0f/vieww, 2.0f/viewh, 2.0f);
    eyematrix.mul(invprojmatrix, invscreenmatrix);
    if(drawtex == DRAWTEX_MINIMAP)
    {
        linearworldmatrix.mul(invcamprojmatrix, invscreenmatrix);
        worldmatrix = linearworldmatrix;
    }
    else
    {
        linearworldmatrix.mul(invprojmatrix, invscreenmatrix);
        float xscale = linearworldmatrix.a.x, yscale = linearworldmatrix.b.y, xoffset = linearworldmatrix.d.x, yoffset = linearworldmatrix.d.y, zscale = linearworldmatrix.d.z;
        glmatrix depthmatrix(vec(xscale/zscale, 0, xoffset/zscale), vec(0, yscale/zscale, yoffset/zscale));
        linearworldmatrix.mul(invcammatrix, depthmatrix);
        if(gdepthformat) worldmatrix = linearworldmatrix;
        else worldmatrix.mul(invcamprojmatrix, invscreenmatrix);
    }

    screenmatrix.identity();
    screenmatrix.settranslation(0.5f*vieww, 0.5f*viewh, 0.5f);
    screenmatrix.setscale(0.5f*vieww, 0.5f*viewh, 0.5f);
    screenmatrix.mul(camprojmatrix);

    GLOBALPARAMF(viewsize, (vieww, viewh, 1.0f/vieww, 1.0f/viewh));
    GLOBALPARAMF(gdepthscale, (eyematrix.d.z, eyematrix.c.w, eyematrix.d.w));
    GLOBALPARAMF(gdepthpackparams, (-1.0f/farplane, -255.0f/farplane, -(255.0f*255.0f)/farplane));
    GLOBALPARAMF(gdepthunpackparams, (-farplane, -farplane/255.0f, -farplane/(255.0f*255.0f)));

    GLERROR;
}

void rendergbuffer(bool depthclear)
{
    timer *gcputimer = drawtex ? NULL : begintimer("g-buffer", false);
    timer *gtimer = drawtex ? NULL : begintimer("g-buffer");

    preparegbuffer(depthclear);

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

    if(drawtex == DRAWTEX_MINIMAP)
    {
        renderminimapmaterials();
    }
    else if(!drawtex)
    {
        game::rendergame();
        rendermodelbatches();
        GLERROR;
        maskgbuffer("c");
        renderdecals();
        maskgbuffer("cngd");
        GLERROR;
        renderavatar();
        GLERROR;
    }

    endtimer(gtimer);
    endtimer(gcputimer);
}

void shademinimap(const vec &color)
{
    GLERROR;

    glBindFramebuffer_(GL_FRAMEBUFFER, msaasamples ? mshdrfbo : hdrfbo);
    glViewport(0, 0, vieww, viewh);

    if(color.x >= 0)
    {
        glClearColor(color.x, color.y, color.z, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    renderlights(-1, -1, 1, 1, NULL, 0, msaasamples ? -1 : 0);
    GLERROR;
}

void shademodelpreview(int x, int y, int w, int h, bool background, bool scissor)
{
    GLERROR;

    GLuint outfbo = w > vieww || h > viewh ? scalefbo[0] : 0;
    glBindFramebuffer_(GL_FRAMEBUFFER, outfbo);
    glViewport(outfbo ? 0 : x, outfbo ? 0 : y, vieww, viewh);

    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mscolortex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gcolortex);
    glActiveTexture_(GL_TEXTURE1);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msnormaltex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gnormaltex);
    glActiveTexture_(GL_TEXTURE2);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msglowtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gglowtex);
    glActiveTexture_(GL_TEXTURE3);
    if(msaasamples) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
    else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
    glActiveTexture_(GL_TEXTURE0);

    GLOBALPARAM(worldmatrix, worldmatrix);

    float lightscale = 2.0f*ldrscale;
    GLOBALPARAMF(lightscale, (0.1f*lightscale, 0.1f*lightscale, 0.1f*lightscale, lightscale));
    GLOBALPARAM(sunlightdir, vec(0, -1, 2).normalize());
    GLOBALPARAMF(sunlightcolor, (0.6f*lightscale, 0.6f*lightscale, 0.6f*lightscale));

    SETSHADER(modelpreview);

    if(outfbo)
    {
        screenquad(vieww, viewh);

        glBindFramebuffer_(GL_FRAMEBUFFER, 0); 
        glViewport(x, y, w, h);
        glBindTexture(GL_TEXTURE_RECTANGLE, scaletex[0]);
        SETSHADER(scalelinear);
    }
    
    if(!background)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    if(scissor) glEnable(GL_SCISSOR_TEST);
    screenquad(vieww, viewh);
    if(scissor) glDisable(GL_SCISSOR_TEST);
    if(!background) glDisable(GL_BLEND);

    GLERROR;
    
    glViewport(0, 0, screenw, screenh);
}

void shadegbuffer()
{
    GLERROR;

    timer *shcputimer = begintimer("deferred shading", false);
    timer *shtimer = begintimer("deferred shading");

    glBindFramebuffer_(GL_FRAMEBUFFER, msaasamples ? mshdrfbo : hdrfbo);
    glViewport(0, 0, vieww, viewh);

    if(hdrclear > 0 || msaasamples)
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        if(hdrclear > 0) hdrclear--;
    }
    drawskybox(farplane);

    if(msaasamples)
    {
        if(stencilformat && msaaedgedetect)
        {
            glEnable(GL_STENCIL_TEST);
            loopi(2) renderlights(-1, -1, 1, 1, NULL, 0, i+1);
            glDisable(GL_STENCIL_TEST);
        }
        else renderlights(-1, -1, 1, 1, NULL, 0, drawtex ? -1 : 3);
    }
    else renderlights();
    GLERROR;

    endtimer(shtimer);
    endtimer(shcputimer);
}

void setupframe(int w, int h)
{
    GLERROR;
    setupgbuffer(w, h);
    if(hdr && (bloomw < 0 || bloomh < 0)) setupbloom(gw, gh);
    if(ao && (aow < 0 || aoh < 0)) setupao(gw, gh);
    if(!shadowatlasfbo) setupshadowatlas();
    if(sunlight && csmshadowmap && gi && giscale && gidist && !rhfbo) setupradiancehints();
    if(!deferredlightshader) loaddeferredlightshaders();
    if(drawtex == DRAWTEX_MINIMAP && !deferredminimapshader) deferredminimapshader = loaddeferredlightshader(msaasamples ? "Mm" : "m");
    setupaa(gw, gh);
    GLERROR;
}

bool debuglights()
{
    if(debugshadowatlas) viewshadowatlas();
    else if(debugao) viewao();
    else if(debugdepth) viewdepth();
    else if(debugrefract) viewrefract();
    else if(debuglightscissor) viewlightscissor();
    else if(debugrsm) viewrsm();
    else if(debugrh) viewrh();
    else if(!debugaa()) return false;
    return true;
}

void cleanuplights()
{
    cleanupgbuffer();
    cleanupbloom();
    cleanupao();
    cleanupshadowatlas();
    cleanupradiancehints();
    cleanuplightsphere();
    cleanupaa();
}

