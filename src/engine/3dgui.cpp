// creates multiple gui windows that float inside the 3d world

// special feature is that its mostly *modeless*: you can use this menu while playing, without turning menus on or off
// implementationwise, it is *stateless*: it keeps no internal gui structure, hit tests are instant, usage & implementation is greatly simplified

#include "engine.h"

#include "textedit.h"

static bool layoutpass, actionon = false;
static int mousebuttons = 0;
static struct gui *windowhit = NULL;

static float firstx, firsty;

enum {FIELDCOMMIT, FIELDABORT, FIELDEDIT, FIELDSHOW, FIELDKEY};

static int fieldmode = FIELDSHOW; 
static bool fieldsactive = false;

static bool hascursor;
static float cursorx = 0.5f, cursory = 0.5f;

#define SHADOW 4
#define ICON_SIZE (FONTH-SHADOW)
#define SKIN_W 256
#define SKIN_H 128
#define SKIN_SCALE 4
#define INSERT (3*SKIN_SCALE)
#define MAXCOLUMNS 16

VARP(guiautotab, 6, 16, 40);
VARP(guiclicktab, 0, 0, 1);
VARP(guifadein, 0, 1, 1);

struct gui : g3d_gui
{
    struct list
    {
        int parent, w, h, springs, curspring, column;
    };

    int firstlist, nextlist;
    int columns[MAXCOLUMNS];

    static vector<list> lists;
    static float hitx, hity;
    static int curdepth, curlist, xsize, ysize, curx, cury;
    static bool shouldmergehits, shouldautotab;

    static void reset()
    {
        lists.setsize(0);
    }

    static int ty, tx, tpos, *tcurrent, tcolor; //tracking tab size and position since uses different layout method...

    bool allowautotab(bool on)
    {
        bool oldval = shouldautotab;
        shouldautotab = on;
        return oldval;
    }

    void autotab() 
    { 
        if(tcurrent)
        {
            if(layoutpass && !tpos) tcurrent = NULL; //disable tabs because you didn't start with one
            if(shouldautotab && !curdepth && (layoutpass ? 0 : cury) + ysize > guiautotab*FONTH) tab(NULL, tcolor); 
        }
    }

    bool shouldtab()
    {
        if(tcurrent && shouldautotab)
        {
            if(layoutpass)
            {
                int space = guiautotab*FONTH - ysize;
                if(space < 0) return true;
                int l = lists[curlist].parent;
                while(l >= 0)
                {
                    space -= lists[l].h;
                    if(space < 0) return true;
                    l = lists[l].parent;
                }
            }
            else
            {
                int space = guiautotab*FONTH - cury;
                if(ysize > space) return true;
                int l = lists[curlist].parent;
                while(l >= 0)
                {
                    if(lists[l].h > space) return true;
                    l = lists[l].parent;
                }
            }
        }
        return false;
    }

    bool visible() { return (!tcurrent || tpos==*tcurrent) && !layoutpass; }

    //tab is always at top of page
    void tab(const char *name, int color) 
    {
        if(curdepth != 0) return;
        if(color) tcolor = color;
        tpos++; 
        if(!name) name = intstr(tpos); 
        int w = max(text_width(name) - 2*INSERT, 0);
        if(layoutpass) 
        {  
            ty = max(ty, ysize); 
            ysize = 0;
        }
        else 
        {	
            cury = -ysize;
            int h = FONTH-2*INSERT,
                x1 = curx + tx,
                x2 = x1 + w + ((skinx[3]-skinx[2]) + (skinx[5]-skinx[4]))*SKIN_SCALE,
                y1 = cury - ((skiny[6]-skiny[1])-(skiny[3]-skiny[2]))*SKIN_SCALE-h,
                y2 = cury;
            bool hit = tcurrent && windowhit==this && hitx>=x1 && hity>=y1 && hitx<x2 && hity<y2;
            if(hit && (!guiclicktab || mousebuttons&G3D_DOWN)) 
                *tcurrent = tpos; //roll-over to switch tab
            
            drawskin(x1-skinx[visible()?2:6]*SKIN_SCALE, y1-skiny[1]*SKIN_SCALE, w, h, visible()?10:19, 9, alpha);
            text_(name, x1 + (skinx[3]-skinx[2])*SKIN_SCALE - (w ? INSERT : INSERT/2), y1 + (skiny[2]-skiny[1])*SKIN_SCALE - INSERT, tcolor, visible());
        }
        tx += w + ((skinx[5]-skinx[4]) + (skinx[3]-skinx[2]))*SKIN_SCALE; 
    }

    bool ishorizontal() const { return curdepth&1; }
    bool isvertical() const { return !ishorizontal(); }

    void pushlist()
    {	
        if(layoutpass)
        {
            if(curlist>=0)
            {
                lists[curlist].w = xsize;
                lists[curlist].h = ysize;
            }
            list &l = lists.add();
            l.parent = curlist;
            l.springs = 0;
            l.column = -1;
            curlist = lists.length()-1;
            xsize = ysize = 0;
        }
        else
        {
            curlist = nextlist++;
            if(curlist >= lists.length()) // should never get here unless script code doesn't use same amount of lists in layout and render passes
            {
                list &l = lists.add();
                l.parent = curlist;
                l.springs = 0;
                l.column = -1;
                l.w = l.h = 0;
            }
            list &l = lists[curlist];
            l.curspring = 0;
            if(l.springs > 0)
            {
                if(ishorizontal()) xsize = l.w; else ysize = l.h;
            }
            else
            {
                xsize = l.w;
                ysize = l.h;
            }
        }
        curdepth++;	
    }

