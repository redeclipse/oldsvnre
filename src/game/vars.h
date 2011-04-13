GVAR(IDF_ADMIN, serverdebug, 0, 0, 3);
GVAR(IDF_ADMIN, serverclients, 1, 16, MAXCLIENTS);
GVAR(IDF_ADMIN, serveropen, 0, 3, 3);
GSVAR(IDF_ADMIN, serverdesc, "");
GSVAR(IDF_ADMIN, servermotd, "");
GVAR(IDF_ADMIN, automaster, 0, 0, 1);

GVAR(IDF_ADMIN, autospectate, 0, 1, 1); // auto spectate if idle, 1 = auto spectate when remaining dead for autospecdelay
GVAR(IDF_ADMIN, autospecdelay, 0, 60000, VAR_MAX);

GVAR(IDF_ADMIN, resetbansonend, 0, 1, 2); // reset bans on end (1: just when empty, 2: when matches end)
GVAR(IDF_ADMIN, resetvarsonend, 0, 1, 2); // reset variables on end (1: just when empty, 2: when matches end)
GVAR(IDF_ADMIN, resetmmonend, 0, 2, 2); // reset mastermode on end (1: just when empty, 2: when matches end)

GVARF(IDF_ADMIN, gamespeed, 1, 100, 10000, timescale = sv_gamespeed, timescale = gamespeed);
GVARF(IDF_ADMIN, gamepaused, 0, 0, 1, paused = sv_gamepaused, paused = gamepaused);

GSVAR(IDF_ADMIN, defaultmap, "");
GVAR(IDF_ADMIN, defaultmode, G_START, G_DEATHMATCH, G_MAX-1);
GVAR(IDF_ADMIN, defaultmuts, 0, 0, G_M_ALL);
GVAR(IDF_ADMIN, rotatemode, 0, 1, 1);
GVARF(IDF_ADMIN, rotatemodefilter, 0, G_LIMIT, G_ALL, sv_rotatemodefilter &= ~G_NEVER, rotatemodefilter &= ~G_NEVER); // modes not in this array are filtered out
GVAR(IDF_ADMIN, rotatemuts, 0, 3, VAR_MAX); // any more than one decreases the chances of it picking
GVAR(IDF_ADMIN, rotatemutsfilter, 0, G_M_FILTER, G_M_ALL); // mutators not in this array are filtered out
GVAR(IDF_ADMIN, campaignplayers, 1, 4, MAXPLAYERS);

GSVAR(IDF_ADMIN, allowmaps, "alphacampaign bath center darkness dawn deadsimple deathtrap deli depot dropzone dutility echo facility forge foundation futuresport ghost hinder keystone lab linear longestyard mist nova panic processing spacetech stone testchamber tranquility tribal ubik venus warp wet");

GSVAR(IDF_ADMIN, mainmaps, "bath center darkness deadsimple deathtrap deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear longestyard mist nova panic processing spacetech stone tranquility tribal ubik venus warp wet");
GSVAR(IDF_ADMIN, capturemaps, "bath center darkness deadsimple deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear mist nova panic stone tranquility tribal venus warp wet");
GSVAR(IDF_ADMIN, defendmaps, "bath center darkness deadsimple deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear mist nova panic processing stone tranquility tribal venus warp wet");
GSVAR(IDF_ADMIN, bombermaps, "bath center deadsimple deli depot dropzone echo forge foundation futuresport tranquility venus");
GSVAR(IDF_ADMIN, holdmaps, "bath center darkness deadsimple deli depot dropzone dutility echo facility forge foundation futuresport ghost keystone lab linear mist nova panic processing stone tranquility tribal venus warp wet");
GSVAR(IDF_ADMIN, trialmaps, "testchamber");
GSVAR(IDF_ADMIN, campaignmaps, "alphacampaign");

GSVAR(IDF_ADMIN, duelmaps, "bath darkness deadsimple dropzone dutility echo longestyard panic");
GSVAR(IDF_ADMIN, jetpackmaps, "alphacampaign center darkness dawn deadsimple deathtrap deli depot dropzone dutility echo forge foundation futuresport ghost keystone linear longestyard mist nova spacetech testchamber tranquility tribal ubik venus warp");

