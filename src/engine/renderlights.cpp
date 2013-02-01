#include "engine.h"

int gw = -1, gh = -1, bloomw = -1, bloomh = -1, lasthdraccum = 0;
GLuint gfbo = 0, gdepthtex = 0, gcolortex = 0, gnormaltex = 0, gglowtex = 0, gdepthrb = 0, gstencilrb = 0;
GLuint scalefbo = 0, scaletex = 0;
GLuint hdrfbo = 0, hdrtex = 0, bloomfbo[6] = { 0, 0, 0, 0, 0, 0 }, bloomtex[6] = { 0, 0, 0, 0, 0, 0 };
int hdrclear = 0;
GLuint infbo = 0, intex[2] = { 0, 0 }, indepth = 0, inw = -1, inh = -1;
GLuint refractfbo = 0, refracttex = 0;
GLenum bloomformat = 0, hdrformat = 0;
bool hdrfloat = false;
int aow = -1, aoh = -1;
GLuint aofbo[3] = { 0, 0, 0 }, aotex[3] = { 0, 0, 0 }, aonoisetex = 0;
glmatrixf eyematrix, worldmatrix, linearworldmatrix, screenmatrix;

extern int bloomsize, bloomprec;
extern int ati_pf_bug;

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

    bloomformat = bloomprec >= 3 && hasTF ? GL_RGB16F_ARB : (bloomprec >= 2 && hasPF && !ati_pf_bug ? GL_R11F_G11F_B10F_EXT : (bloomprec >= 1 ? GL_RGB10 : GL_RGB));
    createtexture(bloomtex[0], max(gw/2, bloomw), max(gh/2, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[1], max(gw/4, bloomw), max(gh/4, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[2], bloomw, bloomh, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[3], bloomw, bloomh, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    if(bloomformat != GL_RGB)
    {
        if(!bloomtex[5]) glGenTextures(1, &bloomtex[5]);
        if(!bloomfbo[5]) glGenFramebuffers_(1, &bloomfbo[5]);
        createtexture(bloomtex[5], bloomw, bloomh, NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    }

    static uchar gray[3] = { 32, 32, 32 };
    static float grayf[3] = { 0.125f, 0.125f, 0.125f };
    createtexture(bloomtex[4], 1, 1, hasTF ? (void *)grayf : (void *)gray, 3, 1, hasTF ? GL_RGB16F_ARB : GL_RGB16, GL_TEXTURE_RECTANGLE_ARB);

    loopi(5 + (bloomformat != GL_RGB ? 1 : 0))
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
    loopi(6) if(bloomfbo[i]) { glDeleteFramebuffers_(1, &bloomfbo[i]); bloomfbo[i] = 0; }
    loopi(6) if(bloomtex[i]) { glDeleteTextures(1, &bloomtex[i]); bloomtex[i] = 0; }
    bloomw = bloomh = -1;
    lasthdraccum = 0;
}

extern int ao, aotaps, aoreduce, aoreducedepth, aonoise, aobilateral, aopackdepth, aodepthformat;

static Shader *bilateralshader[2] = { NULL, NULL };

Shader *loadbilateralshader(int pass)
{
    if(!aobilateral) return nullshader;

    string opts;
    int optslen = 0;

    if(aoreduce) opts[optslen++] = 'r';
    bool linear = aopackdepth || (aoreducedepth && (aoreduce || aoreducedepth > 1));
    if(linear)
    {
        opts[optslen++] = 'l';
        if(aopackdepth) opts[optslen++] = 'p';
    }
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

void setbilateralshader(int radius, int pass, float sigma, float depth, bool linear, bool packed)
{
    float stepx = linear ? 1 : float(vieww)/aow, stepy = linear ? 1 : float(viewh)/aoh;
    bilateralshader[pass]->set();
    sigma *= 2*radius;
    LOCALPARAM(bilateralparams, (1.0f/(2*sigma*sigma), 1.0f/(depth*depth), pass==0 ? stepx : 0, pass==1 ? stepy : 0));
}

static Shader *ambientobscuranceshader = NULL;

Shader *loadambientobscuranceshader()
{
    string opts;
    int optslen = 0;

    if(aoreduce) opts[optslen++] = 'r';
    bool linear = aoreducedepth && (aoreduce || aoreducedepth > 1);
    if(linear) opts[optslen++] = 'l';
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
    w >>= aoreduce;
    h >>= aoreduce;

    if(w == aow && h == aoh) return;

    aow = w;
    aoh = h;

    if(!aonoisetex) glGenTextures(1, &aonoisetex);
    bvec *noise = new bvec[(1<<aonoise)*(1<<aonoise)];
    loopk((1<<aonoise)*(1<<aonoise)) noise[k] = bvec(vec(rndscale(2)-1, rndscale(2)-1, 0).normalize());
    createtexture(aonoisetex, 1<<aonoise, 1<<aonoise, noise, 0, 0, GL_RGB, GL_TEXTURE_2D);
    delete[] noise;

    loopi(2)
    {
        if(!aotex[i]) glGenTextures(1, &aotex[i]);
        if(!aofbo[i]) glGenFramebuffers_(1, &aofbo[i]);
        createtexture(aotex[i], w, h, NULL, 3, 1, aobilateral && aopackdepth && aodepthformat ? GL_RG16F : GL_RGBA8 , GL_TEXTURE_RECTANGLE_ARB);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, aotex[i], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating AO buffer!");
    }

    if(aoreducedepth && (aoreduce || aoreducedepth > 1))
    {
        if(!aotex[2]) glGenTextures(1, &aotex[2]);
        if(!aofbo[2]) glGenFramebuffers_(1, &aofbo[2]);
        createtexture(aotex[2], w, h, NULL, 3, 0, aodepthformat ? GL_R16F : GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[2]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, aotex[2], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating AO buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadaoshaders();
    loadbilateralshaders();
}

void cleanupao()
{
    loopi(3) if(aofbo[i]) { glDeleteFramebuffers_(1, &aofbo[i]); aofbo[i] = 0; }
    loopi(3) if(aotex[i]) { glDeleteTextures(1, &aotex[i]); aotex[i] = 0; }
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
VARFP(aofloatdepth, 0, 1, 1, initwarning("AO setup", INIT_LOAD, CHANGE_SHADERS));
VAR(aodepthformat, 1, 0, 0);
VARF(aonoise, 0, 5, 8, cleanupao());
VARFP(aobilateral, 0, 3, 10, cleanupao());
FVARP(aobilateralsigma, 0, 0.5f, 1e3f);
FVARP(aobilateraldepth, 0, 4, 1e3f);
VARF(aopackdepth, 0, 1, 1, cleanupao());
VARFP(aotaps, 1, 5, 12, cleanupao());
VAR(debugao, 0, 0, 1);

void initao()
{
    aodepthformat = aofloatdepth && hasTRG && hasTF ? 1 : 0;
}

void viewao()
{
    if(!ao) return;
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[0]);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, aoh); glVertex2i(0, 0);
    glTexCoord2f(aow, aoh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(aow, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

void renderao()
{
    if(!ao) return;

    timer *aotimer = begintimer("ambient obscurance");

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);

    bool linear = aoreducedepth && (aoreduce || aoreducedepth > 1);
    float xscale = eyematrix.v[0], yscale = eyematrix.v[5];
    if(linear)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[2]);
        glViewport(0, 0, aow, aoh);
        SETSHADER(linearizedepth);
        screenquad(vieww, viewh);

        xscale *= float(vieww)/aow;
        yscale *= float(viewh)/aoh;

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[2]);
    }

    ambientobscuranceshader->set();

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[0]);
    glViewport(0, 0, aow, aoh);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, aonoisetex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    LOCALPARAM(tapparams, (aoradius*eyematrix.v[14]/xscale, aoradius*eyematrix.v[14]/yscale, aoradius*aoradius*aocutoff*aocutoff));
    LOCALPARAM(contrastparams, ((2.0f*aodark)/aotaps, aosharp));
    LOCALPARAM(offsetscale, (xscale/eyematrix.v[14], yscale/eyematrix.v[14], eyematrix.v[12]/eyematrix.v[14], eyematrix.v[13]/eyematrix.v[14]));
    LOCALPARAM(prefilterdepth, (aoprefilterdepth));
    screenquad(vieww, viewh, aow/float(1<<aonoise), aoh/float(1<<aonoise));

    if(aobilateral)
    {
        if(!linear && aopackdepth) linear = true;
        loopi(2 + 2*aoiter)
        {
            setbilateralshader(aobilateral, i%2, aobilateralsigma, aobilateraldepth, linear, linear && aopackdepth);
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[i%2]);
            if(!linear || !aopackdepth)
            {
                glActiveTexture_(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, linear ? aotex[2] : gdepthtex);
                glActiveTexture_(GL_TEXTURE0_ARB);
            }
            screenquad(vieww, viewh);
        }
    }
    else if(aoblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(aoblur, aosigma, blurweights, bluroffsets);
        loopi(2 + 2*aoiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            setblurshader(i%2, 1, aoblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[i%2]);
            screenquad(aow, aoh);
        }
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
    glViewport(0, 0, vieww, viewh);

    endtimer(aotimer);
}

extern int inferprec, inferdepth;

void setupinferred(int w, int h)
{
    inw = (w+1)/2;
    inh = (h+1)/2;

    loopi(2) if(!intex[i]) glGenTextures(1, &intex[i]);
    if(!infbo) glGenFramebuffers_(1, &infbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, infbo);

    GLenum infmt = inferprec >= 3 && hasTF ? GL_RGB16F_ARB : (inferprec >= 2 && hasPF && !ati_pf_bug ? GL_R11F_G11F_B10F_EXT : (inferprec >= 1 ? GL_RGB10 : GL_RGB));

    loopi(2)
    {
        createtexture(intex[i], inw, inh, NULL, 3, 0, infmt, GL_TEXTURE_RECTANGLE_ARB);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat border[4] = { 0, 0, 0, 0 };
        glTexParameterfv(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_BORDER_COLOR, border);

        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_RECTANGLE_ARB, intex[i], 0);
    }

    static const GLenum drawbufs[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    glDrawBuffers_(2, drawbufs);

    if(inferdepth)
    {
        if(!indepth) glGenRenderbuffers_(1, &indepth);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, indepth);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, inw, inh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, indepth);
    }

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating inferred lighting buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

}

void cleanupinferred()
{
    if(infbo) { glDeleteFramebuffers_(1, &infbo); infbo = 0; }
    loopi(2) if(intex[i]) { glDeleteTextures(1, &intex[i]); intex[i] = 0; }
    if(indepth) { glDeleteRenderbuffers_(1, &indepth); indepth = 0; }
    inw = inh = -1;
}

VARFP(inferlights, 0, 0, 1, { cleardeferredlightshaders(); cleanupinferred(); });
FVAR(inferthreshold, -1e3f, 0.15f, 1e3f);
FVAR(inferspecthreshold, 0, 0.25f, 1e3f);
VAR(inferskiplights, 0, 0, 1);
VARFP(inferprec, 0, 2, 3, cleanupinferred());
VARF(inferdepth, 0, 1, 1, cleanupinferred());
VAR(inferclear, 0, 0, 1);
VAR(infertransparent, 0, 0, 1);
VAR(debuginferred, 0, 0, 2);

void viewinferred()
{
    int w = inw, h = inh;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, intex[debuginferred > 1 ? 1 : 0]);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, inh); glVertex2i(0, 0);
    glTexCoord2f(inw, inh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(inw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

void cleanupscale()
{
    if(scalefbo) { glDeleteFramebuffers_(1, &scalefbo); scalefbo = 0; }
    if(scaletex) { glDeleteTextures(1, &scaletex); scaletex = 0; }
}

void setupscale(int w, int h)
{
    if(!scaletex) glGenTextures(1, &scaletex);
    if(!scalefbo) glGenFramebuffers_(1, &scalefbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, scalefbo);

    createtexture(scaletex, w, h, NULL, 3, 1, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, scaletex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating scale buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    useshaderbyname("scale");
}

GLuint shouldscale()
{
    return scalefbo;
}
     
void doscale(int w, int h)
{
    if(!scaletex) return;

    timer *scaletimer = begintimer("scaling");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
    glViewport(0, 0, w, h);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, scaletex);
    
    SETSHADER(scale);

    screenquad(gw, gh);

    endtimer(scaletimer);
}
   
VARFP(glineardepth, 0, 0, 3, initwarning("g-buffer setup", INIT_LOAD, CHANGE_SHADERS));
VAR(gdepthformat, 1, 0, 0);

void initgbuffer()
{
    if(glineardepth >= 2 && (!hasAFBO || !hasTF || !hasTRG)) gdepthformat = 1;
    else gdepthformat = glineardepth;

    initao();
}

void maskgbuffer(const char *mask)
{
    GLenum drawbufs[4];
    int numbufs = 0;
    while(*mask) switch(*mask++)
    {
        case 'c': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT0_EXT; break;
        case 'n': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT1_EXT; break;
        case 'g': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT2_EXT; break;
        case 'd': if(gdepthformat) drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT3_EXT; break;
    }
    glDrawBuffers_(numbufs, drawbufs);
}

extern int hdrprec;
extern float gscale;

void setupgbuffer(int w, int h)
{
    if(gscale < 1)
    {
        w = max(int(ceil(w*gscale)), 1);
        h = max(int(ceil(h*gscale)), 1);
    }

    if(gw == w && gh == h && (gscale < 1) == (scalefbo!=0)) return;

    if(gscale < 1) setupscale(w, h);
    else cleanupscale();

    gw = w;
    gh = h;

    if(!gdepthtex) glGenTextures(1, &gdepthtex);
    if(!gcolortex) glGenTextures(1, &gcolortex);
    if(!gnormaltex) glGenTextures(1, &gnormaltex);
    if(!gglowtex) glGenTextures(1, &gglowtex);
    if(!gfbo) glGenFramebuffers_(1, &gfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);

    maskgbuffer("cngd");

    static const GLenum depthformats[] = { GL_RGBA8, GL_R16F, GL_R32F };
    GLenum depthformat = gdepthformat ? depthformats[gdepthformat-1] : (gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT);
    createtexture(gdepthtex, gw, gh, NULL, 3, 0, depthformat, GL_TEXTURE_RECTANGLE_ARB);

    createtexture(gcolortex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gnormaltex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gglowtex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        if(!gdepthrb) glGenRenderbuffers_(1, &gdepthrb);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, gdepthrb);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
    }
    else if(gstencil && (!gdepthstencil || !hasDS))
    {
        if(!gstencilrb) glGenRenderbuffers_(1, &gstencilrb);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, gstencilrb);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX8_EXT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
    }

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, gcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, gnormaltex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_RECTANGLE_ARB, gglowtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating g-buffer!");

    if(!hdrtex) glGenTextures(1, &hdrtex);

    if(!hdrfbo) glGenFramebuffers_(1, &hdrfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);

    hdrformat = hdr ? (hdrprec >= 3 && hasTF ? GL_RGB16F_ARB : (hdrprec >= 2 && hasPF && !ati_pf_bug ? GL_R11F_G11F_B10F_EXT : (hdrprec >= 1 ? GL_RGB10 : GL_RGB))) : GL_RGB;
    hdrfloat = floatformat(hdrformat);
    createtexture(hdrtex, gw, gh, NULL, 3, 1, hdrformat, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, hdrtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating HDR buffer!");

    hdrclear = 3;

    if(!refracttex) glGenTextures(1, &refracttex);
    if(!refractfbo) glGenFramebuffers_(1, &refractfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, refractfbo);

    createtexture(refracttex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, refracttex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating refraction buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    cleanupbloom();
    cleanupao();
    cleanupinferred();
    cleanupaa();
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
    cleardeferredlightshaders();
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
FVARFP(gscale, 0.25f, 1, 1, cleanupgbuffer());

float ldrscale = 1.0f, ldrscaleb = 1.0f/255;

void readhdr(int w, int h, GLenum format, GLenum type, void *dst, GLenum target, GLuint tex)
{
    GLERROR;

    if(hasCBF || !hdrfloat) glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(w, h);
    }

    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR_ARB, GL_TRUE);

    if(dst) glReadPixels(0, 0, w, h, format, type, dst);
    else if(target)
    {
        glBindTexture(target, tex);
        GLERROR;
        glCopyTexImage2D(target, 0, format, 0, 0, w, h, 0);
    }

    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR_ARB, GL_FIXED_ONLY_ARB);

    hdrclear = 3;
}