    void poplist()
    {
        if(!lists.inrange(curlist)) return;
        list &l = lists[curlist];
        if(layoutpass)
        {
            l.w = xsize;
            l.h = ysize;
            if(l.column >= 0) columns[l.column] = max(columns[l.column], ishorizontal() ? ysize : xsize);
        }
        curlist = l.parent;
        curdepth--;
        if(lists.inrange(curlist))
        {   
            int w = xsize, h = ysize;
            if(ishorizontal()) cury -= h; else curx -= w;
            list &p = lists[curlist];
            xsize = p.w;
            ysize = p.h;
            if(!layoutpass && p.springs > 0)
            {
                list &s = lists[p.parent];
                if(ishorizontal()) xsize = s.w; else ysize = s.h;
            } 
            layout(w, h);
        }
    }

    int text  (const char *text, int color, const char *icon) { autotab(); return button_(text, color, icon, false, false); }
    int button(const char *text, int color, const char *icon) { autotab(); return button_(text, color, icon, true, false); }
    int title (const char *text, int color, const char *icon) { autotab(); return button_(text, color, icon, false, true); }

    void separator() { autotab(); line_(FONTH/3); }
    void progress(float percent) { autotab(); line_((FONTH*4)/5, percent); }

    //use to set min size (useful when you have progress bars)
    void strut(float size) { layout(isvertical() ? int(size*FONTW) : 0, isvertical() ? 0 : int(size*FONTH)); }
    //add space between list items
    void space(float size) { layout(isvertical() ? 0 : int(size*FONTW), isvertical() ? int(size*FONTH) : 0); }

    void spring(int weight) 
    { 
        if(curlist < 0) return;
        list &l = lists[curlist];
        if(layoutpass) { if(l.parent >= 0) l.springs += weight; return; }
        int nextspring = min(l.curspring + weight, l.springs);
        if(nextspring <= l.curspring) return;
        if(ishorizontal())
        {
            int w = xsize - l.w;
            layout((w*nextspring)/l.springs - (w*l.curspring)/l.springs, 0);
        }
        else
        {
            int h = ysize - l.h;
            layout(0, (h*nextspring)/l.springs - (h*l.curspring)/l.springs);
        }
        l.curspring = nextspring;
    }

    void column(int col)
    {
        if(curlist < 0 || !layoutpass || col < 0 || col >= MAXCOLUMNS) return;
        list &l = lists[curlist];
        l.column = col;
    }

    int layout(int w, int h)
    {
        if(layoutpass)
        {
            if(ishorizontal())
            {
                xsize += w;
                ysize = max(ysize, h);
            }
            else
            {
                xsize = max(xsize, w);
                ysize += h;
            }
            return 0;
        }
        else
        {
            bool hit = ishit(w, h);
            if(ishorizontal()) curx += w;
            else cury += h;
            return (hit && visible()) ? mousebuttons|G3D_ROLLOVER : 0;
        }
    }

    bool mergehits(bool on) 
    { 
        bool oldval = shouldmergehits;
        shouldmergehits = on; 
        return oldval;
    }

    bool ishit(int w, int h, int x = curx, int y = cury)
    {
        if(shouldmergehits) return windowhit==this && (ishorizontal() ? hitx>=x && hitx<x+w : hity>=y && hity<y+h);
        if(ishorizontal()) h = ysize;
        else w = xsize;
        return windowhit==this && hitx>=x && hity>=y && hitx<x+w && hity<y+h;
    }

    int image(Texture *t, float scale, bool overlaid)
    {
        autotab();
        if(scale==0) scale = 1;
        int size = (int)(scale*2*FONTH)-SHADOW;
        if(visible()) icon_(t, overlaid, curx, cury, size, ishit(size+SHADOW, size+SHADOW));
        return layout(size+SHADOW, size+SHADOW);
    }
    
    int texture(VSlot &vslot, float scale, bool overlaid)
    {
        autotab();
        if(scale==0) scale = 1;
        int size = (int)(scale*2*FONTH)-SHADOW;
        if(visible()) previewslot(vslot, overlaid, curx, cury, size, ishit(size+SHADOW, size+SHADOW));
        return layout(size+SHADOW, size+SHADOW);
    }

    int playerpreview(int model, int team, int weap, float sizescale, bool overlaid)
    {
        autotab();
        if(sizescale==0) sizescale = 1;
        int size = (int)(sizescale*2*FONTH)-SHADOW;
        if(visible())
        {
            bool hit = ishit(size+SHADOW, size+SHADOW);
            float xs = size, ys = size, xi = curx, yi = cury;
            if(overlaid && hit && actionon)
            {
                hudnotextureshader->set();
                varray::colorf(0, 0, 0, 0.75f);
                rect_(xi+SHADOW, yi+SHADOW, xs, ys, -1);
                hudshader->set();
            }
            int x1 = int(floor(screenw*(xi*scale.x+origin.x))), y1 = int(floor(screenh*(1 - ((yi+ys)*scale.y+origin.y)))),
                x2 = int(ceil(screenw*((xi+xs)*scale.x+origin.x))), y2 = int(ceil(screenh*(1 - (yi*scale.y+origin.y))));
            glDisable(GL_BLEND);
            varray::disable();
            modelpreview::start(x1, y1, x2-x1, y2-y1, overlaid);
            game::renderplayerpreview(model, team, weap);
            modelpreview::end();
            hudshader->set();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            if(overlaid)
            {
                if(hit)
                {
                    hudnotextureshader->set();
                    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                    varray::colorf(1, 0.5f, 0.5f);
                    rect_(xi, yi, xs, ys, -1);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    hudshader->set();
                }
                if(!overlaytex) overlaytex = textureload("data/guioverlay.png", 3);
                varray::colorf(1, 1, 1);
                glBindTexture(GL_TEXTURE_2D, overlaytex->id);
                rect_(xi, yi, xs, ys, 0);
            }
        }
        return layout(size+SHADOW, size+SHADOW);
    }

