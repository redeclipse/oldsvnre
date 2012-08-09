#include "game.h"
namespace weapons
{
    VAR(IDF_PERSIST, autoreloading, 0, 2, 4); // 0 = never, 1 = when empty, 2 = weapons that don't add a full clip, 3 = always (+1 zooming weaps too)
    VAR(IDF_PERSIST, autoreloaddelay, 0, 100, VAR_MAX);

    VAR(IDF_PERSIST, skipspawnweapon, 0, 0, 6); // skip spawnweapon; 0 = never, 1 = if numweaps > 1 (+1), 3 = if carry > 0 (+2), 6 = always
    VAR(IDF_PERSIST, skipmelee, 0, 7, 10); // skip melee; 0 = never, 1 = if numweaps > 1 (+2), 4 = if carry > 0 (+2), 7 = if carry > 0 and is offset (+2), 10 = always
    VAR(IDF_PERSIST, skippistol, 0, 0, 10); // skip pistol; 0 = never, 1 = if numweaps > 1 (+2), 4 = if carry > 0 (+2), 7 = if carry > 0 and is offset (+2), 10 = always
    VAR(IDF_PERSIST, skipgrenade, 0, 0, 10); // skip grenade; 0 = never, 1 = if numweaps > 1 (+2), 4 = if carry > 0 (+2), 7 = if carry > 0 and is offset (+2), 10 = always

    int lastweapselect = 0;
    VAR(IDF_PERSIST, weapselectdelay, 0, 150, VAR_MAX);
    VARF(IDF_PERSIST, weapselectslot, 0, 1, 1, changedkeys = lastmillis); // 0 = by id, 1 = by slot

    int slot(gameent *d, int n, bool back)
    {
        if(d && weapselectslot)
        {
            int p = m_weapon(game::gamemode, game::mutators), w = 0;
            loopi(WEAP_MAX) if(d->holdweap(i, p, lastmillis))
            {
                if(n == (back ? w : i)) return back ? i : w;
                w++;
            }
            return -1;
        }
        return n;
    }

    ICOMMAND(0, weapslot, "i", (int *o), intret(slot(game::player1, *o >= 0 ? *o : game::player1->weapselect))); // -1 = weapselect slot
    ICOMMAND(0, weapselect, "", (), intret(game::player1->weapselect));
    ICOMMAND(0, ammo, "i", (int *n), intret(isweap(*n) ? game::player1->ammo[*n] : -1));
    ICOMMAND(0, hasweap, "ii", (int *n, int *o), intret(isweap(*n) && game::player1->hasweap(*n, *o) ? 1 : 0));
    ICOMMAND(0, getweap, "ii", (int *n, int *o), {
        if(isweap(*n)) switch(*o)
        {
            case -1: result(weaptype[*n].name); break;
            case 0: result(WEAP(*n, name)); break;
            case 1: result(hud::itemtex(WEAPON, *n)); break;
            default: break;
        }
    });

    bool weapselect(gameent *d, int weap, bool local)
    {
        if(!local || d->canswitch(weap, m_weapon(game::gamemode, game::mutators), lastmillis, WEAP_S_FILTER))
        {
            if(local) client::addmsg(N_WEAPSELECT, "ri3", d->clientnum, lastmillis-game::maptime, weap);
            playsound(WEAPSND(weap, S_W_SWITCH), d->o, d, 0, -1, -1, -1, &d->wschan);
            d->weapswitch(weap, lastmillis, weaponswitchdelay);
            return true;
        }
        return false;
    }

