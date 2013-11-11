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
    VAR(IDF_PERSIST, scoreimage, 0, 0, 1);
    FVAR(IDF_PERSIST, scoreimagesize, FVAR_NONZERO, 6, 10);
    VAR(IDF_PERSIST, scorebgfx, 0, 1, 1);
    VAR(IDF_PERSIST, scorebgrows, 0, 3, 3);

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
            if(!o || o->actortype >= A_ENEMY || (!scoreconnecting && !o->name[0])) continue;
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
        g.start(menustart, menuscale, NULL, false, false, scorebgfx!=0);
        int numgroups = groupplayers();
        uilist(g, {
            uicenterlist(g, {
                g.strut(20);
                uicenter(g, {
                    uicenterlist(g, uicenterlist(g, {
                        uicenterlist(g, uifont(g, "emphasis", g.textf("%s", 0xFFFFFF, NULL, 0, *maptitle ? maptitle : mapname)));
                        if(*mapauthor)
                        {
                            int len = strlen(mapauthor);
                            uicenterlist(g, uifont(g, (len >= 48 ? (len >= 56 ? "tiny" : "little") : "reduced"), g.textf("by %s", 0xFFFFFF, NULL, 0, mapauthor)));
                        }
                        uicenterlist(g, uifont(g, "little", {
                            g.textf("\fy%s", 0xFFFFFF, NULL, 0, server::gamename(game::gamemode, game::mutators, 0, 32));
                            if(game::intermission) g.text(", \fs\fointermission\fS", 0xFFFFFF);
                            else if(client::waitplayers) g.text(", \fs\fowaiting\fS", 0xFFFFFF);
                            else if(paused) g.text(", \fs\fopaused\fS", 0xFFFFFF);
                            if((m_play(game::gamemode) || client::demoplayback) && game::timeremaining >= 0)
                                g.textf(", \fs\fg%s\fS remain", 0xFFFFFF, NULL, 0, timestr(game::timeremaining, 2));
                        }));
                        if(*connectname)
                        {
                            uicenterlist(g, uifont(g, "little", {
                                g.textf("\faon: \fw%s:[%d]", 0xFFFFFF, NULL, 0, connectname, connectport);
                            }));
                            if(*serverdesc)
                            {
                                uicenterlist(g, uifont(g, (strlen(serverdesc) >= 32 ? "tiny" : "little"), {
                                    g.textf("\"\fs%s\fS\"", 0xFFFFFF, NULL, 0, serverdesc);
                                }));
                            }
                        }
                    }));
                    if(scoreimage)
                    {
                        g.space(0.25f);
                        uicenterlist(g, {
                            g.image(NULL, scoreimagesize, true);
                        });
                    }
                    g.space(0.25f);
                    uicenterlist(g, uicenterlist(g, {
                        if(game::player1->quarantine)
                        {
                            uicenterlist(g, uifont(g, "default", g.text("You are \fzoyQUARANTINED", 0xFFFFFF)));
                            uicenterlist(g, uifont(g, "reduced", g.text("Please await instructions from a moderator", 0xFFFFFF)));
                        }
                        if(client::demoplayback)
                        {
                            uicenterlist(g, {
                                uifont(g, "default", g.text("Demo Playback in Progress", 0xFFFFFF));
                            });
                        }
                        else if(client::waitplayers)
                        {
                            uicenterlist(g, {
                                uifont(g, "default", g.text("Waiting for players", 0xFFFFFF));
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
                                    uicenterlist(g, uifont(g, "default", {
                                        if(client::waitplayers || m_duke(game::gamemode, game::mutators)) g.text("Queued for new round", 0xFFFFFF);
                                        else if(delay) g.textf("%s: Down for \fs\fy%s\fS", 0xFFFFFF, NULL, 0, game::player1->state == CS_WAITING ? "Please Wait" : "Fragged", timestr(delay, -1));
                                        else if(game::player1->state == CS_WAITING && m_fight(game::gamemode) && maxalive > 0 && maxalivequeue)
                                        {
                                            int n = game::numwaiting();
                                            if(n) g.textf("Waiting for \fs\fy%d\fS %s", 0xFFFFFF, NULL, 0, n, n != 1 ? "players" : "player");
                                            else g.text("You are \fs\fgnext\fS in the queue", 0xFFFFFF);
                                        }
                                    }));
                                    if(game::player1->state != CS_WAITING && lastmillis-game::player1->lastdeath >= 500)
                                        uicenterlist(g, uifont(g, "little", g.textf("Press %s to enter respawn queue", 0xFFFFFF, NULL, 0, attackkey)));
                                }
                                else
                                {
                                    uicenterlist(g, uifont(g, "default", g.text("Ready to respawn", 0xFFFFFF)));
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
                                uifont(g, "default", uicenterlist(g, {
                                    if(m_team(game::gamemode, game::mutators))
                                        g.textf("Playing for team %s", 0xFFFFFF, NULL, 0, game::colourteam(game::player1->team));
                                    else g.text("Playing free-for-all", 0xFFFFFF);
                                }));
                            }
                            else if(game::player1->state == CS_SPECTATOR)
                            {
                                uifont(g, "default", uicenterlist(g, {
                                    g.textf("You are %s", 0xFFFFFF, NULL, 0, game::tvmode() ? "watching SpecTV" : "a spectator");
                                }));
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

                            if(m_edit(game::gamemode) && (game::player1->state != CS_EDITING || shownotices >= 4))
                            {
                                SEARCHBINDCACHE(editkey)("edittoggle", 1);
                                uicenterlist(g, uifont(g, "reduced", g.textf("Press %s to %s editmode", 0xFFFFFF, NULL, 0, editkey, game::player1->state != CS_EDITING ? "enter" : "exit")));
                            }
                        }

                        SEARCHBINDCACHE(scoreboardkey)("showscores", 1);
                        uicenterlist(g, uifont(g, "little", g.textf("%s %s to close this window", 0xFFFFFF, NULL, 0, scoresoff ? "Release" : "Press", scoreboardkey)));
                        uicenterlist(g, uifont(g, "tiny", g.text("Double-tap to keep the window open", 0xFFFFFF)));

                        if(m_play(game::gamemode) && game::player1->state != CS_SPECTATOR && (game::intermission || scoresinfo))
                        {
                            float ratio = game::player1->frags >= game::player1->deaths ? (game::player1->frags/float(max(game::player1->deaths, 1))) : -(game::player1->deaths/float(max(game::player1->frags, 1)));
                            uicenterlist(g, uifont(g, "little", {
                                g.textf("\fs\fg%d\fS %s, \fs\fg%d\fS %s (\fs\fy%.1f\fS:\fs\fy%.1f\fS) \fs\fg%d\fS damage", 0xFFFFFF, NULL, 0,
                                    game::player1->frags, game::player1->frags != 1 ? "frags" : "frag",
                                    game::player1->deaths, game::player1->deaths != 1 ? "deaths" : "death", ratio >= 0 ? ratio : 1.f, ratio >= 0 ? 1.f : -ratio,
                                    game::player1->totaldamage);
                            }));
                        }
                    }));
                });
            });
            g.space(4);
            uicenterlist(g, {
                g.strut(30);
                uicenter(g, {
                    #define loopscorelist(b) \
                    { \
                        int _n = sg.players.length(); \
                        loopi(_n) if(sg.players[i]) \
                        { \
                            b; \
                        } \
                    }
                    #define loopscoregroup(b) \
                    { \
                        loopv(sg.players) if(sg.players[i]) \
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
                                namepad = max(namepad, (float)text_width(game::colourname(o, NULL, false, true))/guibound[0]*0.51f);
                                if(scorehandles && o->handle[0])
                                {
                                    handlepad = max(handlepad, (float)text_width(o->handle)/guibound[0]*0.51f);
                                    hashandle = true;
                                }
                                if(scorehostinfo)
                                {
                                    const char *host = scorehost(o);
                                    if(host && *host)
                                    {
                                        hostpad = max(hostpad, (float)text_width(host)/guibound[0]*0.51f);
                                        if(o->ownernum != game::player1->clientnum) hashost = true;
                                    }
                                }
                            });
                        }
                        //namepad = max((namepad-text_width("name"))/(guibound[0]*2.f), 0.25f)*1.5f;
                        //if(hashandle) handlepad = max((handlepad-guibound[0])/(guibound[0]*2.f), 0.25f)*1.5f;
                        //if(hashost) hostpad = max((hostpad-guibound[0])/(guibound[0]*2.f), 0.25f)*1.5f;
                        loopk(numgroups)
                        {
                            scoregroup &sg = *groups[k];
                            if(k) g.space(0.5f);
                            vec c = vec::hexcolor(sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators) ? TEAM(sg.team, colour) : TEAM(T_NEUTRAL, colour));
                            int bgcolor = vec(c).mul(0.65f).tohexcolor();
                            int bgc1 = vec(c).mul(0.45f).tohexcolor();
                            int bgc2 = vec(c).mul(0.25f).tohexcolor();
                            uicenterlist(g, {
                                g.pushlist();
                                if(scorebgrows) loopj(4) g.border(bgcolor, 0, 0, -(j+1), -(j+1));
                                if(sg.team && m_team(game::gamemode, game::mutators))
                                {
                                    uilist(g, uifont(g, "default", {
                                        if(scorebgrows) g.background(bgc2);
                                        g.space(0.15f);
                                        g.textf("team %s", 0xFFFFFF, teamtexname(sg.team), TEAM(sg.team, colour), TEAM(sg.team, name));
                                        g.spring();
                                        if(m_defend(game::gamemode) && ((defendlimit && sg.total >= defendlimit) || sg.total == INT_MAX)) g.text("WINNER", 0xFFFFFF);
                                        else if(m_laptime(game::gamemode, game::mutators)) g.textf("best: %s", 0xFFFFFF, NULL, 0, sg.total ? timestr(sg.total) : "\fadnf");
                                        else g.textf("%d points", 0xFFFFFF, NULL, 0, sg.total);
                                        g.space(0.25f);
                                    }));
                                }
                                g.pushlist();

                                uilist(g, {
                                    uilist(g, {
                                        if(scorebgrows > 1) g.background(bgcolor);
                                        uicenter(g, uipad(g, 0.25f, g.space(1); g.strut(1)));
                                    });
                                    loopscorelist(uilist(g, {
                                        if(scorebgrows > 1) g.background(bgcolor);
                                        uicenter(g, uipad(g, 0.25f, g.space(1); g.strut(1)));
                                    }));
                                });

                                uilist(g, {
                                    uilist(g, {
                                        if(scorebgrows > 1) g.background(bgcolor);
                                        uicenter(g, uipad(g, 0.25f, g.space(1); g.strut(1)));
                                    });
                                    loopscoregroup(uilist(g, {
                                        if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                        uicenter(g, uipad(g, 0.25f, uicenterlist(g, g.textf("\f($priv%stex)", game::findcolour(o), NULL, 0, hud::privname(o->privilege, o->actortype)))));
                                    }));
                                });

                                uilist(g, {
                                    uilist(g, {
                                        if(scorebgrows > 1) g.background(bgcolor);
                                        uicenter(g, uipad(g, namepad, uicenterlist(g, g.text("name", 0xFFFFFF))));
                                    });
                                    loopscoregroup(uilist(g, {
                                        if(scorehilight && o == game::player1) loopj(3) g.border(scorehilight, 0, 0, j+1, j+1);
                                        if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                        uicenter(g, uipad(g, 0.25f, uicenterlist(g, g.textf("%s", 0xFFFFFF, NULL, 0, game::colourname(o, NULL, false, true, scorebgrows > 2 || (scorehilight && o == game::player1) ? 0 : 3)))));
                                    }));
                                });

                                if(scorepoints)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 1, g.text("points", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                                if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->points)));
                                        }));
                                    });
                                }

                                if(m_trial(game::gamemode) || m_gauntlet(game::gamemode))
                                {
                                    if(scoretimer && (scoretimer >= 2 || m_laptime(game::gamemode, game::mutators)))
                                    {
                                        uilist(g, {
                                            uilist(g, {
                                                if(scorebgrows > 1) g.background(bgcolor);
                                                uicenter(g, uipad(g, 4, g.text("best", 0xFFFFFF)));
                                            });
                                            loopscoregroup(uilist(g, {
                                                if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                                uicenter(g, uipad(g, 0.5f, g.textf("%s", 0xFFFFFF, NULL, 0, o->cptime ? timestr(o->cptime) : "\fadnf")));
                                            }));
                                        });
                                    }
                                    if(scorelaps && (scorelaps >= 2 || !m_laptime(game::gamemode, game::mutators)))
                                    {
                                        uilist(g, {
                                            uilist(g, {
                                                if(scorebgrows > 1) g.background(bgcolor);
                                                uicenter(g, uipad(g, 4, g.text("laps", 0xFFFFFF)));
                                            });
                                            loopscoregroup(uilist(g, {
                                                if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                                uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->cplaps)));
                                            }));
                                        });
                                    }
                                }

                                if(scorefrags && (scorefrags >= 2 || m_dm(game::gamemode)))
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 1, g.text("frags", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->frags)));
                                        }));
                                    });
                                }

                                if(scoredeaths && (scoredeaths >= 2 || m_dm(game::gamemode)))
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 1, g.text("deaths", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->deaths)));
                                        }));
                                    });
                                }

                                if(scoreratios && (scoreratios >= 2 || m_dm(game::gamemode)))
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 1, g.text("ratio", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            float ratio = o->frags >= o->deaths ? (o->frags/float(max(o->deaths, 1))) : -(o->deaths/float(max(o->frags, 1)));
                                            uicenter(g, uipad(g, 0.5f, g.textf("%.1f\fs\fa:\fS%.1f", 0xFFFFFF, NULL, 0, ratio >= 0 ? ratio : 1.f, ratio >= 0 ? 1.f : -ratio)));
                                        }));
                                    });
                                }

                                if(scorepj)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 2, g.text("pj", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->plag)));
                                        }));
                                    });
                                }

                                if(scoreping)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 2, g.text("ping", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->ping)));
                                        }));
                                    });
                                }

                                if(scoreclientnum || game::player1->privilege >= PRIV_ELEVATED)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 1, g.text("cn", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%d", 0xFFFFFF, NULL, 0, o->clientnum)));
                                        }));
                                    });
                                }

                                if(scorebotinfo && hasbots)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 1, g.text("sk", 0xFFFFFF)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, {
                                                if(o->actortype > A_PLAYER) g.textf("%d", 0xFFFFFF, NULL, 0, o->skill);
                                                else { g.space(1); g.strut(1); }
                                            }));
                                        }));
                                    });
                                }
                                if(scorehandles && hashandle)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, handlepad, g.space(1); g.strut(1)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%s", 0xFFFFFF, NULL, 0, o->handle[0] ? o->handle : "-")));
                                        }));
                                    });
                                }
                                if(scorehostinfo && hashost)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                           if(scorebgrows > 1) g.background(bgcolor);
                                           uicenter(g, uipad(g, hostpad, g.space(1); g.strut(1)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            uicenter(g, uipad(g, 0.5f, g.textf("%s", 0xFFFFFF, NULL, 0, scorehost(o))));
                                        }));
                                    });
                                }
                                if(scoreicons)
                                {
                                    uilist(g, {
                                        uilist(g, {
                                            if(scorebgrows > 1) g.background(bgcolor);
                                            uicenter(g, uipad(g, 0.125f, g.space(1); g.strut(1)));
                                        });
                                        loopscoregroup(uilist(g, {
                                            if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                            const char *status = questiontex;
                                            if(game::player1->dominating.find(o) >= 0) status = dominatedtex;
                                            else if(game::player1->dominated.find(o) >= 0) status = dominatingtex;
                                            else switch(o->state)
                                            {
                                                case CS_ALIVE: status = playertex; break;
                                                case CS_DEAD: status = deadtex; break;
                                                case CS_WAITING: status = waitingtex; break;
                                                case CS_EDITING: status = editingtex; break;
                                                default: break; // spectators shouldn't be here
                                            }
                                            uicenter(g, uipad(g, 0.125f, g.textf("\f(%s)", TEAM(sg.team, colour), NULL, 0, status)));
                                        }));
                                    });
                                }
                                uilist(g, {
                                    uilist(g, {
                                        if(scorebgrows > 1) g.background(bgcolor);
                                        uicenter(g, uipad(g, 0.25f, g.space(1); g.strut(1)));
                                    });
                                    loopscorelist(uilist(g, {
                                        if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                        uicenter(g, uipad(g, 0.25f, g.space(1); g.strut(1)));
                                    }));
                                });
                                g.poplist(); // horizontal
                                g.poplist(); // vertical
                            });
                        }
                    });
                    if(scorespectators && spectators.length())
                    {
                        vec c = vec::hexcolor(TEAM(T_NEUTRAL, colour));
                        int bgc1 = vec(c).mul(0.45f).tohexcolor();
                        int bgc2 = vec(c).mul(0.25f).tohexcolor();
                        g.space(0.5f);
                        uifont(g, "little", uicenterlist(g, {
                            uicenterlist(g, {
                                uicenterlist(g, g.text("spectators", 0xFFFFFF));
                                g.space(0.25f);
                                bool pushed = false;
                                loopv(spectators)
                                {
                                    if(!(i%4))
                                    {
                                        if(pushed) g.poplist();
                                        g.pushlist();
                                        pushed = true;
                                    }
                                    gameent *o = spectators[i];
                                    uilist(g, {
                                        loopj(3) g.border(scorehilight && o == game::player1 ? scorehilight : 0x000000, 0, 0, -(j+1), -(j+1));
                                        if(scorebgrows > 2) g.background(i%2 ? bgc2 : bgc1);
                                        uipad(g, 0.025f, uilist(g, uipad(g, 0.25f, uicenterlist(g, {
                                            if(scoreclientnum || game::player1->privilege >= PRIV_ELEVATED)
                                                g.textf("%s [%d]", 0xFFFFFF, NULL, 0, game::colourname(o, NULL, true, false), o->clientnum);
                                            else g.textf("%s ", 0xFFFFFF, NULL, 0, game::colourname(o));
                                        }))));
                                    });
                                }
                                if(pushed) g.poplist();
                            });
                        }));
                    }
                });
            });
        });
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
                        sy += drawscoreitem(playertex, game::getcolour(d, game::playerteamtone), x, y+sy, s, sk*inventoryscoresize, blend*inventoryblend, pos, score, offset, game::colourname(d));
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
