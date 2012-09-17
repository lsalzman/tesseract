#include "engine.h"

GLuint fxaafbo = 0;

extern int fxaaquality;

static Shader *fxaashader = NULL;

void loadfxaashaders()
{
    loadhdrshaders(true);
    defformatstring(fxaaname)("fxaa%d", fxaaquality);
    fxaashader = generateshader(fxaaname, "fxaashaders %d", fxaaquality);
}

void clearfxaashaders()
{
    fxaashader = NULL;
}

void setupfxaa(int w, int h)
{
    if(!fxaafbo) glGenFramebuffers_(1, &fxaafbo);
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, fxaafbo);
    GLuint tex = gcolortex;
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, tex, 0);
    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating FXAA buffer!");
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadfxaashaders();
}

void cleanupfxaa()
{
    if(fxaafbo) { glDeleteFramebuffers_(1, &fxaafbo); fxaafbo = 0; }

    clearfxaashaders();
}

VARFP(fxaa, 0, 0, 1, cleanupfxaa());
VARFP(fxaaquality, 0, 1, 3, cleanupfxaa());

void dofxaa(GLuint outfbo = 0)
{
    timer *fxaatimer = begintimer("fxaa");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
    fxaashader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    endtimer(fxaatimer);
}

GLuint smaaareatex = 0, smaasearchtex = 0, smaafbo[3] = { 0, 0, 0 };

extern int smaaquality;

static Shader *smaalumaedgeshader = NULL, *smaacoloredgeshader = NULL, *smaablendweightshader = NULL, *smaaneighborhoodshader = NULL;

void loadsmaashaders()
{
    loadhdrshaders(true);
    defformatstring(lumaedgename)("SMAALumaEdgeDetection%d", smaaquality);
    defformatstring(coloredgename)("SMAAColorEdgeDetection%d", smaaquality);
    defformatstring(blendweightname)("SMAABlendingWeightCalculation%d", smaaquality);
    defformatstring(neighborhoodname)("SMAANeighborhoodBlending%d", smaaquality);
    smaalumaedgeshader = lookupshaderbyname(lumaedgename);
    smaacoloredgeshader = lookupshaderbyname(coloredgename);
    smaablendweightshader = lookupshaderbyname(blendweightname);
    smaaneighborhoodshader = lookupshaderbyname(neighborhoodname);
    if(smaalumaedgeshader && smaacoloredgeshader && smaablendweightshader && smaaneighborhoodshader) return;
    generateshader(NULL, "smaashaders %d", smaaquality);
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

#include "AreaTex.h"
#include "SearchTex.h"

void setupsmaa(int w, int h)
{
    if(!smaaareatex) glGenTextures(1, &smaaareatex);
    if(!smaasearchtex) glGenTextures(1, &smaasearchtex);
    createtexture(smaaareatex, AREATEX_WIDTH, AREATEX_HEIGHT, areaTexBytes, 3, 1, GL_LUMINANCE_ALPHA, GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, false);
    createtexture(smaasearchtex, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, searchTexBytes, 3, 0, GL_LUMINANCE, GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, false);
    loopi(3)
    {
        if(!smaafbo[i]) glGenFramebuffers_(1, &smaafbo[i]);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[i]);
        GLuint tex = 0;
        switch(i)
        {
            case 0: tex = gcolortex; break;
            case 1: tex = gnormaltex; break;
            case 2: tex = gglowtex; break;
        }
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, tex, 0);
        if(i > 0)
        {
            if(gdepthformat)
            {
                glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
                if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
            }
            else
            {
                glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
                if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
                else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
            }
        }
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating SMAA buffer!");
    }
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadsmaashaders();
}

void cleanupsmaa()
{
    if(smaaareatex) { glDeleteTextures(1, &smaaareatex); smaaareatex = 0; }
    if(smaasearchtex) { glDeleteTextures(1, &smaasearchtex); smaasearchtex = 0; }
    loopi(3) if(smaafbo[i]) { glDeleteFramebuffers_(1, &smaafbo[i]); smaafbo[i] = 0; }

    clearsmaashaders();
}

VARFP(smaa, 0, 0, 1, cleanupsmaa());
VARFP(smaaquality, 0, 2, 3, cleanupsmaa());
VARP(smaacoloredge, 0, 0, 1);
VAR(smaadepthmask, 0, 1, 1);
VAR(smaastencil, 0, 1, 1);
VAR(debugsmaa, 0, 0, 5);

void viewsmaa()
{
    int w = min(screen->w, screen->h)*1.0f, h = (w*screen->h)/screen->w, tw = gw, th = gh;
    rectshader->set();
    glColor3f(1, 1, 1);
    switch(debugsmaa)
    {
        case 1: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex); break;
        case 2: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex); break;
        case 3: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex); break;
        case 4: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaaareatex); tw = AREATEX_WIDTH; th = AREATEX_HEIGHT; break;
        case 5: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaasearchtex); tw = SEARCHTEX_WIDTH; th = SEARCHTEX_HEIGHT; break;

    }
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, th); glVertex2i(0, 0);
    glTexCoord2f(tw, th); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(tw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

void dosmaa(GLuint outfbo = 0)
{
    timer *smaatimer = begintimer("smaa");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if(smaadepthmask)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthRange(1, 1);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil))
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 8, 8);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    if(smaacoloredge) smaacoloredgeshader->set();
    else smaalumaedgeshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    screenquad(vieww, viewh);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[2]);
    if(smaadepthmask)
    {
        glDepthFunc(GL_EQUAL);
        glDepthMask(GL_FALSE);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil))
    {
        glStencilFunc(GL_EQUAL, 8, 8);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    smaablendweightshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaaareatex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaasearchtex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if(smaadepthmask)
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDepthRange(0, 1);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil)) glDisable(GL_STENCIL_TEST);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
    smaaneighborhoodshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    endtimer(smaatimer);
}

void setupaa(int w, int h)
{
    if(smaa) { if(!smaafbo[0]) setupsmaa(w, h); }
    else if(fxaa) { if(!fxaafbo) setupfxaa(w, h); }
}

void doaa(GLuint outfbo, void (*resolve)(GLuint, bool))
{
    if(smaa) { resolve(smaafbo[0], true); dosmaa(outfbo); }
    else if(fxaa) { resolve(fxaafbo, true); dofxaa(outfbo); }
    else resolve(outfbo, false);
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
}

