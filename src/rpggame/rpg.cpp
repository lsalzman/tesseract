#include "rpg.h"

namespace game
{
    #define N(n) int stats::pointscale_##n, stats::percentscale_##n; 
    RPGSTATNAMES
    #undef N

    rpgent *player1 = NULL;

    int maptime = 0;
    string mapname = "";
      
    void updateworld()
    {
        if(!maptime) { maptime = lastmillis; return; }
        if(!curtime) return;
        physicsframe();
        updateobjects();
        player1->updateplayer();
    }

    bool connected = false;

    void gameconnect(bool _remote) 
    { 
        connected = true; 
        if(editmode) toggleedit();
    }

    void gamedisconnect(bool cleanup) 
    { 
        connected = false; 
    }

    void changemap(const char *name) 
    { 
        if(!connected) localconnect();
        if(editmode) toggleedit();
        if(!load_world(name)) emptymap(0, true, name); 
    }

    void forceedit(const char *name) 
    { 
        changemap(name); 
    }

    ICOMMAND(map, "s", (char *s), changemap(s));
    
    struct rpgmenu : g3d_callback
    {
        int time, tab, which;
        vec pos;
        
        rpgmenu() : time(0), tab(1), which(0), pos(0, 0, 0) {}

        void show(int n)
        {
            if((time && n==which) || !n)
                time = 0;
            else
            {
                time = starttime();
                pos  = menuinfrontofplayer();        
                which = n;
            }
        }

        void gui(g3d_gui &g, bool firstpass)
        {
            g.start(time, 0.03f, &tab);
            switch(which)
            {
                default:
                case 1:
                    g.tab("inventory", 0xFFFFF);
                    playerobj->invgui(g);
                    break;

                case 2:
                    g.tab("stats", 0xFFFFF);
                    playerobj->st_show(g);
                    break;

                case 3:
                    g.tab("active quests", 0xFFFFF);
                    listquests(false, g);
                    g.tab("completed quests", 0xFFFFF);
                    listquests(true, g);
                    break;
            }
            g.end();
        }

        void render()
        {
            if(time) g3d_addgui(this, pos, GUI_2D);
        }
    } menu;
    ICOMMAND(showplayergui, "i", (int *which), menu.show(*which));

    void initclient()
    {
        player1 = new rpgent(playerobj, vec(0, 0, 0), 0, 100, ENT_PLAYER);
        clearworld();
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        if     (waterlevel>0) playsoundname("free/splash1", d==player1 ? NULL : &d->o);
        else if(waterlevel<0) playsoundname("free/splash2", d==player1 ? NULL : &d->o);
        if     (floorlevel>0) { if(local) playsoundname("aard/jump"); else if(d->type==ENT_AI) playsoundname("aard/jump", &d->o); }
        else if(floorlevel<0) { if(local) playsoundname("aard/land"); else if(d->type==ENT_AI) playsoundname("aard/land", &d->o); }    
    }

    void dynentcollide(physent *d, physent *o, const vec &dir) {}
   
    void bounced(physent *d, const vec &surface) {}
 
    void edittrigger(const selinfo &sel, int op, int arg1, int arg2, int arg3) {}
    void vartrigger(ident *id) {}

    const char *getclientmap() { return mapname; }
    const char *getmapinfo() { return NULL; }
    void resetgamestate() {}
    void suicide(physent *d) {}
    void newmap(int size) {}

    void startmap(const char *name)
    {
        clearworld();
        copystring(mapname, name ? name : "");
        maptime = 0;
        findplayerspawn(player1);
        if(name) playerobj->st_init();
        entities::startmap();
    }
    
