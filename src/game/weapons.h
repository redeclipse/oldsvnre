enum
{
    WEAP_MELEE = 0, WEAP_PISTOL, WEAP_OFFSET, // end of unselectable weapon set
    WEAP_SWORD = WEAP_OFFSET, WEAP_SHOTGUN, WEAP_SMG, WEAP_FLAMER, WEAP_PLASMA, WEAP_RIFLE, WEAP_ITEM,
    WEAP_GRENADE = WEAP_ITEM, WEAP_ROCKET, // end of item weapon set
    WEAP_MAX,
    WEAP_CARRY = WEAP_ITEM-WEAP_OFFSET-1
};
#define isweap(a)       (a >= 0 && a < WEAP_MAX)

enum { WEAP_F_NONE = 0, WEAP_F_FORCED = 1<<0 };
enum { WEAP_S_IDLE = 0, WEAP_S_PRIMARY, WEAP_S_SECONDARY, WEAP_S_RELOAD, WEAP_S_POWER, WEAP_S_SWITCH, WEAP_S_USE, WEAP_S_WAIT };

enum
{
    S_W_PRIMARY = 0, S_W_PRIMARY2, S_W_PRIMARY3,
    S_W_SECONDARY, S_W_SECONDARY2, S_W_SECONDARY3,
    S_W_POWER, S_W_POWER2,
    S_W_SWITCH, S_W_RELOAD, S_W_NOTIFY,
    S_W_EXPLODE, S_W_EXPLODE2,
    S_W_DESTROY, S_W_DESTROY2,
    S_W_EXTINGUISH, S_W_EXTINGUISH2,
    S_W_TRANSIT, S_W_TRANSIT2,
    S_W_BOUNCE, S_W_BOUNCE2,
    S_W_OFFSET, S_W_USE = S_W_OFFSET, S_W_SPAWN,
    S_W_MAX,
    S_W_SHOOT = 3
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
    S_ROCKET    = S_GRENADE+S_W_MAX,
    S_MAX
};

enum
{
    IMPACT_GEOM = 1<<0, BOUNCE_GEOM = 1<<1, IMPACT_PLAYER = 1<<2, BOUNCE_PLAYER = 1<<3, IMPACT_SHOTS = 1<<4,
    COLLIDE_RADIAL = 1<<5, COLLIDE_TRACE = 1<<6, COLLIDE_OWNER = 1<<7, COLLIDE_CONT = 1<<8, COLLIDE_STICK = 1<<9, COLLIDE_SHOTS = 1<<10,
    COLLIDE_GEOM = IMPACT_GEOM|BOUNCE_GEOM,
    COLLIDE_PLAYER = IMPACT_PLAYER|BOUNCE_PLAYER,
    HIT_PLAYER = IMPACT_PLAYER|BOUNCE_PLAYER|COLLIDE_RADIAL
};

enum
{
    HIT_NONE = 0, HIT_ALT = 1<<0, HIT_LEGS = 1<<1, HIT_TORSO = 1<<2, HIT_HEAD = 1<<3,
    HIT_WAVE = 1<<4, HIT_PROJ = 1<<5, HIT_EXPLODE = 1<<6, HIT_BURN = 1<<7, HIT_BLEED = 1<<8,
    HIT_MELT = 1<<9, HIT_DEATH = 1<<10, HIT_WATER = 1<<11, HIT_SPAWN = 1<<12,
    HIT_LOST = 1<<13, HIT_KILL = 1<<14, HIT_CRIT = 1<<15, HIT_FLAK = 1<<16,
    HIT_CLEAR = HIT_PROJ|HIT_EXPLODE|HIT_BURN|HIT_BLEED|HIT_MELT|HIT_DEATH|HIT_WATER|HIT_SPAWN|HIT_LOST,
    HIT_SFLAGS = HIT_KILL|HIT_CRIT
};

struct shotmsg { int id; ivec pos; };
struct hitmsg { int flags, proj, target, dist; ivec dir; };

#define hithurts(x)     (x&HIT_BURN || x&HIT_BLEED || x&HIT_EXPLODE || x&HIT_PROJ || x&HIT_MELT || x&HIT_DEATH || x&HIT_WATER)
#define doesburn(x,y)   (isweap(x) && WEAP2(x, residual, y&HIT_ALT) == 1)
#define doesbleed(x,y)  (isweap(x) && WEAP2(x, residual, y&HIT_ALT) == 2)
#define WALT(x)         (WEAP_MAX+WEAP_##x)

