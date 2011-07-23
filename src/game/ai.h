struct gameent;

enum { AI_BOT, AI_TURRET, AI_GRUNT, AI_MAX, AI_START = AI_TURRET, AI_TOTAL = AI_MAX-AI_START };
enum { AI_F_NONE = 0, AI_F_RANDWEAP = 1<<0 };
#define isaitype(a) (a >= 0 && a <= AI_MAX-1)

struct aistyles
{
    int type,           weap,           health;
    float   xradius,    yradius,    height,     weight,     speed,      scale;
    bool    canmove,    canstrafe,  canjump,    cancrouch,  useweap,    living,     hitbox;
    const char  *name,      *tpmdl,              *fpmdl;
};
#ifdef GAMESERVER
aistyles aistyle[] = {
    {
        AI_BOT,         -1,             0,
            3,          3,          14,         200,        50,         1,
            true,       true,       true,       true,       true,       true,       true,
                "bot",      "actors/player",     "actors/player/hwep"
    },
    {
        AI_TURRET,      WEAP_SMG,       100,
            4.75,       4.75,       8.75,       150,        1,          1,
            false,      false,      false,      false,      false,      false,      false,
                "turret",   "actors/turret",     "actors/player/hwep"
    },
    {
        AI_GRUNT,       WEAP_MELEE,     50,
            3,          3,          14,         150,        40,         1,
            true,       false,      true,       true,       true,       true,       true,
                "grunt",    "actors/grunt",      "actors/player/hwep"
    },
};
#else
extern aistyles aistyle[];
#endif

enum
{
    WP_F_NONE = 0,
    WP_F_CROUCH = 1<<0,
    WP_F_HOVER = 1<<1
};
namespace entities { struct avoidset; };

namespace ai
{
    const float CLOSEDIST       = 32.f;    // is close
    const float JUMPMIN         = 1.5f;    // decides to jump
    const float JUMPMAX         = 32.f;    // max jump
    const float SIGHTMIN        = 32.f;    // minimum line of sight
    const float SIGHTMAX        = 1024.f;  // maximum line of sight
    const float ALERTMIN        = 64.f;    // minimum alert distance
    const float ALERTMAX        = 512.f;   // maximum alert distance
    const float VIEWMIN         = 90.f;    // minimum field of view
    const float VIEWMAX         = 180.f;   // maximum field of view

    // ai state information for the owner client
    enum
    {
        AI_S_WAIT = 0,      // waiting for next command
        AI_S_DEFEND,        // defend goal target
        AI_S_PURSUE,        // pursue goal target
        AI_S_INTEREST,      // interest in goal entity
        AI_S_MAX
    };

    enum
    {
        AI_T_NODE,
        AI_T_ACTOR,
        AI_T_AFFINITY,
        AI_T_ENTITY,
        AI_T_DROP,
        AI_T_MAX
    };

    struct interest
    {
        int state, node, target, targtype;
        float score;
        interest() : state(-1), node(-1), target(-1), targtype(-1), score(0.f) {}
        ~interest() {}
    };

    struct aistate
    {
        int type, millis, targtype, target, idle;
        bool override;

        aistate(int m, int t, int r = -1, int v = -1) : type(t), millis(m), targtype(r), target(v)
        {
            reset();
        }
        ~aistate() {}

        void reset()
        {
            idle = 0;
            override = false;
        }
    };

    const int NUMPREVNODES = 6;

    struct aiinfo
    {
        vector<aistate> state;
        vector<int> route;
        vec target, spot;
        int weappref, enemy, enemyseen, enemymillis, prevnodes[NUMPREVNODES], targnode, targlast, targtime, targseq,
            lastrun, lasthunt, lastaction, jumpseed, jumprand, blocktime, huntseq, blockseq, lastaimrnd,
            lastpusher, lastpushtime, lastmelee;
        float scale, targyaw, targpitch, views[3], aimrnd[3];
        bool suspended, dontmove, tryreset, trywipe;

        aiinfo()
        {
            clearsetup();
            reset();
            scale = 1;
            loopk(3) views[k] = aimrnd[k] = 0.f;
            suspended = true;
        }
        ~aiinfo() {}

