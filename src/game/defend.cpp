#include "game.h"
namespace defend
{
    defendstate st;

    bool insideaffinity(const defendstate::flag &b, gameent *d)
    {
        return st.insideaffinity(b, d->feetpos());
    }

    void preload()
    {
        preloadmodel("props/flag");
    }

    static vec skewcolour(int owner, int enemy, float occupy)
    {
        vec colour = vec::hexcolor(TEAM(owner, colour));
        if(enemy)
        {
            int team = owner && enemy && !defendinstant ? T_NEUTRAL : enemy;
            int timestep = totalmillis%1000;
            float amt = clamp((timestep <= 500 ? timestep/500.f : (1000-timestep)/500.f)*occupy, 0.f, 1.f);
            colour.lerp(vec::hexcolor(TEAM(team, colour)), amt);
        }
        return colour;
    }

    void checkcams(vector<cament *> &cameras)
    {
        loopv(st.flags) // flags/bases
        {
            defendstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            cament *c = cameras.add(new cament);
            c->o = f.o;
            c->o.z += enttype[AFFINITY].radius*2/3;
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
                    defendstate::flag &f = st.flags[c->id];
                    c->o = f.o;
                    c->o.z += enttype[AFFINITY].radius*2/3;
                }
                break;
            }
        }
    }

    void render()
    {
        loopv(st.flags)
        {
            defendstate::flag &b = st.flags[i];
            if(!entities::ents.inrange(b.ent)) continue;
            float occupy = b.occupied(defendinstant, defendcount);
            entitylight *light = &entities::ents[b.ent]->light;
            light->material[0] = bvec::fromcolor(skewcolour(b.owner, b.enemy, occupy));
            rendermodel(light, "props/flag", ANIM_MAPMODEL|ANIM_LOOP, b.o, entities::ents[b.ent]->attrs[1], entities::ents[b.ent]->attrs[2], 0, MDL_DYNSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED);
            if(b.enemy && b.owner)
                formatstring(b.info)("\fs\f[%d]\f(%s)%s\fS vs. \fs\f[%d]\f(%s)%s\fS", TEAM(b.owner, colour), hud::teamtexname(b.owner), TEAM(b.owner, name), TEAM(b.enemy, colour), hud::teamtexname(b.enemy), TEAM(b.enemy, name));
            else
            {
                int defend = b.owner ? b.owner : b.enemy;
                formatstring(b.info)("\fs\f[%d]\f(%s)%s\fS", TEAM(defend, colour), hud::teamtexname(defend), TEAM(defend, name));
            }
            vec above = b.o;
            above.z += enttype[AFFINITY].radius*2/3;
            defformatstring(name)("<huge>%s", b.name);
            part_textcopy(above, name);
            above.z += 2.f;
            part_text(above, b.info);
            above.z += 3.f;
            if(b.enemy)
            {
                part_icon(above, textureload(hud::progresstex, 3), 3, 1, 0, 0, 1, (int(light->material[0].x)<<16)|(int(light->material[0].y)<<8)|int(light->material[0].z), (lastmillis%1000)/1000.f, 0.1f);
                part_icon(above, textureload(hud::progresstex, 3), 2, 1, 0, 0, 1, TEAM(b.enemy, colour), 0, occupy);
                part_icon(above, textureload(hud::progresstex, 3), 2, 0.25f, 0, 0, 1, TEAM(b.owner, colour), occupy, 1-occupy);
            }
            else
            {
                part_icon(above, textureload(hud::progresstex, 3), 3, 1, 0, 0, 1, TEAM(b.owner, colour));
                part_icon(above, textureload(hud::progresstex, 3), 2, 1, 0, 0, 1, TEAM(b.owner, colour));
            }
            above.z += 1.f;
            defformatstring(str)("<huge>%d%%", int(occupy*100.f)); part_textcopy(above, str);
        }
    }


    void adddynlights()
    {
        loopv(st.flags)
        {
            defendstate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            float occupy = f.occupied(defendinstant, defendcount);
            adddynlight(vec(f.o).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, skewcolour(f.owner, f.enemy, occupy), 0, 0, DL_KEEP);
        }
    }

    void drawblips(int w, int h, float blend)
    {
        loopv(st.flags)
        {
            defendstate::flag &f = st.flags[i];
            float occupy = f.occupied(defendinstant, defendcount);
            vec colour = skewcolour(f.owner, f.enemy, occupy), dir = vec(f.o).sub(camera1->o);
            const char *tex = f.hasflag ? hud::arrowtex : (f.owner == game::focus->team && f.enemy ? hud::alerttex : hud::flagtex);
            float size = hud::radaraffinitysize*(f.hasflag ? 1.25f : 1);
            if(hud::radaraffinitynames >= (f.hasflag ? 1 : 2))
            {
                bool overthrow = f.owner && f.enemy == game::focus->team;
                if(occupy < 1.f) hud::drawblip(tex, f.hasflag ? 3 : 2, w, h, size, blend*hud::radaraffinityblend, f.hasflag ? -1-hud::radarstyle : hud::radarstyle, f.hasflag ? dir : f.o, colour, "little", "\f[%d]%d%%", f.hasflag ? (overthrow ? 0xFF8800 : (occupy < 1.f ? 0xFFFF00 : 0x00FF00)) : TEAM(f.owner, colour), int(occupy*100.f));
                else hud::drawblip(tex, f.hasflag ? 3 : 2, w, h, size, blend*hud::radaraffinityblend, f.hasflag ? -1-hud::radarstyle : hud::radarstyle, f.hasflag ? dir : f.o, colour, "little", "\f[%d]%s", f.hasflag ? (overthrow ? 0xFF8800 : (occupy < 1.f ? 0xFFFF00 : 0x00FF00)) : TEAM(f.owner, colour), TEAM(f.owner, name));
            }
            else hud::drawblip(tex, f.hasflag ? 3 : 2, w, h, size, blend*hud::radaraffinityblend, f.hasflag ? -1 : hud::radarstyle, f.hasflag ? dir : f.o, colour);
        }
    }

    void drawnotices(int w, int h, int &tx, int &ty, float blend)
    {
        if(game::player1->state == CS_ALIVE && hud::shownotices >= 3)
        {
            loopv(st.flags) if(insideaffinity(st.flags[i], game::player1) && (st.flags[i].owner == game::player1->team || st.flags[i].enemy == game::player1->team))
            {
                defendstate::flag &f = st.flags[i];
                pushfont("emphasis");
                float occupy = !f.owner || f.enemy ? clamp(f.converted/float((!defendinstant && f.owner ? 2 : 1) * defendcount), 0.f, 1.f) : 1.f;
                bool overthrow = f.owner && f.enemy == game::player1->team;
                ty += draw_textx("%s \fs\f[%d]\f(%s)\f(%s)\fS \fs%s%d%%\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, overthrow ? "Overthrow" : "Secure", TEAM(f.owner, colour), hud::teamtexname(f.owner), hud::flagtex, overthrow ? "\fy" : (occupy < 1.f ? "\fc" : "\fg"), int(occupy*100.f))*hud::noticescale;
                popfont();
                break;
            }
        }
    }

    int drawinventory(int x, int y, int s, int m, float blend)
    {
        int sy = 0;
        loopv(st.flags)
        {
            if(y-sy-s < m) break;
            defendstate::flag &f = st.flags[i];
            bool hasflag = game::focus->state == CS_ALIVE && insideaffinity(f, game::focus);
            if(f.hasflag != hasflag) { f.hasflag = hasflag; f.lasthad = lastmillis-max(1000-(lastmillis-f.lasthad), 0); }
            int millis = lastmillis-f.lasthad;
            bool headsup = hud::chkcond(hud::inventorygame, game::player1->state == CS_SPECTATOR || f.owner == game::focus->team || st.flags.length() == 1);
            if(headsup || f.hasflag || millis <= 1000)
            {
                float skew = headsup ? hud::inventoryskew : 0.f,
                    occupy = f.enemy ? clamp(f.converted/float((!defendinstant && f.owner ? 2 : 1)*defendcount), 0.f, 1.f) : (f.owner ? 1.f : 0.f);
                vec c = vec::hexcolor(TEAM(f.owner, colour)), c1 = c;
                if(f.enemy)
                {
                    float amt = float(lastmillis%1000)/500.f;
                    if(amt > 1.f) amt = 2.f-amt;
                    c.lerp(vec::hexcolor(TEAM(f.enemy, colour)), amt);
                }
                if(f.hasflag) skew += (millis <= 1000 ? clamp(float(millis)/1000.f, 0.f, 1.f)*(1.f-skew) : 1.f-skew);
                else if(millis <= 1000) skew += (1.f-skew)-(clamp(float(millis)/1000.f, 0.f, 1.f)*(1.f-skew));
                int oldy = y-sy;
                if(hasflag || f.enemy)
                    sy += hud::drawitem(hud::flagtex, x, oldy, s, 0, true, false, c.r, c.g, c.b, blend, skew, "super", "%s%d%%", hasflag ? (f.owner && f.enemy == game::focus->team ? "\fy" : (occupy < 1.f ? "\fc" : "\fg")) : "\fw", int(occupy*100.f));
                else sy += hud::drawitem(hud::flagtex, x, oldy, s, 0, true, false, c.r, c.g, c.b, blend, skew);
                if(f.enemy)
                {
                    vec c2 = vec::hexcolor(TEAM(f.enemy, colour));
                    hud::drawitem(hud::attacktex, x, oldy, s, 0.5f, true, false, c2.r, c2.g, c2.b, blend, skew);
                    hud::drawitembar(x, oldy, s, false, c.r, c.g, c.b, blend, skew, occupy);
                }
                else if(f.owner)
                    hud::drawitem(hud::teamtexname(f.owner), x, oldy, s, 0.5f, true, false, c1.r, c1.g, c1.b, blend, skew);
            }
        }
        return sy;
    }

    void reset()
    {
        st.reset();
    }

    void setup()
    {
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type != AFFINITY || !m_check(e->attrs[3], e->attrs[4], game::gamemode, game::mutators)) continue;
            int team = e->attrs[0];
            switch(defendflags)
            {
                case 3:
                {
                    if(team && !isteam(game::gamemode, game::mutators, team, T_NEUTRAL)) team = T_NEUTRAL;
                    break;
                }
                case 2:
                {
                    if(!isteam(game::gamemode, game::mutators, team, T_FIRST)) continue;
                    break;
                }
                case 1:
                {
                    if(team && !isteam(game::gamemode, game::mutators, team, T_NEUTRAL)) continue;
                    break;
                }
                case 0: team = T_NEUTRAL; break;
            }
            defendstate::flag &b = st.flags.add();
            b.o = e->o;
            defformatstring(alias)("flag_%d", e->attrs[4]);
            const char *name = getalias(alias);
            if(name[0]) copystring(b.name, name);
            else formatstring(b.name)("flag %d", st.flags.length());
            b.ent = i;
            b.kinship = team;
            b.reset();
        }
        if(!st.flags.length()) return; // map doesn't seem to support this mode at all..
        int bases[T_ALL] = {0};
        bool hasteams = true;
        loopv(st.flags) bases[st.flags[i].kinship]++;
        loopi(numteams(game::gamemode, game::mutators)-1) if(!bases[i+1] || (bases[i+1] != bases[i+2]))
        {
            loopvk(st.flags) st.flags[k].kinship = T_NEUTRAL;
            hasteams = false;
            break;
        }
        if(m_gsp3(game::gamemode, game::mutators))
        {
            vec average(0, 0, 0);
            int count = 0;
            loopv(st.flags) if(!hasteams || st.flags[i].kinship != T_NEUTRAL)
            {
                average.add(st.flags[i].o);
                count++;
            }
            int smallest = -1;
            if(count)
            {
                average.div(count);
                float dist = 1e16f, tdist = 1e16f;
                loopv(st.flags) if(!st.flags.inrange(smallest) || (tdist = st.flags[i].o.dist(average)) < dist)
                {
                    smallest = i;
                    dist = tdist;
                }
            }
            if(!st.flags.inrange(smallest)) smallest = rnd(st.flags.length());
            int ent = st.flags[smallest].ent;
            copystring(st.flags[smallest].name, "the flag");
            st.flags[smallest].kinship = T_NEUTRAL;
            loopv(st.flags) if(st.flags[i].ent != ent) st.flags.remove(i--);
        }
    }

    void sendaffinity(packetbuf &p)
    {
        putint(p, N_SETUPAFFIN);
        putint(p, st.flags.length());
        loopv(st.flags)
        {
            defendstate::flag &b = st.flags[i];
            putint(p, b.kinship);
            putint(p, b.ent);
            loopj(3) putint(p, int(b.o[j]*DMF));
            sendstring(b.name, p);
        }
    }

    void parseaffinity(ucharbuf &p)
    {
        int numflags = getint(p);
        while(st.flags.length() > numflags) st.flags.pop();
        loopi(numflags)
        {
            int kin = getint(p), ent = getint(p), converted = getint(p), owner = getint(p), enemy = getint(p);
            vec o;
            loopj(3) o[j] = getint(p)/DMF;
            string name;
            getstring(name, p);
            if(p.overread()) break;
            while(!st.flags.inrange(i)) st.flags.add();
            st.initaffinity(i, kin, ent, o, owner, enemy, converted, name);
        }
    }

    void updateaffinity(int i, int owner, int enemy, int converted)
    {
        if(!st.flags.inrange(i)) return;
        defendstate::flag &b = st.flags[i];
        if(converted >= 0)
        {
            if(owner)
            {
                if(b.owner != owner)
                {
                    gameent *d = NULL, *e = NULL;
                    int numdyns = game::numdynents();
                    loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && e->type == ENT_PLAYER && insideaffinity(b, e))
                        if((d = e) == game::focus) break;
                    game::announcef(S_V_FLAGSECURED, CON_INFO, d, true, "\fateam \fs\f[%d]%s\fS secured \fw%s", TEAM(owner, colour), TEAM(owner, name), b.name);
                    part_textcopy(vec(b.o).add(vec(0, 0, enttype[AFFINITY].radius)), "<super>\fzZeSECURED", PART_TEXT, game::eventiconfade, TEAM(owner, colour), 3, 1, -10);
                    if(game::dynlighteffects) adddynlight(vec(b.o).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec::hexcolor(TEAM(owner, colour)).mul(2.f), 500, 250);
                    entities::execlink(NULL, b.ent, false);
                }
            }
            else if(b.owner)
            {
                gameent *d = NULL, *e = NULL;
                int numdyns = game::numdynents();
                loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && e->type == ENT_PLAYER && insideaffinity(b, e))
                    if((d = e) == game::focus) break;
                game::announcef(S_V_FLAGOVERTHROWN, CON_INFO, d, true, "\fateam \fs\f[%d]%s\fS overthrew \fw%s", TEAM(enemy, colour), TEAM(enemy, name), b.name);
                part_textcopy(vec(b.o).add(vec(0, 0, enttype[AFFINITY].radius)), "<super>\fzZeOVERTHROWN", PART_TEXT, game::eventiconfade, TEAM(enemy, colour), 3, 1, -10);
                if(game::dynlighteffects) adddynlight(vec(b.o).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec::hexcolor(TEAM(enemy, colour)).mul(2.f), 500, 250);
                entities::execlink(NULL, b.ent, false);
            }
            b.converted = converted;
        }
        b.owner = owner;
        b.enemy = enemy;
    }

    void setscore(int team, int total)
    {
        hud::teamscore(team).total = total;
    }

    int aiowner(gameent *d)
    {
        loopv(st.flags) if(entities::ents.inrange(st.flags[i].ent) && entities::ents[d->aientity]->links.find(st.flags[i].ent) >= 0)
            return st.flags[i].owner ? st.flags[i].owner : st.flags[i].enemy;
        return d->team;
    }

    bool aicheck(gameent *d, ai::aistate &b)
    {
        return false;
    }

    void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests)
    {
        if(d->aitype == AI_BOT)
        {
            vec pos = d->feetpos();
            loopvj(st.flags)
            {
                defendstate::flag &f = st.flags[j];
                static vector<int> targets; // build a list of others who are interested in this
                targets.setsize(0);
                ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, j, true);
                gameent *e = NULL;
                bool regen = !m_regen(game::gamemode, game::mutators) || d->health >= m_health(game::gamemode, game::mutators, d->model);
                int numdyns = game::numdynents();
                loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
                {
                    vec ep = e->feetpos();
                    if(targets.find(e->clientnum) < 0 && ep.squaredist(f.o) <= (enttype[AFFINITY].radius*enttype[AFFINITY].radius))
                        targets.add(e->clientnum);
                }
                if((!regen && f.owner == ai::owner(d)) || (targets.empty() && (f.owner != ai::owner(d) || f.enemy)))
                {
                    ai::interest &n = interests.add();
                    n.state = ai::AI_S_DEFEND;
                    n.node = ai::closestwaypoint(f.o, ai::CLOSEDIST, false);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.o)/(!regen ? 100.f : 1.f);
                    n.tolerance = 0.25f;
                    n.team = true;
                }
            }
        }
    }

    bool aidefense(gameent *d, ai::aistate &b)
    {
        if(st.flags.inrange(b.target))
        {
            defendstate::flag &f = st.flags[b.target];
            bool regen = d->aitype != AI_BOT || !m_regen(game::gamemode, game::mutators) || d->health >= m_health(game::gamemode, game::mutators, d->model);
            int walk = f.enemy && f.enemy != ai::owner(d) ? 1 : 0;
            if(regen && (!f.enemy && ai::owner(d) == f.owner))
            {
                static vector<int> targets; // build a list of others who are interested in this
                targets.setsize(0);
                ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
                if(d->aitype == AI_BOT)
                {
                    gameent *e = NULL;
                    int numdyns = game::numdynents();
                    float mindist = enttype[AFFINITY].radius*4; mindist *= mindist;
                    loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
                    {
                        vec ep = e->feetpos();
                        if(targets.find(e->clientnum) < 0 && ep.squaredist(f.o) <= mindist)
                            targets.add(e->clientnum);
                    }
                }
                if(!targets.empty())
                {
                    if(lastmillis-b.millis >= (201-d->skill)*33)
                    {
                        d->ai->tryreset = true; // re-evaluate so as not to herd
                        return true;
                    }
                    else walk = 2;
                }
                else walk = 1;
            }
            return ai::defense(d, b, f.o, !f.enemy || m_gsp3(game::gamemode, game::mutators) ? ai::CLOSEDIST : float(enttype[AFFINITY].radius), !f.enemy || m_gsp3(game::gamemode, game::mutators) ? ai::CLOSEDIST : float(enttype[AFFINITY].radius*walk*8), walk);
        }
        return false;
    }

    bool aipursue(gameent *d, ai::aistate &b)
    {
        b.type = ai::AI_S_DEFEND;
        return aidefense(d, b);
    }

    void removeplayer(gameent *d)
    {
    }
}
