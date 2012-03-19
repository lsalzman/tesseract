struct rpgobj;

struct rpgaction
{
    rpgaction *next;
    char *initiate, *script;
    rpgquest *q;
    bool used;

    rpgaction(char *_i = NULL, char *_s = NULL, rpgaction *_n = NULL) : next(_n), initiate(_i), script(_s), q(NULL), used(false) {}

    ~rpgaction() { DELETEP(next); }

    void exec(rpgobj *obj, rpgobj *target, rpgobj *user)
    {
        if(!*script) return;
        pushobj(user);
        pushobj(target);
        pushobj(obj);
        execute(script);
        used = true;
    }
};

struct rpgobj : g3d_callback, stats
{
    rpgobj *parent;     // container object, if not top level
    rpgobj *inventory;  // contained objects, if any
    rpgobj *sibling;    // used for linking, if contained

    rpgent *ent;        // representation in the world, if top level

    const char *name;   // name it was spawned as
    const char *model;  // what to display it as

    enum
    {
        IF_INVENTORY = 1,   // parent owns this object, will contribute to parent stats
        IF_LOOT      = 2,   // if parent dies, this object should drop to the ground
        IF_TRADE     = 4,   // parent has this item available for trade, for player, all currently unused weapons etc are of this type
    };

    enum
    {
        MENU_DEFAULT,
        MENU_BUY,
        MENU_SELL
    };

    int itemflags;
    
    rpgaction *actions, action_use;
    char *abovetext;    

    int menutime, menutab, menuwhich;

    #define loopinventory() for(rpgobj *o = inventory; o; o = o->sibling)
    #define loopinventorytype(T) loopinventory() if(o->itemflags&(T))

    rpgobj(const char *_name) : parent(NULL), inventory(NULL), sibling(NULL), ent(NULL), name(_name), model(NULL), itemflags(IF_INVENTORY),
        actions(NULL), abovetext(NULL), menutime(0), menutab(1), menuwhich(MENU_DEFAULT) {}

    ~rpgobj()
    {
        DELETEP(inventory);
        DELETEP(sibling);
        DELETEP(ent);
        DELETEP(actions);
    }

    void scriptinit()
    {
        DELETEP(inventory);
        defformatstring(aliasname)("spawn_%s", name);
        if(identexists(aliasname)) execute(aliasname);
    }

    void decontain() 
    {
        if(parent) parent->remove(this);
    }

    void add(rpgobj *o, int itemflags)
    {
        o->sibling = inventory;
        o->parent = this;
        inventory = o;
        o->itemflags = itemflags;
        
        if(itemflags&IF_INVENTORY) recalcstats();
    }

    void remove(rpgobj *o)
    {
        for(rpgobj **l = &inventory; *l; )
            if(*l==o) 
            {
                *l = o->sibling;
                o->sibling = o->parent = NULL;
            }
            else l = &(*l)->sibling;
            
        if(o->itemflags&IF_INVENTORY) recalcstats();
    }

    void recalcstats()
    {
        st_reset();
        loopinventorytype(IF_INVENTORY) st_accumulate(*o);
    }
    
    rpgobj &selectedweapon()
    {
        if(this==playerobj) return selected ? *selected : *this;
        else { loopinventorytype(IF_INVENTORY) if(o->s_usetype) return *o; };
        return *this;
    }
    
    void placedinworld(rpgent *_ent)
    {
        if(!model) model = "tentus/moneybag";
        ent = _ent;
        setbbfrommodel(ent, model);
        entinmap(ent);
        //ASSERT(!(ent->o.x<0 || ent->o.y<0 || ent->o.z<0 || ent->o.x>4096 || ent->o.y>4096 || ent->o.z>4096));
        st_init();
    }