GSVAR(IDF_ADMIN, smallmaps, "bath darkness deadsimple deli dropzone dutility echo ghost linear longestyard panic stone wet");
GSVAR(IDF_ADMIN, mediummaps, "center darkness deadsimple deathtrap deli dropzone echo facility forge foundation futuresport ghost keystone lab linear mist nova panic processing spacetech stone tranquility tribal ubik venus warp wet");
GSVAR(IDF_ADMIN, largemaps, "center dawn deadsimple deathtrap deli depot facility forge foundation futuresport ghost lab linear mist nova processing spacetech tranquility tribal ubik venus warp");


GVAR(IDF_ADMIN, modelock, 0, 3, 5); // 0 = off, 1 = master only (+1 admin only), 3 = master can only set limited mode and higher (+1 admin), 5 = no mode selection
GVAR(IDF_ADMIN, modelockfilter, 0, G_LIMIT, G_ALL);
GVAR(IDF_ADMIN, mutslockfilter, 0, G_M_ALL, G_M_ALL);

GVAR(IDF_ADMIN, maprotate, 0, 2, 2); // 0 = off, 1 = sequence, 2 = random
GVAR(IDF_ADMIN, mapsfilter, 0, 2, 2); // 0 = off, 1 = filter based on mutators, 2 = also filter based on players
GVAR(IDF_ADMIN, mapslock, 0, 3, 5); // 0 = off, 1 = master can select non-allow maps (+1 admin), 3 = master can select non-rotation maps (+1 admin), 5 = no map selection
GVAR(IDF_ADMIN, varslock, 0, 1, 3); // 0 = off, 1 = master, 2 = admin only, 3 = nobody
GVAR(IDF_ADMIN, votelock, 0, 1, 5); // 0 = off, 1 = master can select same game (+1 admin), 3 = master only can vote (+1 admin), 5 = no voting
GVAR(IDF_ADMIN, votewait, 0, 2500, VAR_MAX);

GVAR(IDF_ADMIN, smallmapmax, 0, 6, VAR_MAX); // maximum number of players for a small map
GVAR(IDF_ADMIN, mediummapmax, 0, 12, VAR_MAX); // maximum number of players for a medium map

namespace server { extern void resetgamevars(bool flush); }
GICOMMAND(0, resetvars, "", (), server::resetgamevars(true), return);
GICOMMAND(IDF_ADMIN, resetconfig, "", (), rehash(true), );

GFVAR(0, maxalive, 0, 0, FVAR_MAX); // only allow this*numplayers to be alive at once
GVAR(0, maxalivequeue, 0, 1, 1); // if number of players exceeds this amount, use a queue system
GVAR(0, maxaliveminimum, 2, 8, VAR_MAX); // kicks in if numplayers >= this
GFVAR(0, maxalivethreshold, 0, 0.5f, FVAR_MAX); // .. or this percentage of players

GVAR(0, maxcarry, 1, 2, WEAP_CARRY);
GVAR(0, spawnrotate, 0, 4, VAR_MAX); // 0 = let client decide, 1 = sequence, 2+ = random
GVAR(0, spawnweapon, 0, WEAP_PISTOL, WEAP_MAX-1);
GVAR(0, instaweapon, 0, WEAP_RIFLE, WEAP_MAX-1);
GVAR(0, trialweapon, 0, WEAP_MELEE, WEAP_MAX-1);
GVAR(0, spawngrenades, 0, 0, 2); // 0 = never, 1 = all but insta/trial, 2 = always
GVAR(0, spawndelay, 0, 5000, VAR_MAX); // delay before spawning in most modes
GVAR(0, instadelay, 0, 3000, VAR_MAX); // .. in instagib/arena matches
GVAR(0, trialdelay, 0, 500, VAR_MAX); // .. in time trial matches
GVAR(0, bomberdelay, 0, 3000, VAR_MAX); // delay before spawning in bomber
GVAR(0, spawnprotect, 0, 3000, VAR_MAX); // delay before damage can be dealt to spawning player
GVAR(0, duelprotect, 0, 5000, VAR_MAX); // .. in duel/survivor matches
GVAR(0, instaprotect, 0, 3000, VAR_MAX); // .. in instagib matches