void loadhdrshaders(int aa)
{
    switch(aa)
    {
        case AA_LUMA:
            useshaderbyname("hdrtonemapluma");
            useshaderbyname("hdrnopluma");
            break;
        case AA_VELOCITY:
            useshaderbyname("hdrtonemapvelocity");
            useshaderbyname("hdrnopvelocity");
            break;
        default:
            break;
    }
}

void processhdr(GLuint outfbo, int aa)
{
    if(!hdr)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
        glViewport(0, 0, vieww, viewh);
        switch(aa)
        {
            case AA_LUMA: SETSHADER(hdrnopluma); break;
            case AA_VELOCITY:
                SETSHADER(hdrnopvelocity);
                glActiveTexture_(GL_TEXTURE3_ARB);
                setaavelocityparams();
                glActiveTexture_(GL_TEXTURE0_ARB);
                break;
            default: SETSHADER(hdrnop); break;
        }
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(vieww, viewh);
        return;
    }

    timer *hdrtimer = begintimer("hdr processing");

    GLOBALPARAM(hdrparams, (hdrbright, hdrsaturate, bloomthreshold, bloomscale));
    GLOBALPARAM(hdrgamma, (hdrgamma, 1.0f/hdrgamma));

    GLuint b0fbo = bloomfbo[1], b0tex = bloomtex[1], b1fbo =  bloomfbo[0], b1tex = bloomtex[0], ptex = hdrtex;
    int b0w = max(vieww/4, bloomw), b0h = max(viewh/4, bloomh), b1w = max(vieww/2, bloomw), b1h = max(viewh/2, bloomh),
        pw = vieww, ph = viewh;
    if(hdrreduce) while(pw > bloomw || ph > bloomh)
    {
        GLuint cfbo = b1fbo, ctex = b1tex;
        int cw = max(pw/2, bloomw), ch = max(ph/2, bloomh);

        if(hdrreduce > 1 && cw/2 >= bloomw && ch/2 >= bloomh)
        {
            cw /= 2;
            ch /= 2;
            SETSHADER(hdrreduce2);
        }
        else SETSHADER(hdrreduce);
        if(cw == bloomw && ch == bloomh) { if(bloomfbo[5]) { cfbo = bloomfbo[5]; ctex = bloomtex[5]; } else { cfbo = bloomfbo[2]; ctex = bloomtex[2]; } }
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, cfbo);
        glViewport(0, 0, cw, ch);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ptex);
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

            if(hdrreduce > 1 && cw/2 >= 2 && ch/2 >= 2)
            {
                cw /= 2;
                ch /= 2;
                if(i) SETSHADER(hdrreduce2); else SETSHADER(hdrluminance2);
            }
            else if(i) SETSHADER(hdrreduce); else SETSHADER(hdrluminance);
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
            glViewport(0, 0, cw, ch);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ltex);
            screenquad(lw, lh);

            ltex = b1tex;
            lw = cw;
            lh = ch;
            swap(b0fbo, b1fbo);
            swap(b0tex, b1tex);
            swap(b0w, b1w);
            swap(b0h, b1h);
        }

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, bloomfbo[4]);
        glViewport(0, 0, 1, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        SETSHADER(hdraccum);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
        LOCALPARAM(accumscale, (lasthdraccum ? pow(hdraccumscale, float(lastmillis - lasthdraccum)/hdraccummillis) : 0));
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

    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomtex[4]);
    glActiveTexture_(GL_TEXTURE0_ARB);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b0fbo);
    glViewport(0, 0, b0w, b0h);
    SETSHADER(hdrbloom);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ptex);
    screenquad(pw, ph);

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
            screenquad(b0w, b0h);
            swap(b0w, b1w);
            swap(b0h, b1h);
            swap(b0tex, b1tex);
            swap(b0fbo, b1fbo);
        }
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
    glViewport(0, 0, vieww, viewh);
    switch(aa)
    {
        case AA_LUMA: SETSHADER(hdrtonemapluma); break;
        case AA_VELOCITY:
            SETSHADER(hdrtonemapvelocity);
            glActiveTexture_(GL_TEXTURE3_ARB);
            setaavelocityparams();
            glActiveTexture_(GL_TEXTURE0_ARB);
            break;
        default: SETSHADER(hdrtonemap); break;
    }       
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh, b0w, b0h);

    endtimer(hdrtimer);
}

