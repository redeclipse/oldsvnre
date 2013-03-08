#ifdef CAMPAIGN
enum
{
    G_DEMO = 0, G_EDITMODE, G_CAMPAIGN, G_DEATHMATCH, G_CAPTURE, G_DEFEND, G_BOMBER, G_TRIAL, G_GAUNTLET, G_MAX,
    G_START = G_EDITMODE, G_PLAY = G_CAMPAIGN, G_FIGHT = G_DEATHMATCH,
    G_RAND = G_BOMBER-G_DEATHMATCH+1, G_COUNT = G_MAX-G_PLAY,
    G_NEVER = (1<<G_DEMO)|(1<<G_EDITMODE),
    G_LIMIT = (1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER),
    G_ALL = (1<<G_DEMO)|(1<<G_EDITMODE)|(1<<G_CAMPAIGN)|(1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER)|(1<<G_TRIAL)|(1<<G_GAUNTLET),
    G_SW = (1<<G_TRIAL),
};
#else
enum
{
    G_DEMO = 0, G_EDITMODE, G_DEATHMATCH, G_CAPTURE, G_DEFEND, G_BOMBER, G_TRIAL, G_GAUNTLET, G_MAX,
    G_START = G_EDITMODE, G_PLAY = G_DEATHMATCH, G_FIGHT = G_DEATHMATCH,
    G_RAND = G_BOMBER-G_DEATHMATCH+1, G_COUNT = G_MAX-G_PLAY,
    G_NEVER = (1<<G_DEMO)|(1<<G_EDITMODE),
    G_LIMIT = (1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER)|(1<<G_GAUNTLET),
    G_ALL = (1<<G_DEMO)|(1<<G_EDITMODE)|(1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER)|(1<<G_TRIAL)|(1<<G_GAUNTLET),
    G_SW = (1<<G_TRIAL),
};
#endif
enum
{
    G_M_MULTI = 0, G_M_FFA, G_M_COOP, G_M_INSTA, G_M_MEDIEVAL, G_M_KABOOM, G_M_DUEL, G_M_SURVIVOR,
    G_M_CLASSIC, G_M_ONSLAUGHT, G_M_JETPACK, G_M_VAMPIRE, G_M_EXPERT, G_M_RESIZE,
    G_M_GSP, G_M_GSP1 = G_M_GSP, G_M_GSP2, G_M_GSP3, G_M_NUM,
    G_M_GSN = G_M_NUM-G_M_GSP,
    G_M_ALL = (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
    G_M_FILTER = (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
    G_M_ROTATE = (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
    G_M_SW = (1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM),
    G_M_DK = (1<<G_M_DUEL)|(1<<G_M_SURVIVOR)
};

enum { G_F_GSP = 0, G_F_NUM };

struct gametypes
{
    int type, flags, implied, mutators[G_M_GSN+1];
    const char *name, *gsp[G_M_GSN], *desc, *gsd[G_M_GSN];
};
struct mutstypes
{
    int type, implied, mutators;
    const char *name, *desc;
};
#ifdef GAMESERVER
gametypes gametype[] = {
    {
        G_DEMO, 0, 0, { 0, 0, 0, 0 },
        "demo", { "", "", "" },
        "play back previously recorded games", { "", "", "" },
    },
    {
        G_EDITMODE, 0, (1<<G_M_FFA)|(1<<G_M_CLASSIC),
        {
            (1<<G_M_FFA)|(1<<G_M_CLASSIC)|(1<<G_M_JETPACK),
            0, 0, 0
        },
        "editing", { "", "", "" },
        "create and edit existing maps", { "", "", "" },
    },
#ifdef CAMPAIGN
    {
        G_CAMPAIGN, 0, 0,
        {
            (1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE),
            0, 0, 0
        },
        "campaign", { "", "", "" },
        "make your way through the mission alive", { "", "", "" },
    },
#endif
    {
        G_DEATHMATCH, 0, 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE),
            0, 0, 0
        },
        "deathmatch", { "", "", "" },
        "shoot to kill and earn points by fragging", { "", "", "" },
    },
    {
        G_CAPTURE, 0, 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP3)
        },
        "capture-the-flag", { "return", "defend", "protect" },
        "take the enemy flag and return it to the base", { "dropped flags must be carried back to base", "dropped flags must be defended until they reset", "protect the flag and hold the enemy flag to score" },
    },
    {
        G_DEFEND, 0, 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP3)
        },
        "defend-the-flag", { "quick", "conquer", "king" },
        "defend the flags to earn points", { "flags secure quicker than normal", "match ends when all flags are secured", "king of the hill with one flag" },
    },
    {
        G_BOMBER, (1<<G_F_GSP), 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            0, 0
        },
        "bomber-ball", { "hold", "", "" },
        "get the bomb into the enemy goal to score", { "hold the bomb as long as possible to score points", "", "" },
    },
    {
        G_TRIAL, 0, 0,
        {
            (1<<G_M_FFA)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE),
            0, 0, 0
        },
        "time-trial", { "", "", "" },
        "compete for the fastest time completing a lap", { "", "", "" },
    },
    {
        G_GAUNTLET, 0, 0,
        {
            (1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            (1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            0, 0
        },
        "gauntlet", { "timed", "", "" },
        "compete for the most laps while the other team attacks", { "compete for the best lap time while the other team attacks", "", "" },
    },
};
mutstypes mutstype[] = {
    {
        G_M_MULTI, (1<<G_M_MULTI),
        (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "multi", "four teams fight to determine the winning side"
    },
    {
        G_M_FFA, (1<<G_M_FFA),
        (1<<G_M_FFA)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "ffa", "every player for themselves"
    },
    {
        G_M_COOP, (1<<G_M_COOP),
        (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "coop", "players versus drones"
    },
    {
        G_M_INSTA, (1<<G_M_INSTA),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "instagib", "one shot, one kill"
    },
    {
        G_M_MEDIEVAL, (1<<G_M_MEDIEVAL),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_MEDIEVAL)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "medieval", "everyone spawns only with swords"
    },
    {
        G_M_KABOOM,  (1<<G_M_KABOOM),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "kaboom",
        "everyone spawns with explosives only"
    },
    {
        G_M_DUEL, (1<<G_M_DUEL),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "duel", "one on one battles to determine the winner"
    },
    {
        G_M_SURVIVOR, (1<<G_M_SURVIVOR),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "survivor", "everyone battles to determine the winner"
    },
    {
        G_M_CLASSIC,    (1<<G_M_CLASSIC),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "classic",
        "weapons must be collected from spawns in the arena"
    },
    {
        G_M_ONSLAUGHT, (1<<G_M_ONSLAUGHT),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "onslaught", "waves of enemies fill the battle arena"
    },
    {
        G_M_JETPACK, (1<<G_M_JETPACK),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "jetpack", "everyone comes equipped with a jetpack"
    },
    {
        G_M_VAMPIRE, (1<<G_M_VAMPIRE),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "vampire", "deal damage to regenerate health"
    },
    {
        G_M_EXPERT, (1<<G_M_EXPERT),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "expert", "players can only deal damage by landing headshots"
    },
    {
        G_M_RESIZE, (1<<G_M_RESIZE),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "resize", "everyone changes size depending on their health"
    },
    {
        G_M_GSP1, (1<<G_M_GSP1),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "gsp1", ""
    },
    {
        G_M_GSP2, (1<<G_M_GSP2),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "gsp2", ""
    },
    {
        G_M_GSP3, (1<<G_M_GSP3),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        "gsp3", ""
    },
};
#else
extern gametypes gametype[];
extern mutstypes mutstype[];
#endif

#define m_game(a)           (a > -1 && a < G_MAX)
#define m_check(a,b,c,d)    ((!a || (a < 0 ? !((0-a)&(1<<(c-G_PLAY))) : a&(1<<(c-G_PLAY)))) && (!b || (b < 0 ? !((0-b)&d) : b&d)))
#define m_local(a)          (a == G_DEMO)

#define m_demo(a)           (a == G_DEMO)
#define m_edit(a)           (a == G_EDITMODE)
#ifdef CAMPAIGN
#define m_campaign(a)       (a == G_CAMPAIGN)
#endif
#define m_dm(a)             (a == G_DEATHMATCH)
#define m_capture(a)        (a == G_CAPTURE)
#define m_defend(a)         (a == G_DEFEND)
#define m_bomber(a)         (a == G_BOMBER)
#define m_trial(a)          (a == G_TRIAL)
#define m_gauntlet(a)       (a == G_GAUNTLET)

#define m_play(a)           (a >= G_PLAY)
#define m_affinity(a)       (m_capture(a) || m_defend(a) || m_bomber(a))
#define m_fight(a)          (a >= G_FIGHT)

#define m_multi(a,b)        ((b&(1<<G_M_MULTI)) || (gametype[a].implied&(1<<G_M_MULTI)))
#define m_ffa(a,b)          ((b&(1<<G_M_FFA)) || (gametype[a].implied&(1<<G_M_FFA)))
#define m_coop(a,b)         ((b&(1<<G_M_COOP)) || (gametype[a].implied&(1<<G_M_COOP)))
#define m_insta(a,b)        ((b&(1<<G_M_INSTA)) || (gametype[a].implied&(1<<G_M_INSTA)))
#define m_medieval(a,b)     ((b&(1<<G_M_MEDIEVAL)) || (gametype[a].implied&(1<<G_M_MEDIEVAL)))
#define m_kaboom(a,b)       ((b&(1<<G_M_KABOOM)) || (gametype[a].implied&(1<<G_M_KABOOM)))
#define m_duel(a,b)         ((b&(1<<G_M_DUEL)) || (gametype[a].implied&(1<<G_M_DUEL)))
#define m_survivor(a,b)     ((b&(1<<G_M_SURVIVOR)) || (gametype[a].implied&(1<<G_M_SURVIVOR)))
#define m_classic(a,b)      ((b&(1<<G_M_CLASSIC)) || (gametype[a].implied&(1<<G_M_CLASSIC)))
#define m_onslaught(a,b)    ((b&(1<<G_M_ONSLAUGHT)) || (gametype[a].implied&(1<<G_M_ONSLAUGHT)))
#define m_jetpack(a,b)      ((b&(1<<G_M_JETPACK)) || (gametype[a].implied&(1<<G_M_JETPACK)))
#define m_vampire(a,b)      ((b&(1<<G_M_VAMPIRE)) || (gametype[a].implied&(1<<G_M_VAMPIRE)))
#define m_expert(a,b)       ((b&(1<<G_M_EXPERT)) || (gametype[a].implied&(1<<G_M_EXPERT)))
#define m_resize(a,b)       ((b&(1<<G_M_RESIZE)) || (gametype[a].implied&(1<<G_M_RESIZE)))

#define m_gsp1(a,b)         ((b&(1<<G_M_GSP1)) || (gametype[a].implied&(1<<G_M_GSP1)))
#define m_gsp2(a,b)         ((b&(1<<G_M_GSP2)) || (gametype[a].implied&(1<<G_M_GSP2)))
#define m_gsp3(a,b)         ((b&(1<<G_M_GSP3)) || (gametype[a].implied&(1<<G_M_GSP3)))
#define m_gsp(a,b)          (m_gsp1(a,b) || m_gsp2(a,b) || m_gsp3(a,b))

#define m_team(a,b)         (m_multi(a, b) || (!(b&(1<<G_M_FFA)) && !(gametype[a].implied&(1<<G_M_FFA))))
#define m_sweaps(a,b)       (m_insta(a, b) || m_medieval(a, b) || m_kaboom(a, b))
#define m_limited(a,b)      (m_insta(a, b) || m_medieval(a, b))
#define m_special(a,b)      (m_sweaps(a, b) || !m_classic(a, b))
#define m_loadout(a,b)      (!m_classic(a, b) && !m_sweaps(a, b))
#define m_duke(a,b)         (m_duel(a, b) || m_survivor(a, b))
#define m_regen(a,b)        (!m_duke(a, b) && !m_insta(a, b))
#ifdef CAMPAIGN
#define m_enemies(a,b)      (m_campaign(a) || m_onslaught(a, b))
#define m_checkpoint(a)     (m_campaign(a) || m_trial(a) || m_gauntlet(a))
#define m_ghost(a)          (m_campaign(a) ? (G(campaignghost) ? 2 : 0) : (m_trial(a) ? G(trialghost) : (m_gauntlet(a) ? G(gauntletghost) : 0)))
#else
#define m_enemies(a,b)      (m_onslaught(a, b))
#define m_checkpoint(a)     (m_trial(a) || m_gauntlet(a))
#define m_ghost(a)          (m_trial(a) ? G(trialghost) : (m_gauntlet(a) ? G(gauntletghost) : 0))
#endif
#define m_bots(a)           (m_fight(a) && !m_trial(a) && !m_gauntlet(a))
#define m_scores(a)         (m_dm(a))
#define m_laptime(a,b)      (m_trial(a) || (m_gauntlet(a) && m_gsp1(a, b)))

#define m_weapon(a,b)       (m_loadout(a, b) ? 0-W_ITEM : (m_medieval(a, b) ? W_SWORD : (m_kaboom(a, b) ? 0-W_BOOM : (m_insta(a, b) ? G(instaweapon) : (m_gauntlet(a) ? G(gauntletweapon) : (m_trial(a) ? G(trialweapon) : G(spawnweapon)))))))
#define m_delay(a,b)        (m_play(a) && !m_duke(a,b) ? (m_trial(a) ? G(trialdelay) : (m_bomber(a) ? G(bomberdelay) : (m_insta(a, b) ? G(instadelay) : G(spawndelay)))) : 0)
#define m_protect(a,b)      (m_duke(a,b) ? G(duelprotect) : (m_insta(a, b) ? G(instaprotect) : G(spawnprotect)))
#define m_noitems(a,b)      (m_trial(a) || G(itemsallowed) < (m_limited(a,b) ? 2 : 1))
#ifdef MEK
#define m_health(a,b,c)     (m_insta(a,b) ? 1 : CLASS(c, health))
#define m_armour(a,b,c)     (m_insta(a,b) ? 0 : CLASS(c, armour))
#else
#define m_health(a,b,c)     (m_insta(a,b) ? 1 : G(spawnhealth))
#define m_armour(a,b,c)     (m_insta(a,b) ? 0 : G(spawnarmour))
#endif
#define m_maxhealth(a,b,c)  (int(m_health(a, b, c)*(m_vampire(a,b) ? G(maxhealthvampire) : G(maxhealth))))
#define m_balance(a)        (m_trial(a) ? 0 : (m_gauntlet(a) ? 1 : (G(forcebalance) >= 0 ? G(forcebalance) : G(mapbalance))))
#define m_balreset(a)       (m_capture(a) || m_bomber(a) || m_gauntlet(a))

#define w_reload(w1,w2)     (w1 != W_MELEE ? (isweap(w2) ? (w1 == w2 ? -1 : W(w1, reloads)) : (w1 < 0-w2 ? -1 : W(w1, reloads))) : 0)
#define w_carry(w1,w2)      (w1 > W_MELEE && (isweap(w2) ? w1 != w2 : w1 >= 0-w2) && (isweap(w1) && W(w1, carried)))
#define w_attx(a,b,w)       (m_kaboom(a,b) && ((w) == W_GRENADE || (w) == W_MINE) ? W_ROCKET : (w))
#define w_attr(a,b,w1,w2)   (m_edit(a) ? w1 : w_attx(a, b, ((w1 >= W_OFFSET && w1 != w2) ? w1 : (w2 == W_GRENADE ? W_MINE : W_GRENADE))))
#define w_spawn(weap)       int(ceilf(G(itemspawntime)*W(weap, frequency)))

#define mapshrink(a,b,c) if((a) && (b) && (c) && *(c)) \
{ \
    char *p = shrinklist(b, c, 1); \
    if(p) \
    { \
        DELETEA(b); \
        b = p; \
    } \
}

#define mapcull(a,b,c,d,e) \
{ \
    mapshrink(m_multi(b, c) && (m_capture(b) || (m_bomber(b) && !m_gsp2(b, c))), a, G(multimaps)); \
    mapshrink(m_duel(b, c), a, G(duelmaps)); \
    mapshrink(m_jetpack(b, c), a, G(jetpackmaps)); \
    if(d > 0 && e >= 2 && m_fight(b) && !m_duel(b, c)) \
    { \
        mapshrink(G(smallmapmax) && d <= G(smallmapmax), a, G(smallmaps)) \
        else mapshrink(G(mediummapmax) && d <= G(mediummapmax), a, G(mediummaps)) \
        else mapshrink(G(mediummapmax) && d > G(mediummapmax), a, G(largemaps)) \
    } \
}
#ifdef CAMPAIGN
#define maplist(a,b,c,d,e) \
{ \
    if(m_campaign(b)) a = newstring(G(campaignmaps)); \
    else if(m_capture(b)) a = newstring(G(capturemaps)); \
    else if(m_defend(b)) a = newstring(m_gsp3(b, c) ? G(kingmaps) : G(defendmaps)); \
    else if(m_bomber(b)) a = newstring(m_gsp2(b, c) ? G(holdmaps) : G(bombermaps)); \
    else if(m_trial(b)) a = newstring(G(trialmaps)); \
    else if(m_gauntlet(b)) a = newstring(G(gauntletmaps)); \
    else if(m_fight(b)) a = newstring(G(mainmaps)); \
    else a = newstring(G(allowmaps)); \
    if(e) mapcull(a, b, c, d, e); \
}
#else
#define maplist(a,b,c,d,e) \
{ \
    if(m_capture(b)) a = newstring(G(capturemaps)); \
    else if(m_defend(b)) a = newstring(m_gsp3(b, c) ? G(kingmaps) : G(defendmaps)); \
    else if(m_bomber(b)) a = newstring(m_gsp2(b, c) ? G(holdmaps) : G(bombermaps)); \
    else if(m_trial(b)) a = newstring(G(trialmaps)); \
    else if(m_gauntlet(b)) a = newstring(G(gauntletmaps)); \
    else if(m_fight(b)) a = newstring(G(mainmaps)); \
    else a = newstring(G(allowmaps)); \
    if(e) mapcull(a, b, c, d, e); \
}
#endif
#ifdef GAMESERVER
#ifdef CAMPAIGN
SVAR(0, modename, "demo editing campaign deathmatch capture-the-flag defend-the-flag bomber-ball time-trial gauntlet");
SVAR(0, modeidxname, "demo editing campaign deathmatch capture defend bomber trial gauntlet");
VAR(0, modeidxcampaign, 1, G_CAMPAIGN, -1);
VAR(0, modebitcampaign, 1, (1<<G_CAMPAIGN), -1);
#else
SVAR(0, modename, "demo editing deathmatch capture-the-flag defend-the-flag bomber-ball time-trial gauntlet");
SVAR(0, modeidxname, "demo editing deathmatch capture defend bomber trial gauntlet");
#endif
VAR(0, modeidxdemo, 1, G_DEMO, -1);
VAR(0, modeidxediting, 1, G_EDITMODE, -1);
VAR(0, modeidxdeathmatch, 1, G_DEATHMATCH, -1);
VAR(0, modeidxcapture, 1, G_CAPTURE, -1);
VAR(0, modeidxdefend, 1, G_DEFEND, -1);
VAR(0, modeidxbomber, 1, G_BOMBER, -1);
VAR(0, modeidxtrial, 1, G_TRIAL, -1);
VAR(0, modeidxgauntlet, 1, G_GAUNTLET, -1);
VAR(0, modeidxstart, 1, G_START, -1);
VAR(0, modeidxplay, 1, G_PLAY, -1);
VAR(0, modeidxfight, 1, G_FIGHT, -1);
VAR(0, modeidxrand, 1, G_RAND, -1);
VAR(0, modeidxnever, 1, G_NEVER, -1);
VAR(0, modeidxlimit, 1, G_LIMIT, -1);
VAR(0, modeidxall, 1, G_ALL, -1);
VAR(0, modeidxnum, 1, G_MAX, -1);
VAR(0, modebitdemo, 1, (1<<G_DEMO), -1);
VAR(0, modebitediting, 1, (1<<G_EDITMODE), -1);
VAR(0, modebitdeathmatch, 1, (1<<G_DEATHMATCH), -1);
VAR(0, modebitcapture, 1, (1<<G_CAPTURE), -1);
VAR(0, modebitdefend, 1, (1<<G_DEFEND), -1);
VAR(0, modebitbomber, 1, (1<<G_BOMBER), -1);
VAR(0, modebittrial, 1, (1<<G_TRIAL), -1);
VAR(0, modebitgauntlet, 1, (1<<G_GAUNTLET), -1);
SVAR(0, mutsname, "multi ffa coop instagib medieval kaboom duel survivor classic onslaught jetpack vampire expert resize");
SVAR(0, mutsidxname, "multi ffa coop instagib medieval kaboom duel survivor classic onslaught jetpack vampire expert resize");
VAR(0, mutsidxmulti, 1, G_M_MULTI, -1);
VAR(0, mutsidxffa, 1, G_M_FFA, -1);
VAR(0, mutsidxcoop, 1, G_M_COOP, -1);
VAR(0, mutsidxinstagib, 1, G_M_INSTA, -1);
VAR(0, mutsidxmedieval, 1, G_M_MEDIEVAL, -1);
VAR(0, mutsidxkaboom, 1, G_M_KABOOM, -1);
VAR(0, mutsidxduel, 1, G_M_DUEL, -1);
VAR(0, mutsidxsurvivor, 1, G_M_SURVIVOR, -1);
VAR(0, mutsidxclassic, 1, G_M_CLASSIC, -1);
VAR(0, mutsidxonslaught, 1, G_M_ONSLAUGHT, -1);
VAR(0, mutsidxjetpack, 1, G_M_JETPACK, -1);
VAR(0, mutsidxvampire, 1, G_M_VAMPIRE, -1);
VAR(0, mutsidxexpert, 1, G_M_EXPERT, -1);
VAR(0, mutsidxresize, 1, G_M_RESIZE, -1);
VAR(0, mutsidxgsp1, 1, G_M_GSP1, -1);
VAR(0, mutsidxgsp2, 1, G_M_GSP2, -1);
VAR(0, mutsidxgsp3, 1, G_M_GSP3, -1);
VAR(0, mutsidxgsn, 1, G_M_GSN, -1);
VAR(0, mutsidxgsp, 1, G_M_GSP, -1);
VAR(0, mutsidxnum, 1, G_M_NUM, -1);
VAR(0, mutsbitmulti, 1, (1<<G_M_MULTI), -1);
VAR(0, mutsbitffa, 1, (1<<G_M_FFA), -1);
VAR(0, mutsbitcoop, 1, (1<<G_M_COOP), -1);
VAR(0, mutsbitinstagib, 1, (1<<G_M_INSTA), -1);
VAR(0, mutsbitmedieval, 1, (1<<G_M_MEDIEVAL), -1);
VAR(0, mutsbitkaboom, 1, (1<<G_M_KABOOM), -1);
VAR(0, mutsbitduel, 1, (1<<G_M_DUEL), -1);
VAR(0, mutsbitsurvivor, 1, (1<<G_M_SURVIVOR), -1);
VAR(0, mutsbitclassic, 1, (1<<G_M_CLASSIC), -1);
VAR(0, mutsbitonslaught, 1, (1<<G_M_ONSLAUGHT), -1);
VAR(0, mutsbitjetpack, 1, (1<<G_M_JETPACK), -1);
VAR(0, mutsbitvampire, 1, (1<<G_M_VAMPIRE), -1);
VAR(0, mutsbitexpert, 1, (1<<G_M_EXPERT), -1);
VAR(0, mutsbitresize, 1, (1<<G_M_RESIZE), -1);
VAR(0, mutsbitgsp1, 1, (1<<G_M_GSP1), -1);
VAR(0, mutsbitgsp2, 1, (1<<G_M_GSP2), -1);
VAR(0, mutsbitgsp3, 1, (1<<G_M_GSP3), -1);
#endif