    bool weapreload(gameent *d, int weap, int load, int ammo, bool local)
    {
        if(!local || d->canreload(weap, m_weapon(game::gamemode, game::mutators), lastmillis))
        {
            bool doact = false;
            if(local)
            {
                client::addmsg(N_RELOAD, "ri3", d->clientnum, lastmillis-game::maptime, weap);
                int oldammo = d->ammo[weap];
                ammo = min(max(d->ammo[weap], 0) + WEAP(weap, add), WEAP(weap, max));
                load = ammo-oldammo;
                doact = true;
            }
            else if(d != game::player1 && !d->ai) doact = true;
            else if(load < 0 && d->ammo[weap] < ammo) return false; // because we've already gone ahead..
            d->weapload[weap] = load;
            d->ammo[weap] = min(ammo, WEAP(weap, max));
            if(doact)
            {
                playsound(WEAPSND(weap, S_W_RELOAD), d->o, d, 0, -1, -1, -1, &d->wschan);
                d->setweapstate(weap, WEAP_S_RELOAD, WEAP(weap, rdelay), lastmillis);
            }
            return true;
        }
        return false;
    }

    void weaponswitch(gameent *d, int a = -1, int b = -1)
    {
        if(a < -1 || b < -1 || a >= WEAP_MAX || b >= WEAP_MAX || (weapselectdelay && lastweapselect && totalmillis-lastweapselect < weapselectdelay)) return;
        if(!d->weapwaited(d->weapselect, lastmillis, WEAP_S_FILTER)) return;
        int s = slot(d, d->weapselect);
        loopi(WEAP_MAX) // only loop the amount of times we have weaps for
        {
            if(a >= 0) s = a;
            else s += b;
            while(s > WEAP_MAX-1) s -= WEAP_MAX;
            while(s < 0) s += WEAP_MAX;

            int n = slot(d, s, true);
            if(a < 0)
            { // weapon skipping when scrolling
                int p = m_weapon(game::gamemode, game::mutators);
                #define skipweap(q,w) \
                { \
                    if(q && n == w && (d->aitype >= AI_START || w != WEAP_MELEE || p == WEAP_MELEE || d->weapselect == WEAP_MELEE)) switch(q) \
                    { \
                        case 10: continue; break; \
                        case 7: case 8: case 9: if(d->carry(p, 5, w) > (q-7)) continue; break; \
                        case 4: case 5: case 6: if(d->carry(p, 1, w) > (q-3)) continue; break; \
                        case 1: case 2: case 3: if(d->carry(p, 0, w) > q) continue; break; \
                        case 0: default: break; \
                    } \
                }
                skipweap(skipspawnweapon, p);
                skipweap(skipmelee, WEAP_MELEE);
                skipweap(skippistol, WEAP_PISTOL);
                skipweap(skipgrenade, WEAP_GRENADE);
            }

            if(weapselect(d, n))
            {
                lastweapselect = totalmillis;
                return;
            }
            else if(a >= 0) break;
        }
        game::errorsnd(d);
    }
    ICOMMAND(0, weapon, "ss", (char *a, char *b), weaponswitch(game::player1, *a ? parseint(a) : -1, *b ? parseint(b) : -1));

    void weapdrop(gameent *d, int w)
    {
        int weap = isweap(w) ? w : d->weapselect;
        bool found = false;
        if(isweap(weap) && weap >= WEAP_OFFSET && weap != m_weapon(game::gamemode, game::mutators))
        {
            if(d->weapwaited(d->weapselect, lastmillis, WEAP_S_FILTER))
            {
                client::addmsg(N_DROP, "ri3", d->clientnum, lastmillis-game::maptime, weap);
                d->setweapstate(d->weapselect, WEAP_S_WAIT, weaponswitchdelay, lastmillis);
                found = true;
            }
        }
        d->action[AC_DROP] = false;
        if(!found) game::errorsnd(d);
    }

    bool autoreload(gameent *d, int flags = 0)
    {
        if(d == game::player1 && WEAP2(d->weapselect, sub, flags&HIT_ALT))
        {
            bool noammo = d->ammo[d->weapselect] < WEAP2(d->weapselect, sub, flags&HIT_ALT),
                 noattack = !d->action[AC_ATTACK] && !d->action[AC_ALTERNATE];
            if((noammo || noattack) && !d->action[AC_USE] && d->weapstate[d->weapselect] == WEAP_S_IDLE && (noammo || lastmillis-d->weaplast[d->weapselect] >= autoreloaddelay))
                return autoreloading >= (noammo ? 1 : (WEAP(d->weapselect, add) < WEAP(d->weapselect, max) ? 2 : (WEAP(d->weapselect, zooms) ? 4 : 3)));
        }
        return false;
    }

