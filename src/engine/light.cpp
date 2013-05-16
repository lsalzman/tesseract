#include "engine.h"

bvec ambientcolor(0x19, 0x19, 0x19);
HVARFR(ambient, 1, 0x191919, 0xFFFFFF, 
{
    if(ambient <= 255) ambient |= (ambient<<8) | (ambient<<16);
    ambientcolor = bvec((ambient>>16)&0xFF, (ambient>>8)&0xFF, ambient&0xFF);
});
FVARR(ambientscale, 0, 1, 16);

bvec skylightcolor(0, 0, 0);
HVARFR(skylight, 0, 0, 0xFFFFFF,
{
    if(skylight <= 255) skylight |= (skylight<<8) | (skylight<<16);
    skylightcolor = bvec((skylight>>16)&0xFF, (skylight>>8)&0xFF, skylight&0xFF);
});
FVARR(skylightscale, 0, 1, 16);

extern void setupsunlight();
bvec sunlightcolor(0, 0, 0);
HVARFR(sunlight, 0, 0, 0xFFFFFF,
{
    if(sunlight <= 255) sunlight |= (sunlight<<8) | (sunlight<<16);
    sunlightcolor = bvec((sunlight>>16)&0xFF, (sunlight>>8)&0xFF, sunlight&0xFF);
    setupsunlight();
    cleardeferredlightshaders();
    clearshadowcache();
});
FVARFR(sunlightscale, 0, 1, 16, setupsunlight());
vec sunlightdir(0, 0, 1);
extern void setsunlightdir();
FVARFR(sunlightyaw, 0, 0, 360, setsunlightdir());
FVARFR(sunlightpitch, -90, 90, 90, setsunlightdir());

void setsunlightdir() 
{ 
    sunlightdir = vec(sunlightyaw*RAD, sunlightpitch*RAD); 
    loopk(3) if(fabs(sunlightdir[k]) < 1e-5f) sunlightdir[k] = 0;
    sunlightdir.normalize();
    setupsunlight(); 
}

entity sunlightent;
void setupsunlight()
{
    memset(&sunlightent, 0, sizeof(sunlightent));
    sunlightent.type = ET_LIGHT;
    sunlightent.attr1 = 0;
    sunlightent.attr2 = int(sunlightcolor.x*sunlightscale);
    sunlightent.attr3 = int(sunlightcolor.y*sunlightscale);
    sunlightent.attr4 = int(sunlightcolor.z*sunlightscale);
    float dist = min(min(sunlightdir.x ? 1/fabs(sunlightdir.x) : 1e16f, sunlightdir.y ? 1/fabs(sunlightdir.y) : 1e16f), sunlightdir.z ? 1/fabs(sunlightdir.z) : 1e16f);
    sunlightent.o = vec(sunlightdir).mul(dist*worldsize).add(vec(worldsize/2, worldsize/2, worldsize/2)); 
    clearradiancehintscache();
}

static const surfaceinfo brightsurfaces[6] =
{
    brightsurface,
    brightsurface,
    brightsurface,
    brightsurface,
    brightsurface,
    brightsurface
};

void brightencube(cube &c)
{
    if(!c.ext) newcubeext(c, 0, false);
    memcpy(c.ext->surfaces, brightsurfaces, sizeof(brightsurfaces));
}

void setsurfaces(cube &c, const surfaceinfo *surfs, const vertinfo *verts, int numverts)
{
    if(!c.ext || c.ext->maxverts < numverts) newcubeext(c, numverts, false);
    memcpy(c.ext->surfaces, surfs, sizeof(c.ext->surfaces));
    memcpy(c.ext->verts(), verts, numverts*sizeof(vertinfo));
}

