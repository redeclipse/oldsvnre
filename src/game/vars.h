GVAR(IDF_ADMIN, serverdebug, 0, 0, 3);
GVAR(IDF_ADMIN, serverclients, 1, 16, MAXCLIENTS);
GVAR(IDF_ADMIN, serveropen, 0, 3, 3);
GSVAR(IDF_ADMIN, serverdesc, "");
GSVAR(IDF_ADMIN, servermotd, "");
GVAR(IDF_ADMIN, automaster, 0, 0, 1);

GVAR(IDF_ADMIN, modelimit, 0, G_DEATHMATCH, G_MAX-1);
GVAR(IDF_ADMIN, mutslimit, 0, G_M_ALL, G_M_ALL);
GVAR(IDF_ADMIN, modelock, 0, 3, 5); // 0 = off, 1 = master only (+1 admin only), 3 = master can only set limited mode and higher (+1 admin), 5 = no mode selection
GVAR(IDF_ADMIN, mapslock, 0, 3, 5); // 0 = off, 1 = master can select non-allow maps (+1 admin), 3 = master can select non-rotation maps (+1 admin), 5 = no map selection
GVAR(IDF_ADMIN, varslock, 0, 1, 3); // 0 = off, 1 = master, 2 = admin only, 3 = nobody
GVAR(IDF_ADMIN, votelock, 0, 0, 5); // 0 = off, 1 = master can select same game (+1 admin), 3 = master only can vote (+1 admin), 5 = no voting
GVAR(IDF_ADMIN, votewait, 0, 2500, INT_MAX-1);

GVAR(IDF_ADMIN, autospectate, 0, 1, 1); // auto spectate if idle, 1 = auto spectate when remaining dead for autospecdelay
GVAR(IDF_ADMIN, autospecdelay, 0, 60000, INT_MAX-1);

GVAR(IDF_ADMIN, resetbansonend, 0, 1, 2); // reset bans on end (1: just when empty, 2: when matches end)
GVAR(IDF_ADMIN, resetvarsonend, 0, 1, 2); // reset variables on end (1: just when empty, 2: when matches end)
GVAR(IDF_ADMIN, resetmmonend, 0, 2, 2); // reset mastermode on end (1: just when empty, 2: when matches end)

GVARF(IDF_ADMIN, gamespeed, 1, 100, 1000, timescale = sv_gamespeed, timescale = gamespeed);
GVARF(IDF_ADMIN, gamepaused, 0, 0, 1, paused = sv_gamepaused, paused = gamepaused);

GSVAR(IDF_ADMIN, defaultmap, "");
GVAR(IDF_ADMIN, defaultmode, G_START, G_DEATHMATCH, G_MAX-1);
GVAR(IDF_ADMIN, defaultmuts, 0, 0, G_M_ALL);
GVAR(IDF_ADMIN, rotatemode, 0, 1, 1);
GVAR(IDF_ADMIN, rotatemuts, 0, 3, INT_MAX-1); // any more than one decreases the chances of it picking
GVAR(IDF_ADMIN, rotatefilter, 0, G_M_FILTER, G_M_ALL); // modes not in this array are filtered out
GVAR(IDF_ADMIN, campaignplayers, 1, 4, MAXPLAYERS);

GSVAR(IDF_ADMIN, allowmaps, "alphacampaign bath center darkness dawn deadsimple deathtrap deli depot dropzone dutility echo facility forge foundation futuresport ghost hinder keystone lab linear longestyard mist nova panic processing spacetech stone testchamber tranquility tribal ubik venus warp wet");
GSVAR(IDF_ADMIN, mainmaps, "bath center darkness deadsimple deathtrap deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear longestyard mist nova panic processing spacetech stone tranquility tribal ubik venus warp wet");

GSVAR(IDF_ADMIN, capturemaps, "bath center darkness deadsimple deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear mist nova panic stone tranquility tribal venus warp wet");
GSVAR(IDF_ADMIN, defendmaps, "bath center darkness deadsimple deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear mist nova panic processing stone tranquility tribal venus warp wet");
GSVAR(IDF_ADMIN, bombermaps, "bath center deadsimple deli depot dropzone echo forge foundation futuresport tranquility venus");
GSVAR(IDF_ADMIN, holdmaps, "bath center darkness deadsimple deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear mist nova panic processing stone tranquility tribal venus warp wet");
GSVAR(IDF_ADMIN, trialmaps, "testchamber");
GSVAR(IDF_ADMIN, campaignmaps, "alphacampaign");