    void checkweapons(gameent *d)
    {
        int sweap = m_weapon(game::gamemode, game::mutators);
        if(!d->hasweap(d->weapselect, sweap)) weapselect(d, d->bestweap(sweap, true));
        else if(d->action[AC_RELOAD] || autoreload(d)) weapreload(d, d->weapselect);
        else if(d->action[AC_DROP]) weapdrop(d, d->weapselect);
    }

    void offsetray(vec &from, vec &to, int spread, int z, vec &dest)
    {
        float f = to.dist(from)*float(spread)/10000.f;
        for(;;)
        {
            #define RNDD rnd(101)-50
            vec v(RNDD, RNDD, RNDD);
            if(v.magnitude() > 50) continue;
            v.mul(f);
            v.z = z > 0 ? v.z/float(z) : 0;
            dest = to;
            dest.add(v);
            vec dir = vec(dest).sub(from).normalize();
            raycubepos(from, dir, dest, 0, RAY_CLIPMAT|RAY_ALPHAPOLY);
            return;
        }
    }

    float accmod(gameent *d, bool zooming, int *x)
    {
        float r = 1; int y = 0;
        if(inairspread && (physics::jetpack(d) || d->timeinair) && !d->onladder) { r += inairspread; y++; }
        if(impulsespread > 0 && physics::sprinting(d)) { r += impulsespread; y++; }
        else if(movespread > 0 && (d->move || d->strafe)) { r += movespread; y++; }
        else if(stillspread > 0 && !physics::sliding(d) && !physics::iscrouching(d) && !zooming) { r += stillspread; y++; }
        if(x) *x = y;
        return r;
    }

