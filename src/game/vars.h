GVAR(IDF_ADMIN, serverdebug, 0, 0, 3);
GVAR(IDF_ADMIN, serverclients, 1, 16, MAXCLIENTS);
GVAR(IDF_ADMIN, serveropen, 0, 3, 3);
GSVAR(IDF_ADMIN, serverdesc, "");
GSVAR(IDF_ADMIN, servermotd, "");
GVAR(IDF_ADMIN, automaster, 0, 0, 1);

GVAR(IDF_ADMIN, modelimit, 0, G_DEATHMATCH, G_MAX-1);
GVAR(IDF_ADMIN, mutslimit, 0, G_M_ALL, G_M_ALL);
GVAR(IDF_ADMIN, modelock, 0, 0, 5); // 0 = off, 1 = master only (+1 admin only), 3 = master can only set limited mode and higher (+1 admin), 5 = no mode selection
GVAR(IDF_ADMIN, mapslock, 0, 0, 5); // 0 = off, 1 = master can select non-allow maps (+1 admin), 3 = master can select non-rotation maps (+1 admin), 5 = no map selection
GVAR(IDF_ADMIN, varslock, 0, 1, 3); // 0 = off, 1 = master, 2 = admin only, 3 = nobody
GVAR(IDF_ADMIN, votelock, 0, 0, 5); // 0 = off, 1 = master can select same game (+1 admin), 3 = master only can vote (+1 admin), 5 = no voting
GVAR(IDF_ADMIN, votewait, 0, 3000, INT_MAX-1);

GVAR(IDF_ADMIN, resetbansonend, 0, 1, 2); // reset bans on end (1: just when empty, 2: when matches end)
GVAR(IDF_ADMIN, resetvarsonend, 0, 1, 2); // reset variables on end (1: just when empty, 2: when matches end)
GVAR(IDF_ADMIN, resetmmonend, 0, 2, 2); // reset mastermode on end (1: just when empty, 2: when matches end)

GVARF(IDF_ADMIN, gamespeed, 1, 100, 1000, timescale = sv_gamespeed, timescale = gamespeed);
GVARF(IDF_ADMIN, gamepaused, 0, 0, 1, paused = sv_gamepaused, paused = gamepaused);

GSVAR(IDF_ADMIN, defaultmap, "");
GVAR(IDF_ADMIN, defaultmode, G_START, G_DEATHMATCH, G_MAX-1);
GVAR(IDF_ADMIN, defaultmuts, 0, G_M_TEAM, G_M_ALL);
GVAR(IDF_ADMIN, rotatemode, 0, 0, 1);
GVAR(IDF_ADMIN, rotatemuts, 0, 0, 1);
GVAR(IDF_ADMIN, campaignplayers, 1, 4, MAXPLAYERS);

GSVAR(IDF_ADMIN, allowmaps, "alphacampaign bath bloodgrounds darkness dawn deadsimple deathtrap deli depot dutility echo enigma forge futuresport futuresport2 ghost hinder keystone longestyard mainframe mist nova panic smouldering spacetech stone testchamber tower tranquility ubik venus warp wet magusland");
GSVAR(IDF_ADMIN, mainmaps, "bath bloodgrounds darkness deadsimple deathtrap deli depot dutility echo enigma forge futuresport futuresport2 ghost keystone longestyard mainframe mist nova panic smouldering spacetech stone tower tranquility ubik venus warp wet magusland");

GSVAR(IDF_ADMIN, capturemaps, "bath darkness deadsimple deli depot dutility echo enigma forge futuresport futuresport2 ghost keystone mist nova panic smouldering stone tranquility ubik venus warp wet magusland");
GSVAR(IDF_ADMIN, defendmaps, "bath bloodgrounds darkness deadsimple deli depot dutility echo enigma forge futuresport futuresport2 ghost keystone mist nova panic smouldering stone tower tranquility ubik venus warp wet");
GSVAR(IDF_ADMIN, bombermaps, "bath deadsimple deli depot echo forge futuresport futuresport2 tranquility venus magusland");
GSVAR(IDF_ADMIN, trialmaps, "hinder testchamber");
GSVAR(IDF_ADMIN, campaignmaps, "alphacampaign");

GSVAR(IDF_ADMIN, duelmaps, "bath darkness deadsimple dutility echo longestyard mainframe panic venus");
GSVAR(IDF_ADMIN, jetpackmaps, "alphacampaign bath bloodgrounds darkness dawn deadsimple deathtrap deli depot dutility echo enigma forge futuresport futuresport2 ghost hinder keystone longestyard mainframe mist nova panic smouldering spacetech stone testchamber tower tranquility ubik venus warp magusland");