#define WEAPON(name, \
    w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, wa, wb1, wb2, wc, wd, we1, we2, we3, we4, we5, we6, \
    wf, wg, wh, wi, wj, wk, mwj, mwk, xwj, xwk, wl, wm, wn1, wn2, wo1, wo2, wo3, wo4, wo5, wo6, wo7, wo8, wo9, wo10, wp, wq, \
    x21, x22, x31, x32, x33, x34, x4, x5, x6, x7, x81, x82, x9, xa, xb, xc, xd, xe, xf, \
    t0, t1, t2, t3, y0, y1, y2, y3, y4, y5, y6, y7, y8, y9, ya, yb, yc, yd, ye1, ye2, yf1, yf2, yg, yh, \
    yi, yj, yk, yl, ym, yn, yo, yp, yq, yr, ys, yt, yw, yx, yy, yz, \
    ya3, ya4, ya5, ya6, ya7, ya8, ya9, ya10, ya11, ya12, ya13, ya14 \
 ) \
    GVAR(0, name##add, 1, w0, 10000);                   GVAR(0, name##max, 1, w1, 10000); \
    GVAR(0, name##sub1, 0, w2, 10000);                  GVAR(0, name##sub2, 0, w3, 10000); \
    GVAR(0, name##adelay1, 20, w4, 10000);              GVAR(0, name##adelay2, 20, w5, 10000);          GVAR(0, name##rdelay, 50, w6, 10000); \
    GVAR(0, name##damage1, -10000, w7, 10000);          GVAR(0, name##damage2, -10000, w8, 10000); \
    GVAR(0, name##speed1, -100000, w9, 100000);         GVAR(0, name##speed2, -100000, wa, 100000); \
    GVAR(0, name##power1, 0, wb1, 10000);               GVAR(0, name##power2, 0, wb2, 10000); \
    GVAR(0, name##time1, 0, wc, 100000);                GVAR(0, name##time2, 0, wd, 100000); \
    GVAR(0, name##pdelay1, 0, we1, 10000);              GVAR(0, name##pdelay2, 0, we2, 10000); \
    GVAR(0, name##gdelay1, 0, we3, 10000);              GVAR(0, name##gdelay2, 0, we4, 10000); \
    GVAR(0, name##edelay1, 0, we5, 10000);              GVAR(0, name##edelay2, 0, we6, 10000); \
    GVAR(0, name##explode1, 0, wf, 10000);              GVAR(0, name##explode2, 0, wg, 10000); \
    GVAR(0, name##rays1, 1, wh, 1000);                  GVAR(0, name##rays2, 1, wi, 1000); \
    GVAR(0, name##spread1, 0, wj, 10000);               GVAR(0, name##spread2, 0, wk, 10000); \
    GVAR(0, name##minspread1, 0, mwj, 10000);           GVAR(0, name##minspread2, 0, mwk, 10000); \
    GVAR(0, name##maxspread1, 0, xwj, 10000);           GVAR(0, name##maxspread2, 0, xwk, 10000); \
    GVAR(0, name##zdiv1, 0, wl, 10000);                 GVAR(0, name##zdiv2, 0, wm, 10000); \
    GVAR(0, name##aiskew1, 0, wn1, 10000);              GVAR(0, name##aiskew2, 0, wn2, 10000); \
    GVAR(0, name##flakweap1, -1, wo1, WEAP_MAX*2-1);    GVAR(0, name##flakweap2, -1, wo2, WEAP_MAX*2-1); \
    GVAR(0, name##flakdmg1, -10000, wo3, 10000);        GVAR(0, name##flakdmg2, -10000, wo4, 10000); \
    GVAR(0, name##flakrays1, 1, wo5, 10000);            GVAR(0, name##flakrays2, 1, wo6, 10000); \
    GVAR(0, name##flaktime1, 1, wo7, 10000);            GVAR(0, name##flaktime2, 1, wo8, 10000); \
    GVAR(0, name##flakspeed1, 0, wo9, 10000);           GVAR(0, name##flakspeed2, 0, wo10, 10000); \
    GVAR(0, name##collide1, 0, wp, INT_MAX-1);          GVAR(0, name##collide2, 0, wq, INT_MAX-1); \
    GVAR(0, name##extinguish1, 0, x21, 3);              GVAR(0, name##extinguish2, 0, x22, 3); \
    GVAR(0, name##cooked1, 0, x31, 5);                  GVAR(0, name##cooked2, 0, x32, 5); \
    GVAR(0, name##guided1, 0, x33, 5);                  GVAR(0, name##guided2, 0, x34, 5); \
    GVAR(0, name##radial1, 0, x4, 1);                   GVAR(0, name##radial2, 0, x5, 1); \
    GVAR(0, name##residual1, 0, x6, 2);                 GVAR(0, name##residual2, 0, x7, 2); \
    GVAR(0, name##reloads, 0, x81, 1);                  GVAR(0, name##carried, 0, x82, 1);              GVAR(0, name##zooms, 0, x9, 1); \
    GVAR(0, name##fullauto1, 0, xa, 1);                 GVAR(0, name##fullauto2, 0, xb, 1);             GVAR(0, name##allowed, 0, xc, 3); \
    GVAR(0, name##laser, 0, xd, 1);                     GVAR(0, name##critdash1, 0, xe, 10000);         GVAR(0, name##critdash2, 0, xf, 10000); \
    GFVAR(0, name##taper1, 0, t0, 1);                   GFVAR(0, name##taper2, 0, t1, 1); \
    GFVAR(0, name##taperspan1, 0, t2, 1);               GFVAR(0, name##taperspan2, 0, t3, 1); \
    GFVAR(0, name##elasticity1, -10000, y0, 10000);     GFVAR(0, name##elasticity2, -10000, y1, 10000); \
    GFVAR(0, name##reflectivity1, 0, y2, 360);          GFVAR(0, name##reflectivity2, 0, y3, 360); \
    GFVAR(0, name##relativity1, -10000, y4, 10000);     GFVAR(0, name##relativity2, -10000, y5, 10000); \
    GFVAR(0, name##waterfric1, 0, y6, 10000);           GFVAR(0, name##waterfric2, 0, y7, 10000); \
    GFVAR(0, name##weight1, -100000, y8, 100000);       GFVAR(0, name##weight2, -100000, y9, 100000); \
    GFVAR(0, name##radius1, 1, ya, 10000);              GFVAR(0, name##radius2, 1, yb, 10000); \
    GFVAR(0, name##kickpush1, -10000, yc, 10000);       GFVAR(0, name##kickpush2, -10000, yd, 10000); \
    GFVAR(0, name##hitpush1, -10000, ye1, 10000);       GFVAR(0, name##hitpush2, -10000, ye2, 10000); \
    GFVAR(0, name##slow1, 0, yf1, 10000);               GFVAR(0, name##slow2, 0, yf2, 10000); \
    GFVAR(0, name##aidist1, 0, yg, 100000);             GFVAR(0, name##aidist2, 0, yh, 10000); \
    GFVAR(0, name##partsize1, 0, yi, 100000);           GFVAR(0, name##partsize2, 0, yj, 100000); \
    GFVAR(0, name##partlen1, 0, yk, 100000);            GFVAR(0, name##partlen2, 0, yl, 100000); \
    GFVAR(0, name##frequency, 0, ym, 10000);            GFVAR(0, name##pusharea, 0, yn, 10000); \
    GFVAR(0, name##critmult, 0, yo, 10000);             GFVAR(0, name##critdist, 0, yp, 10000); \
    GFVAR(0, name##delta1, 1, yq, 10000);               GFVAR(0, name##delta2, 1, yr, 10000); \
    GFVAR(0, name##tracemult1, 1e-4f, ys, 10000);       GFVAR(0, name##tracemult2, 1e-4f, yt, 10000); \
    GFVAR(0, name##torsodmg1, 1e-4f, yw, 1000);         GFVAR(0, name##torsodmg2, 1e-4f, yx, 1000); \
    GFVAR(0, name##legsdmg1, 1e-4f, yy, 1000);          GFVAR(0, name##legsdmg2, 1e-4f, yz, 1000); \
    GFVAR(0, name##flakscale1, 0, ya3, 1000);           GFVAR(0, name##flakscale2, 0, ya4, 1000); \
    GFVAR(0, name##flakspread1, 0, ya5, 1000);          GFVAR(0, name##flakspread2, 0, ya6, 1000); \
    GFVAR(0, name##flakrel1, 0, ya7, 1000);             GFVAR(0, name##flakrel2, 0, ya8, 1000); \
    GFVAR(0, name##flakffwd1, 0, ya9, 1);               GFVAR(0, name##flakffwd2, 0, ya10, 1); \
    GFVAR(0, name##flakoffset1, 0, ya11, 1000);         GFVAR(0, name##flakoffset2, 0, ya12, 1000); \
    GFVAR(0, name##flakskew1, 0, ya13, 1000);           GFVAR(0, name##flakskew2, 0, ya14, 1000);

//  add     max     sub1    sub2    adly1   adly2   rdly    dam1    dam2    spd1    spd2    pow1    pow2    time1   time2
//  pdly1   pdly2   gdly1   gdly2   edly1   edly2   expl1   expl2   rays1   rays2   sprd1   sprd2   msprd1  msprd2  xsprd1  xsprd2
//  zdiv1   zdiv2   aiskew1 aiskew2 fweap1          fweap2          fdam1   fdam2   frays1  frays2  ftime1  ftime2  fspd1   fspd2
//  collide1
//  collide2
//  ext1    ext2    cook1   cook2   guide1  guide2  radl1   radl2   resid1  resid2  rlds    crd     zooms   fa1     fa2
//  allw    laser   cdash1  cdash2
//  tpr1    tpr2    tspan1  tspan2  elas1   elas2   rflt1   rflt2   relt1   relt2   wfrc1   wfrc2   wght1   wght2   rads1   rads2
//  kpsh1   kpsh2   hpsh1   hpsh2   slow1   slow2   aidst1  aidst2  psz1    psz2    plen1   plen2   freq    push
//  cmult   cdist   dlta1   dlta2   tmult1  tmult2  tordm1  tordm2  legdm1  legdm2
//  fscale1 fscale2 fsprd1  fsprd2  frel1   frel2   fffwd1  fffwd2  foff1   foff2   fskew1  fskew2
WEAPON(melee,
    1,      1,      0,      0,      500,    750,    50,     20,     40,     0,      0,      0,      0,      100,    80,
    20,     0,      0,      0,      200,    200,    0,      0,      1,      1,      1,      1,      0,      0,      0,      0,
    1,      1,      1,      1,      -1,             -1,             25,     10,     5,      5,      500,    500,    0,      0,
    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_PLAYER|COLLIDE_TRACE,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      1,      1,
    2,      0,      500,    500,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0,      0,      0,      0,      0,      0,      1,      1,
    -1,     -1,     200,    200,    0.5f,   0.75f,  24,     24,     1,      2,      0,      0,      0,      1,
    2,      0,      10,     10,     2,      4,      0.5f,   0.5f,   0.3f,   0.3f,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1
);
WEAPON(pistol,
    10,     10,     1,      1,      75,     150,    1000,   20,     20,     2000,   2000,   0,      0,      2000,   2000,
    0,      0,      0,      0,      200,    200,    0,      0,      1,      1,      1,      2,      0,      0,      0,      0,
    2,      0,      100,    100,    -1,             -1,             10,     12,     5,      5,      500,    500,    0,      0,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      1,      0,      0,      0,      1,
    2,      0,      0,      0,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0.05f,  0.05f,  2,      2,      0,      0,      1,      1,
    5,      3,      35,     50,     0.25f,  0.15f,  256,    256,    1,      1,      10,     10,     1,      1,
    4,      16,     10,     10,     1,      1,      0.65f,  0.65f,  0.325f, 0.325f,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1
);
WEAPON(sword,
    1,      1,      0,      0,      500,    750,    50,     30,     60,     0,      0,      0,      0,      300,    300,
    100,    100,    0,      0,      200,    200,    0,      0,      1,      1,      1,      1,      0,      0,      0,      0,
    1,      1,      1,      1,      -1,             -1,             15,     30,     5,      5,      500,    500,    0,      0,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|COLLIDE_CONT,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|COLLIDE_CONT,
    2,      2,      0,      0,      0,      0,      0,      0,      2,      2,      0,      1,      0,      1,      1,
    2,      0,      500,    500,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0,      0,      0,      0,      0,      0,      1,      1,
    0,      0,      250,    500,    0.75f,  1,      32,     32,     1,      1.25f,  0,      0,      1,      1,
    2,      0,      10,     10,     5,      3,      0.6f,   0.6f,   0.3f,   0.3f,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1
);
WEAPON(shotgun,
    2,      8,      1,      2,      600,    900,    750,    14,     6,      1000,   250,    0,      0,      400,    5000,
    0,      0,      0,      0,      200,    200,    0,      0,      10,     1,      4,      2,      16,     0,      0,      0,
    1,      4,      10,     10,     -1,             WALT(SHOTGUN),  16,     4,      5,      40,     250,    2000,   0,      0,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      2,      1,      1,      0,      0,      0,
    2,      0,      0,      0,
    0,      0,      0,      0,      0.5f,   0.5f,   50,     0,      0.05f,  0.75f,  2,      2,      0,      275,    1,      1,
    30,     60,     150,    75,     0.8f,   0.4f,   128,    256,    1,      0.75f,  35,     15,     1,      1.5f,
    2,      6,      10,     10,     1,      1,      0.65f,  0.65f,  0.325f, 0.325f,
    1,      1,      1,      0.2f,   1,      1.5f,   0,      0,      8,      8,      1,      1
);
WEAPON(smg,
    40,     40,     1,      2,      100,    200,    1500,   12,     6,      2000,   350,    0,      0,      800,    100,
    0,      0,      0,      100,    200,    200,    0,      0,      1,      1,      3,      6,      0,      0,      0,      0,
    2,      2,      20,     20,     -1,             WALT(SMG),      16,     4,      5,      20,     500,    500,    0,      0,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      0,      1,      1,
    2,      0,      0,      0,
    0,      0,      0,      0,      0.65f,  0.45f,  30,     30,     0.05f,  0.05f,  2,      2,      0,      0,      1,      1,
    5,      10,     100,    50,     1,      0.75f,  384,    96,     0.5f,   0.6f,   35,     25,     1,      1.5f,
    3,      12,     10,     1000,   1,      1,      0.7f,   0.65f,  0.35f,  0.325f,
    1,      0.5f,   0.2f,   0.1f,   1,      1,      0,      0,      8,      8,      1,      1
);
WEAPON(flamer,
    50,     50,     1,      10,     100,    750,    2000,   8,      4,      250,    150,    0,      750,    300,    250,
    0,      50,     0,      0,      200,    200,    16,     24,     1,      1,      5,      5,      0,      0,      0,      0,
    0,      0,      10,     10,     -1,             WALT(FLAMER),   12,     4,      5,      5,      1000,   5000,   200,    250,
    BOUNCE_GEOM|IMPACT_PLAYER,
    IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    3,      3,      0,      1,      0,      0,      1,      1,      1,      1,      1,      1,      0,      1,      0,
    2,      0,      0,      0,
    0,      0,      0,      0,      0.25f,  0.25f,  45,     0,      0.95f,  0.5f,   1,      1,      -300,   50,     1,      1,
    0.25f,  1,      25,     50,     0,      0,      64,     128,    16,     24,     0,      5,      2,      1.5f,
    5,      12,     10,     10,     1,      1,      0.75f,  0.75f,  0.5f,   0.5f,
    0.5f,   0.5f,   0.125f, 0.125f, 1,      1,      0.5f,   0.5f,   8,      8,      1,      1
);
WEAPON(plasma,
    20,     20,     1,      20,     300,    1000,   2000,   16,     10,     500,    40,     0,      2000,   750,    5000,
    0,      100,    0,      0,      200,    200,    20,     52,     1,      1,      2,      1,      0,      0,      0,      0,
    2,      1,      50,     10,     -1,             -1,             10,     5,      5,      5,      500,    500,    0,      0,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_SHOTS|COLLIDE_RADIAL|COLLIDE_OWNER|COLLIDE_STICK,
    1,      0,      0,      1,      0,      0,      1,      1,      0,      0,      1,      1,      0,      1,      0,
    2,      0,      0,      0,
    0.25f,  0.75f,  0.06f,  0.25f,  0.5f,   0.5f,   0,      0,      0.125f, 0.175f, 1,      1,      0,      0,      4,      2,
    10,     50,     75,     -200,   0.5f,   1,      192,    48,     20,     52,     0,      0,      2,      1.5f,
    3,      12,     10,     10,     1,      1,      0.75f,  0.75f,  0.5f,   0.5f,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1
);
WEAPON(rifle,
    5,      5,      1,      1,      750,    1000,   1750,   60,     100,    7500,   30000,  0,      0,      5000,   5000,
    0,      0,      0,      0,      200,    200,    32,     0,      1,      1,      2,      0,      0,      0,      0,      0,
    0,      0,      40,     40,     -1,             -1,             18,     50,     5,      5,      500,    500,    0,      0,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_CONT,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      1,      0,      0,
    2,      0,      0,      0,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      1,      0,      2,      2,      0,      0,      1,      1,
    20,     10,     250,    150,    0.5f,   0.5f,   512,    1024,   2,      2,      256,    512,    1,      1.5f,
    2,      12,     10,     10,     1,      1,      0.75f,  0.6f ,  0.5f,   0.4f,
    1,      1,      0.25f,  0.25f,  1,      1,      0,      0,      8,      8,      1,      1
);
WEAPON(grenade,
    1,      2,      1,      1,      1000,   1000,   1500,   150,    150,    250,    250,    3000,   3000,   3000,   3000,
    200,    200,    0,      0,      200,    200,    56,     56,     1,      1,      1,      1,      0,      0,      0,      0,
    0,      0,      5,      5,      WEAP_SHOTGUN,   WEAP_SHOTGUN,   150,    150,    50,     50,     3000,   3000,    250,    250,
    BOUNCE_GEOM|BOUNCE_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    IMPACT_GEOM|BOUNCE_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_STICK|COLLIDE_SHOTS,
    2,      2,      2,      2,      0,      0,      0,      0,      1,      1,      0,      0,      0,      0,      0,
    3,      0,      0,      0,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      1,      1,      2,      2,      65,     65,     1,      1,
    5,      5,      500,    500,    1,      1,      384,    384,    2,      2,      0,      0,      2,      2,
    2,      0,      10,     10,     1,      1,      0.75f,  0.75f,  0.5f,   0.5f,
    1,      1,      1,      1,      0,      0,      0,      0,      0,      0,      1,      1
);
WEAPON(rocket,
    1,      1,      1,      1,      1000,   1000,   1500,   300,     300,   1000,   250,    2500,   2500,   5000,   5000,
    0,      0,      0,      0,      200,    200,    80,     80,      1,     1,      1,      1,      0,      0,      0,      0,
    0,      0,      10,     10,     WEAP_SMG,       WEAP_SMG,        300,   300,    75,     75,     2500,   2500,   350,    350,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    2,      2,      2,      2,      0,      1,      0,      0,       1,     1,      0,      1,      0,      0,      0,
    1,      0,      0,      0,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0,      0,      2,      2,      0,      0,      1,      1,
    150,    150,    750,    500,    1,      1,      512,    512,    3,      3,      0,      0,      3,      3,
    2,      0,      10,     10,     1,      1,      0.9f,   0.9f,   0.6f,   0.6f,
    1,      1,      1,      1,      0,      0,      0,      0,      0,      0,      1,      1
);

struct weaptypes
{
    int     anim,               colour,         sound,      espeed;
    bool    melee,      traced,     muzzle,     eject;
    float   thrown[2],              halo,       esize;
    const char *name,       *text,  *item,                      *vwep, *hwep,                     *proj,                  *eprj;
};
#ifdef GAMESERVER
weaptypes weaptype[] =
{
    {
            ANIM_MELEE,         0x888888,       S_MELEE,    1,
            true,       true,       false,      false,
            { 0, 0 },               1,          0,
            "melee",        "\fa",  "",                         "", "",                        "",                     ""
    },
    {
            ANIM_PISTOL,        0xDDDDDD,       S_PISTOL,   10,
            false,      false,      true,       true,
            { 0, 0 },               4,          0.35f,
            "pistol",       "\fw",  "weapons/pistol/item",      "weapons/pistol/vwep", "weapons/pistol/hwep",     "",                     "projs/cartridge"
    },
    {
            ANIM_SWORD,         0x2222FF,      S_SWORD,    1,
            true,       true,       true,       false,
            { 0, 0 },               14,         0,
            "sword",     "\fb",     "weapons/sword/item",       "weapons/sword/vwep", "weapons/sword/hwep",    "",                     ""
    },
    {
            ANIM_SHOTGUN,       0xFFFF22,       S_SHOTGUN,  10,
            false,      false,      true,       true,
            { 0, 0 },               8,          0.45f,
            "shotgun",      "\fy",  "weapons/shotgun/item",     "weapons/shotgun/vwep", "weapons/shotgun/hwep",    "",                     "projs/shell"
    },
    {
            ANIM_SMG,           0xFF8822,       S_SMG,      20,
            false,      false,      true,       true,
            { 0, 0 },               6,          0.35f,
            "smg",          "\fo",  "weapons/smg/item",         "weapons/smg/vwep", "weapons/smg/hwep",        "",                     "projs/cartridge"
    },
    {
            ANIM_FLAMER,        0xFF2222,       S_FLAMER,   1,
            false,      false,      true,       false,
            { 0, 0 },               6,          0,
            "flamer",       "\fr",  "weapons/flamer/item",      "weapons/flamer/vwep", "weapons/flamer/hwep",     "",                     ""
    },
    {
            ANIM_PLASMA,        0x22FFFF,       S_PLASMA,   1,
            false,      false,      true,       false,
            { 0, 0 },               6,          0,
            "plasma",       "\fc",  "weapons/plasma/item",      "weapons/plasma/vwep", "weapons/plasma/hwep",     "",                     ""
    },
    {
            ANIM_RIFLE,         0xAA66FF,       S_RIFLE,    1,
            false,      false,      true,       false,
            { 0, 0 },               8,          0,
            "rifle",        "\fv",  "weapons/rifle/item",       "weapons/rifle/vwep", "weapons/rifle/hwep",      "",                     ""
    },
    {
            ANIM_GRENADE,       0x22FF22,       S_GRENADE,  1,
            false,      false,      false,      false,
            { 0.0625f, 0.0625f },   4,          0,
            "grenade",      "\fg",  "weapons/grenade/item",     "weapons/grenade/vwep", "weapons/grenade/hwep",    "weapons/grenade/proj", ""
    },
    {
            ANIM_ROCKET,        0x993311,       S_ROCKET,   1,
            false,      false,      true,      false,
            { 0, 0 },               8,          0,
            "rocket",      "\fn",  "weapons/rocket/item",       "weapons/rocket/vwep", "weapons/rocket/hwep",     "weapons/rocket/proj",  ""
    }
};
#define WEAPDEF(proto,name)     proto *sv_weap_stat_##name[] = {&sv_melee##name, &sv_pistol##name, &sv_sword##name, &sv_shotgun##name, &sv_smg##name, &sv_flamer##name, &sv_plasma##name, &sv_rifle##name, &sv_grenade##name, &sv_rocket##name };
#define WEAPDEF2(proto,name)    proto *sv_weap_stat_##name[][2] = {{&sv_melee##name##1,&sv_melee##name##2}, {&sv_pistol##name##1,&sv_pistol##name##2}, {&sv_sword##name##1,&sv_sword##name##2}, {&sv_shotgun##name##1,&sv_shotgun##name##2}, {&sv_smg##name##1,&sv_smg##name##2}, {&sv_flamer##name##1,&sv_flamer##name##2}, {&sv_plasma##name##1,&sv_plasma##name##2}, {&sv_rifle##name##1,&sv_rifle##name##2}, {&sv_grenade##name##1,&sv_grenade##name##2}, {&sv_rocket##name##1,&sv_rocket##name##2} };
#define WEAP(weap,name)         (*sv_weap_stat_##name[weap])
#define WEAP2(weap,name,second) (*sv_weap_stat_##name[weap][second?1:0])
#define WEAPSTR(a,weap,attr)    defformatstring(a)("sv_%s%s", weaptype[weap].name, #attr)
#else
extern weaptypes weaptype[];
#ifdef GAMEWORLD
#define WEAPDEF(proto,name)     proto *weap_stat_##name[] = {&melee##name, &pistol##name, &sword##name, &shotgun##name, &smg##name, &flamer##name, &plasma##name, &rifle##name, &grenade##name, &rocket##name };
#define WEAPDEF2(proto,name)    proto *weap_stat_##name[][2] = {{&melee##name##1,&melee##name##2}, {&pistol##name##1,&pistol##name##2}, {&sword##name##1,&sword##name##2}, {&shotgun##name##1,&shotgun##name##2}, {&smg##name##1,&smg##name##2}, {&flamer##name##1,&flamer##name##2}, {&plasma##name##1,&plasma##name##2}, {&rifle##name##1,&rifle##name##2}, {&grenade##name##1,&grenade##name##2}, {&rocket##name##1,&rocket##name##2} };
#else
#define WEAPDEF(proto,name)     extern proto *weap_stat_##name[];
#define WEAPDEF2(proto,name)    extern proto *weap_stat_##name[][2];
#endif
#define WEAP(weap,name)         (*weap_stat_##name[weap])
#define WEAP2(weap,name,second) (*weap_stat_##name[weap][second?1:0])
#define WEAPSTR(a,weap,attr)    defformatstring(a)("%s%s", weaptype[weap].name, #attr)
#endif
#define WEAPLM(a,b,c)           (a*(m_limited(b, c) || m_arena(b, c) ? GAME(limitedscale) : GAME(normalscale)))
#define WEAPEX(a,b,c,d,e)       (!m_insta(c, d) || m_arena(c, d) || a != WEAP_RIFLE ? int(ceilf(WEAPLM(WEAP2(a, explode, b)*e, c, d))) : 0)
#define WEAPSP(a,b,c,d,e,f)     (!m_insta(c, d) || m_arena(c, d) || a != WEAP_RIFLE ? clamp(int(ceilf(max(WEAP2(a, spread, b), f)*e)), WEAP2(a, minspread, b), WEAP2(a, maxspread, b) ? WEAP2(a, maxspread, b) : INT_MAX) : 0)
#define WEAPSND(a,b)            (weaptype[a].sound+b)
#define WEAPSNDF(a,b)           (weaptype[a].sound+(b ? S_W_SECONDARY : S_W_PRIMARY))
#define WEAPSND2(a,b,c)         (weaptype[a].sound+(b ? c+1 : c))
#define WEAPUSE(a)              (WEAP(a, reloads) ? WEAP(a, max) : WEAP(a, add))

WEAPDEF(int, add);
WEAPDEF(int, max);
WEAPDEF2(int, sub);
WEAPDEF2(int, adelay);
WEAPDEF(int, rdelay);
WEAPDEF2(int, damage);
WEAPDEF2(int, speed);
WEAPDEF2(int, power);
WEAPDEF2(int, time);
WEAPDEF2(int, pdelay);
WEAPDEF2(int, gdelay);
WEAPDEF2(int, edelay);
WEAPDEF2(int, explode);
WEAPDEF2(int, rays);
WEAPDEF2(int, spread);
WEAPDEF2(int, minspread);
WEAPDEF2(int, maxspread);
WEAPDEF2(int, zdiv);
WEAPDEF2(int, aiskew);
WEAPDEF2(int, flakweap);
WEAPDEF2(int, flakdmg);
WEAPDEF2(int, flakrays);
WEAPDEF2(int, flaktime);
WEAPDEF2(int, flakspeed);
WEAPDEF2(int, collide);
WEAPDEF2(int, extinguish);
WEAPDEF2(int, cooked);
WEAPDEF2(int, guided);
WEAPDEF2(int, radial);
WEAPDEF2(int, residual);
WEAPDEF(int, reloads);
WEAPDEF(int, carried);
WEAPDEF(int, zooms);
WEAPDEF2(int, fullauto);
WEAPDEF(int, allowed);
WEAPDEF(int, laser);
WEAPDEF2(int, critdash);
WEAPDEF2(float, taper);
WEAPDEF2(float, taperspan);
WEAPDEF2(float, elasticity);
WEAPDEF2(float, reflectivity);
WEAPDEF2(float, relativity);
WEAPDEF2(float, waterfric);
WEAPDEF2(float, weight);
WEAPDEF2(float, radius);
WEAPDEF2(float, kickpush);
WEAPDEF2(float, hitpush);
WEAPDEF2(float, slow);
WEAPDEF2(float, aidist);
WEAPDEF2(float, partsize);
WEAPDEF2(float, partlen);
WEAPDEF(float, frequency);
WEAPDEF(float, pusharea);
WEAPDEF(float, critmult);
WEAPDEF(float, critdist);
WEAPDEF2(float, delta);
WEAPDEF2(float, tracemult);
WEAPDEF2(float, torsodmg);
WEAPDEF2(float, legsdmg);
WEAPDEF2(float, flakscale);
WEAPDEF2(float, flakspread);
WEAPDEF2(float, flakrel);
WEAPDEF2(float, flakffwd);
WEAPDEF2(float, flakoffset);
WEAPDEF2(float, flakskew);