VAR(debugdepth, 0, 0, 1);

void viewdepth()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, gh); glVertex2i(0, 0);
    glTexCoord2f(gw, gh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(gw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VAR(debugrefract, 0, 0, 1);

void viewrefract()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, refracttex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, gh); glVertex2i(0, 0);
    glTexCoord2f(gw, gh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(gw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

#define RH_MAXSPLITS 4

GLuint rhtex[6] = { 0, 0, 0, 0, 0, 0 }, rhfbo = 0;
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
    loopi(rhcache ? 6 : 3) if(!rhtex[i]) glGenTextures(1, &rhtex[i]);

    if(!rhfbo) glGenFramebuffers_(1, &rhfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rhfbo);

    GLenum rhformat = hasTF && rhprec >= 1 ? GL_RGBA16F_ARB : GL_RGBA8;

    loopi(rhcache ? 6 : 3)
    {
        create3dtexture(rhtex[i], rhgrid+2*rhborder, rhgrid+2*rhborder, (rhgrid+2*rhborder)*rhsplits, NULL, 7, 1, rhformat);
        if(rhborder)
        {
            glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            GLfloat border[4] = { 0.5f, 0.5f, 0.5f, 0 };
            glTexParameterfv(GL_TEXTURE_3D_EXT, GL_TEXTURE_BORDER_COLOR, border);
        }
        glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_3D_EXT, rhtex[i], 0, 0);
    }

    static const GLenum drawbufs[3] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
    glDrawBuffers_(3, drawbufs);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating radiance hints buffer!");

    if(!rsmdepthtex) glGenTextures(1, &rsmdepthtex);
    if(!rsmcolortex) glGenTextures(1, &rsmcolortex);
    if(!rsmnormaltex) glGenTextures(1, &rsmnormaltex);

    if(!rsmfbo) glGenFramebuffers_(1, &rsmfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rsmfbo);

    GLenum rsmformat = rsmprec >= 3 && hasTF ? GL_RGB16F_ARB : (rsmprec >= 2 && hasPF && !ati_pf_bug ? GL_R11F_G11F_B10F_EXT : (rsmprec >= 1 ? GL_RGB10 : GL_RGBA8));

    createtexture(rsmdepthtex, rsmsize, rsmsize, NULL, 3, 0, GL_DEPTH_COMPONENT16_ARB, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(rsmcolortex, rsmsize, rsmsize, NULL, 3, 0, rsmformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(rsmnormaltex, rsmsize, rsmsize, NULL, 3, 0, rsmformat, GL_TEXTURE_RECTANGLE_ARB);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, rsmdepthtex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, rsmcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, rsmnormaltex, 0);

    glDrawBuffers_(2, drawbufs);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating RSM buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadrhshaders();
}

void cleanupradiancehints()
{
    clearradiancehintscache();

    loopi(6) if(rhtex[i]) { glDeleteTextures(1, &rhtex[i]); rhtex[i] = 0; }
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
FVARF(rsmspread, 0, 0.2f, 1, clearradiancehintscache());
VAR(rhclipgrid, 0, 1, 1);
VARF(rhcache, 0, 1, 1, cleanupradiancehints());
VAR(rsmcull, 0, 1, 1);
VARFP(rhtaps, 0, 20, 32, cleanupradiancehints());
VARFR(gidist, 0, 384, 1024, { cleardeferredlightshaders(); if(!gidist) cleanupradiancehints(); });
FVARFR(giscale, 0, 1.5f, 1e3f, { cleardeferredlightshaders(); if(!giscale) cleanupradiancehints(); });
VARFP(gi, 0, 1, 1, { cleardeferredlightshaders(); cleanupradiancehints(); });

VAR(debugrsm, 0, 0, 2);
void viewrsm()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, debugrsm == 2 ? rsmnormaltex : rsmcolortex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord2f(rsmsize, 0); glVertex2i(screen->w, screen->h-h);
    glTexCoord2f(0, rsmsize); glVertex2i(screen->w-w, screen->h);
    glTexCoord2f(rsmsize, rsmsize); glVertex2i(screen->w, screen->h);
    glEnd();
    notextureshader->set();
}

VAR(debugrh, 0, 0, RH_MAXSPLITS*(128 + 2));
void viewrh()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    SETSHADER(tex3d);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_3D, rhtex[1]);
    float z = (debugrh-1+0.5f)/float((rhgrid+2*rhborder)*rhsplits);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord3f(0, 0, z); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord3f(1, 0, z); glVertex2i(screen->w, screen->h-h);
    glTexCoord3f(0, 1, z); glVertex2i(screen->w-w, screen->h);
    glTexCoord3f(1, 1, z); glVertex2i(screen->w, screen->h);
    glEnd();
    notextureshader->set();
}

#define SHADOWATLAS_SIZE 4096

PackNode shadowatlaspacker(0, 0, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE);

enum { L_NOSHADOW = 1<<0, L_NODYNSHADOW = 1<<1 };

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
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static inline void setsmcomparemode() // use embedded shadow cmp
{
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static inline bool usegatherforsm() { return smfilter > 1 && smgather && (hasTG || hasT4); }
static inline bool usesmcomparemode() { return !usegatherforsm() || (hasTG && hasGPU5); }

void viewshadowatlas()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    float tw = 1, th = 1;
    if(shadowatlastarget == GL_TEXTURE_RECTANGLE_ARB)
    {
        tw = shadowatlaspacker.w;
        th = shadowatlaspacker.h;
        rectshader->set();
    }
    else defaultshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(shadowatlastarget, shadowatlastex);
    if(usesmcomparemode()) setsmnoncomparemode();
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord2f(tw, 0); glVertex2i(screen->w, screen->h-h);
    glTexCoord2f(0, th); glVertex2i(screen->w-w, screen->h);
    glTexCoord2f(tw, th); glVertex2i(screen->w, screen->h);
    glEnd();
    if(usesmcomparemode()) setsmcomparemode();
    notextureshader->set();
}
VAR(debugshadowatlas, 0, 0, 1);

extern int smsize;