void setsurface(cube &c, int orient, const surfaceinfo &src, const vertinfo *srcverts, int numsrcverts)
{
    int dstoffset = 0;
    if(!c.ext) newcubeext(c, numsrcverts, true);
    else
    {
        int numbefore = 0, beforeoffset = 0;
        loopi(orient)
        {
            surfaceinfo &surf = c.ext->surfaces[i];
            int numverts = surf.totalverts();
            if(!numverts) continue;
            numbefore += numverts;
            beforeoffset = surf.verts + numverts;
        }
        int numafter = 0, afteroffset = c.ext->maxverts;
        for(int i = 5; i > orient; i--)
        {
            surfaceinfo &surf = c.ext->surfaces[i];
            int numverts = surf.totalverts();
            if(!numverts) continue;
            numafter += numverts;
            afteroffset = surf.verts;
        }
        if(afteroffset - beforeoffset >= numsrcverts) dstoffset = beforeoffset;
        else
        {
            cubeext *ext = c.ext;
            if(numbefore + numsrcverts + numafter > c.ext->maxverts)
            {
                ext = growcubeext(c.ext, numbefore + numsrcverts + numafter);
                memcpy(ext->surfaces, c.ext->surfaces, sizeof(ext->surfaces));
            }
            int offset = 0;
            if(numbefore == beforeoffset)
            {
                if(numbefore && c.ext != ext) memcpy(ext->verts(), c.ext->verts(), numbefore*sizeof(vertinfo));
                offset = numbefore;
            }
            else loopi(orient)
            {
                surfaceinfo &surf = ext->surfaces[i];
                int numverts = surf.totalverts();
                if(!numverts) continue;
                memmove(ext->verts() + offset, c.ext->verts() + surf.verts, numverts*sizeof(vertinfo));
                surf.verts = offset;
                offset += numverts;
            }
            dstoffset = offset;
            offset += numsrcverts;
            if(numafter && offset > afteroffset)
            {
                offset += numafter;
                for(int i = 5; i > orient; i--)
                {
                    surfaceinfo &surf = ext->surfaces[i];
                    int numverts = surf.totalverts();
                    if(!numverts) continue;
                    offset -= numverts;
                    memmove(ext->verts() + offset, c.ext->verts() + surf.verts, numverts*sizeof(vertinfo));
                    surf.verts = offset;
                }
            }
            if(c.ext != ext) setcubeext(c, ext);
        }
    }
    surfaceinfo &dst = c.ext->surfaces[orient];
    dst = src;
    dst.verts = dstoffset;
    if(srcverts) memcpy(c.ext->verts() + dstoffset, srcverts, numsrcverts*sizeof(vertinfo));
}

bool PackNode::insert(ushort &tx, ushort &ty, ushort tw, ushort th)
{
    if((available < tw && available < th) || w < tw || h < th)
        return false;
    if(child1)
    {
        bool inserted = child1->insert(tx, ty, tw, th) ||
                        child2->insert(tx, ty, tw, th);
        available = max(child1->available, child2->available);
        if(!available) discardchildren();
        return inserted;    
    }
    if(w == tw && h == th)
    {
        available = 0;
        tx = x;
        ty = y;
        return true;
    }
    
    if(w - tw > h - th)
    {
        child1 = new PackNode(x, y, tw, h);
        child2 = new PackNode(x + tw, y, w - tw, h);
    }
    else
    {
        child1 = new PackNode(x, y, w, th);
        child2 = new PackNode(x, y + th, w, h - th);
    }

    bool inserted = child1->insert(tx, ty, tw, th);
    available = max(child1->available, child2->available);
    return inserted;
}