    bool doshot(gameent *d, vec &targ, int weap, bool pressed, bool secondary, int force)
    {
        int offset = WEAP2(weap, sub, secondary), sweap = m_weapon(game::gamemode, game::mutators);
        if(!d->canshoot(weap, secondary ? HIT_ALT : 0, sweap, lastmillis))
        {
            if(!d->canshoot(weap, secondary ? HIT_ALT : 0, sweap, lastmillis, (1<<WEAP_S_RELOAD)))
            {
                // if the problem is not enough ammo, do the reload..
                if(autoreload(d, secondary ? HIT_ALT : 0)) weapreload(d, weap);
                return false;
            }
            else offset = -1;
        }
        float scale = 1;
        int sub = WEAP2(weap, sub, secondary), cooked = force;
        if(WEAP2(weap, power, secondary) && !WEAP(weap, zooms))
        {
            float maxscale = 1;
            if(sub > 1 && d->ammo[weap] < sub) maxscale = d->ammo[weap]/float(sub);
            int len = int(WEAP2(weap, power, secondary)*maxscale);
            if(!cooked)
            {
                if(d->weapstate[weap] != WEAP_S_POWER)
                {
                    if(pressed)
                    {
                        client::addmsg(N_SPHY, "ri3", d->clientnum, SPHY_POWER, len);
                        d->setweapstate(weap, WEAP_S_POWER, len, lastmillis);
                    }
                    else return false;
                }
                cooked = clamp(lastmillis-d->weaplast[weap], 1, len);
                if(pressed && cooked < len) return false;
            }
            scale = cooked/float(WEAP2(weap, power, secondary));
            if(sub > 1 && scale < 1) sub = int(ceilf(sub*scale));
        }
        else if(!pressed) return false;

        if(offset < 0)
        {
            if(weap == d->weapselect)
            {
                offset = max(d->weapload[weap], 1)+sub;
                d->weapload[weap] = -d->weapload[weap];
            }
            else
            {
                offset = max(d->weapload[d->weapselect], 1);
                d->weapload[d->weapselect] = -d->weapload[d->weapselect];
                d->ammo[d->weapselect] = max(d->ammo[d->weapselect]-offset, 0);
                offset = sub;
            }
        }
        else offset = sub;

        vec to, from;
        vector<shotmsg> shots;
        #define addshot(p) \
        { \
            shotmsg &s = shots.add(); \
            s.id = d->getprojid(); \
            s.pos = ivec(int(p.x*DMF), int(p.y*DMF), int(p.z*DMF)); \
        }
        if(weaptype[weap].traced)
        {
            from = d->originpos(weap == WEAP_MELEE, secondary);
            if(weap == WEAP_MELEE) to = vec(targ).sub(from).normalize().mul(d->radius).add(from);
            else to = d->muzzlepos(weap, secondary);
            int rays = WEAP2(weap, rays, secondary);
            if(rays > 1 && WEAP2(weap, power, secondary) && scale < 1) rays = int(ceilf(rays*scale));
            loopi(rays) addshot(to);
        }
        else
        {
            from = d->muzzlepos(weap, secondary);
            to = targ;

            vec unitv;
            float dist = to.dist(from, unitv);
            if(dist > 0) unitv.div(dist);
            else vecfromyawpitch(d->yaw, d->pitch, 1, 0, unitv);

            // move along the eye ray towards the weap origin, stopping when something is hit
            // nudge the target a tiny bit forward in the direction of the target for stability

            vec eyedir(from);
            eyedir.sub(d->o);
            float eyedist = eyedir.magnitude();
            if(eyedist > 0) eyedir.div(eyedist);
            float barrier = eyedist > 0 ? raycube(d->o, eyedir, eyedist, RAY_CLIPMAT) : eyedist;
            if(barrier < eyedist)
            {
                (from = eyedir).mul(barrier).add(d->o);
                (to = targ).sub(from).rescale(1e-3f).add(from);
            }
            else
            {
                barrier = raycube(from, unitv, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
                if(barrier < dist)
                {
                    to = unitv;
                    to.mul(barrier);
                    to.add(from);
                }
            }

            int rays = WEAP2(weap, rays, secondary), x = 0;
            if(rays > 1 && WEAP2(weap, power, secondary) && scale < 1) rays = int(ceilf(rays*scale));
            float m = accmod(d, WEAP(d->weapselect, zooms) && secondary, &x);
            int spread = WEAPSP(weap, secondary, game::gamemode, game::mutators, m, x);
            loopi(rays)
            {
                vec dest;
                if(spread) offsetray(from, to, spread, WEAP2(weap, zdiv, secondary), dest);
                else dest = to;
                if(weaptype[weap].thrown[secondary ? 1 : 0] > 0)
                    dest.z += from.dist(dest)*weaptype[weap].thrown[secondary ? 1 : 0];
                addshot(dest);
            }
        }
        projs::shootv(weap, secondary ? HIT_ALT : 0, offset, scale, from, shots, d, true);
        client::addmsg(N_SHOOT, "ri8iv", d->clientnum, lastmillis-game::maptime, weap, secondary ? HIT_ALT : 0, cooked, int(from.x*DMF), int(from.y*DMF), int(from.z*DMF), shots.length(), shots.length()*sizeof(shotmsg)/sizeof(int), shots.getbuf());

        return true;
    }

    void shoot(gameent *d, vec &targ, int force)
    {
        if(!game::allowmove(d)) return;
        bool secondary = physics::secondaryweap(d), alt = secondary && !WEAP(d->weapselect, zooms);
        if(doshot(d, targ, d->weapselect, d->action[alt ? AC_ALTERNATE : AC_ATTACK], secondary, force))
            if(!WEAP2(d->weapselect, fullauto, secondary)) d->action[alt ? AC_ALTERNATE : AC_ATTACK] = false;
    }

    void preload()
    {
        loopi(WEAP_MAX)
        {
            if(*weaptype[i].item) loadmodel(weaptype[i].item, -1, true);
            if(*weaptype[i].vwep) loadmodel(weaptype[i].vwep, -1, true);
            if(*weaptype[i].hwep) loadmodel(weaptype[i].hwep, -1, true);
        }
    }
}