void setupshadowatlas()
{
    int size = min((1<<smsize), hwtexsize);
    shadowatlaspacker.resize(size, size);

    if(!shadowatlastex) glGenTextures(1, &shadowatlastex);

    shadowatlastarget = usegatherforsm() ? GL_TEXTURE_2D : GL_TEXTURE_RECTANGLE_ARB;
    createtexture(shadowatlastex, shadowatlaspacker.w, shadowatlaspacker.h, NULL, 3, 1, GL_DEPTH_COMPONENT16_ARB, shadowatlastarget);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    glTexParameteri(shadowatlastarget, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);

    if(!shadowatlasfbo) glGenFramebuffers_(1, &shadowatlasfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, shadowatlastarget, shadowatlastex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating shadow atlas!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void cleanupshadowatlas()
{
    if(shadowatlastex) { glDeleteTextures(1, &shadowatlastex); shadowatlastex = 0; }
    if(shadowatlasfbo) { glDeleteFramebuffers_(1, &shadowatlasfbo); shadowatlasfbo = 0; }
    clearshadowcache();
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

FVAR(smpolyfactor, -1e3f, 1, 1e3f);
FVAR(smpolyoffset, -1e3f, 0, 1e3f);
FVAR(smbias, -1e6f, 0.01f, 1e6f);
FVAR(smpolyfactor2, -1e3f, 1.5f, 1e3f);
FVAR(smpolyoffset2, -1e3f, 0, 1e3f);
FVAR(smbias2, -1e6f, 0.02f, 1e6f);
FVAR(smprec, 1e-3f, 1, 1e3f);
FVAR(smtetraprec, 1e-3f, SQRT3, 1e3f);
FVAR(smcubeprec, 1e-3f, 1, 1e3f);
FVAR(smspotprec, 1e-3f, 1, 1e3f);

VARF(smsize, 10, 12, 16, cleanupshadowatlas());
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
VARFP(smtetra, 0, 0, 1, { cleardeferredlightshaders(); clearshadowcache(); });
VAR(smtetraclip, 0, 1, 1);
VAR(smtetraclear, 0, 1, 1);
FVAR(smtetraborder, 0, 0, 1e3f);
VARF(smcullside, 0, 1, 1, cleanupshadowatlas());
VARF(smcache, 0, 1, 2, cleanupshadowatlas());
VARFP(smfilter, 0, 2, 3, { cleardeferredlightshaders(); cleanupshadowatlas(); });
VARFP(smgather, 0, 0, 1, { cleardeferredlightshaders(); cleanupshadowatlas(); });
VAR(smnoshadow, 0, 0, 1);
VAR(smnodynshadow, 0, 0, 1);
VAR(lighttilesused, 1, 0, 0);

int shadowmapping = 0;

plane smtetraclipplane;

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
        glmatrixf proj;      // one projection per split
        vec scale, offset;   // scale and offset of the projection
        int idx;             // shadowmapinfo indices
        vec center, bounds;  // max extents of shadowmap in sunlight model space
        plane cull[4];       // world space culling planes of the split's projected sides
    };
    glmatrixf model;                // model view is shared by all splits
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
        splitinfo &split = this->splits[i];
        if(split.idx < 0) continue;
        const shadowmapinfo &sm = shadowmaps[split.idx];

        vec c;
        float radius = calcfrustumboundsphere(split.nearplane, split.farplane, camera1->o, camdir, c);

        // compute the projected bounding box of the sphere
        vec tc;
        this->model.transform(c, tc);
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
        split.proj.scale(2*split.scale.x/sm.size, 2*split.scale.y/sm.size, 2*split.scale.z);
        split.proj.translate(2*split.offset.x/sm.size - 1, 2*split.offset.y/sm.size - 1, 2*split.offset.z - 1);

        const float bias = (smfilter > 2 ? csmbias2 : csmbias) * (-512.0f / sm.size) * (split.farplane - split.nearplane) / (splits[0].farplane - splits[0].nearplane);
        split.offset.add(vec(sm.x, sm.y, bias));
    }
}