void PackNode::reserve(ushort tx, ushort ty, ushort tw, ushort th)
{
    if(tx + tw <= x || tx >= x + w || ty + th <= y || ty >= y + h) return;
    if(child1)
    {
        child1->reserve(tx, ty, tw, th);
        child2->reserve(tx, ty, tw, th);
        available = max(child1->available, child2->available);
        return;
    }
    int dx1 = tx - x, dx2 = x + w - tx - tw, dx = max(dx1, dx2),
        dy1 = ty - y, dy2 = y + h - ty - th, dy = max(dy1, dy2),
        split;
    if(dx > dy)
    {
        if(dx1 > dx2) split = min(dx1, int(w));
        else split = w - max(dx2, 0);
        if(w - split <= 0)
        {
            w = split; 
            available = min(w, h);
            if(dy > 0) reserve(tx, ty, tw, th);
            else if(tx <= x && tx + tw >= x + w) available = 0;
            return;
        }
        if(split <= 0)
        {
            x += split;
            w -= split;
            available = min(w, h);
            if(dy > 0) reserve(tx, ty, tw, th);
            else if(tx <= x && tx + tw >= x + w) available = 0;
            return;
        }
        child1 = new PackNode(x, y, split, h);
        child2 = new PackNode(x + split, y, w - split, h);
    }
    else
    {
        if(dy1 > dy2) split = min(dy1, int(h));
        else split = h - max(dy2, 0);
        if(h - split <= 0)
        {
            h = split;
            available = min(w, h);
            if(dx > 0) reserve(tx, ty, tw, th);
            else if(ty <= y && ty + th >= y + h) available = 0;
            return;
        }
        if(split <= 0)
        {
            y += split;
            h -= split;
            available = min(w, h);
            if(dx > 0) reserve(tx, ty, tw, th);
            else if(ty <= y && ty + th >= y + h) available = 0;
            return;
        }
        child1 = new PackNode(x, y, w, split);
        child2 = new PackNode(x, y + split, w, h - split);
    }
    child1->reserve(tx, ty, tw, th);
    child2->reserve(tx, ty, tw, th);
    available = max(child1->available, child2->available);
}

static void clearsurfaces(cube *c)
{
    loopi(8)
    {
        if(c[i].ext)
        {
            loopj(6) 
            {
                surfaceinfo &surf = c[i].ext->surfaces[j];
                if(!surf.used()) continue;
                surf.clear();
                int numverts = surf.numverts&MAXFACEVERTS;
                if(numverts)
                {
                    if(!(c[i].merged&(1<<j))) { surf.numverts &= ~MAXFACEVERTS; continue; }

                    vertinfo *verts = c[i].ext->verts() + surf.verts;
                    loopk(numverts)
                    {
                        vertinfo &v = verts[k];
                        v.norm = 0;
                    }
                }
            } 
        }
        if(c[i].children) clearsurfaces(c[i].children);
    }
}

#define LIGHTCACHESIZE 1024

static struct lightcacheentry
{
    int x, y;
    vector<int> lights;
} lightcache[LIGHTCACHESIZE];

#define LIGHTCACHEHASH(x, y) (((((x)^(y))<<5) + (((x)^(y))>>5)) & (LIGHTCACHESIZE - 1))

VARF(lightcachesize, 4, 6, 12, clearlightcache());

void clearlightcache(int id)
{
    if(id >= 0)
    {
        const extentity &light = *entities::getents()[id];
        int radius = light.attr1;
        if(radius)
        {
            for(int x = int(max(light.o.x-radius, 0.0f))>>lightcachesize, ex = int(min(light.o.x+radius, worldsize-1.0f))>>lightcachesize; x <= ex; x++)
            for(int y = int(max(light.o.y-radius, 0.0f))>>lightcachesize, ey = int(min(light.o.y+radius, worldsize-1.0f))>>lightcachesize; y <= ey; y++)
            {
                lightcacheentry &lce = lightcache[LIGHTCACHEHASH(x, y)];
                if(lce.x != x || lce.y != y) continue;
                lce.x = -1;
                lce.lights.setsize(0);
            }
            return;
        }
    }

    for(lightcacheentry *lce = lightcache; lce < &lightcache[LIGHTCACHESIZE]; lce++)
    {
        lce->x = -1;
        lce->lights.setsize(0);
    }
}