GVAR(0, spawnhealth, 0, 100, VAR_MAX);
GVAR(0, extrahealth, 0, 150, VAR_MAX);
GVAR(0, maxhealth, 0, 200, VAR_MAX);

GFVAR(0, actorscale, FVAR_NONZERO, 1, FVAR_MAX);
GFVAR(0, maxresizescale, 1, 2, FVAR_MAX);
GFVAR(0, minresizescale, FVAR_NONZERO, 0.5f, 1);

GVAR(0, burntime, 0, 5500, VAR_MAX);
GVAR(0, burndelay, 0, 1000, VAR_MAX);
GVAR(0, burndamage, 0, 5, VAR_MAX);
GVAR(0, bleedtime, 0, 5500, VAR_MAX);
GVAR(0, bleeddelay, 0, 1000, VAR_MAX);
GVAR(0, bleeddamage, 0, 5, VAR_MAX);

GVAR(0, regendelay, 0, 3000, VAR_MAX); // regen after no damage for this long
GVAR(0, regenguard, 0, 1000, VAR_MAX); // regen this often when guarding an affinity
GVAR(0, regentime, 0, 1000, VAR_MAX); // regen this often when regenerating normally
GVAR(0, regenhealth, 0, 5, VAR_MAX); // regen this amount each regen
GVAR(0, regenextra, 0, 2, VAR_MAX); // add this to regen when influenced by affinity
GVAR(0, regenaffinity, 0, 1, 2); // 0 = off, 1 = only guarding, 2 = also while carrying

GVAR(0, kamikaze, 0, 1, 3); // 0 = never, 1 = holding grenade, 2 = have grenade, 3 = always
GVAR(0, itemsallowed, 0, 2, 2); // 0 = never, 1 = all but limited, 2 = always
GVAR(0, itemspawntime, 1, 30000, VAR_MAX); // when items respawn
GVAR(0, itemspawndelay, 0, 1000, VAR_MAX); // after map start items first spawn
GVAR(0, itemspawnstyle, 0, 1, 3); // 0 = all at once, 1 = staggered, 2 = random, 3 = randomise between both
GFVAR(0, itemthreshold, 0, 2, FVAR_MAX); // if numitems/(players*maxcarry) is less than this, spawn one of this type
GVAR(0, itemcollide, 0, BOUNCE_GEOM, VAR_MAX);
GVAR(0, itemextinguish, 0, 6, 7);
GFVAR(0, itemelasticity, FVAR_MIN, 0.4f, FVAR_MAX);
GFVAR(0, itemrelativity, FVAR_MIN, 1, FVAR_MAX);
GFVAR(0, itemwaterfric, 0, 1.75f, FVAR_MAX);
GFVAR(0, itemweight, FVAR_MIN, 150, FVAR_MAX);
GFVAR(0, itemminspeed, 0, 0, FVAR_MAX);
GFVAR(0, itemrepulsion, 0, 8, FVAR_MAX);
GFVAR(0, itemrepelspeed, 0, 25, FVAR_MAX);

GVAR(0, timelimit, 0, 10, VAR_MAX);
GVAR(0, triallimit, 0, 60000, VAR_MAX);
GVAR(0, intermlimit, 0, 15000, VAR_MAX); // .. before vote menu comes up
GVAR(0, votelimit, 0, 45000, VAR_MAX); // .. before vote passes by default
GVAR(0, duelreset, 0, 1, 1); // reset winner in duel
GVAR(0, duelclear, 0, 1, 1); // clear items in duel
GVAR(0, duellimit, 0, 5000, VAR_MAX); // .. before duel goes to next round

GVAR(0, selfdamage, 0, 1, 1); // 0 = off, 1 = either hurt self or use teamdamage rules
GVAR(0, trialdamage, 0, 1, 1); // 0 = off, 1 = allow damage in time-trial
GVAR(0, teamdamage, 0, 1, 2); // 0 = off, 1 = non-bots damage team, 2 = all players damage team
GVAR(0, teambalance, 0, 1, 3); // 0 = off, 1 = by number then rank, 2 = by rank then number, 3 = humans vs. ai
GVAR(0, teampersist, 0, 1, 2); // 0 = off, 1 = only attempt, 2 = forced
GVAR(0, pointlimit, 0, 0, VAR_MAX); // finish when score is this or more

