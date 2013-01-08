enum
{
    WEAP_MELEE = 0, WEAP_PISTOL, WEAP_OFFSET, // end of unselectable weapon set
    WEAP_SWORD = WEAP_OFFSET, WEAP_SHOTGUN, WEAP_SMG, WEAP_FLAMER, WEAP_PLASMA, WEAP_RIFLE, WEAP_ITEM,
    WEAP_GRENADE = WEAP_ITEM, WEAP_MINE, WEAP_ROCKET, // end of item weapon set
    WEAP_MAX, WEAP_LOADOUT = WEAP_ITEM-WEAP_OFFSET // if you add to this at all, check all arrays with WEAP_MAX
};
#define isweap(a)       (a >= 0 && a < WEAP_MAX)

enum { WEAP_F_NONE = 0, WEAP_F_FORCED = 1<<0 };
enum {
    WEAP_S_IDLE = 0, WEAP_S_PRIMARY, WEAP_S_SECONDARY, WEAP_S_RELOAD, WEAP_S_POWER,
    WEAP_S_SWITCH, WEAP_S_USE, WEAP_S_WAIT, WEAP_S_MAX,
    WEAP_S_FILTER = (1<<WEAP_S_RELOAD)|(1<<WEAP_S_SWITCH),
    WEAP_S_ALL = (1<<WEAP_S_PRIMARY)|(1<<WEAP_S_SECONDARY)|(1<<WEAP_S_RELOAD)|(1<<WEAP_S_POWER)|(1<<WEAP_S_SWITCH)|(1<<WEAP_S_USE)|(1<<WEAP_S_WAIT)
};

enum
{
    S_W_PRIMARY = 0, S_W_SECONDARY,
    S_W_POWER, S_W_POWER2,
    S_W_SWITCH, S_W_RELOAD, S_W_NOTIFY,
    S_W_EXPLODE, S_W_EXPLODE2,
    S_W_DESTROY, S_W_DESTROY2,
    S_W_EXTINGUISH, S_W_EXTINGUISH2,
    S_W_TRANSIT, S_W_TRANSIT2,
    S_W_BOUNCE, S_W_BOUNCE2,
    S_W_OFFSET, S_W_USE = S_W_OFFSET, S_W_SPAWN,
    S_W_MAX
};

enum
{
    S_MELEE     = S_GAME,
    S_PISTOL    = S_MELEE+S_W_OFFSET,
    S_SWORD     = S_PISTOL+S_W_OFFSET,
    S_SHOTGUN   = S_SWORD+S_W_MAX,
    S_SMG       = S_SHOTGUN+S_W_MAX,
    S_FLAMER    = S_SMG+S_W_MAX,
    S_PLASMA    = S_FLAMER+S_W_MAX,
    S_RIFLE     = S_PLASMA+S_W_MAX,
    S_GRENADE   = S_RIFLE+S_W_MAX,
    S_MINE      = S_GRENADE+S_W_MAX,
    S_ROCKET    = S_MINE+S_W_MAX,
    S_MAX
};

enum
{
    COLLIDE_TRACE = 1<<0, COLLIDE_PROJ = 1<<1, COLLIDE_OWNER = 1<<2,
    IMPACT_GEOM = 1<<3, IMPACT_PLAYER = 1<<4, IMPACT_SHOTS = 1<<5,
    BOUNCE_GEOM = 1<<6, BOUNCE_PLAYER = 1<<7, BOUNCE_SHOTS = 1<<8,
    DRILL_GEOM = 1<<9, DRILL_PLAYER = 1<<10, DRILL_SHOTS = 1<<11,
    STICK_GEOM = 1<<12, STICK_PLAYER = 1<<13,
    COLLIDE_GEOM = IMPACT_GEOM|BOUNCE_GEOM,
    COLLIDE_PLAYER = IMPACT_PLAYER|BOUNCE_PLAYER,
    COLLIDE_SHOTS = IMPACT_SHOTS|BOUNCE_SHOTS,
    COLLIDE_DYNENT = COLLIDE_PLAYER|COLLIDE_SHOTS
};

