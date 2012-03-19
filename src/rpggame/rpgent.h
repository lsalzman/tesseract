struct rpgent : dynent
{
    rpgobj *ro;

    int lastaction, lastpain;
    bool attacking;
    
    bool magicprojectile;
    vec mppos, mpdir;
    rpgobj *mpweapon;
    float mpdist;
    
    rpgent *enemy;
    float targetyaw, rotspeed;
    
    enum { R_STARE, R_ROAM, R_SEEK, R_ATTACK, R_BLOCKED, R_BACKHOME };
    int npcstate;
    
    int trigger;

    float sink;

    vec home;
        
    enum { ROTSPEED = 200 };

    rpgent(rpgobj *_ro, const vec &_pos, float _yaw, int _maxspeed = 40, int _type = ENT_AI) : ro(_ro), lastaction(0), lastpain(0), attacking(false), magicprojectile(false), enemy(NULL), rotspeed(0), npcstate(R_STARE), trigger(0), sink(0)
    {
        o = _pos;
        home = _pos;
        targetyaw = yaw = _yaw;
        maxspeed = _maxspeed;
        type = _type;
        enemy = player1;
    }

    float relyaw(vec &t) { return -(float)atan2(t.x-o.x, t.y-o.y)/RAD; };

    void normalize_yaw(float angle)
    {
        while(yaw<angle-180.0f) yaw += 360.0f;
        while(yaw>angle+180.0f) yaw -= 360.0f;
    }

    void tryattackobj(rpgobj &eo, rpgobj &weapon);
    void tryattack(vec &lookatpos, rpgobj &weapon);
    void updateprojectile();
    bool allowmove();

    void transition(int _state, int _moving, int n); 
    void gotoyaw(float yaw, int s, int m, int t);
    void gotopos(vec &pos, int s, int m, int t);
    void goroam();
    void stareorroam();
    
    void update(float playerdist);
    void updateplayer();
};