GVAR(0, capturelimit, 0, 0, VAR_MAX); // finish when score is this or more
GVAR(0, captureresetdelay, 0, 30000, VAR_MAX);
GVAR(0, capturepickupdelay, -1, 5000, VAR_MAX);
GVAR(0, capturecollide, 0, BOUNCE_GEOM, VAR_MAX);
GVAR(0, captureextinguish, 0, 6, 7);
GFVAR(0, capturerelativity, 0, 0.25f, FVAR_MAX);
GFVAR(0, captureelasticity, FVAR_MIN, 0.35f, FVAR_MAX);
GFVAR(0, capturewaterfric, FVAR_MIN, 1.75f, FVAR_MAX);
GFVAR(0, captureweight, FVAR_MIN, 100, FVAR_MAX);
GFVAR(0, captureminspeed, 0, 0, FVAR_MAX);
GFVAR(0, capturethreshold, 0, 0, FVAR_MAX); // if someone 'warps' more than this distance, auto-drop

GVAR(0, defendlimit, 0, 300, VAR_MAX); // finish when score is this or more
GVAR(0, defendpoints, 0, 1, VAR_MAX); // points added to score
GVAR(0, defendinterval, 0, 100, VAR_MAX);
GVAR(0, defendoccupy, 0, 100, VAR_MAX); // points needed to occupy
GVAR(0, defendflags, 0, 3, 3); // 0 = init all (neutral), 1 = init neutral and team only, 2 = init team only, 3 = init all (team + neutral + converted)

GVAR(0, bomberlimit, 0, 0, VAR_MAX); // finish when score is this or more (non-hold)
GVAR(0, bomberholdlimit, 0, 0, VAR_MAX); // finish when score is this or more (hold)
GVAR(0, bomberresetdelay, 0, 15000, VAR_MAX);
GVAR(0, bomberpickupdelay, -1, 5000, VAR_MAX);
GVAR(0, bombercarrytime, 0, 15000, VAR_MAX);
GVAR(0, bomberholdpoints, 0, 1, VAR_MAX); // points added to score
GVAR(0, bomberholdpenalty, 0, 10, VAR_MAX); // penalty for holding too long
GVAR(0, bomberholdinterval, 0, 1000, VAR_MAX);
GVAR(0, bomberlockondelay, 0, 250, VAR_MAX);
GVAR(0, bomberreset, 0, 0, 2); // 0 = off, 1 = kill winners, 2 = kill everyone
GFVAR(0, bomberspeed, 0, 250, FVAR_MAX);
GFVAR(0, bomberdelta, 0, 1000, FVAR_MAX);
GVAR(0, bombercollide, 0, BOUNCE_GEOM, VAR_MAX);
GVAR(0, bomberextinguish, 0, 6, 7);
GFVAR(0, bomberrelativity, 0, 0.25f, FVAR_MAX);
GFVAR(0, bomberelasticity, FVAR_MIN, 0.65f, FVAR_MAX);
GFVAR(0, bomberwaterfric, FVAR_MIN, 1.75f, FVAR_MAX);
GFVAR(0, bomberweight, FVAR_MIN, 150, FVAR_MAX);
GFVAR(0, bomberminspeed, 0, 50, FVAR_MAX);
GFVAR(0, bomberthreshold, 0, 0, FVAR_MAX); // if someone 'warps' more than this distance, auto-drop

