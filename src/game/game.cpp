#define GAMEWORLD 1
#include "game.h"
namespace game
{
    int nextmode = G_EDITMODE, nextmuts = 0, gamemode = G_EDITMODE, mutators = 0, maptime = 0, timeremaining = 0,
        lastcamera = 0, lasttvcam = 0, lasttvchg = 0, lastzoom = 0, lastmousetype = 0, liquidchan = -1;
    bool intermission = false, prevzoom = false, zooming = false;
    float swayfade = 0, swayspeed = 0, swaydist = 0, bobfade = 0, bobdist = 0;
    vec swaydir(0, 0, 0), swaypush(0, 0, 0);

    string clientmap = "";

    gameent *player1 = new gameent(), *focus = player1;
    avatarent avatarmodel;
    vector<gameent *> players, waiting;
    vector<cament *> cameras;

    VAR(IDF_WORLD, numplayers, 0, 4, MAXCLIENTS);
    FVAR(IDF_WORLD, illumlevel, 0, 0, 2);
    VAR(IDF_WORLD, illumradius, 0, 0, VAR_MAX);
    SVAR(IDF_WORLD, obitlava, "");
    SVAR(IDF_WORLD, obitwater, "");
    SVAR(IDF_WORLD, obitdeath, "");

    void stopmapmusic()
    {
        if(connected() && maptime > 0 && !intermission) musicdone(true);
    }
    VARF(IDF_PERSIST, musictype, 0, 1, 5, stopmapmusic()); // 0 = no in-game music, 1 = map music (or random if none), 2 = always random, 3 = map music (silence if none), 4-5 = same as 1-2 but pick new tracks when done
    VARF(IDF_PERSIST, musicedit, 0, 0, 5, stopmapmusic()); // same as above for editmode
    SVARF(IDF_PERSIST, musicdir, "sounds/music", stopmapmusic());
    SVARF(IDF_WORLD, mapmusic, "", stopmapmusic());

    VAR(IDF_PERSIST, mouseinvert, 0, 0, 1);
    VAR(IDF_PERSIST, mouseabsolute, 0, 0, 1);
    VAR(IDF_PERSIST, mousetype, 0, 0, 2);
    VAR(IDF_PERSIST, mousedeadzone, 0, 10, 100);
    VAR(IDF_PERSIST, mousepanspeed, 1, 50, VAR_MAX);

    VAR(IDF_PERSIST, thirdperson, 0, 0, 1);
    VAR(IDF_PERSIST, thirdpersonfollow, 0, 1, 1);
    VAR(IDF_PERSIST, thirdpersonaiming, 0, 1, 1);
    VAR(IDF_PERSIST, dynlighteffects, 0, 2, 2);
    FVAR(IDF_PERSIST, playerblend, 0, 1, 1);

    VAR(IDF_PERSIST, thirdpersonmodel, 0, 1, 1);
    VAR(IDF_PERSIST, thirdpersonfov, 90, 120, 150);
    FVAR(IDF_PERSIST, thirdpersonblend, 0, 1, 1);
    FVAR(IDF_PERSIST, thirdpersondist, 0, 25, 1000);

    VAR(0, follow, 0, 0, VAR_MAX);
    FVAR(IDF_PERSIST, followblend, 0, 1, 1);
    FVAR(IDF_PERSIST, followdist, 0, 25, 1000);

    VAR(IDF_PERSIST, firstpersonmodel, 0, 1, 1);
    VAR(IDF_PERSIST, firstpersonfov, 90, 100, 150);
    FVAR(IDF_PERSIST, firstpersonblend, 0, 1, 1);

    VAR(IDF_PERSIST, firstpersonsway, 0, 1, 1);
    FVAR(IDF_PERSIST, firstpersonswaystep, 1, 28.f, 1000);
    FVAR(IDF_PERSIST, firstpersonswayside, 0, 0.05f, 10);
    FVAR(IDF_PERSIST, firstpersonswayup, 0, 0.06f, 10);

    VAR(IDF_PERSIST, firstpersonbob, 0, 1, 1);
    FVAR(IDF_PERSIST, firstpersonbobstep, 1, 28.f, 1000);
    FVAR(IDF_PERSIST, firstpersonbobroll, 0, 0.3f, 10);
    FVAR(IDF_PERSIST, firstpersonbobside, 0, 0.6f, 10);
    FVAR(IDF_PERSIST, firstpersonbobup, 0, 0.6f, 10);
    FVAR(IDF_PERSIST, firstpersonbobfocusmindist, 0, 64, 10000);
    FVAR(IDF_PERSIST, firstpersonbobfocusmaxdist, 0, 256, 10000);
    FVAR(IDF_PERSIST, firstpersonbobfocus, 0, 0.5f, 1);

    VAR(IDF_PERSIST, editfov, 1, 120, 179);
    VAR(IDF_PERSIST, specfov, 1, 120, 179);

    VARF(IDF_PERSIST, specmode, 0, 1, 1, follow = 0); // 0 = float, 1 = tv
    VARF(IDF_PERSIST, waitmode, 0, 2, 2, follow = 0); // 0 = float, 1 = tv in duel/survivor, 2 = tv always

    VAR(IDF_PERSIST, spectvtime, 1000, 10000, VAR_MAX);
    VAR(IDF_PERSIST, spectvmintime, 1000, 5000, VAR_MAX);
    VAR(IDF_PERSIST, spectvmaxtime, 0, 20000, VAR_MAX);
    FVAR(IDF_PERSIST, spectvrotate, FVAR_MIN, 45, FVAR_MAX); // rotate style, < 0 = absolute angle, 0 = scaled, > 0 = scaled with max angle
    FVAR(IDF_PERSIST, spectvyawspeed, 0, 1, 1000);
    FVAR(IDF_PERSIST, spectvpitchspeed, 0, 1, 1000);

    VAR(IDF_PERSIST, deathcamstyle, 0, 2, 2); // 0 = no follow, 1 = follow attacker, 2 = follow self
    FVAR(IDF_PERSIST, deathcamspeed, 0, 2.f, 1000);

    FVAR(IDF_PERSIST, sensitivity, 1e-4f, 10, 10000);
    FVAR(IDF_PERSIST, sensitivityscale, 1e-4f, 100, 10000);
    FVAR(IDF_PERSIST, yawsensitivity, 1e-4f, 1, 10000);
    FVAR(IDF_PERSIST, pitchsensitivity, 1e-4f, 1, 10000);
    FVAR(IDF_PERSIST, mousesensitivity, 1e-4f, 1, 10000);
    FVAR(IDF_PERSIST, zoomsensitivity, 0, 0.65f, 1000);
    FVAR(IDF_PERSIST, followsensitivity, 0, 2, 1000);

    VAR(IDF_PERSIST, zoommousetype, 0, 0, 2);
    VAR(IDF_PERSIST, zoommousedeadzone, 0, 25, 100);
    VAR(IDF_PERSIST, zoommousepanspeed, 1, 10, VAR_MAX);
    VAR(IDF_PERSIST, zoomfov, 1, 10, 150);

    VARF(IDF_PERSIST, zoomlevel, 1, 4, 10, checkzoom());
    VAR(IDF_PERSIST, zoomlevels, 1, 5, 10);
    VAR(IDF_PERSIST, zoomdefault, 0, 0, 10); // 0 = last used, else defines default level
    VAR(IDF_PERSIST, zoomscroll, 0, 0, 1); // 0 = stop at min/max, 1 = go to opposite end

    VAR(IDF_PERSIST, aboveheadnames, 0, 1, 1);
    VAR(IDF_PERSIST, aboveheadstatus, 0, 1, 1);
    VAR(IDF_PERSIST, aboveheadteam, 0, 1, 2);
    VAR(IDF_PERSIST, aboveheaddamage, 0, 0, 1);
    VAR(IDF_PERSIST, aboveheadicons, 0, 5, 7);
    FVAR(IDF_PERSIST, aboveheadblend, 0.f, 1, 1.f);
    FVAR(IDF_PERSIST, aboveheadnamesize, 0, 2, 1000);
    FVAR(IDF_PERSIST, aboveheadstatussize, 0, 2, 1000);
    FVAR(IDF_PERSIST, aboveheadiconsize, 0, 4, 1000);
    FVAR(IDF_PERSIST, aboveitemiconsize, 0, 4, 1000);

    FVAR(IDF_PERSIST, aboveheadsmooth, 0, 0.5f, 1);
    VAR(IDF_PERSIST, aboveheadsmoothmillis, 1, 200, 10000);

    VAR(IDF_PERSIST, eventiconfade, 500, 5000, VAR_MAX);
    VAR(IDF_PERSIST, eventiconshort, 500, 3000, VAR_MAX);
    VAR(IDF_PERSIST, eventiconcrit, 500, 2000, VAR_MAX);

    VAR(IDF_PERSIST, showobituaries, 0, 4, 5); // 0 = off, 1 = only me, 2 = 1 + announcements, 3 = 2 + but dying bots, 4 = 3 + but bot vs bot, 5 = all
    VAR(IDF_PERSIST, showobitdists, 0, 1, 1);
    VAR(IDF_PERSIST, obitannounce, 0, 1, 2); // 0 = off, 1 = only focus, 2 = everyone
    VAR(IDF_PERSIST, showplayerinfo, 0, 1, 1); // 0 = none, 1 = show events

    VAR(IDF_PERSIST, damagemergedelay, 0, 75, VAR_MAX);
    VAR(IDF_PERSIST, damagemergeburn, 0, 250, VAR_MAX);
    VAR(IDF_PERSIST, damagemergebleed, 0, 250, VAR_MAX);
    VAR(IDF_PERSIST, playdamagetones, 0, 1, 3);
    VAR(IDF_PERSIST, playcrittones, 0, 2, 3);
    VAR(IDF_PERSIST, playreloadnotify, 0, 3, 15);

    VAR(IDF_PERSIST, ragdolls, 0, 1, 1);
    FVAR(IDF_PERSIST, bloodscale, 0, 1, 1000);
    VAR(IDF_PERSIST, bloodfade, 1, 3000, VAR_MAX);
    VAR(IDF_PERSIST, bloodsize, 1, 50, 1000);
    VAR(IDF_PERSIST, bloodsparks, 0, 0, 1);
    FVAR(IDF_PERSIST, debrisscale, 0, 1, 1000);
    VAR(IDF_PERSIST, debrisfade, 1, 5000, VAR_MAX);
    FVAR(IDF_PERSIST, gibscale, 0, 1, 1000);
    VAR(IDF_PERSIST, gibfade, 1, 5000, VAR_MAX);
    VAR(IDF_PERSIST, burnfade, 100, 200, VAR_MAX);
    FVAR(IDF_PERSIST, burnblend, 0.25f, 0.5f, 1);
    FVAR(IDF_PERSIST, impulsescale, 0, 1, 1000);
    VAR(IDF_PERSIST, impulsefade, 0, 200, VAR_MAX);
    VAR(IDF_PERSIST, ragdolleffect, 2, 500, VAR_MAX);

    VAR(IDF_PERSIST, playerovertone, -1, CTONE_TEAM, CTONE_MAX-1);
    VAR(IDF_PERSIST, playerundertone, -1, CTONE_TMIX, CTONE_MAX-1);
    VAR(IDF_PERSIST, playerdisplaytone, -1, CTONE_MIXED, CTONE_MAX-1);
    VAR(IDF_PERSIST, playereffecttone, -1, CTONE_TEAMED, CTONE_MAX-1);

    ICOMMAND(0, gamemode, "", (), intret(gamemode));
    ICOMMAND(0, mutators, "", (), intret(mutators));
    ICOMMAND(0, getintermission, "", (), intret(intermission ? 1 : 0));

    void start() { }

    const char *gametitle() { return connected() ? server::gamename(gamemode, mutators) : "ready"; }
    const char *gametext() { return connected() ? mapname : "not connected"; }

    bool thirdpersonview(bool viewonly)
    {
        if(!viewonly && (focus->state == CS_DEAD || focus->state == CS_WAITING)) return true;
        if(!(focus != player1 ? thirdpersonfollow : thirdperson)) return false;
        if(player1->state == CS_EDITING) return false;
        if(player1->state >= CS_SPECTATOR && focus == player1) return false;
        if(inzoom()) return false;
        return true;
    }
    ICOMMAND(0, isthirdperson, "i", (int *viewonly), intret(thirdpersonview(*viewonly ? true : false) ? 1 : 0));
    ICOMMAND(0, thirdpersonswitch, "", (), int *n = (focus != player1 ? &thirdpersonfollow : &thirdperson); *n = !*n;);

    int mousestyle()
    {
        if(inzoom()) return zoommousetype;
        return mousetype;
    }

    int deadzone()
    {
        if(inzoom()) return zoommousedeadzone;
        return mousedeadzone;
    }

    int panspeed()
    {
        if(inzoom()) return zoommousepanspeed;
        return mousepanspeed;
    }

    int fov()
    {
        int r = curfov;
        if(player1->state == CS_EDITING) r = editfov;
        else if(focus == player1 && player1->state == CS_SPECTATOR) r = specfov;
        else if(thirdpersonview(true)) r = thirdpersonfov;
        else r = firstpersonfov;
        return r;
    }

    void checkzoom()
    {
        if(zoomdefault > zoomlevels) zoomdefault = zoomlevels;
        if(zoomlevel < 0) zoomlevel = zoomdefault ? zoomdefault : zoomlevels;
        if(zoomlevel > zoomlevels) zoomlevel = zoomlevels;
    }

    void setzoomlevel(int level)
    {
        checkzoom();
        zoomlevel += level;
        if(zoomlevel > zoomlevels) zoomlevel = zoomscroll ? 1 : zoomlevels;
        else if(zoomlevel < 1) zoomlevel = zoomscroll ? zoomlevels : 1;
    }
    ICOMMAND(0, setzoom, "i", (int *level), setzoomlevel(*level));

    void zoomset(bool on, int millis)
    {
        if(on != zooming)
        {
            resetcursor();
            lastzoom = millis-max(zoomtime-(millis-lastzoom), 0);
            prevzoom = zooming;
            if(zoomdefault && on) zoomlevel = zoomdefault;
        }
        checkzoom();
        zooming = on;
    }

    bool zoomallow()
    {
        if(allowmove(player1) && WEAP(player1->weapselect, zooms)) switch(zoomlock)
        {
            case 4: if(!physics::iscrouching(player1)) break;
            case 3: if(player1->physstate != PHYS_FLOOR) break;
            case 2: if(player1->move || player1->strafe) break;
            case 1: if(physics::sliding(player1, true) || (player1->timeinair && (!zooming || !lastzoom || player1->timeinair >= zoomlocktime || player1->impulse[IM_JUMP]))) break;
            case 0: default: return true; break;
        }
        zoomset(false, 0);
        return false;
    }

    bool inzoom()
    {
        if(zoomallow() && lastzoom && (zooming || lastmillis-lastzoom <= zoomtime))
            return true;
        return false;
    }
    ICOMMAND(0, iszooming, "", (), intret(inzoom() ? 1 : 0));

    bool inzoomswitch()
    {
        if(zoomallow() && lastzoom && ((zooming && lastmillis-lastzoom >= zoomtime/2) || (!zooming && lastmillis-lastzoom <= zoomtime/2)))
            return true;
        return false;
    }

    void resetsway()
    {
        swaydir = swaypush = vec(0, 0, 0);
        swayfade = swayspeed = swaydist = bobfade = bobdist = 0;
    }