    int modelpreview(const char *name, int anim, float sizescale, bool overlaid)
    {
        autotab();
        if(sizescale==0) sizescale = 1;
        int size = (int)(sizescale*2*FONTH)-SHADOW;
        if(visible())
        {
            bool hit = ishit(size+SHADOW, size+SHADOW);
            float xs = size, ys = size, xi = curx, yi = cury;
            if(overlaid && hit && actionon)
            {
                hudnotextureshader->set();
                varray::colorf(0, 0, 0, 0.75f);
                rect_(xi+SHADOW, yi+SHADOW, xs, ys, -1);
                hudshader->set();
            }
            int x1 = int(floor(screenw*(xi*scale.x+origin.x))), y1 = int(floor(screenh*(1 - ((yi+ys)*scale.y+origin.y)))),
                x2 = int(ceil(screenw*((xi+xs)*scale.x+origin.x))), y2 = int(ceil(screenh*(1 - (yi*scale.y+origin.y))));
            glDisable(GL_BLEND);
            varray::disable();
            modelpreview::start(x1, y1, x2-x1, y2-y1, overlaid);
            model *m = loadmodel(name);
            if(m)
            {
                vec center, radius;
                m->boundbox(center, radius);
                float dist =  2.0f*max(radius.magnitude2(), 1.1f*radius.z),
                      yaw = fmod(lastmillis/10000.0f*360.0f, 360.0f);
                vec o(-center.x, dist - center.y, -0.1f*dist - center.z);
                rendermodel(name, anim, o, yaw, 0, 0, NULL, NULL, 0);
            }
            modelpreview::end();
            hudshader->set();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            if(overlaid)
            {
                if(hit)
                {
                    hudnotextureshader->set();
                    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                    varray::colorf(1, 0.5f, 0.5f);
                    rect_(xi, yi, xs, ys, -1);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    hudshader->set();
                }
                if(!overlaytex) overlaytex = textureload("data/guioverlay.png", 3);
                varray::colorf(1, 1, 1);
                glBindTexture(GL_TEXTURE_2D, overlaytex->id);
                rect_(xi, yi, xs, ys, 0);
            }
        }
        return layout(size+SHADOW, size+SHADOW);
    }

    void slider(int &val, int vmin, int vmax, int color, const char *label)
    {
        autotab();
        int x = curx;
        int y = cury;
        line_((FONTH*2)/3);
        if(visible())
        {
            if(!label) label = intstr(val);
            int w = text_width(label);

            bool hit;
            int px, py;
            if(ishorizontal())
            {
                hit = ishit(FONTH, ysize, x, y);
                px = x + (FONTH-w)/2;
                py = y + (ysize-FONTH) - ((ysize-FONTH)*(val-vmin))/((vmax==vmin) ? 1 : (vmax-vmin)); //vmin at bottom
            }
            else
            {
                hit = ishit(xsize, FONTH, x, y);
                px = x + FONTH/2 - w/2 + ((xsize-w)*(val-vmin))/((vmax==vmin) ? 1 : (vmax-vmin)); //vmin at left
                py = y;
            }

            if(hit) color = 0xFF0000;
            text_(label, px, py, color, hit && actionon, hit);
            if(hit && actionon)
            {
                int vnew = (vmin < vmax ? 1 : -1)+vmax-vmin;
                if(ishorizontal()) vnew = int((vnew*(y+ysize-FONTH/2-hity))/(ysize-FONTH));
                else vnew = int((vnew*(hitx-x-FONTH/2))/(xsize-w));
                vnew += vmin;
                vnew = vmin < vmax ? clamp(vnew, vmin, vmax) : clamp(vnew, vmax, vmin);
                if(vnew != val) val = vnew;
            }
        }
    }

    char *field(const char *name, int color, int length, int height, const char *initval, int initmode)
    {
        return field_(name, color, length, height, initval, initmode, FIELDEDIT);
    }

    char *keyfield(const char *name, int color, int length, int height, const char *initval, int initmode)
    {
        return field_(name, color, length, height, initval, initmode, FIELDKEY);
    }

    char *field_(const char *name, int color, int length, int height, const char *initval, int initmode, int fieldtype = FIELDEDIT)
    {	
        editor *e = useeditor(name, initmode, false, initval); // generate a new editor if necessary
        if(layoutpass)
        {
            if(initval && e->mode==EDITORFOCUSED && (e!=currentfocus() || fieldmode == FIELDSHOW))
            {
                if(strcmp(e->lines[0].text, initval)) e->clear(initval);
            }
            e->linewrap = (length<0);
            e->maxx = (e->linewrap) ? -1 : length;
            e->maxy = (height<=0)?1:-1;
            e->pixelwidth = abs(length)*FONTW;
            if(e->linewrap && e->maxy==1) 
            {
                int temp;
                text_bounds(e->lines[0].text, temp, e->pixelheight, e->pixelwidth); //only single line editors can have variable height
            }
            else 
                e->pixelheight = FONTH*max(height, 1); 
        }
        int h = e->pixelheight;
        int w = e->pixelwidth + FONTW;
        
        bool wasvertical = isvertical();
        if(wasvertical && e->maxy != 1) pushlist();
        
        char *result = NULL;
        if(visible() && !layoutpass)
        {
            e->rendered = true;

            bool hit = ishit(w, h);
            if(hit) 
            {
                if(mousebuttons&G3D_DOWN) //mouse request focus
                {   
                    if(fieldtype==FIELDKEY) e->clear();
                    useeditor(name, initmode, true); 
                    e->mark(false);
                    fieldmode = fieldtype;
                } 
            }
            bool editing = (fieldmode != FIELDSHOW) && (e==currentfocus());
            if(hit && editing && (mousebuttons&G3D_PRESSED)!=0 && fieldtype==FIELDEDIT) e->hit(int(floor(hitx-(curx+FONTW/2))), int(floor(hity-cury)), (mousebuttons&G3D_DRAGGED)!=0); //mouse request position
            if(editing && ((fieldmode==FIELDCOMMIT) || (fieldmode==FIELDABORT) || !hit)) // commit field if user pressed enter or wandered out of focus 
            {
                if(fieldmode==FIELDCOMMIT || (fieldmode!=FIELDABORT && !hit)) result = e->currentline().text;
                e->active = (e->mode!=EDITORFOCUSED);
                fieldmode = FIELDSHOW;
            } 
            else fieldsactive = true;
            
            e->draw(curx+FONTW/2, cury, color, hit && editing);
            
            hudnotextureshader->set();
            glDisable(GL_BLEND);
            if(editing) varray::colorf(1, 0, 0);
            else varray::color(vec::hexcolor(color));
            rect_(curx, cury, w, h, -1, true);
            glEnable(GL_BLEND);
            hudshader->set();
        }
        layout(w, h);
        
        if(e->maxy != 1)
        {
            int slines = e->limitscrolly();
            if(slines > 0) 
            {
                int pos = e->scrolly;
                slider(e->scrolly, slines, 0, color, NULL);
                if(pos != e->scrolly) e->cy = e->scrolly; 
            }
            if(wasvertical) poplist();
        }
        
        return result;
    }