enum
{
    HIT_NONE = 0, HIT_ALT = 1<<0, HIT_LEGS = 1<<1, HIT_TORSO = 1<<2, HIT_WHIPLASH = 1<<3, HIT_HEAD = 1<<4,
    HIT_WAVE = 1<<5, HIT_PROJ = 1<<6, HIT_EXPLODE = 1<<7, HIT_BURN = 1<<8, HIT_BLEED = 1<<9,
    HIT_MELT = 1<<10, HIT_DEATH = 1<<11, HIT_WATER = 1<<12, HIT_SPAWN = 1<<13,
    HIT_LOST = 1<<14, HIT_KILL = 1<<15, HIT_CRIT = 1<<16, HIT_FLAK = 1<<17, HIT_SPEC = 1<<18,
    HIT_CLEAR = HIT_PROJ|HIT_EXPLODE|HIT_BURN|HIT_BLEED|HIT_MELT|HIT_DEATH|HIT_WATER|HIT_SPAWN|HIT_LOST,
    HIT_SFLAGS = HIT_KILL|HIT_CRIT
};

struct shotmsg { int id; ivec pos; };
struct hitmsg { int flags, proj, target, dist; ivec dir; };

#define hithead(x)      (x&HIT_WHIPLASH || x&HIT_HEAD)
#define hithurts(x)     (x&HIT_BURN || x&HIT_BLEED || x&HIT_EXPLODE || x&HIT_PROJ || x&HIT_MELT || x&HIT_DEATH || x&HIT_WATER)
#define doesburn(x,y)   (isweap(x) && WEAP2(x, residual, y&HIT_ALT) == 1)
#define doesbleed(x,y)  (isweap(x) && WEAP2(x, residual, y&HIT_ALT) == 2)
#define WALT(x)         (WEAP_MAX+WEAP_##x)

#include "weapdef.h"