const vector<int> &checklightcache(int x, int y)
{
    x >>= lightcachesize;
    y >>= lightcachesize; 
    lightcacheentry &lce = lightcache[LIGHTCACHEHASH(x, y)];
    if(lce.x == x && lce.y == y) return lce.lights;

    lce.lights.setsize(0);
    int csize = 1<<lightcachesize, cx = x<<lightcachesize, cy = y<<lightcachesize;
    const vector<extentity *> &ents = entities::getents();
    loopv(ents)
    {
        const extentity &light = *ents[i];
        switch(light.type)
        {
            case ET_LIGHT:
            {
                int radius = light.attr1;
                if(radius > 0)
                {
                    if(light.o.x + radius < cx || light.o.x - radius > cx + csize ||
                       light.o.y + radius < cy || light.o.y - radius > cy + csize)
                        continue;
                }
                break;
            }
            default: continue;
        }
        lce.lights.add(i);
    }

    lce.x = x;
    lce.y = y;
    return lce.lights;
}

static uint lightprogress = 0;

bool calclight_canceled = false;
volatile bool check_calclight_progress = false;

void check_calclight_canceled()
{
    if(interceptkey(SDLK_ESCAPE))
    {
        calclight_canceled = true;
    }
    if(!calclight_canceled) check_calclight_progress = false;
}

void show_calclight_progress()
{
    float bar1 = float(lightprogress) / float(allocnodes);
    defformatstring(text1)("%d%%", int(bar1 * 100));

    renderprogress(bar1, text1);
}