    void rect_(float x, float y, float w, float h, int usetc = -1, bool lines = false) 
    {
        varray::defvertex(2);
        if(usetc >= 0) varray::deftexcoord0();
        varray::begin(lines ? GL_LINE_LOOP : GL_TRIANGLE_STRIP);
        static const vec2 tc[4] = { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1) };
        varray::attribf(x, y);
        if(usetc>=0) varray::attrib(tc[usetc]); 
        varray::attribf(x + w, y);
        if(usetc>=0) varray::attrib(tc[(usetc+1)%4]);
        if(lines)
        {
            varray::attribf(x + w, y + h);
            if(usetc>=0) varray::attrib(tc[(usetc+2)%4]);
        }
        varray::attribf(x, y + h);
        if(usetc>=0) varray::attrib(tc[(usetc+3)%4]);
        if(!lines)
        {
            varray::attribf(x + w, y + h);
            if(usetc>=0) varray::attrib(tc[(usetc+2)%4]);
        }
        xtraverts += varray::end();
        
    }

    void text_(const char *text, int x, int y, int color, bool shadow, bool force = false) 
    {
        if(shadow) draw_text(text, x+SHADOW, y+SHADOW, 0x00, 0x00, 0x00, -0xC0);
        draw_text(text, x, y, color>>16, (color>>8)&0xFF, color&0xFF, force ? -0xFF : 0xFF);
    }

    void background(int color, int inheritw, int inherith)
    {
        if(layoutpass) return;
        hudnotextureshader->set();
        varray::color(vec::hexcolor(color), 0.5f);
        int w = xsize, h = ysize;
        if(inheritw>0) 
        {
            int parentw = curlist, parentdepth = 0;
            for(;parentdepth < inheritw && lists[parentw].parent>=0; parentdepth++)
                parentw = lists[parentw].parent;
            list &p = lists[parentw];
            w = p.springs > 0 && (curdepth-parentdepth)&1 ? lists[p.parent].w : p.w;
        }
        if(inherith>0)
        {
            int parenth = curlist, parentdepth = 0;
            for(;parentdepth < inherith && lists[parenth].parent>=0; parentdepth++)
                parenth = lists[parenth].parent;
            list &p = lists[parenth];
            h = p.springs > 0 && !((curdepth-parentdepth)&1) ? lists[p.parent].h : p.h;
        }
        rect_(curx, cury, w, h);
        hudshader->set();
    }

    void icon_(Texture *t, bool overlaid, int x, int y, int size, bool hit)
    {
        float scale = float(size)/max(t->xs, t->ys); //scale and preserve aspect ratio
        float xs = t->xs*scale, ys = t->ys*scale;
        x += int((size-xs)/2);
        y += int((size-ys)/2);
        const vec &color = hit ? vec(1, 0.5f, 0.5f) : vec(1, 1, 1);
        glBindTexture(GL_TEXTURE_2D, t->id);
        if(hit && actionon)
        {
            varray::colorf(0, 0, 0, 0.75f);
            rect_(x+SHADOW, y+SHADOW, xs, ys, 0);
        }
        varray::color(color);
        rect_(x, y, xs, ys, 0);

        if(overlaid)
        {
            if(!overlaytex) overlaytex = textureload("data/guioverlay.png", 3);
            glBindTexture(GL_TEXTURE_2D, overlaytex->id);
            varray::colorf(1, 1, 1);
            rect_(x, y, xs, ys, 0);
        }
    }        

    void previewslot(VSlot &vslot, bool overlaid, int x, int y, int size, bool hit)
    {
        Slot &slot = *vslot.slot;
        if(slot.sts.empty()) return;
        VSlot *layer = NULL;
        Texture *t = NULL, *glowtex = NULL, *layertex = NULL;
        if(slot.loaded)
        {
            t = slot.sts[0].t;
            if(t == notexture) return;
            Slot &slot = *vslot.slot;
            if(slot.texmask&(1<<TEX_GLOW)) { loopvj(slot.sts) if(slot.sts[j].type==TEX_GLOW) { glowtex = slot.sts[j].t; break; } }
            if(vslot.layer)
            {
                layer = &lookupvslot(vslot.layer);
                if(!layer->slot->sts.empty()) layertex = layer->slot->sts[0].t;
            }
        }
        else if(slot.thumbnail && slot.thumbnail != notexture) t = slot.thumbnail;
        else return;
        float xt = min(1.0f, t->xs/(float)t->ys), yt = min(1.0f, t->ys/(float)t->xs), xs = size, ys = size;
        if(hit && actionon) 
        {
            hudnotextureshader->set();
            varray::colorf(0, 0, 0, 0.75f);
            rect_(x+SHADOW, y+SHADOW, xs, ys);
            hudshader->set();	
        }
        SETSHADER(hudrgb);
        const vec &color = hit ? vec(1, 0.5f, 0.5f) : vec(1, 1, 1);
        vec2 tc[4] = { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1) };
        int xoff = vslot.offset.x, yoff = vslot.offset.y;
        if(vslot.rotation)
        {
            if((vslot.rotation&5) == 1) { swap(xoff, yoff); loopk(4) swap(tc[k].x, tc[k].y); }
            if(vslot.rotation >= 2 && vslot.rotation <= 4) { xoff *= -1; loopk(4) tc[k].x *= -1; }
            if(vslot.rotation <= 2 || vslot.rotation == 5) { yoff *= -1; loopk(4) tc[k].y *= -1; }
        }
        loopk(4) { tc[k].x = tc[k].x/xt - float(xoff)/t->xs; tc[k].y = tc[k].y/yt - float(yoff)/t->ys; }
        varray::color(slot.loaded ? vec(color).mul(vslot.colorscale) : color);
        glBindTexture(GL_TEXTURE_2D, t->id);
        varray::defvertex(2);
        varray::deftexcoord0();
        varray::begin(GL_TRIANGLE_STRIP);
        varray::attribf(x,    y);    varray::attrib(tc[0]);
        varray::attribf(x+xs, y);    varray::attrib(tc[1]);
        varray::attribf(x,    y+ys); varray::attrib(tc[3]);
        varray::attribf(x+xs, y+ys); varray::attrib(tc[2]);
        varray::end();
        if(glowtex)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBindTexture(GL_TEXTURE_2D, glowtex->id);
            varray::color(hit || overlaid ? vec(color).mul(vslot.glowcolor) : vslot.glowcolor);
            varray::begin(GL_TRIANGLE_STRIP);
            varray::attribf(x,    y);    varray::attrib(tc[0]);
            varray::attribf(x+xs, y);    varray::attrib(tc[1]);
            varray::attribf(x,    y+ys); varray::attrib(tc[3]);
            varray::attribf(x+xs, y+ys); varray::attrib(tc[2]);
            varray::end();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if(layertex)
        {
            glBindTexture(GL_TEXTURE_2D, layertex->id);
            varray::color(vec(color).mul(layer->colorscale));
            varray::begin(GL_TRIANGLE_STRIP);
            varray::attribf(x+xs/2, y+ys/2); varray::attrib(tc[0]);
            varray::attribf(x+xs,   y+ys/2); varray::attrib(tc[1]);
            varray::attribf(x+xs/2, y+ys);   varray::attrib(tc[3]);
            varray::attribf(x+xs,   y+ys);   varray::attrib(tc[2]);
            varray::end();
        }
            
        hudshader->set();
        if(overlaid) 
        {
            if(!overlaytex) overlaytex = textureload("data/guioverlay.png", 3);
            glBindTexture(GL_TEXTURE_2D, overlaytex->id);
            varray::colorf(1, 1, 1);
            rect_(x, y, xs, ys, 0);
        }
    }

    void line_(int size, float percent = 1.0f)
    {		
        if(visible())
        {
            if(!slidertex) slidertex = textureload("data/guislider.png", 3);
            glBindTexture(GL_TEXTURE_2D, slidertex->id);
            if(percent < 0.99f) 
            {
                varray::colorf(1, 1, 1, 0.375f);
                if(ishorizontal()) 
                    rect_(curx + FONTH/2 - size/2, cury, size, ysize, 0);
                else
                    rect_(curx, cury + FONTH/2 - size/2, xsize, size, 1);
            }
            varray::colorf(1, 1, 1);
            if(ishorizontal()) 
                rect_(curx + FONTH/2 - size/2, cury + ysize*(1-percent), size, ysize*percent, 0);
            else 
                rect_(curx, cury + FONTH/2 - size/2, xsize*percent, size, 1);
        }
        layout(ishorizontal() ? FONTH : 0, ishorizontal() ? 0 : FONTH);
    }

    void textbox(const char *text, int width, int height, int color) 
    {
        width *= FONTW;
        height *= FONTH;
        int w, h;
        text_bounds(text, w, h, width);
        if(h > height) height = h;
        if(visible()) draw_text(text, curx, cury, color>>16, (color>>8)&0xFF, color&0xFF, 0xFF, -1, width);
        layout(width, height);
    }

    int button_(const char *text, int color, const char *icon, bool clickable, bool center)
    {
        const int padding = 10;
        int w = 0;
        if(icon) w += ICON_SIZE;
        if(icon && text) w += padding;
        if(text) w += text_width(text);
    
        if(visible())
        {
            bool hit = ishit(w, FONTH);
            if(hit && clickable) color = 0xFF0000;	
            int x = curx;	
            if(isvertical() && center) x += (xsize-w)/2;
        
            if(icon)
            {
                if(icon[0] != ' ')
                {
                    const char *ext = strrchr(icon, '.');
                    defformatstring(tname)("packages/icons/%s%s", icon, ext ? "" : ".jpg");
                    icon_(textureload(tname, 3), false, x, cury, ICON_SIZE, clickable && hit);
                }
                x += ICON_SIZE;
            }
            if(icon && text) x += padding;
            if(text) text_(text, x, cury, color, center || (hit && clickable && actionon), hit && clickable);
        }
        return layout(w, FONTH);
    }

    static Texture *skintex, *overlaytex, *slidertex;
    static const int skinx[], skiny[];
    static const struct patch { ushort left, right, top, bottom; uchar flags; } patches[];

    static void drawskin(int x, int y, int gapw, int gaph, int start, int n, float alpha = 0.80f)//int vleft, int vright, int vtop, int vbottom, int start, int n) 
    {
        if(!skintex) skintex = textureload("data/guiskin.png", 3);
        glBindTexture(GL_TEXTURE_2D, skintex->id);
        int gapx1 = INT_MAX, gapy1 = INT_MAX, gapx2 = INT_MAX, gapy2 = INT_MAX;
        float wscale = 1.0f/(SKIN_W*SKIN_SCALE), hscale = 1.0f/(SKIN_H*SKIN_SCALE);
        bool quads = false;
        varray::colorf(1, 1, 1, alpha);
        loopi(n)
        {
            const patch &p = patches[start+i];
            int left = skinx[p.left]*SKIN_SCALE, right = skinx[p.right]*SKIN_SCALE,
                top = skiny[p.top]*SKIN_SCALE, bottom = skiny[p.bottom]*SKIN_SCALE;
            float tleft = left*wscale, tright = right*wscale,
                  ttop = top*hscale, tbottom = bottom*hscale;
            if(p.flags&0x1)
            {
                gapx1 = left;
                gapx2 = right;
            }
            else if(left >= gapx2)
            {
                left += gapw - (gapx2-gapx1);
                right += gapw - (gapx2-gapx1);
            }
            if(p.flags&0x10)
            {
                gapy1 = top;
                gapy2 = bottom;
            }
            else if(top >= gapy2)
            {
                top += gaph - (gapy2-gapy1);
                bottom += gaph - (gapy2-gapy1);
            }
           
            //multiple tiled quads if necessary rather than a single stretched one
            int ystep = bottom-top;
            int yo = y+top;
            while(ystep > 0) 
            {
                if(p.flags&0x10 && yo+ystep-(y+top) > gaph) 
                {
                    ystep = gaph+y+top-yo;
                    tbottom = ttop+ystep*hscale;
                }
                int xstep = right-left;
                int xo = x+left;
                float tright2 = tright;
                while(xstep > 0) 
                {
                    if(p.flags&0x01 && xo+xstep-(x+left) > gapw) 
                    {
                        xstep = gapw+x+left-xo; 
                        tright = tleft+xstep*wscale;
                    }
                    if(!quads) { quads = true; varray::defvertex(2); varray::deftexcoord0(); varray::begin(GL_QUADS); }
                    varray::attribf(xo,       yo);       varray::attribf(tleft,  ttop);
                    varray::attribf(xo+xstep, yo);       varray::attribf(tright, ttop);
                    varray::attribf(xo+xstep, yo+ystep); varray::attribf(tright, tbottom);
                    varray::attribf(xo,       yo+ystep); varray::attribf(tleft,  tbottom);
                    if(!(p.flags&0x01)) break;
                    xo += xstep;
                }
                tright = tright2;
                if(!(p.flags&0x10)) break;
                yo += ystep;
            }
        }
        if(quads) xtraverts += varray::end();
    } 

    vec origin, scale, *savedorigin;
    float dist;
    g3d_callback *cb;

    static float basescale, maxscale;
    static bool passthrough;
    static float alpha;

    void adjustscale()
    {
        int w = xsize + (skinx[2]-skinx[1])*SKIN_SCALE + (skinx[10]-skinx[9])*SKIN_SCALE, h = ysize + (skiny[9]-skiny[7])*SKIN_SCALE;
        if(tcurrent) h += ((skiny[5]-skiny[1])-(skiny[3]-skiny[2]))*SKIN_SCALE + FONTH-2*INSERT;
        else h += (skiny[6]-skiny[3])*SKIN_SCALE;

        float aspect = forceaspect ? 1.0f/forceaspect : float(screenh)/float(screenw), fit = 1.0f;
        if(w*aspect*basescale>1.0f) fit = 1.0f/(w*aspect*basescale);
        if(h*basescale*fit>maxscale) fit *= maxscale/(h*basescale*fit);
        origin = vec(0.5f-((w-xsize)/2 - (skinx[2]-skinx[1])*SKIN_SCALE)*aspect*scale.x*fit, 0.5f + (0.5f*h-(skiny[9]-skiny[7])*SKIN_SCALE)*scale.y*fit, 0);
        scale = vec(aspect*scale.x*fit, scale.y*fit, 1);
    }

    void start(int starttime, float initscale, int *tab, bool allowinput)
    {	
        initscale *= 0.025f; 
        if(allowinput) hascursor = true;
        basescale = initscale;
        if(layoutpass) scale.x = scale.y = scale.z = guifadein ? basescale*min((totalmillis-starttime)/300.0f, 1.0f) : basescale;
        alpha = allowinput ? 0.80f : 0.60f;
        passthrough = scale.x<basescale || !allowinput;
        curdepth = -1;
        curlist = -1;
        tpos = 0;
        tx = 0;
        ty = 0;
        tcurrent = tab;
        tcolor = 0xFFFFFF;
        pushlist();
        if(layoutpass) 
        {
            firstlist = nextlist = curlist;
            memset(columns, 0, sizeof(columns));
        }
        else
        {
            if(tcurrent && !*tcurrent) tcurrent = NULL;
            cury = -ysize; 
            curx = -xsize/2;
           
            pushhudmatrix();
            hudmatrix.translate(origin);
            hudmatrix.scale(scale); 
            flushhudmatrix();

            drawskin(curx-skinx[2]*SKIN_SCALE, cury-skiny[6]*SKIN_SCALE, xsize, ysize, 0, 9, alpha);
            if(!tcurrent) drawskin(curx-skinx[5]*SKIN_SCALE, cury-skiny[6]*SKIN_SCALE, xsize, 0, 9, 1, alpha);
        }
    }

    void adjusthorizontalcolumn(int col, int i)
    {
        int h = columns[col], dh = 0;
        for(int d = 1; i >= 0; d ^= 1)
        {
            list &p = lists[i];
            if(d&1) { dh = h - p.h; if(dh <= 0) break; p.h = h; }
            else { p.h += dh; h = p.h; }
            i = p.parent;
        }
        ysize += max(dh, 0);
    }

    void adjustverticalcolumn(int col, int i)
    {
        int w = columns[col], dw = 0;
        for(int d = 0; i >= 0; d ^= 1)
        {
            list &p = lists[i];
            if(d&1) { p.w += dw; w = p.w; }
            else { dw = w - p.w; if(dw <= 0) break; p.w = w; }
            i = p.parent;
        }
        xsize = max(xsize, w);
    }
        
    void adjustcolumns()
    {
        if(lists.inrange(curlist))
        {
            list &l = lists[curlist];
            if(l.column >= 0) columns[l.column] = max(columns[l.column], ishorizontal() ? ysize : xsize);
        }
        int parent = -1, depth = 0;
        for(int i = firstlist; i < lists.length(); i++)
        {
            list &l = lists[i];
            if(l.parent > parent) { parent = l.parent; depth++; }
            else if(l.parent < parent) { parent = l.parent; depth--; }
            if(l.column >= 0)
            {
                if(depth&1) adjusthorizontalcolumn(l.column, i); 
                else adjustverticalcolumn(l.column, i);
            }
        }
    }

    void end()
    {
        if(layoutpass)
        {	
            adjustcolumns();
            xsize = max(tx, xsize);
            ysize = max(ty, ysize);
            ysize = max(ysize, (skiny[7]-skiny[6])*SKIN_SCALE);
            if(tcurrent) *tcurrent = max(1, min(*tcurrent, tpos));
            adjustscale();
            if(!windowhit && !passthrough)
            {
                hitx = (cursorx - origin.x)/scale.x;
                hity = (cursory - origin.y)/scale.y;
                if((mousebuttons & G3D_PRESSED) && (fabs(hitx-firstx) > 2 || fabs(hity - firsty) > 2)) mousebuttons |= G3D_DRAGGED;
                if(hitx>=-xsize/2 && hitx<=xsize/2 && hity<=0)
                {
                    if(hity>=-ysize || (tcurrent && hity>=-ysize-(FONTH-2*INSERT)-((skiny[6]-skiny[1])-(skiny[3]-skiny[2]))*SKIN_SCALE && hitx<=tx-xsize/2))
                        windowhit = this;
                }
            }
        }
        else
        {
            if(tcurrent && tx<xsize) drawskin(curx+tx-skinx[5]*SKIN_SCALE, -ysize-skiny[6]*SKIN_SCALE, xsize-tx, FONTH, 9, 1, alpha);
            pophudmatrix();
        }
        poplist();
    }

    void draw()
    {
        cb->gui(*this, layoutpass);
    }
};

