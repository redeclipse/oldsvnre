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

#define numteams(a,b)   (m_fight(a) && m_isteam(a,b) ? (m_multi(a,b) ? T_TOTAL : T_NUM) : 1)
#define teamcount(a,b)  (m_fight(a) && m_isteam(a,b) ? (m_multi(a,b) ? T_ALL : T_COUNT) : 1)
#define isteam(a,b,c,d) (m_fight(a) && m_isteam(a,b) ? (c >= d && c <= numteams(a,b)) : c == T_NEUTRAL)
#define valteam(a,b)    (a >= b && a <= T_TOTAL)

#ifdef MEK
#define PLAYERTYPES 3
#ifdef GAMEWORLD
const char *playertypes[PLAYERTYPES][4] = {
    { "actors/mek1",   "actors/mek1/hwep",    "mek1",   "light" },
    { "actors/mek2",   "actors/mek2/hwep",    "mek2",   "medium" },
    { "actors/mek3",   "actors/mek3/hwep",    "mek3",   "heavy" },
};
#else
extern const char *playertypes[PLAYERTYPES][3]; //3
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
CLASSES(mek1,   300,    75,     6,      6,      16,     200,        80); // light
CLASSES(mek2,   400,    100,    6,      6,      16,     300,        65); // medium
CLASSES(mek3,   500,    200,    6,      6,      16,     350,        50); // heavy

#ifdef GAMESERVER
#define CLASSDEF(proto,name)     proto *sv_class_stat_##name[] = { &sv_classmek1##name, &sv_classmek2##name, &sv_classmek3##name };
#define CLASS(id,name)           (*sv_class_stat_##name[max(id,0)%PLAYERTYPES])
#else
#ifdef GAMEWORLD
#define CLASSDEF(proto,name)     proto *class_stat_##name[] = { &classmek1##name, &classmek2##name, &classmek3##name };
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
const char *playertypes[PLAYERTYPES][3] = {
    { "actors/player/male",     "actors/player/male/hwep",      "male" },
    { "actors/player/female",   "actors/player/female/hwep",    "female" }
};
#else
extern const char *playertypes[PLAYERTYPES][3];
#endif
enum {
    V_I_MONOCLE_CLASSY = 0,
    V_I_MONOCLE_BIONIC,
    V_I_TOPHAT_FELT,
    V_I_TOPHAT_METAL,
    V_I_HORNS,
    V_I_MAX,
    V_I_ALL = (1<<V_I_MONOCLE_CLASSY)|(1<<V_I_MONOCLE_BIONIC)|(1<<V_I_TOPHAT_FELT)|(1<<V_I_TOPHAT_METAL)|(1<<V_I_HORNS)
};
enum {
    V_T_EYEPIECE = 0,
    V_T_EYEWEAR,
    V_T_HAT,
    V_T_MAX
};
struct vanitys
{
    const char *model, *name;
    int tag;
};
#ifdef GAMEWORLD
const char *vanitytags[V_T_MAX] = { "tag_eyepiece", "tag_eyewear", "tag_hat" };
vanitys vanities[V_I_MAX] = {
    { "vanities/monocle/classy",    "classymonocle",    V_T_EYEPIECE },
    { "vanities/monocle/bionic",    "bionicmonocle",    V_T_EYEPIECE },
    { "vanities/tophat/felt",       "felttophat",       V_T_HAT },
    { "vanities/tophat/metal",      "metaltophat",      V_T_HAT },
    { "vanities/horns",             "horns",            V_T_HAT },
};
#else
extern const char *vanitytags[V_T_MAX];
extern vanitys vanities[V_I_MAX];
#endif
#ifdef GAMESERVER
SVAR(0, vanityname, "classymonocle bionicmonocle felttophat metaltophat horns");
SVAR(0, vanityeyepieces, "classymonocle bionicmonocle");
SVAR(0, vanityhats, "felttophat metaltophat horns");
VAR(0, vanityclassymonocle, 1, V_I_MONOCLE_CLASSY, -1);
VAR(0, vanitybionicmonocle, 1, V_I_MONOCLE_BIONIC, -1);
VAR(0, vanityfelttophat, 1, V_I_TOPHAT_FELT, -1);
VAR(0, vanitymetaltophat, 1, V_I_TOPHAT_METAL, -1);
VAR(0, vanityhorns, 1, V_I_HORNS, -1);
VAR(0, vanityall, 1, V_I_ALL, -1);
VAR(0, vanitynum, 1, V_I_MAX, -1);
#endif
#endif