        void clearsetup()
        {
            spot = target = vec(0, 0, 0);
            lastaction = lasthunt = enemyseen = enemymillis = blocktime = huntseq = blockseq = targtime = targseq = lastaimrnd = lastpushtime = lastmelee = 0;
            lastrun = jumpseed = lastmillis;
            jumprand = lastmillis+5000;
            weappref = targnode = targlast = enemy = lastpusher = -1;
        }

        void clear(bool prev = true)
        {
            if(prev) memset(prevnodes, -1, sizeof(prevnodes));
            route.setsize(0);
        }

        void wipe()
        {
            clear(true);
            state.setsize(0);
            addstate(AI_S_WAIT);
            trywipe = false;
        }

        void clean(bool tryit = false)
        {
            if(!tryit) dontmove = false;
            targyaw = rnd(360);
            targpitch = 0.f;
            tryreset = tryit;
        }

        void reset(bool tryit = false) { wipe(); clean(tryit); }
        void unsuspend() { suspended = false; clean(false); }

        bool hasprevnode(int n) const
        {
            loopi(NUMPREVNODES) if(prevnodes[i] == n) return true;
            return false;
        }

        void addprevnode(int n)
        {
            if(!hasprevnode(n))
            {
                memmove(&prevnodes[1], prevnodes, sizeof(prevnodes) - sizeof(prevnodes[0]));
                prevnodes[0] = n;
            }
        }

        aistate &addstate(int t, int r = -1, int v = -1)
        {
            return state.add(aistate(lastmillis, t, r, v));
        }

        void removestate(int index = -1)
        {
            if(index < 0) state.pop();
            else if(state.inrange(index)) state.remove(index);
            if(!state.length()) addstate(AI_S_WAIT);
        }

        aistate &getstate(int idx = -1)
        {
            if(state.inrange(idx)) return state[idx];
            return state.last();
        }

        aistate &switchstate(aistate &b, int t, int r = -1, int v = -1)
        {
            if(b.type == t && b.targtype == r)
            {
                b.millis = lastmillis;
                b.target = v;
                b.reset();
                return b;
            }
            return addstate(t, r, v);
        }
    };

    extern entities::avoidset obs, wps;
    extern vec aitarget;
    extern int aidebug, aideadfade, showaiinfo;

    extern float viewdist(int x = 101);
    extern float viewfieldx(int x = 101);
    extern float viewfieldy(int x = 101);

    extern bool targetable(gameent *d, gameent *e);
    extern bool cansee(gameent *d, vec &x, vec &y, bool force = false, vec &targ = aitarget);
    extern bool altfire(gameent *d, gameent *e);
    extern int owner(gameent *d);

    extern void init(gameent *d, int at, int et, int on, int sk, int bn, char *name, int tm);

    extern bool badhealth(gameent *d);
    extern bool checkothers(vector<int> &targets, gameent *d = NULL, int state = -1, int targtype = -1, int target = -1, bool teams = false);
    extern bool makeroute(gameent *d, aistate &b, int node, bool changed = true, bool retry = false);
    extern bool makeroute(gameent *d, aistate &b, const vec &pos, bool changed = true, bool retry = false);
    extern bool randomnode(gameent *d, aistate &b, const vec &pos, float guard = ALERTMIN, float wander = ALERTMAX);
    extern bool randomnode(gameent *d, aistate &b, float guard = ALERTMIN, float wander = ALERTMAX);
    extern bool violence(gameent *d, aistate &b, gameent *e, bool pursue = false);
    extern bool patrol(gameent *d, aistate &b, const vec &pos, float guard = ALERTMIN, float wander = ALERTMAX, int walk = 1, bool retry = false);
    extern bool defense(gameent *d, aistate &b, const vec &pos, float guard = ALERTMIN, float wander = ALERTMAX, int walk = 0);

    extern void spawned(gameent *d, int ent);
    extern void damaged(gameent *d, gameent *e, int weap, int flags, int damage);
    extern void killed(gameent *d, gameent *e);
    extern void itemspawned(int ent, int spawned);
    extern void update();
    extern void avoid();
    extern void think(gameent *d, bool run);
    extern void render();
    extern void preload();
};
