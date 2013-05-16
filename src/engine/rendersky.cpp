#include "engine.h"

Texture *sky[6] = { 0, 0, 0, 0, 0, 0 }, *clouds[6] = { 0, 0, 0, 0, 0, 0 };

void loadsky(const char *basename, Texture *texs[6])
{
    const char *wildcard = strchr(basename, '*');
    loopi(6)
    {
        const char *side = cubemapsides[i].name;
        string name;
        copystring(name, makerelpath("packages", basename));
        if(wildcard)
        {
            char *chop = strchr(name, '*');
            if(chop) { *chop = '\0'; concatstring(name, side); concatstring(name, wildcard+1); }
            texs[i] = textureload(name, 3, true, false); 
        }
        else
        {
            defformatstring(ext)("_%s.jpg", side);
            concatstring(name, ext);
            if((texs[i] = textureload(name, 3, true, false))==notexture)
            {
                strcpy(name+strlen(name)-3, "png");
                texs[i] = textureload(name, 3, true, false);
            }
        }
        if(texs[i]==notexture) conoutf(CON_ERROR, "could not load side %s of sky texture %s", side, basename);
    }
}

Texture *cloudoverlay = NULL;

Texture *loadskyoverlay(const char *basename)
{
    const char *ext = strrchr(basename, '.'); 
    string name;
    copystring(name, makerelpath("packages", basename));
    Texture *t = notexture;
    if(ext) t = textureload(name, 0, true, false);
    else
    {
        concatstring(name, ".jpg");
        if((t = textureload(name, 0, true, false)) == notexture)
        {
            strcpy(name+strlen(name)-3, "png");
            t = textureload(name, 0, true, false);
        }
    }
    if(t==notexture) conoutf(CON_ERROR, "could not load sky overlay texture %s", basename);
    return t;
}

SVARFR(skybox, "", { if(skybox[0]) loadsky(skybox, sky); }); 
HVARR(skyboxcolour, 0, 0xFFFFFF, 0xFFFFFF);
FVARR(skyboxoverbright, 1, 2, 16);
FVARR(skyboxoverbrightmin, 0, 1, 16);
FVARR(skyboxoverbrightthreshold, 0, 0.7f, 1);
FVARR(spinsky, -720, 0, 720);
VARR(yawsky, 0, 0, 360);
SVARFR(cloudbox, "", { if(cloudbox[0]) loadsky(cloudbox, clouds); });
HVARR(cloudboxcolour, 0, 0xFFFFFF, 0xFFFFFF);
FVARR(cloudboxalpha, 0, 1, 1);
FVARR(spinclouds, -720, 0, 720);
VARR(yawclouds, 0, 0, 360);
FVARR(cloudclip, 0, 0.5f, 1);
SVARFR(cloudlayer, "", { if(cloudlayer[0]) cloudoverlay = loadskyoverlay(cloudlayer); });
FVARR(cloudoffsetx, 0, 0, 1);
FVARR(cloudoffsety, 0, 0, 1);
FVARR(cloudscrollx, -16, 0, 16);
FVARR(cloudscrolly, -16, 0, 16);
FVARR(cloudscale, 0.001, 1, 64);
FVARR(spincloudlayer, -720, 0, 720);
VARR(yawcloudlayer, 0, 0, 360);
FVARR(cloudheight, -1, 0.2f, 1);
FVARR(cloudfade, 0, 0.2f, 1);
FVARR(cloudalpha, 0, 1, 1);
VARR(cloudsubdiv, 4, 16, 64);
HVARR(cloudcolour, 0, 0xFFFFFF, 0xFFFFFF);

void draw_envbox_face(float s0, float t0, int x0, int y0, int z0,
                      float s1, float t1, int x1, int y1, int z1,
                      float s2, float t2, int x2, int y2, int z2,
                      float s3, float t3, int x3, int y3, int z3,
                      GLuint texture)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(x3, y3, z3); gle::attribf(s3, t3);
    gle::attribf(x2, y2, z2); gle::attribf(s2, t2);
    gle::attribf(x0, y0, z0); gle::attribf(s0, t0);
    gle::attribf(x1, y1, z1); gle::attribf(s1, t1);
    xtraverts += gle::end();
}

