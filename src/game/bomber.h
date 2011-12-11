#define isbomberaffinity(a) (a.enabled && a.team == TEAM_NEUTRAL)
#define isbomberhome(a,b)   (a.enabled && !isbomberaffinity(a) && a.team != TEAM_NEUTRAL && a.team == b)
#define isbombertarg(a,b)   (a.enabled && !isbomberaffinity(a) && a.team != TEAM_NEUTRAL && a.team != b)

#ifdef GAMESERVER
#define bomberstate bomberservstate
#endif
struct bomberstate
{
    struct flag
    {
        vec droploc, inertia, spawnloc;
        int team, droptime, taketime;
        bool enabled;
#ifdef GAMESERVER
        int owner, lastowner;
        vector<int> votes;
#else
        gameent *owner, *lastowner;
        projent *proj;
        entitylight light;
        int ent, interptime, pickuptime, inittime, interpmillis;
        vec interppos;
#endif

        flag()
#ifndef GAMESERVER
          : ent(-1)
#endif
        { reset(); }

        void reset()
        {
            inertia = vec(0, 0, 0);
            droploc = spawnloc = vec(-1, -1, -1);
#ifdef GAMESERVER
            owner = lastowner = -1;
            votes.shrink(0);
#else
            owner = lastowner = NULL;
            proj = NULL;
            interptime = pickuptime = inittime = interpmillis = 0;
            interppos = vec(-1, -1, -1);
#endif
            team = TEAM_NEUTRAL;
            taketime = droptime = 0;
            enabled = false;
        }

#ifndef GAMESERVER
        vec &pos(bool render = true)
        {
            if(owner)
            {
                if(render && lastmillis != interpmillis)
                {
                    float yaw = 360-((lastmillis/2)%360), off = (lastmillis%1000)/500.f;
                    vecfromyawpitch(yaw, 0, 1, 0, interppos);
                    interppos.normalize().mul(owner->radius+4).add(owner->headpos(-owner->height/2));
                    interppos.z += owner->height*(off > 1 ?  2-off : off);
                    interpmillis = lastmillis;
                    return interppos;
                }
                else return owner->waist;
            }
            if(droptime) return proj ? proj->o : droploc;
            return spawnloc;
        }
#endif
    };
    vector<flag> flags;

    void reset()
    {
        flags.shrink(0);
    }

    int addaffinity(const vec &o, int team, int i = -1)
    {
        int x = i < 0 ? flags.length() : i;
        while(!flags.inrange(x)) flags.add();
        flag &f = flags[x];
        f.reset();
        f.team = team;
        f.spawnloc = o;
        return x;
    }
#ifndef GAMESERVER
    void interp(int i, int t)
    {
        flag &f = flags[i];
        f.interptime = f.interptime ? t-max(1000-(t-f.interptime), 0) : t;
    }

    void destroy(int id)
    {
        flags[id].proj = NULL;
        loopv(projs::projs) if(projs::projs[i]->projtype == PRJ_AFFINITY && projs::projs[i]->id == id)
        {
            projs::projs[i]->state = CS_DEAD;
            projs::projs[i]->beenused = 2;
        }
    }

    void create(int id, int target)
    {
        flag &f = flags[id];
        f.proj = projs::create(f.droploc, f.inertia, false, NULL, PRJ_AFFINITY, bomberresetdelay, bomberresetdelay, 1, 1, id, target);
    }
#endif
#ifdef GAMESERVER
    void takeaffinity(int i, int owner, int t)
#else
    void takeaffinity(int i, gameent *owner, int t)
#endif
    {
        flag &f = flags[i];
        f.owner = owner;
        f.taketime = t;
        f.droptime = 0;
#ifdef GAMESERVER
        f.votes.shrink(0);
        f.lastowner = owner;
#else
        f.pickuptime = 0;
        if(!f.inittime) f.inittime = t;
        (f.lastowner = owner)->addicon(eventicon::AFFINITY, t, game::eventiconfade, f.team);
        destroy(i);
        interp(i, t);
#endif
    }

    void dropaffinity(int i, const vec &o, const vec &p, int t, int target = -1)
    {
        flag &f = flags[i];
        f.droploc = o;
        f.inertia = p;
        f.droptime = t;
        f.taketime = 0;
#ifdef GAMESERVER
        f.owner = -1;
        f.votes.shrink(0);
#else
        f.pickuptime = 0;
        if(!f.inittime) f.inittime = t;
        f.owner = NULL;
        destroy(i);
        create(i, target);
        interp(i, t);
#endif
    }

    void returnaffinity(int i, int t, bool enabled)
    {
        flag &f = flags[i];
        f.droptime = f.taketime = 0;
        f.enabled = enabled;
#ifdef GAMESERVER
        f.owner = -1;
        f.votes.shrink(0);
#else
        f.pickuptime = f.inittime = 0;
        f.owner = NULL;
        destroy(i);
        interp(i, t);
#endif
    }
};

#ifndef GAMESERVER
namespace bomber
{
    extern bomberstate st;
    extern bool carryaffinity(gameent *d);
    extern bool dropaffinity(gameent *d);
    extern void sendaffinity(packetbuf &p);
    extern void parseaffinity(ucharbuf &p, bool commit);
    extern void dropaffinity(gameent *d, int i, const vec &droploc, const vec &inertia, int target = -1);
    extern void scoreaffinity(gameent *d, int relay, int goal, int score);
    extern void takeaffinity(gameent *d, int i);
    extern void resetaffinity(int i, int value);
    extern void reset();
    extern void setup();
    extern void setscore(int team, int total);
    extern void checkaffinity(gameent *d);
    extern void drawnotices(int w, int h, int &tx, int &ty, float blend);
    extern void drawblips(int w, int h, float blend);
    extern int drawinventory(int x, int y, int s, int m, float blend);
    extern void preload();
    extern void render();
    extern void adddynlights();
    extern int aiowner(gameent *d);
    extern void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests);
    extern bool aicheck(gameent *d, ai::aistate &b);
    extern bool aidefense(gameent *d, ai::aistate &b);
    extern bool aipursue(gameent *d, ai::aistate &b);
    extern void removeplayer(gameent *d);
    extern vec pulsecolour();
    extern void checkcams(vector<cament> &cameras);
    extern void updatecam(cament &c);
}
#endif