static void calcsurfaces(cube &c, const ivec &co, int size, int usefacemask, int preview = 0)
{
    surfaceinfo surfaces[6];
    vertinfo litverts[6*2*MAXFACEVERTS];
    int numlitverts = 0;
    memset(surfaces, 0, sizeof(surfaces));
    loopi(6)
    {
        int usefaces = usefacemask&0xF;
        usefacemask >>= 4;
        if(!usefaces)
        {
            if(!c.ext) continue;
            surfaceinfo &surf = surfaces[i];
            surf = c.ext->surfaces[i];
            int numverts = surf.totalverts();
            if(numverts)
            {
                memcpy(&litverts[numlitverts], c.ext->verts() + surf.verts, numverts*sizeof(vertinfo));
                surf.verts = numlitverts;
                numlitverts += numverts;
            }
            continue;
        }

        VSlot &vslot = lookupvslot(c.texture[i], false),
             *layer = vslot.layer && !(c.material&MAT_ALPHA) ? &lookupvslot(vslot.layer, false) : NULL;
        Shader *shader = vslot.slot->shader;
        int shadertype = shader->type;
        if(layer) shadertype |= layer->slot->shader->type;

        surfaceinfo &surf = surfaces[i];
        vertinfo *curlitverts = &litverts[numlitverts];
        int numverts = c.ext ? c.ext->surfaces[i].numverts&MAXFACEVERTS : 0;
        ivec mo(co);
        int msz = size, convex = 0;
        if(numverts)
        {
            vertinfo *verts = c.ext->verts() + c.ext->surfaces[i].verts;
            loopj(numverts) curlitverts[j].set(verts[j].getxyz());
            if(c.merged&(1<<i))
            {
                msz = 1<<calcmergedsize(i, mo, size, verts, numverts);
                mo.mask(~(msz-1));

                if(!(surf.numverts&MAXFACEVERTS))
                {
                    surf.verts = numlitverts;
                    surf.numverts |= numverts;
                    numlitverts += numverts;
                }
            }
            else if(!flataxisface(c, i)) convex = faceconvexity(verts, numverts, size);
        }
        else
        {
            ivec v[4];
            genfaceverts(c, i, v);
            if(!flataxisface(c, i)) convex = faceconvexity(v);
            int order = usefaces&4 || convex < 0 ? 1 : 0;
            ivec vo = ivec(co).mask(0xFFF).shl(3);
            curlitverts[numverts++].set(v[order].mul(size).add(vo));
            if(usefaces&1) curlitverts[numverts++].set(v[order+1].mul(size).add(vo));
            curlitverts[numverts++].set(v[order+2].mul(size).add(vo));
            if(usefaces&2) curlitverts[numverts++].set(v[(order+3)&3].mul(size).add(vo));
        }

        vec pos[MAXFACEVERTS], n[MAXFACEVERTS], po = ivec(co).mask(~0xFFF).tovec();
        loopj(numverts) pos[j] = curlitverts[j].getxyz().tovec().mul(1.0f/8).add(po);

        int smooth = vslot.slot->smooth;
        plane planes[2];
        int numplanes = 0;
        planes[numplanes++].toplane(pos[0], pos[1], pos[2]);
        if(numverts < 4 || !convex) loopk(numverts) findnormal(pos[k], smooth, planes[0], n[k]);
        else
        {
            planes[numplanes++].toplane(pos[0], pos[2], pos[3]);
            vec avg = vec(planes[0]).add(planes[1]).normalize();
            findnormal(pos[0], smooth, avg, n[0]);
            findnormal(pos[1], smooth, planes[0], n[1]);
            findnormal(pos[2], smooth, avg, n[2]);
            for(int k = 3; k < numverts; k++) findnormal(pos[k], smooth, planes[1], n[k]);
        }

        loopk(numverts) curlitverts[k].norm = encodenormal(n[k]);
        if(!(surf.numverts&MAXFACEVERTS))
        {
            surf.verts = numlitverts;
            surf.numverts |= numverts;
            numlitverts += numverts;
        }

        if(preview) { surf.numverts |= preview; continue; }

        int surflayer = LAYER_TOP;
        if(vslot.layer)
        {
            int x1 = curlitverts[numverts-1].x, y1 = curlitverts[numverts-1].y, x2 = x1, y2 = y1;
            loopj(numverts-1)
            {
                const vertinfo &v = curlitverts[j];
                x1 = min(x1, int(v.x));
                y1 = min(y1, int(v.y));
                x2 = max(x2, int(v.x));
                y2 = max(y2, int(v.y));
            }
            x2 = max(x2, x1+1);
            y2 = max(y2, y1+1);
            x1 = (x1>>3) + (co.x&~0xFFF);
            y1 = (y1>>3) + (co.y&~0xFFF);
            x2 = ((x2+7)>>3) + (co.x&~0xFFF);
            y2 = ((y2+7)>>3) + (co.y&~0xFFF);
            surflayer = calcblendlayer(x1, y1, x2, y2);
        }
        surf.numverts |= surflayer;
    }
    if(preview) setsurfaces(c, surfaces, litverts, numlitverts);
    else loopk(6)
    {
        surfaceinfo &surf = surfaces[k];
        if(surf.used())
        {
            cubeext *ext = c.ext && c.ext->maxverts >= numlitverts ? c.ext : growcubeext(c.ext, numlitverts);
            memcpy(ext->surfaces, surfaces, sizeof(ext->surfaces));
            memcpy(ext->verts(), litverts, numlitverts*sizeof(vertinfo));
            if(c.ext != ext) setcubeext(c, ext);
            break;
        }
    }
}

static void calcsurfaces(cube *c, const ivec &co, int size)
{
    CHECK_CALCLIGHT_PROGRESS(return, show_calclight_progress);

    lightprogress++;

    loopi(8)
    {
        ivec o(i, co.x, co.y, co.z, size);
        if(c[i].children)
            calcsurfaces(c[i].children, o, size >> 1);
        else if(!isempty(c[i]))
        {
            if(c[i].ext)
            {
                loopj(6) c[i].ext->surfaces[j].clear();
            }    
            int usefacemask = 0;
            loopj(6) if(c[i].texture[j] != DEFAULT_SKY && (!(c[i].merged&(1<<j)) || (c[i].ext && c[i].ext->surfaces[j].numverts&MAXFACEVERTS)))
            {
                usefacemask |= visibletris(c[i], j, o.x, o.y, o.z, size)<<(4*j);
            }
            if(usefacemask) calcsurfaces(c[i], o, size, usefacemask);
        }
    }
}

