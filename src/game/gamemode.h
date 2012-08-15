enum
{
    G_DEMO = 0, G_EDITMODE, G_CAMPAIGN, G_DEATHMATCH, G_CAPTURE, G_DEFEND, G_BOMBER, G_TRIAL, G_MAX,
    G_START = G_EDITMODE, G_PLAY = G_CAMPAIGN, G_FIGHT = G_DEATHMATCH, G_RAND = G_BOMBER-G_DEATHMATCH+1,
    G_NEVER = (1<<G_DEMO)|(1<<G_EDITMODE),
    G_LIMIT = (1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER),
    G_ALL = (1<<G_DEMO)|(1<<G_EDITMODE)|(1<<G_CAMPAIGN)|(1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER)|(1<<G_TRIAL)
};
enum
{
    G_M_NONE = 0,
    G_M_MULTI = 1<<0, G_M_TEAM = 1<<1, G_M_INSTA = 1<<2, G_M_MEDIEVAL = 1<<3, G_M_BALLISTIC = 1<<4,
    G_M_DUEL = 1<<5, G_M_SURVIVOR = 1<<6, G_M_ARENA = 1<<7, G_M_ONSLAUGHT = 1<<8,
    G_M_JETPACK = 1<<9, G_M_VAMPIRE = 1<<10, G_M_EXPERT = 1<<11, G_M_RESIZE = 1<<12,
    G_M_GSP1 = 1<<13, G_M_GSP2 = 1<<14, G_M_GSP3 = 1<<15,
    G_M_ALL = G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
    G_M_FILTER = G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_SURVIVOR|G_M_ARENA|G_M_JETPACK|G_M_VAMPIRE|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
    G_M_GSN = 3, G_M_GSP = 13, G_M_NUM = 16, G_M_IGN = 2 // instagib number for instagibfilter
};

