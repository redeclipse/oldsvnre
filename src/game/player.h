struct actors
{
    int type,           weap,           health, armour;
    float   xradius,    yradius,    height,     weight,     speed,      scale;
    bool    canmove,    canstrafe,  canjump,    cancrouch,  useweap,    living,     hitbox;
    const char  *name,      *playermodel[4];
};

enum { A_PLAYER = 0, A_BOT, A_TURRET, A_GRUNT, A_DRONE, A_MAX, A_ENEMY = A_TURRET, A_TOTAL = A_MAX-A_ENEMY };
#ifdef GAMESERVER
actors actor[] = {
    {
        A_PLAYER,         -1,             0,    -1,
            3,          3,          14,         200,        50,         1,
            true,       true,       true,       true,       true,       true,       true,
                "player",   { "actors/player/male/hwep",      "actors/player/male",     "actors/player/male/body",      "actors/player/male/headless" }
    },
    {
        A_BOT,         -1,             0,       -1,
            3,          3,          14,         200,        50,         1,
            true,       true,       true,       true,       true,       true,       true,
                "bot",      { "actors/player/male/hwep",      "actors/player/male",     "actors/player/male/body",      "actors/player/male/headless" }
    },
    {
        A_TURRET,      W_SMG,       100,        25,
            4.75,       4.75,       8.75,       150,        1,          1,
            false,      false,      false,      false,      false,      false,      false,
                "turret",   { "actors/player/male/hwep",      "actors/turret",          "actors/player/male/body",      "actors/turret" }
    },
    {
        A_GRUNT,       W_PISTOL,   50,          25,
            3,          3,          14,         200,        50,         1,
            true,       true,       true,       true,       true,       true,       true,
                "grunt",   { "actors/player/male/hwep",      "actors/player/male",      "actors/player/male/body",      "actors/player/male/headless" }
    },
    {
        A_DRONE,       W_MELEE,     50,         50,
            3,          3,          14,         150,        40,         1,
            true,       true,       true,       true,       true,       true,       true,
                "drone",    { "actors/player/male/hwep",      "actors/drone",           "actors/player/male/body",      "actors/drone" }
    },
};
#endif

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
TEAMS(enemy, 0x6F6F6F);

#ifdef GAMESERVER
enum { TT_INFO = 1<<0, TT_RESET = 1<<1, TT_SMODE = 1<<2, TT_INFOSM = TT_INFO|TT_SMODE, TT_RESETX = TT_INFO|TT_RESET };

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
    score(int s = -1, int n = 0) : team(s), total(n) {}
    ~score() {}
};

#define numteams(a,b)   (m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? T_TOTAL : T_NUM) : 1)
#define teamcount(a,b)  (m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? T_ALL : T_COUNT) : 1)
#define isteam(a,b,c,d) (m_fight(a) && m_team(a,b) ? (c >= d && c <= numteams(a,b)) : c == T_NEUTRAL)
#define valteam(a,b)    (a >= b && a <= T_TOTAL)

#ifdef GAMESERVER
int mapbals[T_TOTAL][T_TOTAL] = {
    { T_ALPHA, T_OMEGA, T_KAPPA, T_SIGMA },
    { T_OMEGA, T_ALPHA, T_SIGMA, T_KAPPA },
    { T_KAPPA, T_SIGMA, T_ALPHA, T_OMEGA },
    { T_SIGMA, T_KAPPA, T_OMEGA, T_ALPHA }
};
#else
extern int mapbals[T_TOTAL][T_TOTAL];
#endif

