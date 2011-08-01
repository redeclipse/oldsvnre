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
enum { WEAP_S_IDLE = 0, WEAP_S_PRIMARY, WEAP_S_SECONDARY, WEAP_S_RELOAD, WEAP_S_POWER, WEAP_S_SWITCH, WEAP_S_USE, WEAP_S_JAM, WEAP_S_WAIT };

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

enum
{
    TRAIT_HOVER = 0,
    TRAIT_IMPULSE,
    TRAIT_BURNRES,
    TRAIT_BLEEDRES,
    TRAIT_ALL = (1<<TRAIT_HOVER)|(1<<TRAIT_IMPULSE)|(1<<TRAIT_BURNRES)|(1<<TRAIT_BLEEDRES)
};

#define WEAPON(a, \
    w01, w02, w03, w04, w05, w11, w12, w2, w3, w4, w5, w6, w7, w8, w9, wa, wb1, wb2, wc, wd, we1, we2, we3, we4, we5, we6, \
    wf, wg, wh, wi, wj, wk, mwj, mwk, xwj, xwk, wl, wm, wn1, wn2, wo1, wo2, wo3, wo4, wo5, wo6, wo7, wo8, wo9, wo10, wp, wq, \
    x21, x22, x31, x32, x33, x34, x4, x5, x6, x7, x81, x82, x9, xa, xb, xc, xd, xe, xf, xg, xh1, xh2, \
    t0, t1, t2, t3, y0, y1, y2, y3, y4, y5, y6, y7, y8, y9, ya, yb, yc, yd, ye1, ye2, yf1, yf2, yg, yh, \
    yi, yj, yk, yl, ym1, ym2, yn1, yn2, yo1, yo2, yp, yq, yr, ys1, ys2, yt1, yt2, yw1, yw2, yx1, yx2, yy1, yy2, yz1, yz2, \
    ya3, ya4, ya5, ya6, ya7, ya8, ya9, ya10, ya11, ya12, ya13, ya14, ya15, ya16, \
    ll01, ll02, ll03, ll04, ll05 \
 ) \
    GSVAR(0, a##name, #a); GVAR(IDF_HEX, a##colour, 0, w01, 0xFFFFFF); \
    GVAR(0, a##partcol1, -3, w02, 0xFFFFFF); GVAR(0, a##partcol2, -3, w03, 0xFFFFFF); \
    GVAR(0, a##explcol1, -3, w04, 0xFFFFFF); GVAR(0, a##explcol2, -3, w05, 0xFFFFFF); \
    GVAR(0, a##add, 1, w11, VAR_MAX); GVAR(0, a##max, 1, w12, VAR_MAX); \
    GVAR(0, a##sub1, 0, w2, VAR_MAX); GVAR(0, a##sub2, 0, w3, VAR_MAX); \
    GVAR(0, a##adelay1, 20, w4, VAR_MAX); GVAR(0, a##adelay2, 20, w5, VAR_MAX); GVAR(0, a##rdelay, 50, w6, VAR_MAX); \
    GVAR(0, a##damage1, VAR_MIN, w7, VAR_MAX); GVAR(0, a##damage2, VAR_MIN, w8, VAR_MAX); \
    GVAR(0, a##speed1, VAR_MIN, w9, VAR_MAX); GVAR(0, a##speed2, VAR_MIN, wa, VAR_MAX); \
    GVAR(0, a##power1, 0, wb1, VAR_MAX); GVAR(0, a##power2, 0, wb2, VAR_MAX); \
    GVAR(0, a##time1, 0, wc, VAR_MAX); GVAR(0, a##time2, 0, wd, VAR_MAX); \
    GVAR(0, a##pdelay1, 0, we1, VAR_MAX); GVAR(0, a##pdelay2, 0, we2, VAR_MAX); \
    GVAR(0, a##gdelay1, 0, we3, VAR_MAX); GVAR(0, a##gdelay2, 0, we4, VAR_MAX); \
    GVAR(0, a##edelay1, 0, we5, VAR_MAX); GVAR(0, a##edelay2, 0, we6, VAR_MAX); \
    GVAR(0, a##explode1, 0, wf, VAR_MAX); GVAR(0, a##explode2, 0, wg, VAR_MAX); \
    GVAR(0, a##rays1, 1, wh, VAR_MAX); GVAR(0, a##rays2, 1, wi, VAR_MAX); \
    GVAR(0, a##spread1, 0, wj, VAR_MAX); GVAR(0, a##spread2, 0, wk, VAR_MAX); \
    GVAR(0, a##minspread1, 0, mwj, VAR_MAX); GVAR(0, a##minspread2, 0, mwk, VAR_MAX); \
    GVAR(0, a##maxspread1, 0, xwj, VAR_MAX); GVAR(0, a##maxspread2, 0, xwk, VAR_MAX); \
    GVAR(0, a##zdiv1, 0, wl, VAR_MAX); GVAR(0, a##zdiv2, 0, wm, VAR_MAX); \
    GVAR(0, a##aiskew1, 0, wn1, VAR_MAX); GVAR(0, a##aiskew2, 0, wn2, VAR_MAX); \
    GVAR(0, a##flakweap1, -1, wo1, WEAP_MAX*2-1); GVAR(0, a##flakweap2, -1, wo2, WEAP_MAX*2-1); \
    GVAR(0, a##flakdmg1, VAR_MIN, wo3, VAR_MAX);GVAR(0, a##flakdmg2, VAR_MIN, wo4, VAR_MAX); \
    GVAR(0, a##flakrays1, 1, wo5, VAR_MAX); GVAR(0, a##flakrays2, 1, wo6, VAR_MAX); \
    GVAR(0, a##flaktime1, 1, wo7, VAR_MAX); GVAR(0, a##flaktime2, 1, wo8, VAR_MAX); \
    GVAR(0, a##flakspeed1, 0, wo9, VAR_MAX); GVAR(0, a##flakspeed2, 0, wo10, VAR_MAX); \
    GVAR(0, a##collide1, 0, wp, VAR_MAX); GVAR(0, a##collide2, 0, wq, VAR_MAX); \
    GVAR(0, a##extinguish1, 0, x21, 7); GVAR(0, a##extinguish2, 0, x22, 7); \
    GVAR(0, a##cooked1, 0, x31, 5); GVAR(0, a##cooked2, 0, x32, 5); \
    GVAR(0, a##guided1, 0, x33, 5); GVAR(0, a##guided2, 0, x34, 5); \
    GVAR(0, a##radial1, 0, x4, 1); GVAR(0, a##radial2, 0, x5, 1); \
    GVAR(0, a##residual1, 0, x6, 2); GVAR(0, a##residual2, 0, x7, 2); \
    GVAR(0, a##reloads, 0, x81, 1); GVAR(0, a##carried, 0, x82, 1); GVAR(0, a##zooms, 0, x9, 1); \
    GVAR(0, a##fullauto1, 0, xa, 1); GVAR(0, a##fullauto2, 0, xb, 1); \
    GVAR(0, a##allowed, 0, xc, 3); GVAR(0, a##laser, 0, xd, 1); \
    GVAR(0, a##critdash1, 0, xe, VAR_MAX); GVAR(0, a##critdash2, 0, xf, VAR_MAX); \
    GVAR(0, a##jamchance, 0, xg, VAR_MAX); \
    GVAR(0, a##stuntime1, 0, xh1, VAR_MAX); GVAR(0, a##stuntime2, 0, xh2, VAR_MAX); \
    GFVAR(0, a##taper1, 0, t0, 1); GFVAR(0, a##taper2, 0, t1, 1); \
    GFVAR(0, a##taperspan1, 0, t2, 1); GFVAR(0, a##taperspan2, 0, t3, 1); \
    GFVAR(0, a##elasticity1, FVAR_MIN, y0, FVAR_MAX); GFVAR(0, a##elasticity2, FVAR_MIN, y1, FVAR_MAX); \
    GFVAR(0, a##reflectivity1, 0, y2, 360); GFVAR(0, a##reflectivity2, 0, y3, 360); \
    GFVAR(0, a##relativity1, FVAR_MIN, y4, FVAR_MAX); GFVAR(0, a##relativity2, FVAR_MIN, y5, FVAR_MAX); \
    GFVAR(0, a##waterfric1, 0, y6, FVAR_MAX); GFVAR(0, a##waterfric2, 0, y7, FVAR_MAX); \
    GFVAR(0, a##weight1, FVAR_MIN, y8, FVAR_MAX); GFVAR(0, a##weight2, FVAR_MIN, y9, FVAR_MAX); \
    GFVAR(0, a##radius1, FVAR_NONZERO, ya, FVAR_MAX); GFVAR(0, a##radius2, FVAR_NONZERO, yb, FVAR_MAX); \
    GFVAR(0, a##kickpush1, FVAR_MIN, yc, FVAR_MAX); GFVAR(0, a##kickpush2, FVAR_MIN, yd, FVAR_MAX); \
    GFVAR(0, a##hitpush1, FVAR_MIN, ye1, FVAR_MAX); GFVAR(0, a##hitpush2, FVAR_MIN, ye2, FVAR_MAX); \
    GFVAR(0, a##slow1, 0, yf1, FVAR_MAX); GFVAR(0, a##slow2, 0, yf2, FVAR_MAX); \
    GFVAR(0, a##aidist1, 0, yg, FVAR_MAX); GFVAR(0, a##aidist2, 0, yh, FVAR_MAX); \
    GFVAR(0, a##partsize1, 0, yi, FVAR_MAX); GFVAR(0, a##partsize2, 0, yj, FVAR_MAX); \
    GFVAR(0, a##partlen1, 0, yk, FVAR_MAX); GFVAR(0, a##partlen2, 0, yl, FVAR_MAX); \
    GFVAR(0, a##frequency, 0, ym1, FVAR_MAX); GFVAR(0, a##pusharea, 0, ym2, FVAR_MAX); \
    GFVAR(0, a##stunscale1, 0, yn1, FVAR_MAX); GFVAR(0, a##stunscale2, 0, yn2, FVAR_MAX); \
    GFVAR(0, a##critmult, 0, yo1, FVAR_MAX); GFVAR(0, a##critdist, FVAR_MIN, yo2, FVAR_MAX); GFVAR(0, a##critboost, FVAR_MIN, yp, FVAR_MAX);\
    GFVAR(0, a##delta1, FVAR_NONZERO, yq, FVAR_MAX); GFVAR(0, a##delta2, FVAR_NONZERO, yr, FVAR_MAX); \
    GFVAR(0, a##trace1, 0, ys1, FVAR_MAX); GFVAR(0, a##trace2, 0, ys2, FVAR_MAX); \
    GFVAR(0, a##headmin1, 0, yt1, FVAR_MAX); GFVAR(0, a##headmin2, 0, yt2, FVAR_MAX); \
    GFVAR(0, a##whipdmg1, 0, yw1, FVAR_MAX); GFVAR(0, a##whipdmg2, 0, yw2, FVAR_MAX); \
    GFVAR(0, a##torsodmg1, 0, yx1, FVAR_MAX); GFVAR(0, a##torsodmg2, 0, yx2, FVAR_MAX); \
    GFVAR(0, a##legsdmg1, 0, yy1, FVAR_MAX); GFVAR(0, a##legsdmg2, 0, yy2, FVAR_MAX); \
    GFVAR(0, a##selfdmg1, 0, yz1, FVAR_MAX); GFVAR(0, a##selfdmg2, 0, yz2, FVAR_MAX); \
    GFVAR(0, a##flakscale1, 0, ya3, FVAR_MAX); GFVAR(0, a##flakscale2, 0, ya4, FVAR_MAX); \
    GFVAR(0, a##flakspread1, 0, ya5, FVAR_MAX); GFVAR(0, a##flakspread2, 0, ya6, FVAR_MAX); \
    GFVAR(0, a##flakrel1, 0, ya7, FVAR_MAX); GFVAR(0, a##flakrel2, 0, ya8, FVAR_MAX); \
    GFVAR(0, a##flakffwd1, 0, ya9, 1); GFVAR(0, a##flakffwd2, 0, ya10, 1); \
    GFVAR(0, a##flakoffset1, 0, ya11, FVAR_MAX); GFVAR(0, a##flakoffset2, 0, ya12, FVAR_MAX); \
    GFVAR(0, a##flakskew1, 0, ya13, FVAR_MAX); GFVAR(0, a##flakskew2, 0, ya14, FVAR_MAX); \
    GVAR(0, a##flakcollide1, 0, ya15, VAR_MAX); GVAR(0, a##flakcollide2, 0, ya16, VAR_MAX); \
    GVAR(0, a##leaguetraits, 0, ll01, TRAIT_ALL); \
    GFVAR(0, a##leaguehealth, 0, ll02, FVAR_MAX); GFVAR(0, a##leagueweight, 0, ll03, FVAR_MAX); \
    GFVAR(0, a##leaguespeed, 0, ll04, FVAR_MAX); GFVAR(0, a##leaguescale, 0, ll05, FVAR_MAX);

//  name            col             pcol1           pcol2           ecol1           ecol2
//  add     max     sub1    sub2    adly1   adly2   rdly    dam1    dam2    spd1    spd2    pow1    pow2    time1   time2
//  pdly1   pdly2   gdly1   gdly2   edly1   edly2   expl1   expl2   rays1   rays2   sprd1   sprd2   msprd1  msprd2  xsprd1  xsprd2
//  zdiv1   zdiv2   aiskew1 aiskew2 fweap1          fweap2          fdam1   fdam2   frays1  frays2  ftime1  ftime2  fspd1   fspd2
//  collide1
//  collide2
//  ext1    ext2    cook1   cook2   guide1  guide2  radl1   radl2   resid1  resid2  rlds    crd     zooms   fa1     fa2
//  allw    laser   cdash1  cdash2  jamch   stnt1   stnt2
//  tpr1    tpr2    tspan1  tspan2  elas1   elas2   rflt1   rflt2   relt1   relt2   wfrc1   wfrc2   wght1   wght2   rads1   rads2
//  kpsh1   kpsh2   hpsh1   hpsh2   slow1   slow2   aidst1  aidst2  psz1    psz2    plen1   plen2   freq    push    stns1   stns2
//  cmult   cdist   cboost  dlta1   dlta2   trce1   trce2   hdmin1  hdmin2  whpdm1  whipdm2 tordm1  tordm2  legdm1  legdm2  selfdm1 selfdm2
//  fscale1 fscale2 fsprd1  fsprd2  frel1   frel2   fffwd1  fffwd2  foff1   foff2   fskew1  fskew2
//  flakcollide1
//  flakcollide2
//  ltraits
//  lhealth lweight lspeed  lscale
WEAPON(melee,       0xEEEEEE,       0xEEEE22,       0xEEEE22,       -1,             -1,
    1,      1,      0,      0,      250,    500,    50,     30,     40,     0,      0,      0,      0,      100,    100,
    10,     10,     0,      0,      200,    200,    0,      0,      1,      1,      1,      1,      0,      0,      0,      0,
    1,      1,      1,      1,      -1,             -1,             25,     10,     5,      5,      500,    500,    0,      0,
    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_PLAYER|COLLIDE_TRACE,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      1,      1,
    2,      0,      500,    500,    0,      50,     100,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0,      0,      0,      0,      0,      0,      1,      1,
    0,      0,      200,    400,    1,      2,      16,     16,     1,      2,      0,      0,      0,      1,      0.5f,   0.5f,
    2,      0,      2,      10,     10,     2,      4,      0,      0,      0.8f,   0.8f,   0.7f,   0.6f,   0.3f,   0.3f,   0,      0,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1,
    IMPACT_PLAYER|COLLIDE_TRACE,
    IMPACT_PLAYER|COLLIDE_TRACE,
    0,
    1,      1,      1,      1
);
WEAPON(pistol,      0x888888,       0x666611,       0x666611,       -1,             -1,
    10,     10,     1,      2,      150,    300,    1000,   35,     11,     3000,   1000,   0,      0,      2000,   400,
    0,      0,      0,      0,      200,    200,    0,      0,      1,      5,      1,      5,      0,      0,      0,      0,
    2,      1,      100,    100,    -1,             -1,             10,     12,     5,      5,      500,    500,    0,      0,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      1,      0,      0,      0,      0,
    2,      0,      0,      0,      10,     25,     25,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0.05f,  0.05f,  2,      2,      0,      0,      1,      1,
    5,      3,      35,     50,     0.25f,  0.15f,  256,    256,    1,      1,      10,     10,     1,      1,      0.25f,  0.25f,
    3,      128,    2,      10,     10,     1,      1,      0,      0,      0.8f,   0.8f,   0.65f,  0.65f,  0.325f, 0.325f, 0,      0,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE,
    0,
    1,      1,      1,      1
);
WEAPON(sword,       0x111199,       0x111199,       0x111199,       0x111199,       0x111199,
    1,      1,      0,      0,      500,    750,    50,     30,     60,     0,      0,      0,      0,      350,    500,
    10,     10,     0,      0,      200,    200,    0,      0,      1,      1,      1,      1,      0,      0,      0,      0,
    1,      1,      1,      1,      -1,             -1,             15,     30,     5,      5,      500,    500,    0,      0,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|COLLIDE_CONT,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|COLLIDE_CONT,
    2,      2,      0,      0,      0,      0,      0,      0,      2,      2,      0,      1,      0,      1,      1,
    2,      0,      500,    500,    0,      100,    200,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0,      0,      0,      0,      0,      0,      1,      1,
    0,      0,      250,    500,    0.75f,  1,      48,     48,     1,      1.25f,  0,      0,      1,      1,      0.25f,  0.5f,
    2,      0,      2,      10,     10,     3,      5,      0,      0,      0.8f,   0.8f,   0.65f,  0.65f,  0.3f,   0.3f,   0,      0,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|COLLIDE_CONT,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|IMPACT_SHOTS|COLLIDE_CONT,
    (1<<TRAIT_BLEEDRES),
    2.f,    1.5f,   0.5f,   1.5f
);
WEAPON(shotgun,     0x888800,       0x888800,       0x888800,       0x888800,       0x888800,
    2,      8,      1,      2,      600,    900,    750,    15,     5,      1000,   250,    0,      0,      400,    5000,
    0,      0,      0,      0,      200,    200,    0,      0,      10,     1,      4,      2,      16,     0,      0,      0,
    1,      4,      10,     10,     -1,             WALT(SHOTGUN),  16,     4,      5,      40,     250,    2000,   0,      0,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      2,      1,      1,      0,      0,      0,
    2,      0,      0,      0,      100,    50,     50,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0.05f,  0.75f,  2,      2,      0,      250,    1,      1,
    30,     60,     200,    100,    1,      0.5f,   256,    512,    0.75f,  0.5f,   25,     15,     1,      1.5f,   0.5f,   0.25f,
    2,      256,    2,      10,     10,     1,      1,      0,      0,      0.8f,   0.9f,   0.6f,   0.7f,   0.3f,   0.4f,   0.5f,   0.5f,
    1,      1,      1,      0.2f,   1,      1.5f,   0,      0,      8,      8,      1,      1,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    (1<<TRAIT_BLEEDRES),
    1.25f,   1.25f,  0.8f,  1.2f
);
WEAPON(smg,         0xDD6600,       0xDD6600,       0xDD6600,       0xDD6600,       0xDD6600,
    40,     40,     1,      2,      100,    200,    1500,   22,     10,     2000,   350,    0,      0,      800,    150,
    0,      0,      0,      100,    200,    200,    0,      0,      1,      1,      3,      6,      0,      0,      0,      0,
    2,      2,      20,     20,     -1,             WALT(SMG),      16,     4,      5,      20,     500,    500,    0,      0,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      0,      1,      1,
    2,      0,      0,      0,      25,     50,     50,
    0,      0,      0,      0,      0.65f,  0.45f,  0,      0,      0.05f,  0.05f,  2,      2,      0,      0,      1,      1,
    5,      10,     150,    75,     1.5f,   0.75f,  512,    96,     0.5f,   0.6f,   35,     25,     1,      1.5f,   0.5f,   0.5f,
    3,      128,    2,      10,     1000,   1,      1,      0,      0,      0.8f,   0.8f,   0.7f,   0.7f,   0.35f,  0.35f,  0.5f,   0.5f,
    1,      1,      0.2f,   0.1f,   1,      1,      0,      0,      8,      8,      1,      1,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_OWNER,
    (1<<TRAIT_IMPULSE),
    0.85f,  1.f,    1.2f,   0.8f
);
WEAPON(flamer,      0xBB2222,       -1,             -1,             -1,             -1,
    25,     25,     1,      5,      100,    500,    2000,   8,      8,      300,    150,    0,      500,    350,    350,
    0,      25,     0,      0,      200,    200,    18,     28,     1,      1,      5,      5,      0,      0,      0,      0,
    0,      0,      10,     10,     -1,             WALT(FLAMER),   12,     4,      5,      5,      1000,   5000,   200,    250,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    3,      3,      0,      1,      0,      0,      1,      1,      1,      1,      1,      1,      0,      1,      0,
    2,      0,      0,      0,      25,     0,      0,
    0,      0,      0,      0,      0.5f,   0.35f,  0,      0,      0.95f,  0.5f,   1,      1,      200,    100,    1,      1,
    0.25f,  1,      25,     100,    0.25f,  1,      64,     128,    18,     28,     0,      5,      2,      1.5f,   0,      0,
    4,      64,     2,      10,     10,     1,      1,      4,      4,      0.65f,  0.65f,  0.45f,  0.45f,  0.25f,  0.25f,  0.5f,   0.5f,
    0.5f,   0.5f,   0.1f,   0.1f,   1,      1,      0.5f,   0.5f,   8,      8,      1,      1,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
    (1<<TRAIT_BURNRES),
    0.75f,  0.75f,  0.75f,  0.75f
);
WEAPON(plasma,      0x44DDCC,       0x44DDCC,       0x44DDCC,       0x44DDCC,       0x44DDCC,
    20,     20,     1,      20,     300,    1000,   2000,   20,     10,     1000,   30,     0,      2000,   750,    5000,
    0,      75,     0,      0,      200,    200,    10,     50,     1,      1,      2,      1,      0,      0,      0,      0,
    2,      1,      50,     10,     -1,             -1,             10,     5,      5,      5,      500,    500,    0,      0,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_SHOTS|COLLIDE_RADIAL|COLLIDE_OWNER|COLLIDE_STICK,
    1,      0,      0,      1,      0,      0,      0,      1,      0,      0,      1,      1,      0,      1,      0,
    2,      0,      0,      0,      35,     35,     50,
    0,      0.5f,   0,      0.5f,   0.5f,   0.5f,   0,      0,      0.125f, 0.125f, 1,      1,      0,      0,      4,      2,
    10,     150,    100,    -250,   1,      2,      128,    64,     5,      25,     0,      0,      2,      1.5f,   0.25f,  0.75f,
    2,      0,      2,      10,     10,     1,      1,      8,      4,      0.6f,   0.6f,   0.4f,   0.4f,   0.2f,   0.2f,   0.5f,   0.5f,
    1,      1,      1,      1,      1,      1,      0,      0,      8,      8,      1,      1,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER,
    IMPACT_GEOM|IMPACT_SHOTS|COLLIDE_RADIAL|COLLIDE_OWNER|COLLIDE_STICK,
    (1<<TRAIT_HOVER),
    0.9f,   1.2f,   0.9f,   0.9f
);
WEAPON(rifle,       0x6622AA,       0x6622AA,       0x6622AA,       0x6622AA,       0x6622AA,
    5,      5,      1,      1,      750,    1000,   1750,   32,     150,    5000,   100000, 0,      0,      5000,   5000,
    0,      0,      0,      0,      200,    200,    30,     0,      1,      1,      1,      0,      0,      0,      0,      0,
    0,      0,      40,     40,     -1,             -1,             18,     50,     5,      5,      500,    500,    0,      0,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_CONT,
    2,      2,      0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      1,      0,      0,
    2,      0,      0,      0,      75,     50,     100,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      1,      0,      2,      2,      0,      0,      1,      1,
    20,     10,     100,    200,    1,      2,      768,    2048,   1.5f,   3,      256,    512,    1,      1.25f,  0.25f,  0.75f,
    2,      1024,   2,      10,     10,     1,      1,      4,      4,      0.6f,   0.75f,  0.4f,   0.5f,   0.2f,   0.25f,  0.5f,   0.5f,
    1,      1,      0.25f,  0.25f,  1,      1,      0,      0,      8,      8,      1,      1,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_TRACE,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_TRACE|COLLIDE_CONT,
    (1<<TRAIT_BLEEDRES),
    0.8f,   1.1f,   0.8f,   0.8f
);
WEAPON(grenade,     0x119911,       -1,             -1,             0x981808,      0x981808,
    1,      2,      1,      1,      1000,   1000,   1500,   100,    100,    250,    250,    3000,   3000,   3000,   3000,
    75,     75,     0,      0,      200,    200,    75,     75,     1,      1,      1,      1,      0,      0,      0,      0,
    0,      0,      5,      5,      WEAP_SHOTGUN,   WEAP_SHOTGUN,   150,    150,    75,     75,     3000,   3000,   300,    300,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_STICK|COLLIDE_SHOTS,
    2,      2,      2,      2,      0,      0,      0,      0,      1,      1,      0,      0,      0,      0,      0,
    3,      0,      0,      0,      50,     100,    100,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      1,      1,      2,      2,      65,     65,     1,      1,
    5,      5,      250,    250,    2,      2,      384,    256,    1,      1,      0,      0,      2,      2,      0.5f,   0.5f,
    2,      0,      2,      10,     10,     1,      1,      8,      8,      0.6f,   0.6f,   0.4f,   0.4f,   0.2f,   0.2f,   0.5f,   0.5f,
    1,      1,      1,      1,      0,      0,      0,      0,      0,      0,      1,      1,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_STICK|COLLIDE_SHOTS,
    (1<<TRAIT_BURNRES),
    1.1f,   1.1f,   1.f,    1.1f
);
WEAPON(rocket,      0x882200,       -1,             -1,              0x981808,      0x981808,
    1,      1,      1,      1,      1000,   1000,   1500,   200,     200,   1000,   250,    2000,   2000,   5000,   5000,
    0,      0,      0,      0,      200,    200,    100,    100,     1,     1,      1,      1,      0,      0,      0,      0,
    0,      0,      10,     10,     WEAP_SMG,       WEAP_SMG,        300,   300,    75,     75,     3000,   3000,   400,    400,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    2,      2,      2,      2,      0,      1,      0,      0,       1,     1,      0,      1,      0,      0,      0,
    1,      0,      0,      0,      50,     250,    250,
    0,      0,      0,      0,      0.5f,   0.5f,   0,      0,      0,      0,      2,      2,      0,      0,      1,      1,
    150,    150,    750,    500,    4,      3,      1024,   512,    3,      3,      0,      0,      3,      4,      1,      1,
    2,      0,      2,      10,     10,     1,      1,      16,     16,     0.6f,   0.6f,   0.4f,   0.4f,   0.2f,   0.2f,   0.5f,   0.5f,
    1,      1,      1,      1,      0,      0,      0,      0,      0,      0,      1,      1,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    IMPACT_GEOM|IMPACT_PLAYER|IMPACT_SHOTS|COLLIDE_OWNER|COLLIDE_SHOTS,
    (1<<TRAIT_BURNRES),
    1.2f,   1.6f,   0.6f,   1.4f
);

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
            { 0, 0 },               5,          0.35f,
            "pistol",   "weapons/pistol/item",      "weapons/pistol/vwep", "weapons/pistol/hwep",     "",                     "projs/cartridge"
    },
    {
            ANIM_SWORD,         S_SWORD,    1,
            true,       true,       true,       false,
            { 0, 0 },               15,         0,
            "sword",    "weapons/sword/item",       "weapons/sword/vwep", "weapons/sword/hwep",    "",                     ""
    },
    {
            ANIM_SHOTGUN,       S_SHOTGUN,  10,
            false,      false,      true,       true,
            { 0, 0 },               9,          0.45f,
            "shotgun",  "weapons/shotgun/item",     "weapons/shotgun/vwep", "weapons/shotgun/hwep",    "",                     "projs/shell"
    },
    {
            ANIM_SMG,           S_SMG,      20,
            false,      false,      true,       true,
            { 0, 0 },               7,          0.35f,
            "smg",      "weapons/smg/item",         "weapons/smg/vwep", "weapons/smg/hwep",        "",                     "projs/cartridge"
    },
    {
            ANIM_FLAMER,        S_FLAMER,   1,
            false,      false,      true,       true,
            { 0, 0 },               9,          0,
            "flamer",   "weapons/flamer/item",      "weapons/flamer/vwep", "weapons/flamer/hwep",     "",                     ""
    },
    {
            ANIM_PLASMA,        S_PLASMA,   1,
            false,      false,      true,       false,
            { 0, 0 },               8,          0,
            "plasma",   "weapons/plasma/item",      "weapons/plasma/vwep", "weapons/plasma/hwep",     "",                     ""
    },
    {
            ANIM_RIFLE,         S_RIFLE,    1,
            false,      false,      true,       false,
            { 0, 0 },               9,          0,
            "rifle",    "weapons/rifle/item",       "weapons/rifle/vwep", "weapons/rifle/hwep",      "",                     ""
    },
    {
            ANIM_GRENADE,       S_GRENADE,  1,
            false,      false,      false,      false,
            { 0.0625f, 0.0625f },   5,          0,
            "grenade",  "weapons/grenade/item",     "weapons/grenade/vwep", "weapons/grenade/hwep",    "weapons/grenade/proj", ""
    },
    {
            ANIM_ROCKET,        S_ROCKET,   1,
            false,      false,      true,      false,
            { 0, 0 },               9,          0,
            "rocket",   "weapons/rocket/item",       "weapons/rocket/vwep", "weapons/rocket/hwep",     "weapons/rocket/proj",  ""
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
#define WEAPLM(a,b,c)           (a*(m_league(b, c) ? GAME(explodeleague) : (m_limited(b, c) ? GAME(explodelimited) : GAME(explodescale))))
#define WEAPEX(a,b,c,d,e)       (!m_insta(c, d) || m_loadout(c, d) || a != WEAP_RIFLE ? int(ceilf(WEAPLM(WEAP2(a, explode, b)*e, c, d))) : 0)
#define WEAPSP(a,b,c,d,e,f)     (!m_insta(c, d) || m_loadout(c, d) || a != WEAP_RIFLE ? clamp(int(ceilf(max(WEAP2(a, spread, b), f)*e)), WEAP2(a, minspread, b), WEAP2(a, maxspread, b) ? WEAP2(a, maxspread, b) : INT_MAX) : 0)
#define WEAPSND(a,b)            (weaptype[a].sound+b)
#define WEAPSNDF(a,b)           (weaptype[a].sound+(b ? S_W_SECONDARY : S_W_PRIMARY))
#define WEAPSND2(a,b,c)         (weaptype[a].sound+(b ? c+1 : c))
#define WEAPUSE(a)              (WEAP(a, reloads) ? WEAP(a, max) : WEAP(a, add))
#define WEAPPCOL(d,a,b,c)       (WEAP2(a, b, c) >= 0 ? WEAP2(a, b, c) : game::hexpulsecolour(d, clamp(-1-WEAP2(a, b, c), 0, 2), 50))

WEAPDEF(char *, name);
WEAPDEF(int, colour);
WEAPDEF2(int, partcol);
WEAPDEF2(int, explcol);
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
WEAPDEF(int, jamchance);
WEAPDEF2(int, stuntime);
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
WEAPDEF2(float, stunscale);
WEAPDEF(float, critmult);
WEAPDEF(float, critdist);
WEAPDEF(float, critboost);
WEAPDEF2(float, delta);
WEAPDEF2(float, trace);
WEAPDEF2(float, headmin);
WEAPDEF2(float, whipdmg);
WEAPDEF2(float, torsodmg);
WEAPDEF2(float, legsdmg);
WEAPDEF2(float, selfdmg);
WEAPDEF2(float, flakscale);
WEAPDEF2(float, flakspread);
WEAPDEF2(float, flakrel);
WEAPDEF2(float, flakffwd);
WEAPDEF2(float, flakoffset);
WEAPDEF2(float, flakskew);
WEAPDEF2(int, flakcollide);

WEAPDEF(int, leaguetraits);
WEAPDEF(float, leaguehealth);
WEAPDEF(float, leagueweight);
WEAPDEF(float, leaguespeed);
WEAPDEF(float, leaguescale);