    void render()
    {
        if(s_ai) 
        {
            float sink = 0;
            if(ent->physstate>=PHYS_SLIDE)
                sink = raycube(ent->o, vec(0, 0, -1), 2*ent->eyeheight)-ent->eyeheight;
            ent->sink = ent->sink*0.8 + sink*0.2;
            //if(ent->blocked) particle_splash(PART_SPARK, 100, 100, ent->o, 0xB49B4B, 0.24f);
            float oldheight = ent->eyeheight;
            ent->eyeheight += ent->sink;
            renderclient(ent, model, NULL, 0, ANIM_ATTACK1, 300, ent->lastaction, 0);
            ent->eyeheight = oldheight;
            if(s_hp<eff_maxhp() && ent->state==CS_ALIVE) particle_meter(ent->abovehead(), s_hp/(float)eff_maxhp(), PART_METER, 1, 0xFF1932, 0xFFFFFF, 2.0f);

        }
        else
        {
            rendermodel(NULL, model, ANIM_MAPMODEL|ANIM_LOOP, vec(ent->o).sub(vec(0, 0, ent->eyeheight)), ent->yaw+90, 0, MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED | MDL_LIGHT, ent);
        }
    }

    void update()
    {
        float dist = ent->o.dist(player1->o);
        if(s_ai) { ent->update(dist); st_update(); };
        moveplayer(ent, 1, true);    // 10 or above gets blocked less, because physics accuracy doesn't need extra tests
        //ASSERT(!(ent->o.x<0 || ent->o.y<0 || ent->o.z<0 || ent->o.x>4096 || ent->o.y>4096 || ent->o.z>4096));
        if(!menutime && dist<(s_ai ? 40 : 24) && ent->state==CS_ALIVE && s_ai<2) { menutime = starttime(); menuwhich = MENU_DEFAULT; }
        else if(dist>(s_ai ? 96 : 48)) menutime = 0;
    }

    void addaction(char *initiate, char *script, bool startquest)
    {
        for(rpgaction *a = actions; a; a = a->next) if(strcmp(a->initiate, initiate)==0) return;
        actions = new rpgaction(initiate, script, actions);
        if(startquest) actions->q = addquest(abovetext, name);
    }

    void droploot()
    {
        loopinventorytype(IF_LOOT)
        {
            o->decontain();
            pushobj(o);
            placeinworld(ent->o, rnd(360));
            droploot();
            return;
        }
    }

    rpgobj *take(char *name)
    {
        loopinventory() if(strcmp(o->name, name)==0)
        {
            o->decontain();
            return o;
        }
        return NULL;
    }
    
    void takedamage(int damage, rpgobj &attacker)
    {
        ent->enemy = attacker.ent;
        
        particle_splash(PART_BLOOD, max(damage/10, 1), 1000, ent->o, 0x60FFFF, 2.96f);
        defformatstring(ds)("%d", damage);
        particle_textcopy(ent->abovehead(), ds, PART_TEXT, 2000, 0xFF4B19, 4.0f, -8);
        
        if((s_hp -= damage)<=0)
        {
            s_hp = 0;
            ent->state = CS_DEAD;
            ent->attacking = false;
            ent->lastaction = lastmillis;
            menutime = 0;
            conoutf("%s killed: %s", attacker.name, name);
            droploot();
        }    
    }
    
    void usesound(rpgent *user)
    {
        if(s_usesound) playsound(s_usesound, &user->o);    
    }
    
    bool usemana(rpgobj &o)
    {
        if(o.s_manacost>s_mana) { if(this==playerobj) conoutf("\f2not enough mana"); return false; };
        s_mana -= o.s_manacost;
        o.usesound(ent);
        return true;
    }
        
    void useaction(rpgobj &target, rpgent &initiator, bool chargemana)
    {
        if(action_use.script && (!chargemana || initiator.ro->usemana(*this)))
        {
            action_use.exec(this, &target, initiator.ro);
            if(s_useamount && !--s_useamount) removefromsystem(this);
        } 
    }
    
    void selectuse()
    {
        if(s_usetype)
        {
            conoutf("\f2using: %s", name);
            selected = this;                    
        }
        else
        {
            useaction(*playerobj, *playerobj->ent, true);
        }
    }
    
    void guiaction(g3d_gui &g, rpgaction *a)
    {
        if(!a) return;
        guiaction(g, a->next);
        if(g.button(a->initiate, a->used ? 0xAAAAAA : 0xFFFFFF, "chat")&G3D_UP)
        {
            currentquest = a->q;
            a->exec(this, playerobj, playerobj);
            currentquest = NULL;
        } 
    }
    