struct gametypes
{
    int type,           implied,
        mutators[G_M_GSN+1];
    const char *name,                       *gsp[G_M_GSN],
        *desc,                              *gsd[G_M_GSN];
};
struct mutstypes
{
    int type,           implied,            mutators;
    const char *name,
        *desc;
};
#ifdef GAMESERVER
gametypes gametype[] = {
    {
        G_DEMO,         G_M_NONE,
        {
            G_M_NONE, G_M_NONE, G_M_NONE, G_M_NONE
        },
        "demo",                             { "", "", "" },
        "",                                 { "", "", "" },
    },
    {
        G_EDITMODE,     G_M_NONE,
        {
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        "editing",                          { "", "", "" },
        "create and edit existing maps",    { "", "", "" },
    },
    {
        G_CAMPAIGN,     G_M_TEAM,
        {
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        "campaign",                         { "", "", "" },
        "make your way through a maze of monsters", { "", "", "" },
    },
    {
        G_DEATHMATCH,   G_M_NONE,
        {
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        "deathmatch",                       { "", "", "" },
        "shoot to kill and earn points by fragging", { "", "", "" },
    },
    {
        G_CAPTURE,      G_M_TEAM,
        {
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1,
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP2,
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP3
        },
        "capture-the-flag",                 { "return", "defend", "protect" },
        "take the enemy flag and return it to the base", { "dropped flags must be carried back to base", "dropped flags must be defended until they reset", "protect the flag and hold the enemy flag to score" },
    },
    {
        G_DEFEND,       G_M_TEAM,
        {
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_NONE
        },
        "defend-the-flag",                  { "quick", "conquer", "" },
        "secure and defend flags to earn points", { "flags secure quicker than normal", "match ends when all flags are secured", "" },
    },
    {
        G_BOMBER,       G_M_NONE,
        {
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1,
            G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1,
            G_M_NONE, G_M_NONE
        },
        "bomber-ball",                      { "hold", "", "" },
        "get the bomb into the enemy goal to score", { "hold the bomb as long as possible to score points", "", "" },
    },
    {
        G_TRIAL,        G_M_NONE,
        {
            G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        "time-trial",                       { "", "", "" },
        "compete for the fastest time completing a lap", { "", "", "" },
    },
};
mutstypes mutstype[] = {
    {
        G_M_MULTI,      G_M_MULTI,          G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "multi",
        "four teams fight to determine the winning side"
    },
    {
        G_M_TEAM,       G_M_TEAM,           G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "team",
        "two teams fight to determine the winning side"
    },
    {
        G_M_INSTA,      G_M_INSTA,          G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "instagib",
        "one shot, one kill"
    },
    {
        G_M_MEDIEVAL,   G_M_MEDIEVAL,       G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_MEDIEVAL|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "medieval",
        "everyone spawns only with swords"
    },
    {
        G_M_BALLISTIC,  G_M_BALLISTIC,      G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "ballistic",
        "everyone spawns only with rockets"
    },
    {
        G_M_DUEL,       G_M_DUEL,           G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "duel",
        "one on one battles to determine the winner"
    },
    {
        G_M_SURVIVOR,   G_M_SURVIVOR,       G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "survivor",
        "everyone battles to determine the winner"
    },
    {
        G_M_ARENA,      G_M_ARENA,          G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "arena",
        "spawn with a choice of weaponry"
    },
    {
        G_M_ONSLAUGHT,  G_M_ONSLAUGHT,      G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "onslaught",
        "waves of enemies fill the battle arena"
    },
    {
        G_M_JETPACK,    G_M_JETPACK,        G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "jetpack",
        "everyone comes equipped with a jetpack"
    },
    {
        G_M_VAMPIRE,    G_M_VAMPIRE,        G_M_MULTI|G_M_TEAM|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "vampire",
        "deal damage to regenerate health"
    },
    {
        G_M_EXPERT,     G_M_EXPERT,         G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "expert",
        "headshot damage only"
    },
    {
        G_M_RESIZE,     G_M_RESIZE,         G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "resize",
        "everyone changes size depending on their health"
    },
    {
        G_M_GSP1,       G_M_GSP1,           G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "gsp1",
        ""
    },
    {
        G_M_GSP2,       G_M_GSP2,           G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "gsp2",
        ""
    },
    {
        G_M_GSP3,       G_M_GSP3,           G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        "gsp3",
        ""
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
#define m_campaign(a)       (a == G_CAMPAIGN)
#define m_dm(a)             (a == G_DEATHMATCH)
#define m_capture(a)        (a == G_CAPTURE)
#define m_defend(a)         (a == G_DEFEND)
#define m_bomber(a)         (a == G_BOMBER)
#define m_trial(a)          (a == G_TRIAL)

#define m_play(a)           (a >= G_PLAY)
#define m_affinity(a)       (m_capture(a) || m_defend(a) || m_bomber(a))
#define m_fight(a)          (a >= G_FIGHT)

#define m_implied(a,b)      (gametype[a].implied|((b&G_M_MULTI) || (a == G_BOMBER && !((b|gametype[a].implied)&G_M_GSP2)) ? G_M_TEAM : G_M_NONE))
#define m_doimply(a,b,c)    (gametype[a].implied|mutstype[c].implied|((b&G_M_MULTI) || (a == G_BOMBER && !((b|gametype[a].implied|mutstype[c].implied)&G_M_GSP2)) ? G_M_TEAM : G_M_NONE))

#define m_multi(a,b)        ((b&G_M_MULTI) || (m_implied(a,b)&G_M_MULTI))
#define m_team(a,b)         ((b&G_M_TEAM) || (m_implied(a,b)&G_M_TEAM))
#define m_insta(a,b)        ((b&G_M_INSTA) || (m_implied(a,b)&G_M_INSTA))
#define m_medieval(a,b)     ((b&G_M_MEDIEVAL) || (m_implied(a,b)&G_M_MEDIEVAL))
#define m_ballistic(a,b)    ((b&G_M_BALLISTIC) || (m_implied(a,b)&G_M_BALLISTIC))
#define m_duel(a,b)         ((b&G_M_DUEL) || (m_implied(a,b)&G_M_DUEL))
#define m_survivor(a,b)     ((b&G_M_SURVIVOR) || (m_implied(a,b)&G_M_SURVIVOR))
#define m_arena(a,b)        ((b&G_M_ARENA) || (m_implied(a,b)&G_M_ARENA))
#define m_onslaught(a,b)    ((b&G_M_ONSLAUGHT) || (m_implied(a,b)&G_M_ONSLAUGHT))
#define m_jetpack(a,b)      ((b&G_M_JETPACK) || (m_implied(a,b)&G_M_JETPACK))
#define m_vampire(a,b)      ((b&G_M_VAMPIRE) || (m_implied(a,b)&G_M_VAMPIRE))
#define m_expert(a,b)       ((b&G_M_EXPERT) || (m_implied(a,b)&G_M_EXPERT))
#define m_resize(a,b)       ((b&G_M_RESIZE) || (m_implied(a,b)&G_M_RESIZE))

#define m_gsp1(a,b)         ((b&G_M_GSP1) || (m_implied(a,b)&G_M_GSP1))
#define m_gsp2(a,b)         ((b&G_M_GSP2) || (m_implied(a,b)&G_M_GSP2))
#define m_gsp3(a,b)         ((b&G_M_GSP3) || (m_implied(a,b)&G_M_GSP3))
#define m_gsp(a,b)          (m_gsp1(a,b) || m_gsp2(a,b) || m_gsp3(a,b))

#define m_limited(a,b)      (m_insta(a, b) || m_medieval(a, b) || m_ballistic(a, b))
#define m_special(a,b)      (m_arena(a, b) || m_insta(a, b) || m_medieval(a, b) || m_ballistic(a, b))
#define m_duke(a,b)         (m_duel(a, b) || m_survivor(a, b))
#define m_regen(a,b)        (!m_duke(a, b) && !m_insta(a, b))
#define m_enemies(a,b)      (m_campaign(a) || m_onslaught(a, b))
#define m_scores(a)         (m_dm(a))
#define m_checkpoint(a)     (m_campaign(a) || m_trial(a))
#define m_sweaps(a,b)       (m_medieval(a, b) || m_ballistic(a, b) || m_arena(a, b))

#define m_weapon(a,b)       (m_arena(a,b) ? -WEAP_ITEM : (m_medieval(a,b) ? WEAP_SWORD : (m_ballistic(a,b) ? WEAP_ROCKET : (m_insta(a,b) ? GAME(instaweapon) : (m_trial(a) ? GAME(trialweapon) : GAME(spawnweapon))))))
#define m_delay(a,b)        (m_play(a) && !m_duke(a,b) ? (m_trial(a) ? GAME(trialdelay) : (m_bomber(a) ? GAME(bomberdelay) : (m_insta(a, b) ? GAME(instadelay) : GAME(spawndelay)))) : 0)
#define m_protect(a,b)      (m_duke(a,b) ? GAME(duelprotect) : (m_insta(a, b) ? GAME(instaprotect) : GAME(spawnprotect)))
#define m_noitems(a,b)      (m_trial(a) || GAME(itemsallowed) < (m_limited(a,b) ? 2 : 1))
#define m_health(a,b)       (m_insta(a,b) ? 1 : GAME(spawnhealth))

#define w_reload(w1,w2)     (w1 != WEAP_MELEE ? (isweap(w2) ? (w1 == w2 ? -1 : WEAP(w1, reloads)) : (w1 < -w2 ? -1 : WEAP(w1, reloads))) : 0)
#define w_carry(w1,w2)      (w1 > WEAP_MELEE && (isweap(w2) ? w1 != w2 : w1 >= -w2) && (isweap(w1) && WEAP(w1, carried)))
#define w_attr(a,w1,w2)     (m_edit(a) || (w1 >= WEAP_OFFSET && w1 != w2) ? w1 : (w2 == WEAP_GRENADE ? WEAP_ROCKET : WEAP_GRENADE))
#define w_spawn(weap)       int(ceilf(GAME(itemspawntime)*WEAP(weap, frequency)))

#define mapshrink(a,b,c) if((a) && (b) && (c) && *(c)) \
{ \
    char *p = shrinklist(b, c, 1); \
    if(p) \
    { \
        DELETEA(b); \
        b = p; \
    } \
}

#define mapcull(a,b,c,d) \
{ \
    mapshrink(m_multi(b, c) && (m_capture(b) || (m_bomber(b) && !m_gsp2(b, c))), a, GAME(multimaps)); \
    mapshrink(m_duel(b, c), a, GAME(duelmaps)); \
    mapshrink(m_jetpack(b, c), a, GAME(jetpackmaps)); \
    if(d > 0 && GAME(mapsfilter) >= 2 && m_fight(b) && !m_duel(b, c)) \
    { \
        mapshrink(GAME(smallmapmax) && d <= GAME(smallmapmax), a, GAME(smallmaps)) \
        else mapshrink(GAME(mediummapmax) && d <= GAME(mediummapmax), a, GAME(mediummaps)) \
        else mapshrink(GAME(mediummapmax) && d > GAME(mediummapmax), a, GAME(largemaps)) \
    } \
}

#define maplist(a,b,c,d) \
{ \
    if(m_campaign(b)) a = newstring(GAME(campaignmaps)); \
    else if(m_capture(b)) a = newstring(GAME(capturemaps)); \
    else if(m_defend(b)) a = newstring(GAME(defendmaps)); \
    else if(m_bomber(b)) a = newstring(m_gsp2(b, c) ? GAME(holdmaps) : GAME(bombermaps)); \
    else if(m_trial(b)) a = newstring(GAME(trialmaps)); \
    else if(m_fight(b)) a = newstring(GAME(mainmaps)); \
    else a = newstring(GAME(allowmaps)); \
    if(GAME(mapsfilter)) mapcull(a, b, c, d); \
}

#ifdef GAMESERVER
SVAR(0, modename, "demo editing campaign deathmatch capture-the-flag defend-the-flag bomber-ball time-trial");
SVAR(0, modeidxname, "demo editing campaign deathmatch capture defend bomber trial");
VAR(0, modeidxdemo, -1, G_DEMO, 1);
VAR(0, modeidxediting, -1, G_EDITMODE, 1);
VAR(0, modeidxcampaign, -1, G_CAMPAIGN, 1);
VAR(0, modeidxdeathmatch, -1, G_DEATHMATCH, 1);
VAR(0, modeidxcapture, -1, G_CAPTURE, 1);
VAR(0, modeidxdefend, -1, G_DEFEND, 1);
VAR(0, modeidxbomber, -1, G_BOMBER, 1);
VAR(0, modeidxtrial, -1, G_TRIAL, 1);
VAR(0, modeidxstart, -1, G_START, 1);
VAR(0, modeidxplay, -1, G_PLAY, 1);
VAR(0, modeidxfight, -1, G_FIGHT, 1);
VAR(0, modeidxrand, -1, G_RAND, 1);
VAR(0, modeidxnever, -1, G_NEVER, 1);
VAR(0, modeidxlimit, -1, G_LIMIT, 1);
VAR(0, modeidxall, -1, G_ALL, 1);
VAR(0, modeidxnum, -1, G_MAX, 1);
SVAR(0, mutsname, "multi teamplay instagib medieval ballistic duel survivor arena onslaught jetpack vampire expert resize");
SVAR(0, mutsidxname, "multi team instagib medieval ballistic duel survivor arena onslaught jetpack vampire expert resize");
VAR(0, mutsidxmulti, -1, G_M_MULTI, 1);
VAR(0, mutsidxteam, -1, G_M_TEAM, 1);
VAR(0, mutsidxinstagib, -1, G_M_INSTA, 1);
VAR(0, mutsidxmedieval, -1, G_M_MEDIEVAL, 1);
VAR(0, mutsidxballistic, -1, G_M_BALLISTIC, 1);
VAR(0, mutsidxduel, -1, G_M_DUEL, 1);
VAR(0, mutsidxsurvivor, -1, G_M_SURVIVOR, 1);
VAR(0, mutsidxarena, -1, G_M_ARENA, 1);
VAR(0, mutsidxonslaught, -1, G_M_ONSLAUGHT, 1);
VAR(0, mutsidxjetpack, -1, G_M_JETPACK, 1);
VAR(0, mutsidxvampire, -1, G_M_VAMPIRE, 1);
VAR(0, mutsidxexpert, -1, G_M_EXPERT, 1);
VAR(0, mutsidxresize, -1, G_M_RESIZE, 1);
VAR(0, mutsidxgsp1, -1, G_M_GSP1, 1);
VAR(0, mutsidxgsp2, -1, G_M_GSP2, 1);
VAR(0, mutsidxgsp3, -1, G_M_GSP3, 1);
VAR(0, mutsidxall, -1, G_M_ALL, 1);
VAR(0, mutsidxfilter, -1, G_M_FILTER, 1);
VAR(0, mutsidxgsn, -1, G_M_GSN, 1);
VAR(0, mutsidxgsp, -1, G_M_GSP, 1);
VAR(0, mutsidxnum, -1, G_M_NUM, 1);
#endif