GSVAR(IDF_ADMIN, duelmaps, "bath center darkness deadsimple dropzone dutility echo foundation linear longestyard panic venus");
GSVAR(IDF_ADMIN, jetpackmaps, "alphacampaign bath center darkness dawn deadsimple deathtrap deli depot dropzone dutility echo forge foundation futuresport ghost keystone linear longestyard mist nova panic spacetech stone testchamber tranquility tribal ubik venus warp");

namespace server { extern void resetgamevars(bool flush); }
GICOMMAND(0, resetvars, "", (), server::resetgamevars(true), return);
GICOMMAND(IDF_ADMIN, resetconfig, "", (), rehash(true), );

GVAR(0, maprotate, 0, 2, 2); // 0 = off, 1 = sequence, 2 = random

GFVAR(0, maxalive, 0, 0, 1e16f); // only allow this*numplayers to be alive at once
GVAR(0, maxalivequeue, 0, 1, 1); // if number of players exceeds this amount, use a queue system
GVAR(0, maxalivethreshold, 2, 8, INT_MAX-1); // kicks in if numplayers >= this

GVAR(0, maxcarry, 1, 2, WEAP_CARRY);
GVAR(0, spawnrotate, 0, 4, INT_MAX-1); // 0 = let client decide, 1 = sequence, 2+ = random
GVAR(0, spawnweapon, 0, WEAP_PISTOL, WEAP_MAX-1);
GVAR(0, instaweapon, 0, WEAP_RIFLE, WEAP_MAX-1);
GVAR(0, trialweapon, 0, WEAP_SWORD, WEAP_MAX-1);
GVAR(0, spawngrenades, 0, 0, 2); // 0 = never, 1 = all but insta/trial, 2 = always
GVAR(0, spawndelay, 0, 5000, INT_MAX-1); // delay before spawning in most modes
GVAR(0, instadelay, 0, 3000, INT_MAX-1); // .. in instagib/arena matches
GVAR(0, trialdelay, 0, 500, INT_MAX-1); // .. in time trial matches
GVAR(0, bomberdelay, 0, 3000, INT_MAX-1); // delay before spawning in bomber
GVAR(0, spawnprotect, 0, 3000, INT_MAX-1); // delay before damage can be dealt to spawning player
GVAR(0, duelprotect, 0, 5000, INT_MAX-1); // .. in duel/survivor matches
GVAR(0, instaprotect, 0, 3000, INT_MAX-1); // .. in instagib matches

GVAR(0, spawnhealth, 0, 100, INT_MAX-1);
GVAR(0, extrahealth, 0, 150, INT_MAX-1);
GVAR(0, maxhealth, 0, 200, INT_MAX-1);

GFVAR(0, actorscale, 1e-6f, 1, 1e16f);
GFVAR(0, maxresizescale, 1, 2, 1e16f);
GFVAR(0, minresizescale, 1e-6f, 0.5f, 1);

GVAR(0, burntime, 0, 5500, INT_MAX-1);
GVAR(0, burndelay, 0, 1000, INT_MAX-1);
GVAR(0, burndamage, 0, 5, INT_MAX-1);
GVAR(0, bleedtime, 0, 5500, INT_MAX-1);
GVAR(0, bleeddelay, 0, 1000, INT_MAX-1);
GVAR(0, bleeddamage, 0, 5, INT_MAX-1);

GVAR(0, regendelay, 0, 3000, INT_MAX-1); // regen after no damage for this long
GVAR(0, regenguard, 0, 1000, INT_MAX-1); // regen this often when guarding an affinity
GVAR(0, regentime, 0, 1000, INT_MAX-1); // regen this often when regenerating normally
GVAR(0, regenhealth, 0, 5, INT_MAX-1); // regen this amount each regen
GVAR(0, regenextra, 0, 2, INT_MAX-1); // add this to regen when influenced by affinity
GVAR(0, regenaffinity, 0, 1, 2); // 0 = off, 1 = only guarding, 2 = also while carrying

