#define GAMEWORLD 1
#include "game.h"
namespace game
{
    int nextmode = G_EDITMODE, nextmuts = 0, gamemode = G_EDITMODE, mutators = 0, maptime = 0, timeremaining = 0,
        lastcamera = 0, lasttvcam = 0, lasttvchg = 0, lastzoom = 0, lastmousetype = 0, liquidchan = -1;
    bool intermission = false, prevzoom = false, zooming = false;
    float swayfade = 0, swayspeed = 0, swaydist = 0;
    vec swaydir(0, 0, 0), swaypush(0, 0, 0);
    string clientmap = "";

    gameent *player1 = new gameent(), *focus = player1;
    avatarent avatarmodel;
    vector<gameent *> players;
    vector<camstate> cameras;

    VAR(IDF_WORLD, numplayers, 0, 8, MAXCLIENTS);
    FVAR(IDF_WORLD, illumlevel, 0, 0, 2);
    VAR(IDF_WORLD, illumradius, 0, 0, INT_MAX-1);
    SVAR(IDF_WORLD, obitlava, "");
    SVAR(IDF_WORLD, obitwater, "");
    SVAR(IDF_WORLD, obitdeath, "");
    SVAR(IDF_WORLD, mapmusic, "");

    VAR(IDF_PERSIST, mouseinvert, 0, 0, 1);
    VAR(IDF_PERSIST, mouseabsolute, 0, 0, 1);
    VAR(IDF_PERSIST, mousetype, 0, 0, 2);
    VAR(IDF_PERSIST, mousedeadzone, 0, 10, 100);
    VAR(IDF_PERSIST, mousepanspeed, 1, 50, INT_MAX-1);

    VAR(IDF_PERSIST, thirdperson, 0, 0, 1);
    VAR(IDF_PERSIST, thirdpersonfollow, 0, 0, 1);
    VAR(IDF_PERSIST, dynlighteffects, 0, 2, 2);
    FVAR(IDF_PERSIST, playerblend, 0, 1, 1);

    VAR(IDF_PERSIST, thirdpersonmodel, 0, 1, 1);
    VAR(IDF_PERSIST, thirdpersonfov, 90, 120, 150);
    FVAR(IDF_PERSIST, thirdpersonblend, 0, 0.45f, 1);
    FVAR(IDF_PERSIST, thirdpersondist, -1000, 20, 1000);

    VAR(IDF_PERSIST, firstpersonmodel, 0, 1, 1);
    VAR(IDF_PERSIST, firstpersonfov, 90, 100, 150);
    VAR(IDF_PERSIST, firstpersonsway, 0, 1, 1);
    FVAR(IDF_PERSIST, firstpersonswaystep, 1, 28.0f, 100);
    FVAR(IDF_PERSIST, firstpersonswayside, 0, 0.05f, 1);
    FVAR(IDF_PERSIST, firstpersonswayup, 0, 0.06f, 1);
    FVAR(IDF_PERSIST, firstpersonblend, 0, 1, 1);
    FVAR(IDF_PERSIST, firstpersondist, -10000, -0.25f, 10000);
    FVAR(IDF_PERSIST, firstpersonshift, -10000, 0.3f, 10000);
    FVAR(IDF_PERSIST, firstpersonadjust, -10000, -0.07f, 10000);

    VAR(IDF_PERSIST, editfov, 1, 120, 179);
    VAR(IDF_PERSIST, specfov, 1, 120, 179);

    VAR(IDF_PERSIST, follow, 0, 0, INT_MAX-1);
    VARF(IDF_PERSIST, specmode, 0, 1, 1, follow = 0); // 0 = float, 1 = tv
    VARF(IDF_PERSIST, waitmode, 0, 1, 2, follow = 0); // 0 = float, 1 = tv in duel/survivor, 2 = tv always

    VAR(IDF_PERSIST, spectvtime, 1000, 10000, INT_MAX-1);
    FVAR(IDF_PERSIST, spectvspeed, 0, 0.5f, 1000);
    FVAR(IDF_PERSIST, spectvpitch, 0, 0.5f, 1000);
    FVAR(IDF_PERSIST, spectvbias, 0, 2.5f, 1000);

    VAR(IDF_PERSIST, deathcamstyle, 0, 1, 2); // 0 = no follow, 1 = follow attacker, 2 = follow self
    FVAR(IDF_PERSIST, deathcamspeed, 0, 2.f, 1000);

    FVAR(IDF_PERSIST, sensitivity, 1e-4f, 10.0f, 1000);
    FVAR(IDF_PERSIST, yawsensitivity, 1e-4f, 10.0f, 1000);
    FVAR(IDF_PERSIST, pitchsensitivity, 1e-4f, 7.5f, 1000);
    FVAR(IDF_PERSIST, mousesensitivity, 1e-4f, 1.0f, 1000);
    FVAR(IDF_PERSIST, zoomsensitivity, 0, 0.75f, 1);

    VAR(IDF_PERSIST, zoommousetype, 0, 0, 2);
    VAR(IDF_PERSIST, zoommousedeadzone, 0, 25, 100);
    VAR(IDF_PERSIST, zoommousepanspeed, 1, 10, INT_MAX-1);
    VAR(IDF_PERSIST, zoomfov, 1, 10, 150);
    VAR(IDF_PERSIST, zoomtime, 1, 100, 10000);

    VARF(IDF_PERSIST, zoomlevel, 1, 4, 10, checkzoom());
    VAR(IDF_PERSIST, zoomlevels, 1, 5, 10);
    VAR(IDF_PERSIST, zoomdefault, 0, 0, 10); // 0 = last used, else defines default level
    VAR(IDF_PERSIST, zoomscroll, 0, 0, 1); // 0 = stop at min/max, 1 = go to opposite end

    VAR(IDF_PERSIST, aboveheadnames, 0, 1, 1);
    VAR(IDF_PERSIST, aboveheadstatus, 0, 1, 1);
    VAR(IDF_PERSIST, aboveheadteam, 0, 1, 2);
    VAR(IDF_PERSIST, aboveheaddamage, 0, 0, 1);
    VAR(IDF_PERSIST, aboveheadicons, 0, 2, 2);
    FVAR(IDF_PERSIST, aboveheadblend, 0.f, 0.75f, 1.f);
    FVAR(IDF_PERSIST, aboveheadsmooth, 0, 0.5f, 1);
    FVAR(IDF_PERSIST, aboveheadnamesize, 0, 2, 1000);
    FVAR(IDF_PERSIST, aboveheadstatussize, 0, 2, 1000);
    FVAR(IDF_PERSIST, aboveheadiconsize, 0, 4, 1000);
    VAR(IDF_PERSIST, aboveheadsmoothmillis, 1, 200, 10000);
    VAR(IDF_PERSIST, eventiconfade, 500, 5000, INT_MAX-1);
    VAR(IDF_PERSIST, eventiconshort, 500, 3000, INT_MAX-1);
    VAR(IDF_PERSIST, eventiconcrit, 500, 2000, INT_MAX-1);

    VAR(IDF_PERSIST, showobituaries, 0, 4, 5); // 0 = off, 1 = only me, 2 = 1 + announcements, 3 = 2 + but dying bots, 4 = 3 + but bot vs bot, 5 = all
    VAR(IDF_PERSIST, showobitdists, 0, 0, 1);
    VAR(IDF_PERSIST, showplayerinfo, 0, 1, 1); // 0 = none, 1 = show events