void cascadedshadowmap::gencullplanes()
{
    loopi(csmsplits)
    {
        splitinfo &split = splits[i];
        glmatrixf mvp;
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
    glMatrixMode(GL_TEXTURE);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glLoadMatrixf(model.v);
    glActiveTexture_(GL_TEXTURE0_ARB);
    glMatrixMode(GL_MODELVIEW);
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
    glmatrixf model, proj;
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
    proj.scale(2*scale.x/rsmsize, 2*scale.y/rsmsize, 2*scale.z);
    proj.translate(2*offset.x/rsmsize - 1, 2*offset.y/rsmsize - 1, 2*offset.z - 1);
}

void reflectiveshadowmap::gencullplanes()
{
    glmatrixf mvp;
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
    GLOBALPARAM(rhnudge, (rhnudge*step));
}

static Shader *deferredlightshader = NULL, *deferredminimapshader = NULL, *inferredprelightshader = NULL, *inferredlightshader = NULL;

void cleardeferredlightshaders()
{
    deferredlightshader = NULL;
    deferredminimapshader = NULL;
    inferredprelightshader = NULL;
    inferredlightshader = NULL;
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
    if(smtetra && glslversion >= 130) shadow[shadowlen++] = 't';
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
    deferredlightshader = loaddeferredlightshader();
    if(inferlights && ((gdepthstencil && hasDS) || gstencil))
    {
        inferredprelightshader = loaddeferredlightshader("i");
        inferredlightshader = useshaderbyname("inferredlight");
    }
    else inferredprelightshader = inferredlightshader = NULL;
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
        bool tetra = smtetra && glslversion >= 130;
        loopv(shadowmaps)
        {
            shadowmapinfo &sm = shadowmaps[i];
            if(sm.light < 0) continue;
            lightinfo &l = lights[sm.light];
            if(sm.cached && shadowcachefull)
            {
                int w = l.spot ? sm.size : (tetra ? sm.size*2 : sm.size*3), h = l.spot ? sm.size : (tetra ? sm.size : sm.size*2);
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
    glBindBuffer_(GL_ARRAY_BUFFER_ARB, lightspherevbuf);
    glBufferData_(GL_ARRAY_BUFFER_ARB, lightspherenumverts*sizeof(vec), lightsphereverts, GL_STATIC_DRAW_ARB);
    DELETEA(lightsphereverts);

    if(!lightsphereebuf) glGenBuffers_(1, &lightsphereebuf);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, lightsphereebuf);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER_ARB, lightspherenumindices*sizeof(GLushort), lightsphereindices, GL_STATIC_DRAW_ARB);
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

void renderlights(int infer = 0, float bsx1 = -1, float bsy1 = -1, float bsx2 = 1, float bsy2 = 1, const uint *tilemask = NULL, int stencilmask = 0)
{
    Shader *s = minimapping ? deferredminimapshader : (infer > 1 ? inferredprelightshader : deferredlightshader);
    if(!s || s == nullshader) return;

    bool blend = false;
    if(infer > 1)
    {
        glBlendFunc(GL_ONE, GL_ONE);
        if(inferclear)
        {
            glEnable(GL_BLEND);
            blend = true;
        }
    }
    else
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        blend = true;
    }
    glEnable(GL_SCISSOR_TEST);

    bool depth = true;
    if(!depthtestlights) { glDisable(GL_DEPTH_TEST); depth = false; }
    else glDepthMask(GL_FALSE);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE4_ARB);
    glBindTexture(shadowatlastarget, shadowatlastex);
    if(usesmcomparemode()) setsmcomparemode(); else setsmnoncomparemode();
    if(ao)
    {
        glActiveTexture_(GL_TEXTURE5_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[0]);
    }
    if(sunlight && csmshadowmap && gi && giscale && gidist) loopi(3)
    {
        glActiveTexture_(GL_TEXTURE6_ARB + i);
        glBindTexture(GL_TEXTURE_3D_EXT, rhtex[i]);
    }
    if(infer == 1)
    {
        loopi(2)
        {
            glActiveTexture_(GL_TEXTURE9_ARB + i);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, intex[i]);
        }
        GLOBALPARAM(inferthreshold, (inferthreshold*ldrscale, 1/inferspecthreshold));
    }
    glActiveTexture_(GL_TEXTURE0_ARB);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(worldmatrix.v);
    glMatrixMode(GL_MODELVIEW);

    GLOBALPARAM(fogdir, (mvmatrix.getrow(2)));
    GLOBALPARAM(shadowatlasscale, (1.0f/shadowatlaspacker.w, 1.0f/shadowatlaspacker.h));
    if(ao)
    {
        if((editmode && fullbright) || envmapping)
        {
            GLOBALPARAM(aoscale, (0.0f, 0.0f));
            GLOBALPARAM(aoparams, (1.0f, 0.0f, 1.0f, 0.0f));
        }
        else
        {
            GLOBALPARAM(aoscale, (float(aow)/vieww, float(aoh)/viewh));
            GLOBALPARAM(aoparams, (aomin, 1.0f-aomin, aosunmin, 1.0f-aosunmin));
        }
    }
    float lightscale = (infer > 1 ? 1.0f : 2.0f)*ldrscaleb;
    if(editmode && fullbright)
        GLOBALPARAM(lightscale, (fullbrightlevel*lightscale, fullbrightlevel*lightscale, fullbrightlevel*lightscale, 255*lightscale));
    else
        GLOBALPARAM(lightscale, (ambientcolor.x*lightscale*ambientscale, ambientcolor.y*lightscale*ambientscale, ambientcolor.z*lightscale*ambientscale, 255*lightscale));

    bool sunpass = !lighttilebatch;
    if(sunlight && csmshadowmap)
    {
        csm.bindparams();
        rh.bindparams();
        if(editmode && fullbright)
        {
            GLOBALPARAM(sunlightdir, (0, 0, 0));
            GLOBALPARAM(sunlightcolor, (0, 0, 0));
            GLOBALPARAM(giscale, (0));
        }
        else
        {
            GLOBALPARAM(sunlightdir, (sunlightdir));
            GLOBALPARAM(sunlightcolor, (sunlightcolor.x*lightscale*sunlightscale, sunlightcolor.y*lightscale*sunlightscale, sunlightcolor.z*lightscale*sunlightscale));
            GLOBALPARAM(giscale, (2*giscale));
        }
        if(!batchsunlight) sunpass = true;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int btx1, bty1, btx2, bty2;
    calctilebounds(bsx1, bsy1, bsx2, bsy2, btx1, bty1, btx2, bty2);
    if(infer == 1)
    {
        int tx1 = max(int(floor((bsx1*0.5f+0.5f)*vieww)), 0), ty1 = max(int(floor((bsy1*0.5f+0.5f)*viewh)), 0),
            tx2 = min(int(ceil((bsx2*0.5f+0.5f)*vieww)), vieww), ty2 = min(int(ceil((bsy2*0.5f+0.5f)*viewh)), viewh);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        if(stencilmask)
        {
            glStencilFunc(GL_EQUAL, stencilmask|0x10, stencilmask);
            glStencilMask(0x10);
        }
        else
        {
            glStencilFunc(GL_ALWAYS, 0x10, 0x10);
            glEnable(GL_STENCIL_TEST);
        }
        if(depthtestlights && depth) { glDisable(GL_DEPTH_TEST); depth = false; }
        glScissor(tx1, ty1, tx2-tx1, ty2-ty1);
        inferredlightshader->set();
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f((vieww+1)/2, 0); glVertex3f( 1, -1, -1);
        glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
        glTexCoord2f((vieww+1)/2, (viewh+1)/2); glVertex3f( 1,  1, -1);
        glTexCoord2f(0, (viewh+1)/2); glVertex3f(-1,  1, -1);
        glEnd();
        glStencilFunc(GL_EQUAL, stencilmask, stencilmask|0x10);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        if(stencilmask) glStencilMask(~0);
    }

    if(sunpass)
    {
        int tx1 = max(int(floor((bsx1*0.5f+0.5f)*vieww)), 0), ty1 = max(int(floor((bsy1*0.5f+0.5f)*viewh)), 0),
            tx2 = min(int(ceil((bsx2*0.5f+0.5f)*vieww)), vieww), ty2 = min(int(ceil((bsy2*0.5f+0.5f)*viewh)), viewh);
        if(infer > 1) { tx1 /= 2; ty1 /= 2; tx2 = (tx2+1)/2; ty2 = (ty2+1)/2; }
        s->set();
        glScissor(tx1, ty1, tx2-tx1, ty2-ty1);
        if(depthtestlights && depth) { glDisable(GL_DEPTH_TEST); depth = false; }
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f( 1, -1, -1);
        glVertex3f(-1, -1, -1);
        glVertex3f( 1,  1, -1);
        glVertex3f(-1,  1, -1);
        glEnd();
    }

    if(depthtestlights && !depth) { glEnable(GL_DEPTH_TEST); depth = true; }
    if(hasDBT && depthtestlights > 1) glEnable(GL_DEPTH_BOUNDS_TEST_EXT);

    static LocalShaderParam lightpos("lightpos"), lightcolor("lightcolor"), spotparams("spotparams"), spotx("spotx"), spoty("spoty"), shadowparams("shadowparams"), shadowoffset("shadowoffset");
    static vec4 lightposv[8], spotparamsv[8], shadowparamsv[8];
    static vec lightcolorv[8], spotxv[8], spotyv[8];
    static vec2 shadowoffsetv[8];

    if(!lighttilebatch)
    {
        if(!blend) { glEnable(GL_BLEND); blend = true; }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        if(!lightspherevbuf) initlightsphere(10, 5);
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, lightspherevbuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, lightsphereebuf);
        glVertexPointer(3, GL_FLOAT, sizeof(vec), lightsphereverts);
        glEnableClientState(GL_VERTEX_ARRAY);

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

            s->setvariant(0, (shadowmap ? 1 : 0) + 2 + (spotlight ? 4 : 0));
            lightpos.set(lightposv, 1);
            lightcolor.set(lightcolorv, 1);
            if(spotlight) spotparams.set(spotparamsv, 1);
            if(shadowmap)
            {
                if(spotlight)
                {
                    spotx.set(spotxv, 1);
                    spoty.set(spotyv, 1);
                }
                shadowparams.set(shadowparamsv, 1);
                shadowoffset.set(shadowoffsetv, 1);
            }

            int tx1 = int(floor((sx1*0.5f+0.5f)*vieww)), ty1 = int(floor((sy1*0.5f+0.5f)*viewh)),
                tx2 = int(ceil((sx2*0.5f+0.5f)*vieww)), ty2 = int(ceil((sy2*0.5f+0.5f)*viewh));
            if(infer > 1) { tx1 /= 2; ty1 /= 2; tx2 = (tx2+1)/2; ty2 = (ty2+1)/2; }
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

            glPushMatrix();
            glTranslatef(l.o.x, l.o.y, l.o.z);
            glScalef(l.radius*lightradiustweak, l.radius*lightradiustweak, l.radius*lightradiustweak);

            if(hasDRE) glDrawRangeElements_(GL_TRIANGLES, 0, lightspherenumverts-1, lightspherenumindices, GL_UNSIGNED_SHORT, lightsphereindices);
            else glDrawElements(GL_TRIANGLES, lightspherenumindices, GL_UNSIGNED_SHORT, lightsphereindices);
            xtraverts += lightspherenumindices;
            glde++;

            glPopMatrix();
        }

        if(!outside)
        {
            outside = true;
            glDepthFunc(GL_LESS);
            glCullFace(GL_BACK);
        }

        if(hasDC && depthclamplights) glDisable(GL_DEPTH_CLAMP_NV);

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    else if(infer != 1 || !inferskiplights) for(int y = bty1; y < bty2; y++) if(!tilemask || tilemask[y])
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

                if(infer > 1 && !inferclear)
                {
                    if(n && (i || sunpass))
                    {
                        if(!blend) { glEnable(GL_BLEND); blend = true; }
                    }
                    else if(blend) { glDisable(GL_BLEND); blend = false; }
                }

                if(n)
                {
                    s->setvariant(n-1, (shadowmap ? 1 : 0) + (i || sunpass ? 2 : 0) + (spotlight ? 4 : 0));
                    lightpos.set(lightposv, n);
                    lightcolor.set(lightcolorv, n);
                    if(spotlight) spotparams.set(spotparamsv, n);
                    if(shadowmap)
                    {
                        if(spotlight)
                        {
                            spotx.set(spotxv, n);
                            spoty.set(spotyv, n);
                        }
                        shadowparams.set(shadowparamsv, n);
                        shadowoffset.set(shadowoffsetv, n);
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
                    if(infer > 1) { tx1 /= 2; ty1 /= 2; tx2 = (tx2+1)/2; ty2 = (ty2+1)/2; }
                    glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

                    if(hasDBT && depthtestlights > 1) glDepthBounds_(sz1*0.5f + 0.5f, sz2*0.5f + 0.5f);

                    // FIXME: render light geometry here
                    glBegin(GL_TRIANGLE_STRIP);
                    glVertex3f( 1, -1, sz1);
                    glVertex3f(-1, -1, sz1);
                    glVertex3f( 1,  1, sz1);
                    glVertex3f(-1,  1, sz1);
                    glEnd();
                }

                i += n;
                if(i >= tile.length()) break;
            }
        }
    }

    if(lighttilebatch)
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    glDisable(GL_SCISSOR_TEST);
    if(blend) glDisable(GL_BLEND);

    if(!depthtestlights) glEnable(GL_DEPTH_TEST);
    else
    {
        glDepthMask(GL_TRUE);
        if(hasDBT && depthtestlights > 1) glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    if(infer == 1 && !stencilmask) glDisable(GL_STENCIL_TEST);
}

VAR(oqlights, 0, 1, 1);
VAR(debuglightscissor, 0, 0, 1);