GVAR(0, kamikaze, 0, 1, 3); // 0 = never, 1 = holding grenade, 2 = have grenade, 3 = always
GVAR(0, itemsallowed, 0, 2, 2); // 0 = never, 1 = all but limited, 2 = always
GVAR(0, itemspawntime, 1, 30000, INT_MAX-1); // when items respawn
GVAR(0, itemspawndelay, 0, 1000, INT_MAX-1); // after map start items first spawn
GVAR(0, itemspawnstyle, 0, 1, 3); // 0 = all at once, 1 = staggered, 2 = random, 3 = randomise between both
GFVAR(0, itemthreshold, 0, 2, 1e16f); // if numitems/(players*maxcarry) is less than this, spawn one of this type
GVAR(0, itemcollide, 0, BOUNCE_GEOM, INT_MAX-1);
GVAR(0, itemextinguish, 0, 6, 7);
GFVAR(0, itemelasticity, -1e16f, 0.4f, 1e16f);
GFVAR(0, itemrelativity, -1e16f, 1, 1e16f);
GFVAR(0, itemwaterfric, 0, 1.75f, 1e16f);
GFVAR(0, itemweight, -1e16f, 150, 1e16f);
GFVAR(0, itemminspeed, 0, 0, 1e16f);
GFVAR(0, itemrepulsion, 0, 8, 1e16f);
GFVAR(0, itemrepelspeed, 0, 25, 1e16f);

GVAR(0, timelimit, 0, 10, INT_MAX-1);
GVAR(0, triallimit, 0, 60000, INT_MAX-1);
GVAR(0, intermlimit, 0, 15000, INT_MAX-1); // .. before vote menu comes up
GVAR(0, votelimit, 0, 45000, INT_MAX-1); // .. before vote passes by default
GVAR(0, duelreset, 0, 1, 1); // reset winner in duel
GVAR(0, duelclear, 0, 1, 1); // clear items in duel
GVAR(0, duellimit, 0, 5000, INT_MAX-1); // .. before duel goes to next round

GVAR(0, selfdamage, 0, 1, 1); // 0 = off, 1 = either hurt self or use teamdamage rules
GVAR(0, trialdamage, 0, 1, 1); // 0 = off, 1 = allow damage in time-trial
GVAR(0, teamdamage, 0, 1, 2); // 0 = off, 1 = non-bots damage team, 2 = all players damage team
GVAR(0, teambalance, 0, 1, 3); // 0 = off, 1 = by number then rank, 2 = by rank then number, 3 = humans vs. ai
GVAR(0, teampersist, 0, 1, 2); // 0 = off, 1 = only attempt, 2 = forced
GVAR(0, pointlimit, 0, 300, INT_MAX-1); // finish when score is this or more

GVAR(0, capturelimit, 0, 15, INT_MAX-1); // finish when score is this or more
GVAR(0, captureresetdelay, 0, 30000, INT_MAX-1);
GVAR(0, capturepickupdelay, -1, 5000, INT_MAX-1);
GVAR(0, capturecollide, 0, BOUNCE_GEOM, INT_MAX-1);
GVAR(0, captureextinguish, 0, 6, 7);
GFVAR(0, capturerelativity, 0, 0.25f, 1e16f);
GFVAR(0, captureelasticity, -1e16f, 0.35f, 1e16f);
GFVAR(0, capturewaterfric, -1e16f, 1.75f, 1e16f);
GFVAR(0, captureweight, -1e16f, 100, 1e16f);
GFVAR(0, captureminspeed, 0, 0, 1e16f);
GFVAR(0, capturethreshold, 0, 112, 1e16f); // if someone 'warps' more than this distance, auto-drop

GVAR(0, defendlimit, 0, 300, INT_MAX-1); // finish when score is this or more
GVAR(0, defendpoints, 0, 1, INT_MAX-1); // points added to score
GVAR(0, defendinterval, 0, 100, INT_MAX-1);
GVAR(0, defendoccupy, 0, 100, INT_MAX-1); // points needed to occupy
GVAR(0, defendflags, 0, 3, 3); // 0 = init all (neutral), 1 = init neutral and team only, 2 = init team only, 3 = init all (team + neutral + converted)

