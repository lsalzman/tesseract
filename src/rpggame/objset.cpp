#include "rpg.h"

namespace game
{
    vector<rpgobj *> objects, stack;
    hashtable<char *, char *> names;
    
    rpgobj *pointingat = NULL;
    rpgobj *playerobj = NULL;
    rpgobj *selected = NULL;
    
    rpgquest *quests = NULL;
    rpgquest *currentquest = NULL;
    
    void resetstack()
    {
        stack.setsize(0);
        loopi(10) stack.add(playerobj);     // determines the stack depth    
    }
    
    void clearworld()
    {
        if(playerobj) { playerobj->ent = NULL; delete playerobj; }
        playerobj = new rpgobj("player");
        playerobj->ent = player1;
        player1->ro = playerobj;

        pointingat = NULL;
        objects.deletecontents();
        resetstack();
        
        playerobj->scriptinit();            // will fail when this is called from emptymap(), which is ok
    }
    
    void removefromsystem(rpgobj *o)
    {
        removefromworld(o);
        o->decontain();
        if(pointingat==o) pointingat = NULL;
        if(selected==o) selected = NULL;
        resetstack();
        DELETEP(o);
    }
    
    void updateobjects()
    {
        pointingat = NULL;
        loopv(objects)
        {
            objects[i]->update();

            float dist = player1->o.dist(objects[i]->ent->o);
            if(dist<50 && intersect(objects[i]->ent, player1->o, worldpos) && (!pointingat || player1->o.dist(pointingat->ent->o)>dist))    
            {    
                pointingat = objects[i]; 
            }
        }
    }
    
    void spawn(char *name)
    {
        rpgobj *o = new rpgobj(name);
        pushobj(o);
        o->scriptinit();
    }
    
    void placeinworld(vec &pos, float yaw)
    {
        stack[0]->placedinworld(new rpgent(stack[0], pos, yaw));
        objects.add(stack[0]);
    }
   
    void spawnfroment(rpgentity &e)
    {
        spawn(e.name);
        placeinworld(e.o, e.attr1);
    }
 
    void pushobj(rpgobj *o) { stack.pop(); stack.insert(0, o); }       // never overflows, just removes bottom
    void popobj()           { stack.add(stack.remove(0)); }            // never underflows, just puts it at the bottom
    
    void removefromworld(rpgobj *worldobj)
    {
        objects.removeobj(worldobj);
        DELETEP(worldobj->ent);    
    }
    
    void taken(rpgobj *worldobj, rpgobj *newowner)
    {
        removefromworld(worldobj);
        newowner->add(worldobj, false);
    }
    
    void takefromplayer(char *name, char *ok, char *notok)
    {
        rpgobj *o = playerobj->take(name);
        if(o)
        {
            stack[0]->add(o, false);
            conoutf("\f2you hand over a %s", o->name);
            if(currentquest)
            {
                conoutf("\f2you finish a quest for %s", currentquest->npc); 
                currentquest->completed = true;           
            }
        }
        execute(o ? ok : notok);
    }
    
    void givetoplayer(char *name)
    {
        rpgobj *o = stack[0]->take(name);
        if(o)
        {
            conoutf("\f2you receive a %s", o->name);
            playerobj->add(o, false);
        }
    }
    
    rpgquest *addquest(const char *questline, const char *npc)
    {
        quests = new rpgquest(quests, npc, questline);
        conoutf("\f2you have accepted a quest for %s", npc);
        return quests;
    }
    
    void listquests(bool completed, g3d_gui &g)
    {
        for(rpgquest *q = quests; q; q = q->next) if(q->completed==completed)
        {
            defformatstring(info)("%s: %s", q->npc, q->questline);
            g.text(info, 0xAAAAAA, "info");
        }
    }
    
    char *stringpool(char *name)
    {
        char **n = names.access(name);
        if(n) return *n;
        name = newstring(name);
        names[name] = name;
        return name;
    }
    
    void renderobjects() { loopv(objects) objects[i]->render();   }
    void g3d_npcmenus() { loopv(objects) objects[i]->g3d_menu(); } 

    #define N(n) ICOMMAND(r_##n,     "i", (int *val), { stack[0]->s_##n = *val; }); \
                 ICOMMAND(r_get_##n, "",  (), { intret(stack[0]->s_##n); }); 

    RPGNAMES
    #undef N
    #define N(n) ICOMMAND(r_def_##n, "ii", (int *i1, int *i2), { stats::def_##n(*i1, *i2); }); \
                 ICOMMAND(r_eff_##n, "",   (), { intret(stack[0]->eff_##n()); }); 
    RPGSTATNAMES
    #undef N

    ICOMMAND(r_model,       "s",   (char *s), { stack[0]->model = stringpool(s); });    
    ICOMMAND(r_spawn,       "s",   (char *s), { spawn(stringpool(s)); });    
    ICOMMAND(r_contain,     "s",   (char *s), { stack[0]->decontain(); stack[1]->add(stack[0], atoi(s)); });     
    ICOMMAND(r_pop,         "",    (), { popobj(); });    
    ICOMMAND(r_swap,        "",    (), { swap(stack[0], stack[1]); });    
    ICOMMAND(r_say,         "s",   (char *s), { stack[0]->abovetext = stringpool(s); });    
    ICOMMAND(r_quest,       "ss",  (char *s, char *a), { stack[0]->addaction(stringpool(s), stringpool(a), true); });    
    ICOMMAND(r_action,      "ss",  (char *s, char *a), { stack[0]->addaction(stringpool(s), stringpool(a), false); });    
    ICOMMAND(r_action_use,  "s",   (char *s), { stack[0]->action_use.script = stringpool(s); });    
    ICOMMAND(r_take,        "sss", (char *name, char *ok, char *notok), { takefromplayer(name, ok, notok); });    
    ICOMMAND(r_give,        "s",   (char *s), { givetoplayer(s); });    
    ICOMMAND(r_use,         "",    (), { stack[0]->selectuse(); });    
    ICOMMAND(r_applydamage, "i",   (int *d), { stack[0]->takedamage(*d, *stack[1]); });    
}