void draw_envbox(int w, float z1clip = 0.0f, float z2clip = 1.0f, int faces = 0x3F, Texture **sky = NULL)
{
    if(z1clip >= z2clip) return;

    float v1 = 1-z1clip, v2 = 1-z2clip;
    int z1 = int(ceil(2*w*(z1clip-0.5f))), z2 = int(ceil(2*w*(z2clip-0.5f)));

    gle::defvertex();
    gle::deftexcoord0();

    if(faces&0x01)
        draw_envbox_face(0.0f, v2,  -w, -w, z2,
                         1.0f, v2,  -w,  w, z2,
                         1.0f, v1,  -w,  w, z1,
                         0.0f, v1,  -w, -w, z1, sky[0] ? sky[0]->id : notexture->id);

    if(faces&0x02)
        draw_envbox_face(1.0f, v1, w, -w, z1,
                         0.0f, v1, w,  w, z1,
                         0.0f, v2, w,  w, z2,
                         1.0f, v2, w, -w, z2, sky[1] ? sky[1]->id : notexture->id);

    if(faces&0x04)
        draw_envbox_face(1.0f, v1, -w, -w, z1,
                         0.0f, v1,  w, -w, z1,
                         0.0f, v2,  w, -w, z2,
                         1.0f, v2, -w, -w, z2, sky[2] ? sky[2]->id : notexture->id);

    if(faces&0x08)
        draw_envbox_face(1.0f, v1,  w,  w, z1,
                         0.0f, v1, -w,  w, z1,
                         0.0f, v2, -w,  w, z2,
                         1.0f, v2,  w,  w, z2, sky[3] ? sky[3]->id : notexture->id);

    if(z1clip <= 0 && faces&0x10)
        draw_envbox_face(0.0f, 1.0f, -w,  w,  -w,
                         0.0f, 0.0f,  w,  w,  -w,
                         1.0f, 0.0f,  w, -w,  -w,
                         1.0f, 1.0f, -w, -w,  -w, sky[4] ? sky[4]->id : notexture->id);

    if(z2clip >= 1 && faces&0x20)
        draw_envbox_face(0.0f, 1.0f,  w,  w, w,
                         0.0f, 0.0f, -w,  w, w,
                         1.0f, 0.0f, -w, -w, w,
                         1.0f, 1.0f,  w, -w, w, sky[5] ? sky[5]->id : notexture->id);

    gle::disable();
}

void draw_env_overlay(int w, Texture *overlay = NULL, float tx = 0, float ty = 0)
{
    float z = w*cloudheight, tsz = 0.5f*(1-cloudfade)/cloudscale, psz = w*(1-cloudfade);
    glBindTexture(GL_TEXTURE_2D, overlay ? overlay->id : notexture->id);
    vec color = vec::hexcolor(cloudcolour);
    gle::color(color, cloudalpha);
    gle::defvertex();
    gle::deftexcoord0();
    gle::begin(GL_TRIANGLE_FAN);
    loopi(cloudsubdiv+1)
    {
        vec p(1, 1, 0);
        p.rotate_around_z((-2.0f*M_PI*i)/cloudsubdiv);
        gle::attribf(p.x*psz, p.y*psz, z); 
            gle::attribf(tx + p.x*tsz, ty + p.y*tsz);
    }
    xtraverts += gle::end();
    float tsz2 = 0.5f/cloudscale;
    gle::defvertex();
    gle::deftexcoord0();
    gle::defcolor(4);
    gle::begin(GL_TRIANGLE_STRIP);
    loopi(cloudsubdiv+1)
    {
        vec p(1, 1, 0);
        p.rotate_around_z((-2.0f*M_PI*i)/cloudsubdiv);
        gle::attribf(p.x*psz, p.y*psz, z);
            gle::attribf(tx + p.x*tsz, ty + p.y*tsz);
            gle::attribf(color.r, color.g, color.b, cloudalpha);
        gle::attribf(p.x*w, p.y*w, z);
            gle::attribf(tx + p.x*tsz2, ty + p.y*tsz2);
            gle::attribf(color.r, color.g, color.b, 0);
    }
    xtraverts += gle::end();
    gle::disable();
}