Texture *gui::skintex = NULL, *gui::overlaytex = NULL, *gui::slidertex = NULL;

//chop skin into a grid
const int gui::skiny[] = {0, 7, 21, 34, 43, 48, 56, 104, 111, 117, 128},
          gui::skinx[] = {0, 11, 23, 37, 105, 119, 137, 151, 215, 229, 246, 256};
//Note: skinx[3]-skinx[2] = skinx[7]-skinx[6]
//      skinx[5]-skinx[4] = skinx[9]-skinx[8]		 
const gui::patch gui::patches[] = 
{ //arguably this data can be compressed - it depends on what else needs to be skinned in the future
    {1,2,3,6,  0},    // body
    {2,9,5,6,  0x01},
    {9,10,3,6, 0},

    {1,2,6,7,  0x10},
    {2,9,6,7,  0x11},
    {9,10,6,7, 0x10},

    {1,2,7,9,  0},
    {2,9,7,9,  0x01},
    {9,10,7,9, 0},

    {5,6,3,5, 0x01}, // top

    {2,3,1,2, 0},    // selected tab
    {3,4,1,2, 0x01},
    {4,5,1,2, 0},
    {2,3,2,3, 0x10},
    {3,4,2,3, 0x11},
    {4,5,2,3, 0x10},
    {2,3,3,5, 0},
    {3,4,3,5, 0x01},
    {4,5,3,5, 0},

    {6,7,1,2, 0},    // deselected tab
    {7,8,1,2, 0x01},
    {8,9,1,2, 0},
    {6,7,2,3, 0x10},
    {7,8,2,3, 0x11},
    {8,9,2,3, 0x10},
    {6,7,3,5, 0},
    {7,8,3,5, 0x01},
    {8,9,3,5, 0},
};

