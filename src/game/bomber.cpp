#include "game.h"
namespace bomber
{
    bomberstate st;

    void killed(gameent *d, gameent *actor)
    {
        if(actor && m_gsp1(game::gamemode, game::mutators) && (!m_team(game::gamemode, game::mutators) || d->team != actor->team))
        {
            loopv(st.flags) if(isbomberaffinity(st.flags[i]) && st.flags[i].owner == actor)
                st.flags[i].taketime = lastmillis;
        }
    }

    bool carryaffinity(gameent *d)
    {
        loopv(st.flags) if(st.flags[i].owner == d) return true;
        return false;
    }

    float fixrot(float c)
    {
        float angle = c;
        while(angle < 0.0f) angle += 360.0f;
        while(angle >= 360.0f) angle -= 360.0f;
        return angle;
    }

    float offrot(float a, float b)
    {
        float angle = fixrot(a)-fixrot(b);
        while(angle < -180.0f) angle += 360.0f;
        while(angle >= 180.0f) angle -= 360.0f;
        return fabs(angle);
    }

    VAR(IDF_PERSIST, bombertargetintersect, 0, 1, 1);
    VAR(IDF_PERSIST, bombertargetangle, 0, 1, 1);
    int findtarget(gameent *d)
    {
        vec dest;
        gameent *e = NULL;
        float bestangle = 1e16f, bestdist = 1e16f;
        int best = -1;
        int numdyns = game::numdynents();
        loopk(d->aitype != AI_NONE ? 4 : 2)
        {
            if(bombertargetintersect)
            {
                findorientation(d->o, d->yaw, d->pitch, dest);
                if((e = game::intersectclosest(d->o, dest, d)) && e->team == d->team && e->state == CS_ALIVE && (k%2 || e->aitype != AI_BOT))
                    return e->clientnum;
            }
            float fx = k >= 2 ? 360 : (d->ai ? d->ai->views[0] : curfov), fy = k >= 2 ? 360 : (d->ai ? d->ai->views[1] : fovy);
            loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && e->team == d->team && e->state == CS_ALIVE && (k%2 || e->aitype != AI_BOT))
            {
                if(getsight(d->o, d->yaw, d->pitch, e->o, dest, 1e16f, fx, fy))
                {
                    vec dir = vec(e->o).sub(d->o);
                    float dist = dir.magnitude();
                    if(dist > 1e-3f) dir.div(dist);
                    float yaw = 0, pitch = 0;
                    vectoyawpitch(dir, yaw, pitch);
                    float offyaw = offrot(d->yaw, yaw), offpitch = offrot(d->pitch, pitch), offangle = offpitch+offyaw;
                    if(best < 0 || (bombertargetangle ? (offangle < bestangle || (offangle <= bestangle && dist < bestdist)) : (dist < bestdist || (dist <= bestdist && offangle < bestangle))))
                    {
                        best = e->clientnum;
                        bestdist = dist;
                        bestangle = offangle;
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
            vec o = d->feetpos(1), inertia;
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, inertia);
            inertia.normalize().mul(bomberspeed).add(vec(d->vel).add(d->falling).mul(bomberrelativity));
            bool guided = m_team(game::gamemode, game::mutators) && bomberlockondelay && lastmillis-d->actiontime[AC_AFFINITY] >= bomberlockondelay;
            client::addmsg(N_DROPAFFIN, "ri8", d->clientnum, guided ? findtarget(d) : -1, int(o.x*DMF), int(o.y*DMF), int(o.z*DMF), int(inertia.x*DMF), int(inertia.y*DMF), int(inertia.z*DMF));
            d->action[AC_AFFINITY] = false;
            d->actiontime[AC_AFFINITY] = 0;
            return true;
        }
        return false;
    }

    void preload()
    {
        preloadmodel("props/ball");
    }

    vec pulsecolour()
    {
        uint n = lastmillis/100;
        return vec::hexcolor(pulsecols[PULSE_DISCO][n%PULSECOLOURS]).lerp(vec::hexcolor(pulsecols[PULSE_DISCO][(n+1)%PULSECOLOURS]), (lastmillis%100)/100.0f);
    }

    void drawblips(int w, int h, float blend)
    {
        static vector<int> hasbombs; hasbombs.setsize(0);
        loopv(st.flags) if(st.flags[i].owner == game::focus) hasbombs.add(i);
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || hasbombs.find(i) >= 0 || !f.enabled) continue;
            vec pos = f.pos(true), dir = vec(pos).sub(camera1->o), colour = isbomberaffinity(f) ? pulsecolour() : vec::hexcolor(TEAM(f.team, colour));
            float area = 3, size = hud::radaraffinitysize;
            if(isbomberaffinity(f))
            {
                area = 2;
                if(!f.owner && !f.droptime)
                {
                    int millis = lastmillis-f.displaytime;
                    if(millis < 1000) size *= 1.f+(1-clamp(float(millis)/1000.f, 0.f, 1.f));
                }
            }
            else if(!m_gsp1(game::gamemode, game::mutators))
            {
                area = 3;
                if(isbombertarg(f, game::focus->team) && !hasbombs.empty()) size *= 1.25f;
            }
            hud::drawblip(isbomberaffinity(f) ? hud::bombtex : (isbombertarg(f, game::focus->team) ? hud::arrowtex : hud::flagtex), area, w, h, size, blend*hud::radaraffinityblend, (isbombertarg(f, game::focus->team) ? -1-hud::radarstyle : hud::radarstyle), (isbombertarg(f, game::focus->team) ? dir : pos), colour);
        }
    }

    void drawnotices(int w, int h, int &tx, int &ty, float blend)
    {
        if(game::focus->state == CS_ALIVE && hud::shownotices >= 2)
        {
            if(game::focus->lastbuff && hud::shownotices >= 3)
            {
                pushfont("reduced");
                if(m_regen(game::gamemode, game::mutators) && bomberregenbuff && bomberregenextra)
                    ty += draw_textx("Buffing: \fs\fo%d%%\fS damage, \fs\fg%d%%\fS shield, +\fs\fy%d\fS regen", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, int(bomberbuffdamage*100), int(bomberbuffshield*100), bomberregenextra)*hud::noticescale;
                else ty += draw_textx("Buffing: \fs\fo%d%%\fS damage, \fs\fg%d%%\fS shield", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, int(bomberbuffdamage*100), int(bomberbuffshield*100))*hud::noticescale;
                popfont();
            }
            loopv(st.flags)
            {
                bomberstate::flag &f = st.flags[i];
                if(f.owner == game::focus)
                {
                    pushfont("emphasis");
                    ty += draw_textx("Holding: \fs\f[%d]\f(%s)bomb\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, pulsecols[PULSE_DISCO][clamp((lastmillis/100)%PULSECOLOURS, 0, PULSECOLOURS-1)], hud::bombtex)*hud::noticescale;
                    popfont();
                    if(carrytime)
                    {
                        int delay = carrytime-(lastmillis-f.taketime);
                        pushfont("default");
                        ty += draw_textx("Explodes in \fs\fzgy%s\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, timestr(delay, -1))*hud::noticescale;
                        popfont();
                        if(m_gsp1(game::gamemode, game::mutators))
                        {
                            pushfont("reduced");
                            ty += draw_textx("Killing enemies resets fuse timer", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED)*hud::noticescale;
                            popfont();
                        }
                    }
                    SEARCHBINDCACHE(altkey)("affinity", 0);
                    pushfont("reduced");
                    ty += draw_textx("Press \fs\fc%s\fS to throw", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, altkey)*hud::noticescale;
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
            if(!entities::ents.inrange(f.ent) || !f.enabled) continue;
            int millis = lastmillis-f.displaytime;
            vec colour = pulsecolour();
            float skew = hud::inventoryskew;
            if(f.owner || f.droptime)
            {
                if(f.owner == game::focus)
                {
                    if(millis <= 1000) skew += (1.f-skew)*clamp(float(millis)/1000.f, 0.f, 1.f);
                    else skew = 1; // override it
                }
                else if(millis <= 1000) skew += ((1.f-skew)*clamp(float(millis)/1000.f, 0.f, 1.f));
                else skew = 1;
            }
            else if(millis <= 1000) skew += ((1.f-skew)-(clamp(float(millis)/1000.f, 0.f, 1.f)*(1.f-skew)));
            float wait = f.droptime ? clamp((lastmillis-f.droptime)/float(bomberresetdelay), 0.f, 1.f) : (f.owner ? clamp((lastmillis-f.taketime)/float(carrytime), 0.f, 1.f) : 1.f);
            int oldy = y-sy;
            if(!game::intermission && (f.owner || f.droptime))
                sy += hud::drawitem(hud::bombtex, x, oldy, s, 0, true, false, colour.x, colour.y, colour.z, blend, skew, "super", "%d%%", int(wait*100.f));
            else sy += hud::drawitem(hud::bombtex, x, oldy, s, 0, true, false, colour.x, colour.y, colour.z, blend, skew);
            if(f.owner)
            {
                vec c2 = vec::hexcolor(TEAM(f.owner->team, colour));
                hud::drawitem(hud::bombtakentex, x, oldy, s, 0.5f, true, false, c2.r, c2.g, c2.b, blend, skew);
            }
            else if(f.droptime) hud::drawitem(hud::bombdroptex, x, oldy, s, 0.5f, true, false, 0.25f, 1.f, 1.f, blend, skew);
            if(!game::intermission)
            {
                if(f.droptime || (f.owner && carrytime))
                {
                    if(wait > 0.5f)
                    {
                        int delay = wait > 0.7f ? (wait > 0.85f ? 150 : 300) : 600, millis = lastmillis%(delay*2);
                        float amt = (millis <= delay ? millis/float(delay) : 1.f-((millis-delay)/float(delay)));
                        flashcolour(colour.r, colour.g, colour.b, 1.f, 0.f, 0.f, amt);
                    }
                    hud::drawitembar(x, oldy, s, false, colour.r, colour.g, colour.b, blend, skew, wait);
                }
                if(f.owner == game::focus && m_team(game::gamemode, game::mutators) && bomberlockondelay && f.owner->action[AC_AFFINITY] && lastmillis-f.owner->actiontime[AC_AFFINITY] >= bomberlockondelay)
                {
                    gameent *e = game::getclient(findtarget(f.owner));
                    float cx = 0.5f, cy = 0.5f, cz = 1;
                    if(e && vectocursor(e->headpos(), cx, cy, cz))
                    {
                        int interval = lastmillis%500;
                        float rp = 1, gp = 1, bp = 1,
                              sp = interval >= 250 ? (500-interval)/250.f : interval/250.f,
                              sq = max(sp, 0.5f);
                        hud::colourskew(rp, gp, bp, sp);
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
                }
            }
        }
        return sy;
    }

    void checkcams(vector<cament *> &cameras)
    {
        loopv(st.flags) // flags/bases
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            cament *c = cameras.add(new cament);
            c->o = f.pos(true);
            c->o.z += enttype[AFFINITY].radius/2;
            c->type = cament::AFFINITY;
            c->id = i;
        }
    }

    void updatecam(cament *c)
    {
        switch(c->type)
        {
            case cament::AFFINITY:
            {
                if(st.flags.inrange(c->id))
                {
                    bomberstate::flag &f = st.flags[c->id];
                    c->o = f.pos(true);
                    c->o.z += enttype[AFFINITY].radius/2;
                    if(f.owner) c->player = f.owner;
                }
                break;
            }
        }
    }

    void render()
    {
        loopv(st.flags) // flags/bases
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !f.enabled || (f.owner == game::focus && !game::thirdpersonview(true) && rendernormally)) continue;
            vec above(f.pos(true, true));
            float trans = isbomberaffinity(f) ? 1.f : 0.5f;
            if(!isbomberaffinity(f) || !f.interptime) // || (!f.droptime && !f.owner))
            {
                int millis = lastmillis-f.displaytime;
                if(millis <= 1000) trans *= float(millis)/1000.f;
            }
            if(trans > 0)
            {
                if(isbomberaffinity(f))
                {
                    if(!f.owner && !f.droptime) above.z += enttype[AFFINITY].radius/4*trans;
                    entitylight *light = &entities::ents[f.ent]->light;
                    float size = trans, yaw = !f.owner && f.proj ? f.proj->yaw : (lastmillis/4)%360, pitch = !f.owner && f.proj ? f.proj->pitch : 0, roll = !f.owner && f.proj ? f.proj->roll : 0,
                          wait = f.droptime ? clamp((lastmillis-f.droptime)/float(bomberresetdelay), 0.f, 1.f) : ((f.owner && carrytime) ? clamp((lastmillis-f.taketime)/float(carrytime), 0.f, 1.f) : 0.f);
                    int interval = lastmillis%1000;
                    vec effect = pulsecolour();
                    if(wait > 0.5f)
                    {
                        int delay = wait > 0.7f ? (wait > 0.85f ? 150 : 300) : 600, millis = lastmillis%(delay*2);
                        float amt = (millis <= delay ? millis/float(delay) : 1.f-((millis-delay)/float(delay)));
                        flashcolour(effect.r, effect.g, effect.b, 1.f, 0.f, 0.f, amt);
                    }
                    light->material[0] = bvec::fromcolor(effect);
                    if(f.owner == game::focus && game::thirdpersonview(true)) trans *= 0.5f;
                    rendermodel(light, "props/ball", ANIM_MAPMODEL|ANIM_LOOP, above, yaw, pitch, roll, MDL_DYNSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHTFX, NULL, NULL, 0, 0, trans, size);
                    float fluc = interval >= 500 ? (1500-interval)/1000.f : (500+interval)/1000.f;
                    int pcolour = (int(light->material[0].x)<<16)|(int(light->material[0].y)<<8)|int(light->material[0].z);
                    part_create(PART_HINT_SOFT, 1, above, pcolour, enttype[AFFINITY].radius/4*trans+(2*fluc), fluc*trans);
                    if(!game::intermission && f.droptime)
                    {
                        above.z += enttype[AFFINITY].radius/4*trans+2.5f;
                        part_icon(above, textureload(hud::progresstex, 3), 3*trans, 1, 0, 0, 1, pcolour, (lastmillis%1000)/1000.f, 0.1f);
                        part_icon(above, textureload(hud::progresstex, 3), 2*trans, 0.25f, 0, 0, 1, pcolour);
                        part_icon(above, textureload(hud::progresstex, 3), 2*trans, 1, 0, 0, 1, pcolour, 0, wait);
                        above.z += 1.f;
                        defformatstring(str)("<huge>%d%%", int(wait*100.f)); part_textcopy(above, str, PART_TEXT, 1, pcolour, 2, 1);
                    }
                }
                else if(!m_gsp1(game::gamemode, game::mutators))
                {
                    part_explosion(above, enttype[AFFINITY].radius*trans, PART_SHOCKWAVE, 1, TEAM(f.team, colour), 1.f, trans*0.25f);
                    part_explosion(above, enttype[AFFINITY].radius/3*trans, PART_SHOCKBALL, 1, TEAM(f.team, colour), 1.f, trans*0.5f);
                    above.z += enttype[AFFINITY].radius*trans+2.5f;
                    defformatstring(info)("<super>%s base", TEAM(f.team, name));
                    part_textcopy(above, info, PART_TEXT, 1, TEAM(f.team, colour), 2, 1);
                    above.z += 2.5f;
                    part_icon(above, textureload(hud::teamtexname(f.team), 3), 2, 1, 0, 0, 1, TEAM(f.team, colour));
                }
            }
        }
    }

    void adddynlights()
    {
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !f.enabled) continue;
            float trans = 1.f;
            int millis = lastmillis-f.displaytime;
            if(millis <= 1000) trans = float(millis)/1000.f;
            vec colour = isbomberaffinity(f) ? pulsecolour() : vec::hexcolor(TEAM(f.team, colour));
            adddynlight(f.pos(true, true), enttype[AFFINITY].radius*trans, colour, 0, 0, DL_KEEP);
        }
    }

    void reset()
    {
        st.reset();
    }

    void setup()
    {
        loopv(entities::ents) if(entities::ents[i]->type == AFFINITY)
        {
            gameentity &e = *(gameentity *)entities::ents[i];
            if(!m_check(e.attrs[3], e.attrs[4], game::gamemode, game::mutators) || !isteam(game::gamemode, game::mutators, e.attrs[0], T_NEUTRAL))
                continue;
            st.addaffinity(e.o, e.attrs[0], i);
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
            putint(p, f.ent);
            loopj(3) putint(p, int(f.spawnloc[j]*DMF));
        }
    }

    void setscore(int team, int total)
    {
        hud::teamscore(team).total = total;
    }

    void parseaffinity(ucharbuf &p)
    {
        int numflags = getint(p);
        while(st.flags.length() > numflags) st.flags.pop();
        loopi(numflags)
        {
            int team = getint(p), ent = getint(p), enabled = getint(p), owner = getint(p), dropped = 0;
            vec spawnloc(0, 0, 0), droploc(0, 0, 0), inertia(0, 0, 0);
            loopj(3) spawnloc[j] = getint(p)/DMF;
            if(owner < 0)
            {
                dropped = getint(p);
                if(dropped)
                {
                    loopj(3) droploc[j] = getint(p)/DMF;
                    loopj(3) inertia[j] = getint(p)/DMF;
                }
            }
            if(p.overread()) break;
            while(!st.flags.inrange(i)) st.flags.add();
            bomberstate::flag &f = st.flags[i];
            f.team = team;
            f.ent = ent;
            f.enabled = enabled ? 1 : 0;
            f.spawnloc = spawnloc;
            if(owner >= 0) st.takeaffinity(i, game::getclient(owner), lastmillis);
            else if(dropped) st.dropaffinity(i, droploc, inertia, lastmillis);
        }
    }

    void dropaffinity(gameent *d, int i, const vec &droploc, const vec &inertia, int target)
    {
        if(!st.flags.inrange(i)) return;
        st.dropaffinity(i, droploc, inertia, lastmillis, target);
    }

    void removeplayer(gameent *d)
    {
        loopv(st.flags) if(st.flags[i].owner == d)
        {
            bomberstate::flag &f = st.flags[i];
            st.dropaffinity(i, f.owner->feetpos(1), f.owner->vel, lastmillis);
        }
    }

    void affinityeffect(int i, int team, const vec &from, const vec &to, int effect, const char *str)
    {
        if(from.x >= 0)
        {
            if(effect&1)
            {
                defformatstring(text)("<super>\fzZe%s", str);
                part_textcopy(vec(from).add(vec(0, 0, enttype[AFFINITY].radius)), text, PART_TEXT, game::eventiconfade, TEAM(team, colour), 3, 1, -10);
            }
            if(game::dynlighteffects) adddynlight(vec(from).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec::hexcolor(TEAM(team, colour)).mul(2.f), 500, 250);
        }
        if(to.x >= 0)
        {
            if(effect&2)
            {
                defformatstring(text)("<super>\fzZe%s", str);
                part_textcopy(vec(to).add(vec(0, 0, enttype[AFFINITY].radius)), text, PART_TEXT, game::eventiconfade, TEAM(team, colour), 3, 1, -10);
            }
            if(game::dynlighteffects) adddynlight(vec(to).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec::hexcolor(TEAM(team, colour)).mul(2.f), 500, 250);
        }
        if(from.x >= 0 && to.x >= 0) part_trail(PART_SPARK, 500, from, to, TEAM(team, colour), 1, 1, -10);
    }

    void destroyaffinity(const vec &o)
    {
        part_create(PART_PLASMA_SOFT, 250, o, 0xAA4400, enttype[AFFINITY].radius*0.5f, 0.5f);
        part_explosion(o, enttype[AFFINITY].radius, PART_EXPLOSION, 500, 0xAA4400, 1.f, 0.5f);
        part_explosion(o, enttype[AFFINITY].radius*2, PART_SHOCKWAVE, 250, 0xAA4400, 1.f, 0.1f);
        part_create(PART_SMOKE_LERP_SOFT, 500, o, 0x333333, enttype[AFFINITY].radius*0.75f, 0.5f, -15);
        if(!m_kaboom(game::gamemode, game::mutators) && game::nogore != 2 && game::debrisscale > 0)
        {
            int debris = rnd(5)+5, amt = int((rnd(debris)+debris+1)*game::debrisscale);
            loopi(amt) projs::create(o, o, true, NULL, PRJ_DEBRIS, rnd(game::debrisfade)+game::debrisfade, 0, rnd(501), rnd(101)+50);
        }
        playsound(WSND2(W_GRENADE, false, S_W_EXPLODE), o, NULL, 0, 255);
    }

    void resetaffinity(int i, int value)
    {
        if(!st.flags.inrange(i)) return;
        bomberstate::flag &f = st.flags[i];
        bool isreset = false;
        if(f.enabled && value)
        {
            destroyaffinity(f.pos(true, true));
            if(isbomberaffinity(f))
            {
                if(value == 2)
                {
                    affinityeffect(i, T_NEUTRAL, f.pos(true, true), f.spawnloc, 3, "RESET");
                    game::announcef(S_V_BOMBRESET, CON_INFO, NULL, true, "\fathe \fs\fwbomb\fS has been reset");
                    isreset = true;
                }
                entities::execlink(NULL, f.ent, false);
            }
        }
        st.returnaffinity(i, lastmillis, value!=0, isreset);
    }

    void scoreaffinity(gameent *d, int relay, int goal, int score)
    {
        if(!st.flags.inrange(relay) || !st.flags.inrange(goal)) return;
        bomberstate::flag &f = st.flags[relay], &g = st.flags[goal];
        affinityeffect(goal, d->team, g.spawnloc, f.spawnloc, 3, "DESTROYED");
        destroyaffinity(g.spawnloc);
        entities::execlink(NULL, f.ent, false);
        entities::execlink(NULL, g.ent, false);
        hud::teamscore(d->team).total = score;
        defformatstring(gteam)("%s", game::colourteam(g.team, "bombtex"));
        game::announcef(S_V_BOMBSCORE, CON_INFO, d, true, "\fa%s destroyed the %s base for team %s (score: \fs\fc%d\fS, time taken: \fs\fc%s\fS)", game::colourname(d), gteam, game::colourteam(d->team), score, timestr(lastmillis-f.inittime));
        st.returnaffinity(relay, lastmillis, false);
    }

    void takeaffinity(gameent *d, int i)
    {
        if(!st.flags.inrange(i)) return;
        bomberstate::flag &f = st.flags[i];
        d->action[AC_AFFINITY] = false;
        d->actiontime[AC_AFFINITY] = 0;
        playsound(S_CATCH, d->o, d);
        if(!f.droptime)
        {
            affinityeffect(i, d->team, d->feetpos(), f.pos(true, true), 1, "TAKEN");
            game::announcef(S_V_BOMBPICKUP, CON_INFO, d, true, "\fa%s picked up the \fs\fzwv\f($bombtex)bomb\fS", game::colourname(d));
            entities::execlink(NULL, f.ent, false);
        }
        st.takeaffinity(i, d, lastmillis);
    }

    void checkaffinity(dynent *e)
    {
        if(e->state != CS_ALIVE || !gameent::is(e)) return;
        gameent *d = (gameent *)e;
        vec o = d->feetpos();
        loopv(st.flags)
        {
            bomberstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !f.enabled || !isbomberaffinity(f)) continue;
            if(f.owner)
            {
                int takemillis = lastmillis-f.taketime;
                if(carrytime && d->ai && f.owner == d && takemillis >= carrytime-550-bomberlockondelay)
                {
                    if(d->action[AC_AFFINITY])
                    {
                        if(takemillis >= carrytime-500 || lastmillis-d->actiontime[AC_AFFINITY] >= bomberlockondelay)
                            d->action[AC_AFFINITY] = false;
                    }
                    else
                    {
                        d->action[AC_AFFINITY] = true;
                        d->actiontime[AC_AFFINITY] = lastmillis;
                    }
                }
                continue;
            }
            else if(f.droptime)
            {
                f.droploc = f.pos();
                if(f.lastowner && (f.lastowner == game::player1 || f.lastowner->ai) && f.proj && (!f.movetime || totalmillis-f.movetime >= 40))
                {
                    f.inertia = f.proj->vel;
                    f.movetime = totalmillis;
                    client::addmsg(N_MOVEAFFIN, "ri8", f.lastowner->clientnum, i, int(f.droploc.x*DMF), int(f.droploc.y*DMF), int(f.droploc.z*DMF), int(f.inertia.x*DMF), int(f.inertia.y*DMF), int(f.inertia.z*DMF));
                }
            }
            if(f.pickuptime && lastmillis-f.pickuptime <= 1000) continue;
            if(f.lastowner == d && f.droptime && (bomberpickupdelay < 0 || lastmillis-f.droptime <= bomberpickupdelay)) continue;
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
        if(m_team(game::gamemode, game::mutators) && !m_gsp1(game::gamemode, game::mutators))
        {
            int goal = -1;
            loopv(st.flags)
            {
                bomberstate::flag &g = st.flags[i];
                if(isbombertarg(g, ai::owner(d)) && (!st.flags.inrange(goal) || g.pos().squaredist(pos) < st.flags[goal].pos().squaredist(pos)))
                    goal = i;
            }
            if(st.flags.inrange(goal) && ai::makeroute(d, b, st.flags[goal].pos()))
            {
                d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, goal);
                return true;
            }
        }
	    if(b.type == ai::AI_S_PURSUE && b.targtype == ai::AI_T_NODE) return true; // we already did this..
		if(ai::randomnode(d, b, ai::ALERTMIN, 1e16f))
		{
            d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_NODE, d->ai->route[0]);
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
            static vector<int> taken; taken.setsize(0);
            loopv(st.flags)
            {
                bomberstate::flag &g = st.flags[i];
                if(g.owner == d) return aihomerun(d, b);
                else if((g.owner && ai::owner(g.owner) != ai::owner(d)) || g.droptime) taken.add(i);
            }
            if(!ai::badhealth(d)) while(!taken.empty())
            {
                int flag = taken.length() > 2 ? rnd(taken.length()) : 0;
                if(ai::makeroute(d, b, st.flags[taken[flag]].pos()))
                {
                    d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, taken[flag]);
                    return true;
                }
                else taken.remove(flag);
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
            if(!entities::ents.inrange(f.ent) || !f.enabled) continue;
            int owner = ai::owner(d);
            bool home = isbomberhome(f, owner) || isbombertarg(f, owner);
            if(d->aitype == AI_BOT && m_duke(game::gamemode, game::mutators) && home) continue;
            static vector<int> targets; // build a list of others who are interested in this
            targets.setsize(0);
            bool regen = d->aitype != AI_BOT || f.team != owner || !m_regen(game::gamemode, game::mutators) || d->health >= m_health(game::gamemode, game::mutators, d->model);
            ai::checkothers(targets, d, home || d->aitype != AI_BOT ? ai::AI_S_DEFEND : ai::AI_S_PURSUE, ai::AI_T_AFFINITY, j, true);
            if(d->aitype == AI_BOT)
            {
                gameent *e = NULL;
                int numdyns = game::numdynents();
                float mindist = enttype[AFFINITY].radius*4; mindist *= mindist;
                loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && owner == ai::owner(e))
                {
                    if(targets.find(e->clientnum) < 0 && (f.owner == e || e->feetpos().squaredist(f.pos()) <= mindist))
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
                        if((t->ai && !t->hasweap(t->ai->weappref, m_weapon(game::gamemode, game::mutators))) || (!t->ai && t->weapselect < W_OFFSET))
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
                    n.node = ai::closestwaypoint(f.pos(), ai::CLOSEDIST, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos())/(!regen ? 100.f : 1.f);
                    n.tolerance = f.team != owner ? 0.5f : 0.25f;
                    n.team = true;
                }
            }
            else if(isbomberaffinity(f))
            {
                if(targets.empty())
                { // attack the flag
                    ai::interest &n = interests.add();
                    n.state = d->aitype == AI_BOT ? ai::AI_S_PURSUE : ai::AI_S_DEFEND;
                    n.node = ai::closestwaypoint(f.pos(), ai::CLOSEDIST, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos());
                    n.tolerance = 0.5f;
                    n.team = true;
                }
                else
                { // help by defending the attacker
                    gameent *t;
                    loopvk(targets) if((t = game::getclient(targets[k])))
                    {
                        ai::interest &n = interests.add();
                        bool team = owner == ai::owner(t);
                        if(d->aitype == AI_BOT && m_duke(game::gamemode, game::mutators) && team) continue;
                        n.state = team ? ai::AI_S_DEFEND : ai::AI_S_PURSUE;
                        n.node = t->lastnode;
                        n.target = t->clientnum;
                        n.targtype = ai::AI_T_ACTOR;
                        n.score = d->o.squaredist(t->o);
                        n.tolerance = 0.5f;
                        n.team = team;
                    }
                }
            }
        }
    }

    bool aidefense(gameent *d, ai::aistate &b)
    {
        if(d->aitype == AI_BOT)
        {
            loopv(st.flags) if(st.flags[i].owner == d) return aihomerun(d, b);
            if(m_duke(game::gamemode, game::mutators)) return false;
        }
        if(st.flags.inrange(b.target))
        {
            bomberstate::flag &f = st.flags[b.target];
            if(isbomberaffinity(f) && f.owner && ai::owner(d) != ai::owner(f.owner))
                return ai::violence(d, b, f.owner, 4);
            int walk = f.owner && ai::owner(f.owner) != ai::owner(d) ? 1 : 0;
            if(d->aitype == AI_BOT)
            {
                bool regen = !m_regen(game::gamemode, game::mutators) || d->health >= m_health(game::gamemode, game::mutators, d->model);
                if(regen && lastmillis-b.millis >= (201-d->skill)*33)
                {
                    static vector<int> targets; // build a list of others who are interested in this
                    targets.setsize(0);
                    ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
                    gameent *e = NULL;
                    int numdyns = game::numdynents();
                    float mindist = enttype[AFFINITY].radius*4; mindist *= mindist;
                    loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
                    {
                        if(targets.find(e->clientnum) < 0 && (f.owner == e || e->feetpos().squaredist(f.pos()) <= mindist))
                            targets.add(e->clientnum);
                    }
                    if(!targets.empty())
                    {
                        d->ai->tryreset = true; // re-evaluate so as not to herd
                        return true;
                    }
                    else
                    {
                        walk = 2;
                        b.millis = lastmillis;
                    }
                }
                vec pos = d->feetpos();
                float mindist = enttype[AFFINITY].radius*8; mindist *= mindist;
                loopv(st.flags)
                { // get out of the way of the returnee!
                    bomberstate::flag &g = st.flags[i];
                    if(pos.squaredist(g.pos()) <= mindist)
                    {
                        if(g.owner && ai::owner(g.owner) == ai::owner(d) && !walk) walk = 1;
                        if(g.droptime && ai::makeroute(d, b, g.pos())) return true;
                    }
                }
            }
            return ai::defense(d, b, f.pos(), f.owner ? ai::CLOSEDIST : float(enttype[AFFINITY].radius), f.owner ? ai::ALERTMIN : float(enttype[AFFINITY].radius*walk*16), walk);
        }
        return false;
    }

    bool aipursue(gameent *d, ai::aistate &b)
    {
        if(st.flags.inrange(b.target) && d->aitype == AI_BOT)
        {
            bomberstate::flag &f = st.flags[b.target];
            if(!entities::ents.inrange(f.ent) || !f.enabled) return false;
            if(isbomberaffinity(f))
            {
                if(f.owner)
                {
                    if(d == f.owner) return aihomerun(d, b);
                    else if(ai::owner(d) != ai::owner(f.owner)) return ai::violence(d, b, f.owner, 4);
                    else return ai::defense(d, b, f.pos());
                }
                return ai::makeroute(d, b, f.pos());
            }
            else if(isbombertarg(f, ai::owner(d)))
                loopv(st.flags) if(st.flags[i].owner == d) return ai::makeroute(d, b, f.pos());
        }
        return false;
    }
}