WPSVAR(0, name,
    "melee",    "pistol",   "sword",    "shotgun",  "smg",      "flamer",   "plasma",   "rifle",    "grenade",  "mine",     "rocket"
);
WPVAR(0, add, 1, VAR_MAX,
    1,          10,         1,          2,          40,         25,         20,         5,          1,          1,          1
);
WPVARM(0, adelay, 1, VAR_MAX,
    250,        150,        500,        500,        100,        100,        300,        750,        1000,       1000,       1000,
    1000,       350,        750,        900,        450,        500,        1000,       1000,       1000,       1000,       1000
);
WPFVARM(0, aidist, 0, FVAR_MAX,
    16.0f,      256.0f,     48.0f,      256.0f,     512.0f,     64.0f,      128.0f,     768.0f,     384.0f,     128.0f,     1024.0f,
    16.0f,      256.0f,     48.0f,      512.0f,     96.0f,      128.0f,     64.0f,      2048.0f,    256.0f,     128.0f,     512.0f
);
WPVARM(0, aiskew, 0, VAR_MAX,
    1,          100,        1,          10,         20,         10,         50,         40,         5,          5,          10,
    1,          100,        1,          10,         20,         10,         10,         40,         5,          5,          10
);
WPVAR(0, allowed, 0, 3,
    2,          2,          2,          2,          2,          2,          2,          2,          3,          3,          3
);
WPVAR(0, carried, 0, 1,
    0,          0,          1,          1,          1,          1,          1,          1,          0,          0,          1
);
WPVAR(IDF_HEX, colour, 0, 0xFFFFFF,
    0xEEEEEE,   0x888888,   0x4444FF,   0x999900,   0xFF6600,   0xFF2222,   0x44DDCC,   0x8822DD,   0x119911,   0x225522,   0xAA3300
);
WPVARM(0, cooked, 0, VAR_MAX,
    0,          0,          0,          0,          0,          0,          0,          0,          8,          8,          8,
    0,          0,          0,          0,          0,          1,          33,         0,          8,          8,          8
);
WPFVAR(0, critboost, 0, FVAR_MAX,
    2.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f
);
WPVARM(0, critdash, 0, VAR_MAX,
    500,        0,          500,        0,          0,          0,          0,          0,          0,          0,          0,
    500,        0,          500,        0,          0,          0,          0,          0,          0,          0,          0
);
WPFVAR(0, critdist, 0, FVAR_MAX,
    0.0f,       128.0f,     0.0f,       256.0f,     128.0f,     64.0f,      0.0f,       1024.0f,    0.0f,       0.0f,       0.0f
);
WPFVAR(0, critmult, 0, FVAR_MAX,
    2.0f,       3.0f,       2.0f,       2.0f,       3.0f,       4.0f,       2.0f,       2.0f,       2.0f,       2.0f,       2.0f
);
WPVARM(0, damage, VAR_MIN, VAR_MAX,
    30,         35,         30,         15,         22,         5,          15,         35,         100,        100,        150,
    40,         5,          60,         4,          4,          5,          10,         150,        100,        50,         150
);
WPFVARM(0, delta, 0, FVAR_MAX,
    10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f,
    10.0f,      10.0f,      10.0f,      10.0f,      1000.0f,    10.0f,      10.0f,      10.0f,      10.0f,      10.0f,      10.0f
);
WPVARM(0, drill, 0, VAR_MAX,
    0,          0,          0,          2,          2,          0,          0,          2,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          4,          0,          0,          0
);
WPVARM(0, edelay, 0, VAR_MAX,
    200,        200,        200,        200,        200,        200,        200,        200,        200,        200,        200,
    200,        200,        200,        200,        200,        200,        200,        200,        200,        200,        200
);
WPFVARM(0, elasticity, 0, FVAR_MAX,
    0.5f,       0.5f,       0.5f,       0.5f,       0.65f,      0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,
    0.5f,       0.5f,       0.5f,       0.5f,       0.45f,      0.35f,      0.5f,       0.5f,       0.5f,       0.5f,       0.5f
);
WPVARM(IDF_HEX, explcol, -3, 0xFFFFFF,
    -1,         -1,         0x4444FF,   0x999900,   0xFF6600,   -1,         0x44DDCC,   0x8822DD,   0x981808,   0x981808,   0x981808,
    -1,         -1,         0x4444FF,   0x999900,   0xFF6600,   -1,         0x44DDCC,   0x8822DD,   0x981808,   0x981808,   0x981808
);
WPFVARM(0, explode, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       10.0f,      10.0f,      24.0f,      75.0f,      24.0f,      100.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       10.0f,      48.0f,      0.0f,       75.0f,      36.0f,      100.0f
);
WPVARM(0, extinguish, 0, 7,
    2,          2,          2,          2,          2,          3,          1,          2,          2,          2,          2,
    2,          2,          2,          2,          2,          3,          0,          2,          2,          2,          2
);
WPVARM(0, flakdmg, VAR_MIN, VAR_MAX,
    25,         10,         15,         16,         16,         12,         10,         18,         150,        100,        300,
    10,         5,          30,         4,          4,          4,          5,          50,         150,        100,        300
);
WPVARM(0, flakffwd, 0, 1,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.5f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.5f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPFVARM(0, flakminspeed, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       25.0f,      25.0f,      0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       25.0f,      25.0f,      25.0f,      0.0f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPFVARM(0, flakoffset, 0, FVAR_MAX,
    8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       0.0f,       0.0f,       0.0f,
    8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       8.0f,       0.0f,       0.0f,       0.0f
);
WPVARM(0, flakrays, 0, VAR_MAX,
    5,          5,          5,          5,          5,          5,          5,          5,          75,         50,         75,
    5,          5,          5,          40,         35,         5,          5,          5,          75,         50,         75
);
WPFVARM(0, flakrel, 0, FVAR_MAX,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,       0.0f,       0.0f,
    1.0f,       1.0f,       1.0f,       1.5f,       0.05f,      1.0f,       1.0f,       1.0f,       0.0f,       0.0f,       0.0f
);
WPFVARM(0, flakscale, FVAR_NONZERO, FVAR_MAX,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.5f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,
    1.0f,       0.5f,       1.0f,       1.0f,       1.0f,       0.5f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f
);
WPFVARM(0, flakskew, 0, FVAR_MAX,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f
);
WPVARM(0, flakspeed, 0, VAR_MAX,
    0,          0,          0,          0,          0,          200,        0,          0,          300,        300,        400,
    0,          0,          0,          0,          0,          250,        0,          0,          300,        300,        400
);
WPFVARM(0, flakspread, 0, FVAR_MAX,
    1.0f,       1.0f,       1.0f,       1.0f,       0.2f,       0.1f,       1.0f,       0.25f,      1.0f,       1.0f,       1.0f,
    1.0f,       0.25f,      1.0f,       0.2f,       1.0f,       0.1f,       1.0f,       0.25f,      1.0f,       1.0f,       1.0f
);
WPVARM(0, flaktime, 1, VAR_MAX,
    500,        500,        500,        250,        500,        1000,       500,        500,        3000,       3000,       3000,
    500,        750,        500,        2000,       800,        3000,       500,        500,        3000,       3000,       3000
);
WPVARM(0, flakweap, -1, WEAP_MAX*2-1,
    -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1,         14,         14,         15,
    -1,         1,          -1,         14,         15,         -1,         -1,         -1,         14,         14,         15
);
WPFVAR(0, frequency, 0, FVAR_MAX,
    0.0f,       0.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       2.0f,       2.0f,       4.0f
);
WPVARM(0, fullauto, 0, 1,
    1,          0,          1,          0,          1,          1,          1,          0,          0,          0,          0,
    1,          0,          1,          0,          1,          0,          0,          0,          0,          0,          0
);
WPVARM(0, gdelay, 0, VAR_MAX,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          100,        0,          0,          0,          0,          0,          0
);
WPVARM(0, guided, 0, 6,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          1
);
WPFVARM(0, headmin, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       4.0f,       8.0f,       4.0f,       8.0f,       8.0f,       16.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       4.0f,       4.0f,       4.0f,       8.0f,       8.0f,       16.0f
);
WPFVARM(0, hitpush, FVAR_MIN, FVAR_MAX,
    100.0f,     35.0f,      50.0f,      50.0f,      50.0f,      5.0f,       20.0f,      50.0f,      250.0f,     250.0f,     500.0f,
    200.0f,     50.0f,      100.0f,     25.0f,      50.0f,      10.0f,      -100.0f,    100.0f,     250.0f,     250.0f,     500.0f
);
WPFVARM(0, kickpush, FVAR_MIN, FVAR_MAX,
    0.0f,       6.0f,       0.0f,       50.0f,      5.0f,       1.0f,       25.0f,      50.0f,      5.0f,       5.0f,       150.0f,
    0.0f,       4.0f,       0.0f,       75.0f,      10.0f,      2.0f,       150.0f,     50.0f,      5.0f,       5.0f,       150.0f
);
WPVAR(0, laser, 0, 1,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0
);
WPFVARM(0, legsdmg, FVAR_MIN, FVAR_MAX,
    0.3f,       0.325f,     0.3f,       0.3f,       0.3f,       0.25f,      0.2f,       0.2f,       0.2f,       0.2f,       0.2f,
    0.6f,       0.325f,     0.3f,       0.3f,       0.3f,       0.25f,      0.2f,       0.2f,       0.2f,       0.2f,       0.2f
);
WPVARM(0, limspeed, 0, VAR_MAX,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          35,         0,          0,          0,          0
);
WPVAR(0, max, 1, VAR_MAX,
    1,          10,         1,          8,          40,         25,         20,         5,          2,          4,          1
);
WPFVARM(0, maxspread, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPFVARM(0, minspeed, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       25.0f,      25.0f,      0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       25.0f,      25.0f,      25.0f,      0.0f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPFVARM(0, minspread, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       16.0f,      0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPVARM(IDF_HEX, partcol, -3, 0xFFFFFF,
    0xEEEE22,   0x666611,   0x4444FF,   0x999900,   0xFF6600,   -1,         0x44DDCC,   0x8822DD,   -1,         -1,         -1,
    0xEEEE22,   0x666611,   0x4444FF,   0x999900,   0xFF6600,   -1,         0x44DDCC,   0x8822DD,   -1,         -1,         -1
);
WPFVARM(0, partlen, 0, FVAR_MAX,
    0.0f,       10.0f,      0.0f,       25.0f,      30.0f,      0.0f,       0.0f,       256.0f,     0.0f,       0.0f,       0.0f,
    0.0f,       10.0f,      0.0f,       15.0f,      15.0f,      5.0f,       0.0f,       512.0f,     0.0f,       0.0f,       0.0f
);
WPFVARM(0, partsize, 0, FVAR_MAX,
    1.0f,       2.0f,       1.0f,       0.65f,      0.5f,       10.0f,      8.0f,       1.5f,       1.0f,       4.0f,       3.0f,
    2.0f,       0.5f,       1.25f,      0.45f,      0.35f,      10.0f,      24.0f,      3.0f,       1.0f,       4.0f,       3.0f
);
WPVARM(0, parttype, 0, WEAP_MAX-1,
    0,          1,          2,          3,          4,          5,          6,          7,          8,          9,          10,
    0,          1,          2,          3,          4,          5,          6,          7,          8,          9,          10
);
WPVARM(0, pdelay, 0, VAR_MAX,
    0,          0,          10,         0,          0,          0,          0,          0,          75,         75,         0,
    0,          0,          10,         0,          0,          25,         75,         0,          75,         75,         0
);
WPVARM(0, power, 0, VAR_MAX,
    0,          0,          0,          0,          0,          0,          0,          0,          3000,       0,          2000,
    0,          0,          0,          0,          0,          500,        2000,       0,          3000,       0,          2000
);
WPFVARM(0, proximity, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       24.0f,      0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       24.0f,      0.0f
);
WPVARM(0, proxtime, 0, VAR_MAX,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          500,        0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          500,        0
);
WPVARM(0, radial, 0, VAR_MAX,
    0,          0,          0,          0,          0,          20,         40,         0,          0,          0,          0,
    0,          0,          0,          0,          0,          20,         80,         0,          0,          0,          0
);
WPFVARM(0, radius, FVAR_NONZERO, FVAR_MAX,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       2.0f,       1.0f,       1.0f,       1.0f,       1.0f,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       2.0f,       1.0f,       1.0f,       1.0f,       1.0f
);
WPVARM(0, rays, 0, VAR_MAX,
    1,          1,          1,          10,         1,          1,          1,          1,          1,          1,          1,
    1,          10,         1,          1,          1,          4,          1,          1,          1,          1,          1
);
WPVAR(0, rdelay, 0, VAR_MAX,
    50,         1000,       50,         750,        1500,       2000,       2000,       1750,       1500,       1500,       1500
);
WPFVARM(0, reflectivity, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPFVARM(0, relativity, 0, FVAR_MAX,
    0.0f,       0.05f,      0.0f,       0.05f,      0.05f,      0.95f,      0.1f,       1.0f,       1.0f,       1.0f,       0.0f,
    0.0f,       0.05f,      0.0f,       0.75f,      0.05f,      0.5f,       0.1f,       0.0f,       1.0f,       1.0f,       0.0f
);
WPVAR(0, reloads, -1, VAR_MAX,
    0,          -1,         0,          -1,         -1,         -1,         -1,         -1,         0,          0,          0
);
WPVARM(0, residual, 0, 2,
    0,          0,          2,          0,          0,          1,          0,          0,          1,          1,          1,
    0,          0,          2,          2,          0,          1,          0,          0,          1,          1,          1
);
WPFVARM(0, selfdmg, FVAR_MIN, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,
    0.0f,       0.0f,       0.0f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       0.5f
);
WPFVARM(0, slow, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.2f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPVARM(0, speed, 0, VAR_MAX,
    0,          3000,       0,          1000,       2500,       300,        1000,       10000,      250,        100,        1000,
    0,          1000,       0,          200,        350,        150,        85,         100000,     250,        100,        250
);
WPFVARM(0, spread, 0, FVAR_MAX,
    1.0f,       1.0f,       1.0f,       6.0f,       8.0f,       5.0f,       2.0f,       2.0f,       1.0f,       1.0f,       1.0f,
    1.0f,       8.0f,       1.0f,       3.0f,       4.0f,       20.0f,      1.0f,       1.0f,       1.0f,       1.0f,       1.0f
);
WPFVARM(0, stunscale, 0, FVAR_MAX,
    0.5f,       0.25f,      1.0f,       1.0f,       2.0f,       0.0f,       0.5f,       0.35f,      2.0f,       2.0f,       4.0f,
    1.0f,       0.25f,      2.0f,       0.5f,       2.0f,       0.0f,       1.0f,       1.0f,       2.0f,       2.0f,       4.0f
);
WPVARM(0, stuntime, 0, VAR_MAX,
    100,        25,         200,        100,        200,        0,          50,         100,        200,        200,        500,
    200,        25,         200,        100,        200,        0,          50,         200,        200,        200,        500
);
WPVARM(0, sub, 0, VAR_MAX,
    0,          1,          0,          1,          1,          1,          1,          1,          1,          1,          1,
    0,          2,          0,          2,          4,          5,          20,         1,          1,          1,          1
);
WPVARM(0, taper, 0, 6,
    0,          0,          0,          5,          5,          1,          1,          5,          0,          0,          0,
    0,          0,          0,          0,          0,          1,          2,          5,          0,          0,          0
);
WPFVARM(0, taperin, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       10.0f,      10.0f,      0.5f,       0.025f,     100.0f,     0.0f,       0.0f,       0.0f,
    0.0f,       10.0f,      0.0f,       0.0f,       0.0f,       0.5f,       0.5f,       100.0f,     0.0f,       0.0f,       0.0f
);
WPFVARM(0, taperout, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       500.0f,     700.0f,     0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.5f,       0.0f,       0.0f,       0.0f,       0.0f
);
WPVARM(0, time, 1, VAR_MAX,
    100,        2000,       350,        400,        500,        200,        600,        5000,       3000,       120000,     5000,
    500,        100,        500,        5000,       2500,       1500,       5000,       5000,       3000,       30000,      5000
);
WPFVARM(0, torsodmg, FVAR_MIN, FVAR_MAX,
    0.5f,       0.65f,      0.65f,      0.6f,       0.6f,       0.45f,      0.4f,       0.4f,       0.4f,       0.4f,       0.4f,
    0.8f,       0.65f,      0.65f,      0.6f,       0.6f,       0.45f,      0.4f,       0.4f,       0.4f,       0.4f,       0.4f
);
WPFVARM(0, trace, 0, FVAR_MAX,
    4.0f,       1.0f,       3.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,
    4.0f,       1.0f,       5.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f
);
WPFVARM(0, visfade, 0, 1,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,
    1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f
);
WPVARM(0, vistime, 0, VAR_MAX,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,          1000,       0
);
WPFVARM(0, waterfric, 0, FVAR_MAX,
    0.0f,       2.0f,       0.0f,       2.0f,       2.0f,       1.0f,       1.0f,       2.0f,       2.0f,       2.0f,       2.0f,
    0.0f,       2.0f,       0.0f,       2.0f,       2.0f,       1.0f,       1.0f,       2.0f,       2.0f,       2.0f,       2.0f
);
WPFVARM(0, wavepush, 0, FVAR_MAX,
    1.5f,       1.5f,       1.5f,       1.5f,       1.5f,       0.0f,       1.5f,       1.5f,       2.0f,       2.0f,       4.0f,
    1.5f,       1.5f,       1.5f,       1.5f,       1.5f,       0.0f,       2.0f,       1.5f,       2.0f,       2.0f,       4.0f
);
WPFVARM(0, weight, 0, FVAR_MAX,
    0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       200.0f,     0.0f,       0.0f,       65.0f,      150.0f,     0.0f,
    0.0f,       0.0f,       0.0f,       250.0f,     0.0f,       100.0f,     0.0f,       0.0f,       65.0f,      150.0f,     0.0f
);
WPFVARM(0, whipdmg, FVAR_MIN, FVAR_MAX,
    0.8f,       0.8f,       0.8f,       0.8f,       0.8f,       0.65f,      0.6f,       0.6f,       0.6f,       0.6f,       0.6f,
    0.8f,       0.8f,       0.8f,       0.8f,       0.8f,       0.65f,      0.6f,       0.6f,       0.6f,       0.6f,       0.6f
);
WPFVARM(0, zdiv, 0, FVAR_MAX,
    1.0f,       2.0f,       1.0f,       1.0f,       1.0f,       0.0f,       2.0f,       1.0f,       0.0f,       0.0f,       0.0f,
    1.0f,       1.0f,       1.0f,       4.0f,       2.0f,       1.0f,       1.0f,       1.0f,       0.0f,       0.0f,       0.0f
);
WPVAR(0, zooms, 0, 1,
    0,          0,          0,          0,          0,          0,          0,          1,          0,          0,          0
);
WPVARM(0, collide, 0, VAR_MAX,
    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|DRILL_PLAYER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER|DRILL_GEOM,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER|DRILL_GEOM,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_TRACE|DRILL_GEOM,
    BOUNCE_GEOM|BOUNCE_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_PROJ,

    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|DRILL_PLAYER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER,
    BOUNCE_GEOM|BOUNCE_PLAYER|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|DRILL_GEOM|DRILL_PLAYER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_PROJ
);
WPVARM(0, flakcollide, 0, VAR_MAX,
    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|DRILL_PLAYER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_PROJ,

    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|DRILL_PLAYER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    BOUNCE_GEOM|BOUNCE_PLAYER|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|DRILL_PLAYER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|STICK_GEOM|STICK_PLAYER|COLLIDE_PROJ,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_PROJ
);

#define WEAPRS(a,b,c)           (a*(m_limited(b, c) ? GAME(radiallimited) : GAME(radialscale)))
#define WEAPS(a,b,c,d,e,f)      (!m_insta(d, e) || m_arena(d, e) || a != WEAP_RIFLE ? WEAPRS(WEAP2(a, b, c)*f, d, e) : 0.f)
#define WEAPSP(a,b,c,d,e,f)     (!m_insta(c, d) || m_arena(c, d) || a != WEAP_RIFLE ? clamp(max(WEAP2(a, spread, b), f*0.5f)*e, WEAP2(a, minspread, b), WEAP2(a, maxspread, b) > 0 ? WEAP2(a, maxspread, b) : FVAR_MAX) : 0.f)
#define WEAPSND(a,b)            (weaptype[a].sound+b)
#define WEAPSNDF(a,b)           (weaptype[a].sound+(b ? S_W_SECONDARY : S_W_PRIMARY))
#define WEAPSND2(a,b,c)         (weaptype[a].sound+(b ? c+1 : c))
#define WEAPUSE(a)              (WEAP(a, reloads) != 0 ? WEAP(a, max) : WEAP(a, add))
#define WEAPHCOL(d,a,b,c)       (WEAP2(a, b, c) >= 0 ? WEAP2(a, b, c) : game::hexpulsecolour(d, clamp(-1-WEAP2(a, b, c), 0, 2), 50))
#define WEAPPCOL(d,a,b,c)       (WEAP2(a, b, c) >= 0 ? vec::hexcolor(WEAP2(a, b, c)) : game::pulsecolour(d, clamp(-1-WEAP2(a, b, c), 0, 2), 50))

struct weaptypes
{
    int     anim,               sound,      espeed;
    bool    melee,      traced,     muzzle,     eject;
    float   thrown[2],              halo,       esize;
    const char *name,   *item,                      *vwep, *hwep,                     *proj,                  *eprj;
};
#ifdef GAMESERVER
weaptypes weaptype[] =
{
    {
            ANIM_MELEE,         S_MELEE,    1,
            true,       true,       false,      false,
            { 0, 0 },               1,          0,
            "melee",    "",                         "", "",                        "",                     ""
    },
    {
            ANIM_PISTOL,        S_PISTOL,   10,
            false,      false,      true,       true,
            { 0, 0 },               8,          0.35f,
            "pistol",   "weapons/pistol/item",      "weapons/pistol/vwep", "weapons/pistol/hwep",     "",                     "projs/cartridge"
    },
    {
            ANIM_SWORD,         S_SWORD,    1,
            true,       true,       true,       false,
            { 0, 0 },               14,         0,
            "sword",    "weapons/sword/item",       "weapons/sword/vwep", "weapons/sword/hwep",    "",                     ""
    },
    {
            ANIM_SHOTGUN,       S_SHOTGUN,  10,
            false,      false,      true,       true,
            { 0, 0 },               12,         0.45f,
            "shotgun",  "weapons/shotgun/item",     "weapons/shotgun/vwep", "weapons/shotgun/hwep",    "",                     "projs/shell"
    },
    {
            ANIM_SMG,           S_SMG,      20,
            false,      false,      true,       true,
            { 0, 0 },               10,         0.35f,
            "smg",      "weapons/smg/item",         "weapons/smg/vwep", "weapons/smg/hwep",        "",                     "projs/cartridge"
    },
    {
            ANIM_FLAMER,        S_FLAMER,   1,
            false,      false,      true,       true,
            { 0, 0 },               12,         0,
            "flamer",   "weapons/flamer/item",      "weapons/flamer/vwep", "weapons/flamer/hwep",     "",                     ""
    },
    {
            ANIM_PLASMA,        S_PLASMA,   1,
            false,      false,      true,       false,
            { 0, 0 },               10,         0,
            "plasma",   "weapons/plasma/item",      "weapons/plasma/vwep", "weapons/plasma/hwep",     "",                     ""
    },
    {
            ANIM_RIFLE,         S_RIFLE,    1,
            false,      false,      true,       false,
            { 0, 0 },               12,         0,
            "rifle",    "weapons/rifle/item",       "weapons/rifle/vwep", "weapons/rifle/hwep",      "",                     ""
    },
    {
            ANIM_GRENADE,       S_GRENADE,  1,
            false,      false,      false,      false,
            { 0.0625f, 0.0625f },   6,          0,
            "grenade",  "weapons/grenade/item",     "weapons/grenade/vwep", "weapons/grenade/hwep",    "weapons/grenade/proj", ""
    },
    {
            ANIM_MINE,          S_MINE,     1,
            false,      false,      false,      false,
            { 0.0625f, 0.0625f },   6,          0,
            "mine",     "weapons/mine/item",        "weapons/mine/vwep",    "weapons/mine/hwep",        "weapons/mine/proj", ""
    },
    {
            ANIM_ROCKET,        S_ROCKET,   1,
            false,      false,      true,      false,
            { 0, 0 },               10,          0,
            "rocket",   "weapons/rocket/item",       "weapons/rocket/vwep", "weapons/rocket/hwep",     "weapons/rocket/proj",  ""
    }
};
SVAR(0, weapname, "melee pistol sword shotgun smg flamer plasma rifle grenade mine rocket");
VAR(0, weapidxmelee, 1, WEAP_MELEE, -1);
VAR(0, weapidxpistol, 1, WEAP_PISTOL, -1);
VAR(0, weapidxsword, 1, WEAP_SWORD, -1);
VAR(0, weapidxshotgun, 1, WEAP_SHOTGUN, -1);
VAR(0, weapidxsmg, 1, WEAP_SMG, -1);
VAR(0, weapidxflamer, 1, WEAP_FLAMER, -1);
VAR(0, weapidxplasma, 1, WEAP_PLASMA, -1);
VAR(0, weapidxrifle, 1, WEAP_RIFLE, -1);
VAR(0, weapidxgrenade, 1, WEAP_GRENADE, -1);
VAR(0, weapidxmine, 1, WEAP_MINE, -1);
VAR(0, weapidxrocket, 1, WEAP_ROCKET, -1);
VAR(0, weapidxoffset, 1, WEAP_OFFSET, -1);
VAR(0, weapidxitem, 1, WEAP_ITEM, -1);
VAR(0, weapidxloadout, 1, WEAP_LOADOUT, -1);
VAR(0, weapidxnum, 1, WEAP_MAX, -1);
#else
extern weaptypes weaptype[];
#endif