GVAR(0, bomberlimit, 0, 30, INT_MAX-1); // finish when score is this or more (non-hold)
GVAR(0, bomberholdlimit, 0, 300, INT_MAX-1); // finish when score is this or more (hold)
GVAR(0, bomberresetdelay, 0, 15000, INT_MAX-1);
GVAR(0, bomberpickupdelay, -1, 5000, INT_MAX-1);
GVAR(0, bombercarrytime, 0, 15000, INT_MAX-1);
GVAR(0, bomberholdpoints, 0, 1, INT_MAX-1); // points added to score
GVAR(0, bomberholdpenalty, 0, 10, INT_MAX-1); // penalty for holding too long
GVAR(0, bomberholdinterval, 0, 1000, INT_MAX-1);
GVAR(0, bomberlockondelay, 0, 250, INT_MAX-1);
GVAR(0, bomberreset, 0, 0, 2); // 0 = off, 1 = kill winners, 2 = kill everyone
GFVAR(0, bomberspeed, 0, 250, 1e16f);
GFVAR(0, bomberdelta, 0, 1000, 1e16f);
GVAR(0, bombercollide, 0, BOUNCE_GEOM, INT_MAX-1);
GVAR(0, bomberextinguish, 0, 6, 7);
GFVAR(0, bomberrelativity, 0, 0.25f, 1e16f);
GFVAR(0, bomberelasticity, -1e16f, 0.65f, 1e16f);
GFVAR(0, bomberwaterfric, -1e16f, 1.75f, 1e16f);
GFVAR(0, bomberweight, -1e16f, 150, 1e16f);
GFVAR(0, bomberminspeed, 0, 50, 1e16f);
GFVAR(0, bomberthreshold, 0, 112, 1e16f); // if someone 'warps' more than this distance, auto-drop

GVAR(IDF_ADMIN, airefresh, 0, 1000, INT_MAX-1);
GVAR(0, skillmin, 1, 50, 101);
GVAR(0, skillmax, 1, 75, 101);
GVAR(0, botbalance, -1, -1, INT_MAX-1); // -1 = always use numplayers, 0 = don't balance, 1 or more = fill only with this*numteams
GVAR(0, botlimit, 0, 16, INT_MAX-1);
GFVAR(0, botspeed, 0, 1, 1e16f);
GFVAR(0, botscale, 1e-6f, 1, 1e16f);
GVAR(0, enemybalance, 0, 1, 3);
GVAR(0, enemyspawntime, 1, 30000, INT_MAX-1); // when enemies respawn
GVAR(0, enemyspawndelay, 0, 1000, INT_MAX-1); // after map start enemies first spawn
GVAR(0, enemyspawnstyle, 0, 1, 3); // 0 = all at once, 1 = staggered, 2 = random, 3 = randomise between both
GFVAR(0, enemyspeed, 0, 1, 1e16f);
GFVAR(0, enemyscale, 1e-6f, 1, 1e16f);
GFVAR(0, enemystrength, 1e-6f, 1, 1e16f); // scale enemy health values by this much

GFVAR(0, forcegravity, -1, -1, 1e16f);
GFVAR(0, forceliquidspeed, -1, -1, 1);
GFVAR(0, forceliquidcurb, -1, -1, 1e16f);
GFVAR(0, forcefloorcurb, -1, -1, 1e16f);
GFVAR(0, forceaircurb, -1, -1, 1e16f);
GFVAR(0, forceslidecurb, -1, -1, 1e16f);

GFVAR(0, movespeed, 0, 100, 1e16f); // speed
GFVAR(0, movecrawl, 0, 0.6f, 1e16f); // crawl modifier
GFVAR(0, movesprint, 0, 1.6f, 1e16f); // sprinting modifier
GFVAR(0, movejetpack, 0, 1.6f, 1e16f); // jetpack modifier
GFVAR(0, movestraight, 0, 1.2f, 1e16f); // non-strafe modifier
GFVAR(0, movestrafe, 0, 1, 1e16f); // strafe modifier
GFVAR(0, moveinair, 0, 0.9f, 1e16f); // in-air modifier
GFVAR(0, movestepup, 0, 0.95f, 1e16f); // step-up modifier
GFVAR(0, movestepdown, 0, 1.15f, 1e16f); // step-down modifier

