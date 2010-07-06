#define isbomberaffinity(a) (a.team == TEAM_NEUTRAL)
#define isbomberhome(a,b)   (!isbomberaffinity(a) && a.team == b)
#define isbombertarg(a,b)   (!isbomberaffinity(a) && a.team != b)

#ifdef GAMESERVER
#define bomberstate bomberservstate
#endif
struct bomberstate
{
    struct flag
    {
        vec droploc, inertia, spawnloc;
        int team, droptime, taketime, inittime;
#ifdef GAMESERVER
        int owner, lastowner;
        vector<int> votes;
#else
        gameent *owner, *lastowner;
        projent *proj;
        entitylight light;
        int ent, interptime, pickuptime;
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
            interptime = pickuptime = 0;
#endif
            team = TEAM_NEUTRAL;
            inittime = taketime = droptime = 0;
        }

#ifndef GAMESERVER
        vec &pos(bool render = false)
        {
            if(owner) return render ? owner->waist : owner->o;
            if(droptime) return proj ? proj->o : droploc;
            return spawnloc;
        }
#endif
    };
    vector<flag> flags;

    struct score
    {
        int team, total;
    };

    vector<score> scores;

    void reset()
    {
        flags.shrink(0);
        scores.shrink(0);
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

#ifdef GAMESERVER
    void takeaffinity(int i, int owner, int t)
#else
    void takeaffinity(int i, gameent *owner, int t)
#endif
    {
        flag &f = flags[i];
        f.owner = owner;
        f.taketime = t;
        if(!f.inittime) f.inittime = t;
        f.droptime = 0;
#ifdef GAMESERVER
        f.votes.shrink(0);
        f.lastowner = owner;
#else
        f.pickuptime = 0;
        f.lastowner = owner;
        if(f.proj)
        {
            f.proj->beenused = 2;
            f.proj->lifetime = min(f.proj->lifetime, f.proj->fadetime);
        }
        f.proj = NULL;
#endif
    }

    void dropaffinity(int i, const vec &o, const vec &p, int t)
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
        f.owner = NULL;
        if(f.proj)
        {
            f.proj->beenused = 2;
            f.proj->lifetime = min(f.proj->lifetime, f.proj->fadetime);
        }
        f.proj = NULL;
#endif
    }

    void returnaffinity(int i, int t, bool score)
    {
        flag &f = flags[i];
        f.droptime = f.taketime = 0;
        if(score) f.inittime = 0;
#ifdef GAMESERVER
        f.owner = -1;
        f.votes.shrink(0);
#else
        f.pickuptime = 0;
        f.owner = NULL;
        if(f.proj)
        {
            f.proj->beenused = 2;
            f.proj->lifetime = min(f.proj->lifetime, f.proj->fadetime);
        }
        f.proj = NULL;
#endif
    }

    score &findscore(int team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(cs.team == team) return cs;
        }
        score &cs = scores.add();
        cs.team = team;
        cs.total = 0;
        return cs;
    }

#ifndef GAMESERVER
    void interp(int i, int t)
    {
        flag &f = flags[i];
        f.interptime = f.interptime ? t-max(1000-(t-f.interptime), 0) : t;
    }
#endif
};

#ifndef GAMESERVER
namespace bomber
{
    extern bomberstate st;
    extern bool dropaffinity(gameent *d);
    extern void sendaffinity(packetbuf &p);
    extern void parseaffinity(ucharbuf &p, bool commit);
    extern void dropaffinity(gameent *d, int i, const vec &droploc, const vec &inertia);
    extern void scoreaffinity(gameent *d, int relay, int goal, int score);
    extern void takeaffinity(gameent *d, int i);
    extern void resetaffinity(int i);
    extern void setupaffinity();
    extern void setscore(int team, int total);
    extern void checkaffinity(gameent *d);
    extern void drawlast(int w, int h, int &tx, int &ty, float blend);
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
}
#endif