vector<gui::list> gui::lists;
float gui::basescale, gui::maxscale = 1, gui::hitx, gui::hity, gui::alpha;
bool gui::passthrough, gui::shouldmergehits = false, gui::shouldautotab = true;
int gui::curdepth, gui::curlist, gui::xsize, gui::ysize, gui::curx, gui::cury;
int gui::ty, gui::tx, gui::tpos, *gui::tcurrent, gui::tcolor;
static vector<gui> guis2d;

VARP(guipushdist, 1, 4, 64);

bool g3d_input(const char *str, int len)
{
    editor *e = currentfocus();
    if(fieldmode == FIELDKEY || fieldmode == FIELDSHOW || !e) return false;
    
    e->input(str, len);
    return true;
}

bool g3d_key(int code, bool isdown)
{
    editor *e = currentfocus();
    if(fieldmode == FIELDKEY)
    {
        switch(code)
        {
            case SDLK_ESCAPE:
                if(isdown) fieldmode = FIELDCOMMIT;
                return true;
        }
        const char *keyname = getkeyname(code);
        if(keyname && isdown)
        {
            if(e->lines.length()!=1 || !e->lines[0].empty()) e->insert(" ");
            e->insert(keyname);
        }
        return true;
    }

    if(code==-1 && g3d_windowhit(isdown, true)) return true;
    else if(code==-3 && g3d_windowhit(isdown, false)) return true;

    if(fieldmode == FIELDSHOW || !e)
    {
        if(windowhit) switch(code)
        {
            case -4: // window "management" 
                if(isdown)
                {
                    vec origin = *guis2d.last().savedorigin;
                    int i = windowhit - &guis2d[0];
                    for(int j = guis2d.length()-1; j > i; j--) *guis2d[j].savedorigin = *guis2d[j-1].savedorigin;
                    *windowhit->savedorigin = origin;
                    if(guis2d.length() > 1)
                    {
                        if(camera1->o.dist(*windowhit->savedorigin) <= camera1->o.dist(*guis2d.last().savedorigin))
                            windowhit->savedorigin->add(camdir);
                    }
                }
                return true;
            case -5:
                if(isdown)
                {
                    vec origin = *guis2d[0].savedorigin;
                    loopj(guis2d.length()-1) *guis2d[j].savedorigin = *guis2d[j + 1].savedorigin;
                    *guis2d.last().savedorigin = origin;
                    if(guis2d.length() > 1)
                    {
                        if(camera1->o.dist(*guis2d.last().savedorigin) >= camera1->o.dist(*guis2d[0].savedorigin))
                            guis2d.last().savedorigin->sub(camdir);
                    }
                }
                return true;
        }

        return false;
    }
    switch(code)
    {
        case SDLK_ESCAPE: //cancel editing without commit
            if(isdown) fieldmode = FIELDABORT;
            return true;
        case SDLK_RETURN:
        case SDLK_TAB:
            if(e->maxy != 1) break;
        case SDLK_KP_ENTER:
            if(isdown) fieldmode = FIELDCOMMIT; //signal field commit (handled when drawing field)
            return true;
    }
    if(isdown) e->key(code);
    return true;
}