namespace server { extern void resetgamevars(bool flush); }
GICOMMAND(0, resetvars, "", (), server::resetgamevars(true), return);
GICOMMAND(IDF_ADMIN, resetconfig, "", (), rehash(true), );

GVAR(0, maprotate, 0, 2, 2); // 0 = off, 1 = sequence, 2 = random

GVAR(0, maxcarry, 1, 2, WEAP_CARRY);
GVAR(0, spawnrotate, 0, 4, INT_MAX-1); // 0 = let client decide, 1 = sequence, 2+ = random
GVAR(0, spawnweapon, 0, WEAP_PISTOL, WEAP_MAX-1);
GVAR(0, instaweapon, 0, WEAP_RIFLE, WEAP_MAX-1);
GVAR(0, trialweapon, 0, WEAP_MELEE, WEAP_MAX-1);
GVAR(0, spawngrenades, 0, 0, 2); // 0 = never, 1 = all but insta/trial, 2 = always
GVAR(0, spawndelay, 0, 5000, INT_MAX-1); // delay before spawning in most modes
GVAR(0, instadelay, 0, 2500, INT_MAX-1); // .. in instagib/arena matches
GVAR(0, trialdelay, 0, 500, INT_MAX-1); // .. in time trial matches
GVAR(0, bomberdelay, 0, 5000, INT_MAX-1); // delay before spawning in bomber
GVAR(0, spawnprotect, 0, 3000, INT_MAX-1); // delay before damage can be dealt to spawning player
GVAR(0, duelprotect, 0, 5000, INT_MAX-1); // .. in duel/survivor matches
GVAR(0, instaprotect, 0, 1500, INT_MAX-1); // .. in instagib matches

GVAR(0, maxhealth, 0, 100, INT_MAX-1);
GVAR(0, extrahealth, 0, 150, INT_MAX-1);

GVAR(0, burntime, 0, 5500, INT_MAX-1);
GVAR(0, burndelay, 0, 1000, INT_MAX-1);
GVAR(0, burndamage, 0, 5, INT_MAX-1);
GVAR(0, bleedtime, 0, 5500, INT_MAX-1);
GVAR(0, bleeddelay, 0, 1000, INT_MAX-1);
GVAR(0, bleeddamage, 0, 5, INT_MAX-1);

GVAR(0, regendelay, 0, 3000, INT_MAX-1);
GVAR(0, regenguard, 0, 1000, INT_MAX-1);
GVAR(0, regentime, 0, 1000, INT_MAX-1);
GVAR(0, regenhealth, 0, 5, INT_MAX-1);
GVAR(0, regenextra, 0, 10, INT_MAX-1);
GVAR(0, regenaffinity, 0, 1, 2); // 0 = off, 1 = only guarding, 2 = also while carrying

GVAR(0, kamikaze, 0, 1, 3); // 0 = never, 1 = holding grenade, 2 = have grenade, 3 = always
GVAR(0, itemsallowed, 0, 2, 2); // 0 = never, 1 = all but limited, 2 = always
GVAR(0, itemspawntime, 1, 30000, INT_MAX-1); // when items respawn
GVAR(0, itemspawndelay, 0, 1000, INT_MAX-1); // after map start items first spawn
GVAR(0, itemspawnstyle, 0, 1, 3); // 0 = all at once, 1 = staggered, 2 = random, 3 = randomise between both
GFVAR(0, itemthreshold, 0, 1, 1000); // if numitems/numclients/maxcarry is less than this, spawn one of this type
GFVAR(0, itemelasticity, -10000, 0.35f, 10000);
GFVAR(0, itemrelativity, -10000, 1, 10000);
GFVAR(0, itemwaterfric, 0, 1.75f, 10000);
GFVAR(0, itemweight, -10000, 150, 10000);

GVAR(0, timelimit, 0, 30, INT_MAX-1);
GVAR(0, triallimit, 0, 60000, INT_MAX-1);
GVAR(0, intermlimit, 0, 10000, INT_MAX-1); // .. before vote menu comes up
GVAR(0, votelimit, 0, 30000, INT_MAX-1); // .. before vote passes by default
GVAR(0, duelreset, 0, 1, 1); // reset winner in duel
GVAR(0, duelclear, 0, 1, 1); // clear items in duel
GVAR(0, duellimit, 0, 7500, INT_MAX-1); // .. before duel goes to next round

