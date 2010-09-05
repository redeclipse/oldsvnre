#include "game.h"
namespace bomber
{
    bomberstate st;

    bool carryaffinity(gameent *d)
    {
        loopv(st.flags) if(st.flags[i].owner == d) return true;
        return false;
    }

    int findtarget(gameent *d)
    {
        float bestdist = 1e16f;
        gameent *e = NULL;
        int best = -1;
        vec targ;
        loopk(2)
        {
            loopi(game::numdynents()) if((e = (gameent *)game::iterdynents(i)) && e->team == d->team && e->state == CS_ALIVE && (k%2 ? d->aitype == AI_BOT : d->aitype < 0))
            {
                float md = d->ai ? d->ai->views[2] : hdr.worldsize, fx = d->ai ? d->ai->views[0] : curfov, fy = d->ai ? d->ai->views[1] : fovy;
                if(getsight(d->o, d->yaw, d->pitch, e->o, targ, md, fx, fy))
                {
                    vec dir = vec(e->o).sub(d->o).normalize();
                    float yaw, pitch; vectoyawpitch(dir, yaw, pitch);
                    while(yaw < 0) yaw += 360; while(yaw >= 360) yaw -= 360;
                    float dist = fabs(d->yaw-yaw);
                    if(dist < bestdist)
                    {
                        best = e->clientnum;
                        bestdist = dist;
                    }
                }
            }
            if(best >= 0) break;
        }
        return best;
    }