void g3d_cursorpos(float &x, float &y)
{
    if(guis2d.length()) { x = cursorx; y = cursory; }
    else x = y = 0.5f;
}

void g3d_resetcursor()
{
    cursorx = cursory = 0.5f;
}

FVARP(guisens, 1e-3f, 1, 1e3f);

bool g3d_movecursor(int dx, int dy)
{
    if(!guis2d.length() || !hascursor) return false;
    const float CURSORSCALE = 500.0f;
    cursorx = max(0.0f, min(1.0f, cursorx+guisens*dx*(screenh/(screenw*CURSORSCALE))));
    cursory = max(0.0f, min(1.0f, cursory+guisens*dy/CURSORSCALE));
    return true;
}

void g3d_addgui(g3d_callback *cb, vec &origin, int flags)
{
    gui &g = guis2d.add();
    g.cb = cb;
    g.origin = origin;
    g.savedorigin = &origin;
    g.dist = flags&GUI_BOTTOM ? 1e16f : camera1->o.dist(g.origin);
}

void g3d_limitscale(float scale)
{
    gui::maxscale = scale;
}

static inline bool g3d_sort(const gui &a, const gui &b) { return a.dist < b.dist; }

bool g3d_windowhit(bool on, bool act)
{
    extern int cleargui(int n);
    if(act) 
    {
        if(actionon || windowhit)
        {
            if(on) { firstx = gui::hitx; firsty = gui::hity; }
            mousebuttons |= (actionon=on) ? G3D_DOWN : G3D_UP;
        }
    } else if(!on && windowhit) cleargui(1);
    return guis2d.length() && hascursor;
}