void viewlightscissor()
{
    vector<extentity *> &ents = entities::getents();
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
                glColor3f(l.color.x/255, l.color.y/255, l.color.z/255);
                float x1 = (l.sx1+1)/2*screen->w, x2 = (l.sx2+1)/2*screen->w,
                      y1 = (1-l.sy1)/2*screen->h, y2 = (1-l.sy2)/2*screen->h;
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2f(x1, y1);
                glVertex2f(x2, y1);
                glVertex2f(x1, y2);
                glVertex2f(x2, y2);
                glEnd();
            }
        }
    }
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

    if(!envmapping && smquery && hasOQ && oqfrags && oqlights) loopv(lightorder)
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
                startquery(l.query);
                ivec bo = bbmin, br = bbmax;
                br.sub(bo).add(1);
                drawbb(bo, br);
                endquery(l.query);
            }
        }
    }
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
    lighttilesused = 0;
    smused = 0;

    bool tetra = smtetra && glslversion >= 130;
    if(smcache && !smnoshadow && shadowcache.numelems) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.flags&L_NOSHADOW || l.radius <= smminradius) continue;
        if(l.query && l.query->owner == &l && checkquery(l.query)) continue;

        float prec = smprec, lod;
        int w, h;
        if(l.spot) { w = 1; h = 1; const vec2 &sc = sincos360[l.spot]; prec = sc.y/sc.x; lod = smspotprec; }
        else if(tetra) { w = 2; h = 1; lod = smtetraprec; }
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
            else if(tetra) { w = 2; h = 1; lod = smtetraprec; }
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

void radiancehints::renderslices()
{
    if(rhcache) loopi(3) swap(rhtex[i], rhtex[i+3]);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rhfbo);
    glViewport(0, 0, rhgrid+2*rhborder, rhgrid+2*rhborder);

    GLOBALPARAM(rhatten, (1.0f/(gidist*gidist)));
    GLOBALPARAM(rsmspread, (gidist*rsmspread*rsm.scale.x, gidist*rsmspread*rsm.scale.y));

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(rsm.offset.x, rsm.offset.y, rsm.offset.z);
    glScalef(rsm.scale.x, rsm.scale.y, rsm.scale.z);
    glMultMatrixf(rsm.model.v);
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rsmdepthtex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rsmcolortex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rsmnormaltex);
    loopi(rhcache ? 6 : 3)
    {
        glActiveTexture_(GL_TEXTURE3_ARB + i);
        glBindTexture(GL_TEXTURE_3D_EXT, rhtex[i]);
    }
    glActiveTexture_(GL_TEXTURE0_ARB);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    loopirev(rhsplits)
    {
        splitinfo &split = splits[i];

        float cellradius = split.bounds/rhgrid, step = 2*cellradius;
        GLOBALPARAM(rhspread, (cellradius));

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
            GLOBALPARAM(bordercenter, (0.5f, 0.5f, float(i+1 + 0.5f)/rhsplits));
            GLOBALPARAM(borderrange, (0.5f - 0.5f/(rhgrid+2), 0.5f - 0.5f/(rhgrid+2), (0.5f - 0.5f/(rhgrid+2))/rhsplits));
            GLOBALPARAM(borderscale, (rhgrid+2, rhgrid+2, (rhgrid+2)*rhsplits));
            splitinfo &next = splits[i+1];
            loopk(3)
            {
                bmin[k] = floor((max(float(worldmin[k]), next.center[k] - next.bounds) - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
                bmax[k] = ceil((min(float(worldmax[k]), next.center[k] + next.bounds) - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
            }
        }

        loopjrev(rhgrid+2*rhborder)
        {
            glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D_EXT, rhtex[0], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_3D_EXT, rhtex[1], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_3D_EXT, rhtex[2], 0, i*(rhgrid+2*rhborder) + j);

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

                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord3f(btx2, bty1, bz); glVertex2f(bvx2, bvy1);
                glTexCoord3f(btx1, bty1, bz); glVertex2f(bvx1, bvy1);
                glTexCoord3f(btx2, bty2, bz); glVertex2f(bvx2, bvy2);
                glTexCoord3f(btx1, bty2, bz); glVertex2f(bvx1, bvy2);
                glEnd();
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

                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord3f(px2, py1, z); glVertex2f(pvx2, pvy1);
                            glTexCoord3f(tx2, ty1, z); glVertex2f(vx2, vy1);
                        glTexCoord3f(px1, py1, z); glVertex2f(pvx1, pvy1);
                            glTexCoord3f(tx1, ty1, z); glVertex2f(vx1, vy1);
                        glTexCoord3f(px1, py2, z); glVertex2f(pvx1, pvy2);
                            glTexCoord3f(tx1, ty2, z); glVertex2f(vx1, vy2);
                        glTexCoord3f(px2, py2, z); glVertex2f(pvx2, pvy2);
                            glTexCoord3f(tx2, ty2, z); glVertex2f(vx2, vy2);
                        glTexCoord3f(px2, py1, z); glVertex2f(pvx2, pvy1);
                            glTexCoord3f(tx2, ty1, z); glVertex2f(vx2, vy1);
                        glEnd();
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

                                glBegin(GL_TRIANGLE_STRIP);
                                glTexCoord3f(dtx2, dty1, dz); glVertex2f(dvx2, dvy1);
                                    glTexCoord3f(ptx2, pty1, pz); glVertex2f(pvx2, pvy1);
                                glTexCoord3f(dtx1, dty1, dz); glVertex2f(dvx1, dvy1);
                                    glTexCoord3f(ptx1, pty1, pz); glVertex2f(pvx1, pvy1);
                                glTexCoord3f(dtx1, dty2, dz); glVertex2f(dvx1, dvy2);
                                    glTexCoord3f(ptx1, pty2, pz); glVertex2f(pvx1, pvy2);
                                glTexCoord3f(dtx2, dty2, dz); glVertex2f(dvx2, dvy2);
                                    glTexCoord3f(ptx2, pty2, pz); glVertex2f(pvx2, pvy2);
                                glTexCoord3f(dtx2, dty1, dz); glVertex2f(dvx2, dvy1);
                                    glTexCoord3f(ptx2, pty1, pz); glVertex2f(pvx2, pvy1);
                                glEnd();
                            }

                            radiancehintsshader->set();

                            glBegin(GL_TRIANGLE_STRIP);
                            glTexCoord3f(dx2, dy1, z); glVertex2f(dvx2, dvy1);
                            glTexCoord3f(dx1, dy1, z); glVertex2f(dvx1, dvy1);
                            glTexCoord3f(dx2, dy2, z); glVertex2f(dvx2, dvy2);
                            glTexCoord3f(dx1, dy2, z); glVertex2f(dvx1, dvy2);
                            glEnd();

                            continue;
                        }
                    }

                    SETSHADER(radiancehintscached);

                    glBegin(GL_TRIANGLE_STRIP);
                    glTexCoord3f(ptx2, pty1, pz); glVertex2f(pvx2, pvy1);
                    glTexCoord3f(ptx1, pty1, pz); glVertex2f(pvx1, pvy1);
                    glTexCoord3f(ptx2, pty2, pz); glVertex2f(pvx2, pvy2);
                    glTexCoord3f(ptx1, pty2, pz); glVertex2f(pvx1, pvy2);
                    glEnd();

                    continue;
                }
            }

            radiancehintsshader->set();

            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord3f(tx2, ty1, z); glVertex2f(vx2, vy1);
            glTexCoord3f(tx1, ty1, z); glVertex2f(vx1, vy1);
            glTexCoord3f(tx2, ty2, z); glVertex2f(vx2, vy2);
            glTexCoord3f(tx1, ty2, z); glVertex2f(vx1, vy2);
            glEnd();
        }
    }
}

VAR(rhinoq, 0, 0, 1);