GVAR(IDF_ADMIN, airefresh, 0, 1000, VAR_MAX);
GVAR(0, skillmin, 1, 50, 101);
GVAR(0, skillmax, 1, 75, 101);
GVAR(0, botbalance, -1, -1, VAR_MAX); // -1 = always use numplayers, 0 = don't balance, 1 or more = fill only with this*numteams
GVAR(0, botlimit, 0, 16, VAR_MAX);
GFVAR(0, botspeed, 0, 1, FVAR_MAX);
GFVAR(0, botscale, FVAR_NONZERO, 1, FVAR_MAX);
GVAR(0, enemybalance, 0, 1, 3);
GVAR(0, enemyspawntime, 1, 30000, VAR_MAX); // when enemies respawn
GVAR(0, enemyspawndelay, 0, 1000, VAR_MAX); // after map start enemies first spawn
GVAR(0, enemyspawnstyle, 0, 1, 3); // 0 = all at once, 1 = staggered, 2 = random, 3 = randomise between both
GFVAR(0, enemyspeed, 0, 1, FVAR_MAX);
GFVAR(0, enemyscale, FVAR_NONZERO, 1, FVAR_MAX);
GFVAR(0, enemystrength, FVAR_NONZERO, 1, FVAR_MAX); // scale enemy health values by this much

GFVAR(0, forcegravity, -1, -1, FVAR_MAX);
GFVAR(0, forceliquidspeed, -1, -1, 1);
GFVAR(0, forceliquidcurb, -1, -1, FVAR_MAX);
GFVAR(0, forcefloorcurb, -1, -1, FVAR_MAX);
GFVAR(0, forceaircurb, -1, -1, FVAR_MAX);
GFVAR(0, forceslidecurb, -1, -1, FVAR_MAX);

GFVAR(0, movespeed, FVAR_NONZERO, 100, FVAR_MAX); // speed
GFVAR(0, movecrawl, 0, 0.6f, FVAR_MAX); // crawl modifier
GFVAR(0, movesprint, FVAR_NONZERO, 1.6f, FVAR_MAX); // sprinting modifier
GFVAR(0, movejetpack, FVAR_NONZERO, 1.6f, FVAR_MAX); // jetpack modifier
GFVAR(0, movepowerjump, FVAR_NONZERO, 4.6f, FVAR_MAX); // powerjump modifier
GFVAR(0, movestraight, FVAR_NONZERO, 1.2f, FVAR_MAX); // non-strafe modifier
GFVAR(0, movestrafe, FVAR_NONZERO, 1, FVAR_MAX); // strafe modifier
GFVAR(0, moveinair, FVAR_NONZERO, 0.9f, FVAR_MAX); // in-air modifier
GFVAR(0, movestepup, FVAR_NONZERO, 0.95f, FVAR_MAX); // step-up modifier
GFVAR(0, movestepdown, FVAR_NONZERO, 1.15f, FVAR_MAX); // step-down modifier

GFVAR(0, jumpspeed, FVAR_NONZERO, 110, FVAR_MAX); // extra velocity to add when jumping
GFVAR(0, impulsespeed, FVAR_NONZERO, 90, FVAR_MAX); // extra velocity to add when impulsing