static inline bool previewblends(cube &c, const ivec &o, int size)
{
    if(isempty(c) || c.material&MAT_ALPHA) return false;
    int usefacemask = 0;
    loopj(6) if(c.texture[j] != DEFAULT_SKY && lookupvslot(c.texture[j], false).layer)
        usefacemask |= visibletris(c, j, o.x, o.y, o.z, size)<<(4*j);
    if(!usefacemask) return false;
    int layer = calcblendlayer(o.x, o.y, o.x + size, o.y + size);
    if(!(layer&LAYER_BOTTOM))
    {
        if(!c.ext) return false;
        bool blends = false;
        loopi(6) if(c.ext->surfaces[i].numverts&LAYER_BOTTOM)
        {
            c.ext->surfaces[i].brighten();
            blends = true;
        }
        return blends;
    }
    calcsurfaces(c, o, size, usefacemask, layer);
    return true;
}

static bool previewblends(cube *c, const ivec &co, int size, const ivec &bo, const ivec &bs)
{
    bool changed = false;
    loopoctabox(co, size, bo, bs)
    {
        ivec o(i, co.x, co.y, co.z, size);
        cubeext *ext = c[i].ext;
        if(ext && ext->va && ext->va->hasmerges)
        {
            changed = true;
            destroyva(ext->va);
            ext->va = NULL;
            invalidatemerges(c[i], co, size, true);
        }
        if(c[i].children ? previewblends(c[i].children, o, size/2, bo, bs) : previewblends(c[i], o, size))  
        {
            changed = true;
            ext = c[i].ext;
            if(ext && ext->va)
            {
                destroyva(ext->va);
                ext->va = NULL;
            }
        }
    }
    return changed;
}

void previewblends(const ivec &bo, const ivec &bs)
{
    updateblendtextures(bo.x, bo.y, bo.x+bs.x, bo.y+bs.y);
    if(previewblends(worldroot, ivec(0, 0, 0), worldsize/2, bo, bs))
        commitchanges(true);
}
                            
extern int filltjoints;

static Uint32 calclighttimer(Uint32 interval, void *param)
{
    check_calclight_progress = true;
    return interval;
}

void calclight()
{
    renderbackground("computing lighting... (esc to abort)");
    mpremip(true);
    optimizeblendmap();
    clearlightcache();
    clearsurfaces(worldroot);
    lightprogress = 0;
    calclight_canceled = false;
    check_calclight_progress = false;
    SDL_TimerID timer = SDL_AddTimer(250, calclighttimer, NULL);
    Uint32 start = SDL_GetTicks();
    calcnormals(filltjoints > 0);
    calcsurfaces(worldroot, ivec(0, 0, 0), worldsize >> 1);
    clearnormals();
    Uint32 end = SDL_GetTicks();
    if(timer) SDL_RemoveTimer(timer);
    initlights();
    renderbackground("lighting done...");
    allchanged();
    if(calclight_canceled)
        conoutf("calclight aborted");
    else
        conoutf("computed lighting (%.1f seconds)",
            (end - start) / 1000.0f);
}

COMMAND(calclight, "");

VARF(fullbright, 0, 0, 1, initlights());
VARF(fullbrightlevel, 0, 128, 255, initlights());

void clearlights()
{
    clearlightcache();
    clearshadowcache();
    cleardeferredlightshaders();
    resetsmoothgroups();
}

void initlights()
{
    clearlightcache();
    clearshadowcache();
    loaddeferredlightshaders();
}