    VAR(IDF_PERSIST, damagemergedelay, 0, 75, INT_MAX-1);
    VAR(IDF_PERSIST, damagemergeburn, 0, 250, INT_MAX-1);
    VAR(IDF_PERSIST, damagemergebleed, 0, 250, INT_MAX-1);
    VAR(IDF_PERSIST, playdamagetones, 0, 1, 3);
    VAR(IDF_PERSIST, playcrittones, 0, 2, 3);
    VAR(IDF_PERSIST, playreloadnotify, 0, 3, 15);

    VAR(IDF_PERSIST, quakefade, 0, 100, INT_MAX-1);
    VAR(IDF_PERSIST, ragdolls, 0, 1, 1);
    FVAR(IDF_PERSIST, bloodscale, 0, 1, 1000);
    VAR(IDF_PERSIST, bloodfade, 1, 5000, INT_MAX-1);
    VAR(IDF_PERSIST, bloodsize, 1, 20, 1000);
    VAR(IDF_PERSIST, bloodsparks, 0, 0, 1);
    FVAR(IDF_PERSIST, debrisscale, 0, 1, 1000);
    VAR(IDF_PERSIST, debrisfade, 1, 5000, INT_MAX-1);
    FVAR(IDF_PERSIST, gibscale, 0, 1, 1000);
    VAR(IDF_PERSIST, gibfade, 1, 5000, INT_MAX-1);
    VAR(IDF_PERSIST, burnfade, 100, 250, INT_MAX-1);
    FVAR(IDF_PERSIST, burnblend, 0.1f, 0.25f, 1);
    FVAR(IDF_PERSIST, impulsescale, 0, 1, 1000);
    VAR(IDF_PERSIST, impulsefade, 0, 200, INT_MAX-1);

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
        if(player1->state == CS_SPECTATOR && focus == player1) return false;
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
        if(player1->state == CS_EDITING) return editfov;
        if(focus == player1 && player1->state == CS_SPECTATOR) return specfov;
        if(thirdpersonview(true)) return thirdpersonfov;
        return firstpersonfov;
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
        if(allowmove(player1) && WEAP(player1->weapselect, zooms)) return true;
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
        swayfade = swayspeed = swaydist = 0;
    }

    void addsway(gameent *d)
    {
        if(firstpersonsway)
        {
            float maxspeed = physics::movevelocity(d);
            if(d->physstate >= PHYS_SLOPE)
            {
                swayspeed = min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), maxspeed);
                swaydist += swayspeed*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*firstpersonswaystep);
                swayfade = 1;
            }
            else if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*firstpersonswaystep);
                swayfade -= 0.5f*(curtime*maxspeed)/(firstpersonswaystep*1000.0f);
            }

            float k = pow(0.7f, curtime/25.0f);
            swaydir.mul(k);
            vec vel = vec(d->vel).add(d->falling);
            float speedscale = max(vel.magnitude(), maxspeed);
            if(speedscale > 0) swaydir.add(vec(vel).mul((1-k)/(15*speedscale)));
            swaypush.mul(pow(0.5f, curtime/25.0f));
        }
        else resetsway();
    }

    void announce(int idx, int targ, gameent *d, const char *msg, ...)
    {
        if(targ >= 0 && msg && *msg)
        {
            defvformatstring(text, msg, msg);
            conoutft(targ, "%s", text);
        }
        if(idx >= 0)
        {
            physent *t = !d || d == focus ? camera1 : d;
            playsound(idx, t->o, t, t == camera1 ? SND_FORCED : SND_DIRECT, -1, -1, -1, d ? &d->aschan : NULL);
        }
    }
    ICOMMAND(0, announce, "iis", (int *idx, int *targ, char *s), announce(*idx, *targ, NULL, "\fw%s", s));

    bool tvmode()
    {
        if(!m_edit(gamemode)) switch(player1->state)
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
        follow += n;
        #define checkfollow \
            if(follow >= numdynents()) follow = 0; \
            else if(follow < 0) follow = numdynents()-1;
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
    ICOMMAND(0, followdelta, "i", (int *n), followswitch(*n ? *n : 1));

    bool allowmove(physent *d)
    {
        if(d == player1)
        {
            if(hud::hasinput(true, false)) return false;
            if(tvmode()) return false;
        }
        if(d->type == ENT_PLAYER || d->type == ENT_AI)
        {
            if(d->state == CS_DEAD || d->state == CS_WAITING || d->state == CS_SPECTATOR || intermission)
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
                else loopi(WEAP_MAX) if(!strcasecmp(weaptype[i].name, s))
                {
                    d->loadweap[j] = i;
                    break;
                }
                if(d->loadweap[j] < WEAP_OFFSET || d->loadweap[j] >= WEAP_ITEM) d->loadweap[j] = WEAP_MELEE;
            }
            client::addmsg(N_LOADWEAP, "ri3", d->clientnum, d->loadweap[0], d->loadweap[1]);
            conoutft(CON_SELF, "weapon selection is now: \fs%s%s\fS and \fs%s%s\fS",
                weaptype[d->loadweap[0]].text, (d->loadweap[0] >= WEAP_OFFSET ? weaptype[d->loadweap[0]].name : "random"),
                weaptype[d->loadweap[1]].text, (d->loadweap[1] >= WEAP_OFFSET ? weaptype[d->loadweap[1]].name : "random")
            );
        }
        else conoutft(CON_MESG, "\foweapon selection is only available in arena");
    }
    ICOMMAND(0, loadweap, "ss", (char *a, char *b), chooseloadweap(player1, a, b));
    ICOMMAND(0, getloadweap, "i", (int *n), intret(game::player1->loadweap[*n!=0 ? 1 : 0]));
    ICOMMAND(0, allowedweap, "i", (int *n), intret(isweap(*n) && WEAP(*n, allowed) >= (m_duke(gamemode, mutators) ? 2 : 1) ? 1 : 0));

    void respawn(gameent *d)
    {
        if(d->state == CS_DEAD && d->respawned < 0 && (!d->lastdeath || lastmillis-d->lastdeath >= 500))
        {
            client::addmsg(N_TRYSPAWN, "ri", d->clientnum);
            d->respawned = lastmillis;
        }
    }

    float deadscale(gameent *d, float amt = 1, bool timechk = false)
    {
        float total = amt;
        if(d->state == CS_DEAD || d->state == CS_WAITING)
        {
            int len = d->aitype >= AI_START ? min(ai::aideadfade, enemyspawntime ? enemyspawntime : INT_MAX-1) : m_delay(gamemode, mutators);
            if(len > 0 && (!timechk || len > 1000))
            {
                int interval = min(len/3, 1000), over = max(len-interval, 500), millis = lastmillis-d->lastdeath;
                if(millis <= len) { if(millis >= over) total *= 1.f-((millis-over)/float(interval)); }
                else total = 0;
            }
        }
        return total;
    }

    float transscale(gameent *d, bool third = true)
    {
        float total = d == focus ? (third ? thirdpersonblend : firstpersonblend) : playerblend;
        if(d->state == CS_ALIVE)
        {
            int prot = m_protect(gamemode, mutators), millis = d->protect(lastmillis, prot); // protect returns time left
            if(millis > 0) total *= 1.f-(float(millis)/float(prot));
            if(d == player1 && inzoom())
            {
                int frame = lastmillis-lastzoom;
                float pc = frame <= zoomtime ? (frame)/float(zoomtime) : 1.f;
                total *= zooming ? 1.f-pc : pc;
            }
        }
        else total = deadscale(d, total);
        return total;
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
            loopi(numdynents()) if((d = (gameent *)iterdynents(i)) != NULL)
            {
                if(burntime && d->burning(lastmillis, burntime))
                {
                    int millis = lastmillis-d->lastburn; float pc = 1, intensity = 0.25f+(rnd(75)/100.f);
                    if(burntime-millis < burndelay) pc = float(burntime-millis)/float(burndelay);
                    else pc = 0.5f+(float(millis%burndelay)/float(burndelay*2));
                    pc = deadscale(d, pc);
                    adddynlight(d->headpos(-d->height*0.5f), d->height*(1.5f+intensity)*pc, vec(1.1f*max(pc,0.5f), 0.45f*max(pc,0.2f), 0.05f*pc), 0, 0, DL_KEEP);
                    continue;
                }
                if(d->aitype < AI_START && illumlevel > 0 && illumradius > 0)
                {
                    vec col((teamtype[d->team].colour>>16)/255.f, ((teamtype[d->team].colour>>8)&0xFF)/255.f, (teamtype[d->team].colour&0xFF)/255.f);
                    adddynlight(d->headpos(-d->height*0.5f), illumradius, col.mul(illumlevel), 0, 0, DL_KEEP);
                }
            }
        }
    }

    void boosteffect(gameent *d, const vec &pos, int num, int len)
    {
        float intensity = 0.25f+(rnd(75)/100.f), blend = 0.5f+(rnd(50)/100.f);
        regularshape(PART_FIREBALL, int(d->radius)*2, firecols[rnd(FIRECOLOURS)], 21, num, len, pos, intensity, blend, -1, 0, 5);
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
                    if(num > 0 && len > 0) loopi(2) boosteffect(d, d->jet[i], num, len);
                    break;
                }
                case 2:
                {
                    if(issound(d->jschan))
                    {
                        sounds[d->jschan].vol = min((lastmillis-sounds[d->jschan].millis)*2, 255);
                        sounds[d->jschan].ends = lastmillis+250;
                    }
                    else playsound(S_JETPACK, d->o, d, (d == game::focus ? SND_FORCED : 0)|SND_LOOP, 1, -1, -1, &d->jschan, lastmillis+250);
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

    void heightoffset(gameent *d, bool local)
    {
        d->o.z -= d->height;
        if(d->state == CS_ALIVE)
        {
            if(physics::iscrouching(d))
            {
                bool crouching = d->action[AC_CROUCH];
                float crouchoff = 1.f-CROUCHHEIGHT;
                if(!crouching)
                {
                    float z = d->o.z, zoff = d->zradius*crouchoff, zrad = d->zradius-zoff, frac = zoff/10.f;
                    d->o.z += zrad;
                    loopi(10)
                    {
                        d->o.z += frac;
                        if(!collide(d, vec(0, 0, 1), 0.f, false))
                        {
                            crouching = true;
                            break;
                        }
                    }
                    if(crouching)
                    {
                        if(d->actiontime[AC_CROUCH] >= 0) d->actiontime[AC_CROUCH] = max(PHYSMILLIS-(lastmillis-d->actiontime[AC_CROUCH]), 0)-lastmillis;
                    }
                    else if(d->actiontime[AC_CROUCH] < 0)
                        d->actiontime[AC_CROUCH] = lastmillis-max(PHYSMILLIS-(lastmillis+d->actiontime[AC_CROUCH]), 0);
                    d->o.z = z;
                }
                if(d->type == ENT_PLAYER || d->type == ENT_AI)
                {
                    int crouchtime = abs(d->actiontime[AC_CROUCH]);
                    float amt = lastmillis-crouchtime <= PHYSMILLIS ? clamp(float(lastmillis-crouchtime)/PHYSMILLIS, 0.f, 1.f) : 1.f;
                    if(!crouching) amt = 1.f-amt;
                    crouchoff *= amt;
                }
                d->height = d->zradius-(d->zradius*crouchoff);
            }
            else d->height = d->zradius;
        }
        else d->height = d->zradius;
        d->o.z += d->height;
    }

    void checkoften(gameent *d, bool local)
    {
        d->checktags();
        adjustscaled(int, d->quake, quakefade);
        if(d->aitype < AI_START) heightoffset(d, local);
        loopi(WEAP_MAX) if(d->weapstate[i] != WEAP_S_IDLE)
        {
            bool timeexpired = lastmillis-d->weaplast[i] >= d->weapwait[i]+(d->weapselect != i || d->weapstate[i] != WEAP_S_POWER ? 0 : PHYSMILLIS);
            if(i == d->weapselect && d->weapstate[i] == WEAP_S_RELOAD && timeexpired)
            {
                if(timeexpired && playreloadnotify&(d == focus ? 1 : 2) && (d->ammo[i] >= WEAP(i, max) || playreloadnotify&(d == focus ? 4 : 8)))
                    playsound(WEAPSND(i, S_W_NOTIFY), d->o, d, d == focus ? SND_FORCED : 0, -1, -1, -1, &d->wschan);
            }
            if(d->state != CS_ALIVE || timeexpired)
                d->setweapstate(i, WEAP_S_IDLE, 0, lastmillis);
        }
        if(d->weapstate[d->weapselect] == WEAP_S_POWER)
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
                else playsound(WEAPSND2(d->weapselect, secondary, S_W_POWER), d->o, d, (d == game::focus ? SND_FORCED : 0)|SND_LOOP, vol, -1, -1, &d->pschan);
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
        if(d->lastbleed > 0 && lastmillis-d->lastbleed >= bleedtime) d->lastbleed = 0;
        if(issound(d->jschan) && !physics::jetpack(d))
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
            const int lagtime = lastmillis-d->lastupdate;
            if(d->ai || !lagtime || intermission) continue;
            //else if(lagtime > 1000) continue;
            physics::smoothplayer(d, 1, false);
        }
    }

    bool burn(gameent *d, int weap, int flags)
    {
        if(burntime && hithurts(flags) && (flags&HIT_MELT || (weap == -1 && flags&HIT_BURN) || doesburn(weap, flags)))
        {
            if(!issound(d->fschan)) playsound(S_BURNFIRE, d->o, d, SND_LOOP, d != focus ? 128 : 224, -1, -1, &d->fschan);
            if(isweap(weap)) d->lastburn = lastmillis;
            else return true;
        }
        return false;
    }

    bool bleed(gameent *d, int weap, int flags)
    {
        if(bleedtime && hithurts(flags) && ((weap == -1 && flags&HIT_BLEED) || doesbleed(weap, flags)))
        {
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
                    const int dmgsnd[S_R_DAMAGE] = { 0, 10, 25, 50, 75, 100, 150, 200 };
                    int snd = -1;
                    if(flags&BURN) snd = S_BURNED;
                    else loopirev(S_R_DAMAGE) if(damage >= dmgsnd[i]) { snd = S_DAMAGE+i; break; }
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
                    vec p = d->headpos(-d->height/3);
                    if(!isaitype(d->aitype) || aistyle[d->aitype].living)
                    {
                        if(!kidmode && bloodscale > 0)
                            part_splash(PART_BLOOD, int(clamp(damage/2, 2, 10)*bloodscale)*(bleeding ? 5 : 1), bloodfade, p, 0x88FFFF, (rnd(bloodsize)+2)/10.f, 1, 200, DECAL_BLOOD, int(d->radius));
                        if(kidmode || bloodscale <= 0 || bloodsparks)
                            part_splash(PART_PLASMA, int(clamp(damage/2, 2, 10))*(bleeding ? 3 : 1), bloodfade, p, 0x882222, 1.f, 1, 50, DECAL_STAIN, int(d->radius));
                    }
                    if(d->aitype < AI_START && !issound(d->vschan)) playsound(S_PAIN+rnd(S_R_PAIN), d->o, d, 0, -1, -1, -1, &d->vschan);
                    if(!burning && !bleeding) d->quake = clamp(d->quake+max(damage/2, 1), 0, 1000);
                    d->lastpain = lastmillis;
                }
                if(d != actor)
                {
                    bool sameteam = m_team(gamemode, mutators) && d->team == actor->team;
                    if(!sameteam) pushdamagemerge(d, actor, weap, damage, burning ? damagemerge::BURN : (bleeding ? damagemerge::BLEED : 0));
                    else if(actor == focus && !burning && !bleeding && !issound(alarmchan))
                        playsound(S_ALARM, actor->o, actor, 0, -1, -1, -1, &alarmchan);
                    if(!burning && !bleeding && !sameteam) actor->lasthit = totalmillis;
                }
            }
            if(isweap(weap) && !burning && !bleeding && (d == player1 || !isaitype(d->aitype) || aistyle[d->aitype].canmove))
            {
                if(WEAP2(weap, slow, flags&HIT_ALT) != 0 && !(flags&HIT_WAVE) && hithurts(flags))
                    d->vel.mul(1.f-((float(damage)/float(WEAP2(weap, damage, flags&HIT_ALT)))*WEAP2(weap, slow, flags&HIT_ALT))*slowscale);
                if(WEAP2(weap, hitpush, flags&HIT_ALT) != 0)
                {
                    float force = 1.f;
                    if(flags&HIT_WAVE || !hithurts(flags)) force = wavepushscale;
                    else if(d->health <= 0) force = deadpushscale;
                    else force = hitpushscale;
                    d->vel.add(vec(dir).mul((float(damage)/float(WEAP2(weap, damage, flags&HIT_ALT)))*WEAP2(weap, hitpush, flags&HIT_ALT)*WEAPLM(force, gamemode, mutators)));
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
            d->dodamage(health);
            if(actor->type == ENT_PLAYER || actor->type == ENT_AI) actor->totaldamage += damage;
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
        bool burning = burn(d, weap, flags), bleeding = bleed(d, weap, flags);
        d->lastregen = 0;
        d->lastpain = lastmillis;
        d->state = CS_DEAD;
        d->deaths++;
        d->obliterated = (style&FRAG_OBLITERATE) != 0;
        int anc = -1, dth = d->aitype >= AI_START || d->obliterated ? S_SPLOSH+rnd(S_R_SPLOSH) : S_PAIN+rnd(S_R_DIE);
        if(d == focus) anc = !m_duke(gamemode, mutators) && !m_trial(gamemode) ? S_V_FRAGGED : -1;
        else d->resetinterp();
        formatstring(d->obit)("%s ", colorname(d));
        if(d != actor && actor->lastattacker == d->clientnum) actor->lastattacker = -1;
        d->lastattacker = actor->clientnum;
        if(d == actor)
        {
            if(isaitype(d->aitype) && !aistyle[d->aitype].living) concatstring(d->obit, "was destroyed");
            else if(flags&HIT_MELT) concatstring(d->obit, *obitlava ? obitlava : "melted into a ball of fire");
            else if(flags&HIT_WATER) concatstring(d->obit, *obitwater ? obitwater : "died");
            else if(flags&HIT_DEATH) concatstring(d->obit, *obitdeath ? obitdeath : "died");
            else if(flags&HIT_SPAWN) concatstring(d->obit, "tried to spawn inside solid matter");
            else if(flags&HIT_LOST) concatstring(d->obit, "got very, very lost");
            else if(flags && isweap(weap) && !burning && !bleeding)
            {
                static const char *suicidenames[WEAP_MAX] = {
                    "hit themself",
                    "ate a bullet",
                    "created too much torsional stress",
                    "tested the effectiveness of their own shrapnel",
                    "got caught in their own crossfire",
                    "spontaneously combusted",
                    "tried to make out with plasma",
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
            if(isaitype(d->aitype) && !aistyle[d->aitype].living) concatstring(d->obit, "destroyed by a");
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
                    anc = S_V_REVENGE; override = true;
                }
                else if(style&FRAG_DOMINATE)
                {
                    concatstring(d->obit, " \fs\fzoydominating\fS");
                    actor->addicon(eventicon::DOMINATE, lastmillis, eventiconfade); // dominating
                    if(actor->dominated.find(d) < 0) actor->dominated.add(d);
                    if(d->dominating.find(actor) < 0) d->dominating.add(actor);
                    anc = S_V_DOMINATE; override = true;
                }
                concatstring(d->obit, " ");
                concatstring(d->obit, colorname(actor));

                if(style&FRAG_MKILL1)
                {
                    concatstring(d->obit, " \fs\fzRedouble-killing\fS");
                    actor->addicon(eventicon::MULTIKILL, lastmillis, eventiconfade, 0);
                    if(!override) anc = S_V_MULTI;
                }
                else if(style&FRAG_MKILL2)
                {
                    concatstring(d->obit, " \fs\fzRetriple-killing\fS");
                    actor->addicon(eventicon::MULTIKILL, lastmillis, eventiconfade, 1);
                    if(!override) anc = S_V_MULTI;
                }
                else if(style&FRAG_MKILL3)
                {
                    concatstring(d->obit, " \fs\fzRemulti-killing\fS");
                    actor->addicon(eventicon::MULTIKILL, lastmillis, eventiconfade, 2);
                    if(!override) anc = S_V_MULTI;
                }
            }

            if(style&FRAG_HEADSHOT)
            {
                actor->addicon(eventicon::HEADSHOT, lastmillis, eventiconfade, 0);
                if(!override) anc = S_V_HEADSHOT;
            }

            if(flags&HIT_CRIT) concatstring(d->obit, " with a \fs\fzgrcritical\fS hit");
            if(style&FRAG_SPREE1)
            {
                concatstring(d->obit, " in total \fs\fzcgcarnage\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 0);
                if(!override) anc = S_V_SPREE;
                override = true;
            }
            else if(style&FRAG_SPREE2)
            {
                concatstring(d->obit, " on a \fs\fzcgslaughter\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 1);
                if(!override) anc = S_V_SPREE2;
                override = true;
            }
            else if(style&FRAG_SPREE3)
            {
                concatstring(d->obit, " on a \fs\fzcgmassacre\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 2);
                if(!override) anc = S_V_SPREE3;
                override = true;
            }
            else if(style&FRAG_SPREE4)
            {
                concatstring(d->obit, " in a \fs\fzcgbloodbath\fS");
                actor->addicon(eventicon::SPREE, lastmillis, eventiconfade, 3);
                if(!override) anc = S_V_SPREE4;
                override = true;
            }
        }
        if(!log.empty())
        {
            concatstring(d->obit, rnd(2) ? ", assisted by" : ", helped by");
            loopv(log)
            {
                defformatstring(entry)(" %s%s%s", log.length() > 1 && i == log.length()-1 ? "and " : "", colorname(log[i]), log.length() > 1 && i < log.length()-1 ? "," : "");
                concatstring(d->obit, entry);
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
            bool isme = (d == player1 || actor == player1), show = false;
            if(!isme) { loopv(log) if(log[i] == player1) { isme = true; break; } }
            if(((!m_fight(gamemode) && !isme) || actor->aitype >= AI_START) && anc >= 0) anc = -1;
            if(flags&HIT_LOST) show = true;
            else switch(showobituaries)
            {
                case 1: if(isme || m_duke(gamemode, mutators)) show = true; break;
                case 2: if(isme || anc >= 0 || m_duke(gamemode, mutators)) show = true; break;
                case 3: if(isme || d->aitype < 0 || anc >= 0 || m_duke(gamemode, mutators)) show = true; break;
                case 4: if(isme || d->aitype < 0 || actor->aitype < 0 || anc >= 0 || m_duke(gamemode, mutators)) show = true; break;
                case 5: default: show = true; break;
            }
            int target = show ? (isme ? CON_SELF : CON_INFO) : -1;
            if(showobitdists) announce(anc, target, d, "\fs\fw%s\fS (@\fs\fc%.2f\fSm)", d->obit, actor->o.dist(d->o)/8.f);
            else announce(anc, target, d, "\fw%s", d->obit);
        }
        if(gibscale > 0)
        {
            vec pos = d->headpos(-d->height*0.5f);
            int gib = clamp(max(damage,5)/5, 1, 15), amt = int((rnd(gib)+gib+1)*gibscale);
            if(d->obliterated) amt *= 3;
            loopi(amt) projs::create(pos, pos, true, d, !isaitype(d->aitype) || aistyle[d->aitype].living ? PRJ_GIBS : PRJ_DEBRIS, rnd(gibfade)+gibfade, 0, rnd(500)+1, rnd(50)+10);
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

    void clientdisconnected(int cn, int reason)
    {
        if(!players.inrange(cn)) return;
        gameent *d = players[cn];
        if(!d) return;
        if(d->name[0] && showplayerinfo && (d->aitype < 0 || ai::showaiinfo))
            conoutft(CON_EVENT, "\fo%s left the game (%s)", colorname(d), reason >= 0 ? disc_reasons[reason] : "normal");
        gameent *e = NULL;
        loopi(numdynents()) if((e = (gameent *)iterdynents(i)))
        {
            e->dominating.removeobj(d);
            e->dominated.removeobj(d);
            if(d == e && follow >= i)
            {
                followswitch(-1);
                focus = (gameent *)iterdynents(follow);
                resetcamera();
            }
        }
        cameras.shrink(0);
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
        intermission = false;
        maptime = hud::lastnewgame = 0;
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
        loopi(numdynents()) if((d = (gameent *)iterdynents(i)) && (d->type == ENT_PLAYER || d->type == ENT_AI))
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
        loopi(numdynents()) if((o = (gameent *)iterdynents(i)))
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
        if(all) loopvj(projs::projs) if(projs::projs[j]->projtype == PRJ_SHOT && projs::projs[j]->projcollide&COLLIDE_SHOTS) i++;
        return i;
    }
    dynent *iterdynents(int i, bool all)
    {
        if(!i) return player1;
        i--;
        if(i<players.length()) return players[i];
        i -= players.length();
        if(all) loopvj(projs::projs) if(projs::projs[j]->projtype == PRJ_SHOT && projs::projs[j]->projcollide&COLLIDE_SHOTS)
        {
            if(!i) return projs::projs[j];
            i--;
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
        formatstring(cname)("%s\fs%s%s", *prefix ? prefix : "", teamtype[d->team].chat, name);
        if(!name[0] || d->aitype == AI_BOT || (d->aitype < AI_START && dupname && duplicatename(d, name)))
        {
            defformatstring(s)(" [\fs%s%d\fS]", d->aitype >= 0 ? "\fc" : "\fw", d->clientnum);
            concatstring(cname, s);
        }
        concatstring(cname, "\fS");
        return cname;
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

    void lighteffects(dynent *e, vec &color, vec &dir) { }

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
            int frame = lastmillis-lastzoom, f = zoomfov, t = zoomtime;
            checkzoom();
            if(zoomlevels > 1 && zoomlevel < zoomlevels) f = fov()-(((fov()-zoomfov)/zoomlevels)*zoomlevel);
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
        if(hasinput || (mousestyle() >= 1 && player1->state != CS_WAITING && player1->state != CS_SPECTATOR))
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
            physent *target = player1->state == CS_WAITING || player1->state == CS_SPECTATOR ? camera1 : (allowmove(player1) ? player1 : NULL);
            if(target)
            {
                float scale = (inzoom() && zoomsensitivity > 0 && zoomsensitivity < 1 ? 1.f-(zoomlevel/float(zoomlevels+1)*zoomsensitivity) : 1.f)*sensitivity;
                target->yaw += mousesens(dx, w, yawsensitivity*scale);
                target->pitch -= mousesens(dy, h, pitchsensitivity*scale*(!hasinput && mouseinvert ? -1.f : 1.f));
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

    void scaleyawpitch(float &yaw, float &pitch, float targyaw, float targpitch, float frame, float scale)
    {
        if(yaw < targyaw-180.0f) yaw += 360.0f;
        if(yaw > targyaw+180.0f) yaw -= 360.0f;
        float offyaw = fabs(targyaw-yaw)*frame, offpitch = fabs(targpitch-pitch)*frame*scale;
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
                if(deathcamspeed > 0) scaleyawpitch(yaw, pitch, camera1->aimyaw, camera1->aimpitch, (float(curtime)/1000.f)*deathcamspeed, 4.f);
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

    void cameratv()
    {
        bool isspec = player1->state == CS_SPECTATOR;
        if(cameras.empty())
        {
            loopk(2)
            {
                physent d = *player1;
                d.radius = d.height = 4.f;
                d.state = CS_ALIVE;
                loopv(entities::ents) if(entities::ents[i]->type == CAMERA || (k && !enttype[entities::ents[i]->type].noisy && entities::ents[i]->type != MAPMODEL))
                {
                    gameentity &e = *(gameentity *)entities::ents[i];
                    d.o = e.o;
                    if(enttype[e.type].radius) d.o.z += enttype[e.type].radius;
                    if(physics::entinmap(&d, false))
                    {
                        camstate &c = cameras.add();
                        c.pos = d.o; c.ent = i;
                        if(!k)
                        {
                            c.idx = e.attrs[0];
                            if(e.attrs[1]) c.mindist = e.attrs[1];
                            if(e.attrs[2]) c.maxdist = e.attrs[2];
                        }
                    }
                }
                if(!cameras.empty()) break;
            }
            gameent *d = NULL;
            loopi(numdynents()) if((d = (gameent *)iterdynents(i)) != NULL && (d->type == ENT_PLAYER || d->type == ENT_AI) && d->aitype < AI_START)
            {
                camstate &c = cameras.add();
                c.pos = d->headpos();
                c.ent = -1; c.idx = i;
                if(d->state == CS_DEAD) deathcamyawpitch(d, d->yaw, d->pitch);
                vecfromyawpitch(d->yaw, d->pitch, 1, 0, c.dir);
            }
        }
        else loopv(cameras) if(cameras[i].ent < 0 && cameras[i].idx >= 0 && cameras[i].idx < numdynents())
        {
            gameent *d = (gameent *)iterdynents(cameras[i].idx);
            if(!d) { cameras.remove(i--); continue; }
            cameras[i].pos = d->headpos();
            if(d->state == CS_DEAD) deathcamyawpitch(d, d->yaw, d->pitch);
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, cameras[i].dir);
        }

        if(!cameras.empty())
        {
            camstate *cam = &cameras[0];
            int ent = cam->ent, entidx = cam->ent >= 0 ? cam->ent : cam->idx;
            bool renew = !lasttvcam || lastmillis-lasttvcam >= spectvtime, override = !lasttvcam || lastmillis-lasttvcam >= max(spectvtime/2, 2500);
            loopk(3)
            {
                int found = 0;
                loopvj(cameras)
                {
                    camstate &c = cameras[j]; c.reset();
                    gameent *d, *t = c.ent < 0 && c.idx >= 0 ? (gameent *)iterdynents(c.idx) : NULL;
                    loopi(numdynents()) if((d = (gameent *)iterdynents(i)) && d != t && (d->state == CS_ALIVE || d->state == CS_DEAD) && d->aitype < AI_START)
                    {
                        vec trg, pos = d->feetpos();
                        float dist = c.pos.dist(d->feetpos());
                        if(dist >= c.mindist && dist <= min(c.maxdist, float(fog)) && (raycubelos(c.pos, pos, trg) || raycubelos(c.pos, pos = d->headpos(), trg)))
                        {
                            if(t && (t->state == CS_DEAD || t->state == CS_WAITING) && !t->lastdeath) continue;
                            bool fixed = t && t->state != CS_WAITING;
                            float yaw = fixed ? t->yaw : camera1->yaw, pitch = fixed ? t->pitch : camera1->pitch;
                            if(!fixed && (k || renew))
                            {
                                vec dir = pos;
                                if(c.cansee.length()) dir.add(vec(c.dir).div(c.cansee.length()));
                                dir.sub(c.pos).normalize();
                                vectoyawpitch(dir, yaw, pitch);
                            }
                            if(k || renew || getsight(c.pos, yaw, pitch, pos, trg, min(c.maxdist, float(fog)), curfov, fovy) || getsight(c.pos, yaw, pitch, pos = d->headpos(), trg, min(c.maxdist, float(fog)), curfov, fovy))
                            {
                                c.cansee.add(i);
                                c.dir.add(pos);
                                c.score += dist;
                            }
                        }
                    }
                    if(c.cansee.length() > (override && !t ? 1 : 0))
                    {
                        if(c.cansee.length() > 1)
                        {
                            float amt = float(c.cansee.length());
                            c.dir.div(amt);
                            c.score /= amt;
                        }
                        found++;
                    }
                    else { c.dir = c.pos; c.score = t ? 1e14f : 1e16f; }
                    if(t && t->state == CS_ALIVE && spectvbias > 0) c.score /= spectvbias;
                    if(k <= 1) break;
                }
                if(!found)
                {
                    if(k == 1)
                    {
                        if(override) renew = true;
                        else break;
                    }
                }
                else break;
            }
            if(renew)
            {
                cameras.sort(camstate::camsort);
                cam = &cameras[0];
                lasttvcam = lastmillis;
                if(!lasttvchg || (cam->ent >= 0 ? (ent >= 0 && cam->ent != entidx) : (ent < 0 && cam->idx != entidx)))
                {
                    lasttvchg = lastmillis;
                    ent = entidx = -1;
                }
                if(cam->ent < 0 && cam->idx >= 0)
                {
                    if((focus = (gameent *)iterdynents(cam->idx)) != NULL) follow = cam->idx;
                    else { focus = player1; follow = 0; }
                }
                else { focus = player1; follow = 0; }
            }
            camera1->o = cam->pos;
            if(cam->ent < 0 && focus != player1 && focus->state != CS_WAITING)
            {
                camera1->yaw = camera1->aimyaw = focus->yaw;
                camera1->pitch = camera1->aimpitch = focus->pitch;
                camera1->roll = focus->roll;
            }
            else
            {
                vectoyawpitch(vec(cam->dir).sub(camera1->o).normalize(), camera1->aimyaw, camera1->aimpitch);
                if(spectvspeed > 0) scaleyawpitch(camera1->yaw, camera1->pitch, camera1->aimyaw, camera1->aimpitch, (float(curtime)/1000.f)*spectvspeed, spectvpitch);
                else { camera1->yaw = camera1->aimyaw; camera1->pitch = camera1->aimpitch; }
            }
            camera1->resetinterp();
        }
        else setvar(isspec ? "specmode" : "waitmode", 0, true);
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
        if(tvmode()) cameratv();
        else if(focus->state == CS_DEAD)
        {
            deathcamyawpitch(focus, camera1->yaw, camera1->pitch);
            camera1->aimyaw = camera1->yaw;
            camera1->aimpitch = camera1->pitch;
        }
        else if(focus->state == CS_WAITING || focus->state == CS_SPECTATOR)
        {
            camera1->move = player1->move;
            camera1->strafe = player1->strafe;
            physics::move(camera1, 10, true);
        }
        if(focus->state == CS_SPECTATOR)
        {
            player1->aimyaw = player1->yaw = camera1->yaw;
            player1->aimpitch = player1->pitch = camera1->pitch;
            player1->o = camera1->o;
            player1->resetinterp();
        }
    }

    void resetcamera()
    {
        lastcamera = 0;
        zoomset(false, 0);
        checkcamera();
        camera1->o = focus->o;
        camera1->yaw = focus->yaw;
        camera1->pitch = focus->pitch;
        camera1->roll = focus->calcroll(false);
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
                maptime = lastmillis;
                if(*mapmusic && (!music || !Mix_PlayingMusic() || strcmp(mapmusic, musicfile))) playmusic(mapmusic, "");
                else musicdone(false);
                RUNWORLD("on_start");
                return;
            }
        }
        if(!curtime) { gets2c(); if(player1->clientnum >= 0) client::c2sinfo(); return; }

        if(!*player1->name && !menuactive()) showgui("name", -1);
        if(connected())
        {
            player1->conopen = commandmillis > 0 || hud::hasinput(true);
            // do shooting/projectile update here before network update for greater accuracy with what the player sees
            if(allowmove(player1)) cameraplayer();
            else player1->stopmoving(player1->state != CS_WAITING && player1->state != CS_SPECTATOR);

            gameent *d = NULL; bool allow = player1->state == CS_SPECTATOR || player1->state == CS_WAITING, found = false;
            loopi(numdynents()) if((d = (gameent *)iterdynents(i)) != NULL)
            {
                if(d->state != CS_SPECTATOR && allow && i == follow)
                {
                    if(focus != d)
                    {
                        focus = d;
                        resetcamera();
                    }
                    follow = i;
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
            if(!found || !allow)
            {
                if(focus != player1)
                {
                    focus = player1;
                    resetcamera();
                }
                follow = 0;
            }

            physics::update();
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
            if(!lastcamera)
            {
                cameras.shrink(0);
                if(mousestyle() == 2 && focus->state != CS_WAITING && focus->state != CS_SPECTATOR)
                {
                    camera1->yaw = focus->aimyaw = focus->yaw;
                    camera1->pitch = focus->aimpitch = focus->pitch;
                }
            }

            if(focus->state == CS_DEAD || focus->state == CS_WAITING || focus->state == CS_SPECTATOR)
            {
                camera1->aimyaw = camera1->yaw;
                camera1->aimpitch = camera1->pitch;
            }
            else
            {
                camera1->o = focus->headpos();
                if(mousestyle() <= 1)
                    findorientation(camera1->o, focus->yaw, focus->pitch, worldpos);

                camera1->aimyaw = mousestyle() <= 1 ? focus->yaw : focus->aimyaw;
                camera1->aimpitch = mousestyle() <= 1 ? focus->pitch : focus->aimpitch;
                if(thirdpersonview(true) && thirdpersondist)
                {
                    vec dir;
                    vecfromyawpitch(camera1->aimyaw, camera1->aimpitch, thirdpersondist > 0 ? -1 : 1, 0, dir);
                    physics::movecamera(camera1, dir, fabs(thirdpersondist), 1.0f);
                }
                camera1->resetinterp();

                switch(mousestyle())
                {
                    case 0:
                    case 1:
                    {
                        camera1->yaw = focus->yaw;
                        camera1->pitch = focus->pitch;
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
            }
            camera1->roll = focus->calcroll(physics::iscrouching(focus));
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
        int type = clamp(d->aitype, 0, AI_MAX-1);
        const char *mdl = third ? aistyle[type].tpmdl : aistyle[type].fpmdl;
        float yaw = d->yaw, pitch = d->pitch, roll = d->calcroll(physics::iscrouching(d));
        vec o = vec(third ? d->feetpos() : d->headpos());
        if(!third)
        {
            vec dir;
            if(firstpersonsway && !intermission)
            {
                vecfromyawpitch(d->yaw, 0, 0, 1, dir);
                float steps = swaydist/firstpersonswaystep*M_PI;
                dir.mul(firstpersonswayside*cosf(steps));
                dir.z = firstpersonswayup*(fabs(sinf(steps)) - 1);
                o.add(dir).add(swaydir).add(swaypush);
            }
            if(firstpersondist != 0.f)
            {
                vecfromyawpitch(yaw, pitch, 1, 0, dir);
                dir.mul(focus->radius*firstpersondist);
                o.add(dir);
            }
            if(firstpersonshift != 0.f)
            {
                vecfromyawpitch(yaw, pitch, 0, -1, dir);
                dir.mul(focus->radius*firstpersonshift);
                o.add(dir);
            }
            if(firstpersonadjust != 0.f)
            {
                vecfromyawpitch(yaw, pitch+90.f, 1, 0, dir);
                dir.mul(focus->height*firstpersonadjust);
                o.add(dir);
            }
        }

        int anim = animflags, basetime = lastaction, basetime2 = 0;
        if(animoverride)
        {
            anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
            basetime = 0;
        }
        else
        {
            if(secondary && allowmove(d) && (!isaitype(d->aitype) || aistyle[d->aitype].canmove))
            {
                if(physics::jetpack(d))
                {
                    if(d->move>0) anim |= (ANIM_JETPACK_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_JETPACK_LEFT : ANIM_JETPACK_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= (ANIM_JETPACK_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                    else anim |= (ANIM_JETPACK_UP|ANIM_LOOP)<<ANIM_SECONDARY;
                }
                else if(physics::liquidcheck(d) && d->physstate <= PHYS_FALL)
                    anim |= ((d->move || d->strafe || d->vel.z+d->falling.z>0 ? int(ANIM_SWIM) : int(ANIM_SINK))|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->physstate == PHYS_FALL && !d->turnside && !d->onladder && d->impulse[IM_TYPE] != IM_T_NONE && lastmillis-d->impulse[IM_TIME] <= 1000) { anim |= ANIM_IMPULSE_DASH<<ANIM_SECONDARY; basetime2 = d->impulse[IM_TIME]; }
                else if(d->physstate == PHYS_FALL && !d->turnside && !d->onladder && d->impulse[IM_JUMP] && lastmillis-d->impulse[IM_JUMP] <= 1000) { anim |= ANIM_JUMP<<ANIM_SECONDARY; basetime2 = d->impulse[IM_JUMP]; }
                else if(d->physstate == PHYS_FALL && !d->turnside && !d->onladder && d->timeinair >= 1000) anim |= (ANIM_JUMP|ANIM_END)<<ANIM_SECONDARY;
                else if(physics::sprinting(d) || d->turnside)
                {
                    if(d->move>0 || d->turnside) anim |= (ANIM_IMPULSE_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
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
        if(anim == ANIM_DYING) pitch *= max(1.f-(lastmillis-basetime)/500.f, 0.f);

        if(d->ragdoll && (!ragdolls || anim!=ANIM_DYING)) cleanragdoll(d);

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
        bool burning = burntime && lastmillis%100 < 50 && d->burning(lastmillis, burntime);
        int colour = burning ? firecols[rnd(FIRECOLOURS)] : (type >= AI_START ? 0xFFFFFF : teamtype[d->team].colour);
        e->light.material = vec((colour>>16)/255.f, ((colour>>8)&0xFF)/255.f, (colour&0xFF)/255.f);
        rendermodel(NULL, mdl, anim, o, yaw, pitch, roll, flags, e, attachments, basetime, basetime2, trans, size);
    }

    void renderabovehead(gameent *d, bool third, float trans)
    {
        if(third && d->type == ENT_PLAYER && !shadowmapping && !envmapping && trans > 1e-16f && d->o.squaredist(camera1->o) <= maxparticledistance*maxparticledistance)
        {
            vec pos = d->abovehead();
            float blend = aboveheadblend*trans;
            if(aboveheadnames && d != focus)
            {
                const char *name = colorname(d, NULL, d->aitype < 0 ? "<super>" : "<default>");
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
                if(d->state == CS_DEAD || d->state == CS_WAITING) t = textureload(hud::deadtex, 3);
                else if(d->state == CS_ALIVE)
                {
                    if(d->conopen) t = textureload(hud::conopentex, 3);
                    else if(m_team(gamemode, mutators) && aboveheadteam > (d->team != focus->team ? 1 : 0))
                        t = textureload(hud::teamtex(d->team), 3);
                    else if(d->dominating.find(focus) >= 0) t = textureload(hud::dominatingtex, 3);
                    else if(d->dominated.find(focus) >= 0) t = textureload(hud::dominatedtex, 3);
                }
                if(t && t != notexture)
                {
                    pos.z += aboveheadstatussize/2;
                    part_icon(pos, t, aboveheadstatussize, blend);
                    pos.z += aboveheadstatussize/2+0.25f;
                }
            }
            if(aboveheadicons && d != focus && d->state != CS_EDITING && d->state != CS_SPECTATOR) loopv(d->icons)
            {
                if(d->icons[i].type >= eventicon::VERBOSE && aboveheadicons < 2) break;
                if(d->icons[i].type == eventicon::CRITICAL && d->icons[i].value) continue;
                int millis = lastmillis-d->icons[i].millis;
                if(millis <= d->icons[i].fade)
                {
                    Texture *t = textureload(hud::icontex(d->icons[i].type, d->icons[i].value));
                    if(t && t != notexture)
                    {
                        int olen = min(d->icons[i].length/5, 1000), ilen = olen/2, colour = 0xFFFFFF;
                        float skew = millis < ilen ? millis/float(ilen) : (millis > d->icons[i].fade-olen ? (d->icons[i].fade-millis)/float(olen) : 1.f),
                              size = aboveheadiconsize*skew, fade = blend*skew, nudge = size/2;
                        if(d->icons[i].type >= eventicon::WEAPON)
                        {
                            switch(d->icons[i].type)
                            {
                                case eventicon::WEAPON: colour = weaptype[d->icons[i].value].colour; break;
                                case eventicon::AFFINITY: if(!m_bomber(gamemode)) colour = teamtype[d->icons[i].value].colour; break;
                                default: break;
                            }
                            nudge *= 2;
                        }
                        pos.z += nudge+0.125f;
                        part_icon(pos, t, size, fade, 0, 0, 1, colour);
                        pos.z += nudge;
                    }
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
            animflags = ANIM_DYING;
            lastaction = d->lastpain;
            if(ragdolls)
            {
                if(!validragdoll(d, lastaction)) animflags |= ANIM_RAGDOLL;
            }
            else
            {
                int t = lastmillis-lastaction;
                if(t < 0) return;
                if(t > 1000) animflags = ANIM_DEAD|ANIM_LOOP;
            }
        }
        else if(d->state == CS_EDITING)
        {
            animflags = ANIM_EDIT|ANIM_LOOP;
            showweap = false;
        }
        else if(third && lastmillis-d->lastpain <= 300)
        {
            secondary = third;
            lastaction = d->lastpain;
            animflags = ANIM_PAIN;
            animdelay = 300;
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
                        weapflags = animflags = weaptype[weap].anim+d->weapstate[weap];
                        break;
                    }
                    case WEAP_S_RELOAD:
                    {
                        if(weaptype[weap].anim != ANIM_MELEE && weaptype[weap].anim != ANIM_SWORD)
                        {
                            if(!d->hasweap(weap, m_weapon(gamemode, mutators)) || (!w_reload(weap, m_weapon(gamemode, mutators)) && lastmillis-d->weaplast[weap] <= d->weapwait[weap]/3))
                                showweap = false;
                            weapflags = animflags = weaptype[weap].anim+d->weapstate[weap];
                            break;
                        }
                    }
                    case WEAP_S_IDLE: case WEAP_S_WAIT: default:
                    {
                        if(!d->hasweap(weap, m_weapon(gamemode, mutators))) showweap = false;
                        weapflags = animflags = weaptype[weap].anim|ANIM_LOOP;
                        break;
                    }
                }
            }
        }
        if(!early) renderabovehead(d, third, trans);
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
                if(d->weapselect == WEAP_RIFLE && WEAP(d->weapselect, laser) && d->weapstate[d->weapselect] != WEAP_S_RELOAD)
                {
                    vec v, origin = d->originpos(), muzzle = d->muzzlepos(d->weapselect);
                    origin.z += 0.25f; muzzle.z += 0.25f;
                    float yaw, pitch;
                    vectoyawpitch(vec(muzzle).sub(origin).normalize(), yaw, pitch);
                    findorientation(d->o, d->yaw, d->pitch, v);
                    part_flare(origin, v, 1, PART_FLARE, teamtype[d->team].colour, 0.25f, 0.25f);
                }
                if(d->weapselect == WEAP_SWORD || (d->weapstate[d->weapselect] == WEAP_S_POWER && lastmillis-d->weaplast[d->weapselect] > 0))
                {
                    static const struct powerfxs {
                        int type, parttype, colour;
                        float size, radius;
                    } powerfx[WEAP_MAX] = {
                        { 0, 0, 0, 0 },
                        { 2, PART_SPARK, 0xFFCC22, 0.1f, 1.5f },
                        { 4, PART_LIGHTNING_FLARE, 0x1111CC, 1, 1 },
                        { 2, PART_SPARK, 0xFFAA00, 0.15f, 2 },
                        { 2, PART_SPARK, 0xFF8800, 0.1f, 2 },
                        { 2, PART_FIREBALL_SOFT, 0, 0.25f, 3 },
                        { 1, PART_PLASMA_SOFT, 0x226688, 0.15f, 2 },
                        { 2, PART_PLASMA_SOFT, 0x6611FF, 0.1f, 2.5f },
                        { 3, PART_PLASMA_SOFT, 0, 0.5f, 0.125f },
                        { 0, 0, 0, 0 },
                    };
                    float amt = (lastmillis-d->weaplast[d->weapselect])/float(d->weapwait[d->weapselect]);
                    switch(powerfx[d->weapselect].type)
                    {
                        case 1: case 2:
                        {
                            int colour = powerfx[d->weapselect].colour > 0 ? powerfx[d->weapselect].colour : firecols[rnd(FIRECOLOURS)];
                            regularshape(powerfx[d->weapselect].parttype, 1+(amt*powerfx[d->weapselect].radius), colour, powerfx[d->weapselect].type == 2 ? 21 : 53, 5, 60+int(30*amt), d->muzzlepos(d->weapselect), powerfx[d->weapselect].size*max(amt, 0.25f), max(amt, 0.5f), 1, 0, 5+(amt*5));
                            break;
                        }
                        case 3:
                        {
                            int colour = powerfx[d->weapselect].colour > 0 ? powerfx[d->weapselect].colour : ((int(254*max(1.f-amt,0.5f))<<16)+1)|((int(128*max(1.f-amt,0.f))+1)<<8), interval = lastmillis%1000;
                            float fluc = powerfx[d->weapselect].size+(interval ? (interval <= 500 ? interval/500.f : (1000-interval)/500.f) : 0.f);
                            part_create(powerfx[d->weapselect].parttype, 1, d->originpos(), colour, (powerfx[d->weapselect].radius*max(amt, 0.25f))+fluc);
                            break;
                        }
                        case 4:
                        {
                            int colour = powerfx[d->weapselect].colour > 0 ? powerfx[d->weapselect].colour : firecols[rnd(FIRECOLOURS)];
                            part_flare(d->originpos(), d->muzzlepos(d->weapselect), 1, PART_LIGHTNING, colour, powerfx[d->weapselect].size, max(amt, 0.1f));
                            break;
                        }
                        case 0: default: break;
                    }
                }
            }
            if(burntime && d->burning(lastmillis, burntime))
            {
                int millis = lastmillis-d->lastburn; float pc = 1, intensity = 0.25f+(rnd(75)/100.f), blend = 0.5f+(rnd(50)/100.f);
                if(burntime-millis < burndelay) pc = float(burntime-millis)/float(burndelay);
                else pc = 0.75f+(float(millis%burndelay)/float(burndelay*4));
                vec pos = vec(d->headpos(-d->height*0.35f)).add(vec(rnd(9)-4, rnd(9)-4, rnd(5)-2).mul(pc));
                regular_part_create(PART_FIREBALL_SOFT, max(burnfade, 100), pos, firecols[rnd(FIRECOLOURS)], d->height*0.75f*deadscale(d, intensity*pc), blend*pc*burnblend, -15, 0);
            }
            if(physics::sprinting(d)) impulseeffect(d, 1);
            if(physics::jetpack(d)) impulseeffect(d, 2);
        }
    }

    void render()
    {
        startmodelbatches();
        gameent *d;
        loopi(numdynents()) if((d = (gameent *)iterdynents(i)) && d != focus) renderplayer(d, true, transscale(d, true), deadscale(d, 1, true));
        entities::render();
        projs::render();
        if(m_capture(gamemode)) capture::render();
        else if(m_defend(gamemode)) defend::render();
        else if(m_bomber(gamemode)) bomber::render();
        ai::render();
        if(rendernormally) loopi(numdynents()) if((d = (gameent *)iterdynents(i)) && d != focus) d->cleartags();
        endmodelbatches();
        if(rendernormally) loopi(numdynents()) if((d = (gameent *)iterdynents(i)) && d != focus) rendercheck(d);
    }

    void renderavatar(bool early)
    {
        if(rendernormally && early) focus->cleartags();
        if(thirdpersonview() || !rendernormally)
            renderplayer(focus, true, transscale(focus, thirdpersonview(true)), deadscale(focus, 1, true), early);
        else if(!thirdpersonview() && focus->state == CS_ALIVE)
            renderplayer(focus, false, transscale(focus, false), deadscale(focus, 1, true), early);
        if(rendernormally && early) rendercheck(focus);
    }

    bool clientoption(char *arg) { return false; }
}
#undef GAMEWORLD
