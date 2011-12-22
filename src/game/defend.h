#ifdef GAMESERVER
#define defendstate stfservstate
#endif
struct defendstate
{
    struct flag
    {
        vec o;
        int kinship, owner, enemy;
#ifndef GAMESERVER
        string name, info;
        bool hasflag;
        int ent, lasthad;
#endif
        int owners, enemies, converted, securetime;

        flag()
        {
            kinship = TEAM_NEUTRAL;
#ifndef GAMESERVER
            ent = -1;
#endif
            reset();
        }

        void noenemy()
        {
            enemy = TEAM_NEUTRAL;
            enemies = 0;
            converted = 0;
        }

        void reset()
        {
            noenemy();
            owner = kinship;
            securetime = -1;
            owners = 0;
#ifndef GAMESERVER
            hasflag = false;
            lasthad = 0;
#endif
        }

        bool enter(int team)
        {
            if(owner == team)
            {
                owners++;
                return false;
            }
            if(!enemies)
            {
                if(enemy != team)
                {
                    converted = 0;
                    enemy = team;
                }
                enemies++;
                return true;
            }
            else if(enemy != team) return false;
            else enemies++;
            return false;
        }

        bool steal(int team)
        {
            return !enemies && owner != team;
        }

        bool leave(int team)
        {
            if(owner == team)
            {
                owners--;
                return false;
            }
            if(enemy != team) return false;
            enemies--;
            return !enemies;
        }

        int occupy(int team, int units, int occupy, bool instant)
        {
            if(enemy != team) return -1;
            converted += units;
            if(units<0)
            {
                if(converted<=0) noenemy();
                return -1;
            }
            else if(converted<(!instant && owner ? 2 : 1)*occupy) return -1;
            if(!instant && owner) { owner = TEAM_NEUTRAL; converted = 0; enemy = team; return 0; }
            else { owner = team; securetime = 0; owners = enemies; noenemy(); return 1; }
        }

        float occupied(bool instant, float amt)
        {
            return (enemy ? enemy : owner) ? (!owner || enemy ? clamp(converted/float((!instant && owner ? 2 : 1)*amt), 0.f, 1.f) : 1.f) : 0.f;
        }
    };

    vector<flag> flags;
    int secured;

    defendstate() : secured(0) {}

    void reset()
    {
        flags.shrink(0);
        secured = 0;
    }

    void addaffinity(const vec &o, int team)
    {
        flag &b = flags.add();
        b.o = o;
        b.kinship = team;
        b.reset();
    }

    void initaffinity(int i, int kin, int owner, int enemy, int converted)
    {
        if(!flags.inrange(i)) return;
        flag &b = flags[i];
        b.kinship = kin;
        b.reset();
        b.owner = owner;
        b.enemy = enemy;
        b.converted = converted;
    }

    bool hasaffinity(int team)
    {
        loopv(flags)
        {
            flag &b = flags[i];
            if(b.owner && b.owner == team) return true;
        }
        return false;
    }

    float disttoenemy(flag &b)
    {
        float dist = 1e10f;
        loopv(flags)
        {
            flag &e = flags[i];
            if(e.owner && b.owner != e.owner)
                dist = min(dist, b.o.dist(e.o));
        }
        return dist;
    }

    bool insideaffinity(const flag &b, const vec &o, float scale = 1.f)
    {
        float dx = (b.o.x-o.x), dy = (b.o.y-o.y), dz = (b.o.z-o.z);
        return dx*dx + dy*dy <= (enttype[AFFINITY].radius*enttype[AFFINITY].radius)*scale && fabs(dz) <= enttype[AFFINITY].radius*scale;
    }
};

#ifndef GAMESERVER
namespace defend
{
    extern defendstate st;
    extern void sendaffinity(packetbuf &p);
    extern void parseaffinity(ucharbuf &p);
    extern void updateaffinity(int i, int owner, int enemy, int converted);
    extern void setscore(int team, int total);
    extern void reset();
    extern void setup();
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
    extern void checkcams(vector<cament *> &cameras);
    extern void updatecam(cament *c);
}
#endif