    void gui(g3d_gui &g, bool firstpass)
    {
        g.start(menutime, 0.015f, &menutab);
        switch(menuwhich)
        {
            case MENU_DEFAULT:
            {
                g.tab(name, 0xFFFFFF);
                if(abovetext) g.text(abovetext, 0xDDFFDD);

                guiaction(g, actions);

                if(s_ai)
                {
                    bool trader = false;
                    loopinventorytype(IF_TRADE) trader = true;
                    if(trader)
                    {
                        if(g.button("buy",  0xFFFFFF, "coins")&G3D_UP) menuwhich = MENU_BUY;
                        if(g.button("sell", 0xFFFFFF, "coins")&G3D_UP) menuwhich = MENU_SELL;                    
                    }
                }
                else
                {
                    defformatstring(wtext)("worth %d", s_worth);
                    g.text(wtext, 0xAAAAAA, "coins");
                    if(g.button("take", 0xFFFFFF, "hand")&G3D_UP)
                    {
                        conoutf("\f2you take a %s (worth %d gold)", name, s_worth);
                        taken(this, playerobj);
                    }
                    if(!s_usetype && g.button("use", 0xFFFFFF, "hand")&G3D_UP)
                    {
                        selectuse();
                    }
                }
                break;
            }
               
            case MENU_BUY:
            {
                defformatstring(info)("buying from: %s", name);
                g.tab(info, 0xFFFFFF);
                loopinventorytype(IF_TRADE)
                {
                    int price = o->s_worth;
                    defformatstring(info)("%s (%d)", o->name, price);
                    int ret = g.button(info, 0xFFFFFF, "coins");
                    if(ret&G3D_UP)
                    {
                        if(playerobj->s_gold>=price)
                        {
                            conoutf("\f2you bought %s for %d gold", o->name, price);
                            playerobj->s_gold -= price;
                            s_gold += price;
                            o->decontain();
                            playerobj->add(o, IF_INVENTORY);
                        }
                        else
                        {
                            conoutf("\f2you cannot afford this item!");
                        }
                    }
                }
                formatstring(info)("you have %d gold", playerobj->s_gold);
                g.text(info, 0xAAAAAA, "info");   
                if(g.button("done buying", 0xFFFFFF, "coins")&G3D_UP) menuwhich = MENU_DEFAULT;
                break;
            }
               
            case MENU_SELL:
            {
                defformatstring(info)("selling to: %s", name);
                g.tab(info, 0xFFFFFF);
                playerobj->invgui(g, this);
                if(g.button("done selling", 0xFFFFFF, "coins")&G3D_UP) menuwhich = MENU_DEFAULT;
                break;
            }
        }
        g.end();
    }
    
    void invgui(g3d_gui &g, rpgobj *buyer = NULL)
    {
        loopinventory()
        {
            int price = o->s_worth/2;
            defformatstring(info)("%s (%d)", o->name, price);
            int ret = g.button(info, 0xFFFFFF, "coins");
            if(ret&G3D_UP)
            {
                if(buyer)
                {
                    if(price>buyer->s_gold)
                    {
                        conoutf("\f2%s cannot afford to buy %s from you!", buyer->name, o->name);                    
                    }
                    else
                    {
                        if(price)
                        {
                            conoutf("\f2you sold %s for %d gold", o->name, price);
                            s_gold += price;
                            buyer->s_gold -= price;
                            o->decontain();
                            buyer->add(o, IF_TRADE);                                            
                        }
                        else
                        {
                            conoutf("\f2you cannot sell %s", o->name);
                        }
                    }
                }
                else    // player wants to use this item
                {
                    o->selectuse();
                }
            }
        }
        defformatstring(info)("you have %d gold", playerobj->s_gold);
        g.text(info, 0xAAAAAA, "info");   
    }

    void g3d_menu()
    {
        if(!menutime) return;
        g3d_addgui(this, vec(ent->o).add(vec(0, 0, 2)));
    }
};
