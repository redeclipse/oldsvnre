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

    struct sline { string s; };
    struct scoregroup : score
    {
        vector<gameent *> players;
    };
    vector<scoregroup *> groups;
    vector<gameent *> spectators;

    VAR(IDF_PERSIST, autoshowscores, 0, 2, 3); // 1 = when dead, 2 = also in spectv, 3 = and in waittv too
    VAR(IDF_PERSIST, showscoresdelay, 0, 0, VAR_MAX); // otherwise use a static timespan
    VAR(IDF_PERSIST, showscoresinfo, 0, 1, 1);
    VAR(IDF_PERSIST, highlightscore, 0, 1, 1);

    VAR(IDF_PERSIST, showpj, 0, 0, 1);
    VAR(IDF_PERSIST, showping, 0, 1, 1);
    VAR(IDF_PERSIST, showpoints, 0, 1, 1);
    VAR(IDF_PERSIST, showscore, 0, 2, 2);
    VAR(IDF_PERSIST, showclientnum, 0, 1, 1);
    VAR(IDF_PERSIST, showskills, 0, 1, 1);
    VAR(IDF_PERSIST, showownernum, 0, 0, 1);
    VAR(IDF_PERSIST, showspectators, 0, 1, 1);
    VAR(IDF_PERSIST, showconnecting, 0, 0, 1);

    static bool scoreson = false, scoresoff = false, shownscores = false;
    static int menustart = 0, menulastpress = 0;

    bool canshowscores()
    {
        if(!scoresoff && !scoreson && !shownscores && autoshowscores && (game::player1->state == CS_DEAD || (autoshowscores >= (game::player1->state == CS_SPECTATOR ? 2 : 3) && game::tvmode())))
        {
            if(game::tvmode()) return true;
            else if(game::player1->state == CS_DEAD)
            {
                int delay = !showscoresdelay ? m_delay(game::gamemode, game::mutators) : showscoresdelay;
                if(!delay || lastmillis-game::player1->lastdeath > delay) return true;
            }
        }
        return false;
    }

    static inline bool playersort(const gameent *a, const gameent *b)
    {
        if(a->state==CS_SPECTATOR)
        {
            if(b->state==CS_SPECTATOR) return strcmp(a->name, b->name) < 0;
            else return false;
        }
        else if(b->state==CS_SPECTATOR) return true;
        if(m_trial(game::gamemode))
        {
            if((a->cptime && !b->cptime) || (a->cptime && b->cptime && a->cptime < b->cptime)) return true;
            if((b->cptime && !a->cptime) || (a->cptime && b->cptime && b->cptime < a->cptime)) return false;
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
        if(x->total > y->total) return true;
        if(x->total < y->total) return false;
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
            if(!o || o->type!=ENT_PLAYER || (!showconnecting && !o->name[0])) continue;
            if(o->state==CS_SPECTATOR) { spectators.add(o); continue; }
            int team = m_fight(game::gamemode) && m_team(game::gamemode, game::mutators) ? o->team : TEAM_NEUTRAL;
            bool found = false;
            loopj(numgroups)
            {
                scoregroup &g = *groups[j];
                if(team != g.team) continue;
                if(team) g.total = teamscore(team).total;
                g.players.add(o);
                found = true;
            }
            if(found) continue;
            if(numgroups>=groups.length()) groups.add(new scoregroup);
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
            if(m_play(game::gamemode) && interm)
            {
                if(m_campaign(game::gamemode)) game::announcef(S_V_MCOMPLETE, CON_MESG, game::player1, "\fwmission completed");
                else if(m_fight(game::gamemode) && !m_trial(game::gamemode))
                {
                    int numgroups = groupplayers();
                    if(!numgroups) return;
                    scoregroup &sg = *groups[0];
                    if(m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
                    {
                        int anc = sg.players.find(game::player1) >= 0 ? S_V_YOUWIN : (game::player1->state != CS_SPECTATOR ? S_V_YOULOSE : -1);
                        if(m_defend(game::gamemode) && sg.total == INT_MAX)
                            game::announcef(anc, CON_MESG, game::player1, "\fw\fs\f[%d]%s\fS team secured all flags", TEAM(sg.team, colour), TEAM(sg.team, name));
                        else
                        {
                            if(numgroups > 1 && sg.total == groups[1]->total)
                            {
                                mkstring(winner);
                                loopi(numgroups) if(i)
                                {
                                    if(sg.total == groups[i]->total)
                                    {
                                        defformatstring(tw)("\fw\fs\f[%d]%s\fS, ", TEAM(groups[i]->team, colour), TEAM(groups[i]->team, name));
                                        concatstring(winner, tw);
                                    }
                                    else break;
                                }
                                game::announcef(S_V_DRAW, CON_MESG, game::player1, "\fw\fs\f[%d]%s\fS tied %s with a total score of: \fs\fc%d\fS", TEAM(sg.team, colour), TEAM(sg.team, name), winner, sg.total);
                            }
                            else game::announcef(anc, CON_MESG, game::player1, "\fw\fs\f[%d]%s\fS team won the match with a total score of: \fs\fc%d\fS", TEAM(sg.team, colour), TEAM(sg.team, name), sg.total);
                        }
                    }
                    else
                    {
                        int anc = sg.players[0] == game::player1 ? S_V_YOUWIN : (game::player1->state != CS_SPECTATOR ? S_V_YOULOSE : -1);
                        if(m_trial(game::gamemode))
                        {
                            if(sg.players.length() > 1 && sg.players[0]->cptime == sg.players[1]->cptime)
                            {
                                mkstring(winner);
                                loopv(sg.players) if(i)
                                {
                                    if(sg.players[0]->cptime == sg.players[i]->cptime)
                                    {
                                        concatstring(winner, game::colorname(sg.players[i]));
                                        concatstring(winner, ", ");
                                    }
                                    else break;
                                }
                                game::announcef(S_V_DRAW, CON_MESG, game::player1, "\fw%s tied %s with the fastest lap: \fs\fc%s\fS", game::colorname(sg.players[0]), winner, sg.players[0]->cptime ? timetostr(sg.players[0]->cptime) : "dnf");
                            }
                            else game::announcef(anc, CON_MESG, game::player1, "\fw%s won the match with the fastest lap: \fs\fc%s\fS", game::colorname(sg.players[0]), sg.players[0]->cptime ? timetostr(sg.players[0]->cptime) : "dnf");
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
                                        concatstring(winner, game::colorname(sg.players[i]));
                                        concatstring(winner, ", ");
                                    }
                                    else break;
                                }
                                game::announcef(S_V_DRAW, CON_MESG, game::player1, "\fw%s tied %s with a total score of: \fs\fc%d\fS", game::colorname(sg.players[0]), winner, sg.players[0]->points);
                            }
                            else game::announcef(anc, CON_MESG, game::player1, "\fw%s won the match with a total score of: \fs\fc%d\fS", game::colorname(sg.players[0]), sg.players[0]->points);
                        }
                    }
                }
            }
        }
        else scoresoff = scoreson = false;
    }

    void renderscoreboard(guient &g, bool firstpass)
    {
        g.start(menustart, menuscale, NULL, false);
        int numgroups = groupplayers();
        g.pushlist();
        g.image(NULL, 6, true);
        g.space(2);
        g.pushlist();
        g.space(1);
        g.pushfont("default");
        if(*maptitle) g.textf("%s", 0xFFFFFF, NULL, 0, maptitle);
        else g.textf("(%s)", 0xFFFFFF, NULL, 0, mapname);
        g.popfont();
        if(*mapauthor)
        {
            g.pushlist();
            g.space(3);
            g.pushfont("reduced");
            g.textf("by %s", 0xFFFFFF, NULL, 0, mapauthor);
            g.popfont();
            g.poplist();
        }
        g.pushlist();
        g.pushfont("reduced");
        defformatstring(gname)("%s", server::gamename(game::gamemode, game::mutators));
        if(strlen(gname) > 32) formatstring(gname)("%s", server::gamename(game::gamemode, game::mutators, 1));
        g.textf("%s", 0xFFFFFF, NULL, 0, gname);
        if((m_play(game::gamemode) || client::demoplayback) && game::timeremaining >= 0)
        {
            if(!game::timeremaining) g.textf(", \fs\fyintermission\fS", 0xFFFFFF, NULL, 0);
            else if(paused) g.textf(", \fs\fopaused\fS", 0xFFFFFF, NULL, 0);
            else g.textf(", \fs\fg%s\fS remain", 0xFFFFFF, NULL, 0, hud::timetostr(game::timeremaining, 2));
        }
        g.popfont();
        g.poplist();
        if(*connectname)
        {
            g.pushlist();
            g.space(2);
            g.pushfont("little");
            g.textf("\fdon ", 0xFFFFFF, NULL, 0);
            g.popfont();
            if(*serverdesc)
            {
                g.pushfont("reduced");
                g.textf("%s ", 0xFFFFFF, NULL, 0, serverdesc);
                g.popfont();
            }
            g.pushfont("little");
            g.textf("\fd(\fa%s:[%d]\fd)", 0xFFFFFF, NULL, 0, connectname, connectport);
            g.popfont();
            g.poplist();
        }

        if(game::player1->state == CS_DEAD || game::player1->state == CS_WAITING)
        {
            int sdelay = m_delay(game::gamemode, game::mutators), delay = game::player1->lastdeath ? game::player1->respawnwait(lastmillis, sdelay) : 0;
            const char *msg = game::player1->state != CS_WAITING && game::player1->lastdeath ? "Fragged" : "Please Wait";
            g.space(1);
            g.pushlist();
            g.pushfont("reduced"); g.textf("%s", 0xFFFFFF, NULL, 0, msg); g.popfont();
            g.space(2);
            SEARCHBINDCACHE(attackkey)("action 0", 0);
            g.pushfont("little");
            if(delay || m_campaign(game::gamemode) || (m_trial(game::gamemode) && !game::player1->lastdeath) || m_duke(game::gamemode, game::mutators) || (m_fight(game::gamemode) && maxalive > 0))
            {
                if(m_duke(game::gamemode, game::mutators)) g.textf("Queued for new round", 0xFFFFFF, NULL, 0);
                else if(delay) g.textf("Down for \fs\fy%s\fS", 0xFFFFFF, NULL, 0, hud::timetostr(delay, -1));
                else if(game::player1->state == CS_WAITING && m_fight(game::gamemode) && maxalive > 0 && maxalivequeue)
                {
                    int n = game::numwaiting();
                    if(n) g.textf("Waiting for \fs\fy%d\fS %s", 0xFFFFFF, NULL, 0, n, n != 1 ? "players" : "player");
                    else g.textf("You are \fs\fgnext\fS in the queue", 0xFFFFFF, NULL, 0);
                }
                g.poplist();
                if(game::player1->state != CS_WAITING && lastmillis-game::player1->lastdeath > 500)
                    g.textf("Press \fs\fc%s\fS to enter respawn queue", 0xFFFFFF, NULL, 0, attackkey);
            }
            else
            {
                g.textf("Ready to respawn", 0xFFFFFF, NULL, 0);
                g.poplist();
                if(game::player1->state != CS_WAITING) g.textf("Press \fs\fc%s\fS to respawn now", 0xFFFFFF, NULL, 0, attackkey);
            }
            if(game::player1->state == CS_WAITING && lastmillis-game::player1->lastdeath >= 500)
            {
                SEARCHBINDCACHE(waitmodekey)("waitmodeswitch", 3);
                g.textf("Press \fs\fc%s\fS to enter respawn queue", 0xFFFFFF, NULL, 0, waitmodekey);
            }
            if(m_arena(game::gamemode, game::mutators))
            {
                SEARCHBINDCACHE(loadkey)("showgui loadout", 0);
                g.textf("Press \fs\fc%s\fS to \fs%s\fS loadout", 0xFFFFFF, NULL, 0, loadkey, game::player1->loadweap[0] < 0 ? "\fzoyselect" : "change");
            }
            if(m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
            {
                SEARCHBINDCACHE(teamkey)("showgui team", 0);
                g.textf("Press \fs\fc%s\fS to change teams", 0xFFFFFF, NULL, 0, teamkey);
            }
            g.popfont();
        }
        else if(game::player1->state == CS_ALIVE)
        {
            g.space(1);
            g.pushfont("reduced");
            if(m_edit(game::gamemode)) g.textf("Map Editing", 0xFFFFFF, NULL, 0);
            else if(m_campaign(game::gamemode)) g.textf("Campaign", 0xFFFFFF, NULL, 0);
            else if(m_team(game::gamemode, game::mutators))
                g.textf("Play for team \fs\f[%d]\f(%s)%s\fS", 0xFFFFFF, NULL, 0, TEAM(game::player1->team, colour), hud::teamtex(game::player1->team), TEAM(game::player1->team, name));
            else g.textf("Free for All Deathmatch", 0xFFFFFF, NULL, 0);
            g.popfont();
        }
        else if(game::player1->state == CS_SPECTATOR)
        {
            g.space(1);
            g.pushfont("reduced"); g.textf("%s", 0xFFFFFF, NULL, 0, game::tvmode() ? "SpecTV" : "Spectating"); g.popfont();
            SEARCHBINDCACHE(speconkey)("spectator 0", 1);
            g.pushfont("little");
            g.textf("Press \fs\fc%s\fS to join the game", 0xFFFFFF, NULL, 0, speconkey);
            SEARCHBINDCACHE(specmodekey)("specmodeswitch", 1);
            g.textf("Press \fs\fc%s\fS to %s", 0xFFFFFF, NULL, 0, specmodekey, game::tvmode() ? "interact" : "switch to TV");
            g.popfont();
        }

        SEARCHBINDCACHE(scoreboardkey)("showscores", 1);
        g.pushfont("little");
        g.textf("%s \fs\fc%s\fS to close this window", 0xFFFFFF, NULL, 0, scoresoff ? "Release" : "Press", scoreboardkey);
        g.pushlist();
        g.space(2);
        g.textf("Double-tap to keep the window open", 0xFFFFFF, NULL, 0);
        g.popfont();
        g.poplist();
        g.poplist();
        g.poplist();
        g.space(1);
        g.pushfont(numgroups>1 ? "little" : "default");
        loopk(numgroups)
        {
            if((k%2)==0) g.pushlist(); // horizontal

            scoregroup &sg = *groups[k];
            int bgcolor = sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators) ? TEAM(sg.team, colour) : 0x333333,
                fgcolor = 0xFFFFFF;

            g.pushlist(); // vertical
            g.pushlist(); // horizontal

            #define loopscoregroup(b) \
                loopv(sg.players) \
                { \
                    gameent *o = sg.players[i]; \
                    b; \
                }

            g.pushlist();
            if(sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
            {
                g.pushlist();
                g.background(bgcolor, numgroups>1 ? 3 : 5);
                g.text(" ", 0, hud::teamtex(sg.team), TEAM(sg.team, colour));
                g.poplist();
            }
            g.pushlist();
            g.strut(1);
            g.poplist();
            loopscoregroup({
                const char *status = hud::playertex;
                if(o->state == CS_DEAD || o->state == CS_WAITING) status = hud::deadtex;
                else if(o->state == CS_ALIVE)
                {
                    if(o->dominating.find(game::focus) >= 0) status = hud::dominatingtex;
                    else if(o->dominated.find(game::focus) >= 0) status = hud::dominatedtex;
                }
                int bgcol = o==game::player1 && highlightscore ? 0x999999 : 0;
                if(o->privilege) bgcol |= o->privilege >= PRIV_ADMIN ? 0x339933 : 0x999933;
                g.pushlist();
                if(bgcol) g.background(bgcol, 3);
                g.pushlist();
                g.background(game::getcolour(o));
                g.text("", 0, status, game::getcolour(o, CTONE_TONE));
                g.poplist();
                g.poplist();
            });
            g.poplist();

            if(sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
            {
                g.pushlist(); // vertical
                if(m_defend(game::gamemode) && ((defendlimit && sg.total >= defendlimit) || sg.total == INT_MAX))
                    g.textf("%s: WIN", fgcolor, NULL, 0, TEAM(sg.team, name));
                else g.textf("%s: %d", fgcolor, NULL, 0, TEAM(sg.team, name), sg.total);
                g.pushlist(); // horizontal
            }

            g.pushlist();
            g.pushlist();
            g.text(" name ", fgcolor);
            g.poplist();
            loopscoregroup(g.pushlist(); g.textf(" %s ", 0xFFFFFF, NULL, 0, game::colorname(o, NULL, "", false)); g.poplist());
            g.poplist();

            if(showpoints)
            {
                g.pushlist();
                g.strut(6);
                g.text("points ", fgcolor);
                loopscoregroup(g.textf("%d", 0xFFFFFF, NULL, 0, o->points));
                g.poplist();
            }

            if(showscore && (showscore >= 2 || !m_affinity(game::gamemode)))
            {
                g.pushlist();
                if(m_trial(game::gamemode))
                {
                    g.strut(10);
                    g.text("best lap", fgcolor);
                    loopscoregroup(g.textf("%s", 0xFFFFFF, NULL, 0, o->cptime ? timetostr(o->cptime) : "\fadnf"));
                }
                else
                {
                    g.strut(5);
                    g.text("frags ", fgcolor);
                    loopscoregroup(g.textf("%d", 0xFFFFFF, NULL, 0, o->frags));
                }
                g.poplist();
            }

            if(showpj)
            {
                g.pushlist();
                g.strut(4);
                g.text("pj ", fgcolor);
                loopscoregroup({
                    g.textf("%d", 0xFFFFFF, NULL, 0, o->plag);
                });
                g.poplist();
            }

            if(showping)
            {
                g.pushlist();
                g.strut(4);
                g.text("ping ", fgcolor);
                loopscoregroup(g.textf("%d", 0xFFFFFF, NULL, 0, o->ping));
                g.poplist();
            }

            if(showclientnum || game::player1->privilege>=PRIV_MASTER)
            {
                g.pushlist();
                g.strut(3);
                g.text("cn ", fgcolor);
                loopscoregroup(g.textf("%d", 0xFFFFFF, NULL, 0, o->clientnum));
                g.poplist();
            }

            if(showskills)
            {
                g.pushlist();
                g.strut(3);
                g.text("sk ", fgcolor);
                loopscoregroup({
                    if(o->aitype >= 0) g.textf("%d", 0xFFFFFF, NULL, 0, o->skill);
                    else g.space(1);
                });
                g.poplist();
            }

            if(showownernum)
            {
                g.pushlist();
                g.strut(3);
                g.text("on", fgcolor);
                loopscoregroup({
                    if(o->aitype >= 0) g.textf("%d", 0xFFFFFF, NULL, 0, o->ownernum);
                    else g.space(1);
                });
                g.poplist();
            }

            if(sg.team && m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
            {
                g.poplist(); // horizontal
                g.poplist(); // vertical
            }

            g.poplist(); // horizontal
            g.poplist(); // vertical

            if(k+1<numgroups && (k+1)%2) g.space(2);
            else g.poplist(); // horizontal
        }
        g.popfont();
        if(showspectators && spectators.length())
        {
            g.space(1);
            g.pushfont("little");
            g.pushlist();
            g.pushlist();
            int count = numgroups > 1 ? 5 : 3;
            bool pushed = false;
            loopv(spectators)
            {
                gameent *o = spectators[i];
                int bgcol = o==game::player1 && highlightscore ? 0x888888 : 0;
                if(o->privilege) bgcol |= o->privilege >= PRIV_ADMIN ? 0x226622 : 0x666622;
                if((i%count)==0)
                {
                    g.pushlist();
                    pushed = true;
                }
                g.pushlist();
                if(bgcol) g.background(bgcol);
                if(showclientnum || game::player1->privilege>=PRIV_MASTER)
                    g.textf("%s (%d)", 0x888888, hud::conopentex, game::getcolour(o), game::colorname(o, NULL, "", false), o->clientnum);
                else g.textf("%s", 0x888888, hud::conopentex, game::getcolour(o), game::colorname(o, NULL, "", false));
                g.poplist();
                if((i+1)%count)
                {
                    if(i+1<spectators.length()) g.space(1);
                }
                else if(pushed)
                {
                    g.poplist();
                    pushed = false;
                }
            }
            if(pushed) g.poplist();
            g.poplist();
            g.poplist();
            g.popfont();
        }
        if(m_play(game::gamemode) && game::player1->state != CS_SPECTATOR && (game::intermission || showscoresinfo))
        {
            float ratio = game::player1->frags >= game::player1->deaths ? (game::player1->frags/float(max(game::player1->deaths, 1))) : -(game::player1->deaths/float(max(game::player1->frags, 1)));
            g.space(1);
            g.pushfont("default");
            g.textf("\fs\fg%d\fS %s, \fs\fg%d\fS %s, \fs\fy%.1f\fS:\fs\fy%.1f\fS ratio, \fs\fg%d\fS damage", 0xFFFFFF, NULL, 0,
                game::player1->frags, game::player1->frags != 1 ? "frags" : "frag",
                game::player1->deaths, game::player1->deaths != 1 ? "deaths" : "death", ratio >= 0 ? ratio : 1.f, ratio >= 0 ? 1.f : -ratio,
                game::player1->totaldamage);
            g.popfont();
        }
        g.end();
    }

    int drawscoreitem(const char *icon, int colour, int x, int y, int s, float skew, float fade, int pos, int score, const char *name)
    {
        const char *col = "\fa";
        switch(pos)
        {
            case 0: col = "\fg"; break;
            case 1: col = "\fy"; break;
            case 2: default: col = "\fo"; break;
        }
        vec c = vec::hexcolor(colour);
        int size = int(s*skew); size += int(size*inventoryglow);
        hud::drawitem(icon, x, y+size, s, inventoryscoreglow!=0, false, c.r, c.g, c.b, fade, skew, "super", "%s%d", col, score);
        hud::drawitemsubtext(x, y+size, s, TEXT_RIGHT_UP, skew, "default", fade, "\f[%d]%s", colour, name);
        return size;
    }

    int drawscore(int x, int y, int s, int m, float blend)
    {
        if(!m_fight(game::gamemode) || m_trial(game::gamemode)) return 0;
        int sy = 0, numgroups = groupplayers(), numout = 0;
        loopi(2) loopk(numgroups)
        {
            if(sy > m) break;
            scoregroup &sg = *groups[k];
            if(m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
            {
                if(!sg.team || ((sg.team != game::focus->team) == !i)) continue;
                float sk = numout && inventoryscoreshrink > 0 ? 1.f-min(numout*inventoryscoreshrink, inventoryscoreshrinkmax) : 1;
                sy += drawscoreitem(hud::teamtex(sg.team), TEAM(sg.team, colour), x, y+sy, s, sk*inventoryscoresize, blend*inventoryblend, k, sg.total, TEAM(sg.team, name));
                if(++numout >= inventoryscore) return sy;
            }
            else
            {
                if(sg.team) continue;
                loopvj(sg.players)
                {
                    gameent *d = sg.players[j];
                    if((d != game::focus) == !i) continue;
                    float sk = numout && inventoryscoreshrink > 0 ? 1.f-min(numout*inventoryscoreshrink, inventoryscoreshrinkmax) : 1;
                    sy += drawscoreitem(hud::playertex, game::getcolour(d, CTONE_MIXED), x, y+sy, s, sk*inventoryscoresize, blend*inventoryblend, j, d->points, game::colorname(d, NULL, "", false));
                    if(++numout >= inventoryscore) return sy;
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
            pushfont("reduced");
            scoregroup &sg = *groups[0];
            if(m_team(game::gamemode, game::mutators))
            {
                if(sg.total) sy += draw_textx("\fg%s", x, y, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, timetostr(sg.total));
            }
            else if(!sg.players.empty())
            {
                if(sg.players[0]->cptime) sy += draw_textx("\fg%s", x, y, 255, 255, 255, int(blend*255), TEXT_LEFT_UP, -1, -1, timetostr(sg.players[0]->cptime));
            }
            popfont();
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