FVARR(fogdomeheight, -1, -0.5f, 1);
FVARR(fogdomemin, 0, 0, 1);
FVARR(fogdomemax, 0, 0, 1);
VARR(fogdomecap, 0, 1, 1);
FVARR(fogdomeclip, 0, 1, 1);
bvec fogdomecolor(0, 0, 0);
HVARFR(fogdomecolour, 0, 0, 0xFFFFFF,
{
    fogdomecolor = bvec((fogdomecolour>>16)&0xFF, (fogdomecolour>>8)&0xFF, fogdomecolour&0xFF);
});
VARR(fogdomeclouds, 0, 1, 1);

namespace dome
{
    struct vert
    {
        vec pos;
        uchar color[4];

	    vert() {}
	    vert(const vec &pos, const bvec &fcolor, float alpha) : pos(pos)
	    {
		    memcpy(color, fcolor.v, 3);
		    color[3] = uchar(alpha*255);
	    }
	    vert(const vert &v0, const vert &v1) : pos(vec(v0.pos).add(v1.pos).normalize())
	    {
            memcpy(color, v0.color, 4);
            if(v0.pos.z != v1.pos.z) color[3] += uchar((v1.color[3] - v0.color[3]) * (pos.z - v0.pos.z) / (v1.pos.z - v0.pos.z));
	    }
    } *verts = NULL;
    GLushort *indices = NULL;
    int numverts = 0, numindices = 0, capindices = 0;
    GLuint vbuf = 0, ebuf = 0;
    bvec lastcolor(0, 0, 0);
    float lastminalpha = 0, lastmaxalpha = 0, lastcapsize = -1, lastclipz = 1;

    void subdivide(int depth, int face);

    void genface(int depth, int i1, int i2, int i3)
    {
        int face = numindices; numindices += 3;
        indices[face]   = i3;
        indices[face+1] = i2;
        indices[face+2] = i1;
        subdivide(depth, face);
    }

    void subdivide(int depth, int face)
    {
        if(depth-- <= 0) return;
        int idx[6];
        loopi(3) idx[i] = indices[face+2-i];
        loopi(3)
        {
            int curvert = numverts++;
            verts[curvert] = vert(verts[idx[i]], verts[idx[(i+1)%3]]); //push on to unit sphere
            idx[3+i] = curvert;
            indices[face+2-i] = curvert;
        }
        subdivide(depth, face);
        loopi(3) genface(depth, idx[i], idx[3+i], idx[3+(i+2)%3]);
    }

    static inline int sortcap(GLushort x, GLushort y)
    {
        const vec &xv = verts[x].pos, &yv = verts[y].pos;
        return xv.y < 0 ? yv.y >= 0 || xv.x < yv.x : yv.y >= 0 && xv.x > yv.x;
    }