GFVAR(0, jumpspeed, 0, 110, 1e16f); // extra velocity to add when jumping
GFVAR(0, impulsespeed, 0, 90, 1e16f); // extra velocity to add when impulsing
GFVAR(0, impulselimit, 0, 0, 1e16f); // maximum impulse speed
GFVAR(0, impulseboost, 0, 1, 1e16f); // thrust modifier
GFVAR(0, impulseboostz, -1, 0, 1); // thrust z modifier
GFVAR(0, impulsedash, 0, 1.1f, 1e16f); // dashing modifier
GFVAR(0, impulsejump, 0, 1.1f, 1e16f); // jump modifier
GFVAR(0, impulsemelee, 0, 0.5f, 1e16f); // melee modifier
GFVAR(0, impulseparkour, 0, 1.1f, 1e16f); // parkour modifier
GFVAR(0, impulseparkournorm, 0, 0.5f, 1e16f); // minimum parkour surface z normal
GVAR(0, impulseallowed, 0, 3, 3); // impulse allowed; 0 = off, 1 = dash/boost only, 2 = dash/boost and sprint, 3 = all mechanics including parkour
GVAR(0, impulsestyle, 0, 1, 3); // impulse style; 0 = off, 1 = touch and count, 2 = count only, 3 = freestyle
GVAR(0, impulsecount, 0, 5, INT_MAX-1); // number of impulse actions per air transit
GVAR(0, impulsedelay, 0, 125, INT_MAX-1); // minimum time between boosts
GVAR(0, impulsedashdelay, 0, 1000, INT_MAX-1); // minimum time between dashes
GVAR(0, impulsejetdelay, 0, 250, INT_MAX-1); // minimum time between jetpack
GVAR(0, impulseslide, 0, 500, INT_MAX-1); // minimum time before floor friction kicks back in
GVAR(0, impulsemeter, 0, 20000, INT_MAX-1); // impulse dash length; 0 = unlimited, anything else = timer
GVAR(0, impulsecost, 0, 4000, INT_MAX-1); // cost of impulse jump
GVAR(0, impulseskate, 0, 1000, INT_MAX-1); // length of time a run along a wall can last
GFVAR(0, impulsesprint, 0, 0, 1e16f); // sprinting impulse meter depletion
GFVAR(0, impulsejetpack, 0, 1.5f, 1e16f); // jetpack impulse meter depletion
GFVAR(0, impulseregen, 0, 4.0, 1e16f); // impulse regen multiplier
GFVAR(0, impulseregencrouch, 0, 2, 1e16f); // impulse regen crouch modifier
GFVAR(0, impulseregensprint, 0, 0.75f, 1e16f); // impulse regen sprinting modifier
GFVAR(0, impulseregenjetpack, 0, 1.5f, 1e16f); // impulse regen jetpack modifier
GFVAR(0, impulseregenmove, 0, 1, 1e16f); // impulse regen moving modifier
GFVAR(0, impulseregeninair, 0, 0.75f, 1e16f); // impulse regen in-air modifier
GVAR(0, impulseregendelay, 0, 250, INT_MAX-1); // delay before impulse regens
GVAR(0, impulseregenjetdelay, -1, -1, INT_MAX-1); // delay before impulse regens after jetting, -1 = must touch ground

GFVAR(0, stillspread, 0, 0, 1e16f);
GFVAR(0, movespread, 0, 1, 1e16f);
GFVAR(0, inairspread, 0, 1, 1e16f);
GFVAR(0, impulsespread, 0, 1, 1e16f);

GVAR(0, zoomlock, 0, 1, 3); // 0 = free, 1 = must be on floor, 2 = also must not be moving, 3 = also must be on flat floor
GVAR(0, zoomlocktime, 0, 500, INT_MAX-1); // time before zoomlock kicks in when in the air
GVAR(0, zoomlimit, 1, 10, 150);
GVAR(0, zoomtime, 1, 100, INT_MAX-1);

GFVAR(0, normalscale, 0, 1, 1e16f);
GFVAR(0, limitedscale, 0, 0.75f, 1e16f);
GFVAR(0, damagescale, 0, 1, 1e16f);
GVAR(0, criticalchance, 0, 100, INT_MAX-1);

GFVAR(0, hitpushscale, 0, 1, 1e16f);
GFVAR(0, hitslowscale, 0, 1, 1e16f);
GFVAR(0, deadpushscale, 0, 2, 1e16f);
GFVAR(0, wavepushscale, 0, 1, 1e16f);
GFVAR(0, waveslowscale, 0, 0.5f, 1e16f);
GFVAR(0, kickpushscale, 0, 1, 1e16f);
GFVAR(0, kickpushcrouch, 0, 0, 1e16f);
GFVAR(0, kickpushsway, 0, 0.0125f, 1e16f);
GFVAR(0, kickpushzoom, 0, 0.125f, 1e16f);

GVAR(0, assistkilldelay, 0, 5000, INT_MAX-1);
GVAR(0, multikilldelay, 0, 5000, INT_MAX-1);
GVAR(0, spreecount, 0, 5, INT_MAX-1);
GVAR(0, dominatecount, 0, 5, INT_MAX-1);

GVAR(0, alloweastereggs, 0, 1, 1); // 0 = off, 1 = on
GVAR(0, returningfire, 0, 0, 1); // 0 = off, 1 = on
