enum
{
    T_NEUTRAL = 0, T_ALPHA, T_OMEGA, T_KAPPA, T_SIGMA, T_ENEMY, T_MAX,
    T_FIRST = T_ALPHA, T_LAST = T_OMEGA, T_MULTI = T_SIGMA,
    T_COUNT = T_LAST+1, T_ALL = T_MULTI+1,
    T_NUM = (T_LAST-T_FIRST)+1,
    T_TOTAL = (T_MULTI-T_FIRST)+1
};

#define TEAMS(a,b) \
    GSVAR(0, team##a##name, #a); \
    GVAR(IDF_HEX, team##a##colour, 0, b, 0xFFFFFF);

TEAMS(neutral, 0x90A090);
TEAMS(alpha, 0x5F66FF);
TEAMS(omega, 0xFF4F44);
TEAMS(kappa, 0xFFD022);
TEAMS(sigma, 0x22FF22);
TEAMS(enemy, 0xF88820);

#ifdef GAMESERVER
enum { TT_INFO = 1<<0, TT_RESET = 1<<1, TT_SMODE = 1<<2, TT_DEFAULT = TT_RESET|TT_SMODE, TT_SMINFO = TT_INFO|TT_SMODE, TT_DFINFO = TT_INFO|TT_DEFAULT };

#define TEAMDEF(proto,name)     proto *sv_team_stat_##name[] = { &sv_teamneutral##name, &sv_teamalpha##name, &sv_teamomega##name, &sv_teamkappa##name, &sv_teamsigma##name, &sv_teamenemy##name };
#define TEAM(id,name)           (*sv_team_stat_##name[id])
#else
#ifdef GAMEWORLD
#define TEAMDEF(proto,name)     proto *team_stat_##name[] = { &teamneutral##name, &teamalpha##name, &teamomega##name, &teamkappa##name, &teamsigma##name, &teamenemy##name };
#else
#define TEAMDEF(proto,name)     extern proto *team_stat_##name[];
#endif
#define TEAM(id,name)           (*team_stat_##name[id])
#endif
TEAMDEF(char *, name);
TEAMDEF(int, colour);

struct score
{
    int team, total;
    score() {}
    score(int s, int n) : team(s), total(n) {}
};

#define numteams(a,b)   (m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? T_TOTAL : T_NUM) : 1)
#define teamcount(a,b)  (m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? T_ALL : T_COUNT) : 1)
#define isteam(a,b,c,d) (m_fight(a) && m_team(a,b) ? (c >= d && c <= numteams(a,b)) : c == T_NEUTRAL)
#define valteam(a,b)    (a >= b && a <= T_TOTAL)

#ifdef GAMESERVER
const int mapbals[T_TOTAL][T_TOTAL] = {
    { T_ALPHA, T_OMEGA, T_KAPPA, T_SIGMA },
    { T_OMEGA, T_ALPHA, T_SIGMA, T_KAPPA },
    { T_KAPPA, T_SIGMA, T_ALPHA, T_OMEGA },
    { T_SIGMA, T_KAPPA, T_OMEGA, T_ALPHA }
};
#else
extern const int mapbals[T_TOTAL][T_TOTAL];
#endif

#ifdef MEK
#define PLAYERTYPES 4
#ifdef GAMEWORLD
const char *playertypes[PLAYERTYPES][4] = {
    { "actors/mek1/hwep",    "actors/mek1",   "mek1",   "light" },
    { "actors/mek2/hwep",    "actors/mek2",   "mek2",   "medium" },
    { "actors/mek3/hwep",    "actors/mek3",   "mek3",   "flyer" },
    { "actors/mek4/hwep",    "actors/mek4",   "mek4",   "heavy" },
};
#else
extern const char *playertypes[PLAYERTYPES][4]; //3
#endif

#define CLASSES(a,b1,b2,c1,c2,c3,c4,c5) \
    GSVAR(0, class##a##name, #a); \
    GVAR(0, class##a##health, 0, b1, VAR_MAX); \
    GVAR(0, class##a##armour, 0, b2, VAR_MAX); \
    GFVAR(0, class##a##xradius, 0, c1, FVAR_MAX); \
    GFVAR(0, class##a##yradius, 0, c2, FVAR_MAX); \
    GFVAR(0, class##a##height, 0, c3, FVAR_MAX); \
    GFVAR(0, class##a##weight, 0, c4, FVAR_MAX); \
    GFVAR(0, class##a##speed, 0, c5, FVAR_MAX);

//      name    health  armour  xrad    yrad    height  weight      speed
CLASSES(mek1,   300,    80,     6,      6,      16,     200,        80); // light
CLASSES(mek2,   400,    100,    6,      6,      16,     300,        60); // medium
CLASSES(mek3,   330,    90,     6,      6,      16,     250,        70); // flyer
CLASSES(mek4,   500,    200,    6,      6,      16,     350,        40); // heavy

#ifdef GAMESERVER
#define CLASSDEF(proto,name)     proto *sv_class_stat_##name[] = { &sv_classmek1##name, &sv_classmek2##name, &sv_classmek3##name, &sv_classmek4##name };
#define CLASS(id,name)           (*sv_class_stat_##name[max(id,0)%PLAYERTYPES])
#else
#ifdef GAMEWORLD
#define CLASSDEF(proto,name)     proto *class_stat_##name[] = { &classmek1##name, &classmek2##name, &classmek3##name, &classmek4##name };
#else
#define CLASSDEF(proto,name)     extern proto *class_stat_##name[];
#endif
#define CLASS(id,name)           (*class_stat_##name[max(id,0)%PLAYERTYPES])
#endif
CLASSDEF(char *, name);
CLASSDEF(int, health);
CLASSDEF(int, armour);
CLASSDEF(float, xradius);
CLASSDEF(float, yradius);
CLASSDEF(float, height);
CLASSDEF(float, weight);
CLASSDEF(float, speed);
#else // FPS
#define PLAYERTYPES 2
#ifdef GAMEWORLD
const char *playertypes[PLAYERTYPES][4] = {
    { "actors/player/male/hwep",      "actors/player/male",     "actors/player/male/body",      "male" },
    { "actors/player/female/hwep",    "actors/player/female",   "actors/player/male/body",      "female" }
};
#else
extern const char *playertypes[PLAYERTYPES][3];
#endif
#endif
#ifdef VANITY
#define VANITYMAX 16
struct vanityfile
{
    char *id, *name;

    vanityfile() : id(NULL), name(NULL) {}
    vanityfile(const char *d, const char *n) : id(newstring(d)), name(newstring(n)) {}
    ~vanityfile()
    {
        if(id) delete[] id;
        if(name) delete[] name;
    }
};
struct vanitys
{
    int type, cond, style, priv;
    char *ref, *model, *name, *tag;
    vector<vanityfile> files;

    vanitys() : type(-1), cond(0), style(0), priv(0), ref(NULL), model(NULL), name(NULL), tag(NULL) {}
    vanitys(int t, const char *r, const char *n, const char *g, int c, int s, int p) : type(t), cond(c), style(s), priv(p), ref(newstring(r)), name(newstring(n)), tag(newstring(g)) { setmodel(r); }
    ~vanitys()
    {
        if(ref) delete[] ref;
        if(model) delete[] model;
        if(name) delete[] name;
        if(tag) delete[] tag;
        loopvrev(files) files.remove(i);
    }

    void setmodel(const char *r)
    {
        if(model) delete[] model;
        defformatstring(m)("vanities/%s", r);
        model = newstring(m);
    }
};
#ifdef GAMEWORLD
vector<vanitys> vanities;
#else
extern vector<vanitys> vanities;
#endif
#endif