GFVAR(0, impulselimit, 0, 0, FVAR_MAX); // maximum impulse speed
GFVAR(0, impulseboost, 0, 1, FVAR_MAX); // thrust modifier
GFVAR(0, impulsedash, 0, 1.2f, FVAR_MAX); // dashing/powerslide modifier
GFVAR(0, impulsejump, 0, 1.1f, FVAR_MAX); // jump modifier
GFVAR(0, impulsemelee, 0, 0.5f, FVAR_MAX); // melee modifier
GFVAR(0, impulseparkour, 0, 1, FVAR_MAX); // parkour modifier
GFVAR(0, impulseparkourkick, 0, 1.3f, FVAR_MAX); // parkour kick modifier
GFVAR(0, impulseparkournorm, 0, 0.5f, FVAR_MAX); // minimum parkour surface z normal
GVAR(0, impulseallowed, 0, 3, 3); // impulse allowed; 0 = off, 1 = dash/boost only, 2 = dash/boost and sprint, 3 = all mechanics including parkour
GVAR(0, impulsestyle, 0, 1, 3); // impulse style; 0 = off, 1 = touch and count, 2 = count only, 3 = freestyle
GVAR(0, impulsecount, 0, 6, VAR_MAX); // number of impulse actions per air transit
GVAR(0, impulseslip, 0, 300, VAR_MAX); // time before floor friction kicks back in
GVAR(0, impulseslide, 0, 750, VAR_MAX); // time before powerslides end
GVAR(0, impulsedelay, 0, 250, VAR_MAX); // minimum time between boosts
GVAR(0, impulsedashdelay, 0, 750, VAR_MAX); // minimum time between dashes/powerslides
GVAR(0, impulsejetdelay, 0, 250, VAR_MAX); // minimum time between jetpack
GVAR(0, impulsemeter, 0, 20000, VAR_MAX); // impulse dash length; 0 = unlimited, anything else = timer
GVAR(0, impulsecost, 0, 4000, VAR_MAX); // cost of impulse jump
GVAR(0, impulseskate, 0, 1000, VAR_MAX); // length of time a run along a wall can last
GFVAR(0, impulsesprint, 0, 0, FVAR_MAX); // sprinting impulse meter depletion
GFVAR(0, impulsejetpack, 0, 1.5f, FVAR_MAX); // jetpack impulse meter depletion
GFVAR(0, impulsepowerup, 0, 5.f, FVAR_MAX); // power jump impulse meter charge rate
GFVAR(0, impulsepowerjump, 0, 15.f, FVAR_MAX); // power jump impulse meter depletion
GFVAR(0, impulseregen, 0, 4.f, FVAR_MAX); // impulse regen multiplier
GFVAR(0, impulseregencrouch, 0, 2, FVAR_MAX); // impulse regen crouch modifier
GFVAR(0, impulseregensprint, 0, 0.75f, FVAR_MAX); // impulse regen sprinting modifier
GFVAR(0, impulseregenjetpack, 0, 1.5f, FVAR_MAX); // impulse regen jetpack modifier
GFVAR(0, impulseregenmove, 0, 1, FVAR_MAX); // impulse regen moving modifier
GFVAR(0, impulseregeninair, 0, 0.75f, FVAR_MAX); // impulse regen in-air modifier
GVAR(0, impulseregendelay, 0, 250, VAR_MAX); // delay before impulse regens
GVAR(0, impulseregenjetdelay, -1, -1, VAR_MAX); // delay before impulse regens after jetting, -1 = must touch ground

GFVAR(0, stillspread, 0, 0, FVAR_MAX);
GFVAR(0, movespread, 0, 1, FVAR_MAX);
GFVAR(0, inairspread, 0, 1, FVAR_MAX);
GFVAR(0, impulsespread, 0, 1, FVAR_MAX);

GVAR(0, zoomlock, 0, 1, 4); // 0 = free, 1 = must be on floor, 2 = also must not be moving, 3 = also must be on flat floor, 4 = must also be crouched
GVAR(0, zoomlocktime, 0, 500, VAR_MAX); // time before zoomlock kicks in when in the air
GVAR(0, zoomlimit, 1, 10, 150);
GVAR(0, zoomtime, 1, 100, VAR_MAX);

GFVAR(0, normalscale, 0, 1, FVAR_MAX);
GFVAR(0, limitedscale, 0, 0.75f, FVAR_MAX);
GFVAR(0, damagescale, 0, 1, FVAR_MAX);
GVAR(0, criticalchance, 0, 100, VAR_MAX);

GFVAR(0, hitpushscale, 0, 1, FVAR_MAX);
GFVAR(0, hitslowscale, 0, 1, FVAR_MAX);
GFVAR(0, deadpushscale, 0, 2, FVAR_MAX);
GFVAR(0, wavepushscale, 0, 1, FVAR_MAX);
GFVAR(0, waveslowscale, 0, 0.5f, FVAR_MAX);
GFVAR(0, kickpushscale, 0, 1, FVAR_MAX);
GFVAR(0, kickpushcrouch, 0, 0, FVAR_MAX);
GFVAR(0, kickpushsway, 0, 0.0125f, FVAR_MAX);
GFVAR(0, kickpushzoom, 0, 0.125f, FVAR_MAX);

GVAR(0, assistkilldelay, 0, 5000, VAR_MAX);
GVAR(0, multikilldelay, 0, 5000, VAR_MAX);
GVAR(0, spreecount, 0, 5, VAR_MAX);
GVAR(0, dominatecount, 0, 5, VAR_MAX);

GVAR(0, alloweastereggs, 0, 1, 1); // 0 = off, 1 = on
GVAR(0, returningfire, 0, 0, 1); // 0 = off, 1 = on