    bool dropaffinity(gameent *d)
    {
        if(carryaffinity(d) && (d->action[AC_AFFINITY] || d->actiontime[AC_AFFINITY] > 0))
        {
            if(d->action[AC_AFFINITY]) return true;
            vec inertia;
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, inertia);
            bool guided = false;
            if(bomberpowertime && lastmillis-d->actiontime[AC_AFFINITY] >= bomberpowertime) guided = true;
            inertia.normalize().mul(bomberspeed).add(vec(d->vel).add(d->falling).mul(bomberinertia));
            client::addmsg(N_DROPAFFIN, "ri8", d->clientnum, guided ? findtarget(d) : -1, int(d->o.x*DMF), int(d->o.y*DMF), int(d->o.z*DMF), int(inertia.x*DMF), int(inertia.y*DMF), int(inertia.z*DMF));
            d->action[AC_AFFINITY] = false;
            d->actiontime[AC_AFFINITY] = 0;
            return true;
        }
        return false;
    }

    void preload()
    {
        loadmodel("ball", -1, true);
    }

    void drawblips(int w, int h, float blend)
    {
        static vector<int> hasflags; hasflags.setsize(0);
        loopv(st.flags) if(st.flags[i].owner == game::focus) hasflags.add(i);
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || hasflags.find(i) >= 0 || !f.enabled) continue;
            vec dir = vec(f.pos()).sub(camera1->o);
            int colour = isbomberaffinity(f) ? 0xFFFFFF : teamtype[f.team].colour;
            float r = (colour>>16)/255.f, g = ((colour>>8)&0xFF)/255.f, b = (colour&0xFF)/255.f, fade = blend*hud::radaraffinityblend, size = hud::radaraffinitysize;
            if(isbomberaffinity(f))
            {
                size *= 1.25f;
                if(!f.owner && !f.droptime)
                {
                    int millis = lastmillis-f.interptime;
                    if(millis < 1000) size *= 1.f+(1-clamp(float(millis)/1000.f, 0.f, 1.f));
                }
            }
            else
            {
                float dist = dir.magnitude(), diff = dist <= hud::radarrange() ? clamp(1.f-(dist/hud::radarrange()), 0.f, 1.f) : 0.f;
                if(isbombertarg(f, game::focus->team) && !hasflags.empty()) fade += (1.f-fade)*diff;
            }
            dir.rotate_around_z(-camera1->yaw*RAD).normalize();
            hud::drawblip(isbomberaffinity(f) ? hud::bombtex : (isbombertarg(f, game::focus->team) ? hud::arrowtex : hud::flagtex), 3, w, h, size, fade, dir, r, g, b);
        }
    }

    void drawlast(int w, int h, int &tx, int &ty, float blend)
    {
        if(game::player1->state == CS_ALIVE && hud::shownotices >= 3)
        {
            loopv(st.flags)
            {
                bomberstate::flag &f = st.flags[i];
                if(f.owner == game::player1)
                {
                    pushfont("super");
                    ty += draw_textx("\fzwaYou have the bomb", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1)*hud::noticescale;
                    popfont();
                    if(bomberholdtime)
                    {
                        int delay = bomberholdtime-(lastmillis-f.taketime);
                        pushfont("emphasis");
                        ty += draw_textx("Explodes in \fs\fzgy%s\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, hud::timetostr(delay, -1))*hud::noticescale;
                        popfont();
                    }
                    SEARCHBINDCACHE(altkey)("action 9", 0);
                    pushfont("sub");
                    ty += draw_textx("Press \fs\fc%s\fS to throw it", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, altkey)*hud::noticescale;
                    popfont();
                    break;
                }
            }
        }
    }

    int drawinventory(int x, int y, int s, int m, float blend)
    {
        int sy = 0;
        loopv(st.flags) if(isbomberaffinity(st.flags[i]))
        {
            if(y-sy-s < m) break;
            bomberstate::flag &f = st.flags[i];
            int millis = lastmillis-f.interptime, pos[2] = { x, y-sy };
            float skew = hud::inventoryskew, fade = blend*hud::inventoryblend, r = 1.f, g = 1.f, b = 1.f, rescale = 1.f;
            if(f.owner || f.droptime)
            {
                if(f.owner == game::focus)
                {
                    if(hud::inventoryaffinity && millis <= hud::inventoryaffinity)
                    {
                        int off[2] = { hud::hudwidth/2, hud::hudheight/4 };
                        skew = 1; // override it
                        if(millis <= hud::inventoryaffinity*3/4)
                        {
                            float tweak = millis <= hud::inventoryaffinity/4 ? clamp(float(millis)/float(hud::inventoryaffinity/4), 0.f, 1.f) : 1.f;
                            skew += tweak*hud::inventorygrow;
                            loopk(2) pos[k] = off[k]+(s/2*tweak*skew);
                            skew *= tweak; fade *= tweak; rescale = 0;
                        }
                        else
                        {
                            float tweak = clamp(float(millis-(hud::inventoryaffinity*3/4))/float(hud::inventoryaffinity/4), 0.f, 1.f);
                            skew += (1.f-tweak)*hud::inventorygrow;
                            loopk(2) pos[k] -= int((pos[k]-(off[k]+s/2*skew))*(1.f-tweak));
                            rescale = tweak;
                        }
                    }
                    else
                    {
                        float pc = (millis%1000)/500.f, amt = pc > 1 ? 2.f-pc : pc;
                        fade += (1.f-fade)*amt;
                        if(!hud::inventoryaffinity && millis <= 1000)
                            skew += (1.f-skew)*clamp(float(millis)/1000.f, 0.f, 1.f);
                        else skew = 1; // override it
                    }
                }
                else if(millis <= 1000) skew += ((1.f-skew)*clamp(float(millis)/1000.f, 0.f, 1.f));
                else skew = 0.5f;
            }
            else if(millis <= 1000) skew += ((1.f-skew)-(clamp(float(millis)/1000.f, 0.f, 1.f)*(1.f-skew)));
            sy += int(hud::drawitem(hud::bombtex, pos[0], pos[1], s, false, r, g, b, fade, skew)*rescale);
            if(f.droptime)
            {
                int time = lastmillis-f.droptime, delay = bomberresetdelay-time;
                hud::drawitemsubtext(pos[0], pos[1], s, TEXT_RIGHT_UP, skew, "emphasis", fade, "\fy%s", hud::timetostr(delay, -1));
            }
            else if(f.owner)
            {
                if(f.owner == game::focus)
                {
                    if(bomberpowertime && f.owner->action[AC_AFFINITY])
                    {
                        int px = pos[0]-int(s*skew);
                        if(lastmillis-f.owner->actiontime[AC_AFFINITY] >= bomberpowertime)
                        {
                            gameent *e = game::getclient(findtarget(f.owner));
                            if(e)
                            {
                                hud::drawitemsubtext(px, pos[1]-s/2, s, TEXT_RIGHT_UP, skew, "emphasis", fade, "\fzgy[%s\fzgy]", game::colorname(e));
                                vec pos = e->headpos();
                                int interval = lastmillis%500;
                                float cx = 0.5f, cy = 0.5f, cz = 1, rp = 1, gp = 1, bp = 1,
                                      sp = interval >= 250 ? (500-interval)/250.f : interval/250.f,
                                      sq = max(sp, 0.5f);
                                hud::colourskew(rp, gp, bp, sp);
                                vectocursor(pos, cx, cy, cz);
                                int sx = int(cx*hud::hudwidth-s*sq), sy = int(cy*hud::hudsize-s*sq), ss = int(s*2*sq);
                                Texture *t = textureload(hud::indicatortex, 3);
                                if(t && t != notexture)
                                {
                                    glBindTexture(GL_TEXTURE_2D, t->id);
                                    glColor4f(rp, gp, bp, sq);
                                    hud::drawsized(sx, sy, ss);
                                }
                                t = textureload(hud::crosshairtex, 3);
                                if(t && t != notexture)
                                {
                                    glBindTexture(GL_TEXTURE_2D, t->id);
                                    glColor4f(rp, gp, bp, sq*0.5f);
                                    hud::drawsized(sx+ss/4, sy+ss/4, ss/2);
                                }
                            }
                            else hud::drawitemsubtext(px, pos[1]-s/2, s, TEXT_RIGHT_UP, skew, "emphasis", fade, "\fzoyready");
                        }
                    }
                    if(bomberholdtime)
                    {
                        int time = lastmillis-f.taketime, delay = bomberholdtime-time;
                        hud::drawitemsubtext(pos[0], pos[1], s, TEXT_RIGHT_UP, skew, "emphasis", fade, "\fzgy%s", hud::timetostr(delay, -1));
                    }
                    else hud::drawitemsubtext(pos[0], pos[1], s, TEXT_RIGHT_UP, skew, "super", fade, "\fzaw[\fzgy!\fzaw]");
                }
                else hud::drawitemsubtext(pos[0], pos[1], s, TEXT_RIGHT_UP, skew, "sub", fade, "\fs%s\fS", game::colorname(f.owner));
            }
        }
        return sy;
    }

    void render()
    {
        loopv(st.flags) // flags/bases
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !f.enabled) continue;
            vec above(f.pos(true));
            float trans = 0.f;
            if(isbomberaffinity(f))
            {
                int millis = lastmillis-f.interptime;
                if(millis <= 1000) trans += float(millis)/1000.f;
                else trans = 1.f;
            }
            else if(!isbomberaffinity(f)) trans = 0.5f;
            if(trans > 0)
            {
                if(isbomberaffinity(f))
                {
                    if(!f.owner) above.z += enttype[AFFINITY].radius/(f.droptime ? 16 : 4);
                    entitylight *light = &entities::ents[f.ent]->light;
                    if(light->millis != lastmillis) light->material = vec(1, 1, 1);
                    float yaw = !f.owner && f.proj ? f.proj->yaw : (lastmillis/10)%360, pitch = !f.owner && f.proj ? f.proj->pitch : 0, roll = !f.owner && f.proj ? f.proj->roll : 0;
                    rendermodel(light, "ball", ANIM_MAPMODEL|ANIM_LOOP, above, yaw, pitch, roll, MDL_SHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED, NULL, NULL, 0, 0, trans);
                    int interval = lastmillis%1000;
                    float fluc = interval >= 500 ? (1500-interval)/1000.f : (500+interval)/1000.f;
                    part_create(PART_HINT_SOFT, 1, above, 0xFFFFFF, enttype[AFFINITY].radius/4+(2*fluc), fluc*trans);
                }
                else
                {
                    part_explosion(above, enttype[AFFINITY].radius/2, PART_SHOCKWAVE, 1, teamtype[f.team].colour, 1.f, 0.125f);
                    part_explosion(above, enttype[AFFINITY].radius/4, PART_SHOCKBALL, 1, teamtype[f.team].colour, 1.f, 0.5f);
                }
            }
            above.z += (isbomberaffinity(f) ? 1 : enttype[AFFINITY].radius/2)+2.5f;
            if(!isbomberaffinity(f))
            {
                defformatstring(info)("<super>%s goal", teamtype[f.team].name);
                part_textcopy(above, info, PART_TEXT, 1, teamtype[f.team].colour, 2, max(trans, 0.5f));
                above.z += 2.5f;
            }
        }
    }

    void adddynlights()
    {
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            float trans = 1.f;
            if(!f.owner)
            {
                int millis = lastmillis-f.interptime;
                if(millis <= 1000) trans = float(millis)/1000.f;
            }
            int colour = isbomberaffinity(f) ? 0xFFFFFF : teamtype[f.team].colour;
            adddynlight(f.pos(true), enttype[AFFINITY].radius*2*trans, vec((colour>>16), ((colour>>8)&0xFF), (colour&0xFF)).div(255.f), 0, 0, DL_KEEP);
        }
    }

    void setupaffinity()
    {
        st.reset();
        #define setupaddaffinity(a,b) \
        { \
            index = st.addaffinity(entities::ents[a]->o, entities::ents[a]->attrs[0]); \
            if(st.flags.inrange(index)) \
            { \
                st.flags[index].ent = a; \
                int colour = st.flags[index].team == TEAM_NEUTRAL ? 0xFFFFFF : teamtype[st.flags[index].team].colour; \
                entities::ents[a]->light.material = st.flags[index].light.material = vec(colour>>16, (colour>>8)&0xFF, colour&0xFF).div(255.f); \
            } \
            else continue; \
        }
        #define setupchkaffinity(a,b) \
        { \
            if(entities::ents[a]->type != AFFINITY || !m_check(entities::ents[a]->attrs[3], game::gamemode) || !isteam(game::gamemode, game::mutators, entities::ents[a]->attrs[0], TEAM_NEUTRAL)) \
                continue; \
            else \
            { \
                int already = -1; \
                loopvk(st.flags) if(st.flags[k].ent == a) \
                { \
                    already = k; \
                    break; \
                } \
                if(st.flags.inrange(already)) b; \
            } \
        }
        int index = -1;
        loopv(entities::ents)
        {
            setupchkaffinity(i, { continue; });
            if(isteam(game::gamemode, game::mutators, entities::ents[i]->attrs[0], TEAM_NEUTRAL)) // not linked and is a team flag
                setupaddaffinity(i, entities::ents[i]->attrs[0]); // add as both
        }
    }

    void sendaffinity(packetbuf &p)
    {
        putint(p, N_INITAFFIN);
        putint(p, st.flags.length());
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            putint(p, f.team);
            loopk(3) putint(p, int(f.spawnloc[k]*DMF));
        }
    }

    void setscore(int team, int total)
    {
        st.findscore(team).total = total;
    }

    void parseaffinity(ucharbuf &p, bool commit)
    {
        int numflags = getint(p);
        loopi(numflags)
        {
            int team = getint(p), owner = getint(p), dropped = 0;
            vec droploc(0, 0, 0), inertia(0, 0, 0);
            if(owner < 0)
            {
                dropped = getint(p);
                if(dropped)
                {
                    loopk(3) droploc[k] = getint(p)/DMF;
                    loopk(3) inertia[k] = getint(p)/DMF;
                }
            }
            if(commit && st.flags.inrange(i))
            {
                bomberstate::flag &f = st.flags[i];
                f.team = team;
                if(owner >= 0) st.takeaffinity(i, game::getclient(owner), lastmillis);
                else if(dropped) st.dropaffinity(i, droploc, inertia, lastmillis);
            }
        }
    }

    void dropaffinity(gameent *d, int i, const vec &droploc, const vec &inertia, int target)
    {
        if(!st.flags.inrange(i)) return;
        st.dropaffinity(i, droploc, inertia, lastmillis);
    }

    void removeplayer(gameent *d)
    {
        loopv(st.flags) if(st.flags[i].owner == d)
        {
            bomberstate::flag &f = st.flags[i];
            st.dropaffinity(i, f.owner->o, f.owner->vel, lastmillis);
        }
    }

    void affinityeffect(int i, int team, const vec &from, const vec &to, int effect, const char *str)
    {
        if(from.x >= 0)
        {
            if(effect&1)
            {
                defformatstring(text)("<super>%s\fzRe%s", teamtype[team].chat, str);
                part_textcopy(vec(from).add(vec(0, 0, enttype[AFFINITY].radius)), text, PART_TEXT, game::aboveheadfade, 0xFFFFFF, 3, 1, -10);
            }
            if(game::dynlighteffects) adddynlight(vec(from).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec(teamtype[team].colour>>16, (teamtype[team].colour>>8)&0xFF, teamtype[team].colour&0xFF).mul(2.f/0xFF), 500, 250);
        }
        if(to.x >= 0)
        {
            if(effect&2)
            {
                defformatstring(text)("<super>%s\fzRe%s", teamtype[team].chat, str);
                part_textcopy(vec(to).add(vec(0, 0, enttype[AFFINITY].radius)), text, PART_TEXT, game::aboveheadfade, 0xFFFFFF, 3, 1, -10);
            }
            if(game::dynlighteffects) adddynlight(vec(to).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec(teamtype[team].colour>>16, (teamtype[team].colour>>8)&0xFF, teamtype[team].colour&0xFF).mul(2.f/0xFF), 500, 250);
        }
        if(from.x >= 0 && to.x >= 0) part_trail(PART_FIREBALL, 500, from, to, teamtype[team].colour, 2, 1, -5);
    }

    void destroyaffinity(const vec &o)
    {
        float radius = max(WEAPEX(WEAP_GRENADE, false, game::gamemode, game::mutators, 1), enttype[AFFINITY].radius);
        part_create(PART_PLASMA_SOFT, 250, o, 0xAA4400, radius*0.5f);
        part_explosion(o, radius, PART_EXPLOSION, 500, 0xAA4400, 1.f, 0.5f);
        part_explosion(o, radius*2, PART_SHOCKWAVE, 250, 0xAA4400, 1.f, 0.1f);
        part_create(PART_SMOKE_LERP_SOFT, 500, o, 0x333333, radius*0.75f, 0.5f, -15);
        int debris = rnd(5)+5, amt = int((rnd(debris)+debris+1)*game::debrisscale);
        loopi(amt) projs::create(o, o, true, NULL, PRJ_DEBRIS, rnd(game::debrisfade)+game::debrisfade, 0, rnd(501), rnd(101)+50);
        playsound(WEAPSND2(WEAP_GRENADE, false, S_W_EXPLODE), o, NULL, 0, 255);
    }

    void resetaffinity(int i, bool enabled)
    {
        if(!st.flags.inrange(i)) return;
        bomberstate::flag &f = st.flags[i];
        if(f.enabled && enabled)
        {
            affinityeffect(i, TEAM_NEUTRAL, f.pos(), f.spawnloc, 3, "RESET");
            destroyaffinity(f.pos());
            game::announce(S_V_BOMBRESET, CON_INFO, NULL, "\fathe bomb has been reset");
        }
        st.returnaffinity(i, lastmillis, true, enabled);
    }

    void scoreaffinity(gameent *d, int relay, int goal, int score)
    {
        if(!st.flags.inrange(relay) || !st.flags.inrange(goal)) return;
        bomberstate::flag &f = st.flags[relay], &g = st.flags[goal];
        affinityeffect(goal, d->team, g.spawnloc, f.spawnloc, 3, "EXPLODED");
        destroyaffinity(g.spawnloc);
        (st.findscore(d->team)).total = score;
        gameent *e = game::player1->state != CS_SPECTATOR ? game::player1 : game::focus;
        int snd = e->team ? (e->team != g.team ? S_V_YOUWIN : S_V_YOULOSE) : WEAPSND2(WEAP_GRENADE, false, S_W_EXPLODE);
        game::announce(snd, d == e ? CON_SELF : CON_INFO, d, "\fa%s destroyed the \fs%s%s\fS base for \fs%s%s\fS team (score: \fs\fc%d\fS, time taken: \fs\fc%s\fS)", game::colorname(d), teamtype[g.team].chat, teamtype[g.team].name, teamtype[d->team].chat, teamtype[d->team].name, score, hud::timetostr(lastmillis-f.inittime));
        st.returnaffinity(relay, lastmillis, true, false);
    }

    void takeaffinity(gameent *d, int i)
    {
        if(!st.flags.inrange(i)) return;
        bomberstate::flag &f = st.flags[i];
        d->action[AC_AFFINITY] = false;
        d->actiontime[AC_AFFINITY] = 0;
        if(!f.droptime)
        {
            affinityeffect(i, d->team, d->feetpos(), f.pos(), 1, "TAKEN");
            game::announce(S_V_BOMBPICKUP, d == game::focus ? CON_SELF : CON_INFO, d, "\fa%s picked up the bomb", game::colorname(d));
        }
        st.takeaffinity(i, d, lastmillis);
    }

    void checkaffinity(gameent *d)
    {
        vec o = d->feetpos();
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !f.enabled || !isbomberaffinity(f)) continue;
            if(f.owner)
            {
                if(d->ai && f.owner == d && !d->action[AC_AFFINITY] && lastmillis-f.taketime >= (bomberholdtime ? abs(bomberholdtime-bomberpowertime) : 1500))
                {
                    d->action[AC_AFFINITY] = true;
                    d->actiontime[AC_AFFINITY] = lastmillis;
                }
                continue;
            }
            else if(f.droptime) f.droploc = f.pos();
            if(f.pickuptime && lastmillis-f.pickuptime <= 1000) continue;
            if(f.lastowner == d && f.droptime && lastmillis-f.droptime <= 1000) continue;
            if(o.dist(f.pos()) <= enttype[AFFINITY].radius/2)
            {
                client::addmsg(N_TAKEAFFIN, "ri2", d->clientnum, i);
                f.pickuptime = lastmillis;
            }
        }
        dropaffinity(d);
    }

    bool aihomerun(gameent *d, ai::aistate &b)
    {
        vec pos = d->feetpos();
        int goal = -1;
        loopv(st.flags)
        {
            bomberstate::flag &g = st.flags[i];
            if(isbombertarg(g, ai::owner(d)) && (!st.flags.inrange(goal) || g.pos().squaredist(pos) < st.flags[goal].pos().squaredist(pos)))
            {
                goal = i;
            }
        }
        if(st.flags.inrange(goal) && ai::makeroute(d, b, st.flags[goal].pos()))
        {
            d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, goal);
            return true;
        }
	    if(b.type == ai::AI_S_INTEREST && b.targtype == ai::AI_T_NODE) return true; // we already did this..
		if(ai::randomnode(d, b, ai::SIGHTMIN, 1e16f))
		{
            d->ai->switchstate(b, ai::AI_S_INTEREST, ai::AI_T_NODE, d->ai->route[0]);
            return true;
		}
        return false;
    }

    int aiowner(gameent *d)
    {
        loopv(st.flags) if(entities::ents.inrange(st.flags[i].ent) && entities::ents[d->aientity]->links.find(st.flags[i].ent) >= 0)
            return st.flags[i].team;
        return d->team;
    }

    bool aicheck(gameent *d, ai::aistate &b)
    {
        if(d->aitype == AI_BOT)
        {
            static vector<int> hasflags, takenflags;
            hasflags.setsize(0);
            takenflags.setsize(0);
            loopv(st.flags)
            {
                bomberstate::flag &g = st.flags[i];
                if(g.owner == d) hasflags.add(i);
                else if((g.owner && ai::owner(g.owner) != ai::owner(d)) || g.droptime) takenflags.add(i);
            }
            if(!hasflags.empty()) return aihomerun(d, b);
            if(!ai::badhealth(d))
            {
                while(!takenflags.empty())
                {
                    int flag = takenflags.length() > 2 ? rnd(takenflags.length()) : 0;
                    if(ai::makeroute(d, b, st.flags[takenflags[flag]].pos()))
                    {
                        d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, takenflags[flag]);
                        return true;
                    }
                    else takenflags.remove(flag);
                }
            }
        }
        return false;
    }

    void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests)
    {
        vec pos = d->feetpos();
        loopvj(st.flags)
        {
            bomberstate::flag &f = st.flags[j];
            bool home = isbomberhome(f, ai::owner(d));
            static vector<int> targets; // build a list of others who are interested in this
            targets.setsize(0);
            bool regen = d->aitype != AI_BOT || f.team == TEAM_NEUTRAL || !m_regen(game::gamemode, game::mutators) || d->health >= max(maxhealth, extrahealth);
            ai::checkothers(targets, d, home || d->aitype != AI_BOT ? ai::AI_S_DEFEND : ai::AI_S_PURSUE, ai::AI_T_AFFINITY, j, true);
            if(d->aitype == AI_BOT)
            {
                gameent *e = NULL;
                loopi(game::numdynents()) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
                {
                    vec ep = e->feetpos();
                    if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.pos()) <= (enttype[AFFINITY].radius*enttype[AFFINITY].radius*4) || f.owner == e))
                        targets.add(e->clientnum);
                }
            }
            if(home)
            {
                bool guard = false;
                if(f.owner || f.droptime || targets.empty()) guard = true;
                else if(d->hasweap(d->ai->weappref, m_weapon(game::gamemode, game::mutators)))
                { // see if we can relieve someone who only has a piece of crap
                    gameent *t;
                    loopvk(targets) if((t = game::getclient(targets[k])))
                    {
                        if((t->ai && !t->hasweap(t->ai->weappref, m_weapon(game::gamemode, game::mutators))) || (!t->ai && t->weapselect < WEAP_OFFSET))
                        {
                            guard = true;
                            break;
                        }
                    }
                }
                if(guard)
                { // defend the flag
                    ai::interest &n = interests.add();
                    n.state = ai::AI_S_DEFEND;
                    n.node = entities::closestent(WAYPOINT, f.pos(), ai::SIGHTMIN, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos())/(!regen ? 100.f : 1.f);
                }
            }
            else if(isbomberaffinity(f))
            {
                if(targets.empty())
                { // attack the flag
                    ai::interest &n = interests.add();
                    n.state = d->aitype == AI_BOT ? ai::AI_S_PURSUE : ai::AI_S_DEFEND;
                    n.node = entities::closestent(WAYPOINT, f.pos(), ai::SIGHTMIN, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos());
                }
                else
                { // help by defending the attacker
                    gameent *t;
                    loopvk(targets) if((t = game::getclient(targets[k])))
                    {
                        ai::interest &n = interests.add();
                        n.state = ai::owner(d) == ai::owner(t) ? ai::AI_S_PURSUE : ai::AI_S_DEFEND;
                        n.node = t->lastnode;
                        n.target = t->clientnum;
                        n.targtype = ai::AI_T_PLAYER;
                        n.score = d->o.squaredist(t->o);
                    }
                }
            }
        }
    }

    bool aidefense(gameent *d, ai::aistate &b)
    {
        if(d->aitype == AI_BOT)
        {
            static vector<int> hasflags;
            hasflags.setsize(0);
            loopv(st.flags)
            {
                bomberstate::flag &g = st.flags[i];
                if(isbomberaffinity(g) && g.owner == d) hasflags.add(i);
            }
            if(!hasflags.empty()) return aihomerun(d, b);
        }
        if(st.flags.inrange(b.target))
        {
            bomberstate::flag &f = st.flags[b.target];
            if(isbomberaffinity(f) && f.owner)
            {
                ai::violence(d, b, f.owner, false);
                if(d->aitype != AI_BOT) return true;
            }
            int walk = f.owner && ai::owner(f.owner) != ai::owner(d) ? 1 : 0;
            if(d->aitype == AI_BOT)
            {
                bool regen = !m_regen(game::gamemode, game::mutators) || d->health >= max(maxhealth, extrahealth);
                if(regen && lastmillis-b.millis >= (201-d->skill)*33)
                {
                    static vector<int> targets; // build a list of others who are interested in this
                    targets.setsize(0);
                    ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
                    gameent *e = NULL;
                    loopi(game::numdynents()) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
                    {
                        vec ep = e->feetpos();
                        if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.pos()) <= (enttype[AFFINITY].radius*enttype[AFFINITY].radius*4) || f.owner == e))
                            targets.add(e->clientnum);
                    }
                    if(!targets.empty())
                    {
                        d->ai->trywipe = true; // re-evaluate so as not to herd
                        return true;
                    }
                    else
                    {
                        walk = 2;
                        b.millis = lastmillis;
                    }
                }
                vec pos = d->feetpos();
                float mindist = float(enttype[AFFINITY].radius*enttype[AFFINITY].radius*8);
                loopv(st.flags)
                { // get out of the way of the returnee!
                    bomberstate::flag &g = st.flags[i];
                    if(pos.squaredist(g.pos()) <= mindist)
                    {
                        if(g.owner && ai::owner(g.owner) == ai::owner(d)) walk = 1;
                        if(g.droptime && ai::makeroute(d, b, g.pos())) return true;
                    }
                }
            }
            return ai::defense(d, b, f.pos(), f.owner ? ai::CLOSEDIST : float(enttype[AFFINITY].radius), f.owner ? ai::SIGHTMIN : float(enttype[AFFINITY].radius*(1+walk)), walk);
        }
        return false;
    }

    bool aipursue(gameent *d, ai::aistate &b)
    {
        if(st.flags.inrange(b.target) && d->aitype == AI_BOT)
        {
            bomberstate::flag &f = st.flags[b.target];
            b.idle = -1;
            if(isbomberaffinity(f)) return f.owner ? ai::violence(d, b, f.owner, true) : ai::makeroute(d, b, f.pos());
            else return ai::makeroute(d, b, f.pos());
        }
        return false;
    }
}