    void addsway(gameent *d)
    {
        float speed = physics::movevelocity(d), step = firstpersonbob ? firstpersonbobstep : firstpersonswaystep;
        if(d->state == CS_ALIVE && (d->physstate >= PHYS_SLOPE || d->onladder || d->turnside))
        {
            swayspeed = min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), speed);
            swaydist += swayspeed*curtime/1000.0f;
            swaydist = fmod(swaydist, 2*step);
            bobdist += swayspeed*curtime/1000.0f;
            bobdist = fmod(bobdist, 2*firstpersonbobstep);
            bobfade = swayfade = 1;
        }
        else
        {
            if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*step);
                swayfade -= 0.5f*(curtime*speed)/(step*1000.0f);
            }
            if(bobfade > 0)
            {
                bobdist += swayspeed*bobfade*curtime/1000.0f;
                bobdist = fmod(bobdist, 2*firstpersonbobstep);
                bobfade -= 0.5f*(curtime*speed)/(firstpersonbobstep*1000.0f);
            }
        }

        float k = pow(0.7f, curtime/25.0f);
        swaydir.mul(k);
        vec inertia = vec(d->vel).add(d->falling);
        float speedscale = max(inertia.magnitude(), speed);
        if(d->state == CS_ALIVE && speedscale > 0) swaydir.add(vec(inertia).mul((1-k)/(15*speedscale)));
        swaypush.mul(pow(0.5f, curtime/25.0f));
    }

    int errorchan = -1;
    void errorsnd(gameent *d)
    {
        if(d == player1 && !issound(errorchan))
            playsound(S_ERROR, d->o, d, SND_FORCED, -1, -1, -1, &errorchan);
    }

    void announce(int idx, gameent *d)
    {
        if(idx >= 0)
        {
            physent *t = !d || d == focus ? camera1 : d;
            playsound(idx, t->o, t, (t == camera1 ? SND_FORCED : SND_DIRECT)|SND_NOCULL, -1, -1, -1, d ? &d->aschan : NULL);
        }
    }
    void announcef(int idx, int targ, gameent *d, const char *msg, ...)
    {
        if(targ >= 0 && msg && *msg)
        {
            defvformatstring(text, msg, msg);
            conoutft(targ, "%s", text);
        }
        announce(idx, d);
    }
    ICOMMAND(0, announce, "iis", (int *idx, int *targ, char *s), announcef(*idx, *targ, NULL, "\fw%s", s));

    bool tvmode(bool check)
    {
        if(!m_edit(gamemode) && (!check || !cameras.empty())) switch(player1->state)
        {
            case CS_SPECTATOR: if(specmode) return true; break;
            case CS_WAITING: if(waitmode >= (m_duke(gamemode, mutators) ? 1 : 2) && (!player1->lastdeath || lastmillis-player1->lastdeath >= 500))
                return true; break;
            default: break;
        }
        return false;
    }

    ICOMMAND(0, specmodeswitch, "", (), specmode = specmode ? 0 : 1; hud::showscores(false); follow = 0);
    ICOMMAND(0, waitmodeswitch, "", (), waitmode = waitmode ? 0 : (m_duke(gamemode, mutators) ? 1 : 2); hud::showscores(false); follow = 0);

    void followswitch(int n)
    {
        if(!tvmode())
        {
            follow += n;
            int numdyns = numdynents();
            #define checkfollow \
                if(follow >= numdyns) follow = 0; \
                else if(follow < 0) follow = numdyns-1;
            checkfollow;
            while(true)
            {
                gameent *d = (gameent *)iterdynents(follow);
                if(!d || d->aitype >= AI_START)
                {
                    follow += clamp(n, -1, 1);
                    checkfollow;
                }
                else break;
            }
        }
    }
    ICOMMAND(0, followdelta, "i", (int *n), followswitch(*n ? *n : 1));

    bool allowmove(physent *d)
    {
        if(d == player1 && tvmode()) return false;
        if(d->type == ENT_PLAYER || d->type == ENT_AI)
        {
            if(d->state == CS_DEAD || d->state >= CS_SPECTATOR || intermission)
                return false;
        }
        return true;
    }

    void chooseloadweap(gameent *d, const char *a, const char *b)
    {
        if(m_arena(gamemode, mutators))
        {
            loopj(2)
            {
                const char *s = j ? b : a;
                if(*s >= '0' && *s <= '9') d->loadweap[j] = parseint(s);
                else loopi(WEAP_MAX) if(!strcasecmp(WEAP(i, name), s))
                {
                    d->loadweap[j] = i;
                    break;
                }
                if(d->loadweap[j] < WEAP_OFFSET || d->loadweap[j] >= WEAP_ITEM) d->loadweap[j] = WEAP_MELEE;
            }
            client::addmsg(N_LOADWEAP, "ri3", d->clientnum, d->loadweap[0], d->loadweap[1]);
            conoutft(CON_SELF, "weapon selection is now: \fs\f[%d]\f(%s)%s\fS and \fs\f[%d]\f(%s)%s\fS",
                WEAP(d->loadweap[0] != WEAP_MELEE ? d->loadweap[0] : WEAP_MELEE, colour), (d->loadweap[0] != WEAP_MELEE ? hud::itemtex(WEAPON, d->loadweap[0]) : hud::questiontex), (d->loadweap[0] != WEAP_MELEE ? WEAP(d->loadweap[0], name) : "random"),
                WEAP(d->loadweap[1] != WEAP_MELEE ? d->loadweap[1] : WEAP_MELEE, colour), (d->loadweap[1] != WEAP_MELEE ? hud::itemtex(WEAPON, d->loadweap[1]) : hud::questiontex), (d->loadweap[1] != WEAP_MELEE ? WEAP(d->loadweap[1], name) : "random")
            );
        }
        else conoutft(CON_MESG, "\foweapon selection is only available in arena");
    }
    ICOMMAND(0, loadweap, "ss", (char *a, char *b), chooseloadweap(player1, a, b));
    ICOMMAND(0, getloadweap, "i", (int *n), intret(player1->loadweap[*n!=0 ? 1 : 0]));
    ICOMMAND(0, allowedweap, "i", (int *n), intret(isweap(*n) && WEAP(*n, allowed) >= (m_duke(gamemode, mutators) ? 2 : 1) ? 1 : 0));

    void respawn(gameent *d)
    {
        if(d->state == CS_DEAD && d->respawned < 0 && (!d->lastdeath || lastmillis-d->lastdeath >= 500))
        {
            client::addmsg(N_TRYSPAWN, "ri", d->clientnum);
            d->respawned = lastmillis;
        }
    }

    vec pulsecolour(physent *d, int i, int cycle)
    {
        size_t seed = size_t(d) + (lastmillis/cycle);
        int n = detrnd(seed, PULSECOLOURS), n2 = detrnd(seed + 1, PULSECOLOURS);
        return vec::hexcolor(pulsecols[i][n]).lerp(vec::hexcolor(pulsecols[i][n2]), (lastmillis%cycle)/float(cycle));
    }

    int hexpulsecolour(physent *d, int i, int cycle)
    {
        bvec h = bvec::fromcolor(pulsecolour(d, i, cycle));
        return (h.r<<16)|(h.g<<8)|h.b;
    }

    vec getpalette(int palette, int index)
    { // colour palette abstractions for textures, etc.
        switch(palette)
        {
            case 0: // misc
            {
                switch(index)
                {
                    case 0: break; // off
                    case 1: case 2: case 3:
                        return vec::hexcolor(pulsecols[index-1][clamp((lastmillis/100)%PULSECOLOURS, 0, PULSECOLOURS-1)]);
                        break;
                    case 4: case 5: case 6:
                        return pulsecolour(camera1, index-4, 50);
                        break;
                    default: break;
                }
                break;
            }
            case 1: // teams
            {
                int team = index;
                if(team < 0 || team >= TEAM_MAX+TEAM_TOTAL || (!m_team(gamemode, mutators) && !m_edit(gamemode) && team >= TEAM_FIRST && team <= TEAM_MULTI))
                    team = TEAM_NEUTRAL; // abstract team coloured levels to neutral
                else if(team >= TEAM_MAX) team = (team%TEAM_MAX)+TEAM_FIRST; // force team colour palette
                return vec::hexcolor(TEAM(team, colour));
                break;
            }
            case 2: // weapons
            {
                int weap = index;
                if(weap < 0 || weap >= WEAP_MAX*2-1) weap = -1;
                else if(weap >= WEAP_MAX) weap = w_attr(gamemode, weap%WEAP_MAX, m_weapon(gamemode, mutators));
                else
                {
                    weap = w_attr(gamemode, weap, m_weapon(gamemode, mutators));
                    if(!isweap(weap)) weap = -1;
                    else if(m_arena(gamemode, mutators) && weap < WEAP_ITEM && GAME(maxcarry) <= 2) weap = -1;
                    else switch(WEAP(weap, allowed))
                    {
                        case 0: weap = -1; break;
                        case 1: if(m_duke(gamemode, mutators)) weap = -1; // fall through
                        case 2: if(m_limited(gamemode, mutators)) weap = -1;
                        case 3: default: break;
                    }
                }
                if(isweap(weap)) return vec::hexcolor(WEAP(weap, colour));
                break;
            }
            default: break;
        }
        return vec(1, 1, 1);
    }

    void adddynlights()
    {
        if(dynlighteffects)
        {
            projs::adddynlights();
            entities::adddynlights();
            if(dynlighteffects >= 2)
            {
                if(m_capture(gamemode)) capture::adddynlights();
                else if(m_defend(gamemode)) defend::adddynlights();
                else if(m_bomber(gamemode)) bomber::adddynlights();
            }
            gameent *d = NULL;
            int numdyns = numdynents();
            loopi(numdyns) if((d = (gameent *)iterdynents(i)) != NULL)
            {
                if(d->state == CS_ALIVE && isweap(d->weapselect))
                {
                    bool last = lastmillis-d->weaplast[d->weapselect] > 0,
                         powering = last && d->weapstate[d->weapselect] == WEAP_S_POWER,
                         reloading = last && d->weapstate[d->weapselect] == WEAP_S_RELOAD;
                    float amt = last ? clamp(float(lastmillis-d->weaplast[d->weapselect])/d->weapwait[d->weapselect], 0.f, 1.f) : 0.f;
                    if(d->weapselect == WEAP_FLAMER && (!reloading || amt > 0.5f))
                    {
                        float scale = powering ? 1.f+(amt*1.5f) : (d->weapstate[d->weapselect] == WEAP_S_IDLE ? 1.f : (reloading ? (amt-0.5f)*2 : amt));
                        adddynlight(d->ejectpos(d->weapselect), 16*scale, pulsecolour(d), 0, 0, DL_KEEP);
                    }
                    if(d->weapselect == WEAP_SWORD || powering)
                    {
                        static const struct powerdls {
                            int type, colour;
                            float radius;
                        } powerdl[WEAP_MAX] = {
                            { 0, 0, 0 }, // melee
                            { 1, 0x997711, 16 }, // pistol
                            { 1, 0x111177, 18 }, // sword
                            { 1, 0x997700, 20 }, // shotgun
                            { 2, 0xAA6600, 18 }, // smg
                            { 2, 0x991111, 20 }, // flamer
                            { 2, 0x116699, 24 }, // plasma
                            { 1, 0x5511CC, 18 }, // rifle
                            { 2, 0, 18 }, // grenades
                            { 2, 0, 18 }, // rocket
                        };
                        if(powerdl[d->weapselect].type)
                        {
                            float thresh = max(amt, 0.25f), size = 4+powerdl[d->weapselect].radius*thresh;
                            int span = max(WEAP2(d->weapselect, power, physics::secondaryweap(d))/4, 500),
                                interval = lastmillis%span, part = span/2;
                            if(interval)
                                size += size*0.5f*(interval <= part ? interval/float(part) : (span-interval)/float(part));
                            vec col;
                            if(powerdl[d->weapselect].colour > 0) col = vec::hexcolor(powerdl[d->weapselect].colour).mul(thresh);
                            else if(powerdl[d->weapselect].type != 2) col = pulsecolour(d).mul(thresh);
                            else col = vec(max(1.f-amt,0.5f), 0.5f*max(1.f-amt,0.f), 0);
                            adddynlight(d->muzzlepos(d->weapselect), size, col, 0, 0, DL_KEEP);
                        }
                    }
                }
                if(burntime && d->burning(lastmillis, burntime))
                {
                    int millis = lastmillis-d->lastburn;
                    size_t seed = size_t(d) + (millis/50);
                    float pc = d->curscale, amt = (millis%50)/50.0f, intensity = 0.75f+(detrnd(seed, 25)*(1-amt) + detrnd(seed + 1, 25)*amt)/100.f;
                    if(burntime-millis < burndelay) pc *= float(burntime-millis)/float(burndelay);
                    else
                    {
                        float fluc = float(millis%burndelay)*(0.25f+0.03f)/burndelay;
                        if(fluc >= 0.25f) fluc = (0.25f+0.03f-fluc)*(0.25f/0.03f);
                        pc *= 0.75f+fluc;
                    }
                    adddynlight(d->headpos(-d->height*0.5f), d->height*(1.5f+intensity)*pc, vec(1.1f*max(pc,0.5f), 0.45f*max(pc,0.2f), 0.05f*pc), 0, 0, DL_KEEP);
                    continue;
                }
                if(d->aitype < AI_START && illumlevel > 0 && illumradius > 0)
                {
                    vec col = vec::hexcolor(getcolour(d, playereffecttone)).mul(illumlevel);
                    adddynlight(d->headpos(-d->height*0.5f), illumradius, col, 0, 0, DL_KEEP);
                }
            }
        }
    }

    void boosteffect(gameent *d, const vec &pos, int num, int len, bool shape = false)
    {
        float scale = 0.75f+(rnd(25)/100.f);
        part_create(PART_HINT, shape ? 10 : 1, pos, 0x1818A8, scale, min(0.75f*scale, 0.95f), 0, 0);
        part_create(PART_FIREBALL, shape ? 10 : 1, pos, 0xFF6818, scale*0.75f, min(0.75f*scale, 0.95f), 0, 0);
        if(shape) regularshape(PART_FIREBALL, int(d->radius)*2, pulsecols[0][rnd(PULSECOLOURS)], 21, num, len, pos, scale, 0.75f, -5, 0, 10);
        else regular_part_create(PART_FIREBALL, len, pos, pulsecols[0][rnd(PULSECOLOURS)], 0.6f*scale, min(0.75f*scale, 0.95f), -10, 0);
    }

    void impulseeffect(gameent *d, int effect)
    {
        if(d->type == ENT_PLAYER || d->type == ENT_AI)
        {
            int num = int((effect ? 5 : 25)*impulsescale), len = effect ? impulsefade/5 : impulsefade;
            switch(effect)
            {
                case 0: playsound(S_IMPULSE, d->o, d); // faill through
                case 1:
                {
                    if(num > 0 && len > 0) loopi(2) boosteffect(d, d->jet[i], num, len, effect==0);
                    break;
                }
                case 2:
                {
                    if(issound(d->jschan))
                    {
                        sounds[d->jschan].vol = min((lastmillis-sounds[d->jschan].millis)*2, 255);
                        sounds[d->jschan].ends = lastmillis+250;
                    }
                    else playsound(S_HOVER, d->o, d, (d == focus ? SND_FORCED : 0)|SND_LOOP, 1, -1, -1, &d->jschan, lastmillis+250);
                    if(num > 0 && len > 0) boosteffect(d, d->jet[2], num, len);
                }
            }
        }
    }

    gameent *pointatplayer()
    {
        loopv(players)
        {
            gameent *o = players[i];
            if(!o) continue;
            vec pos = focus->headpos();
            float dist;
            if(intersect(o, pos, worldpos, dist)) return o;
        }
        return NULL;
    }

    void setmode(int nmode, int nmuts) { server::modecheck(nextmode = nmode, nextmuts = nmuts); }
    ICOMMAND(0, mode, "ii", (int *val, int *mut), setmode(*val, *mut));

    float spawnfade(gameent *d)
    {
        int len = d->aitype >= AI_START ? (aistyle[d->aitype].living ? min(ai::aideadfade, enemyspawntime ? enemyspawntime : INT_MAX-1) : 500) : m_delay(gamemode, mutators);
        if(len > 0)
        {
            int interval = min(len/3, ragdolleffect), over = max(len-interval, 1), millis = lastmillis-d->lastdeath;
            if(millis <= len) { if(millis >= over) return 1.f-((millis-over)/float(interval)); }
            else return 0;
        }
        return 1;
    }

    float rescale(gameent *d)
    {
        float total = actorscale;
        if(d->aitype > AI_NONE)
        {
            bool hasent = d->aitype >= AI_START && entities::ents.inrange(d->aientity) && entities::ents[d->aientity]->type == ACTOR;
            if(hasent && entities::ents[d->aientity]->attrs[8] > 0) total *= (entities::ents[d->aientity]->attrs[8]/100.f)*enemyscale;
            else total *= aistyle[clamp(d->aitype, int(AI_BOT), int(AI_MAX-1))].scale*(d->aitype >= AI_START ? enemyscale : botscale);
        }
        if(d->state != CS_SPECTATOR && d->state != CS_EDITING)
        {
            if(m_resize(gamemode, mutators) || d->aitype >= AI_START)
            {
                float minscale = 1, amtscale = m_insta(gamemode, mutators) ? 1+(d->spree*instaresizeamt) : max(d->health, 1)/float(d->aitype >= AI_START ? aistyle[d->aitype].health*enemystrength : m_health(gamemode, mutators));
                if(m_resize(gamemode, mutators))
                {
                    minscale = minresizescale;
                    if(amtscale < 1) amtscale = (amtscale*(1-minscale))+minscale;
                }
                total *= clamp(amtscale, minscale, maxresizescale);
            }
            if(d->state == CS_DEAD || d->state == CS_WAITING) total *= spawnfade(d);
        }
        return total;
    }

    float opacity(gameent *d, bool third = true)
    {
        float total = d == focus ? (third ? (d != player1 ? followblend : thirdpersonblend) : firstpersonblend) : playerblend;
        if(d->state == CS_DEAD || d->state == CS_WAITING) total *= spawnfade(d);
        else if(d->state == CS_ALIVE)
        {
            if(d == focus && d->weapselect == WEAP_MELEE && !thirdpersonview(true)) return 0; // hack
            int prot = m_protect(gamemode, mutators), millis = d->protect(lastmillis, prot); // protect returns time left
            if(millis > 0) total *= 1.f-(float(millis)/float(prot));
            if(d == player1 && inzoom())
            {
                int frame = lastmillis-lastzoom;
                float pc = frame <= zoomtime ? (frame)/float(zoomtime) : 1.f;
                total *= zooming ? 1.f-pc : pc;
            }
        }
        return total;
    }

    void checkoften(gameent *d, bool local)
    {
        adjustscaled(int, d->quake, quakefade);

        d->setscale(rescale(d), curtime, false, gamemode, mutators);
        d->speedscale = d->curscale;
        if(d->aitype > AI_NONE)
        {
            bool hasent = d->aitype >= AI_START && entities::ents.inrange(d->aientity) && entities::ents[d->aientity]->type == ACTOR;
            if(hasent && entities::ents[d->aientity]->attrs[7] > 0) d->speedscale *= entities::ents[d->aientity]->attrs[7]*enemyspeed;
            else d->speedscale *= d->aitype >= AI_START ? enemyspeed : botspeed;
        }

        float offset = d->height; d->o.z -= d->height;
        if(aistyle[clamp(d->aitype, int(AI_BOT), int(AI_MAX-1))].cancrouch)
        {
            bool crouching = d->action[AC_CROUCH];
            float crouchoff = 1.f-CROUCHHEIGHT;
            if(!crouching) loopk(2)
            {
                vec old = d->o;
                float zoff = d->zradius*crouchoff, zrad = d->zradius-zoff, frac = zoff/10.f;
                d->o.z += zrad;
                if(k)
                {
                    vec dir;
                    vecfromyawpitch(d->yaw, 0, d->move, d->strafe, dir);
                    d->o.add(dir);
                    if(!collide(d, vec(0, 0, 1), 0.f, false)) break;
                }
                loopi(10)
                {
                    d->o.z += frac;
                    if(!collide(d, vec(0, 0, 1), 0.f, false))
                    {
                        crouching = true;
                        break;
                    }
                }
                d->o = old;
                if(crouching)
                {
                    if(d->actiontime[AC_CROUCH] >= 0) d->actiontime[AC_CROUCH] = max(PHYSMILLIS-(lastmillis-d->actiontime[AC_CROUCH]), 0)-lastmillis;
                    break;
                }
                else if(k && d->actiontime[AC_CROUCH] < 0)
                {
                    d->actiontime[AC_CROUCH] = lastmillis-max(PHYSMILLIS-(lastmillis+d->actiontime[AC_CROUCH]), 0);
                    break;
                }
            }
            if(physics::iscrouching(d))
            {
                int crouchtime = abs(d->actiontime[AC_CROUCH]);
                float amt = lastmillis-crouchtime <= PHYSMILLIS ? clamp(float(lastmillis-crouchtime)/PHYSMILLIS, 0.f, 1.f) : 1.f;
                if(!crouching) amt = 1.f-amt;
                crouchoff *= amt;
                d->height = d->zradius-(d->zradius*crouchoff);
            }
            else d->height = d->zradius;
        }
        else d->height = d->zradius;
        d->o.z += d->timeinair ? offset+(d->height-offset) : d->height;

        d->checktags();

        loopi(WEAP_MAX) if(d->weapstate[i] != WEAP_S_IDLE)
        {
            bool timeexpired = lastmillis-d->weaplast[i] >= d->weapwait[i]+(d->weapselect != i || d->weapstate[i] != WEAP_S_POWER ? 0 : PHYSMILLIS);
            if(d->state == CS_ALIVE && i == d->weapselect && d->weapstate[i] == WEAP_S_RELOAD && timeexpired)
            {
                if(timeexpired && playreloadnotify&(d == focus ? 1 : 2) && (d->ammo[i] >= WEAP(i, max) || playreloadnotify&(d == focus ? 4 : 8)))
                    playsound(WEAPSND(i, S_W_NOTIFY), d->o, d, d == focus ? SND_FORCED : 0, -1, -1, -1, &d->wschan);
            }
            if(d->state != CS_ALIVE || timeexpired)
                d->setweapstate(i, WEAP_S_IDLE, 0, lastmillis);
        }
        if(d->state == CS_ALIVE && isweap(d->weapselect) && d->weapstate[d->weapselect] == WEAP_S_POWER)
        {
            int millis = lastmillis-d->weaplast[d->weapselect];
            if(millis > 0)
            {
                bool secondary = physics::secondaryweap(d);
                float amt = millis/float(d->weapwait[d->weapselect]);
                int vol = 255;
                if(WEAP2(d->weapselect, power, secondary)) switch(WEAP2(d->weapselect, cooked, secondary))
                {
                    case 4: case 5: vol = 10+int(245*(1.f-amt)); break; // longer
                    case 1: case 2: case 3: default: vol = 10+int(245*amt); break; // shorter
                }
                if(issound(d->pschan)) sounds[d->pschan].vol = vol;
                else playsound(WEAPSND2(d->weapselect, secondary, S_W_POWER), d->o, d, (d == focus ? SND_FORCED : 0)|SND_LOOP, vol, -1, -1, &d->pschan);
            }
        }
        else if(issound(d->pschan)) removesound(d->pschan);
        if(d->respawned > 0 && lastmillis-d->respawned >= 2500) d->respawned = -1;
        if(d->suicided > 0 && lastmillis-d->suicided >= 2500) d->suicided = -1;
        if(d->lastburn > 0 && lastmillis-d->lastburn >= burntime-500)
        {
            if(lastmillis-d->lastburn >= burntime) d->resetburning();
            else if(issound(d->fschan)) sounds[d->fschan].vol = int((d != focus ? 128 : 224)*(1.f-(lastmillis-d->lastburn-(burntime-500))/500.f));
        }
        else if(issound(d->fschan)) removesound(d->fschan);
        if(d->lastbleed > 0 && lastmillis-d->lastbleed >= bleedtime) d->resetbleeding();
        if(issound(d->jschan) && !physics::hover(d))
        {
            if(sounds[d->jschan].ends < lastmillis) removesound(d->jschan);
            else sounds[d->jschan].vol = int(ceilf(255*(float(sounds[d->jschan].ends-lastmillis)/250.f)));
        }
        loopv(d->icons) if(lastmillis-d->icons[i].millis > d->icons[i].fade) d->icons.remove(i--);
    }


    void otherplayers()
    {
        loopv(players) if(players[i])
        {
            gameent *d = players[i];
            const int lagtime = totalmillis-d->lastupdate;
            if(d->ai || !lagtime || intermission) continue;
            //else if(lagtime > 1000) continue;
            physics::smoothplayer(d, 1, false);
        }
    }

    bool burn(gameent *d, int weap, int flags)
    {
        if(burntime && hithurts(flags) && (flags&HIT_MELT || (weap == -1 && flags&HIT_BURN) || doesburn(weap, flags)))
        {
            d->lastburntime = lastmillis;
            if(!issound(d->fschan)) playsound(S_BURNING, d->o, d, SND_LOOP, d != focus ? 128 : 224, -1, -1, &d->fschan);
            if(isweap(weap)) d->lastburn = lastmillis;
            else return true;
        }
        return false;
    }

    bool bleed(gameent *d, int weap, int flags)
    {
        if(bleedtime && hithurts(flags) && ((weap == -1 && flags&HIT_BLEED) || doesbleed(weap, flags)))
        {
            d->lastbleedtime = lastmillis;
            if(isweap(weap)) d->lastbleed = lastmillis;
            else return true;
        }
        return false;
    }

    struct damagemerge
    {
        enum { BURN = 1<<0, BLEED = 1<<1, CRIT = 1<<2 };

        gameent *d, *actor;
        int weap, damage, flags, millis;

        damagemerge() { millis = totalmillis; }
        damagemerge(gameent *d, gameent *actor, int weap, int damage, int flags) : d(d), actor(actor), weap(weap), damage(damage), flags(flags) { millis = totalmillis; }

        bool merge(const damagemerge &m)
        {
            if(actor != m.actor || flags != m.flags) return false;
            damage += m.damage;
            return true;
        }

        void play()
        {
            if(flags&CRIT)
            {
                if(playcrittones >= (actor == focus ? 1 : (d == focus ? 2 : 3)))
                    playsound(S_CRITICAL, d->o, d, d == focus ? SND_FORCED : SND_DIRECT);
            }
            else
            {
                if(playdamagetones >= (actor == focus ? 1 : (d == focus ? 2 : 3)))
                {
                    const int dmgsnd[8] = { 0, 10, 25, 50, 75, 100, 150, 200 };
                    int snd = -1;
                    if(flags&BURN) snd = S_BURNED;
                    else if(flags&BLEED) snd = S_BLEED;
                    else loopirev(8) if(damage >= dmgsnd[i]) { snd = S_DAMAGE+i; break; }
                    if(snd >= 0) playsound(snd, d->o, d, d == focus ? SND_FORCED : SND_DIRECT);
                }
                if(aboveheaddamage)
                {
                    defformatstring(ds)("<sub>\fr+%d", damage);
                    part_textcopy(d->abovehead(), ds, d != focus ? PART_TEXT : PART_TEXT_ONTOP, eventiconfade, 0xFFFFFF, 4, 1, -10, 0, d);
                }
            }
        }
    };
    vector<damagemerge> damagemerges;

    void removedamagemerges(gameent *d)
    {
        loopvrev(damagemerges) if(damagemerges[i].d == d || damagemerges[i].actor == d) damagemerges.removeunordered(i);
    }

    void pushdamagemerge(gameent *d, gameent *actor, int weap, int damage, int flags)
    {
        damagemerge dt(d, actor, weap, damage, flags);
        loopv(damagemerges) if(damagemerges[i].merge(dt)) return;
        damagemerges.add(dt);
    }

    void flushdamagemerges()
    {
        loopv(damagemerges)
        {
            int delay = damagemergedelay;
            if(damagemerges[i].flags&damagemerge::BURN) delay = damagemergeburn;
            else if(damagemerges[i].flags&damagemerge::BLEED) delay = damagemergebleed;
            if(damagemerges[i].flags&damagemerge::CRIT || totalmillis-damagemerges[i].millis >= delay)
            {
                damagemerges[i].play();
                damagemerges.remove(i--);
            }
        }
    }

    static int alarmchan = -1;
    void hiteffect(int weap, int flags, int damage, gameent *d, gameent *actor, vec &dir, bool local)
    {
        bool burning = burn(d, weap, flags), bleeding = bleed(d, weap, flags);
        if(!local || burning || bleeding)
        {
            if(hithurts(flags))
            {
                if(d == focus) hud::damage(damage, actor->o, actor, weap, flags);
                if(d->type == ENT_PLAYER || d->type == ENT_AI)
                {
                    vec p = d->headpos(-d->height/4);
                    if(aistyle[d->aitype].living)
                    {
                        if(!kidmode && bloodscale > 0)
                            part_splash(PART_BLOOD, int(clamp(damage/2, 2, 10)*bloodscale)*(bleeding ? 2 : 1), bloodfade, p, 0x229999, (rnd(bloodsize/2)+(bloodsize/2))/10.f, 1, 100, DECAL_BLOOD, int(d->radius), 10);
                        if(kidmode || bloodscale <= 0 || bloodsparks)
                            part_splash(PART_PLASMA, int(clamp(damage/2, 2, 10))*(bleeding ? 2: 1), bloodfade, p, 0x882222, 1, 0.5f, 50, DECAL_STAIN, int(d->radius));
                    }
                    if(d->aitype < AI_START && !issound(d->vschan)) playsound(S_PAIN, d->o, d, 0, -1, -1, -1, &d->vschan);
                    if(!burning && !bleeding) d->quake = clamp(d->quake+max(damage/2, 1), 0, 1000);
                    d->lastpain = lastmillis;
                }
                if(d != actor)
                {
                    bool sameteam = m_team(gamemode, mutators) && d->team == actor->team;
                    if(!sameteam) pushdamagemerge(d, actor, weap, damage, burning ? damagemerge::BURN : (bleeding ? damagemerge::BLEED : 0));
                    else if(actor == player1 && !burning && !bleeding)
                    {
                        player1->lastteamhit = d->lastteamhit = lastmillis;
                        if(!issound(alarmchan)) playsound(S_ALARM, actor->o, actor, 0, -1, -1, -1, &alarmchan);
                    }
                    if(!burning && !bleeding && !sameteam) actor->lasthit = totalmillis;
                }
            }
            if(isweap(weap) && !burning && !bleeding && (d->aitype < AI_START || aistyle[d->aitype].canmove))
            {
                float scale = float(damage)/float(WEAP2(weap, damage, flags&HIT_ALT));
                if(hithurts(flags) && WEAP2(weap, stuntime, flags&HIT_ALT))
                    d->addstun(weap, lastmillis, int(scale*WEAP2(weap, stuntime, flags&HIT_ALT)), scale*WEAP2(weap, stunscale, flags&HIT_ALT));
                if(WEAP2(weap, slow, flags&HIT_ALT) > 0)
                {
                    float force = flags&HIT_WAVE || !hithurts(flags) ? waveslowscale : hitslowscale;
                    d->vel.mul(1.f-(scale*WEAP2(weap, slow, flags&HIT_ALT))*force);
                }
                if(WEAP2(weap, hitpush, flags&HIT_ALT) != 0)
                {
                    if(d == actor) scale *= 1/WEAP2(weap, selfdmg, flags&HIT_ALT);
                    float force = flags&HIT_WAVE || !hithurts(flags) ? wavepushscale : (d->health <= 0 ? deadpushscale : hitpushscale);
                    d->vel.add(vec(dir).mul(scale*WEAP2(weap, hitpush, flags&HIT_ALT)*WEAPLM(force, gamemode, mutators)));
                }
            }
            ai::damaged(d, actor, weap, flags, damage);
        }
    }

    void damaged(int weap, int flags, int damage, int health, gameent *d, gameent *actor, int millis, vec &dir)
    {
        if(d->state != CS_ALIVE || intermission) return;
        if(hithurts(flags))
        {
            d->health = health;
            if(d->health <= m_health(gamemode, mutators)) d->lastregen = 0;
            d->lastpain = lastmillis;
            actor->totaldamage += damage;
        }
        hiteffect(weap, flags, damage, d, actor, dir, actor == player1 || actor->ai);
        if(flags&HIT_CRIT)
        {
            pushdamagemerge(d, actor, weap, damage, damagemerge::CRIT);
            d->addicon(eventicon::CRITICAL, lastmillis, eventiconcrit, 0);
            actor->addicon(eventicon::CRITICAL, lastmillis, eventiconcrit, 1);
        }
    }

    void killed(int weap, int flags, int damage, gameent *d, gameent *actor, vector<gameent *> &log, int style)
    {
        if(d->type != ENT_PLAYER && d->type != ENT_AI) return;
        d->lastregen = 0;
        d->lastpain = lastmillis;
        d->state = CS_DEAD;
        d->deaths++;
        d->obliterated = (style&FRAG_OBLITERATE) != 0;
        bool burning = burn(d, weap, flags), bleeding = bleed(d, weap, flags), isfocus = d == focus || actor == focus,
             isme = d == player1 || actor == player1, allowanc = obitannounce && (obitannounce > 1 || isfocus) && (m_fight(gamemode) || isme) && actor->aitype < AI_START;
        int anc = d == focus && !m_duke(gamemode, mutators) && !m_trial(gamemode) && allowanc ? S_V_FRAGGED : -1,
            dth = d->aitype >= AI_START || d->obliterated ? S_SPLOSH : S_DEATH;
        if(d != player1) d->resetinterp();
        if(!isme) { loopv(log) if(log[i] == player1) { isme = true; break; } }
        formatstring(d->obit)("%s ", colorname(d));
        if(d != actor && actor->lastattacker == d->clientnum) actor->lastattacker = -1;
        d->lastattacker = actor->clientnum;
        if(d == actor)
        {
            if(!aistyle[d->aitype].living) concatstring(d->obit, "was destroyed");
            else if(flags&HIT_MELT) concatstring(d->obit, *obitlava ? obitlava : "melted into a ball of fire");
            else if(flags&HIT_WATER) concatstring(d->obit, *obitwater ? obitwater : "died");
            else if(flags&HIT_DEATH) concatstring(d->obit, *obitdeath ? obitdeath : "died");
            else if(flags&HIT_SPAWN) concatstring(d->obit, "tried to spawn inside solid matter");
            else if(flags&HIT_LOST) concatstring(d->obit, "got very, very lost");
            else if(flags&HIT_SPEC) concatstring(d->obit, "gave up their corporeal form");
            else if(flags && isweap(weap) && !burning && !bleeding)
            {
                static const char *suicidenames[WEAP_MAX] = {
                    "hit themself",
                    "ate a bullet",
                    "created too much torsional stress",
                    "tested the effectiveness of their own shrapnel",
                    "fell victim to their own crossfire",
                    "spontaneously combusted",
                    "was caught up in their own plasma-filled mayhem",
                    "got a good shock",
                    "kicked it, kamikaze style",
                    "exploded with style",
                };
                concatstring(d->obit, suicidenames[weap]);
            }
            else if(flags&HIT_BURN || burning) concatstring(d->obit, "burned up");
            else if(flags&HIT_BLEED || bleeding) concatstring(d->obit, "bled out");
            else if(d->obliterated) concatstring(d->obit, "was obliterated");
            else concatstring(d->obit, "suicided");
        }
        else
        {
            concatstring(d->obit, "was ");
            if(!aistyle[d->aitype].living) concatstring(d->obit, "destroyed by");
            else if(burning) concatstring(d->obit, "set ablaze by");
            else if(bleeding) concatstring(d->obit, "fatally wounded by");
            else if(isweap(weap))
            {
                static const char *obitnames[5][WEAP_MAX] = {
                    {
                        "punched by",
                        "pierced by",
                        "impaled by",
                        "sprayed with buckshot by",
                        "riddled with holes by",
                        "char-grilled by",
                        "plasmified by",
                        "laser shocked by",
                        "blown to pieces by",
                        "exploded by",
                    },
                    {
                        "kicked by",
                        "pierced by",
                        "impaled by",
                        "filled with lead by",
                        "spliced apart by",
                        "fireballed by",
                        "shown the light by",
                        "given laser burn by",
                        "blown to pieces by",
                        "exploded by",
                    },
                    {
                        "given kung-fu lessons by",
                        "capped by",
                        "sliced in half by",
                        "scrambled by",
                        "air conditioned courtesy of",
                        "char-grilled by",
                        "plasmafied by",
                        "expertly sniped by",
                        "blown to pieces by",
                        "exploded by",
                    },
                    {
                        "given kung-fu lessons by",
                        "skewered by",
                        "sliced in half by",
                        "turned into little chunks by",
                        "swiss-cheesed by",
                        "barbequed by",
                        "reduced to ooze by",
                        "given laser shock treatment by",
                        "turned into shrapnel by",
                        "obliterated by",
                    },
                    {
                        "given kung-fu lessons by",
                        "picked to pieces by",
                        "melted in half by",
                        "filled with shrapnel by",
                        "air-conditioned by",
                        "cooked alive by",
                        "melted alive by",
                        "electrified by",
                        "turned into shrapnel by",
                        "obliterated by",
                    }
                };
                concatstring(d->obit, obitnames[flags&HIT_FLAK ? 4 : (d->obliterated ? 3 : (style&FRAG_HEADSHOT ? 2 : (flags&HIT_ALT ? 1 : 0)))][weap]);
            }
            else concatstring(d->obit, "killed by");
            bool override = false;
            if(!m_fight(gamemode) || actor->aitype >= AI_START)
            {
                concatstring(d->obit, actor->aitype >= AI_START ? " a " : " ");
                concatstring(d->obit, colorname(actor));
            }
            else if(m_team(gamemode, mutators) && d->team == actor->team)
            {
                concatstring(d->obit, " \fs\fzawteam-mate\fS ");
                concatstring(d->obit, colorname(actor));
                if(actor == focus) { anc = S_ALARM; override = true; }
            }
            else
            {
                if(style&FRAG_REVENGE)
                {
                    concatstring(d->obit, " \fs\fzoyvengeful\fS");
                    actor->addicon(eventicon::REVENGE, lastmillis, eventiconfade); // revenge
                    actor->dominating.removeobj(d);
                    d->dominated.removeobj(actor);
                    if(allowanc)
                    {
                        anc = S_V_REVENGE;
                        override = true;
                    }
                }
                else if(style&FRAG_DOMINATE)
                {
                    concatstring(d->obit, " \fs\fzoydominating\fS");
                    actor->addicon(eventicon::DOMINATE, lastmillis, eventiconfade); // dominating
                    if(actor->dominated.find(d) < 0) actor->dominated.add(d);
                    if(d->dominating.find(actor) < 0) d->dominating.add(actor);
                    if(allowanc)
                    {
                        anc = S_V_DOMINATE;
                        override = true;
                    }
                }
                concatstring(d->obit, " ");
                concatstring(d->obit, colorname(actor));

                if(style&FRAG_MKILL1)
                {
                    concatstring(d->obit, " \fs\fzZedouble-killing\fS");
                    actor->addicon(eventicon::MULTIKILL, lastmillis, eventiconfade, 0);
                    if(!override && allowanc) anc = S_V_MULTI;
                }
                else if(style&FRAG_MKILL2)
                {
                    concatstring(d->obit, " \fs\fzZetriple-killing\fS");
                    actor->addicon(eventicon::MULTIKILL, lastmillis, eventiconfade, 1);
                    if(!override && allowanc) anc = S_V_MULTI2;
                }
                else if(style&FRAG_MKILL3)
                {
                    concatstring(d->obit, " \fs\fzZemulti-killing\fS");
                    actor->addicon(eventicon::MULTIKILL, lastmillis, eventiconfade, 2);
                    if(!override && allowanc) anc = S_V_MULTI3;
                }
            }

            if(style&FRAG_HEADSHOT)
            {
                actor->addicon(eventicon::HEADSHOT, lastmillis, eventiconfade, 0);
                if(!override && allowanc) anc = S_V_HEADSHOT;
            }
            if(style&FRAG_FIRSTBLOOD)
            {
                concatstring(d->obit, " for \fs\fzwRfirst blood\fS");
                actor->addicon(eventicon::FIRSTBLOOD, lastmillis, eventiconfade, 0);
                if(!override && allowanc)
                {
                    anc = S_V_FIRSTBLOOD;
                    override = true;
                }
            }

            if(style&FRAG_SPREE1)
            {
                concatstring(d->obit, " in total \fs\fzcgcarnage\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 0);
                if(!override && allowanc)
                {
                    anc = S_V_SPREE;
                    override = true;
                }
            }
            else if(style&FRAG_SPREE2)
            {
                concatstring(d->obit, " on a \fs\fzcgslaughter\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 1);
                if(!override && allowanc)
                {
                    anc = S_V_SPREE2;
                    override = true;
                }
            }
            else if(style&FRAG_SPREE3)
            {
                concatstring(d->obit, " on a \fs\fzcgmassacre\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 2);
                if(!override && allowanc)
                {
                    anc = S_V_SPREE3;
                    override = true;
                }
            }
            else if(style&FRAG_SPREE4)
            {
                concatstring(d->obit, " in a \fs\fzcgbloodbath\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 3);
                if(!override && allowanc)
                {
                    anc = S_V_SPREE4;
                    override = true;
                }
            }
            if(flags&HIT_CRIT) concatstring(d->obit, " with a \fs\fzgrcritical\fS hit");
        }
        if(!log.empty())
        {
            concatstring(d->obit, rnd(2) ? ", assisted by" : ", helped by");
            loopv(log) if(log[i])
            {
                concatstring(d->obit, log.length() > 1 && i == log.length()-1 ? " and " : (i ? ", " : " "));
                if(log[i]->aitype >= AI_START) concatstring(d->obit, "a ");
                concatstring(d->obit, colorname(log[i]));
            }
        }
        if(d != actor)
        {
            if(actor->state == CS_ALIVE && d->aitype < AI_START)
            {
                copystring(actor->obit, d->obit);
                actor->lastkill = totalmillis;
            }
        }
        if(dth >= 0) playsound(dth, d->o, d, 0, -1, -1, -1, &d->vschan);
        if(showobituaries && d->aitype < AI_START)
        {
            bool show = false;
            if(flags&HIT_LOST) show = true;
            else switch(showobituaries)
            {
                case 1: if(isme || m_duke(gamemode, mutators)) show = true; break;
                case 2: if(isme || anc >= 0 || m_duke(gamemode, mutators)) show = true; break;
                case 3: if(isme || d->aitype == AI_NONE || anc >= 0 || m_duke(gamemode, mutators)) show = true; break;
                case 4: if(isme || d->aitype == AI_NONE || actor->aitype == AI_NONE || anc >= 0 || m_duke(gamemode, mutators)) show = true; break;
                case 5: default: show = true; break;
            }
            int target = show ? (isme ? CON_SELF : CON_INFO) : -1;
            if(showobitdists && d != actor) announcef(anc, target, d, "\fw%s \fs[\fo@\fy%.2f\fom\fS]", d->obit, actor->o.dist(d->o)/8.f);
            else announcef(anc, target, d, "\fw%s", d->obit);
        }
        if(aistyle[d->aitype].living && gibscale > 0)
        {
            vec pos = d->headpos(-d->height*0.5f);
            int gib = clamp(max(damage,5)/5, 1, 15), amt = int((rnd(gib)+gib+1)*gibscale);
            if(d->obliterated) amt *= 3;
            loopi(amt) projs::create(pos, pos, true, d, PRJ_GIBS, rnd(gibfade)+gibfade, 0, rnd(500)+1, rnd(50)+10);
        }
        if(m_team(gamemode, mutators) && d->team == actor->team && d != actor && actor == player1)
        {
            hud::teamkills.add(totalmillis);
            if(hud::numteamkills() >= hud::teamkillnum) hud::lastteam = totalmillis;
        }
        ai::killed(d, actor);
    }

    void timeupdate(int timeremain)
    {
        timeremaining = timeremain;
        if(!timeremain && !intermission)
        {
            player1->stopmoving(true);
            hud::showscores(true, true);
            intermission = true;
            smartmusic(true, false);
        }
    }

    gameent *newclient(int cn)
    {
        if(cn < 0 || cn >= MAXPLAYERS)
        {
            defformatstring(cnmsg)("clientnum [%d]", cn);
            neterr(cnmsg);
            return NULL;
        }

        if(cn == player1->clientnum) return player1;

        while(cn >= players.length()) players.add(NULL);

        if(!players[cn])
        {
            gameent *d = new gameent();
            d->clientnum = cn;
            players[cn] = d;
        }

        return players[cn];
    }

    gameent *getclient(int cn)
    {
        if(cn == player1->clientnum) return player1;
        if(players.inrange(cn)) return players[cn];
        return NULL;
    }

    int numwaiting()
    {
        int n = 0;
        loopv(waiting) if(waiting[i]->state == CS_WAITING && (!m_team(gamemode, mutators) || waiting[i]->team == player1->team)) n++;
        return n;
    }

    void clientdisconnected(int cn, int reason)
    {
        if(!players.inrange(cn)) return;
        gameent *d = players[cn];
        if(!d) return;
        if(d->name[0] && showplayerinfo && (d->aitype == AI_NONE || ai::showaiinfo))
            conoutft(CON_EVENT, "\fo%s left the game (%s)", colorname(d), reason >= 0 ? disc_reasons[reason] : "normal");
        gameent *e = NULL;
        int numdyns = numdynents();
        loopi(numdyns) if((e = (gameent *)iterdynents(i)))
        {
            e->dominating.removeobj(d);
            e->dominated.removeobj(d);
        }
        if(focus == d) { focus = player1; follow = 0; } // just in case
        cameras.deletecontents();
        client::unignore(d->clientnum);
        waiting.removeobj(d);
        client::clearvotes(d);
        projs::remove(d);
        removedamagemerges(d);
        if(m_capture(gamemode)) capture::removeplayer(d);
        else if(m_defend(gamemode)) defend::removeplayer(d);
        else if(m_bomber(gamemode)) bomber::removeplayer(d);
        DELETEP(players[cn]);
        players[cn] = NULL;
        cleardynentcache();
    }

    void preload()
    {
        maskpackagedirs(~PACKAGEDIR_OCTA);
        ai::preload();
        weapons::preload();
        projs::preload();
        if(m_edit(gamemode) || m_capture(gamemode)) capture::preload();
        if(m_edit(gamemode) || m_defend(gamemode)) defend::preload();
        if(m_edit(gamemode) || m_bomber(gamemode)) bomber::preload();
        maskpackagedirs(~0);
    }

    void resetmap(bool empty) // called just before a map load
    {
        if(!empty) smartmusic(true, false);
    }

    void startmap(const char *name, const char *reqname, bool empty)    // called just after a map load
    {
        ai::startmap(name, reqname, empty);
        intermission = false;
        maptime = hud::lastnewgame = 0;
        cameras.deletecontents();
        projs::reset();
        physics::reset();
        resetworld();
        resetcursor();
        if(*name)
        {
            conoutft(CON_MESG, "\fs\fw%s by %s [\fa%s\fS]", *maptitle ? maptitle : "Untitled", *mapauthor ? mapauthor : "Unknown", server::gamename(gamemode, mutators));
            preload();
        }
        // reset perma-state
        gameent *d;
        int numdyns = numdynents();
        loopi(numdyns) if((d = (gameent *)iterdynents(i)) && (d->type == ENT_PLAYER || d->type == ENT_AI))
            d->mapchange(lastmillis, m_health(gamemode, mutators));
        entities::spawnplayer(player1, -1, false); // prevent the player from being in the middle of nowhere
        resetcamera();
        if(!empty) client::sendinfo = client::sendcrc = true;
        copystring(clientmap, reqname ? reqname : (name ? name : ""));
        resetsway();
    }

    gameent *intersectclosest(vec &from, vec &to, gameent *at)
    {
        gameent *best = NULL, *o;
        float bestdist = 1e16f;
        int numdyns = numdynents();
        loopi(numdyns) if((o = (gameent *)iterdynents(i)))
        {
            if(!o || o==at || o->state!=CS_ALIVE || !physics::issolid(o, at)) continue;
            float dist;
            if(intersect(o, from, to, dist) && dist < bestdist)
            {
                best = o;
                bestdist = dist;
            }
        }
        return best;
    }

    int numdynents(bool all)
    {
        int i = 1+players.length();
        if(all) i += projs::collideprojs.length();
        return i;
    }
    dynent *iterdynents(int i, bool all)
    {
        if(!i) return player1;
        i--;
        if(i<players.length()) return players[i];
        i -= players.length();
        if(all)
        {
            if(i<projs::collideprojs.length()) return projs::collideprojs[i];
        }
        return NULL;
    }
    dynent *focusedent(bool force)
    {
        if(force) return player1;
        return focus;
    }

    bool duplicatename(gameent *d, char *name = NULL)
    {
        if(!name) name = d->name;
        if(d!=player1 && !strcmp(name, player1->name)) return true;
        loopv(players) if(players[i] && d!=players[i] && !strcmp(name, players[i]->name)) return true;
        return false;
    }

    char *colorname(gameent *d, char *name, const char *prefix, bool team, bool dupname)
    {
        if(!name) name = d->name;
        static string cname;
        formatstring(cname)("%s\fs\f[%d]%s", *prefix ? prefix : "", TEAM(d->team, colour), name);
        if(!name[0] || d->aitype == AI_BOT || (d->aitype < AI_START && dupname && duplicatename(d, name)))
        {
            defformatstring(s)(" [%d]", d->clientnum);
            concatstring(cname, s);
        }
        concatstring(cname, "\fS");
        return cname;
    }

    int findcolour(gameent *d, bool tone, bool mix)
    {
        if(tone)
        {
            int col = d->aitype < AI_START ? d->colour : 0;
            if(!col && isweap(d->weapselect)) col = WEAP(d->weapselect, colour);
            if(col)
            {
                if(mix)
                {
                    int r1 = (col>>16), g1 = ((col>>8)&0xFF), b1 = (col&0xFF),
                        c = TEAM(d->team, colour), r2 = (c>>16), g2 = ((c>>8)&0xFF), b2 = (c&0xFF);
                    col = (clamp((r1/2)+(r2/2), 0, 255)<<16)|(clamp((g1/2)+(g2/2), 0, 255)<<8)|clamp((b1/2)+(b2/2), 0, 255);
                }
                return col;
            }
        }
        return TEAM(d->team, colour);
    }

    int getcolour(gameent *d, int level)
    {
        switch(level)
        {
            case -1: return d->colour;
            case CTONE_TMIX: return findcolour(d, true, d->team != TEAM_NEUTRAL); break;
            case CTONE_AMIX: return findcolour(d, true, d->team == TEAM_NEUTRAL); break;
            case CTONE_MIXED: return findcolour(d, true, true); break;
            case CTONE_ALONE: return findcolour(d, d->team != TEAM_NEUTRAL); break;
            case CTONE_TEAMED: return findcolour(d, d->team == TEAM_NEUTRAL); break;
            case CTONE_TONE: return findcolour(d, true); break;
            case CTONE_TEAM: default: return findcolour(d); break;
        }
    }

    void suicide(gameent *d, int flags)
    {
        if((d == player1 || d->ai) && d->state == CS_ALIVE && d->suicided < 0)
        {
            burn(d, -1, flags);
            client::addmsg(N_SUICIDE, "ri2", d->clientnum, flags);
            d->suicided = lastmillis;
        }
    }
    ICOMMAND(0, kill, "",  (), { suicide(player1, 0); });

    vec burncolour(dynent *d)
    {
        size_t seed = size_t(d) + (lastmillis/50);
        int n = detrnd(seed, 2*PULSECOLOURS), n2 = detrnd(seed + 1, 2*PULSECOLOURS);
        return vec::hexcolor(pulsecols[n/PULSECOLOURS][n%PULSECOLOURS]).lerp(vec::hexcolor(pulsecols[n2/PULSECOLOURS][n2%PULSECOLOURS]), (lastmillis%50)/50.0f);
    }

    void particletrack(particle *p, uint type, int &ts,  bool lastpass)
    {
        if(!p || !p->owner || (p->owner->type != ENT_PLAYER && p->owner->type != ENT_AI)) return;
        gameent *d = (gameent *)p->owner;
        switch(type&0xFF)
        {
            case PT_TEXT: case PT_ICON:
            {
                vec q = p->owner->abovehead(p->m.z+4);
                float k = pow(aboveheadsmooth, float(curtime)/float(aboveheadsmoothmillis));
                p->o.mul(k).add(q.mul(1-k));
                break;
            }
            case PT_TAPE: case PT_LIGHTNING:
            {
                float dist = p->o.dist(p->d);
                p->d = p->o = d->muzzlepos(d->weapselect);
                vec dir; vecfromyawpitch(d->yaw, d->pitch, 1, 0, dir);
                p->d.add(dir.mul(dist));
                break;
            }
            case PT_PART: case PT_FIREBALL: case PT_FLARE:
            {
                p->o = d->muzzlepos(d->weapselect);
                break;
            }
            default: break;
        }
    }

    void dynlighttrack(physent *owner, vec &o) { }

    void newmap(int size) { client::addmsg(N_NEWMAP, "ri", size); }

    void loadworld(stream *f, int maptype) { }
    void saveworld(stream *f) { }

    void fixfullrange(float &yaw, float &pitch, float &roll, bool full)
    {
        if(full)
        {
            while(pitch < -180.0f) pitch += 360.0f;
            while(pitch >= 180.0f) pitch -= 360.0f;
            while(roll < -180.0f) roll += 360.0f;
            while(roll >= 180.0f) roll -= 360.0f;
        }
        else
        {
            if(pitch > 89.9f) pitch = 89.9f;
            if(pitch < -89.9f) pitch = -89.9f;
            if(roll > 89.9f) roll = 89.9f;
            if(roll < -89.9f) roll = -89.9f;
        }
        while(yaw < 0.0f) yaw += 360.0f;
        while(yaw >= 360.0f) yaw -= 360.0f;
    }

    void fixrange(float &yaw, float &pitch)
    {
        float r = 0.f;
        fixfullrange(yaw, pitch, r, false);
    }

    void fixview(int w, int h)
    {
        if(inzoom())
        {
            int frame = lastmillis-lastzoom, m = max(zoomfov, zoomlimit), f = m, t = zoomtime;
            checkzoom();
            if(zoomlevels > 1 && zoomlevel < zoomlevels) f = fov()-(((fov()-m)/zoomlevels)*zoomlevel);
            float diff = float(fov()-f), amt = frame < t ? clamp(float(frame)/float(t), 0.f, 1.f) : 1.f;
            if(!zooming) amt = 1.f-amt;
            curfov = fov()-(amt*diff);
        }
        else curfov = float(fov());
    }

    bool mousemove(int dx, int dy, int x, int y, int w, int h)
    {
        bool hasinput = hud::hasinput(true);
        #define mousesens(a,b,c) ((float(a)/float(b))*c)
        if(hasinput || mousestyle() >= 1)
        {
            if(mouseabsolute) // absolute positions, unaccelerated
            {
                cursorx = clamp(float(x)/float(w), 0.f, 1.f);
                cursory = clamp(float(y)/float(h), 0.f, 1.f);
                return false;
            }
            else
            {
                cursorx = clamp(cursorx+mousesens(dx, w, mousesensitivity), 0.f, 1.f);
                cursory = clamp(cursory+mousesens(dy, h, mousesensitivity*(!hasinput && mouseinvert ? -1.f : 1.f)), 0.f, 1.f);
                return true;
            }
        }
        else if(!tvmode())
        {
            bool self = player1->state >= CS_SPECTATOR && focus != player1 && thirdpersonview(true) && thirdpersonaiming;
            physent *target = player1->state >= CS_SPECTATOR ? (self ? player1 : camera1) : (allowmove(player1) ? player1 : NULL);
            if(target)
            {
                float scale = (inzoom() && zoomsensitivity > 0 ? (1.f-(zoomlevel/float(zoomlevels+1)))*zoomsensitivity : (self ? followsensitivity : 1.f))*sensitivity;
                target->yaw += mousesens(dx, sensitivityscale, yawsensitivity*scale);
                target->pitch -= mousesens(dy, sensitivityscale, pitchsensitivity*scale*(!hasinput && mouseinvert ? -1.f : 1.f));
                fixfullrange(target->yaw, target->pitch, target->roll, false);
            }
            return true;
        }
        return false;
    }

    void project(int w, int h)
    {
        int style = hud::hasinput() ? -1 : mousestyle();
        if(style != lastmousetype)
        {
            resetcursor();
            lastmousetype = style;
        }
        if(style >= 0) vecfromcursor(cursorx, cursory, 1.f, cursordir);
    }

    void getyawpitch(const vec &from, const vec &pos, float &yaw, float &pitch)
    {
        float dist = from.dist(pos);
        yaw = -atan2(pos.x-from.x, pos.y-from.y)/RAD;
        pitch = asin((pos.z-from.z)/dist)/RAD;
    }

    void scaleyawpitch(float &yaw, float &pitch, float targyaw, float targpitch, float yawspeed, float pitchspeed, float rotate)
    {
        if(yaw < targyaw-180.0f) yaw += 360.0f;
        if(yaw > targyaw+180.0f) yaw -= 360.0f;
        float offyaw = (rotate < 0 ? fabs(rotate) : (rotate > 0 ? min(float(fabs(targyaw-yaw)), rotate) : fabs(targyaw-yaw)))*yawspeed,
            offpitch = (rotate < 0 ? fabs(rotate) : (rotate > 0 ? min(float(fabs(targpitch-pitch)), rotate) : fabs(targpitch-pitch)))*pitchspeed;
        if(targyaw > yaw)
        {
            yaw += offyaw;
            if(targyaw < yaw) yaw = targyaw;
        }
        else if(targyaw < yaw)
        {
            yaw -= offyaw;
            if(targyaw > yaw) yaw = targyaw;
        }
        if(targpitch > pitch)
        {
            pitch += offpitch;
            if(targpitch < pitch) pitch = targpitch;
        }
        else if(targpitch < pitch)
        {
            pitch -= offpitch;
            if(targpitch > pitch) pitch = targpitch;
        }
        fixrange(yaw, pitch);
    }

    void deathcamyawpitch(gameent *d, float &yaw, float &pitch)
    {
        if(deathcamstyle)
        {
            gameent *a = deathcamstyle > 1 ? d : getclient(d->lastattacker);
            if(a)
            {
                vec dir = vec(a->headpos(-a->height*0.5f)).sub(camera1->o).normalize();
                vectoyawpitch(dir, camera1->aimyaw, camera1->aimpitch);
                float speed = (float(curtime)/1000.f)*deathcamspeed;
                if(deathcamspeed > 0) scaleyawpitch(yaw, pitch, camera1->aimyaw, camera1->aimpitch, speed, speed*4.f);
                else { yaw = camera1->aimyaw; pitch = camera1->aimpitch; }
            }
        }
    }

    void cameraplayer()
    {
        if(player1->state != CS_WAITING && player1->state != CS_SPECTATOR && player1->state != CS_DEAD && !tvmode())
        {
            player1->aimyaw = camera1->yaw;
            player1->aimpitch = camera1->pitch;
            fixrange(player1->aimyaw, player1->aimpitch);
            if(lastcamera && mousestyle() >= 1 && !hud::hasinput())
            {
                physent *d = mousestyle() != 2 ? player1 : camera1;
                float amt = clamp(float(lastmillis-lastcamera)/100.f, 0.f, 1.f)*panspeed();
                float zone = float(deadzone())/200.f, cx = cursorx-0.5f, cy = 0.5f-cursory;
                if(cx > zone || cx < -zone) d->yaw += ((cx > zone ? cx-zone : cx+zone)/(1.f-zone))*amt;
                if(cy > zone || cy < -zone) d->pitch += ((cy > zone ? cy-zone : cy+zone)/(1.f-zone))*amt;
                fixfullrange(d->yaw, d->pitch, d->roll, false);
            }
        }
    }

    bool camrefresh(cament *cam, bool renew = false)
    {
        bool rejigger = false;
        if(cam->player && cam->type != cament::PLAYER)
        {
            loopv(cameras) if(cameras[i]->type == cament::PLAYER && cameras[i]->player == cam->player)
            {
                cam = cameras[i];
                rejigger = true;
                break;
            }
            if(!rejigger) cam->player = NULL;
        }
        if(renew || rejigger || (cam->player && focus != cam->player) || (!cam->player && focus != player1))
        {
            if(cam->player)
            {
                focus = cam->player;
                follow = cam->id;
            }
            else
            {
                focus = player1;
                follow = 0;
            }
            return true;
        }
        return false;
    }

    bool camupdate(cament *c, const vec &pos, bool update = false)
    {
        float foglevel = float(fog*2/3);
        c->reset(true);
        if(c->player && (c->player->state == CS_DEAD || c->player->state == CS_WAITING) && !c->player->lastdeath) return false;
        loopj(c->player ? 1 : 2)
        {
            int players = c->player ? 1 : 0;
            loopv(cameras) if(c != cameras[i])
            {
                cament *cam = cameras[i];
                switch(cam->type)
                {
                    case cament::ENTITY: continue;
                    case cament::PLAYER:
                    {
                        if(!cam->player || cam->player->state != CS_ALIVE || (c->player && cam->player && c->player == cam->player)) continue;
                        break;
                    }
                    default: break;
                }
                vec trg, from = cam->o;
                float dist = pos.dist(from), fogdist = min(c->maxdist, foglevel);
                if(dist >= c->mindist && dist <= fogdist)
                {
                    bool hassight = false;
                    if(j && !c->cansee) hassight = true; // rejigger and get a direction
                    else
                    {
                        float yaw = c->player ? c->player->yaw : camera1->yaw, pitch = c->player ? c->player->pitch : camera1->pitch;
                        if(!c->player && (update || j))
                        {
                            vec dir = from;
                            if(c->cansee) dir.add(vec(c->dir).div(c->cansee));
                            dir.sub(pos).normalize();
                            vectoyawpitch(dir, yaw, pitch);
                        }
                        float x = fmod(fabs(asin((from.z-pos.z)/dist)/RAD-pitch), 360);
                        float y = fmod(fabs(-atan2(from.x-pos.x, from.y-pos.y)/RAD-yaw), 360);
                        if(min(x, 360-x) <= curfov && min(y, 360-y) <= fovy) hassight = true;
                    }
                    if(hassight && raycubelos(pos, from, trg))
                    {
                        if(cam->type == cament::PLAYER) players++;
                        c->cansee++;
                        c->dir.add(cam->o);
                        c->score += dist;
                    }
                }
            }
            if(c->cansee > 0)
            {
                if(c->cansee > 1)
                {
                    c->dir.div(c->cansee);
                    c->score /= c->cansee;
                }
                if(c->player) c->cansee++;
                if(players || j) return players!=0;
            }
            c->dir = c->olddir;
            c->reset(true);
        }
        c->dir = c->olddir;
        return false;
    }

    vec camerapos(physent *d)
    {
        vec pos = d->headpos();
        if(firstpersonbob && d == focus && !intermission && !thirdpersonview(true))
        {
            vec dir;
            vecfromyawpitch(d->yaw, 0, 0, 1, dir);
            float steps = bobdist/firstpersonbobstep*M_PI;
            dir.mul(firstpersonbobside*cosf(steps));
            dir.z = firstpersonbobup*(fabs(sinf(steps)) - 1);
            pos.add(dir);
        }
        return pos;
    }

    bool cameratv()
    {
        if(!tvmode(false)) return false;
        if(cameras.empty())
        {
            loopv(entities::ents)
            {
                gameentity &e = *(gameentity *)entities::ents[i];
                if(e.type!=PLAYERSTART && e.type!=LIGHT && e.type<MAPSOUND) continue;
                //if(e.type==MAPMODEL) continue;
                cament *c = cameras.add(new cament);
                c->o = e.o;
                c->o.z += max(enttype[e.type].radius, 2);
                c->type = cament::ENTITY;
                c->id = i;
            }
            gameent *d = NULL;
            int numdyns = numdynents();
            loopi(numdyns-1) if((d = (gameent *)iterdynents(i+1)) != NULL && (d->type == ENT_PLAYER || d->type == ENT_AI) && d->aitype < AI_START)
            {
                cament *c = cameras.add(new cament);
                c->o = camerapos(d);
                c->type = cament::PLAYER;
                c->id = i+1;
                c->player = d;
            }
            if(m_capture(gamemode)) capture::checkcams(cameras);
            else if(m_defend(gamemode)) defend::checkcams(cameras);
            else if(m_bomber(gamemode)) bomber::checkcams(cameras);
        }
        if(cameras.empty()) return false;
        loopv(cameras) switch(cameras[i]->type)
        {
            case cament::PLAYER:
            {
                if(cameras[i]->player || ((cameras[i]->player = (gameent *)iterdynents(cameras[i]->id))))
                {
                    cameras[i]->o = camerapos(cameras[i]->player);
                    vecfromyawpitch(cameras[i]->player->yaw, cameras[i]->player->pitch, 1, 0, cameras[i]->dir);
                }
            }
            default:
            {
                if(cameras[i]->type != cament::PLAYER && cameras[i]->player) cameras[i]->player = NULL;
                if(m_capture(gamemode)) capture::updatecam(cameras[i]);
                else if(m_defend(gamemode)) defend::updatecam(cameras[i]);
                else if(m_bomber(gamemode)) bomber::updatecam(cameras[i]);
                break;
            }
        }
        int millis = lasttvchg ? lastmillis-lasttvchg : 0;
        bool force = spectvmaxtime && millis >= spectvmaxtime,
             renew = force || !lasttvcam || lastmillis-lasttvcam >= spectvtime,
             override = !lasttvchg || millis >= spectvmintime;
        float amt = float(millis)/float(spectvmaxtime);
        cament *cam = cameras[0];
        camrefresh(cam);
        int lasttype = cam->type, lastid = cam->id;
        bool updated = camupdate(cam, cam->pos(amt));
        if(renew || (!updated && override))
        {
            loopv(cameras) if(!camupdate(cameras[i], cameras[i]->o, true)) cameras[i]->reset();
            if(!updated && override) renew = force = true;
        }
        if(renew)
        {
            if(force)
            {
                cam->ignore = true;
                if(cam->player) loopv(cameras) if(cameras[i]->player == cam->player)
                    cameras[i]->ignore = true;
            }
            cameras.sort(cament::camsort);
            if(force) loopv(cameras) if(cameras[i]->ignore) cameras[i]->ignore = false;
            cam->current = false;
            cam = cameras[0];
            cam->current = true;
            lasttvcam = lastmillis;
        }
        camrefresh(cam, renew);
        if(!lasttvchg || cam->type != lasttype || cam->id != lastid)
        {
            lasttvchg = lastmillis;
            if(!renew) renew = true;
            cam->moveto = NULL;
            if(cam->type == cament::ENTITY)
            {
                vector<cament *> mcams;
                mcams.reserve(cameras.length());
                mcams.put(cameras.getbuf(), cameras.length());
                while(mcams.length())
                {
                    cament *mcam = mcams.removeunordered(rnd(mcams.length()));
                    if(mcam->type == cament::ENTITY)
                    {
                        vec ray = vec(mcam->o).sub(cam->o);
                        float mag = ray.magnitude();
                        ray.mul(1.0f/mag);
                        if(raycube(cam->o, ray, mag, RAY_CLIPMAT|RAY_POLY) >= mag)
                        {
                            cam->moveto = mcam;
                            break;
                        }
                    }
                }
            }
            amt = 0;
        }
        else if(renew) renew = false;
        camera1->o = cam->pos(amt);
        if(focus != player1)
        {
            camera1->yaw = camera1->aimyaw = focus->yaw;
            camera1->pitch = camera1->aimpitch = focus->pitch;
            camera1->roll = focus->roll;
        }
        else
        {
            vectoyawpitch(vec(cam->dir).sub(camera1->o).normalize(), camera1->aimyaw, camera1->aimpitch);
            if(!renew && spectvyawspeed > 0)
            {
                float speed = float(curtime)/1000.f;
                scaleyawpitch(camera1->yaw, camera1->pitch, camera1->aimyaw, camera1->aimpitch, speed*spectvyawspeed, speed*spectvpitchspeed, spectvrotate);
            }
            else
            {
                camera1->yaw = camera1->aimyaw;
                camera1->pitch = camera1->aimpitch;
            }
        }
        if(cam->type == cament::AFFINITY && followdist > 0)
        {
            vec dir; vecfromyawpitch(camera1->yaw, camera1->pitch, -1, 0, dir);
            physics::movecamera(camera1, dir, followdist, 1.0f);
        }
        camera1->resetinterp();
        return true;
    }

    void checkcamera()
    {
        camera1 = &camera;
        if(camera1->type != ENT_CAMERA)
        {
            camera1->reset();
            camera1->type = ENT_CAMERA;
            camera1->collidetype = COLLIDE_AABB;
            camera1->state = CS_ALIVE;
            camera1->height = camera1->zradius = camera1->radius = camera1->xradius = camera1->yradius = 2;
        }
        if((focus->state != CS_WAITING && focus->state != CS_SPECTATOR) || tvmode())
        {
            camera1->vel = vec(0, 0, 0);
            camera1->move = camera1->strafe = 0;
        }
        if(!cameratv())
        {
            if(focus->state == CS_DEAD)
            {
                deathcamyawpitch(focus, camera1->yaw, camera1->pitch);
                camera1->aimyaw = camera1->yaw;
                camera1->aimpitch = camera1->pitch;
            }
            else if(focus->state >= CS_SPECTATOR)
            {
                camera1->move = player1->move;
                camera1->strafe = player1->strafe;
                physics::move(camera1, 10, true);
            }
        }
        if(focus->state == CS_SPECTATOR)
        {
            player1->aimyaw = player1->yaw = camera1->yaw;
            player1->aimpitch = player1->pitch = camera1->pitch;
            player1->o = camera1->o;
            player1->resetinterp();
        }
    }

    float calcroll(gameent *d)
    {
        bool thirdperson = d != focus || thirdpersonview(true);
        float r = thirdperson ? 0 : d->roll, wobble = float(rnd(quakewobble)-quakewobble/2)*(float(min(d->quake, 100))/100.f);
        switch(d->state)
        {
            case CS_SPECTATOR: case CS_WAITING: r = wobble*0.5f; break;
            case CS_ALIVE: if(physics::iscrouching(d)) wobble *= 0.5f; r += wobble; break;
            case CS_DEAD: r += wobble; break;
            default: break;
        }
        return r;
    }

    void calcangles(physent *c, gameent *d, bool bob = true)
    {
        c->roll = calcroll(d);
        if(bob && firstpersonbob && !thirdperson && !intermission)
        {
            vec dir; vecfromyawpitch(c->yaw, c->pitch, 1, 0, dir);
            float steps = bobdist/firstpersonbobstep*M_PI, dist = raycube(c->o, dir, firstpersonbobfocusmaxdist, RAY_CLIPMAT|RAY_POLY), yaw, pitch;
            if(dist < 0 || dist > firstpersonbobfocusmaxdist) dist = firstpersonbobfocusmaxdist;
            else if(dist < firstpersonbobfocusmindist) dist = firstpersonbobfocusmindist;
            vectoyawpitch(vec(firstpersonbobside*cosf(steps), dist, firstpersonbobup*(fabs(sinf(steps)) - 1)), yaw, pitch);
            c->yaw -= yaw*firstpersonbobfocus;
            c->pitch -= pitch*firstpersonbobfocus;
            c->roll += (1 - firstpersonbobfocus)*firstpersonbobroll*cosf(steps);
            fixfullrange(c->yaw, c->pitch, c->roll, false);
        }
    }

    void resetcamera()
    {
        lastcamera = 0;
        zoomset(false, 0);
        if(!focus) focus = player1;
        checkcamera();
        camera1->o = camerapos(focus);
        camera1->yaw = focus->yaw;
        camera1->pitch = focus->pitch;
        camera1->roll = calcroll(focus);
        camera1->resetinterp();
        focus->resetinterp();
    }

    void resetworld()
    {
        follow = 0; focus = player1;
        hud::showscores(false);
        cleargui();
    }

    void resetstate()
    {
        resetworld();
        resetcamera();
    }

    void updateworld()      // main game update loop
    {
        if(connected())
        {
            if(!maptime) { maptime = -1; return; } // skip the first loop
            else if(maptime < 0)
            {
                maptime = max(lastmillis, 1);
                musicdone(false);
                RUNWORLD("on_start");
                return;
            }
            else if(!nosound && mastervol && musicvol)
            {
                int type = m_edit(gamemode) ? musicedit : musictype;
                if(type && !playingmusic())
                {
                    defformatstring(musicfile)("%s", mapmusic);
                    if(*musicdir && (type == 2 || type == 5 || ((type == 1 || type == 4) && (!*musicfile || !fileexists(findfile(musicfile, "r"), "r")))))
                    {
                        vector<char *> files;
                        listfiles(musicdir, NULL, files);
                        while(!files.empty())
                        {
                            int r = rnd(files.length());
                            formatstring(musicfile)("%s/%s", musicdir, files[r]);
                            if(files[r][0] != '.' && playmusic(musicfile, type >= 4 ? "music" : NULL)) break;
                            else files.remove(r);
                        }
                    }
                    else if(*musicfile) playmusic(musicfile, type >= 4 ? "music" : NULL);
                }
            }
        }
        if(!curtime) { gets2c(); if(player1->clientnum >= 0) client::c2sinfo(); return; }

        if(!*player1->name && !menuactive()) showgui("profile", -1);
        if(connected())
        {
            player1->conopen = commandmillis > 0 || hud::hasinput(true);

            gameent *d = NULL;
            bool allow = player1->state >= CS_SPECTATOR, found = false;
            int numdyns = numdynents();
            loopi(numdyns) if((d = (gameent *)iterdynents(i)) != NULL)
            {
                if(d->state != CS_SPECTATOR && allow && i == follow)
                {
                    if(focus != d)
                    {
                        focus = d;
                        resetcamera();
                    }
                    found = true;
                }
                if(d->type == ENT_PLAYER || d->type == ENT_AI)
                {
                    checkoften(d, d == player1 || d->ai);
                    if(d == player1)
                    {
                        int state = d->weapstate[d->weapselect];
                        if(WEAP(d->weapselect, zooms))
                        {
                            if(state == WEAP_S_PRIMARY || state == WEAP_S_SECONDARY || (state == WEAP_S_RELOAD && lastmillis-d->weaplast[d->weapselect] >= max(d->weapwait[d->weapselect]-zoomtime, 1)))
                                state = WEAP_S_IDLE;
                        }
                        if(zooming && (!WEAP(d->weapselect, zooms) || state != WEAP_S_IDLE)) zoomset(false, lastmillis);
                        else if(WEAP(d->weapselect, zooms) && state == WEAP_S_IDLE && zooming != d->action[AC_ALTERNATE])
                            zoomset(d->action[AC_ALTERNATE], lastmillis);
                    }
                }
            }
            if((!found || !allow) && focus != player1)
            {
                focus = player1;
                resetcamera();
            }
            if(allowmove(player1)) cameraplayer();
            else player1->stopmoving(player1->state != CS_WAITING && player1->state != CS_SPECTATOR);

            physics::update();
            ai::navigate();
            projs::update();
            ai::update();
            if(!intermission)
            {
                entities::update();
                if(player1->state == CS_ALIVE) weapons::shoot(player1, worldpos);
            }
            otherplayers();
            if(m_arena(gamemode, mutators) && player1->state != CS_SPECTATOR && player1->loadweap[0] < 0 && !client::waiting() && !menuactive())
                showgui("loadout", -1);
        }
        else if(!menuactive()) showgui("main", -1);

        gets2c();
        adjustscaled(int, hud::damageresidue, hud::damageresiduefade);
        if(connected())
        {
            flushdamagemerges();
            if(player1->state == CS_DEAD || player1->state == CS_WAITING)
            {
                if(player1->ragdoll) moveragdoll(player1, true);
                else if(lastmillis-player1->lastpain < 5000)
                    physics::move(player1, 10, true);
            }
            else
            {
                if(player1->ragdoll) cleanragdoll(player1);
                if(player1->state == CS_EDITING) physics::move(player1, 10, true);
                else if(!intermission && player1->state == CS_ALIVE)
                {
                    physics::move(player1, 10, true);
                    entities::checkitems(player1);
                    weapons::checkweapons(player1);
                }
                if(!intermission) addsway(focus);
            }
            checkcamera();
            if(hud::canshowscores()) hud::showscores(true);
        }

        if(player1->clientnum >= 0) client::c2sinfo();
    }

    void recomputecamera(int w, int h)
    {
        fixview(w, h);
        checkcamera();
        if(!client::waiting())
        {
            if(!lastcamera && mousestyle() == 2 && focus->state != CS_WAITING && focus->state != CS_SPECTATOR)
            {
                camera1->yaw = focus->aimyaw = focus->yaw;
                camera1->pitch = focus->aimpitch = focus->pitch;
            }

            bool self = thirdpersonview(true) && thirdpersonaiming && focus != player1 && !tvmode(), bob = false;
            if(!self && (focus->state == CS_DEAD || focus->state >= CS_SPECTATOR))
            {
                camera1->aimyaw = camera1->yaw;
                camera1->aimpitch = camera1->pitch;
            }
            else
            {
                camera1->o = camerapos(focus);
                if(mousestyle() <= 1)
                    findorientation(camera1->o, (self ? player1 : focus)->yaw, (self ? player1 : focus)->pitch, worldpos);
                camera1->aimyaw = self ? player1->yaw : (mousestyle() <= 1 ? focus->yaw : focus->aimyaw);
                camera1->aimpitch = self ? player1->pitch : (mousestyle() <= 1 ? focus->pitch : focus->aimpitch);
                if(thirdpersonview(true))
                {
                    float dist = focus != player1 ? followdist : thirdpersondist;
                    if(dist > 0)
                    {
                        vec dir; vecfromyawpitch(camera1->aimyaw, camera1->aimpitch, -1, 0, dir);
                        physics::movecamera(camera1, dir, dist, 1.0f);
                    }
                }
                camera1->resetinterp();

                switch(mousestyle())
                {
                    case 0:
                    case 1:
                    {
                        camera1->yaw = (self ? player1 : focus)->yaw;
                        camera1->pitch = (self ? player1 : focus)->pitch;
                        if(mousestyle())
                        {
                            camera1->aimyaw = camera1->yaw;
                            camera1->aimpitch = camera1->pitch;
                        }
                        break;
                    }
                    case 2:
                    {
                        float yaw, pitch;
                        vectoyawpitch(cursordir, yaw, pitch);
                        fixrange(yaw, pitch);
                        findorientation(camera1->o, yaw, pitch, worldpos);
                        if(focus == player1 && allowmove(player1))
                        {
                            player1->yaw = yaw;
                            player1->pitch = pitch;
                        }
                        break;
                    }
                }
                fixfullrange(camera1->yaw, camera1->pitch, camera1->roll, false);
                fixrange(camera1->aimyaw, camera1->aimpitch);

                bob = true;
            }

            calcangles(camera1, focus, bob);

            vecfromyawpitch(camera1->yaw, camera1->pitch, 1, 0, camdir);
            vecfromyawpitch(camera1->yaw, 0, 0, -1, camright);
            vecfromyawpitch(camera1->yaw, camera1->pitch+90, 1, 0, camup);

            camera1->inmaterial = lookupmaterial(camera1->o);
            camera1->inliquid = isliquid(camera1->inmaterial&MATF_VOLUME);

            switch(camera1->inmaterial)
            {
                case MAT_WATER:
                {
                    if(!issound(liquidchan))
                        playsound(S_UNDERWATER, camera1->o, camera1, SND_LOOP|SND_NOATTEN|SND_NODELAY|SND_NOCULL, -1, -1, -1, &liquidchan);
                    break;
                }
                default:
                {
                    if(issound(liquidchan)) removesound(liquidchan);
                    liquidchan = -1;
                    break;
                }
            }

            lastcamera = lastmillis;
        }
    }

    VAR(0, animoverride, -1, 0, ANIM_MAX-1);
    VAR(0, testanims, 0, 0, 1);

    int numanims() { return ANIM_MAX; }

    void findanims(const char *pattern, vector<int> &anims)
    {
        loopi(sizeof(animnames)/sizeof(animnames[0]))
            if(*animnames[i] && matchanim(animnames[i], pattern))
                anims.add(i);
    }

    void renderclient(gameent *d, bool third, float trans, float size, int team, modelattach *attachments, bool secondary, int animflags, int animdelay, int lastaction, bool early)
    {
        const char *mdl = playermodels[0][third ? 0 : 1];
        if(d->aitype >= AI_START) mdl = aistyle[d->aitype%AI_MAX].playermodel[third ? 0 : 1];
        else  mdl = playermodels[d->model%NUMPLAYERMODELS][third ? 0 : 1];
        float yaw = d->yaw, pitch = d->pitch, roll = calcroll(focus);
        vec o = third ? d->feetpos() : camerapos(d);
        if(!third && firstpersonsway && !intermission)
        {
            vec dir; vecfromyawpitch(d->yaw, 0, 0, 1, dir);
            float steps = swaydist/(firstpersonbob ? firstpersonbobstep : firstpersonswaystep)*M_PI;
            dir.mul(firstpersonswayside*cosf(steps));
            dir.z = firstpersonswayup*(fabs(sinf(steps)) - 1);
            o.add(dir).add(swaydir).add(swaypush);
        }

        int anim = animflags, basetime = lastaction, basetime2 = 0;
        if(animoverride)
        {
            anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
            basetime = 0;
        }
        else
        {
            if(secondary && allowmove(d) && aistyle[d->aitype].canmove)
            {
                if(physics::hover(d))
                {
                    if(d->canshoot(WEAP_MELEE, HIT_ALT, m_weapon(gamemode, mutators), lastmillis, (1<<WEAP_S_RELOAD)) && d->action[AC_SPECIAL] && (!impulsemeleestyle || (d->impulse[IM_TYPE] && lastmillis-d->impulse[IM_TIME] <= impulsemeleedelay)))
                    {
                        anim |= ANIM_FLYKICK<<ANIM_SECONDARY;
                        basetime2 = d->actiontime[AC_SPECIAL];
                    }
                    else if(d->impulse[IM_TYPE] == IM_T_MELEE)
                    {
                        anim |= ANIM_FLYKICK<<ANIM_SECONDARY;
                        basetime2 = d->impulse[IM_TIME];
                    }
                    else if(d->move>0) anim |= (ANIM_HOVER_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_HOVER_LEFT : ANIM_HOVER_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= (ANIM_HOVER_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else anim |= (ANIM_HOVER_UP|ANIM_LOOP)<<ANIM_SECONDARY;
                }
                else if(physics::liquidcheck(d) && d->physstate <= PHYS_FALL)
                    anim |= ((d->move || d->strafe || d->vel.z+d->falling.z>0 ? int(ANIM_SWIM) : int(ANIM_SINK))|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->physstate == PHYS_FALL && !d->turnside && !d->onladder && d->impulse[IM_TYPE] != IM_T_NONE && lastmillis-d->impulse[IM_TIME] <= 1000)
                {
                    basetime2 = d->impulse[IM_TIME];
                    if(d->impulse[IM_TYPE] == IM_T_KICK) anim |= ANIM_WALL_JUMP<<ANIM_SECONDARY;
                    else if(d->canshoot(WEAP_MELEE, HIT_ALT, m_weapon(gamemode, mutators), lastmillis, (1<<WEAP_S_RELOAD)) && d->action[AC_SPECIAL] && (!impulsemeleestyle || (d->impulse[IM_TYPE] && lastmillis-d->impulse[IM_TIME] <= impulsemeleedelay)))
                    {
                        anim |= ANIM_FLYKICK<<ANIM_SECONDARY;
                        basetime2 = d->actiontime[AC_SPECIAL];
                    }
                    else if(d->impulse[IM_TYPE] == IM_T_MELEE)
                    {
                        anim |= ANIM_FLYKICK<<ANIM_SECONDARY;
                        basetime2 = d->impulse[IM_TIME];
                    }
                    else if(d->move>0) anim |= ANIM_DASH_FORWARD<<ANIM_SECONDARY;
                    else if(d->strafe) anim |= (d->strafe>0 ? ANIM_DASH_LEFT : ANIM_DASH_RIGHT)<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= ANIM_DASH_BACKWARD<<ANIM_SECONDARY;
                    else anim |= ANIM_DASH_UP<<ANIM_SECONDARY;
                }
                else if(d->physstate == PHYS_FALL && !d->turnside && !d->onladder && ((d->impulse[IM_JUMP] && d->timeinair) || d->timeinair >= PHYSMILLIS))
                {
                    if(d->impulse[IM_JUMP] && d->timeinair) basetime2 = d->impulse[IM_JUMP];
                    else if(d->timeinair) basetime2 = lastmillis-d->timeinair;
                    if(d->canshoot(WEAP_MELEE, HIT_ALT, m_weapon(gamemode, mutators), lastmillis, (1<<WEAP_S_RELOAD)) && d->action[AC_SPECIAL] && (!impulsemeleestyle || (d->impulse[IM_TYPE] && lastmillis-d->impulse[IM_TIME] <= impulsemeleedelay)))
                    {
                        anim |= ANIM_FLYKICK<<ANIM_SECONDARY;
                        basetime2 = d->actiontime[AC_SPECIAL];
                    }
                    else if(d->action[AC_CROUCH] || d->actiontime[AC_CROUCH]<0)
                    {
                        if(d->move>0) anim |= ANIM_CROUCH_JUMP_FORWARD<<ANIM_SECONDARY;
                        else if(d->strafe) anim |= (d->strafe>0 ? ANIM_CROUCH_JUMP_LEFT : ANIM_CROUCH_JUMP_RIGHT)<<ANIM_SECONDARY;
                        else if(d->move<0) anim |= ANIM_CROUCH_JUMP_BACKWARD<<ANIM_SECONDARY;
                        else anim |= ANIM_CROUCH_JUMP<<ANIM_SECONDARY;
                    }
                    else if(d->move>0) anim |= ANIM_JUMP_FORWARD<<ANIM_SECONDARY;
                    else if(d->strafe) anim |= (d->strafe>0 ? ANIM_JUMP_LEFT : ANIM_JUMP_RIGHT)<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= ANIM_JUMP_BACKWARD<<ANIM_SECONDARY;
                    else anim |= ANIM_JUMP<<ANIM_SECONDARY;
                    if(!basetime2) anim |= ANIM_END<<ANIM_SECONDARY;
                }
                else if(d->turnside) anim |= ((d->turnside>0 ? ANIM_WALL_RUN_LEFT : ANIM_WALL_RUN_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(physics::sliding(d, true)) anim |= (ANIM_POWERSLIDE|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(physics::sprinting(d))
                {
                    if(d->move>0) anim |= (ANIM_IMPULSE_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_IMPULSE_LEFT : ANIM_IMPULSE_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= (ANIM_IMPULSE_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                }
                else if(d->action[AC_CROUCH] || d->actiontime[AC_CROUCH]<0)
                {
                    if(d->move>0) anim |= (ANIM_CRAWL_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_CRAWL_LEFT : ANIM_CRAWL_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= (ANIM_CRAWL_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else anim |= (ANIM_CROUCH|ANIM_LOOP)<<ANIM_SECONDARY;
                }
                else if(d->move>0) anim |= (ANIM_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_LEFT : ANIM_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->move<0) anim |= (ANIM_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
            }

            if((anim>>ANIM_SECONDARY)&ANIM_INDEX) switch(anim&ANIM_INDEX)
            {
                case ANIM_IDLE: case ANIM_MELEE: case ANIM_PISTOL: case ANIM_SWORD:
                case ANIM_SHOTGUN: case ANIM_SMG: case ANIM_FLAMER: case ANIM_PLASMA:
                case ANIM_RIFLE: case ANIM_GRENADE: case ANIM_ROCKET:
                {
                    anim = (anim>>ANIM_SECONDARY) | ((anim&((1<<ANIM_SECONDARY)-1))<<ANIM_SECONDARY);
                    swap(basetime, basetime2);
                    break;
                }
                default: break;
            }
        }

        if(third && testanims && d == focus) yaw = 0; else yaw += 90;

        if(d->ragdoll && (!ragdolls || (anim&ANIM_INDEX)!=ANIM_DYING)) cleanragdoll(d);

        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;

        int flags = MDL_LIGHT;
        if(d != focus && !(anim&ANIM_RAGDOLL)) flags |= MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_CULL_QUERY;
        if(d->type == ENT_PLAYER)
        {
            if(!early && third) flags |= MDL_FULLBRIGHT;
        }
        else flags |= MDL_CULL_DIST;
        if(early) flags |= MDL_NORENDER;
        else if(third && (anim&ANIM_INDEX)!=ANIM_DEAD) flags |= MDL_DYNSHADOW;
        dynent *e = third ? (dynent *)d : (dynent *)&avatarmodel;
        if(e->light.millis != lastmillis)
        {
            e->light.material[0] = bvec(getcolour(d, playerovertone));
            e->light.material[1] = bvec(getcolour(d, playerundertone));
            if(renderpath != R_FIXEDFUNCTION && isweap(d->weapselect) && (WEAP2(d->weapselect, sub, false) || WEAP2(d->weapselect, sub, true)) && WEAP(d->weapselect, max) > 1)
            {
                int ammo = d->ammo[d->weapselect], maxammo = WEAP(d->weapselect, max);
                float scale = 1;
                switch(d->weapstate[d->weapselect])
                {
                    case WEAP_S_RELOAD:
                    {
                        int millis = lastmillis-d->weaplast[d->weapselect], check = d->weapwait[d->weapselect]/2;
                        scale = millis >= check ? float(millis-check)/check : 0.f;
                        if(d->weapload[d->weapselect] > 0)
                            scale = max(scale, float(ammo - d->weapload[d->weapselect])/maxammo);
                        break;
                    }
                    default: scale = float(ammo)/maxammo; break;
                }
                uchar wepmat = uchar(255*scale);
                e->light.material[2] = bvec(wepmat, wepmat, wepmat);
            }
            else e->light.material[2] = bvec(255, 255, 255);
            e->light.effect = vec(0, 0, 0);
            if(d->lastbuff)
            {
                flags |= MDL_LIGHTFX;
                int millis = lastmillis%1000;
                float amt = millis <= 500 ? 1.f-(millis/500.f) : (millis-500)/500.f;
                flashcolour(e->light.material[1].r, e->light.material[1].g, e->light.material[1].b, uchar(64), uchar(255), uchar(255), amt);
                flashcolour(e->light.effect.r, e->light.effect.g, e->light.effect.b, 0.25f, 1.f, 1.f, amt);
                //vec col = vec(0.25f, 1.f, 1.f).mul(amt);
                //e->light.material[1] = bvec::fromcolor(e->light.material[1].tocolor().max(col));
                //e->light.effect.max(col);
            }
            if(burntime && d->burning(lastmillis, burntime))
            {
                flags |= MDL_LIGHTFX;
                vec col = burncolour(d);
                e->light.material[1] = bvec::fromcolor(e->light.material[1].tocolor().max(col));
                e->light.effect.max(col);
            }
            if(bleedtime && d->bleeding(lastmillis, bleedtime))
            {
                flags |= MDL_LIGHTFX;
                int millis = lastmillis%1000;
                float amt = millis <= 500 ? millis/500.f : 1.f-((millis-500)/500.f);
                flashcolour(e->light.material[1].r, e->light.material[1].g, e->light.material[1].b, uchar(255), uchar(52), uchar(52), amt);
                flashcolour(e->light.effect.r, e->light.effect.g, e->light.effect.b, 1.f, 0.2f, 0.2f, amt);
                //vec col = vec(1, 0.2f, 0.2f).mul(amt);
                //e->light.material[1] = bvec::fromcolor(e->light.material[1].tocolor().max(col));
                //e->light.effect.max(col);
            }
        }
        rendermodel(NULL, mdl, anim, o, yaw, pitch, roll, flags, e, attachments, basetime, basetime2, trans, size);
    }

    void renderabovehead(gameent *d, float trans)
    {
        vec pos = d->abovehead(d->state != CS_DEAD ? 1 : -1);
        float blend = aboveheadblend*trans;
        if(aboveheadnames && d != player1)
        {
            const char *name = colorname(d, NULL, d->aitype == AI_NONE ? "<super>" : "<emphasis>");
            if(name && *name)
            {
                pos.z += aboveheadnamesize/2;
                part_textcopy(pos, name, PART_TEXT, 1, 0xFFFFFF, aboveheadnamesize, blend);
                pos.z += aboveheadnamesize/2+0.5f;
            }
        }
        if(aboveheadstatus)
        {
            Texture *t = NULL;
            int colour = getcolour(d, playerovertone);
            if(d->state == CS_DEAD || d->state == CS_WAITING) t = textureload(hud::deadtex, 3);
            else if(d->state == CS_ALIVE)
            {
                if(d->conopen) t = textureload(hud::chattex, 3);
                else if(m_team(gamemode, mutators) && aboveheadteam > (d->team != focus->team ? 1 : 0))
                    t = textureload(hud::teamtexname(d->team), 3+max(hud::numteamkills()-hud::teamkillnum, 0));
                else
                {
                    if(d->dominating.find(focus) >= 0) t = textureload(hud::dominatingtex, 3);
                    else if(d->dominated.find(focus) >= 0) t = textureload(hud::dominatedtex, 3);
                    colour = pulsecols[2][clamp((lastmillis/100)%PULSECOLOURS, 0, PULSECOLOURS-1)];
                }
            }
            if(t && t != notexture)
            {
                pos.z += aboveheadstatussize/2;
                part_icon(pos, t, aboveheadstatussize, blend, 0, 0, 1, colour);
                pos.z += aboveheadstatussize/2+0.25f;
            }
        }
        if(aboveheadicons && d->state != CS_EDITING && d->state != CS_SPECTATOR) loopv(d->icons)
        {
            if(d->icons[i].type == eventicon::AFFINITY && !(aboveheadicons&2)) break;
            if(d->icons[i].type == eventicon::WEAPON && !(aboveheadicons&4)) break;
            if(d->icons[i].type == eventicon::CRITICAL && d->icons[i].value) continue;
            int millis = lastmillis-d->icons[i].millis;
            if(millis <= d->icons[i].fade)
            {
                Texture *t = textureload(hud::icontex(d->icons[i].type, d->icons[i].value));
                if(t && t != notexture)
                {
                    int olen = min(d->icons[i].length/5, 1000), ilen = olen/2, colour = 0xFFFFFF;
                    float skew = millis < ilen ? millis/float(ilen) : (millis > d->icons[i].fade-olen ? (d->icons[i].fade-millis)/float(olen) : 1.f),
                          size = aboveheadiconsize*skew, fade = blend*skew, nudge = size*2/3;
                    if(d->icons[i].type >= eventicon::SORTED)
                    {
                        switch(d->icons[i].type)
                        {
                            case eventicon::WEAPON: colour = WEAP(d->icons[i].value, colour); size = size*2/3; nudge = size; break;
                            case eventicon::AFFINITY:
                                if(m_bomber(gamemode))
                                {
                                    bvec pcol = bvec::fromcolor(bomber::pulsecolour());
                                    colour = (pcol.x<<16)|(pcol.y<<8)|pcol.z;
                                    size *= 0.75f;
                                }
                                else colour = TEAM(d->icons[i].value, colour);
                                // fall through
                            default: nudge *= 1.5f; break;
                        }
                    }
                    pos.z += nudge;
                    part_icon(pos, t, size, fade, 0, 0, 1, colour);
                    pos.z += nudge;
                }
            }
        }
    }

    void renderplayer(gameent *d, bool third, float trans, float size, bool early = false)
    {
        if(d->state == CS_SPECTATOR) return;
        if(trans <= 0.f || (d == focus && (third ? thirdpersonmodel : firstpersonmodel) < 1))
        {
            if(d->state == CS_ALIVE && rendernormally && (early || d != focus))
                trans = 1e-16f; // we need tag_muzzle/tag_waist
            else return; // screw it, don't render them
        }
        int team = m_fight(gamemode) && m_team(gamemode, mutators) ? d->team : TEAM_NEUTRAL,
            weap = d->weapselect, lastaction = 0, animflags = ANIM_IDLE|ANIM_LOOP, weapflags = animflags, weapaction = 0, animdelay = 0;
        bool secondary = false, showweap = isweap(weap) && (d->aitype < AI_START || aistyle[d->aitype].useweap);

        if(d->state == CS_DEAD || d->state == CS_WAITING)
        {
            showweap = false;
            animflags = ANIM_DYING|ANIM_NOPITCH;
            lastaction = d->lastpain;
            if(ragdolls)
            {
                if(!validragdoll(d, lastaction)) animflags |= ANIM_RAGDOLL;
            }
            else
            {
                int t = lastmillis-lastaction;
                if(t < 0) return;
                if(t > 1000) animflags = ANIM_DEAD|ANIM_LOOP|ANIM_NOPITCH;
            }
        }
        else if(d->state == CS_EDITING)
        {
            animflags = ANIM_EDIT|ANIM_LOOP;
            showweap = false;
        }
        else
        {
            secondary = third;
            if(showweap)
            {
                weapaction = lastaction = d->weaplast[weap];
                animdelay = d->weapwait[weap];
                switch(d->weapstate[weap])
                {
                    case WEAP_S_SWITCH:
                    case WEAP_S_USE:
                    {
                        if(lastmillis-d->weaplast[weap] <= d->weapwait[weap]/3)
                        {
                            if(!d->hasweap(d->lastweap, m_weapon(gamemode, mutators))) showweap = false;
                            else weap = d->lastweap;
                        }
                        else if(!d->hasweap(weap, m_weapon(gamemode, mutators))) showweap = false;
                        weapflags = animflags = ANIM_SWITCH+(d->weapstate[weap]-WEAP_S_SWITCH);
                        break;
                    }
                    case WEAP_S_JAM:
                    {
                        showweap = false;
                        weapflags = animflags = ANIM_JAM;
                        break;
                    }
                    case WEAP_S_POWER:
                    {
                        if(weaptype[weap].anim == ANIM_GRENADE) weapflags = animflags = weaptype[weap].anim+d->weapstate[weap];
                        else weapflags = animflags = weaptype[weap].anim|ANIM_LOOP;
                        break;
                    }
                    case WEAP_S_PRIMARY:
                    case WEAP_S_SECONDARY:
                    {
                        if(weaptype[weap].thrown[0] > 0 && (lastmillis-d->weaplast[weap] <= d->weapwait[weap]/2 || !d->hasweap(weap, m_weapon(gamemode, mutators))))
                            showweap = false;
                        weapflags = animflags = (weaptype[weap].anim+d->weapstate[weap])|ANIM_CLAMP;
                        break;
                    }
                    case WEAP_S_RELOAD:
                    {
                        if(!d->hasweap(weap, m_weapon(gamemode, mutators)) || (!w_reload(weap, m_weapon(gamemode, mutators)) && lastmillis-d->weaplast[weap] <= d->weapwait[weap]/3))
                            showweap = false;
                        weapflags = animflags = weaptype[weap].anim+d->weapstate[weap];
                        break;
                    }
                    case WEAP_S_IDLE: case WEAP_S_WAIT: default:
                    {
                        if(!d->hasweap(weap, m_weapon(gamemode, mutators))) showweap = false;
                        weapflags = animflags = weaptype[weap].anim|ANIM_LOOP;
                        break;
                    }
                }
            }
            if(third && (animflags&ANIM_IDLE) && lastmillis-d->lastpain <= 300)
            {
                secondary = third;
                lastaction = d->lastpain;
                animflags = ANIM_PAIN;
                animdelay = 300;
            }
        }
        if(!early && third && d->type == ENT_PLAYER && !shadowmapping && !envmapping) renderabovehead(d, trans);
        const char *weapmdl = isweap(weap) ? (third ? weaptype[weap].vwep : weaptype[weap].hwep) : "";
        bool hasweapon = showweap && *weapmdl;
        modelattach a[11]; int ai = 0;
        if(hasweapon) a[ai++] = modelattach("tag_weapon", weapmdl, weapflags, weapaction); // we could probably animate this too now..
        if(rendernormally && (early || d != focus))
        {
            const char *muzzle = "tag_weapon";
            if(hasweapon)
            {
                muzzle = "tag_muzzle";
                if(weaptype[weap].eject) a[ai++] = modelattach("tag_eject", &d->eject);
            }
            a[ai++] = modelattach(muzzle, &d->muzzle);
            a[ai++] = modelattach("tag_weapon", &d->origin);
            if(third && d->wantshitbox())
            {
                a[ai++] = modelattach("tag_head", &d->head);
                a[ai++] = modelattach("tag_torso", &d->torso);
                a[ai++] = modelattach("tag_waist", &d->waist);
                a[ai++] = modelattach("tag_ljet", &d->jet[0]);
                a[ai++] = modelattach("tag_rjet", &d->jet[1]);
                a[ai++] = modelattach("tag_bjet", &d->jet[2]);
            }
        }
        renderclient(d, third, trans, size, team, a[0].tag ? a : NULL, secondary, animflags, animdelay, lastaction, early);
    }

    void rendercheck(gameent *d)
    {
        d->checktags();
        if(rendernormally)
        {
            if(d->state == CS_ALIVE)
            {
                bool last = lastmillis-d->weaplast[d->weapselect] > 0,
                     powering = last && d->weapstate[d->weapselect] == WEAP_S_POWER,
                     reloading = last && d->weapstate[d->weapselect] == WEAP_S_RELOAD,
                     secondary = physics::secondaryweap(d);
                float amt = last ? (lastmillis-d->weaplast[d->weapselect])/float(d->weapwait[d->weapselect]) : 0.f;
                int colour = WEAPPCOL(d, d->weapselect, partcol, secondary);
                if(d->weapselect == WEAP_FLAMER && (!reloading || amt > 0.5f))
                {
                    float scale = powering ? 1.f+(amt*1.5f) : (d->weapstate[d->weapselect] == WEAP_S_IDLE ? 1.f : (reloading ? (amt-0.5f)*2 : amt));
                    part_create(PART_HINT, 1, d->ejectpos(d->weapselect), 0x1818A8, 0.5f*scale, min(0.65f*scale, 0.8f), 0, 0);
                    part_create(PART_FIREBALL, 1, d->ejectpos(d->weapselect), colour, 0.75f*scale, min(0.75f*scale, 0.95f), 0, 0);
                    regular_part_create(PART_FIREBALL, d->vel.magnitude() > 10 ? 30 : 75, d->ejectpos(d->weapselect), colour, 0.75f*scale, min(0.75f*scale, 0.95f), d->vel.magnitude() > 10 ? -40 : -10, 0);
                }
                if(d->weapselect == WEAP_RIFLE && WEAP(d->weapselect, laser) && !reloading)
                {
                    vec v, origin = d->originpos(), muzzle = d->muzzlepos(d->weapselect);
                    origin.z += 0.25f; muzzle.z += 0.25f;
                    float yaw, pitch;
                    vectoyawpitch(vec(muzzle).sub(origin).normalize(), yaw, pitch);
                    findorientation(d->o, d->yaw, d->pitch, v);
                    part_flare(origin, v, 1, PART_FLARE, colour, 0.5f*amt, amt);
                }
                if(d->weapselect == WEAP_SWORD || powering)
                {
                    static const struct powerfxs {
                        int type, parttype;
                        float size, radius;
                    } powerfx[WEAP_MAX] = {
                        { 0, 0, 0, 0 },
                        { 2, PART_SPARK, 0.1f, 1.5f },
                        { 4, PART_LIGHTNING_FLARE, 1, 1 },
                        { 2, PART_SPARK, 0.15f, 2 },
                        { 2, PART_SPARK, 0.1f, 2 },
                        { 2, PART_FIREBALL, 0.1f, 6 },
                        { 1, PART_PLASMA, 0.05f, 2 },
                        { 2, PART_PLASMA, 0.05f, 2.5f },
                        { 3, PART_PLASMA, 0.1f, 0.125f },
                        { 0, 0, 0, 0 },
                    };
                    switch(powerfx[d->weapselect].type)
                    {
                        case 1: case 2:
                        {
                            regularshape(powerfx[d->weapselect].parttype, 1+(amt*powerfx[d->weapselect].radius), colour, powerfx[d->weapselect].type == 2 ? 21 : 53, 5, 60+int(30*amt), d->muzzlepos(d->weapselect), powerfx[d->weapselect].size*max(amt, 0.25f), max(amt*0.25f, 0.05f), 1, 0, 5+(amt*5));
                            break;
                        }
                        case 3:
                        {
                            int interval = lastmillis%1000;
                            float fluc = powerfx[d->weapselect].size+(interval ? (interval <= 500 ? interval/500.f : (1000-interval)/500.f) : 0.f);
                            part_create(powerfx[d->weapselect].parttype, 1, d->originpos(), colour, (powerfx[d->weapselect].radius*max(amt, 0.25f))+fluc, max(amt, 0.1f));
                            break;
                        }
                        case 4:
                        {
                            part_flare(d->originpos(), d->muzzlepos(d->weapselect), 1, PART_LIGHTNING, colour, powerfx[d->weapselect].size, max(amt, 0.1f));
                            break;
                        }
                        case 0: default: break;
                    }
                }
                if(d->turnside || d->impulse[IM_JUMP] || physics::sliding(d)) impulseeffect(d, 1);
                if(physics::hover(d)) impulseeffect(d, 2);
            }
            if(burntime && d->burning(lastmillis, burntime))
            {
                int millis = lastmillis-d->lastburn;
                float pc = d->curscale, intensity = 0.5f+(rnd(50)/100.f), blend = (d != focus ? 0.5f : 0.f)+(rnd(50)/100.f);
                if(burntime-millis < burndelay) pc *= float(burntime-millis)/float(burndelay);
                else pc *= 0.75f+(float(millis%burndelay)/float(burndelay*4));
                vec pos = vec(d->o).sub(vec(rnd(11)-5, rnd(11)-5, d->height/2+rnd(5)-2).mul(pc));
                regular_part_create(PART_FIREBALL, max(burnfade, 100), pos, pulsecols[0][rnd(PULSECOLOURS)], d->height*0.75f*d->curscale*intensity*pc, blend*pc*burnblend, -10, 0);
            }
        }
    }

    void render()
    {
        startmodelbatches();
        gameent *d;
        int numdyns = numdynents();
        loopi(numdyns) if((d = (gameent *)iterdynents(i)) && d != focus) renderplayer(d, true, opacity(d, true), d->curscale);
        entities::render();
        projs::render();
        if(m_capture(gamemode)) capture::render();
        else if(m_defend(gamemode)) defend::render();
        else if(m_bomber(gamemode)) bomber::render();
        ai::render();
        if(rendernormally) loopi(numdyns) if((d = (gameent *)iterdynents(i)) && d != focus) d->cleartags();
        endmodelbatches();
        if(rendernormally) loopi(numdyns) if((d = (gameent *)iterdynents(i)) && d != focus) rendercheck(d);
    }

    void renderavatar(bool early)
    {
        if(rendernormally && early) focus->cleartags();
        if(thirdpersonview() || !rendernormally)
            renderplayer(focus, true, opacity(focus, thirdpersonview(true)), focus->curscale, early);
        else if(!thirdpersonview() && focus->state == CS_ALIVE)
            renderplayer(focus, false, opacity(focus, false), focus->curscale, early);
        if(rendernormally && early) rendercheck(focus);
    }

    bool clientoption(char *arg) { return false; }
}
#undef GAMEWORLD