GVAR(0, selfdamage, 0, 1, 1); // 0 = off, 1 = either hurt self or use teamdamage rules
GVAR(0, trialdamage, 0, 1, 1); // 0 = off, 1 = allow damage in time-trial
GVAR(0, teamdamage, 0, 1, 2); // 0 = off, 1 = non-bots damage team, 2 = all players damage team
GVAR(0, teambalance, 0, 1, 3); // 0 = off, 1 = by number then rank, 2 = by rank then number, 3 = humans vs. ai
GVAR(0, pointlimit, 0, 300, INT_MAX-1); // finish when score is this or more

GVAR(0, capturelimit, 0, 15, INT_MAX-1); // finish when score is this or more
GVAR(0, captureresetdelay, 0, 30000, INT_MAX-1);
GFVAR(0, captureelasticity, -10000, 0.25f, 10000);
GFVAR(0, captureweight, -10000, 75, 10000);

GVAR(0, defendlimit, 0, 300, INT_MAX-1); // finish when score is this or more
GVAR(0, defendpoints, 0, 1, INT_MAX-1); // points added to score
GVAR(0, defendinterval, 0, 100, INT_MAX-1);
GVAR(0, defendoccupy, 0, 100, INT_MAX-1); // points needed to occupy
GVAR(0, defendflags, 0, 3, 3); // 0 = init all (neutral), 1 = init neutral and team only, 2 = init team only, 3 = init all (team + neutral + converted)

GVAR(0, bomberlimit, 0, 15, INT_MAX-1); // finish when score is this or more (non-hold)
GVAR(0, bomberholdlimit, 0, 300, INT_MAX-1); // finish when score is this or more (hold)
GVAR(0, bomberresetdelay, 0, 15000, INT_MAX-1);
GVAR(0, bombercarrytime, 0, 15000, INT_MAX-1);
GVAR(0, bomberholdpoints, 0, 1, INT_MAX-1); // points added to score
GVAR(0, bomberholdpenalty, 0, 10, INT_MAX-1); // penalty for holding too long
GVAR(0, bomberholdinterval, 0, 1000, INT_MAX-1);
GVAR(0, bomberlockondelay, 0, 1000, INT_MAX-1);
GFVAR(0, bomberspeed, 0, 200, 10000);
GFVAR(0, bomberminvel, 0, 50, 10000);
GFVAR(0, bomberdelta, 0, 1000, 10000);
GFVAR(0, bomberrelativity, 0, 0.25f, 10000);
GFVAR(0, bomberelasticity, -10000, 0.5f, 10000);
GFVAR(0, bomberweight, -10000, 100, 10000);

GVAR(0, skillmin, 1, 50, 101);
GVAR(0, skillmax, 1, 70, 101);
GVAR(0, botbalance, -1, -1, INT_MAX-1); // -1 = always use numplayers, 0 = don't balance, 1 or more = fill only with this*numteams
GVAR(0, botlimit, 0, 16, INT_MAX-1);
GVAR(0, enemybalance, 0, 1, 3);
GVAR(0, enemyspawntime, 1, 60000, INT_MAX-1); // when enemies respawn
GVAR(0, enemyspawndelay, 0, 500, INT_MAX-1); // after map start enemies first spawn
GVAR(0, enemyspawnstyle, 0, 1, 3); // 0 = all at once, 1 = staggered, 2 = random, 3 = randomise between both
GFVAR(0, enemystrength, 1e-4f, 1, 1000); // scale enemy health values by this much

GFVAR(0, forcegravity, -1, -1, 1000);
GFVAR(0, forceliquidspeed, -1, -1, 1);
GFVAR(0, forceliquidcurb, -1, -1, 1000);
GFVAR(0, forcefloorcurb, -1, -1, 1000);
GFVAR(0, forceaircurb, -1, -1, 1000);
GFVAR(0, forceslidecurb, -1, -1, 1000);

GFVAR(0, movespeed, 0, 100, 1000); // speed
GFVAR(0, movecrawl, 0, 0.5f, 1000); // crawl modifier
GFVAR(0, movesprint, 0, 1.6f, 1000); // sprinting modifier
GFVAR(0, movejetpack, 0, 1.6f, 1000); // jetpack modifier
GFVAR(0, movestraight, 0, 1.2f, 1000); // non-strafe modifier
GFVAR(0, movestrafe, 0, 1, 1000); // strafe modifier
GFVAR(0, moveinair, 0, 0.9f, 1000); // in-air modifier
GFVAR(0, movestepup, 0, 0.9f, 1000); // step-up modifier
GFVAR(0, movestepdown, 0, 1.1f, 1000); // step-down modifier

