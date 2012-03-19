#include "cube.h"

enum { ETR_SPAWN = ET_GAMESPECIFIC, };

static const int SPAWNNAMELEN = 64;

struct rpgentity : extentity
{
    char name[SPAWNNAMELEN];

    rpgentity() { memset(name, 0, SPAWNNAMELEN); }
};

namespace entities
{
    extern vector<extentity *> ents;
    extern void startmap();
}

namespace game
{
    struct rpgent;
    struct rpgobj;
    struct rpgaction;

    struct rpgquest
    {
        rpgquest *next;
        const char *npc;
        const char *questline;
        bool completed;

        rpgquest(rpgquest *_n, const char *_npc, const char *_ql) : next(_n), npc(_npc), questline(_ql), completed(false) {}
    };

    extern rpgent *player1;
    extern vector<rpgobj *> objects, stack;
    extern rpgobj *pointingat;
    extern rpgobj *playerobj;
    extern rpgobj *selected;
    extern rpgquest *quest;
    extern rpgquest *currentquest;

    extern void pushobj(rpgobj *o);
    extern void popobj();
    extern void spawn(char *name);
    extern void placeinworld(vec &pos, float yaw);
    extern void spawnfroment(rpgentity &e);
    extern void removefromworld(rpgobj *worldobj);
    extern void removefromsystem(rpgobj *o);
    extern void taken(rpgobj *worldobj, rpgobj *newowner);
    extern void clearworld();
    extern void updateobjects();
    extern void renderobjects();
    extern void listquests(bool completed, g3d_gui &g);
    extern rpgquest *addquest(const char *questline, const char *npc);
    extern void g3d_npcmenus();
     
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float &dist = intersectdist);

#include "stats.h"
#include "rpgent.h"
#include "rpgobj.h"
}