void renderradiancehints()
{
    if(!sunlight || !csmshadowmap || !gi || !giscale || !gidist) return;

    timer *rhtimer = !rhinoq ? begintimer("radiance hints") : NULL;

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
    dynamicshadowvabounds(1<<shadowside, rh.dynmin, rh.dynmax);
    batcheddynamicmodelbounds(1<<shadowside, rh.dynmin, rh.dynmax);

    if(!rhcache || rh.prevdynmin.z < rh.prevdynmax.z || rh.dynmin.z < rh.dynmax.z || !rh.allcached())
    {
        if(rhinoq) glFlush();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(rsm.proj.v);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(rsm.model.v);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rsmfbo);

        GLOBALPARAM(rsmdir, (-rsm.lightview.x, -rsm.lightview.y, -rsm.lightview.z));

        glViewport(0, 0, rsmsize, rsmsize);
        glClearColor(0, 0, 0, 0);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

        renderrsmgeom();
        rendermodelbatches();

        rh.renderslices();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        if(rhinoq)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
            glViewport(0, 0, vieww, viewh);
        }
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

    shadowmaskbatchedmodels(!smnodynshadow);
    batchshadowmapmodels();

    loopi(csmsplits) if(csm.splits[i].idx >= 0)
    {
        const shadowmapinfo &sm = shadowmaps[csm.splits[i].idx];

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(csm.splits[i].proj.v);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(csm.model.v);
        glViewport(sm.x, sm.y, sm.size, sm.size);
        glScissor(sm.x, sm.y, sm.size, sm.size);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowside = i;

        rendershadowmapworld();
        rendermodelbatches();
    }

    clearbatchedmapmodels();

    if(polyfactor || polyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowmaps()
{
    bool tetra = smtetra && glslversion >= 130;
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
            if(shadowmapping == SM_TETRA && smtetraclip) glDisable(GL_CLIP_PLANE0);
            shadowmapping = SM_SPOT;
            border = 0;
            sidemask = 1;
        }
        else if(tetra)
        {
            if(shadowmapping != SM_TETRA && smtetraclip) glEnable(GL_CLIP_PLANE0);
            shadowmapping = SM_TETRA;
            border = smfilter > 2 ? smborder2 : smborder;
            sidemask = smsidecull ? cullfrustumtetra(l.o, l.radius, sm.size, border) : 0xF;
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

        shadowmaskbatchedmodels(!(l.flags&L_NODYNSHADOW) && !smnodynshadow);
        batchshadowmapmodels();

        shadowcacheval *cached = NULL;
        int cachemask = 0;
        if(smcache)
        {
            int dynmask = smcache <= 1 ? batcheddynamicmodels() : 0;
            if(shadowmapping == SM_TETRA && !smtetraclear)
            {
                if(dynmask&0x3) dynmask |= 0x3;
                if(dynmask&0xC) dynmask |= 0xC;
            }
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
        GLfloat smprojmatrix[16] =
        {
            float(sm.size - border) / sm.size, 0, 0, 0,
            0, float(sm.size - border) / sm.size, 0, 0,
            0, 0, -(smfarclip + smnearclip) / (smfarclip - smnearclip), -1,
            0, 0, -2*smnearclip*smfarclip / (smfarclip - smnearclip), 0
        };
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(smprojmatrix);
        glMatrixMode(GL_MODELVIEW);

        GLfloat lightmatrix[16] =
        {
            1.0f/l.radius, 0, 0, 0,
            0, 1.0f/l.radius, 0, 0,
            0, 0, 1.0f/l.radius, 0,
            -l.o.x/l.radius, -l.o.y/l.radius, -l.o.z/l.radius, 1
        };

        glmatrixf smviewmatrix;

        if(shadowmapping == SM_SPOT)
        {
            glViewport(sm.x, sm.y, sm.size, sm.size);
            glScissor(sm.x, sm.y, sm.size, sm.size);
            glClear(GL_DEPTH_BUFFER_BIT);

            GLfloat spotmatrix[16] =
            {
                l.spotx.x, l.spoty.x, -l.dir.x, 0,
                l.spotx.y, l.spoty.y, -l.dir.y, 0,
                l.spotx.z, l.spoty.z, -l.dir.z, 0,
                0, 0, 0, 1
            };
            smviewmatrix.mul(spotmatrix, lightmatrix);
            glLoadMatrixf(smviewmatrix.v);

            glCullFace((l.dir.scalartriple(l.spoty, l.spotx) < 0) == (smcullside != 0) ? GL_BACK : GL_FRONT);

            shadowside = 0;

            if(mesh) rendershadowmesh(mesh); else rendershadowmapworld();
            rendermodelbatches();
        }
        else if(shadowmapping == SM_TETRA)
        {
            if(!cachemask)
            {
                glScissor(sm.x + (sidemask & 0x3 ? 0 : sm.size), sm.y, sidemask & 0x3 && sidemask & 0xC ? 2*sm.size : sm.size, sm.size);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
            loop(side, 4) if(sidemask&(1<<side))
            {
                int sidex = (side>>1)*sm.size;
                if(!(side&1) || !(sidemask&(1<<(side-1))))
                {
                    glViewport(sm.x + sidex, sm.y, sm.size, sm.size);
                    glScissor(sm.x + sidex, sm.y, sm.size, sm.size);
                    if(cachemask && (!smtetraclear || (sidemask&(3<<(side&~1))) == (3<<(side&~1)))) glClear(GL_DEPTH_BUFFER_BIT);
                }

                if(cachemask && smtetraclear && (sidemask&(3<<(side&~1))) != (3<<(side&~1)))
                {
                    if(smtetraclip) glDisable(GL_CLIP_PLANE0);
                    if(smpolyfactor || smpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);
                    glDisable(GL_CULL_FACE);
                    glDepthFunc(GL_ALWAYS);

                    SETSHADER(tetraclear);
                    glBegin(GL_TRIANGLES);
                    switch(side)
                    {
                    case 0: glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1); glVertex3f(1, -1, 1); break;
                    case 1: glVertex3f(-1, -1, 1); glVertex3f(-1, 1, 1); glVertex3f(1, -1, 1); break;
                    case 2: glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1); glVertex3f(1, 1, 1); break;
                    case 3: glVertex3f(-1, 1, 1); glVertex3f(-1, -1, 1); glVertex3f(1, 1, 1); break;
                    }
                    glEnd();

                    glDepthFunc(GL_LESS);
                    glEnable(GL_CULL_FACE);
                    if(smpolyfactor || smpolyoffset) glEnable(GL_POLYGON_OFFSET_FILL);
                    if(smtetraclip) glEnable(GL_CLIP_PLANE0);
                }

                smviewmatrix.mul(tetrashadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side>>1) ^ smcullside ? GL_BACK : GL_FRONT);

                if(smtetraclip)
                {
                    smtetraclipplane.toplane(vec(-smviewmatrix.v[2], -smviewmatrix.v[6], 0), l.o);
                    smtetraclipplane.offset += smtetraborder/(0.5f*sm.size);
                    GLOBALPARAM(tetraclip, (smtetraclipplane));
                }

                shadowside = side;

                if(mesh) rendershadowmesh(mesh); else rendershadowmapworld();
                rendermodelbatches();
            }
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

                smviewmatrix.mul(cubeshadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side & 1) ^ (side >> 2) ^ smcullside ? GL_FRONT : GL_BACK);

                shadowside = side;

                if(mesh) rendershadowmesh(mesh); else rendershadowmapworld();
                rendermodelbatches();
            }
        }

        clearbatchedmapmodels();
    }

    if(shadowmapping == SM_TETRA && smtetraclip) glDisable(GL_CLIP_PLANE0);
    if(polyfactor || polyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowatlas()
{
    timer *smcputimer = begintimer("shadow map", false);
    timer *smtimer = begintimer("shadow map");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    if(debugshadowatlas)
    {
        glClearDepth(0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearDepth(1);
    }

    glEnable(GL_SCISSOR_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // sun light
    if(sunlight && csmshadowmap)
    {
        csm.setup();
        rendercsmshadowmaps();
    }

    packlights();

    // point lights
    rendershadowmaps();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

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
    if(!hasalphavas && !hasmats) return;

    timer *transtimer = begintimer("transparent");

    if(hasalphavas&4 || hasmats&4)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, refractfbo);
        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
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
        SETSHADER(refractmask);
        LOCALPARAM(refractdepth, (1.0f/refractdepth));
        if(hasalphavas&4) renderrefractmask();
        if(hasmats&4) rendermaterialmask();

        glDepthMask(GL_TRUE);
    }

    glActiveTexture_(GL_TEXTURE7_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, refracttex);
    glActiveTexture_(GL_TEXTURE8_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
    glActiveTexture_(GL_TEXTURE9_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    if((gdepthstencil && hasDS) || gstencil) glEnable(GL_STENCIL_TEST);

    glmatrixf raymatrix = mvmatrix.v;
    loopk(4)
    {
        raymatrix.v[0 + k*4] = 0.5f*vieww*(raymatrix.v[2 + k*4] - raymatrix.v[0 + k*4]*projmatrix.v[0]);
        raymatrix.v[1 + k*4] = 0.5f*viewh*(raymatrix.v[2 + k*4] - raymatrix.v[1 + k*4]*projmatrix.v[5]);
    }

    loop(layer, 3)
    {
        uint tiles[LIGHTTILE_MAXH];
        float sx1, sy1, sx2, sy2;
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
        default:
            continue;
        }

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
        if((gdepthstencil && hasDS) || gstencil)
        {
            glStencilFunc(GL_ALWAYS, 1<<layer, 1<<layer);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            bool aamask = maskedaa();
            maskgbuffer(aamask ? "cn" : "cng");
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
            if(aamask)
            {
                maskgbuffer("g");
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
                glClear(GL_COLOR_BUFFER_BIT);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            } 
            if(scissor) glDisable(GL_SCISSOR_TEST);
            maskgbuffer("cngd");
        }

        extern int wireframe;
        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(linearworldmatrix.v);
        glActiveTexture_(GL_TEXTURE1_ARB);
        glLoadMatrixf(raymatrix.v);
        glActiveTexture_(GL_TEXTURE0_ARB);

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
        }

        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if(inferredprelightshader && ((gdepthstencil && hasDS) || gstencil) && infertransparent)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, infbo);
            glViewport(0, 0, (vieww+1)/2, (viewh+1)/2);

            glDisable(GL_STENCIL_TEST);

            if(inferclear)
            {
                bool scissor = sx1 > -1 || sy1 > -1 || sx2 < 1 || sy2 < 1;
                if(scissor)
                {
                    int x1 = int(floor((sx1*0.5f+0.5f)*vieww)), y1 = int(floor((sy1*0.5f+0.5f)*viewh)),
                        x2 = int(ceil((sx2*0.5f+0.5f)*vieww)), y2 = int(ceil((sy2*0.5f+0.5f)*viewh));
                    x1 /= 2; y1 /= 2; x2 = (x2+1)/2; y2 = (y2+1)/2;
                    glEnable(GL_SCISSOR_TEST);
                    glScissor(x1, y1, x2 - x1, y2 - y1);
                }
                glClearColor(0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);
                if(scissor) glDisable(GL_SCISSOR_TEST);
            }

            renderlights(2, sx1, sy1, sx2, sy2, tiles, 1<<layer);

            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
            glViewport(0, 0, vieww, viewh);

            glEnable(GL_STENCIL_TEST);
            renderlights(1, sx1, sy1, sx2, sy2, tiles, 1<<layer);
        }
        else
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
            if((gdepthstencil && hasDS) || gstencil)
            {
                glStencilFunc(GL_EQUAL, 1<<layer, 1<<layer);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            }

            renderlights(0, sx1, sy1, sx2, sy2, tiles);
        }
    }

    if((gdepthstencil && hasDS) || gstencil) glDisable(GL_STENCIL_TEST);

    endtimer(transtimer);
}