GFVAR(0, jumpspeed, 0, 110, 1000); // extra velocity to add when jumping
GFVAR(0, impulsespeed, 0, 80, 1000); // extra velocity to add when impulsing
GFVAR(0, impulselimit, 0, 0, 10000); // maximum impulse speed
GFVAR(0, impulseboost, 0, 1, 1000); // thrust modifier
GFVAR(0, impulseboostz, -1, 0, 1); // thrust z modifier
GFVAR(0, impulsedash, 0, 1.2f, 1000); // dashing modifier
GFVAR(0, impulsejump, 0, 1.1f, 1000); // jump modifier
GFVAR(0, impulsemelee, 0, 0.5f, 1000); // melee modifier
GFVAR(0, impulseparkour, 0, 1.1f, 1000); // parkour modifier
GFVAR(0, impulseparkournorm, 0, 0.5f, 1000); // minimum parkour surface z normal
GVAR(0, impulseallowed, 0, 3, 3); // impulse allowed; 0 = off, 1 = dash/boost only, 2 = dash/boost and sprint, 3 = all mechanics including parkour
GVAR(0, impulsestyle, 0, 1, 3); // impulse style; 0 = off, 1 = touch and count, 2 = count only, 3 = freestyle
GVAR(0, impulsecount, 0, 5, INT_MAX-1); // number of impulse actions per air transit
GVAR(0, impulsedelay, 0, 125, INT_MAX-1); // minimum time between boosts
GVAR(0, impulsedashdelay, 0, 1000, INT_MAX-1); // minimum time between dashes
GVAR(0, impulsejetdelay, 0, 250, INT_MAX-1); // minimum time between jetpack
GVAR(0, impulseslide, 0, 500, INT_MAX-1); // minimum time before floor friction kicks back in
GVAR(0, impulsemeter, 0, 20000, INT_MAX-1); // impulse dash length; 0 = unlimited, anything else = timer
GVAR(0, impulsecost, 0, 1000, INT_MAX-1); // cost of impulse jump
GVAR(0, impulseskate, 0, 1000, INT_MAX-1); // length of time a run along a wall can last
GFVAR(0, impulsesprint, 0, 0, 1000); // sprinting impulse meter depletion
GFVAR(0, impulsejetpack, 0, 1.5f, 1000); // jetpack impulse meter depletion
GFVAR(0, impulseregen, 0, 1, 1000); // impulse regen multiplier
GFVAR(0, impulseregencrouch, 0, 2, 1000); // impulse regen crouch modifier
GFVAR(0, impulseregensprint, 0, 0.75f, 1000); // impulse regen sprinting modifier
GFVAR(0, impulseregenjetpack, 0, 2, 1000); // impulse regen jetpack modifier
GFVAR(0, impulseregenmove, 0, 1, 1000); // impulse regen moving modifier
GFVAR(0, impulseregeninair, 0, 0, 1000); // impulse regen in-air modifier
GVAR(0, impulseregendelay, 0, 1000, INT_MAX-1); // delay before impulse regens

GFVAR(0, stillspread, 0, 1.5f, 1000);
GFVAR(0, movespread, 0, 3, 1000);
GFVAR(0, inairspread, 0, 3, 1000);
GFVAR(0, impulsespread, 0, 6, 1000);

GVAR(0, zoomlock, 0, 1, 2); // 0 = free, 1 = must be on floor, 2 = also must not be moving/sloping
GVAR(0, zoomlimit, 1, 10, 150);
GVAR(0, zoomtime, 1, 100, 10000);

GFVAR(0, normalscale, 0, 1, 1000);
GFVAR(0, limitedscale, 0, 0.75f, 1000);
GFVAR(0, damagescale, 0, 1, 1000);
GVAR(0, damagecritchance, 0, 100, INT_MAX-1);

GFVAR(0, hitpushscale, 0, 1, 1000);
GFVAR(0, slowscale, 0, 1, 1000);
GFVAR(0, deadpushscale, 0, 2, 1000);
GFVAR(0, wavepushscale, 0, 1, 1000);
GFVAR(0, kickpushscale, 0, 1, 1000);
GFVAR(0, kickpushcrouch, 0, 0.5f, 1000);
GFVAR(0, kickpushsway, 0, 0.0125f, 1000);
GFVAR(0, kickpushzoom, 0, 0.125f, 1000);

GVAR(0, assistkilldelay, 0, 5000, INT_MAX-1);
GVAR(0, multikilldelay, 0, 5000, INT_MAX-1);
GVAR(0, spreecount, 0, 5, INT_MAX-1);
GVAR(0, dominatecount, 0, 5, INT_MAX-1);

GVAR(0, alloweastereggs, 0, 1, 1); // 0 = off, 1 = on
GVAR(0, returningfire, 0, 0, 1); // 0 = off, 1 = on