void g3d_render()   
{
    windowhit = NULL;    
    if(actionon) mousebuttons |= G3D_PRESSED;
   
    gui::reset(); 
    guis2d.shrink(0);
 
    // call all places in the engine that may want to render a gui from here, they call g3d_addgui()
    extern void g3d_texturemenu();
    
    if(!mainmenu) g3d_texturemenu();
    g3d_mainmenu();
    if(!mainmenu) game::g3d_gamemenus();

    guis2d.sort(g3d_sort);
    
    readyeditors();
    bool wasfocused = (fieldmode!=FIELDSHOW);
    fieldsactive = false;

    hascursor = false;

    layoutpass = true;
    loopv(guis2d) guis2d[i].draw();
    layoutpass = false;

    if(guis2d.length())
    {
        hudmatrix.ortho(0, 1, 1, 0, -1, 1);
        resethudmatrix();
        hudshader->set();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        loopvrev(guis2d) guis2d[i].draw();

        glDisable(GL_BLEND);

        varray::disable();
    }

    flusheditors();
    if(!fieldsactive) fieldmode = FIELDSHOW; //didn't draw any fields, so loose focus - mainly for menu closed
    if((fieldmode!=FIELDSHOW) != wasfocused) 
    {
        textinput(fieldmode!=FIELDSHOW, TI_GUI);
        keyrepeat(fieldmode!=FIELDSHOW, KR_GUI);
    }
    
    mousebuttons = 0;
}

void consolebox(int x1, int y1, int x2, int y2)
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float bw = x2 - x1, bh = y2 - y1, aspect = bw/bh, sh = bh, sw = sh*aspect;
    bw *= float(4*FONTH)/(SKIN_H*SKIN_SCALE);
    bh *= float(4*FONTH)/(SKIN_H*SKIN_SCALE);
    sw /= bw + (gui::skinx[2]-gui::skinx[1] + gui::skinx[10]-gui::skinx[9])*SKIN_SCALE;
    sh /= bh + (gui::skiny[9]-gui::skiny[7] + gui::skiny[6]-gui::skiny[4])*SKIN_SCALE;
    pushhudmatrix();
    hudmatrix.translate(x1, y1, 0);
    hudmatrix.scale(sw, sh, 1);
    flushhudmatrix();
    gui::drawskin(-gui::skinx[1]*SKIN_SCALE, -gui::skiny[4]*SKIN_SCALE, int(bw), int(bh), 0, 9, 0.60f);
    gui::drawskin((-gui::skinx[1] + gui::skinx[2] - gui::skinx[5])*SKIN_SCALE, -gui::skiny[4]*SKIN_SCALE, int(bw), 0, 9, 1, 0.60f);
    pophudmatrix();
}

