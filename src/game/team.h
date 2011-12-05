enum
{
    TEAM_NEUTRAL = 0, TEAM_ALPHA, TEAM_OMEGA, TEAM_KAPPA, TEAM_SIGMA, TEAM_ENEMY, TEAM_MAX,
    TEAM_FIRST = TEAM_ALPHA, TEAM_LAST = TEAM_OMEGA, TEAM_MULTI = TEAM_SIGMA,
    TEAM_COUNT = TEAM_LAST+1, TEAM_ALL = TEAM_MULTI+1,
    TEAM_NUM = (TEAM_LAST-TEAM_FIRST)+1,
    TEAM_TOTAL = (TEAM_MULTI-TEAM_FIRST)+1
};

#define TEAMS(a,b) \
    GSVAR(0, team##a##name, #a); \
    GVAR(IDF_HEX, team##a##colour, 0, b, 0xFFFFFF);

TEAMS(neutral, 0x90A090);
TEAMS(alpha, 0x5F66FF);
TEAMS(omega, 0xFF4F44);
TEAMS(kappa, 0xFFFF44);
TEAMS(sigma, 0x22FF22);
TEAMS(enemy, 0x999999);

#ifdef GAMESERVER
#define TEAMDEF(proto,name)     proto *sv_team_stat_##name[] = { &sv_teamneutral##name, &sv_teamalpha##name, &sv_teamomega##name, &sv_teamkappa##name, &sv_teamsigma##name, &sv_teamenemy##name };
#define TEAM(team,name)         (*sv_team_stat_##name[team])
#else
#ifdef GAMEWORLD
#define TEAMDEF(proto,name)     proto *team_stat_##name[] = { &teamneutral##name, &teamalpha##name, &teamomega##name, &teamkappa##name, &teamsigma##name, &teamenemy##name };
#else
#define TEAMDEF(proto,name)     extern proto *team_stat_##name[];
#endif
#define TEAM(team,name)         (*team_stat_##name[team])
#endif
TEAMDEF(char *, name);
TEAMDEF(int, colour);

struct score
{
    int team, total;
    score() {}
    score(int s, int n) : team(s), total(n) {}
};
enum { BASE_NONE = 0, BASE_HOME = 1<<0, BASE_FLAG = 1<<1, BASE_BOTH = BASE_HOME|BASE_FLAG };

#define numteams(a,b)   (m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? TEAM_TOTAL : TEAM_NUM) : 1)
#define teamcount(a,b)  (m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? TEAM_ALL : TEAM_COUNT) : 1)
#define isteam(a,b,c,d) (m_fight(a) && m_team(a,b) ? (c >= d && c <= numteams(a,b)) : c == TEAM_NEUTRAL)
#define valteam(a,b)    (a >= b && a <= TEAM_TOTAL)