    void quad(int x, int y, int xs, int ys)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2i(x,    y);
        glTexCoord2f(1, 0); glVertex2i(x+xs, y);
        glTexCoord2f(0, 1); glVertex2i(x,    y+ys);
        glTexCoord2f(1, 1); glVertex2i(x+xs, y+ys);
        glEnd();
    }
  
    bool needminimap() { return false; }
 
    float abovegameplayhud(int w, int h)
    {
        switch(player1->state)
        {
            case CS_EDITING:
                return 1;
            default:
                return (h-min(128, h))/float(h);
        }
    }
 
    void gameplayhud(int w, int h)
    {
        glPushMatrix();
        glScalef(0.5f, 0.5f, 1);
        draw_textf("using: %s", 636*2, h*2-256+149, selected ? selected->name : "(none)");       // temp     
        glPopMatrix();

        settexture("packages/hud/hud_rpg.png", 3);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        quad(0, h-128, 768, 128);        
        settexture("packages/hud/hbar.png", 3);
        glColor4f(1, 0, 0, 0.5f);
        quad(130, h-128+57, 193*playerobj->s_hp/playerobj->eff_maxhp(), 17);        
        glColor4f(0, 0, 1, 0.5f);
        quad(130, h-128+87, 193*playerobj->s_mana/playerobj->eff_maxmana(), 17);        
    }
   
    int clipconsole(int w, int h)
    {
        return 0;
    }
 
    void preload() {}

    void renderavatar()
    {
    }

    bool canjump() { return true; }
    bool allowmove(physent *d) { return d->type!=ENT_AI || ((rpgent *)d)->allowmove(); }
    void doattack(bool on) { player1->attacking = on; }
    dynent *iterdynents(int i) { return i ? objects[i-1]->ent : player1; }
    int numdynents() { return objects.length()+1; }

    void rendergame(bool mainpass)
    {
        if(isthirdperson()) renderclient(player1, "ogro", NULL, 0, ANIM_ATTACK1, 300, player1->lastaction, player1->lastpain);
        renderobjects();
    }
    
    void g3d_gamemenus() { g3d_npcmenus(); menu.render(); }

    const char *defaultcrosshair(int index)
    {
        return "data/crosshair.png";
    }

    int selectcrosshair(float &r, float &g, float &b)
    {
        if(player1->state==CS_DEAD) return -1;
        return 0;
    }

    bool serverinfostartcolumn(g3d_gui *g, int i) { return false; }
    void serverinfoendcolumn(g3d_gui *g, int i) {}
    bool serverinfoentry(g3d_gui *g, int i, const char *name, int port, const char *sdesc, const char *map, int ping, const vector<int> &attr, int np) { return false; }

    void setupcamera()
    {
    }

    bool detachcamera()
    {
        return player1->state==CS_DEAD;
    }

    bool collidecamera()
    {
        return player1->state!=CS_EDITING;
    }

    void lighteffects(dynent *e, vec &color, vec &dir)
    {
    }

    void adddynlights()
    {
    }

    void dynlighttrack(physent *owner, vec &o, vec &hud)
    {
    }

    void particletrack(physent *owner, vec &o, vec &d)
    {
    }

    void writegamedata(vector<char> &extras) {}
    void readgamedata (vector<char> &extras) {}

    const char *gameident()     { return "rpg"; }
    const char *savedconfig()   { return "rpg_config.cfg"; }
    const char *restoreconfig() { return "rpg_restore.cfg"; }
    const char *defaultconfig() { return "data/defaults.cfg"; }
    const char *autoexec()      { return "rpg_autoexec.cfg"; }
    const char *savedservers()  { return NULL; }

    void loadconfigs() {}

    void parseoptions(vector<const char *> &args) {}
    void connectattempt(const char *name, const char *password, const ENetAddress &address) {}
    void connectfail() {}
    void parsepacketclient(int chan, packetbuf &p) {}
    bool allowedittoggle() { return true; }
    void edittoggled(bool on) {}
    void writeclientinfo(stream *f) {}
    void toserver(char *text) {}
    bool ispaused() { return false; }
}

namespace server
{
    void *newclientinfo() { return NULL; }
    void deleteclientinfo(void *ci) {}
    void serverinit() {}
    int reserveclients() { return 0; }
    int numchannels() { return 0; }
    void clientdisconnect(int n) {}
    int clientconnect(int n, uint ip) { return DISC_NONE; }
    void localdisconnect(int n) {}
    void localconnect(int n) {}
    bool allowbroadcast(int n) { return true; }
    void recordpacket(int chan, void *data, int len) {}
    void parsepacket(int sender, int chan, packetbuf &p) {}
    void sendservmsg(const char *s) {}
    bool sendpackets(bool force) { return false; }
    void serverinforeply(ucharbuf &req, ucharbuf &p) {}
    void serverupdate() {}
    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np) { return true; }

    int serverinfoport(int servport) { return 0; }
    int serverport(int infoport) { return 0; }
    const char *defaultmaster() { return ""; }
    int masterport() { return 0; }
    int laninfoport() { return 0; }
    void processmasterinput(const char *cmd, int cmdlen, const char *args) {}
    bool ispaused() { return false; }
}