#define CLASSES(a,b1,b2,c1,c2) \
    GSVAR(0, class##a##name, #a); \
    GVAR(0, class##a##health, 0, b1, VAR_MAX); \
    GVAR(0, class##a##armour, 0, b2, VAR_MAX); \
    GFVAR(0, class##a##weight, 0, c1, FVAR_MAX); \
    GFVAR(0, class##a##speed, 0, c2, FVAR_MAX);

#ifdef MEK
#define PLAYERTYPES 4
#ifdef GAMESERVER
const char *playertypes[PLAYERTYPES][5] = {
    { "actors/mek1/hwep",    "actors/mek1",    "actors/mek1",   "actors/mek1",      "light" },
    { "actors/mek2/hwep",    "actors/mek2",    "actors/mek2",   "actors/mek2",      "medium" },
    { "actors/mek3/hwep",    "actors/mek3",    "actors/mek3",   "actors/mek3",      "flyer" },
    { "actors/mek4/hwep",    "actors/mek4",    "actors/mek4",   "actors/mek4",      "heavy" },
};
float playerdims[PLAYERTYPES][3] = {
    { 6,      6,      16 },
    { 6,      6,      16 },
    { 6,      6,      16 },
    { 6,      6,      16 },
};
#else
extern const char *playertypes[PLAYERTYPES][5];
extern float playerdims[PLAYERTYPES][3];
#endif

//      name    health  armour  weight      speed
CLASSES(mek1,   300,    80,     200,        80); // light
CLASSES(mek2,   400,    100,    300,        60); // medium
CLASSES(mek3,   330,    90,     250,        70); // flyer
CLASSES(mek4,   500,    200,    350,        40); // heavy

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
#else // FPS
#define PLAYERTYPES 2
#ifdef GAMESERVER
const char *playertypes[PLAYERTYPES][5] = {
    { "actors/player/male/hwep",      "actors/player/male",     "actors/player/male/body",      "actors/player/male/headless",      "male" },
    { "actors/player/female/hwep",    "actors/player/female",   "actors/player/male/body",      "actors/player/female/headless",    "female" }
};
float playerdims[PLAYERTYPES][3] = {
    { 3,      3,      14 },
    { 3,      3,      14 },
};
#else
extern const char *playertypes[PLAYERTYPES][5];
extern float playerdims[PLAYERTYPES][3];
#endif

//      name    health  armour  weight      speed
CLASSES(player, 100,    0,      200,        50);
#ifdef GAMESERVER
#define CLASSDEF(proto,name)     proto *sv_class_stat_##name[] = { &sv_classplayer##name, &sv_classplayer##name };
#define CLASS(id,name)           (*sv_class_stat_##name[max(id,0)%PLAYERTYPES])
#else
#ifdef GAMEWORLD
#define CLASSDEF(proto,name)     proto *class_stat_##name[] = { &classplayer##name, &classplayer##name };
#else
#define CLASSDEF(proto,name)     extern proto *class_stat_##name[];
#endif
#define CLASS(id,name)           (*class_stat_##name[max(id,0)%PLAYERTYPES])
#endif
#endif

#define VANITYMAX 16
struct vanityfile
{
    char *id, *name;
    bool proj;

    vanityfile() : id(NULL), name(NULL), proj(false) {}
    vanityfile(const char *d, const char *n, bool p = false) : id(newstring(d)), name(newstring(n)), proj(p) {}
    ~vanityfile()
    {
        if(id) delete[] id;
        if(name) delete[] name;
    }
};
struct vanitys
{
    int type, cond, style, priv;
    char *ref, *model, *proj, *name, *tag;
    vector<vanityfile> files;

    vanitys() : type(-1), cond(0), style(0), priv(0), ref(NULL), model(NULL), proj(NULL), name(NULL), tag(NULL) {}
    vanitys(int t, const char *r, const char *n, const char *g, int c, int s, int p) : type(t), cond(c), style(s), priv(p), ref(newstring(r)), model(NULL), proj(NULL), name(newstring(n)), tag(newstring(g)) { setmodel(r); }
    ~vanitys()
    {
        if(ref) delete[] ref;
        if(model) delete[] model;
        if(proj) delete[] proj;
        if(name) delete[] name;
        if(tag) delete[] tag;
        loopvrev(files) files.remove(i);
    }

    void setmodel(const char *r)
    {
        if(model) delete[] model;
        defformatstring(m)("vanities/%s", r);
        model = newstring(m);
        if(proj)
        {
            delete[] proj;
            formatstring(m)("vanities/%s/proj", r);
            proj = newstring(m);
        }
    }
};
#ifdef GAMEWORLD
vector<vanitys> vanities;
#else
extern vector<vanitys> vanities;
#endif
CLASSDEF(char *, name);
CLASSDEF(int, health);
CLASSDEF(int, armour);
CLASSDEF(float, weight);
CLASSDEF(float, speed);