    static void init(const bvec &color, float minalpha = 0.0f, float maxalpha = 1.0f, float capsize = -1, float clipz = 1, int hres = 16, int depth = 2)
    {
        const int tris = hres << (2*depth);
        numverts = numindices = capindices = 0;
        verts = new vert[tris+1 + (capsize >= 0 ? 1 : 0)];
        indices = new GLushort[(tris + (capsize >= 0 ? hres<<depth : 0))*3];
        if(clipz >= 1)
        {
            verts[numverts++] = vert(vec(0.0f, 0.0f, 1.0f), color, minalpha); //build initial 'hres' sided pyramid
            loopi(hres) verts[numverts++] = vert(vec(sincos360[(360*i)/hres], 0.0f), color, maxalpha);
            loopi(hres) genface(depth, 0, i+1, 1+(i+1)%hres);
        }
        else if(clipz <= 0)
        {
            loopi(hres<<depth) verts[numverts++] = vert(vec(sincos360[(360*i)/(hres<<depth)], 0.0f), color, maxalpha);
        }
        else
        {
            float clipxy = sqrtf(1 - clipz*clipz);
            const vec2 &scm = sincos360[180/hres];
            loopi(hres)
            {
                const vec2 &sc = sincos360[(360*i)/hres];
                verts[numverts++] = vert(vec(sc.x*clipxy, sc.y*clipxy, clipz), color, minalpha);
                verts[numverts++] = vert(vec(sc.x, sc.y, 0.0f), color, maxalpha);
                verts[numverts++] = vert(vec(sc.x*scm.x - sc.y*scm.y, sc.y*scm.x + sc.x*scm.y, 0.0f), color, maxalpha);
            }
            loopi(hres)
            {
                genface(depth-1, 3*i, 3*i+1, 3*i+2);
                genface(depth-1, 3*i, 3*i+2, 3*((i+1)%hres));
                genface(depth-1, 3*i+2, 3*((i+1)%hres)+1, 3*((i+1)%hres));
            }
        }

        if(capsize >= 0)
        {
            GLushort *cap = &indices[numindices];
            int capverts = 0;
            loopi(numverts) if(!verts[i].pos.z) cap[capverts++] = i;
            verts[numverts++] = vert(vec(0.0f, 0.0f, -capsize), color, maxalpha);
            quicksort(cap, capverts, sortcap); 
            loopi(capverts)
            {
                int n = capverts-1-i;
                cap[n*3] = cap[n];
                cap[n*3+1] = cap[(n+1)%capverts];
                cap[n*3+2] = numverts-1;
                capindices += 3;
            }
        }

        if(!vbuf) glGenBuffers_(1, &vbuf);
        glBindBuffer_(GL_ARRAY_BUFFER, vbuf);
        glBufferData_(GL_ARRAY_BUFFER, numverts*sizeof(vert), verts, GL_STATIC_DRAW);
        DELETEA(verts);

        if(!ebuf) glGenBuffers_(1, &ebuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebuf);
        glBufferData_(GL_ELEMENT_ARRAY_BUFFER, (numindices + capindices)*sizeof(GLushort), indices, GL_STATIC_DRAW);
        DELETEA(indices);
    }

    void cleanup()
    {
	    numverts = numindices = 0;
        if(vbuf) { glDeleteBuffers_(1, &vbuf); vbuf = 0; }
        if(ebuf) { glDeleteBuffers_(1, &ebuf); ebuf = 0; }
    }

