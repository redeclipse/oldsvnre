#include "game.h"
namespace capture
{
    capturestate st;

    bool carryaffinity(gameent *d)
    {
        loopv(st.flags) if(st.flags[i].owner == d) return true;
        return false;
    }

    bool dropaffinity(gameent *d)
    {
        if(m_capture(game::gamemode) && !m_gsp3(game::gamemode, game::mutators) && carryaffinity(d) && d->action[AC_AFFINITY])
        {
            vec inertia = vec(d->vel).add(d->falling);
            client::addmsg(N_DROPAFFIN, "ri8", d->clientnum, -1, int(d->o.x*DMF), int(d->o.y*DMF), int(d->o.z*DMF), int(inertia.x*DMF), int(inertia.y*DMF), int(inertia.z*DMF));
            d->action[AC_AFFINITY] = false;
            d->actiontime[AC_AFFINITY] = 0;
            return true;
        }
        return false;
    }

    void preload()
    {
        loadmodel("flag", -1, true);
    }

    void drawblips(int w, int h, float blend)
    {
        static vector<int> hasflags; hasflags.setsize(0);
        loopv(st.flags) if(entities::ents.inrange(st.flags[i].ent) && st.flags[i].owner == game::focus) hasflags.add(i);
        loopv(st.flags)
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            loopk(2)
            {
                vec dir, pos, colour = vec::hexcolor(TEAM(f.team, colour));
                const char *tex = hud::flagtex;
                bool arrow = false;
                float fade = blend*hud::radaraffinityblend, size = hud::radaraffinitysize;
                int millis = lastmillis-f.interptime;
                if(millis < 1000) size *= 1.f+(1-clamp(float(millis)/1000.f, 0.f, 1.f));
                if(f.owner) size *= 0.75f;
                if(k)
                {
                    if(!(f.base&BASE_FLAG) || f.owner == game::focus || (!f.owner && !f.droptime)) break;
                    pos = f.pos();
                    dir = vec(pos).sub(camera1->o);
                    int interval = lastmillis%500;
                    if(interval >= 300 || interval <= 200) fade *= clamp(interval >= 300 ? 1.f-((interval-300)/200.f) : interval/200.f, 0.f, 1.f);
                }
                else
                {
                    pos = f.spawnloc;
                    dir = vec(pos).sub(camera1->o);
                    float dist = dir.magnitude(), diff = dist <= hud::radarrange() ? clamp(1.f-(dist/hud::radarrange()), 0.f, 1.f) : 0.f;
                    if(iscapturehome(f, game::focus->team) && !m_gsp3(game::gamemode, game::mutators) && !hasflags.empty())
                    {
                        fade += (1.f-fade)*diff;
                        size *= 1.25f;
                        tex = hud::arrowtex;
                        arrow = true;
                    }
                    else if(!(f.base&BASE_FLAG) || f.owner || f.droptime)
                    {
                        fade += (1.f-fade)*diff;
                        tex = hud::alerttex;
                    }
                }
                if(hud::radaraffinitynames > (arrow ? 0 : 1)) hud::drawblip(tex, 3, w, h, size, fade, arrow ? 0 : hud::radarstyle, arrow ? dir : pos, colour, "radar", "\f[%d]%s", TEAM(f.team, colour), k ? "flag" : "base");
                else hud::drawblip(tex, 3, w, h, hud::radaraffinitysize, fade, arrow ? 0 : hud::radarstyle, arrow ? dir : pos, colour);
            }
        }
    }

    char *buildflagstr(vector<int> &f, bool named = false)
    {
        static string s; s[0] = 0;
        loopv(f)
        {
            defformatstring(d)("%s\f[%d]\f(%s)%s", i && named ? " " : "", TEAM(st.flags[f[i]].team, colour), hud::flagtex, named ? TEAM(st.flags[f[i]].team, name) : "");
            concatstring(s, d);
        }
        return s;
    }

    void drawnotices(int w, int h, int &tx, int &ty, float blend)
    {
        if(game::player1->state == CS_ALIVE && hud::shownotices >= 3)
        {
            static vector<int> hasflags, takenflags, droppedflags;
            hasflags.setsize(0); takenflags.setsize(0); droppedflags.setsize(0);
            loopv(st.flags)
            {
                capturestate::flag &f = st.flags[i];
                if(f.owner == game::player1) hasflags.add(i);
                else if(iscaptureaffinity(f, game::player1->team))
                {
                    if(f.owner && f.owner->team != game::player1->team) takenflags.add(i);
                    else if(f.droptime) droppedflags.add(i);
                }
            }
            if(!hasflags.empty())
            {
                pushfont("emphasis");
                char *str = buildflagstr(hasflags, hasflags.length() <= 3);
                ty += draw_textx("You have: \fs%s\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, str)*hud::noticescale;
                popfont();
                SEARCHBINDCACHE(altkey)("action 9", 0);
                pushfont("sub");
                ty += draw_textx("Press \fs\fc%s\fS to drop", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, altkey)*hud::noticescale;
                popfont();
            }
            pushfont("default");
            if(!takenflags.empty())
            {
                char *str = buildflagstr(takenflags, takenflags.length() <= 3);
                ty += draw_textx("%s taken: \fs%s\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, takenflags.length() == 1 ? "Flag" : "Flags", str)*hud::noticescale;
            }
            if(!droppedflags.empty())
            {
                char *str = buildflagstr(droppedflags, droppedflags.length() <= 3);
                ty += draw_textx("%s dropped: \fs%s\fS", tx, ty, 255, 255, 255, int(255*blend), TEXT_CENTERED, -1, -1, droppedflags.length() == 1 ? "Flag" : "Flags", str)*hud::noticescale;
            }
            popfont();
        }
    }

    int drawinventory(int x, int y, int s, int m, float blend)
    {
        int sy = 0;
        loopv(st.flags) if(st.flags[i].base&BASE_FLAG)
        {
            if(y-sy-s < m) break;
            capturestate::flag &f = st.flags[i];
            bool headsup = hud::chkcond(hud::inventorygame, game::player1->state == CS_SPECTATOR || f.team == TEAM_NEUTRAL || f.team == game::focus->team);
            if(headsup || f.lastowner == game::focus)
            {
                int millis = lastmillis-f.interptime, colour = TEAM(f.team, colour);
                float skew = headsup ? hud::inventoryskew : 0.f, fade = blend*hud::inventoryblend;
                vec c = vec::hexcolor(colour);
                if(f.owner || f.droptime)
                {
                    if(f.owner == game::focus)
                    {
                        float pc = (millis%1000)/500.f, amt = pc > 1 ? 2.f-pc : pc;
                        fade += (1.f-fade)*amt;
                        if(millis <= 1000) skew += (1.f-skew)*clamp(float(millis)/1000.f, 0.f, 1.f);
                        else skew = 1; // override it
                    }
                    else if(millis <= 1000) skew += (1.f-skew)*clamp(float(millis)/1000.f, 0.f, 1.f);
                    else skew = 1;
                }
                else if(millis <= 1000) skew += (1.f-skew)-(clamp(float(millis)/1000.f, 0.f, 1.f)*(1.f-skew));
                int oldy = y-sy;
                sy += hud::drawitem(hud::flagtex, x, oldy, s, false, c.r, c.g, c.b, fade, skew, "sub", f.owner ? (f.team == f.owner->team ? "\fysecured by" : "\frtaken by") : (f.droptime ? "\fodropped" : ""));
                if((f.base&BASE_FLAG) && (f.droptime || (m_gsp3(game::gamemode, game::mutators) && f.taketime && f.owner && f.owner->team != f.team)))
                {
                    float wait = f.droptime ? clamp((lastmillis-f.droptime)/float(captureresetdelay), 0.f, 1.f) : clamp((lastmillis-f.taketime)/float(captureresetdelay), 0.f, 1.f);
                    if(wait < 1) hud::drawprogress(x, oldy, wait, 1-wait, s, false, c.r, c.g, c.b, fade*0.25f, skew);
                    hud::drawprogress(x, oldy, 0, wait, s, false, c.r, c.g, c.b, fade, skew, "default", "%d%%", int(wait*100.f));
                }
                if(f.owner) hud::drawitemsubtext(x, oldy, s, TEXT_RIGHT_UP, skew, "sub", fade, "\fs%s\fS", game::colorname(f.owner));
            }
        }
        return sy;
    }

    void checkcams(vector<cament> &cameras)
    {
        loopv(st.flags) // flags/bases
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            int pri = f.team == game::player1->team ? 1 : 0;
            if(f.owner || f.droptime) pri++;
            vec pos = f.pos(); pos.z += enttype[AFFINITY].radius/2;
            cameras.add(cament(pos, cament::AFFINITY, i, pri));
        }
    }

    void updatecam(cament &c)
    {
        switch(c.type)
        {
            case cament::PLAYER:
            {
                if(c.player) loopv(st.flags)
                {
                    capturestate::flag &f = st.flags[i];
                    if(f.owner == c.player) c.pri += f.team == game::player1->team ? 3 : (c.player->team == game::player1->team ? 2 : 1);
                }
                break;
            }
            case cament::AFFINITY:
            {
                if(st.flags.inrange(c.id))
                {
                    capturestate::flag &f = st.flags[c.id];
                    int pri = f.team == game::player1->team ? 1 : 0;
                    if(f.owner || f.droptime) pri++;
                    c.pos = f.pos(); c.pos.z += enttype[AFFINITY].radius/2;
                    c.pri = pri;
                    if(f.owner) c.player = f.owner;
                }
                break;
            }
            default: break;
        }
    }

    void render()
    {
        loopv(st.flags) // flags/bases
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || (f.owner == game::focus && !game::thirdpersonview(true))) continue;
            entitylight *light = &entities::ents[f.ent]->light;
            light->material[0] = bvec(TEAM(f.team, colour));
            vec above(f.spawnloc);
            float trans = 0.f;
            if((f.base&BASE_FLAG) && !f.owner && !f.droptime)
            {
                int millis = lastmillis-f.interptime;
                if(millis <= 1000) trans += float(millis)/1000.f;
                else trans = 1.f;
            }
            else if(f.base&BASE_HOME) trans = 0.25f;
            if(trans > 0) rendermodel(light, "flag", ANIM_MAPMODEL|ANIM_LOOP, above, entities::ents[f.ent]->attrs[1], entities::ents[f.ent]->attrs[2], 0, MDL_DYNSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED, NULL, NULL, 0, 0, trans);
            above.z += enttype[AFFINITY].radius/2+2.5f;
            if((f.base&BASE_HOME) || (!f.owner && !f.droptime))
            {
                defformatstring(info)("<super>%s %s", TEAM(f.team, name), f.base&BASE_HOME ? "base" : "flag");
                part_textcopy(above, info, PART_TEXT, 1, TEAM(f.team, colour), 2, max(trans, 0.5f));
                above.z += 2.5f;
            }
            if((f.base&BASE_FLAG) && ((m_gsp(game::gamemode, game::mutators) && f.droptime) || (m_gsp3(game::gamemode, game::mutators) && f.taketime && f.owner && f.owner->team != f.team)))
            {
                float wait = f.droptime ? clamp((lastmillis-f.droptime)/float(captureresetdelay), 0.f, 1.f) : clamp((lastmillis-f.taketime)/float(captureresetdelay), 0.f, 1.f);
                part_icon(above, textureload(hud::progresstex, 3), 3, max(trans, 0.5f), 0, 0, 1, TEAM(f.team, colour), (lastmillis%1000)/1000.f, 0.1f);
                part_icon(above, textureload(hud::progresstex, 3), 2, max(trans, 0.5f)*0.25f, 0, 0, 1, TEAM(f.team, colour));
                part_icon(above, textureload(hud::progresstex, 3), 2, max(trans, 0.5f), 0, 0, 1, TEAM(f.team, colour), 0, wait);
                above.z += 0.5f;
                defformatstring(str)("<emphasis>%d%%", int(wait*100.f)); part_textcopy(above, str, PART_TEXT, 1, 0xFFFFFF, 2, max(trans, 0.5f)*0.5f);
                above.z += 2.5f;
            }
            if((f.base&BASE_FLAG) && (f.owner || f.droptime))
            {
                if(f.owner)
                {
                    defformatstring(info)("<super>%s", game::colorname(f.owner));
                    part_textcopy(above, info, PART_TEXT, 1, 0xFFFFFF, 2, max(trans, 0.5f));
                    above.z += 1.5f;
                }
                const char *info = f.owner ? (f.team == f.owner->team ? "\fysecured by" : "\frtaken by") : "\fodropped";
                part_text(above, info, PART_TEXT, 1, TEAM(f.team, colour), 2, max(trans, 0.5f));
            }
        }
        static vector<int> numflags, iterflags; // dropped/owned
        loopv(numflags) numflags[i] = iterflags[i] = 0;
        loopv(st.flags)
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !(f.base&BASE_FLAG) || !f.owner) continue;
            while(numflags.length() <= f.owner->clientnum) { numflags.add(0); iterflags.add(0); }
            numflags[f.owner->clientnum]++;
        }
        loopv(st.flags)
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !(f.base&BASE_FLAG) || (!f.owner && !f.droptime)) continue;
            vec above(f.pos());
            float yaw = 0;
            if(f.owner)
            {
                yaw += f.owner->yaw-45.f+(90/float(numflags[f.owner->clientnum]+1)*(iterflags[f.owner->clientnum]+1));
                while(yaw >= 360.f) yaw -= 360.f;
            }
            else
            {
                yaw += (f.interptime+(360/st.flags.length()*i))%360;
                if(f.proj) above.z -= f.proj->height;
            }
            int interval = lastmillis%1000;
            entitylight *light = &f.light;
            light->material[0] = bvec(TEAM(f.team, colour));
            light->effect = vec::hexcolor(TEAM(f.team, colour)).mul(interval >= 500 ? (1000-interval)/500.f : interval/500.f);
            rendermodel(light, "flag", ANIM_MAPMODEL|ANIM_LOOP, above, yaw, 0, 0, MDL_DYNSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHT|MDL_LIGHTFX, NULL, NULL, 0, 0, 1);
            above.z += enttype[AFFINITY].radius*2/3;
            if(f.owner) { above.z += iterflags[f.owner->clientnum]*2; iterflags[f.owner->clientnum]++; }
            defformatstring(info)("<super>%s flag", TEAM(f.team, name));
            part_textcopy(above, info, PART_TEXT, 1, TEAM(f.team, colour), 2, 1);
            above.z += 2.5f;
            if((f.base&BASE_FLAG) && (f.droptime || (m_gsp3(game::gamemode, game::mutators) && f.taketime && f.owner && f.owner->team != f.team)))
            {
                float wait = f.droptime ? clamp((lastmillis-f.droptime)/float(captureresetdelay), 0.f, 1.f) : clamp((lastmillis-f.taketime)/float(captureresetdelay), 0.f, 1.f);
                part_icon(above, textureload(hud::progresstex, 3), 3, 1, 0, 0, 1, TEAM(f.team, colour), (lastmillis%1000)/1000.f, 0.1f);
                part_icon(above, textureload(hud::progresstex, 3), 2, 0.25f, 0, 0, 1, TEAM(f.team, colour));
                part_icon(above, textureload(hud::progresstex, 3), 2, 1, 0, 0, 1, TEAM(f.team, colour), 0, wait);
                above.z += 0.5f;
                defformatstring(str)("%d%%", int(wait*100.f)); part_textcopy(above, str, PART_TEXT, 1, 0xFFFFFF, 2, 1);
                above.z += 2.5f;
            }
        }
    }

    void adddynlights()
    {
        loopv(st.flags)
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent)) continue;
            float trans = 1.f;
            if(!f.owner)
            {
                int millis = lastmillis-f.interptime;
                if(millis <= 1000) trans = float(millis)/1000.f;
            }
            adddynlight(vec(f.pos()).add(vec(0, 0, enttype[AFFINITY].radius/2*trans)), enttype[AFFINITY].radius*trans, vec::hexcolor(TEAM(f.team, colour)), 0, 0, DL_KEEP);
        }
    }

    void reset()
    {
        st.reset();
    }

    void setup()
    {
        #define setupaddaffinity(a,b) \
        { \
            index = st.addaffinity(entities::ents[a]->o, entities::ents[a]->attrs[0], b); \
            if(st.flags.inrange(index)) st.flags[index].ent = a; \
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
        #define setuphomeaffinity(a) if(!added) { setupaddaffinity(a, BASE_HOME); added = true; }
        int index = -1;
        loopv(entities::ents)
        {
            setupchkaffinity(i, { continue; });
            bool added = false; // check for a linked flag to see if we should use a seperate flag/home assignment
            loopvj(entities::ents[i]->links) if(entities::ents.inrange(entities::ents[i]->links[j]))
            {
                setupchkaffinity(entities::ents[i]->links[j],
                {
                    capturestate::flag &f = st.flags[already];
                    if((f.base&BASE_FLAG) && (f.base&BASE_HOME)) // got found earlier, but it is linked!
                        f.base &= ~BASE_HOME;
                    setuphomeaffinity(i);
                    continue;
                });
                setupaddaffinity(entities::ents[i]->links[j], BASE_FLAG); // add link as flag
                setuphomeaffinity(i);
            }
            if(!added && isteam(game::gamemode, game::mutators, entities::ents[i]->attrs[0], TEAM_FIRST)) // not linked and is a team flag
                setupaddaffinity(i, BASE_BOTH); // add as both
        }
        if(!st.flags.empty()) loopi(numteams(game::gamemode, game::mutators))
        {
            bool found = false;
            loopvj(st.flags) if(st.flags[j].team == i+TEAM_FIRST) { found = true; break; }
            if(!found)
            {
                loopvj(st.flags) if(st.flags[j].team == TEAM_NEUTRAL) { found = true; break; }
                loopvj(st.flags) st.flags[j].team = TEAM_NEUTRAL;
                if(!found)
                {
                    loopvj(st.flags) st.flags[j].base &= ~BASE_FLAG;
                    int best = -1;
                    float margin = 1e16f, mindist = 1e16f;
                    vector<int> route;
                    entities::avoidset obstacles;
                    for(int q = 0; q < 2; q++) loopvj(entities::ents)
                    {
                        extentity *e = entities::ents[j];
                        setupchkaffinity(j, { continue; });
                        vector<float> dists;
                        float v = 0;
                        if(!q)
                        {
                            int node = entities::closestent(WAYPOINT, e->o, 1e16f, true);
                            bool found = true;
                            loopvk(st.flags)
                            {
                                int goal = entities::closestent(WAYPOINT, st.flags[k].spawnloc, 1e16f, true);
                                float u[2] = { entities::route(node, goal, route, obstacles), entities::route(goal, node, route, obstacles) };
                                if(u[0] > 0 && u[1] > 0) v += dists.add((u[0]+u[1])*0.5f);
                                else
                                {
                                    found = false;
                                    break;
                                }
                            }
                            if(!found)
                            {
                                best = -1;
                                margin = mindist = 1e16f;
                                break;
                            }
                        }
                        else loopvk(st.flags) v += dists.add(st.flags[k].spawnloc.dist(e->o));
                        v /= st.flags.length();
                        if(v <= mindist)
                        {
                            float m = 0;
                            loopvk(dists) if(k) m += fabs(dists[k]-dists[k-1]);
                            if(m <= margin) { best = j; margin = m; mindist = v; }
                        }
                    }
                    if(entities::ents.inrange(best)) setupaddaffinity(best, BASE_FLAG);
                }
                break;
            }
        }
    }

    void sendaffinity(packetbuf &p)
    {
        putint(p, N_INITAFFIN);
        putint(p, st.flags.length());
        loopv(st.flags)
        {
            capturestate::flag &f = st.flags[i];
            putint(p, f.team);
            putint(p, f.base);
            loopk(3) putint(p, int(f.spawnloc[k]*DMF));
        }
    }

    void setscore(int team, int total)
    {
        hud::teamscore(team).total = total;
    }

    void parseaffinity(ucharbuf &p, bool commit)
    {
        int numflags = getint(p);
        loopi(numflags)
        {
            int team = getint(p), base = getint(p), owner = getint(p), dropped = 0;
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
                capturestate::flag &f = st.flags[i];
                f.team = team;
                f.base = base;
                if(owner >= 0) st.takeaffinity(i, game::getclient(owner), lastmillis);
                else if(dropped) st.dropaffinity(i, droploc, inertia, lastmillis);
            }
        }
    }

    void dropaffinity(gameent *d, int i, const vec &droploc, const vec &inertia, int target)
    {
        if(!st.flags.inrange(i)) return;
        capturestate::flag &f = st.flags[i];
        game::announcef(S_V_FLAGDROP, d == game::focus ? CON_SELF : CON_INFO, d, "\fa%s dropped the the \fs\f[%d]%s\fS flag", game::colorname(d), TEAM(f.team, colour), TEAM(f.team, name));
        st.dropaffinity(i, droploc, inertia, lastmillis);
    }

    void removeplayer(gameent *d)
    {
        loopv(st.flags) if(st.flags[i].owner == d)
        {
            capturestate::flag &f = st.flags[i];
            st.dropaffinity(i, f.owner->o, f.owner->vel, lastmillis);
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
                defformatstring(text)("<super>\fzZe%s",str);
                part_textcopy(vec(to).add(vec(0, 0, enttype[AFFINITY].radius)), text, PART_TEXT, game::eventiconfade, TEAM(team, colour), 3, 1, -10);
            }
            if(game::dynlighteffects) adddynlight(vec(to).add(vec(0, 0, enttype[AFFINITY].radius)), enttype[AFFINITY].radius*2, vec::hexcolor(TEAM(team, colour)).mul(2.f), 500, 250);
        }
        if(from.x >= 0 && to.x >= 0) part_trail(PART_SPARK, 500, from, to, TEAM(team, colour), 1, 1, -10);
    }

    void returnaffinity(gameent *d, int i)
    {
        if(!st.flags.inrange(i)) return;
        capturestate::flag &f = st.flags[i];
        affinityeffect(i, d->team, d->feetpos(), f.spawnloc, m_gsp(game::gamemode, game::mutators) ? 2 : 3, "RETURNED");
        game::announcef(S_V_FLAGRETURN, d == game::focus ? CON_SELF : CON_INFO, d, "\fa%s returned the \fs\f[%d]%s\fS flag (time taken: \fs\fc%s\fS)", game::colorname(d), TEAM(f.team, colour), TEAM(f.team, name), hud::timetostr(lastmillis-(m_gsp1(game::gamemode, game::mutators) || m_gsp3(game::gamemode, game::mutators) ? f.taketime : f.droptime)));
        st.returnaffinity(i, lastmillis);
    }

    void resetaffinity(int i, int value)
    {
        if(!st.flags.inrange(i)) return;
        capturestate::flag &f = st.flags[i];
        if(value > 0)
        {
            affinityeffect(i, TEAM_NEUTRAL, f.droploc, f.spawnloc, 3, "RESET");
            game::announcef(S_V_FLAGRESET, CON_INFO, NULL, "\fathe \fs\f[%d]%s\fS flag has been reset", TEAM(f.team, colour), TEAM(f.team, name));
        }
        st.returnaffinity(i, lastmillis);
    }

    void scoreaffinity(gameent *d, int relay, int goal, int score)
    {
        if(!st.flags.inrange(relay)) return;
        capturestate::flag &f = st.flags[relay];
        if(st.flags.inrange(goal))
        {
            capturestate::flag &g = st.flags[goal];
            affinityeffect(goal, d->team, g.spawnloc, f.spawnloc, 3, "CAPTURED");
        }
        else affinityeffect(goal, d->team, f.pos(), f.spawnloc, 3, "CAPTURED");
        hud::teamscore(d->team).total = score;
        game::announcef(S_V_FLAGSCORE, d == game::focus ? CON_SELF : CON_INFO, d, "\fa%s scored the \fs\f[%d]%s\fS flag for \fs\f[%d]%s\fS team (score: \fs\fc%d\fS, time taken: \fs\fc%s\fS)", game::colorname(d), TEAM(f.team, colour), TEAM(f.team, name), TEAM(d->team, colour), TEAM(d->team, name), score, hud::timetostr(lastmillis-f.taketime));
        st.returnaffinity(relay, lastmillis);
    }

    void takeaffinity(gameent *d, int i)
    {
        if(!st.flags.inrange(i)) return;
        capturestate::flag &f = st.flags[i];
        d->action[AC_AFFINITY] = false;
        d->actiontime[AC_AFFINITY] = 0;
        affinityeffect(i, d->team, d->feetpos(), f.pos(), 1, f.team == d->team ? "SECURED" : "TAKEN");
        game::announcef(f.team == d->team ? S_V_FLAGSECURED : S_V_FLAGPICKUP, d == game::focus ? CON_SELF : CON_INFO, d, "\fa%s %s the \fs\f[%d]%s\fS flag", game::colorname(d), f.droptime ? (f.team == d->team ? "secured" : "picked up") : "stole", TEAM(f.team, colour), TEAM(f.team, name));
        st.takeaffinity(i, d, lastmillis);
    }

    void checkaffinity(gameent *d)
    {
        vec o = d->feetpos();
        loopv(st.flags)
        {
            capturestate::flag &f = st.flags[i];
            if(!entities::ents.inrange(f.ent) || !(f.base&BASE_FLAG) || f.owner) continue;
            if(f.droptime) f.droploc = f.pos();
            if(f.pickuptime && lastmillis-f.pickuptime <= 1000) continue;
            if(f.team == d->team && !m_gsp3(game::gamemode, game::mutators) && (m_gsp2(game::gamemode, game::mutators) || !f.droptime)) continue;
            if(f.lastowner == d && f.droptime && (capturepickupdelay < 0 || lastmillis-f.droptime <= capturepickupdelay))
                continue;
            if(o.dist(f.pos()) <= enttype[AFFINITY].radius*2/3)
            {
                client::addmsg(N_TAKEAFFIN, "ri2", d->clientnum, i);
                f.pickuptime = lastmillis;
            }
        }
        dropaffinity(d);
    }

    bool aihomerun(gameent *d, ai::aistate &b)
    {
        if(!m_gsp3(game::gamemode, game::mutators))
        {
            vec pos = d->feetpos();
            loopk(2)
            {
                int goal = -1;
                loopv(st.flags)
                {
                    capturestate::flag &g = st.flags[i];
                    if(iscapturehome(g, ai::owner(d)) && (k || (!g.owner && !g.droptime)) &&
                        (!st.flags.inrange(goal) || g.pos().squaredist(pos) < st.flags[goal].pos().squaredist(pos)))
                    {
                        goal = i;
                    }
                }
                if(st.flags.inrange(goal) && ai::makeroute(d, b, st.flags[goal].pos()))
                {
                    d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, goal);
                    return true;
                }
            }
        }
	    if(b.type == ai::AI_S_INTEREST && b.targtype == ai::AI_T_NODE) return true; // we already did this..
		if(ai::randomnode(d, b, ai::ALERTMIN, 1e16f))
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
                capturestate::flag &g = st.flags[i];
                if(g.owner == d) hasflags.add(i);
                else if(iscaptureaffinity(g, ai::owner(d)) && (m_gsp3(game::gamemode, game::mutators) || (g.owner && ai::owner(g.owner) != ai::owner(d)) || g.droptime))
                    takenflags.add(i);
            }
            if(!hasflags.empty() && !m_gsp3(game::gamemode, game::mutators)) return aihomerun(d, b);
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
            capturestate::flag &f = st.flags[j];
            bool home = iscapturehome(f, ai::owner(d));
            if(d->aitype == AI_BOT && (!home || m_gsp3(game::gamemode, game::mutators)) && !(f.base&BASE_FLAG)) continue; // don't bother with other bases
            static vector<int> targets; // build a list of others who are interested in this
            targets.setsize(0);
            bool regen = d->aitype != AI_BOT || f.team == TEAM_NEUTRAL || m_gsp3(game::gamemode, game::mutators) || !m_regen(game::gamemode, game::mutators) || d->health >= m_health(game::gamemode, game::mutators, d->loadweap[0]);
            ai::checkothers(targets, d, home || d->aitype != AI_BOT ? ai::AI_S_DEFEND : ai::AI_S_PURSUE, ai::AI_T_AFFINITY, j, true);
            if(d->aitype == AI_BOT)
            {
                gameent *e = NULL;
                int numdyns = game::numdynents();
                loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
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
                    n.node = entities::closestent(WAYPOINT, f.pos(), ai::CLOSEDIST, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos())/(!regen ? 100.f : 1.f);
                }
            }
            else
            {
                if(targets.empty())
                { // attack the flag
                    ai::interest &n = interests.add();
                    n.state = d->aitype == AI_BOT ? ai::AI_S_PURSUE : ai::AI_S_DEFEND;
                    n.node = entities::closestent(WAYPOINT, f.pos(), ai::CLOSEDIST, true);
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
                        n.targtype = ai::AI_T_ACTOR;
                        n.score = d->o.squaredist(t->o);
                    }
                }
            }
        }
    }

    bool aidefense(gameent *d, ai::aistate &b)
    {
        if(!m_gsp3(game::gamemode, game::mutators) && d->aitype == AI_BOT)
        {
            static vector<int> hasflags;
            hasflags.setsize(0);
            loopv(st.flags)
            {
                capturestate::flag &g = st.flags[i];
                if(g.owner == d) hasflags.add(i);
            }
            if(!hasflags.empty()) return aihomerun(d, b);
        }
        if(st.flags.inrange(b.target))
        {
            capturestate::flag &f = st.flags[b.target];
            if(iscaptureaffinity(f, ai::owner(d)) && f.owner)
            {
                ai::violence(d, b, f.owner, false);
                if(d->aitype != AI_BOT) return true;
            }
            int walk = f.owner && ai::owner(f.owner) != ai::owner(d) ? 1 : 0;
            if(d->aitype == AI_BOT)
            {
                bool regen = !m_regen(game::gamemode, game::mutators) || d->health >= m_health(game::gamemode, game::mutators, d->loadweap[0]);
                if(regen && lastmillis-b.millis >= (201-d->skill)*33)
                {
                    static vector<int> targets; // build a list of others who are interested in this
                    targets.setsize(0);
                    ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
                    gameent *e = NULL;
                    int numdyns = game::numdynents();
                    loopi(numdyns) if((e = (gameent *)game::iterdynents(i)) && !e->ai && e->state == CS_ALIVE && ai::owner(d) == ai::owner(e))
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
                    capturestate::flag &g = st.flags[i];
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
            capturestate::flag &f = st.flags[b.target];
            b.idle = -1;
            if(iscapturehome(f, ai::owner(d)) && !m_gsp3(game::gamemode, game::mutators))
            {
                static vector<int> hasflags; hasflags.setsize(0);
                loopv(st.flags)
                {
                    capturestate::flag &g = st.flags[i];
                    if(g.owner == d) hasflags.add(i);
                }
                if(!hasflags.empty()) return ai::makeroute(d, b, f.pos());
                else if(!iscaptureaffinity(f, ai::owner(d))) return false;
            }
            if(iscaptureaffinity(f, ai::owner(d))) return f.owner ? ai::violence(d, b, f.owner, true) : ai::makeroute(d, b, f.pos());
            else return ai::makeroute(d, b, f.pos());
        }
        return false;
    }
}
