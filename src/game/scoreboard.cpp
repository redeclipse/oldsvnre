#include "game.h"

namespace hud
{
    static vector<score> scores;

    score &teamscore(int team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(cs.team == team) return cs;
        }
        score &cs = scores.add();
        cs.team = team;
        cs.total = 0;
        return cs;
    }

    void resetscores()
    {
        scores.shrink(0);
    }

    struct scoregroup : score
    {
        vector<gameent *> players;
    };
    vector<scoregroup *> groups;
    vector<gameent *> spectators;

    VAR(IDF_PERSIST, autoscores, 0, 2, 3); // 1 = when dead, 2 = also in spectv, 3 = and in waittv too
    VAR(IDF_PERSIST, scoresdelay, 0, 0, VAR_MAX); // otherwise use respawn delay
    VAR(IDF_PERSIST, scoresinfo, 0, 1, 1);
    VAR(IDF_PERSIST, scorehandles, 0, 1, 1);

    VAR(IDF_PERSIST, scorepj, 0, 0, 1);
    VAR(IDF_PERSIST, scoreping, 0, 1, 1);
    VAR(IDF_PERSIST, scorepoints, 0, 1, 1);
    VAR(IDF_PERSIST, scoretimer, 0, 1, 2);
    VAR(IDF_PERSIST, scorelaps, 0, 1, 2);
    VAR(IDF_PERSIST, scorefrags, 0, 2, 2);
    VAR(IDF_PERSIST, scoredeaths, 0, 2, 2);
    VAR(IDF_PERSIST, scoreratios, 0, 0, 2);
    VAR(IDF_PERSIST, scoreclientnum, 0, 1, 1);
    VAR(IDF_PERSIST, scorebotinfo, 0, 0, 1);
    VAR(IDF_PERSIST, scorespectators, 0, 1, 1);
    VAR(IDF_PERSIST, scoreconnecting, 0, 0, 1);
    VAR(IDF_PERSIST, scorehostinfo, 0, 0, 1);
    VAR(IDF_PERSIST, scoreicons, 0, 1, 1);
    VAR(IDF_PERSIST|IDF_HEX, scorehilight, 0, 0x888888, 0xFFFFFF);
    VAR(IDF_READONLY, scoretarget, -1, -1, VAR_MAX);

    static bool scoreson = false, scoresoff = false, shownscores = false;
    static int menustart = 0, menulastpress = 0;

    bool canshowscores()
    {
        if(!scoresoff && !scoreson && !shownscores && autoscores)
        {
            if(game::player1->state == CS_DEAD)
            {
                int delay = scoresdelay ? scoresdelay : m_delay(game::gamemode, game::mutators);
                if(!delay || lastmillis-game::player1->lastdeath > delay) return true;
            }
            else return game::tvmode() && autoscores >= (game::player1->state == CS_SPECTATOR ? 2 : 3);
        }
        return false;
    }

    static inline bool playersort(const gameent *a, const gameent *b)
    {
        if(a->state == CS_SPECTATOR || a->state == CS_EDITING)
        {
            if(b->state == CS_SPECTATOR || b->state == CS_EDITING) return strcmp(a->name, b->name) < 0;
            else return false;
        }
        else if(b->state == CS_SPECTATOR || b->state == CS_EDITING) return true;
        if(m_laptime(game::gamemode, game::mutators))
        {
            if((a->cptime && !b->cptime) || (a->cptime && b->cptime && a->cptime < b->cptime)) return true;
            if((b->cptime && !a->cptime) || (a->cptime && b->cptime && b->cptime < a->cptime)) return false;
        }
        else if(m_gauntlet(game::gamemode))
        {
            if(a->cplaps > b->cplaps) return true;
            if(b->cplaps > a->cplaps) return false;
        }
        if(a->points > b->points) return true;
        if(a->points < b->points) return false;
        if(a->frags > b->frags) return true;
        if(a->frags < b->frags) return false;
        return strcmp(a->name, b->name) < 0;
    }

    static inline bool scoregroupcmp(const scoregroup *x, const scoregroup *y)
    {
        if(!x->team)
        {
            if(y->team) return false;
        }
        else if(!y->team) return true;
        if(m_laptime(game::gamemode, game::mutators))
        {
            if((x->total && !y->total) || (x->total && y->total && x->total < y->total)) return true;
            if((y->total && !x->total) || (x->total && y->total && x->total > y->total)) return false;
        }
        else
        {
            if(x->total > y->total) return true;
            if(x->total < y->total) return false;
        }
        if(x->players.length() > y->players.length()) return true;
        if(x->players.length() < y->players.length()) return false;
        return x->team && y->team && x->team < y->team;
    }

    int groupplayers()
    {
        int numgroups = 0;
        spectators.shrink(0);
        int numdyns = game::numdynents();
        loopi(numdyns)
        {
            gameent *o = (gameent *)game::iterdynents(i);
            if(!o || o->actortype < A_ENEMY || (!scoreconnecting && !o->name[0])) continue;
            if(o->state == CS_SPECTATOR)
            {
                if(o != game::player1 || !client::demoplayback) spectators.add(o);
                continue;
            }
            int team = m_fight(game::gamemode) && m_team(game::gamemode, game::mutators) ? o->team : T_NEUTRAL;
            bool found = false;
            loopj(numgroups)
            {
                scoregroup &g = *groups[j];
                if(team != g.team) continue;
                if(team) g.total = teamscore(team).total;
                g.players.add(o);
                found = true;
                break;
            }
            if(found) continue;
            if(numgroups >= groups.length()) groups.add(new scoregroup);
            scoregroup &g = *groups[numgroups++];
            g.team = team;
            if(!team) g.total = 0;
            else if(m_team(game::gamemode, game::mutators)) g.total = teamscore(o->team).total;
            else g.total = o->points;

            g.players.shrink(0);
            g.players.add(o);
        }
        loopi(numgroups) groups[i]->players.sort(playersort);
        spectators.sort(playersort);
        groups.sort(scoregroupcmp, 0, numgroups);
        return numgroups;
    }

    void showscores(bool on, bool interm, bool onauto, bool ispress)
    {
        if(!client::waiting())
        {
            if(ispress)
            {
                bool within = menulastpress && totalmillis-menulastpress < PHYSMILLIS;
                if(on)
                {
                    if(within) onauto = true;
                    menulastpress = totalmillis;
                }
                else if(within && !scoresoff) { menulastpress = 0; return; }
            }
            if(!scoreson && on) menustart = guicb::starttime();
            scoresoff = !onauto;
            scoreson = on;
            if(m_play(game::gamemode) && m_fight(game::gamemode) && interm)
            {
                int numgroups = groupplayers();
                if(!numgroups) return;
                scoregroup &sg = *groups[0];
                if(m_team(game::gamemode, game::mutators))
                {
                    int anc = sg.players.find(game::player1) >= 0 ? S_V_YOUWIN : (game::player1->state != CS_SPECTATOR ? S_V_YOULOSE : -1);
                    if(m_defend(game::gamemode) && sg.total == INT_MAX)
                        game::announcef(anc, CON_MESG, NULL, true, "\fwteam %s secured all flags", game::colourteam(sg.team));
                    else
                    {
                        if(numgroups > 1 && sg.total == groups[1]->total)
                        {
                            mkstring(winner);
                            loopi(numgroups) if(i)
                            {
                                if(sg.total == groups[i]->total)
                                {
                                    defformatstring(tw)("%s, ", game::colourteam(groups[i]->team));
                                    concatstring(winner, tw);
                                }
                                else break;
                            }
                            game::announcef(S_V_DRAW, CON_MESG, NULL, true, "\fw%s tied %swith a total score of: \fs\fc%s\fS", game::colourteam(sg.team), winner, m_laptime(game::gamemode, game::mutators) ? timestr(sg.total) : intstr(sg.total));
                        }
                        else game::announcef(anc, CON_MESG, NULL, true, "\fwteam %s won the match with a total score of: \fs\fc%s\fS", game::colourteam(sg.team), m_laptime(game::gamemode, game::mutators) ? timestr(sg.total) : intstr(sg.total));
                    }
                }
                else
                {
                    int anc = sg.players[0] == game::player1 ? S_V_YOUWIN : (game::player1->state != CS_SPECTATOR ? S_V_YOULOSE : -1);
                    if(m_laptime(game::gamemode, game::mutators))
                    {
                        if(sg.players.length() > 1 && sg.players[0]->cptime == sg.players[1]->cptime)
                        {
                            mkstring(winner);
                            loopv(sg.players) if(i)
                            {
                                if(sg.players[0]->cptime == sg.players[i]->cptime)
                                {
                                    concatstring(winner, game::colourname(sg.players[i]));
                                    concatstring(winner, ", ");
                                }
                                else break;
                            }
                            game::announcef(S_V_DRAW, CON_MESG, NULL, true, "\fw%s tied %swith the fastest lap: \fs\fc%s\fS", game::colourname(sg.players[0]), winner, sg.players[0]->cptime ? timestr(sg.players[0]->cptime) : "dnf");
                        }
                        else game::announcef(anc, CON_MESG, NULL, true, "\fw%s won the match with the fastest lap: \fs\fc%s\fS", game::colourname(sg.players[0]), sg.players[0]->cptime ? timestr(sg.players[0]->cptime) : "dnf");
                    }
                    else
                    {
                        if(sg.players.length() > 1 && sg.players[0]->points == sg.players[1]->points)
                        {
                            mkstring(winner);
                            loopv(sg.players) if(i)
                            {
                                if(sg.players[0]->points == sg.players[i]->points)
                                {
                                    concatstring(winner, game::colourname(sg.players[i]));
                                    concatstring(winner, ", ");
                                }
                                else break;
                            }
                            game::announcef(S_V_DRAW, CON_MESG, NULL, true, "\fw%s tied %swith a total score of: \fs\fc%d\fS", game::colourname(sg.players[0]), winner, sg.players[0]->points);
                        }
                        else game::announcef(anc, CON_MESG, NULL, true, "\fw%s won the match with a total score of: \fs\fc%d\fS", game::colourname(sg.players[0]), sg.players[0]->points);
                    }
                }
            }
        }
        else scoresoff = scoreson = false;
    }

    const char *scorehost(gameent *d)
    {
        if(d->actortype > A_PLAYER)
        {
            static string hoststr;
            hoststr[0] = 0;
            gameent *e = game::getclient(d->ownernum);
            if(e)
            {
                concatstring(hoststr, game::colourname(e, NULL, false, false));
                concatstring(hoststr, " ");
            }
            defformatstring(owner)("[%d]", d->ownernum);
            concatstring(hoststr, owner);
            return hoststr;
        }
        return d->hostname;
    }

    void renderscoreboard(guient &g, bool firstpass)
    {
        g.start(menustart, menuscale, NULL, false, false);
        int numgroups = groupplayers();
        uilist(g, {
            uicenter(g, {
                uicenterlist(g, {
                    g.image(NULL, 6, true);
                });
                g.space(2);
                uicenterlist(g, {
                    g.space(0.25f);
                    uicenterlist(g, uifont(g, "emphasis", g.textf("%s", 0xFFFFFF, NULL, 0, *maptitle ? maptitle : mapname)));
                    if(*mapauthor) uicenterlist(g, uifont(g, "default", g.textf("by %s", 0xFFFFFF, NULL, 0, mapauthor)));
                    uicenterlist(g, uifont(g, "reduced", {
                        defformatstring(gname)("%s", server::gamename(game::gamemode, game::mutators, 0, 32));
                        g.textf("%s", 0xFFFFFF, NULL, 0, gname);
                        if((m_play(game::gamemode) || client::demoplayback) && game::timeremaining >= 0)
                        {
                            if(game::intermission) g.textf(", \fs\fyintermission\fS", 0xFFFFFF, NULL, 0);
                            else if(client::waitplayers) g.textf(", \fs\fywaiting\fS", 0xFFFFFF, NULL, 0);
                            else if(paused) g.textf(", \fs\fopaused\fS", 0xFFFFFF, NULL, 0);
                            else if(game::timeremaining) g.textf(", \fs\fg%s\fS remain", 0xFFFFFF, NULL, 0, timestr(game::timeremaining, 2));
                        }
                    }));
                    if(*connectname)
                    {
                        uicenterlist(g, {
                            uifont(g, "little", g.textf("\fdon ", 0xFFFFFF, NULL, 0));
                            if(*serverdesc) uifont(g, "reduced", g.textf("%s ", 0xFFFFFF, NULL, 0, serverdesc));
                            uifont(g, "little", g.textf("\fd(\fa%s:[%d]\fd)", 0xFFFFFF, NULL, 0, connectname, connectport));
                        });
                    }
                    if(game::player1->quarantine)
                    {
                        uicenterlist(g, uifont(g, "default", g.textf("You are \fzoyQUARANTINED", 0xFFFFFF, NULL, 0)));
                        uicenterlist(g, uifont(g, "reduced", g.textf("Please await instructions from a moderator", 0xFFFFFF, NULL, 0)));
                    }
                    if(client::demoplayback)
                    {
                        uicenterlist(g, {
                            uifont(g, "reduced", g.textf("Demo Playback in Progress", 0xFFFFFF, NULL, 0));
                        });
                    }
                    else if(client::waitplayers)
                    {
                        uicenterlist(g, {
                            uifont(g, "reduced", g.textf("Waiting for players", 0xFFFFFF, NULL, 0));
                        });
                    }
                    if(!game::intermission && !client::demoplayback)
                    {
                        if(game::player1->state == CS_DEAD || game::player1->state == CS_WAITING)
                        {
                            SEARCHBINDCACHE(attackkey)("primary", 0);
                            int sdelay = m_delay(game::gamemode, game::mutators);
                            int delay = game::player1->respawnwait(lastmillis, sdelay);
                            if(delay || m_duke(game::gamemode, game::mutators) || (m_fight(game::gamemode) && maxalive > 0))
                            {
                                uicenterlist(g, uifont(g, "reduced", {
                                    if(client::waitplayers || m_duke(game::gamemode, game::mutators)) g.textf("Queued for new round", 0xFFFFFF, NULL, 0);
                                    else if(delay) g.textf("%s: Down for \fs\fy%s\fS", 0xFFFFFF, NULL, 0, game::player1->state == CS_WAITING ? "Please Wait" : "Fragged", timestr(delay, -1));
                                    else if(game::player1->state == CS_WAITING && m_fight(game::gamemode) && maxalive > 0 && maxalivequeue)
                                    {
                                        int n = game::numwaiting();
                                        if(n) g.textf("Waiting for \fs\fy%d\fS %s", 0xFFFFFF, NULL, 0, n, n != 1 ? "players" : "player");
                                        else g.textf("You are \fs\fgnext\fS in the queue", 0xFFFFFF, NULL, 0);
                                    }
                                }));
                                if(game::player1->state != CS_WAITING && lastmillis-game::player1->lastdeath >= 500)
                                    uicenterlist(g, uifont(g, "little", g.textf("Press %s to enter respawn queue", 0xFFFFFF, NULL, 0, attackkey)));
                            }
                            else
                            {
                                uicenterlist(g, uifont(g, "reduced", g.textf("Ready to respawn", 0xFFFFFF, NULL, 0)));
                                if(game::player1->state != CS_WAITING) uicenterlist(g, uifont(g, "little", g.textf("Press %s to respawn now", 0xFFFFFF, NULL, 0, attackkey)));
                            }
                            if(shownotices >= 2)
                            {
                                uifont(g, "little", {
                                    if(game::player1->state == CS_WAITING && lastmillis-game::player1->lastdeath >= 500)
                                    {
                                        SEARCHBINDCACHE(waitmodekey)("waitmodeswitch", 3);
                                        uicenterlist(g, g.textf("Press %s to %s", 0xFFFFFF, NULL, 0, waitmodekey, game::tvmode() ? "interact" : "switch to TV"));
                                    }
                                    if(m_loadout(game::gamemode, game::mutators))
                                    {
                                        SEARCHBINDCACHE(loadkey)("showgui loadout", 0);
                                        uicenterlist(g, g.textf("Press %s to \fs%s\fS loadout", 0xFFFFFF, NULL, 0, loadkey, game::player1->loadweap.empty() ? "\fzoyselect" : "change"));
                                    }
                                    if(m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
                                    {
                                        SEARCHBINDCACHE(teamkey)("showgui team", 0);
                                        uicenterlist(g, g.textf("Press %s to change teams", 0xFFFFFF, NULL, 0, teamkey));
                                    }
                                });
                            }
                        }
                        else if(game::player1->state == CS_ALIVE)
                        {
                            uifont(g, "reduced", {
                                uicenterlist(g, {
                                    // In two cases, the main mode-description is not applicable
                                    if(m_bomber(game::gamemode) && m_gsp2(game::gamemode, game::mutators)) // hold bomber
                                        g.textf("%s", 0xFFFFFF, NULL, 0, gametype[game::gamemode].gsd[1]);
                                    else if(m_capture(game::gamemode) && m_gsp3(game::gamemode, game::mutators)) // protect capture
                                        g.textf("%s", 0xFFFFFF, NULL, 0, gametype[game::gamemode].gsd[2]);
                                    else if(m_defend(game::gamemode) && m_gsp2(game::gamemode, game::mutators)) // protect capture
                                        g.textf("%s", 0xFFFFFF, NULL, 0, gametype[game::gamemode].gsd[1]);
                                    else if(m_gauntlet(game::gamemode) && m_gsp1(game::gamemode, game::mutators)) // timed gauntlet
                                        g.textf("%s", 0xFFFFFF, NULL, 0, gametype[game::gamemode].gsd[0]);
                                    else g.textf("%s", 0xFFFFFF, NULL, 0, gametype[game::gamemode].desc);
                                });
                                if(m_team(game::gamemode, game::mutators))
                                    uicenterlist(g, g.textf("Playing for team %s", 0xFFFFFF, NULL, 0, game::colourteam(game::player1->team)));
                            });
                        }
                        else if(game::player1->state == CS_SPECTATOR)
                        {
                            uicenterlist(g, uifont(g, "reduced", g.textf("%s", 0xFFFFFF, NULL, 0, game::tvmode() ? "SpecTV" : "Spectating")));
                            SEARCHBINDCACHE(speconkey)("spectator 0", 1);
                            uifont(g, "little", {
                                uicenterlist(g, g.textf("Press %s to join the game", 0xFFFFFF, NULL, 0, speconkey));
                                if(!m_edit(game::gamemode) && shownotices >= 2)
                                {
                                    SEARCHBINDCACHE(specmodekey)("specmodeswitch", 1);
                                    uicenterlist(g, g.textf("Press %s to %s", 0xFFFFFF, NULL, 0, specmodekey, game::tvmode() ? "interact" : "switch to TV"));
                                }
                            });
                        }

                        if(m_edit(game::gamemode) && (game::focus->state != CS_EDITING || shownotices >= 4))
                        {
                            SEARCHBINDCACHE(editkey)("edittoggle", 1);
                            uicenterlist(g, uifont(g, "reduced", g.textf("Press %s to %s editmode", 0xFFFFFF, NULL, 0, editkey, game::focus->state != CS_EDITING ? "enter" : "exit")));
                        }
                    }

                    SEARCHBINDCACHE(scoreboardkey)("showscores", 1);
                    uicenterlist(g, uifont(g, "little", g.textf("%s %s to close this window", 0xFFFFFF, NULL, 0, scoresoff ? "Release" : "Press", scoreboardkey)));
                    uicenterlist(g, uifont(g, "tiny", g.textf("Double-tap to keep the window open", 0xFFFFFF, NULL, 0)));
                });
            });
        });
        g.space(0.5f);
        #define loopscoregroup(b) \
        { \
            loopv(sg.players) \
            { \
                gameent *o = sg.players[i]; \
                b; \
            } \
        }
        uifont(g, numgroups > 1 ? "little" : "reduced", {
            float namepad = 0;
            float handlepad = 0;
            float hostpad = 0;
            bool hashandle = false;
            bool hashost = false;
            bool hasbots = false;
            loopk(numgroups)
            {
                scoregroup &sg = *groups[k];
                loopscoregroup({
                    if(scorebotinfo && o->actortype > A_PLAYER) hasbots = true;
                    namepad = max(namepad, (float)text_width(game::colourname(o, NULL, false, true)));
                    if(scorehandles && o->handle[0])
                    {
                        handlepad = max(handlepad, (float)text_width(o->handle));
                        hashandle = true;
                    }
                    if(scorehostinfo)
                    {
                        const char *host = scorehost(o);
                        if(host && *host)
                        {
                            hostpad = max(hostpad, (float)text_width(host));
                            if(o->ownernum != game::player1->clientnum) hashost = true;
                        }
                    }
                });
            }
            namepad = max((namepad-text_width("name"))/(guibound[0]*2.f), 0.25f)*1.25f;
            if(hashandle) handlepad = max((handlepad-guibound[0])/(guibound[0]*2.f), 0.25f)*1.25f;
            if(hashost) hostpad = max((hostpad-guibound[0])/(guibound[0]*2.f), 0.25f)*1.25f;
            loopk(numgroups)
            {
                if((k%2)==0)
                {
                    if(k) g.space(0.5f);
                    g.pushlist();
                }
                uicenter(g, {
                    scoregroup &sg = *groups[k];
                    int bgcolor = sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators) ? TEAM(sg.team, colour) : 0x333333;
                    if(sg.team && m_team(game::gamemode, game::mutators))
                    {
                        g.pushlist();
                        uilist(g, {
                            g.background(bgcolor);
                            if(m_defend(game::gamemode) && ((defendlimit && sg.total >= defendlimit) || sg.total == INT_MAX))
                                g.textf("%s: WIN", 0xFFFFFF, teamtexname(sg.team), TEAM(sg.team, colour), TEAM(sg.team, name));
                            else if(m_laptime(game::gamemode, game::mutators)) g.textf("%s: %s", 0xFFFFFF, teamtexname(sg.team), TEAM(sg.team, colour), TEAM(sg.team, name), sg.total ? timestr(sg.total) : "\fadnf");
                            else g.textf("%s: %d", 0xFFFFFF, teamtexname(sg.team), TEAM(sg.team, colour), TEAM(sg.team, name), sg.total);
                            g.spring();
                        });
                        g.pushlist();
                    }

                    uilist(g, {
                        uicenterlist(g, uipad(g, 0.25f, g.strut(1)));
                        loopscoregroup(uicenterlist(g, {
                            uipad(g, 0.25f, uicenterlist(g, g.textf("\f[%d]\f($priv%stex)", 0xFFFFFF, NULL, 0, game::findcolour(o), hud::privname(o->privilege, o->actortype))));
                        }));
                    });

                    uilist(g, {
                        uicenterlist(g, uipad(g, namepad, uicenterlist(g, g.text("name", 0xFFFFFF))));
                        loopscoregroup(uicenterlist(g, {
                            uipad(g, 0.25f, uicenterlist(g, {
                                if(o == game::player1 && scorehilight) g.background(scorehilight);
                                uilist(g, uipad(g, 0.25f, {
                                    if(g.button(game::colourname(o, NULL, false, true), 0xFFFFFF)&GUI_UP)
                                    {
                                        scoretarget = o->clientnum;
                                        showgui("client");
                                    }
                                }));
                            }));
                        }));
                    });

                    if(scorepoints)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 1, g.text("points", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->points));
                            }));
                        });
                    }

                    if(m_trial(game::gamemode) || m_gauntlet(game::gamemode))
                    {
                        if(scoretimer && (scoretimer >= 2 || m_laptime(game::gamemode, game::mutators)))
                        {
                            uilist(g, {
                                uicenterlist(g, uipad(g, 4, g.text("best", 0xFFFFFF)));
                                loopscoregroup(uicenterlist(g, {
                                    uipad(g, 0.5f, g.textf("%s", 0xFFFFFF, NULL, 0, o->cptime ? timestr(o->cptime) : "\fadnf"))
                                }));
                            });
                        }
                        if(scorelaps && (scorelaps >= 2 || !m_laptime(game::gamemode, game::mutators)))
                        {
                            uilist(g, {
                                uicenterlist(g, uipad(g, 4, g.text("laps", 0xFFFFFF)));
                                loopscoregroup(uicenterlist(g, {
                                    uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->cplaps))
                                }));
                            });
                        }
                    }

                    if(scorefrags && (scorefrags >= 2 || m_dm(game::gamemode)))
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 1, g.text("frags", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->frags))
                            }));
                        });
                    }

                    if(scoredeaths && (scoredeaths >= 2 || m_dm(game::gamemode)))
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 1, g.text("deaths", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->deaths))
                            }));
                        });
                    }

                    if(scoreratios && (scoreratios >= 2 || m_dm(game::gamemode)))
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 1, g.text("ratio", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                float ratio = o->frags >= o->deaths ? (o->frags/float(max(o->deaths, 1))) : -(o->deaths/float(max(o->frags, 1)));
                                uipad(g, 0.5f, g.textf("%.1f\fs\fa:\fS%.1f", 0xFFFFFF, NULL, 0, ratio >= 0 ? ratio : 1.f, ratio >= 0 ? 1.f : -ratio))
                            }));
                        });
                    }

                    if(scorepj)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 2, g.text("pj", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->plag))
                            }));
                        });
                    }

                    if(scoreping)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 2, g.text("ping", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->ping))
                            }));
                        });
                    }

                    if(scoreclientnum || game::player1->privilege >= PRIV_ELEVATED)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 1, g.text("cn", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->clientnum))
                            }));
                        });
                    }

                    if(scorebotinfo && hasbots)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 1, g.text("sk", 0xFFFFFF)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, {
                                    if(o->actortype > A_PLAYER) g.textf("%d", 0xFFFFFF, NULL, 0, o->skill);
                                    else g.strut(1);
                                });
                            }));
                        });
                    }
                    if(scorehandles && hashandle)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, handlepad, g.strut(1)));
                            loopscoregroup({
                                uicenterlist(g, {
                                    uipad(g, 0.5f, g.textf("%s", 0xFFFFFF, NULL, 0, o->handle[0] ? o->handle : "-"))
                                });
                            });
                        });
                    }
                    if(scorehostinfo && hashost)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, hostpad, g.strut(1)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.5f, g.textf("%s", 0xFFFFFF, NULL, 0, scorehost(o)))
                            }));
                        });
                    }
                    if(scoreicons)
                    {
                        uilist(g, {
                            uicenterlist(g, uipad(g, 0.125f, g.strut(1)));
                            loopscoregroup(uicenterlist(g, {
                                uipad(g, 0.125f, {
                                    if(!m_team(game::gamemode, game::mutators) || o->team != game::focus->team)
                                    {
                                        if(game::focus->dominating.find(o) >= 0) g.text("", 0, dominatedtex, TEAM(sg.team, colour));
                                        else if(game::focus->dominated.find(o) >= 0) g.text("", 0, dominatingtex, TEAM(sg.team, colour));
                                        else g.strut(1);
                                    }
                                    else g.strut(1);
                                });
                            }));
                        });
                        uilist(g, {
                            uicenterlist(g, uipad(g, 0.125f, g.strut(1)));
                            loopscoregroup(uicenterlist(g, {
                                const char *status = questiontex;
                                switch(o->state)
                                {
                                    case CS_ALIVE: status = playertex; break;
                                    case CS_DEAD: status = deadtex; break;
                                    case CS_WAITING: status = waitingtex; break;
                                    case CS_EDITING: status = editingtex; break;
                                    default: break; // spectators shouldn't be here
                                }
                                uipad(g, 0.125f, g.text("", 0, status, TEAM(sg.team, colour)));
                            }));
                        });
                    }
                    if(sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
                    {
                        g.poplist(); // horizontal
                        g.poplist(); // vertical
                    }
                });
                if(k+1<numgroups && (k+1)%2) g.space(1);
                else g.poplist();
            }
        });
        if(scorespectators && spectators.length())
        {
            g.space(0.5f);
            uicenterlist(g, uicenterlist(g, uifont(g, "little", {
                int count = numgroups > 1 ? 5 : 3;
                bool pushed = false;
                loopv(spectators)
                {
                    gameent *o = spectators[i];
                    if((i%count)==0)
                    {
                        g.pushlist();
                        g.spring();
                        pushed = true;
                    }
                    uicenterlist(g, uicenterlist(g, uipad(g, 0.25f, {
                        if(o == game::player1 && scorehilight) g.background(scorehilight);
                        uipad(g, 0.25f, {
                            if(scoreclientnum || game::player1->privilege >= PRIV_ELEVATED)
                            {
                                if(g.buttonf("%s [%d]", 0xFFFFFF, NULL, 0, false, game::colourname(o, NULL, true, false), o->clientnum)&GUI_UP)
                                {
                                    scoretarget = o->clientnum;
                                    showgui("client");
                                }
                            }
                            else if(g.buttonf("%s ", 0xFFFFFF, NULL, 0, false, game::colourname(o))&GUI_UP)
                            {
                                scoretarget = o->clientnum;
                                showgui("client");
                            }
                        });
                    })));
                    if(!((i+1)%count) && pushed)
                    {
                        g.spring();
                        g.poplist();
                        pushed = false;
                    }
                }
                if(pushed) g.poplist();
            })));
        }
        if(m_play(game::gamemode) && game::player1->state != CS_SPECTATOR && (game::intermission || scoresinfo))
        {
            float ratio = game::player1->frags >= game::player1->deaths ? (game::player1->frags/float(max(game::player1->deaths, 1))) : -(game::player1->deaths/float(max(game::player1->frags, 1)));
            g.space(0.5f);
            uicenterlist(g, uifont(g, "reduced", {
                g.textf("\fs\fg%d\fS %s, \fs\fg%d\fS %s, \fs\fy%.1f\fS:\fs\fy%.1f\fS ratio, \fs\fg%d\fS damage", 0xFFFFFF, NULL, 0,
                    game::player1->frags, game::player1->frags != 1 ? "frags" : "frag",
                    game::player1->deaths, game::player1->deaths != 1 ? "deaths" : "death", ratio >= 0 ? ratio : 1.f, ratio >= 0 ? 1.f : -ratio,
                    game::player1->totaldamage);
            }));
        }
        g.end();
    }

    static const char *posnames[10] = { "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th" };

    int drawscoreitem(const char *icon, int colour, int x, int y, int s, float skew, float fade, int pos, int score, int offset, const char *name, const char *ext = NULL)
    {
        int col = 0xFF0000;
        switch(pos)
        {
            case 1: col = 0x00FF00; break;
            case 2: col = 0x00FFFF; break;
            case 3: col = 0xFFFF00; break;
            case 4: col = 0xFF8800; break;
            default: break;
        }
        int size = int(s*skew);
        string str, q;
        if(m_laptime(game::gamemode, game::mutators)) { formatstring(str)("\fs\f[%d]\f(%s)\fS %s", col, insigniatex, timestr(score)); }
        else if(m_defend(game::gamemode) && score == INT_MAX) { formatstring(str)("\fs\f[%d]\f(%s)\fS WIN", col, insigniatex); }
        else { formatstring(str)("\fs\f[%d]\f(%s)\fS %d", col, insigniatex, score); }
        if(inventoryscoreinfo)
        {
            if(m_laptime(game::gamemode, game::mutators))
                { formatstring(q)("\n\fs\f[%d]\f(%s)\fS %s", col, offset ? (offset < 0 ? arrowtex : arrowdowntex) : arrowrighttex, timestr(offset < 0 ? 0-offset : offset)); }
            else { formatstring(q)("%s\fs\f[%d]\f(%s)\fS %d", inventoryscorebreak ? "\n" : " ", col, offset ? (offset > 0 ? arrowtex : arrowdowntex) : arrowrighttex, offset < 0 ? 0-offset : offset); }
            concatstring(str, q);
        }
        vec c = vec::hexcolor(colour);
        drawitem(icon, x, y+size, s, 0, inventoryscorebg!=0, false, c.r, c.g, c.b, fade, skew, m_laptime(game::gamemode, game::mutators) ? "reduced" : "emphasis", "%s", str);
        int sy = 0;
        if(ext) sy += drawitemtextx(x, y+size, 0, TEXT_RIGHT_UP, skew, "default", fade, "%s", ext);
        drawitemtextx(x, y+size-sy, 0, TEXT_RIGHT_UP, skew, "default", fade, "\f[%d]%s", colour, name);
        if(inventoryscorepos) drawitemtextx(x, y, 0, TEXT_RIGHT_JUSTIFY, skew, "emphasis", fade, "\f[%d]%d%s", col, pos, posnames[pos < 10 || pos > 13 ? pos%10 : 0]);
        return size;
    }

    int drawscore(int x, int y, int s, int m, float blend, int count)
    {
        int sy = 0, numgroups = groupplayers(), numout = 0;
        loopi(2)
        {
            if(!i && game::focus->state == CS_SPECTATOR) continue;
            int pos = 0, realpos = 0, lastpos = -1;
            loopk(numgroups)
            {
                if(sy > m) break;
                scoregroup &sg = *groups[k];
                if(m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
                {
                    realpos++;
                    if(!pos || (m_laptime(game::gamemode, game::mutators) ? sg.total > groups[lastpos]->total : sg.total < groups[lastpos]->total))
                    {
                        pos = realpos;
                        lastpos = k;
                    }
                    if(!sg.team || ((sg.team != game::focus->team) == !i)) continue;
                    float sk = numout && inventoryscoreshrink > 0 ? 1.f-min(numout*inventoryscoreshrink, inventoryscoreshrinkmax) : 1;
                    int offset = numgroups > 1 ? sg.total-groups[k ? 0 : 1]->total : 0;
                    sy += drawscoreitem(teamtexname(sg.team), TEAM(sg.team, colour), x, y+sy, s, sk*inventoryscoresize, blend*inventoryblend, pos, sg.total, offset, TEAM(sg.team, name), i ? NULL : game::colourname(game::focus));
                    if(++numout >= count) return sy;
                }
                else
                {
                    if(sg.team) continue;
                    loopvj(sg.players)
                    {
                        gameent *d = sg.players[j];
                        realpos++;
                        if(!pos || (m_laptime(game::gamemode, game::mutators) ? d->cptime > sg.players[lastpos]->cptime : d->points < sg.players[lastpos]->points))
                        {
                            pos = realpos;
                            lastpos = j;
                        }
                        if((d != game::focus) == !i) continue;
                        float sk = numout && inventoryscoreshrink > 0 ? 1.f-min(numout*inventoryscoreshrink, inventoryscoreshrinkmax) : 1;
                        int score = m_laptime(game::gamemode, game::mutators) ? d->cptime : d->points,
                            offset = sg.players.length() > 1 ? score-(m_laptime(game::gamemode, game::mutators) ? sg.players[j ? 0 : 1]->cptime : sg.players[j ? 0 : 1]->points) : 0;
                        sy += drawscoreitem(playertex, game::getcolour(d, game::playerdisplaytone), x, y+sy, s, sk*inventoryscoresize, blend*inventoryblend, pos, score, offset, game::colourname(d));
                        if(++numout >= count) return sy;
                    }
                }
            }
        }
        return sy;
    }

    int trialinventory(int x, int y, int s, float blend)
    {
        int sy = 0;
        if(groupplayers())
        {
            scoregroup &sg = *groups[0];
            if(m_laptime(game::gamemode, game::mutators))
            {
                if(m_team(game::gamemode, game::mutators))
                {
                    if(sg.total)
                    {
                        pushfont("little");
                        sy += draw_textx("by %s", x+FONTW*2, y, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, game::colourteam(sg.team));
                        popfont();
                        sy += draw_textx("\fg%s", x, y-sy, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, timestr(sg.total));
                    }
                }
                else if(!sg.players.empty())
                {
                    if(sg.players[0]->cptime)
                    {
                        pushfont("little");
                        sy += draw_textx("by %s", x+FONTW*2, y, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, game::colourname(sg.players[0]));
                        popfont();
                        sy += draw_textx("\fg%s", x, y-sy, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, timestr(sg.players[0]->cptime));
                    }
                }
            }
            else if(m_team(game::gamemode, game::mutators))
            {
                if(sg.total)
                {
                    pushfont("little");
                    sy += draw_textx("by %s", x+FONTW*2, y, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, game::colourteam(sg.team));
                    popfont();
                    sy += draw_textx("\fg%d", x, y-sy, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, sg.total);
                }
            }
        }
        return sy;
    }

    struct scoreboard : guicb
    {
        void gui(guient &g, bool firstpass)
        {
            renderscoreboard(g, firstpass);
        }
    } sb;

    ICOMMAND(0, showscores, "D", (int *down), showscores(*down!=0, false, false, true));

    void gamemenus()
    {
        if(scoreson) UI::addcb(&sb);
        if(game::player1->state == CS_DEAD) { if(scoreson) shownscores = true; }
        else shownscores = false;
    }
}