void lightreaching(const vec &target, vec &color, vec &dir, bool fast, extentity *t, float ambient)
{
    if(fullbright && editmode)
    {
        color = vec(1, 1, 1);
        dir = vec(0, 0, 1);
        return;
    }

    color = dir = vec(0, 0, 0);
    const vector<extentity *> &ents = entities::getents();
    const vector<int> &lights = checklightcache(int(target.x), int(target.y));
    loopv(lights)
    {
        extentity &e = *ents[lights[i]];
        if(e.type != ET_LIGHT)
            continue;
    
        vec ray(target);
        ray.sub(e.o);
        float mag = ray.magnitude();
        if(e.attr1 && mag >= float(e.attr1))
            continue;
    
        if(mag < 1e-4f) ray = vec(0, 0, -1);
        else
        {
            ray.div(mag);
            if(shadowray(e.o, ray, mag, RAY_SHADOW | RAY_POLY, t) < mag)
                continue;
        }

        float intensity = 1;
        if(e.attr1)
            intensity -= mag / float(e.attr1);
        if(e.attached && e.attached->type==ET_SPOTLIGHT)
        {
            vec spot = vec(e.attached->o).sub(e.o).normalize();
            float maxatten = sincos360[clamp(int(e.attached->attr1), 1, 89)].x, spotatten = (ray.dot(spot) - maxatten) / (1 - maxatten);
            if(spotatten <= 0) continue;
            intensity *= spotatten;
        }

        //if(target==player->o)
        //{
        //    conoutf(CON_DEBUG, "%d - %f %f", i, intensity, mag);
        //}
 
        vec lightcol = vec(e.attr2, e.attr3, e.attr4).mul(1.0f/255);
        color.add(vec(lightcol).mul(intensity));
        dir.add(vec(ray).mul(-intensity*lightcol.x*lightcol.y*lightcol.z));
    }
    if(sunlight && shadowray(target, sunlightdir, 1e16f, RAY_SHADOW | RAY_POLY, t) > 1e15f) 
    {
        vec lightcol = vec(sunlightcolor.x, sunlightcolor.y, sunlightcolor.z).mul(sunlightscale/255);
        color.add(lightcol);
        dir.add(vec(sunlightdir).mul(lightcol.x*lightcol.y*lightcol.z));
    }
    loopk(3) color[k] = min(1.5f, max(max(ambientcolor[k]/255.0f, ambient), color[k]));
    if(dir.iszero()) dir = vec(0, 0, 1);
    else dir.normalize();
}

entity *brightestlight(const vec &target, const vec &dir)
{
    if(sunlight && sunlightdir.dot(dir) > 0 && shadowray(target, sunlightdir, 1e16f, RAY_SHADOW | RAY_POLY) > 1e15f)    
        return &sunlightent;
    const vector<extentity *> &ents = entities::getents();
    const vector<int> &lights = checklightcache(int(target.x), int(target.y));
    extentity *brightest = NULL;
    float bintensity = 0;
    loopv(lights)
    {
        extentity &e = *ents[lights[i]];
        if(e.type != ET_LIGHT || vec(e.o).sub(target).dot(dir)<0)
            continue;

        vec ray(target);
        ray.sub(e.o);
        float mag = ray.magnitude();
        if(e.attr1 && mag >= float(e.attr1))
             continue;

        ray.div(mag);
        if(shadowray(e.o, ray, mag, RAY_SHADOW | RAY_POLY) < mag)
            continue;
        float intensity = 1;
        if(e.attr1)
            intensity -= mag / float(e.attr1);
        if(e.attached && e.attached->type==ET_SPOTLIGHT)
        {
            vec spot = vec(e.attached->o).sub(e.o).normalize();
            float maxatten = sincos360[clamp(int(e.attached->attr1), 1, 89)].x, spotatten = (ray.dot(spot) - maxatten) / (1 - maxatten);
            if(spotatten <= 0) continue;
            intensity *= spotatten;
        }

        if(!brightest || intensity > bintensity)
        {
            brightest = &e;
            bintensity = intensity;
        }
    }
    return brightest;
}