    void draw()
    {
        float capsize = fogdomecap && fogdomeheight < 1 ? (1 + fogdomeheight) / (1 - fogdomeheight) : -1;
        bvec color = fogdomecolour ? fogdomecolor : fogcolor;
        if(!numverts || lastcolor != color || lastminalpha != fogdomemin || lastmaxalpha != fogdomemax || lastcapsize != capsize || lastclipz != fogdomeclip) 
        {
            init(color, min(fogdomemin, fogdomemax), fogdomemax, capsize, fogdomeclip);
            lastcolor = color;
            lastminalpha = fogdomemin;
            lastmaxalpha = fogdomemax;
            lastcapsize = capsize;
            lastclipz = fogdomeclip;
        }

        glBindBuffer_(GL_ARRAY_BUFFER, vbuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebuf);

        gle::vertexpointer(sizeof(vert), &verts->pos);
        gle::colorpointer(sizeof(vert), &verts->color);
        gle::enablevertex();
        gle::enablecolor();

        glDrawRangeElements_(GL_TRIANGLES, 0, numverts-1, numindices + fogdomecap*capindices, GL_UNSIGNED_SHORT, indices);
        xtraverts += numverts;
        glde++;

        gle::disablevertex();
        gle::disablecolor();

        glBindBuffer_(GL_ARRAY_BUFFER, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void cleanupsky()
{
    dome::cleanup();
}

static void drawfogdome(int farplane)
{
    SETSHADER(skyfog);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glmatrix skymatrix = cammatrix, skyprojmatrix;
    skymatrix.d = vec4(0, 0, 0, 1);
    skymatrix.translate(0, 0, farplane*fogdomeheight*0.5f);
    skymatrix.scale(farplane/2, farplane/2, farplane*(0.5f - fogdomeheight*0.5f)); 
    skyprojmatrix.mul(projmatrix, skymatrix);
    LOCALPARAM(skymatrix, skyprojmatrix);

    dome::draw();

    glDisable(GL_BLEND);
}

VAR(showsky, 0, 1, 1);
VAR(clampsky, 0, 1, 1);
VARNR(skytexture, useskytexture, 0, 0, 1);
VARFR(skyshadow, 0, 0, 1, clearshadowcache());

int explicitsky = 0;

bool limitsky()
{
    return explicitsky && (useskytexture || editmode);
}

void drawskybox(int farplane)
{
    float skyclip = 0, topclip = 1;
    if(skyclip) skyclip = 0.5f + 0.5f*(skyclip-camera1->o.z)/float(worldsize); 

    if(limitsky())
    {
        glDisable(GL_DEPTH_TEST);
    }
    else
    {
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
    }
   
    if(ldrscale < 1 && (skyboxoverbrightmin != 1 || (skyboxoverbright > 1 && skyboxoverbrightthreshold < 1)))
    {
        SETSHADER(skyboxoverbright);
        LOCALPARAMF(overbrightparams, (skyboxoverbrightmin, max(skyboxoverbright, skyboxoverbrightmin), skyboxoverbrightthreshold));
    }
    else SETSHADER(skybox);

    if(clampsky) glDepthRange(1, 1);

    gle::color(vec::hexcolor(skyboxcolour));

    glmatrix skymatrix = cammatrix, skyprojmatrix;
    skymatrix.d = vec4(0, 0, 0, 1);
    skymatrix.rotate_around_z((spinsky*lastmillis/1000.0f+yawsky)*-RAD);
    skyprojmatrix.mul(projmatrix, skymatrix);
    LOCALPARAM(skymatrix, skyprojmatrix);

    draw_envbox(farplane/2, skyclip, topclip, 0x3F, sky);

    if(fogdomemax && !fogdomeclouds) 
    {
        drawfogdome(farplane);
    }
   
    SETSHADER(skybox); 

    if(cloudbox[0])
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gle::color(vec::hexcolor(cloudboxcolour), cloudboxalpha);

        skymatrix = cammatrix;
        skymatrix.d = vec4(0, 0, 0, 1);
        skymatrix.rotate_around_z((spinclouds*lastmillis/1000.0f+yawclouds)*-RAD);
        skyprojmatrix.mul(projmatrix, skymatrix);
        LOCALPARAM(skymatrix, skyprojmatrix);

        draw_envbox(farplane/2, skyclip ? skyclip : cloudclip, topclip, 0x3F, clouds);

        glDisable(GL_BLEND);
    }

    if(cloudlayer[0] && cloudheight)
    {
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        skymatrix = cammatrix;
        skymatrix.d = vec4(0, 0, 0, 1);
        skymatrix.rotate_around_z((spincloudlayer*lastmillis/1000.0f+yawcloudlayer)*-RAD);
        skyprojmatrix.mul(projmatrix, skymatrix);
        LOCALPARAM(skymatrix, skyprojmatrix);

        draw_env_overlay(farplane/2, cloudoverlay, cloudoffsetx + cloudscrollx * lastmillis/1000.0f, cloudoffsety + cloudscrolly * lastmillis/1000.0f);

        glDisable(GL_BLEND);

        glEnable(GL_CULL_FACE);
    }

	if(fogdomemax && fogdomeclouds)
	{
        drawfogdome(farplane);
	}

    if(clampsky) glDepthRange(0, 1);

    if(limitsky())
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
}