VAR(gdepthclear, 0, 1, 1);
VAR(gcolorclear, 0, 1, 1);

void preparegbuffer()
{
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
    glViewport(0, 0, vieww, viewh);

    if(gdepthformat && gdepthclear)
    {
        maskgbuffer("d");
        if(gdepthformat == 1) glClearColor(1, 1, 1, 1);
        else glClearColor(-farplane, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        maskgbuffer("cng");
    }
    if(gcolorclear) glClearColor(0, 0, 0, 0);
    glClear((minimapping < 2 ? GL_DEPTH_BUFFER_BIT : 0)|(gcolorclear ? GL_COLOR_BUFFER_BIT : 0)|(minimapping < 2 && ((gdepthstencil && hasDS) || gstencil) ? GL_STENCIL_BUFFER_BIT : 0));
    if(gdepthformat && gdepthclear) maskgbuffer("cngd");

    glmatrixf invscreenmatrix;
    invscreenmatrix.identity();
    invscreenmatrix.scale(2.0f/vieww, 2.0f/viewh, 2.0f);
    invscreenmatrix.translate(-1.0f, -1.0f, -1.0f);
    eyematrix.mul(invprojmatrix, invscreenmatrix);
    if(minimapping)
    {
        linearworldmatrix.mul(invmvpmatrix, invscreenmatrix);
        worldmatrix = linearworldmatrix;
    }
    else
    {
        linearworldmatrix.mul(invprojmatrix, invscreenmatrix);
        float xscale = linearworldmatrix.v[0], yscale = linearworldmatrix.v[5], xoffset = linearworldmatrix.v[12], yoffset = linearworldmatrix.v[13], zscale = linearworldmatrix.v[14];
        float depthmatrix[16] =
        {
            xscale/zscale,  0,              0, 0,
            0,              yscale/zscale,  0, 0,
            xoffset/zscale, yoffset/zscale, 1, 0,
            0,              0,              0, 1
        };
        linearworldmatrix.mul(invmvmatrix.v, depthmatrix);
        if(gdepthformat) worldmatrix = linearworldmatrix;
        else worldmatrix.mul(invmvpmatrix, invscreenmatrix);
    }

    screenmatrix.identity();
    screenmatrix.scale(0.5f*vieww, 0.5f*viewh, 0.5f);
    screenmatrix.translate(0.5f*vieww, 0.5f*viewh, 0.5f);
    screenmatrix.mul(mvpmatrix);

    GLOBALPARAM(viewsize, (vieww, viewh, 1.0f/vieww, 1.0f/viewh));
    GLOBALPARAM(gdepthscale, (eyematrix.v[14], eyematrix.v[11], eyematrix.v[15]));
    GLOBALPARAM(gdepthpackparams, (-1.0f/farplane, -255.0f/farplane, -(255.0f*255.0f)/farplane));
    GLOBALPARAM(gdepthunpackparams, (-farplane, -farplane/255.0f, -farplane/(255.0f*255.0f)));

    GLERROR;
}

void rendergbuffer()
{
    timer *gcputimer = envmapping ? NULL : begintimer("g-buffer", false);
    timer *gtimer = envmapping ? NULL : begintimer("g-buffer");

    preparegbuffer();

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

    if(minimapping) 
    {
        renderminimapmaterials();
    }
    else if(!envmapping)
    {
        game::rendergame();
        rendermodelbatches();
        GLERROR;
        maskgbuffer("c");
        renderdecals(true);
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

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    glViewport(0, 0, vieww, viewh);

    if(minimapping == 1)
    {
        glClearColor(color.x, color.y, color.z, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    renderlights();
    GLERROR;
}

void shademodelpreview(int x, int y, int w, int h, bool background, bool scissor)
{
    GLERROR;

    GLuint outfbo = w > vieww || h > viewh ? scalefbo : 0;
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
    glViewport(outfbo ? 0 : x, outfbo ? 0 : y, vieww, viewh);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(worldmatrix.v);
    glMatrixMode(GL_MODELVIEW);

    float lightscale = 2.0f*ldrscale;
    GLOBALPARAM(lightscale, (0.1f*lightscale, 0.1f*lightscale, 0.1f*lightscale, lightscale));
    GLOBALPARAM(sunlightdir, (vec(0, -1, 2).normalize()));
    GLOBALPARAM(sunlightcolor, (0.6f*lightscale, 0.6f*lightscale, 0.6f*lightscale));

    SETSHADER(modelpreview);

    if(background || outfbo)
    {
        if(!outfbo && scissor) glEnable(GL_SCISSOR_TEST);
        screenquad(vieww, viewh);
        if(!outfbo && scissor) glDisable(GL_SCISSOR_TEST);

        if(outfbo)
        {
           glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0); 
           glViewport(x, y, w, h);
           glBindTexture(GL_TEXTURE_RECTANGLE_ARB, scaletex);
           SETSHADER(scale);
        }
    }
    
    if(!background)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        if(scissor) glEnable(GL_SCISSOR_TEST);
        screenquad(vieww, viewh);
        if(scissor) glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    GLERROR;
    
    glViewport(0, 0, screen->w, screen->h);
}
 
void shadegbuffer()
{
    GLERROR;

    bool shouldinfer = inferredprelightshader && ((gdepthstencil && hasDS) || gstencil);
    if(shouldinfer)
    {
        timer *intimer = begintimer("inferred lighting");

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, infbo);
        glViewport(0, 0, (vieww+1)/2, (viewh+1)/2);

        if(inferclear)
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | (inferdepth ? GL_DEPTH_BUFFER_BIT : 0));
        }

        if(inferdepth)
        {
            if(!inferclear) glClear(GL_DEPTH_BUFFER_BIT);

            extern void renderinferdepth();
            renderinferdepth();
        }

        renderlights(2);

        endtimer(intimer);
    }

    GLERROR;
    timer *shcputimer = begintimer("deferred shading", false);
    timer *shtimer = begintimer("deferred shading");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    glViewport(0, 0, vieww, viewh);

    if(hdrclear > 0)
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        hdrclear--;
    }
    drawskybox(farplane);

    renderlights(shouldinfer ? 1 : 0);
    GLERROR;

    endtimer(shtimer);
    endtimer(shcputimer);
}

void setupframe(int w, int h)
{
    setupgbuffer(w, h);
    if(hdr && (bloomw < 0 || bloomh < 0)) setupbloom(gw, gh);
    if(ao && (aow < 0 || aoh < 0)) setupao(gw, gh);
    if(inferlights && ((gdepthstencil && hasDS) || gstencil) && !infbo) setupinferred(gw, gh);
    if(!shadowatlasfbo) setupshadowatlas();
    if(sunlight && csmshadowmap && gi && giscale && gidist && !rhfbo) setupradiancehints();
    if(!deferredlightshader) loaddeferredlightshaders();
    if(minimapping && !deferredminimapshader) deferredminimapshader = loaddeferredlightshader("m");
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
    else if(debuginferred) viewinferred();
    else if(!debugaa()) return false;
    return true;
}

void cleanuplights()
{
    cleanupgbuffer();
    cleanupbloom();
    cleanupao();
    cleanupinferred();
    cleanupshadowatlas();
    cleanupradiancehints();
    cleanuplightsphere();
    cleanupaa();
}

