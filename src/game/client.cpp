#include "game.h"

namespace client
{
    bool sendplayerinfo = false, sendcrcinfo = false, sendgameinfo = false, isready = false, remote = false,
        demoplayback = false, needsmap = false, gettingmap = false;
    int lastping = 0, sessionid = 0;
    string connectpass = "";
    int needclipboard = -1;

    VAR(IDF_PERSIST, showpresence, 0, 1, 2); // 0 = never show join/leave, 1 = show only during game, 2 = show when connecting/disconnecting
    VAR(IDF_PERSIST, showteamchange, 0, 1, 2); // 0 = never show, 1 = show only when switching between, 2 = show when entering match too

    int state() { return game::player1->state; }
    ICOMMAND(0, getplayerstate, "", (), intret(state()));

    int otherclients()
    {
        int n = 0; // ai don't count
        loopv(game::players) if(game::players[i] && game::players[i]->aitype == AI_NONE) n++;
        return n;
    }

    int numplayers()
    {
        int n = 1; // count ourselves
        loopv(game::players) if(game::players[i] && game::players[i]->aitype < AI_START) n++;
        return n;
    }

    int waiting(bool state)
    {
        if(!connected(false) || !isready || game::maptime <= 0 || (state && needsmap && otherclients()))
            return state && needsmap ? (gettingmap ? 3 : 2) : 1;
        return 0;
    }
    ICOMMAND(0, waiting, "i", (int *n), intret(waiting(!*n)));

    ICOMMAND(0, getmaplist, "iii", (int *g, int *m, int *c), {
         char *list = NULL;
         maplist(list, *g, *m, *c, mapsfilter);
         if(list)
         {
             result(list);
             DELETEA(list);
         }
         else result("");
    });

    extern int sortvotes;
    struct mapvote
    {
        vector<gameent *> players;
        string map;
        int millis, mode, muts;

        mapvote() {}
        ~mapvote() { players.shrink(0); }

        static bool compare(const mapvote &a, const mapvote &b)
        {
            if(sortvotes)
            {
                if(a.players.length() > b.players.length()) return true;
                if(a.players.length() < b.players.length()) return false;
            }
            return a.millis < b.millis;
        }
    };
    vector<mapvote> mapvotes;

    VARF(IDF_PERSIST, sortvotes, 0, 0, 1, mapvotes.sort(mapvote::compare));
    VARF(IDF_PERSIST, cleanvotes, 0, 0, 1, {
        if(cleanvotes && !mapvotes.empty()) loopvrev(mapvotes) if(mapvotes[i].players.empty()) mapvotes.remove(i);
    });

    void clearvotes(gameent *d, bool msg)
    {
        int found = 0;
        loopvrev(mapvotes) if(mapvotes[i].players.find(d) >= 0)
        {
            found++;
            mapvotes[i].players.removeobj(d);
            if(cleanvotes && mapvotes[i].players.empty()) mapvotes.remove(i);
        }
        if(found)
        {
            if(!mapvotes.empty()) mapvotes.sort(mapvote::compare);
            if(msg && d == game::player1) conoutft(CON_EVENT, "%s cleared their previous vote", game::colorname(d));
        }
    }

    void vote(gameent *d, const char *text, int mode, int muts)
    {
        mapvote *m = NULL;
        if(!mapvotes.empty()) loopvrev(mapvotes)
        {
            if(mapvotes[i].players.find(d) >= 0)
            {
                if(!strcmp(text, mapvotes[i].map) && mode == mapvotes[i].mode && muts == mapvotes[i].muts) return;
                mapvotes[i].players.removeobj(d);
                if(cleanvotes && mapvotes[i].players.empty()) mapvotes.remove(i);
            }
            if(!strcmp(text, mapvotes[i].map) && mode == mapvotes[i].mode && muts == mapvotes[i].muts) m = &mapvotes[i];
        }
        if(!m)
        {
            m = &mapvotes.add();
            copystring(m->map, text);
            m->mode = mode;
            m->muts = muts;
            m->millis = totalmillis;
        }
        m->players.add(d);
        mapvotes.sort(mapvote::compare);
        if(!isignored(d->clientnum))
        {
            SEARCHBINDCACHE(votekey)("showgui maps 2", 0);
            conoutft(CON_EVENT, "%s suggests: \fs\fy%s\fS on map \fs\fo%s\fS, press \fs\fc%s\fS to vote", game::colorname(d), server::gamename(mode, muts), text, votekey);
        }
    }

    void getvotes(int vote, int prop, int idx)
    {
        if(vote < 0) intret(mapvotes.length());
        else if(mapvotes.inrange(vote))
        {
            mapvote &v = mapvotes[vote];
            if(prop < 0) intret(4);
            else switch(prop)
            {
                case 0:
                    if(idx < 0) intret(v.players.length());
                    else if(v.players.inrange(idx)) intret(v.players[idx]->clientnum);
                    break;
                case 1: intret(v.mode); break;
                case 2: intret(v.muts); break;
                case 3: result(v.map); break;
            }
        }
    }
    ICOMMAND(0, getvote, "bbb", (int *vote, int *prop, int *idx, int *numargs), getvotes(*vote, *prop, *idx));

    struct demoinfo
    {
        demoheader hdr;
        string file, mapname;
        int gamemode, mutators;
    };
    vector<demoinfo> demoinfos;

    int demoint(stream *f)
    {
        int c = (char)f->get<uchar>();
        if(c==-128) { int n = f->get<uchar>(); n |= char(f->get<uchar>())<<8; return n; }
        else if(c==-127) { int n = f->get<uchar>(); n |= f->get<uchar>()<<8; n |= f->get<uchar>()<<16; return n|(f->get<uchar>()<<24); }
        else return c;
    }

    int scandemo(const char *name)
    {
        if(!name || !*name) return -1;
        loopv(demoinfos) if(!strcmp(demoinfos[i].file, name)) return i;
        stream *f = opengzfile(name, "rb");
        if(!f) return -1;
        int num = demoinfos.length();
        demoinfo &d = demoinfos.add();
        copystring(d.file, name);
        mkstring(msg);
        if(f->read(&d.hdr, sizeof(demoheader))!=sizeof(demoheader) || memcmp(d.hdr.magic, DEMO_MAGIC, sizeof(d.hdr.magic)))
            formatstring(msg)("\frsorry, \fs\fc%s\fS is not a demo file", name);
        else
        {
            lilswap(&d.hdr.version, 2);
            if(d.hdr.version!=DEMO_VERSION) formatstring(msg)("\frdemo \fs\fc%s\fS requires %s version of %s", name, d.hdr.version<DEMO_VERSION ? "an older" : "a newer", RE_NAME);
            else if(d.hdr.gamever!=GAMEVERSION) formatstring(msg)("\frdemo \fs\fc%s\fS requires %s version of %s", name, d.hdr.gamever<GAMEVERSION ? "an older" : "a newer", RE_NAME);
        }
        if(msg[0])
        {
            conoutft(CON_INFO, "%s", msg);
            demoinfos.pop();
            delete f;
            return -1;
        }
        if(f->seek(sizeof(int)*3, SEEK_CUR) && demoint(f) == N_WELCOME && demoint(f) == N_MAPCHANGE)
        {
            char *t = d.mapname;
            do // serialised version of getstring
            {
                if(t >= &d.mapname[MAXSTRLEN]) { d.mapname[MAXSTRLEN-1] = 0; break; }
                int c = (char)demoint(f);
                if(c == -1)
                {
                    conoutft(CON_INFO, "\frunable to parse map name from \fs\fc%s\fS", name);
                    demoinfos.pop();
                    delete f;
                    return -1;
                }
                *t = c;
            } while(*t++);
            demoint(f);
            d.gamemode = demoint(f);
            d.mutators = demoint(f);
            //conoutft(CON_INFO, "read demo %s: [%d:%d] %s on %s", d.file, d.hdr.version, d.hdr.gamever, server::gamename(d.gamemode, d.mutators), d.mapname);
            delete f;
            return num;
        }
        conoutft(CON_INFO, "\frunexpected message while reading \fs\fc%s\fS", name);
        demoinfos.pop();
        delete f;
        return -1;
    }
    ICOMMAND(0, demoscan, "s", (char *name), intret(scandemo(name)));

    void infodemo(int idx, int prop)
    {
        if(idx < 0) intret(demoinfos.length());
        else if(demoinfos.inrange(idx))
        {
            demoinfo &d = demoinfos[idx];
            switch(prop)
            {
                case 0: intret(d.hdr.version); break;
                case 1: intret(d.hdr.gamever); break;
                case 2: result(d.mapname); break;
                case 3: intret(d.gamemode); break;
                case 4: intret(d.mutators); break;
                default: break;
            }
        }
    }
    ICOMMAND(0, demoinfo, "bb", (int *idx, int *prop, int *numargs), infodemo(*idx, *prop));

    VAR(IDF_PERSIST, authconnect, 0, 1, 1);
    SVAR(IDF_PERSIST, accountname, "");
    SVAR(IDF_PERSIST, accountpass, "");
    ICOMMAND(0, authkey, "ss", (char *name, char *key), {
        setsvar("accountname", name);
        setsvar("accountpass", key);
    });
    ICOMMAND(0, hasauthkey, "", (), intret(accountname[0] && accountpass[0] ? 1 : 0));

    void writegamevars(const char *name, bool all = false, bool server = false)
    {
        if(!name || !*name) name = "vars.cfg";
        stream *f = openfile(name, "w");
        if(!f) return;
        vector<ident *> ids;
        enumerate(idents, ident, id, ids.add(&id));
        ids.sort(ident::compare);
        loopv(ids)
        {
            ident &id = *ids[i];
            if(id.flags&IDF_CLIENT) switch(id.type)
            {
                case ID_VAR:
                    if(*id.storage.i == id.def.i)
                    {
                        if(all) f->printf("// ");
                        else break;
                    }
                    if(server) f->printf("sv_");
                    f->printf((id.flags&IDF_HEX && *id.storage.i >= 0 ? (id.maxval==0xFFFFFF ? "%s 0x%.6X\n" : "%s 0x%X\n") : "%s %d\n"), id.name, *id.storage.i);
                    break;
                case ID_FVAR:
                    if(*id.storage.f == id.def.f)
                    {
                        if(all) f->printf("// ");
                        else break;
                    }
                    if(server) f->printf("sv_");
                    f->printf("%s %s\n", id.name, floatstr(*id.storage.f));
                    break;
                case ID_SVAR:
                    if(!strcmp(*id.storage.s, id.def.s))
                    {
                        if(all) f->printf("// ");
                        else break;
                    }
                    if(server) f->printf("sv_");
                    f->printf("%s %s\n", id.name, escapestring(*id.storage.s));
                    break;
            }
        }
        delete f;
    }
    ICOMMAND(0, writevars, "sii", (char *name, int *all, int *sv), if(!(identflags&IDF_WORLD)) writegamevars(name, *all!=0, *sv!=0));

    // collect c2s messages conveniently
    vector<uchar> messages;
    bool messagereliable = false;

    VAR(IDF_PERSIST, colourchat, 0, 1, 1);
    VAR(IDF_PERSIST, showlaptimes, 0, 2, 3); // 0 = off, 1 = only player, 2 = +humans, 3 = +bots

    const char *defaultserversort()
    {
        static string vals;
        formatstring(vals)("%d %d", SINFO_NUMPLRS, SINFO_PING);
        return vals;
    }

    void resetserversort()
    {
        setsvarchecked(getident("serversort"), defaultserversort());
    }
    ICOMMAND(0, serversortreset, "", (), resetserversort());

    vector<int> serversortstyles;
    void updateserversort();
    SVARF(IDF_PERSIST, serversort, defaultserversort(),
    {
        if(!serversort[0] || serversort[0] == '[')
        {
            delete[] serversort;
            serversort = newstring(defaultserversort());
        }
        updateserversort();
    });

    void updateserversort()
    {
        vector<char *> styles;
        explodelist(serversort, styles, SINFO_MAX);
        serversortstyles.setsize(0);
        loopv(styles) serversortstyles.add(parseint(styles[i]));
        styles.deletearrays();
    }

    void getvitem(gameent *d, int n, int v)
    {
        if(n < 0) intret(d->vitems.length());
        else if(v < 0) intret(2);
        else if(d->vitems.inrange(n)) switch(v)
        {
            case 0: intret(d->vitems[n]); break;
            case 1: if(vanities.inrange(d->vitems[n])) result(vanities[d->vitems[n]].ref); break;
            default: break;
        }
    }

    ICOMMAND(0, mastermode, "i", (int *val), addmsg(N_MASTERMODE, "ri", *val));
    ICOMMAND(0, getplayername, "", (), result(game::player1->name));
    ICOMMAND(0, getplayercolour, "i", (int *m), intret(*m >= 0 ? game::getcolour(game::player1, *m) : game::player1->colour));
    ICOMMAND(0, getplayermodel, "", (), intret(game::player1->model));
    ICOMMAND(0, getplayervanity, "", (), result(game::player1->vanity));
    ICOMMAND(0, getplayervitem, "bi", (int *n, int *v), getvitem(game::player1, *n, *v));
    ICOMMAND(0, getplayerteam, "i", (int *p), *p ? intret(game::player1->team) : result(TEAM(game::player1->team, name)));
    ICOMMAND(0, getplayerteamicon, "", (), result(hud::teamtexname(game::player1->team)));
    ICOMMAND(0, getplayerteamcolour, "", (), intret(TEAM(game::player1->team, colour)));

    const char *getname() { return game::player1->name; }

    void setplayername(const char *name)
    {
        if(name && *name)
        {
            string text;
            filtertext(text, name, true, true, true, MAXNAMELEN);
            const char *namestr = text;
            while(*namestr && iscubespace(*namestr)) namestr++;
            if(*namestr && strcmp(game::player1->name, namestr))
            {
                game::player1->setname(namestr);
                sendplayerinfo = true;
            }
        }
    }
    SVARF(IDF_PERSIST, playername, "", setplayername(playername));

    void setplayercolour(int colour)
    {
        if(colour >= 0 && colour <= 0xFFFFFF && game::player1->colour != colour)
        {
            game::player1->colour = colour;
            sendplayerinfo = true;
        }
    }
    VARF(IDF_PERSIST|IDF_HEX, playercolour, -1, -1, 0xFFFFFF, setplayercolour(playercolour));

    void setplayermodel(int model)
    {
        if(model >= 0 && game::player1->model != model)
        {
            game::player1->model = model;
            sendplayerinfo = true;
        }
    }
    VARF(IDF_PERSIST, playermodel, 0, 0, PLAYERTYPES-1, setplayermodel(playermodel));

#ifdef VANITY
    SVARF(IDF_PERSIST, playervanity, "", if(game::player1->setvanity(playervanity)) sendplayerinfo = true;);
#endif

    int teamname(const char *team)
    {
        if(m_fight(game::gamemode) && m_isteam(game::gamemode, game::mutators))
        {
            if(team[0])
            {
                int t = atoi(team);

                loopi(numteams(game::gamemode, game::mutators))
                {
                    if((t && t == i+T_FIRST) || !strcasecmp(TEAM(i+T_FIRST, name), team))
                    {
                        return i+T_FIRST;
                    }
                }
            }
            return T_FIRST;
        }
        return T_NEUTRAL;
    }

    void switchteam(const char *team)
    {
        if(team[0])
        {
            if(m_fight(game::gamemode) && m_isteam(game::gamemode, game::mutators))
            {
                int t = teamname(team);
                if(t != game::player1->team) addmsg(N_SWITCHTEAM, "ri", t);
            }
            else conoutft(CON_INFO, "\frcan only change teams when actually playing in team games");
        }
        else conoutft(CON_INFO, "\fs\fgyour team is:\fS \fs\f[%d]%s\fS", TEAM(game::player1->team, colour), TEAM(game::player1->team, name));
    }
    ICOMMAND(0, team, "s", (char *s), switchteam(s));

    bool allowedittoggle(bool edit)
    {
        bool allow = edit || m_edit(game::gamemode); // && game::player1->state == CS_ALIVE);
        if(!allow) conoutft(CON_INFO, "\fryou must start an editing game to edit the map");
        return allow;
    }

    void edittoggled(bool edit)
    {
        game::player1->editspawn(game::gamemode, game::mutators);
        game::player1->state = edit ? CS_EDITING : (m_edit(game::gamemode) ? CS_ALIVE : CS_DEAD);
        game::player1->o = camera1->o;
        game::player1->yaw = camera1->yaw;
        game::player1->pitch = camera1->pitch;
        game::player1->resetinterp();
        game::resetstate();
        game::resetfollow();
        physics::entinmap(game::player1, true); // find spawn closest to current floating pos
        projs::remove(game::player1);
        if(m_edit(game::gamemode)) addmsg(N_EDITMODE, "ri", edit ? 1 : 0);
    }

    const char *getclientname(int cn, int colour)
    {
        gameent *d = game::getclient(cn);
        if(colour && d) return game::colorname(d);
        return d ? d->name : "";
    }
    ICOMMAND(0, getclientname, "ii", (int *cn, int *colour), result(getclientname(*cn, *colour)));

    int getclientcolour(int cn)
    {
        gameent *d = game::getclient(cn);
        return d ? d->colour : -1;
    }
    ICOMMAND(0, getclientcolour, "i", (int *cn), intret(getclientcolour(*cn)));

    int getclientmodel(int cn)
    {
        gameent *d = game::getclient(cn);
        return d ? d->model%PLAYERTYPES : -1;
    }
    ICOMMAND(0, getclientmodel, "i", (int *cn), intret(getclientmodel(*cn)));

    const char *getmodelname(int mdl, int idx)
    {
        return mdl >= 0 ? playertypes[mdl%PLAYERTYPES][clamp(idx, 0, 2)] : "";
    }
    ICOMMAND(0, getmodelname, "iiN", (int *mdl, int *idx, int *numargs), result(getmodelname(*mdl, *numargs >= 2 ? *idx : 2)));

    const char *getclientvanity(int cn)
    {
        gameent *d = game::getclient(cn);
        return d ? d->vanity : "";
    }
    ICOMMAND(0, getclientvanity, "i", (int *cn), result(getclientvanity(*cn)));

    void getclientvitem(int cn, int n, int v)
    {
        gameent *d = game::getclient(cn);
        if(d) getvitem(d, n, v);
    }
    ICOMMAND(0, getclientvitem, "ibi", (int *cn, int *n, int *v), getclientvitem(*cn, *n, *v));

    const char *getclienthost(int cn)
    {
        gameent *d = game::getclient(cn);
        return d ? d->hostname : "";
    }
    ICOMMAND(0, getclienthost, "i", (int *cn), result(getclienthost(*cn)));

    int getclientteam(int cn)
    {
        gameent *d = game::getclient(cn);
        return d ? d->team : -1;
    }
    ICOMMAND(0, getclientteam, "i", (int *cn), intret(getclientteam(*cn)));

    bool haspriv(gameent *d, int priv)
    {
        if(!d) return false;
        if(d == game::player1 && !remote) return true;
        return d->privilege >= priv;
    }
    ICOMMAND(0, issupporter, "i", (int *cn), intret(haspriv(game::getclient(*cn), PRIV_SUPPORTER) ? 1 : 0));
    ICOMMAND(0, ismoderator, "i", (int *cn), intret(haspriv(game::getclient(*cn), PRIV_MODERATOR) ? 1 : 0));
    ICOMMAND(0, isadministrator, "i", (int *cn), intret(haspriv(game::getclient(*cn), PRIV_ADMINISTRATOR) ? 1 : 0));
    ICOMMAND(0, isdeveloper, "i", (int *cn), intret(haspriv(game::getclient(*cn), PRIV_DEVELOPER) ? 1 : 0));
    ICOMMAND(0, iscreator, "i", (int *cn), intret(haspriv(game::getclient(*cn), PRIV_CREATOR) ? 1 : 0));

    ICOMMAND(0, getclientpriv, "i", (int *cn, int *priv), intret(haspriv(game::getclient(*cn), *priv) ? 1 : 0));

    const char *getclienthandle(int cn)
    {
        gameent *d = game::getclient(cn);
        return d ? d->handle : "";
    }
    ICOMMAND(0, getclienthandle, "i", (int *cn), result(getclienthandle(*cn)));

    bool isspectator(int cn)
    {
        gameent *d = game::getclient(cn);
        return d && d->state == CS_SPECTATOR;
    }
    ICOMMAND(0, isspectator, "i", (int *cn), intret(isspectator(*cn) ? 1 : 0));

    bool isai(int cn, int type)
    {
        gameent *d = game::getclient(cn);
        int aitype = type > 0 && type < AI_MAX ? type : AI_BOT;
        return d && d->aitype == aitype;
    }
    ICOMMAND(0, isai, "ii", (int *cn, int *type), intret(isai(*cn, *type) ? 1 : 0));

    bool mutscmp(int req, int limit)
    {
        if(req)
        {
            if(!limit) return false;
            loopi(G_M_NUM) if(req&(1<<i) && !(limit&(1<<i))) return false;
        }
        return true;
    }

    bool ismodelocked(int reqmode, int reqmuts, int askmuts = 0, const char *reqmap = NULL)
    {
        if(m_local(reqmode) && remote) return true;
        if(G(modelock) == PRIV2(MAX) && G(mapslock) == PRIV2(MAX) && !haspriv(game::player1, PRIV_MAX)) return true;
        else if(G(votelock)) switch(G(votelocktype))
        {
            case 1: if(!haspriv(game::player1, PRIVZ(G(votelock)))) return true; break;
            case 2:
                if(!m_edit(reqmode) && reqmap && *reqmap)
                {
                    int n = listincludes(prevmaps, reqmap, strlen(reqmap));
                    if(n >= 0 && n < G(maphistory) && !haspriv(game::player1, PRIVZ(G(votelock)))) return true;
                }
                break;
            case 0: default: break;
        }
        int rmode = reqmode, rmuts = reqmuts;
        modecheck(rmode, rmuts, askmuts);
        if(askmuts && !mutscmp(askmuts, rmuts)) return true;
        if(G(modelock)) switch(G(modelocktype))
        {
            case 1: if(!haspriv(game::player1, PRIVZ(G(modelock)))) return true; break;
            case 2: if((!((1<<reqmode)&G(modelockfilter)) || !mutscmp(reqmuts, G(mutslockfilter))) && !haspriv(game::player1, PRIVZ(G(modelock)))) return true; break;
            case 0: default: break;
        }
        if(reqmode != G_EDITMODE && G(mapslock) && reqmap && *reqmap)
        {
            char *list = NULL;
            switch(G(mapslocktype))
            {
                case 1:
                {
                    list = newstring(G(allowmaps));
                    mapcull(list, reqmode, reqmuts, numplayers(), G(mapsfilter));
                    break;
                }
                case 2:
                {
                    maplist(list, reqmode, reqmuts, numplayers(), G(mapsfilter));
                    break;
                }
                case 0: default: break;
            }
            if(list)
            {
                if(listincludes(list, reqmap, strlen(reqmap)) < 0 && !haspriv(game::player1, PRIVZ(G(modelock))))
                {
                    DELETEA(list);
                    return true;
                }
                DELETEA(list);
            }
        }
        return false;
    }
    ICOMMAND(0, ismodelocked, "iiis", (int *g, int *m, int *a, char *s), intret(ismodelocked(*g, *m, *a, s) ? 1 : 0));

    int parseplayer(const char *arg)
    {
        char *end;
        int n = strtol(arg, &end, 10);
        if(*arg && !*end)
        {
            if(n!=game::player1->clientnum && !game::players.inrange(n)) return -1;
            return n;
        }
        // try case sensitive first
        loopv(game::players) if(game::players[i])
        {
            gameent *o = game::players[i];
            if(!strcmp(arg, o->name)) return o->clientnum;
        }
        // nothing found, try case insensitive
        loopv(game::players) if(game::players[i])
        {
            gameent *o = game::players[i];
            if(!strcasecmp(arg, o->name)) return o->clientnum;
        }
        return -1;
    }
    ICOMMAND(0, getclientnum, "s", (char *name), intret(name[0] ? parseplayer(name) : game::player1->clientnum));

    void listclients(bool local)
    {
        vector<char> buf;
        string cn;
        int numclients = 0;
        if(local)
        {
            formatstring(cn)("%d", game::player1->clientnum);
            buf.put(cn, strlen(cn));
            numclients++;
        }
        loopv(game::players) if(game::players[i])
        {
            formatstring(cn)("%d", game::players[i]->clientnum);
            if(numclients++) buf.add(' ');
            buf.put(cn, strlen(cn));
        }
        buf.add('\0');
        result(buf.getbuf());
    }
    ICOMMAND(0, listclients, "i", (int *local), listclients(*local!=0));

    void addcontrol(const char *arg, int type, const char *msg)
    {
        int i = parseplayer(arg);
        if(i >= 0) addmsg(N_ADDCONTROL, "ri2s", i, type, msg);
    }
    ICOMMAND(0, kick, "ss", (char *s, char *m), addcontrol(s, -1, m));
    ICOMMAND(0, allow, "ss", (char *s, char *m), addcontrol(s, ipinfo::ALLOW, m));
    ICOMMAND(0, ban, "ss", (char *s, char *m), addcontrol(s, ipinfo::BAN, m));
    ICOMMAND(0, mute, "ss", (char *s, char *m), addcontrol(s, ipinfo::MUTE, m));
    ICOMMAND(0, limit, "ss", (char *s, char *m), addcontrol(s, ipinfo::LIMIT, m));

    ICOMMAND(0, clearallows, "", (), addmsg(N_CLRCONTROL, "ri", ipinfo::ALLOW));
    ICOMMAND(0, clearbans, "", (), addmsg(N_CLRCONTROL, "ri", ipinfo::BAN));
    ICOMMAND(0, clearmutes, "", (), addmsg(N_CLRCONTROL, "ri", ipinfo::MUTE));
    ICOMMAND(0, clearlimits, "", (), addmsg(N_CLRCONTROL, "ri", ipinfo::LIMIT));

    vector<int> ignores;
    void ignore(int cn)
    {
        gameent *d = game::getclient(cn);
        if(!d || d == game::player1) return;
        conoutft(CON_EVENT, "\fs\fcignoring:\fS %s", d->name);
        if(ignores.find(cn) < 0) ignores.add(cn);
    }

    void unignore(int cn)
    {
        if(ignores.find(cn) < 0) return;
        gameent *d = game::getclient(cn);
        if(d) conoutft(CON_EVENT, "\fs\fcstopped ignoring:\fS %s", d->name);
        ignores.removeobj(cn);
    }

    bool isignored(int cn) { return ignores.find(cn) >= 0; }

    ICOMMAND(0, ignore, "s", (char *arg), ignore(parseplayer(arg)));
    ICOMMAND(0, unignore, "s", (char *arg), unignore(parseplayer(arg)));
    ICOMMAND(0, isignored, "s", (char *arg), intret(isignored(parseplayer(arg)) ? 1 : 0));

    void setteam(const char *arg1, const char *arg2)
    {
        if(m_fight(game::gamemode) && m_isteam(game::gamemode, game::mutators))
        {
            int i = parseplayer(arg1);
            if(i>=0)
            {
                int t = teamname(arg2);
                if(t) addmsg(N_SETTEAM, "ri2", i, t);
            }
        }
        else conoutft(CON_INFO, "\frcan only change teams in team games");
    }
    ICOMMAND(0, setteam, "ss", (char *who, char *team), setteam(who, team));

    void hashpwd(const char *pwd)
    {
        if(game::player1->clientnum<0) return;
        string hash;
        server::hashpassword(game::player1->clientnum, sessionid, pwd, hash);
        result(hash);
    }
    COMMAND(0, hashpwd, "s");

    void setpriv(const char *arg)
    {
        if(!arg[0]) return;
        int val = 1;
        mkstring(hash);
        if(!arg[1] && isdigit(arg[0])) val = parseint(arg);
        else server::hashpassword(game::player1->clientnum, sessionid, arg, hash);
        addmsg(N_SETPRIV, "ris", val, hash);
    }
    COMMAND(0, setpriv, "s");

    void tryauth()
    {
        if(!accountname[0]) return;
        addmsg(N_AUTHTRY, "rs", accountname);
    }
    ICOMMAND(0, auth, "", (), tryauth());

    void togglespectator(int val, const char *who)
    {
        int i = who[0] ? parseplayer(who) : game::player1->clientnum;
        if(i>=0) addmsg(N_SPECTATOR, "rii", i, val);
    }
    ICOMMAND(0, spectator, "is", (int *val, char *who), togglespectator(*val, who));

    ICOMMAND(0, checkmaps, "", (), addmsg(N_CHECKMAPS, "r"));

    void connectattempt(const char *name, int port, const char *password, const ENetAddress &address)
    {
        if(*password) { copystring(connectpass, password); }
        else memset(connectpass, 0, sizeof(connectpass));
    }

    void connectfail()
    {
        memset(connectpass, 0, sizeof(connectpass));
    }

    void gameconnect(bool _remote)
    {
        remote = _remote;
        if(editmode) toggleedit();
    }

    void gamedisconnect(int clean)
    {
        if(editmode) toggleedit();
        gettingmap = needsmap = remote = isready = sendgameinfo = sendplayerinfo = false;
        sessionid = 0;
        ignores.shrink(0);
        messages.shrink(0);
        mapvotes.shrink(0);
        messagereliable = false;
        projs::remove(game::player1);
        removetrackedparticles(game::player1);
        removetrackedsounds(game::player1);
        game::player1->clientnum = -1;
        game::player1->privilege = PRIV_NONE;
        game::player1->handle[0] = 0;
        game::gamemode = G_EDITMODE;
        game::mutators = 0;
        loopv(game::players) if(game::players[i]) game::clientdisconnected(i);
        game::waiting.setsize(0);
        emptymap(0, true, NULL, true);
        smartmusic(true, false);
        enumerate(idents, ident, id, {
            if(id.flags&IDF_CLIENT) switch(id.type)
            {
                case ID_VAR: setvar(id.name, id.def.i, true); break;
                case ID_FVAR: setfvar(id.name, id.def.f, true); break;
                case ID_SVAR: setsvar(id.name, *id.def.s ? id.def.s : "", true); break;
                default: break;
            }
        });
        if(clean) game::clientmap[0] = '\0';
    }

    void addmsg(int type, const char *fmt, ...)
    {
        static uchar buf[MAXTRANS];
        ucharbuf p(buf, MAXTRANS);
        putint(p, type);
        int numi = 1, nums = 0;
        bool reliable = false;
        if(fmt)
        {
            va_list args;
            va_start(args, fmt);
            while(*fmt) switch(*fmt++)
            {
                case 'r': reliable = true; break;
                case 'v':
                {
                    int n = va_arg(args, int);
                    int *v = va_arg(args, int *);
                    loopi(n) putint(p, v[i]);
                    numi += n;
                    break;
                }
                case 'i':
                {
                    int n = isdigit(*fmt) ? *fmt++-'0' : 1;
                    loopi(n) putint(p, va_arg(args, int));
                    numi += n;
                    break;
                }
                case 'f':
                {
                    int n = isdigit(*fmt) ? *fmt++-'0' : 1;
                    loopi(n) putfloat(p, (float)va_arg(args, double));
                    numi += n;
                    break;
                }
                case 's': sendstring(va_arg(args, const char *), p); nums++; break;
            }
            va_end(args);
        }
        int num = nums?0:numi, msgsize = msgsizelookup(type);
        if(msgsize && num!=msgsize) { fatal("inconsistent msg size for %d (%d != %d)", type, num, msgsize); }
        if(reliable) messagereliable = true;
        messages.put(buf, p.length());
    }

    void saytext(gameent *d, int flags, char *text)
    {
        filtertext(text, text, true, colourchat ? false : true);
        mkstring(s);
        defformatstring(m)("%s", game::colorname(d));
        if(flags&SAY_TEAM)
        {
            defformatstring(t)(" (\fs\f[%d]%s\fS)", TEAM(d->team, colour), TEAM(d->team, name));
            concatstring(m, t);
        }
        if(flags&SAY_ACTION) formatstring(s)("\fv* \fs%s\fS \fs\fv%s\fS", m, text);
        else formatstring(s)("\fa<\fs\fw%s\fS> \fs\fw%s\fS", m, text);

        int snd = S_CHAT;
        ident *wid = idents.access(flags&SAY_ACTION ? "on_action" : "on_text");
        if(wid && wid->type == ID_ALIAS && wid->getstr()[0])
        {
            defformatstring(act)("%s %d %d %s %s %s",
                flags&SAY_ACTION ? "on_action" : "on_text", d->clientnum, flags&SAY_TEAM ? 1 : 0,
                escapestring(game::colorname(d)), escapestring(text), escapestring(s));
            int ret = execute(act);
            if(ret > 0) snd = ret;
        }
        conoutft(CON_CHAT, "%s", s);
        if(snd >= 0 && !issound(d->cschan)) playsound(snd, d->o, d, snd != S_CHAT ? 0 : SND_DIRECT, -1, -1, -1, &d->cschan);
    }

    void toserver(int flags, char *text)
    {
        if(!waiting(false) && !client::demoplayback)
        {
            if(flags&SAY_TEAM && !m_isteam(game::gamemode, game::mutators))
                flags &= ~SAY_TEAM;
            saytext(game::player1, flags, text);
            addmsg(N_TEXT, "ri2s", game::player1->clientnum, flags, text);
        }
    }
    ICOMMAND(0, say, "C", (char *s), toserver(SAY_NONE, s));
    ICOMMAND(0, me, "C", (char *s), toserver(SAY_ACTION, s));
    ICOMMAND(0, sayteam, "C", (char *s), toserver(SAY_TEAM, s));
    ICOMMAND(0, meteam, "C", (char *s), toserver(SAY_ACTION|SAY_TEAM, s));

    void parsecommand(gameent *d, const char *cmd, const char *arg)
    {
        ident *id = idents.access(cmd);
        if(id && id->flags&IDF_CLIENT)
        {
            const char *val = NULL;
            switch(id->type)
            {
                case ID_COMMAND:
                {
#if 0 // these shouldn't get here
                    int slen = strlen(cmd)+1+strlen(arg);
                    char *s = newstring(slen+1);
                    formatstring(s)(slen, "%s %s", cmd, arg);
                    char *ret = executestr(s);
                    delete[] s;
                    if(ret) conoutft(CON_EVENT, "\fc%s: %s", cmd, ret);
                    delete[] ret;
#endif
                    return;
                }
                case ID_VAR:
                {
                    int ret = parseint(arg);
                    *id->storage.i = ret;
                    id->changed();
                    val = intstr(id);
                    break;
                }
                case ID_FVAR:
                {
                    float ret = parsefloat(arg);
                    *id->storage.f = ret;
                    id->changed();
                    val = floatstr(*id->storage.f);
                    break;
                }
                case ID_SVAR:
                {
                    delete[] *id->storage.s;
                    *id->storage.s = newstring(arg);
                    id->changed();
                    val = *id->storage.s;
                    break;
                }
                default: return;
            }
            if((d || verbose >= 2) && val)
                conoutft(CON_EVENT, "\fc%s set %s to %s", d ? game::colorname(d) : (connected(false) ? "the server" : "you"), cmd, val);
        }
        else if(verbose) conoutft(CON_EVENT, "\fr%s sent unknown command: %s", d ? game::colorname(d) : "the server", cmd);
    }

    bool sendcmd(int nargs, const char *cmd, const char *arg)
    {
        if(connected(false))
        {
            addmsg(N_COMMAND, "ri2sis", game::player1->clientnum, nargs, cmd, strlen(arg), arg);
            return true;
        }
        else
        {
            defformatstring(scmd)("sv_%s", cmd);
            if(server::servcmd(nargs, scmd, arg))
            {
                if(nargs > 1 && arg) parsecommand(NULL, cmd, arg);
                return true;
            }
        }
        return false;
    }

    void changemapserv(char *name, int gamemode, int mutators, bool temp)
    {
        game::gamemode = gamemode;
        game::mutators = mutators;
        modecheck(game::gamemode, game::mutators);
        game::nextmode = game::gamemode;
        game::nextmuts = game::mutators;
        game::timeremaining = -1;
        game::maptime = 0;
        hud::resetscores();
        mapvotes.shrink(0);
        if(editmode) toggleedit();
        if(m_demo(game::gamemode))
        {
            game::maptime = 1;
            game::intermission = true;
            game::timeremaining = 0;
            return;
        }
        if(m_capture(game::gamemode)) capture::reset();
        else if(m_defend(game::gamemode)) defend::reset();
        else if(m_bomber(game::gamemode)) bomber::reset();
        needsmap = false;
        if(!name || !*name || !load_world(name, temp))
        {
            emptymap(0, true, NULL);
            setnames(name, MAP_MAPZ);
            needsmap = true;
        }
        if(m_capture(game::gamemode)) capture::setup();
        else if(m_defend(game::gamemode)) defend::setup();
        else if(m_bomber(game::gamemode)) bomber::setup();
    }

    void receivefile(uchar *data, int len)
    {
        ucharbuf p(data, len);
        int type = getint(p);
        switch(type)
        {
            case N_SENDDEMO:
            {
                data += p.length();
                len -= p.length();
                string fname;
                if(*filetimeformat) formatstring(fname)("demos/%s.dmo", gettime(clocktime, filetimeformat));
                else formatstring(fname)("demos/%u.dmo", uint(clocktime));
                stream *demo = openfile(fname, "wb");
                if(!demo) return;
                conoutft(CON_MESG, "received demo \"%s\"", fname);
                demo->write(data, len);
                delete demo;
                break;
            }

            case N_SENDMAPFILE:
            {
                int filetype = getint(p);
                data += p.length();
                len -= p.length();
                if(filetype < 0 || filetype >= SENDMAP_MAX) break;
                const char *reqmap = mapname;
                if(!reqmap || !*reqmap) reqmap = "maps/untitled";
                defformatstring(reqfile)(strstr(reqmap, "temp/")==reqmap || strstr(reqmap, "temp\\")==reqmap ? "%s" : "temp/%s", reqmap);
                defformatstring(reqfext)("%s.%s", reqfile, sendmaptypes[filetype]);
                stream *f = openfile(reqfext, "wb");
                if(!f)
                {
                    conoutft(CON_MESG, "\frfailed to open map file: %s", reqfext);
                    break;
                }
                gettingmap = true;
                f->write(data, len);
                delete f;
                break;
            }
        }
    }
    ICOMMAND(0, getmap, "", (), if(multiplayer(false)) addmsg(N_GETMAP, "r"));

    void stopdemo()
    {
        if(remote) addmsg(N_STOPDEMO, "r");
        else server::stopdemo();
    }
    ICOMMAND(0, stopdemo, "", (), stopdemo());

    void recorddemo(int val)
    {
        addmsg(N_RECORDDEMO, "ri", val);
    }
    ICOMMAND(0, recorddemo, "i", (int *val), recorddemo(*val));

    void cleardemos(int val)
    {
        addmsg(N_CLEARDEMOS, "ri", val);
    }
    ICOMMAND(0, cleardemos, "i", (int *val), cleardemos(*val));

    void getdemo(int i)
    {
        if(i<=0) conoutft(CON_MESG, "getting demo...");
        else conoutft(CON_MESG, "getting demo %d...", i);
        addmsg(N_GETDEMO, "ri", i);
    }
    ICOMMAND(0, getdemo, "i", (int *val), getdemo(*val));

    void listdemos()
    {
        conoutft(CON_MESG, "listing demos...");
        addmsg(N_LISTDEMOS, "r");
    }
    ICOMMAND(0, listdemos, "", (), listdemos());

    void changemap(const char *name) // request map change, server may ignore
    {
        int nextmode = game::nextmode, nextmuts = game::nextmuts; // in case stopdemo clobbers these
        if(!remote) stopdemo();
        if(!connected())
        {
            server::changemap(name, nextmode, nextmuts);
            localconnect(true);
        }
        else
        {
            string reqfile;
            copystring(reqfile, !strncasecmp(name, "temp/", 5) || !strncasecmp(name, "temp\\", 5) ? name+5 : name);
            addmsg(N_MAPVOTE, "rsi2", reqfile, nextmode, nextmuts);
        }
    }
    ICOMMAND(0, map, "s", (char *s), changemap(s));
    ICOMMAND(0, clearvote, "", (), addmsg(N_CLEARVOTE, "r"));

    void sendmap()
    {
        conoutft(CON_MESG, "sending map...");
        const char *reqmap = mapname;
        if(!reqmap || !*reqmap) reqmap = "maps/untitled";
        bool edit = m_edit(game::gamemode);
        defformatstring(reqfile)("%s%s", edit ? "temp/" : "", reqmap);
        loopi(SENDMAP_MAX)
        {
            defformatstring(reqfext)("%s.%s", reqfile, sendmaptypes[i]);
            if(!i && edit)
            {
                save_world(reqfile, edit, true);
                setnames(reqmap, MAP_MAPZ);
            }
            stream *f = openfile(reqfext, "rb");
            if(f)
            {
                conoutft(CON_MESG, "\fgtransmitting file: %s", reqfext);
                sendfile(-1, 2, f, "ri2", N_SENDMAPFILE, i);
                if(needclipboard >= 0) needclipboard++;
                delete f;
            }
            else if(i <= SENDMAP_MIN) conoutft(CON_MESG, "\frfailed to open map file: %s", reqfext);
        }
    }
    ICOMMAND(0, sendmap, "", (), sendmap());

    void gotoplayer(const char *arg)
    {
        if(game::player1->state!=CS_SPECTATOR && game::player1->state!=CS_EDITING) return;
        int i = parseplayer(arg);
        if(i>=0 && i!=game::player1->clientnum)
        {
            gameent *d = game::getclient(i);
            if(!d) return;
            game::player1->o = d->o;
            vec dir(game::player1->yaw, game::player1->pitch);
            game::player1->o.add(dir.mul(-32));
            game::player1->resetinterp();
            game::resetcamera(true);
        }
    }
    ICOMMAND(0, goto, "s", (char *s), gotoplayer(s));

    void editvar(ident *id, bool local)
    {
        if(id && id->flags&IDF_WORLD && !(id->flags&IDF_SERVER) && local && m_edit(game::gamemode))
        {
            switch(id->type)
            {
                case ID_VAR:
                    addmsg(N_EDITVAR, "risi", id->type, id->name, *id->storage.i);
                    break;
                case ID_FVAR:
                    addmsg(N_EDITVAR, "risf", id->type, id->name, *id->storage.f);
                    break;
                case ID_SVAR:
                    addmsg(N_EDITVAR, "risis", id->type, id->name, strlen(*id->storage.s), *id->storage.s);
                    break;
                case ID_ALIAS:
                {
                    const char *s = id->getstr();
                    addmsg(N_EDITVAR, "risis", id->type, id->name, strlen(s), s);
                    break;
                }
                default: break;
            }
        }
    }

    void sendclipboard()
    {
        uchar *outbuf = NULL;
        int inlen = 0, outlen = 0;
        if(!packeditinfo(localedit, inlen, outbuf, outlen))
        {
            outbuf = NULL;
            inlen = outlen = 0;
        }
        packetbuf p(16 + outlen, ENET_PACKET_FLAG_RELIABLE);
        putint(p, N_CLIPBOARD);
        putint(p, inlen);
        putint(p, outlen);
        if(outlen > 0) p.put(outbuf, outlen);
        sendclientpacket(p.finalize(), 1);
        needclipboard = -1;
    }

    void edittrigger(const selinfo &sel, int op, int arg1, int arg2, int arg3)
    {
        if(m_edit(game::gamemode) && game::player1->state == CS_EDITING) switch(op)
        {
            case EDIT_FLIP:
            case EDIT_COPY:
            case EDIT_PASTE:
            case EDIT_DELCUBE:
            {
                switch(op)
                {
                    case EDIT_COPY: needclipboard = 0; break;
                    case EDIT_PASTE:
                        if(needclipboard > 0)
                        {
                            c2sinfo(true);
                            sendclipboard();
                        }
                        break;
                }
                addmsg(N_EDITF + op, "ri9i4",
                    sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                    sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner);
                break;
            }
            case EDIT_MAT:
            {
                addmsg(N_EDITF + op, "ri9i7",
                    sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                    sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                    arg1, arg2, arg3);
                break;
            }
            case EDIT_ROTATE:
            {
                addmsg(N_EDITF + op, "ri9i5",
                    sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                    sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                    arg1);
                break;
            }
            case EDIT_FACE:
            case EDIT_TEX:
            {
                addmsg(N_EDITF + op, "ri9i6",
                    sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                    sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                    arg1, arg2);
                break;
            }
            case EDIT_REPLACE:
            {
                addmsg(N_EDITF + op, "ri9i7",
                    sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                    sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                    arg1, arg2, arg3);
                break;
            }
            case EDIT_REMIP:
            {
                addmsg(N_EDITF + op, "r");
                break;
            }
        }
    }

    void sendintro()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        putint(p, N_CONNECT);
        sendstring(game::player1->name, p);
        putint(p, game::player1->colour);
        putint(p, game::player1->model);
        sendstring(game::player1->vanity, p);
        mkstring(hash);
        if(connectpass[0])
        {
            server::hashpassword(game::player1->clientnum, sessionid, connectpass, hash);
            memset(connectpass, 0, sizeof(connectpass));
        }
        sendstring(hash, p);
        sendstring(authconnect ? accountname : "", p);
        sendclientpacket(p.finalize(), 1);
    }

    static void sendposition(gameent *d, packetbuf &q)
    {
        putint(q, N_POS);
        putuint(q, d->clientnum);
        // 3 bits phys state, 2 bits move, 2 bits strafe, 2 bits turnside
        uchar physstate = d->physstate | ((d->move&3)<<3) | ((d->strafe&3)<<5) | ((d->turnside&3)<<7);
        q.put(physstate);
        putuint(q, d->impulse[IM_METER]);
        ivec o = ivec(vec(d->o.x, d->o.y, d->o.z-d->height).mul(DMF));
        uint vel = min(int(d->vel.magnitude()*DVELF), 0xFFFF), fall = min(int(d->falling.magnitude()*DVELF), 0xFFFF);
        // 3 bits position, 1 bit velocity, 3 bits falling, 1 bit aim, 1 bit crouching, 1 bit conopen
        uint flags = 0;
        if(o.x < 0 || o.x > 0xFFFF) flags |= 1<<0;
        if(o.y < 0 || o.y > 0xFFFF) flags |= 1<<1;
        if(o.z < 0 || o.z > 0xFFFF) flags |= 1<<2;
        if(vel > 0xFF) flags |= 1<<3;
        if(fall > 0)
        {
            flags |= 1<<4;
            if(fall > 0xFF) flags |= 1<<5;
            if(d->falling.x || d->falling.y || d->falling.z > 0) flags |= 1<<6;
        }
        if(d->conopen) flags |= 1<<8;
        if(d->action[AC_JUMP]) flags |= 1<<9;
        if(d->action[AC_SPRINT] == (d!=game::player1 || physics::sprintstyle < 3)) flags |= 1<<10;
        if(d->action[AC_CROUCH]) flags |= 1<<11;
        if(d->action[AC_SPECIAL]) flags |= 1<<12;
        putuint(q, flags);
        loopk(3)
        {
            q.put(o[k]&0xFF);
            q.put((o[k]>>8)&0xFF);
            if(o[k] < 0 || o[k] > 0xFFFF) q.put((o[k]>>16)&0xFF);
        }
        float yaw = d->yaw, pitch = d->pitch;
        if(d == game::player1 && game::thirdpersonview(true, d))
        {
            vectoyawpitch(vec(worldpos).sub(d->headpos()).normalize(), yaw, pitch);
            game::fixrange(yaw, pitch);
        }
        uint dir = (yaw < 0 ? 360 + int(yaw)%360 : int(yaw)%360) + clamp(int(pitch+90), 0, 180)*360;
        q.put(dir&0xFF);
        q.put((dir>>8)&0xFF);
        q.put(clamp(int(d->roll+90), 0, 180));
        q.put(vel&0xFF);
        if(vel > 0xFF) q.put((vel>>8)&0xFF);
        float velyaw, velpitch;
        vectoyawpitch(d->vel, velyaw, velpitch);
        uint veldir = (velyaw < 0 ? 360 + int(velyaw)%360 : int(velyaw)%360) + clamp(int(velpitch+90), 0, 180)*360;
        q.put(veldir&0xFF);
        q.put((veldir>>8)&0xFF);
        if(fall > 0)
        {
            q.put(fall&0xFF);
            if(fall > 0xFF) q.put((fall>>8)&0xFF);
            if(d->falling.x || d->falling.y || d->falling.z > 0)
            {
                float fallyaw, fallpitch;
                vectoyawpitch(d->falling, fallyaw, fallpitch);
                uint falldir = (fallyaw < 0 ? 360 + int(fallyaw)%360 : int(fallyaw)%360) + clamp(int(fallpitch+90), 0, 180)*360;
                q.put(falldir&0xFF);
                q.put((falldir>>8)&0xFF);
            }
        }
    }

    void sendpositions()
    {
        gameent *d = NULL;
        int numdyns = game::numdynents();
        loopi(numdyns) if((d = (gameent *)game::iterdynents(i)))
        {
            if((d == game::player1 || d->ai) && (d->state == CS_ALIVE || d->state == CS_EDITING))
            {
                packetbuf q(100);
                sendposition(d, q);
                for(int j = i+1; j < numdyns; j++)
                {
                    gameent *e = (gameent *)game::iterdynents(j);
                    if(e && (e == game::player1 || e->ai) && (e->state == CS_ALIVE || e->state == CS_EDITING))
                        sendposition(e, q);
                }
                sendclientpacket(q.finalize(), 0);
                break;
            }
        }
    }

    void sendmessages()
    {
        packetbuf p(MAXTRANS);
        if(sendplayerinfo)
        {
            p.reliable();
            sendplayerinfo = false;
            putint(p, N_SETPLAYERINFO);
            sendstring(game::player1->name, p);
            putint(p, game::player1->colour);
            putint(p, game::player1->model);
            sendstring(game::player1->vanity, p);
        }
        if(sendcrcinfo)
        {
            p.reliable();
            sendcrcinfo = false;
            putint(p, N_MAPCRC);
            sendstring(game::clientmap, p);
            putint(p, game::clientmap[0] ? getmapcrc() : 0);
        }
        if(sendgameinfo && !needsmap)
        {
            p.reliable();
            putint(p, N_GAMEINFO);
            enumerate(idents, ident, id, {
                if(id.flags&IDF_CLIENT && id.flags&IDF_WORLD) switch(id.type)
                {
                    case ID_VAR:
                        putint(p, id.type);
                        sendstring(id.name, p);
                        putint(p, *id.storage.i);
                        break;
                    case ID_FVAR:
                        putint(p, id.type);
                        sendstring(id.name, p);
                        putfloat(p, *id.storage.f);
                        break;
                    case ID_SVAR:
                        putint(p, id.type);
                        sendstring(id.name, p);
                        sendstring(*id.storage.s, p);
                        break;
                    default: break;
                }
            });
            putint(p, -1);
            entities::putitems(p);
            putint(p, -1);
            if(m_capture(game::gamemode)) capture::sendaffinity(p);
            else if(m_defend(game::gamemode)) defend::sendaffinity(p);
            else if(m_bomber(game::gamemode)) bomber::sendaffinity(p);
            sendgameinfo = false;
        }
        if(messages.length())
        {
            p.put(messages.getbuf(), messages.length());
            messages.setsize(0);
            if(messagereliable) p.reliable();
            messagereliable = false;
        }
        if(totalmillis-lastping>250)
        {
            putint(p, N_PING);
            putint(p, totalmillis);
            lastping = totalmillis;
        }

        sendclientpacket(p.finalize(), 1);
    }

    void c2sinfo(bool force) // send update to the server
    {
        static int lastupdate = -1000;
        if(totalmillis-lastupdate < 40 && !force) return;    // don't update faster than 25fps
        lastupdate = totalmillis;
        sendpositions();
        sendmessages();
        flushclient();
    }

    void parsestate(gameent *d, ucharbuf &p, bool resume = false)
    {
        if(!d) { static gameent dummy; d = &dummy; }
        if(d == game::player1 || d->ai) getint(p);
        else d->state = getint(p);
        d->points = getint(p);
        d->frags = getint(p);
        d->deaths = getint(p);
        d->health = getint(p);
        d->armour = getint(p);
        d->cptime = getint(p);
        d->cplaps = getint(p);
        if(resume && (d == game::player1 || d->ai))
        {
            d->weapreset(false);
            getint(p);
            loopi(W_MAX) getint(p);
            loopi(W_MAX) getint(p);
        }
        else
        {
            d->weapreset(true);
            int weap = getint(p);
            d->lastweap = d->weapselect = isweap(weap) ? weap : W_MELEE;
            loopi(W_MAX) d->ammo[i] = getint(p);
            loopi(W_MAX) d->reloads[i] = getint(p);
        }
    }

    void updatepos(gameent *d)
    {
        // update the position of other clients in the game in our world
        // don't care if he's in the scenery or other players,
        // just don't overlap with our client

        const float r = game::player1->radius+d->radius;
        const float dx = game::player1->o.x-d->o.x;
        const float dy = game::player1->o.y-d->o.y;
        const float dz = game::player1->o.z-d->o.z;
        const float rz = game::player1->aboveeye+game::player1->height;
        const float fx = (float)fabs(dx), fy = (float)fabs(dy), fz = (float)fabs(dz);
        if(fx<r && fy<r && fz<rz && d->state!=CS_SPECTATOR && d->state!=CS_WAITING && d->state!=CS_DEAD)
        {
            if(fx<fy) d->o.y += dy<0 ? r-fy : -(r-fy);  // push aside
            else      d->o.x += dx<0 ? r-fx : -(r-fx);
        }
        int lagtime = totalmillis-d->lastupdate;
        if(lagtime)
        {
            if(d->lastupdate) d->plag = (d->plag*5+lagtime)/6;
            d->lastupdate = totalmillis;
        }
    }

    void parsepositions(ucharbuf &p)
    {
        int type;
        while(p.remaining()) switch(type = getint(p))
        {
            case N_POS:                        // position of another client
            {
                int lcn = getuint(p), physstate = p.get(), meter = getuint(p), flags = getuint(p);
                vec o, vel, falling;
                float yaw, pitch, roll;
                loopk(3)
                {
                    int n = p.get(); n |= p.get()<<8; if(flags&(1<<k)) { n |= p.get()<<16; if(n&0x800000) n |= -1<<24; }
                    o[k] = n/DMF;
                }
                int dir = p.get(); dir |= p.get()<<8;
                yaw = dir%360;
                pitch = clamp(dir/360, 0, 180)-90;
                roll = clamp(int(p.get()), 0, 180)-90;
                int mag = p.get(); if(flags&(1<<3)) mag |= p.get()<<8;
                dir = p.get(); dir |= p.get()<<8;
                vecfromyawpitch(dir%360, clamp(dir/360, 0, 180)-90, 1, 0, vel);
                vel.mul(mag/DVELF);
                if(flags&(1<<4))
                {
                    mag = p.get(); if(flags&(1<<5)) mag |= p.get()<<8;
                    if(flags&(1<<6))
                    {
                        dir = p.get(); dir |= p.get()<<8;
                        vecfromyawpitch(dir%360, clamp(dir/360, 0, 180)-90, 1, 0, falling);
                    }
                    else falling = vec(0, 0, -1);
                    falling.mul(mag/DVELF);
                }
                else falling = vec(0, 0, 0);
                gameent *d = game::getclient(lcn);
                if(!d || d == game::player1 || d->ai) continue;
                float oldyaw = d->yaw, oldpitch = d->pitch;
                d->yaw = yaw;
                d->pitch = pitch;
                d->roll = roll;
                d->move = (physstate>>3)&2 ? -1 : (physstate>>3)&1;
                d->strafe = (physstate>>5)&2 ? -1 : (physstate>>5)&1;
                d->turnside = (physstate>>7)&2 ? -1 : (physstate>>7)&1;
                d->impulse[IM_METER] = meter;
                d->conopen = flags&(1<<8) ? true : false;
                #define actmod(x,y) \
                { \
                    bool val = d->action[x]; \
                    d->action[x] = flags&(1<<y) ? true : false; \
                    if(val != d->action[x]) d->actiontime[x] = lastmillis; \
                }
                actmod(AC_JUMP, 9);
                actmod(AC_SPRINT, 10);
                actmod(AC_CROUCH, 11);
                actmod(AC_SPECIAL, 12);
                vec oldpos(d->o);
                d->o = o;
                d->o.z += d->height;
                d->vel = vel;
                d->falling = falling;
                d->physstate = physstate&7;
                physics::updatephysstate(d);
                updatepos(d);
                if(d->state == CS_DEAD || d->state == CS_WAITING)
                {
                    d->resetinterp();
                    d->smoothmillis = 0;
                }
                else if(d->respawned < 0)
                {
                    d->resetinterp();
                    d->smoothmillis = 0;
                    game::respawned(d, false);
                    d->respawned = lastmillis;
                }
                else if(physics::smoothmove && d->smoothmillis >= 0 && oldpos.dist(d->o) < physics::smoothdist)
                {
                    d->newpos = d->o;
                    d->newpos.z -= d->height;
                    d->newyaw = d->yaw;
                    d->newpitch = d->pitch;

                    d->o = oldpos;
                    d->yaw = oldyaw;
                    d->pitch = oldpitch;

                    oldpos.z -= d->height;
                    (d->deltapos = oldpos).sub(d->newpos);

                    d->deltayaw = oldyaw - d->newyaw;
                    if(d->deltayaw > 180) d->deltayaw -= 360;
                    else if(d->deltayaw < -180) d->deltayaw += 360;
                    d->deltapitch = oldpitch - d->newpitch;

                    d->smoothmillis = lastmillis;
                }
                else d->smoothmillis = 0;
                break;
            }

            default:
                neterr("type");
                return;
        }
    }

    void parsemessages(int cn, gameent *d, ucharbuf &p)
    {
        static char text[MAXTRANS];
        int type = -1, prevtype = -1;

        while(p.remaining())
        {
            prevtype = type;
            type = getint(p);
            if(verbose > 5) conoutf("[client] msg: %d, prev: %d", type, prevtype);
            switch(type)
            {
                case N_SERVERINIT:                 // welcome messsage from the server
                {
                    int mycn = getint(p), gver = getint(p);
                    if(gver!=GAMEVERSION)
                    {
                        conoutft(CON_MESG, "\fryou are using a different game version (you: %d, server: %d)", GAMEVERSION, gver);
                        disconnect();
                        return;
                    }
                    getstring(game::player1->hostname, p);
                    if(!game::player1->hostname[0]) copystring(game::player1->hostname, "unknown");
                    sessionid = getint(p);
                    game::player1->clientnum = mycn;
                    if(getint(p)) conoutft(CON_MESG, "\frthe server is password protected");
                    else if(verbose) conoutf("\fathe server welcomes us, yay");
                    sendintro();
                    break;
                }
                case N_WELCOME: isready = true; break;

                case N_CLIENT:
                {
                    int lcn = getint(p), len = getuint(p);
                    ucharbuf q = p.subbuf(len);
                    gameent *t = game::getclient(lcn);
                    parsemessages(lcn, t, q);
                    break;
                }

                case N_SPHY: // simple phys events
                {
                    int lcn = getint(p), st = getint(p), param = st == SPHY_POWER || st == SPHY_BUFF ? getint(p) : 0;
                    gameent *t = game::getclient(lcn);
                    if(t && (st == SPHY_EXTINGUISH || st == SPHY_BUFF || (t != game::player1 && !t->ai))) switch(st)
                    {
                        case SPHY_JUMP:
                        {
                            t->resetphys();
                            t->impulse[IM_JUMP] = lastmillis;
                            playsound(S_JUMP, t->o, t);
                            regularshape(PART_SMOKE, int(t->radius), 0x222222, 21, 20, 250, t->feetpos(), 1, 1, -10, 0, 10.f);
                            break;
                        }
                        case SPHY_BOOST: case SPHY_KICK: case SPHY_VAULT: case SPHY_SKATE: case SPHY_DASH: case SPHY_MELEE:
                        {
                            t->resetphys();
                            t->doimpulse(0, IM_T_BOOST+(st-SPHY_BOOST), lastmillis);
                            game::impulseeffect(t);
                            break;
                        }
                        case SPHY_POWER: t->setweapstate(t->weapselect, W_S_POWER, param, lastmillis); break;
                        case SPHY_EXTINGUISH:
                        {
                            t->resetburning();
                            playsound(S_EXTINGUISH, t->o, t);
                            part_create(PART_SMOKE, 500, t->feetpos(t->height/2), 0xAAAAAA, t->radius*4, 1, -10);
                            break;
                        }
                        case SPHY_BUFF: t->lastbuff = param ? lastmillis : 0; break;
                        default: break;
                    }
                    break;
                }

                case N_ANNOUNCE:
                {
                    int snd = getint(p), targ = getint(p);
                    getstring(text, p);
                    if(targ >= 0 && text[0]) game::announcef(snd, targ, NULL, false, "%s", text);
                    else game::announce(snd);
                    break;
                }

                case N_TEXT:
                {
                    int tcn = getint(p);
                    gameent *t = game::getclient(tcn);
                    int flags = getint(p);
                    getstring(text, p);
                    if(!t || isignored(t->clientnum) || isignored(t->ownernum)) break;
                    saytext(t, flags, text);
                    break;
                }

                case N_COMMAND:
                {
                    int lcn = getint(p);
                    gameent *f = lcn >= 0 ? game::getclient(lcn) : NULL;
                    getstring(text, p);
                    int alen = getint(p);
                    if(alen < 0 || alen > p.remaining()) break;
                    char *arg = newstring(alen);
                    getstring(arg, p, alen+1);
                    parsecommand(f, text, arg);
                    delete[] arg;
                    break;
                }

                case N_EXECLINK:
                {
                    int tcn = getint(p), index = getint(p);
                    gameent *t = game::getclient(tcn);
                    if(!t || !d || (t->clientnum != d->clientnum && t->ownernum != d->clientnum) || t == game::player1 || t->ai) break;
                    entities::execlink(t, index, false);
                    break;
                }

                case N_MAPCHANGE:
                {
                    getstring(text, p);
                    int getit = getint(p), mode = getint(p), muts = getint(p);
                    changemapserv(getit != 1 ? text : NULL, mode, muts, getit == 2);
                    if(needsmap) switch(getit)
                    {
                        case 0: case 2:
                        {
                            conoutft(CON_MESG, "server requested map change to %s, and we need it, so asking for it", text);
                            addmsg(N_GETMAP, "r");
                            break;
                        }
                        case 1:
                        {
                            conoutft(CON_MESG, "server is requesting the map from another client for us");
                            break;
                        }
                        default: needsmap = false; break;
                    }
                    break;
                }

                case N_GAMEINFO:
                {
                    int n;
                    while((n = getint(p)) != -1) entities::setspawn(n, getint(p));
                    sendgameinfo = false;
                    break;
                }

                case N_NEWGAME: // server requests next game
                {
                    hud::showscores(false);
                    if(!menuactive()) showgui("maps", 1);
                    if(game::intermission) hud::lastnewgame = totalmillis;
                    break;
                }

                case N_SETPLAYERINFO:
                {
                    getstring(text, p);
                    int colour = getint(p), model = getint(p);
                    string vanity;
                    getstring(vanity, p);
                    if(!d) break;
                    filtertext(text, text, true, true, true, MAXNAMELEN);
                    const char *namestr = text;
                    while(*namestr && iscubespace(*namestr)) namestr++;
                    if(!*namestr) namestr = copystring(text, "unnamed");
                    if(strcmp(d->name, namestr))
                    {
                        string oldname, newname;
                        copystring(oldname, game::colorname(d));
                        d->setinfo(namestr, colour, model, vanity);
                        copystring(newname, game::colorname(d));
                        if(showpresence >= (waiting(false) ? 2 : 1) && !isignored(d->clientnum))
                            conoutft(CON_EVENT, "\fm%s is now known as %s", oldname, newname);
                    }
                    else d->setinfo(namestr, colour, model, vanity);
                    break;
                }

                case N_CLIENTINIT: // another client either connected or changed name/team
                {
                    int cn = getint(p);
                    gameent *d = game::newclient(cn);
                    if(!d)
                    {
                        loopi(2) getstring(text, p);
                        loopi(2) getint(p);
                        getstring(text, p);
                        getint(p);
                        break;
                    }
                    getstring(d->hostname, p);
                    if(!d->hostname[0]) copystring(d->hostname, "unknown");
                    getstring(text, p);
                    int colour = getint(p), model = getint(p);
                    string vanity;
                    getstring(vanity, p);
                    filtertext(text, text, true, true, true, MAXNAMELEN);
                    const char *namestr = text;
                    while(*namestr && iscubespace(*namestr)) namestr++;
                    if(!*namestr) namestr = copystring(text, "unnamed");
                    if(d->name[0])        // already connected
                    {
                        if(strcmp(d->name, namestr))
                        {
                            string oldname;
                            copystring(oldname, game::colorname(d, NULL, "", false));
                            d->setinfo(namestr, colour, model, vanity);
                            if(showpresence >= (waiting(false) ? 2 : 1) && !isignored(d->clientnum))
                                conoutft(CON_EVENT, "\fm%s (%s) is now known as %s", oldname, d->hostname, game::colorname(d));
                        }
                        else d->setinfo(namestr, colour, model, vanity);
                    }
                    else // new client
                    {
                        d->setinfo(namestr, colour, model, vanity);
                        if(showpresence >= (waiting(false) ? 2 : 1)) conoutft(CON_EVENT, "\fg%s (%s) has joined the game", game::colorname(d), d->hostname);
                        if(needclipboard >= 0) needclipboard++;
                        game::specreset(d);
                    }
                    int team = clamp(getint(p), int(T_NEUTRAL), int(T_ENEMY));
                    if(d == game::focus && d->team != team) hud::lastteam = 0;
                    d->team = team;
                    break;
                }

                case N_DISCONNECT:
                {
                    int lcn = getint(p), reason = getint(p);
                    game::clientdisconnected(lcn, reason);
                    break;
                }

                case N_LOADW:
                {
                    hud::showscores(false);
                    game::player1->loadweap.shrink(0);
                    if(!menuactive()) showgui("loadout", -1);
                    break;
                }

                case N_SPAWN:
                {
                    int lcn = getint(p);
                    gameent *f = game::newclient(lcn);
                    if(!f || f == game::player1 || f->ai)
                    {
                        parsestate(NULL, p);
                        break;
                    }
                    f->respawn(lastmillis);
                    parsestate(f, p);
                    break;
                }

                case N_SPAWNSTATE:
                {
                    int lcn = getint(p), ent = getint(p);
                    gameent *f = game::newclient(lcn);
                    if(!f || (f != game::player1 && !f->ai))
                    {
                        parsestate(NULL, p);
                        break;
                    }
                    if(f == game::player1 && editmode) toggleedit();
                    f->respawn(lastmillis);
                    parsestate(f, p);
                    game::respawned(f, true, ent);
                    break;
                }

                case N_SHOTFX:
                {
                    int scn = getint(p), weap = getint(p), flags = getint(p), len = getint(p);
                    vec from;
                    loopk(3) from[k] = getint(p)/DMF;
                    int ls = getint(p);
                    vector<shotmsg> shots;
                    loopj(ls)
                    {
                        shotmsg &s = shots.add();
                        s.id = getint(p);
                        loopk(3) s.pos[k] = getint(p);
                    }
                    gameent *t = game::getclient(scn);
                    if(!t || !isweap(weap) || t == game::player1 || t->ai) break;
                    if(weap != t->weapselect && weap != W_MELEE) t->weapswitch(weap, lastmillis);
                    float scale = 1;
                    int sub = W2(weap, sub, WS(flags));
                    if(W2(weap, power, WS(flags)))
                    {
                        scale = len/float(W2(weap, power, WS(flags)));
                        if(sub > 1) sub = int(ceilf(sub*scale));
                    }
                    projs::shootv(weap, flags, sub, scale, from, shots, t, false);
                    break;
                }

                case N_DESTROY:
                {
                    int scn = getint(p), num = getint(p);
                    gameent *t = game::getclient(scn);
                    loopi(num)
                    {
                        int id = getint(p);
                        if(t) projs::destruct(t, id);
                    }
                    break;
                }

                case N_STICKY:
                {
                    int scn = getint(p), tcn = getint(p), id = getint(p);
                    vec norm(0, 0, 0), pos(0, 0, 0);
                    loopk(3) norm[k] = getint(p)/DNF;
                    loopk(3) pos[k] = getint(p)/DMF;
                    gameent *t = game::getclient(scn), *v = tcn >= 0 ? game::getclient(tcn) : NULL;
                    if(t && (tcn < 0 || v)) projs::sticky(t, id, norm, pos, v);
                    break;
                }

                case N_DAMAGE:
                {
                    int tcn = getint(p), acn = getint(p), weap = getint(p), flags = getint(p), damage = getint(p),
                        health = getint(p), armour = getint(p);
                    vec dir;
                    loopk(3) dir[k] = getint(p)/DNF;
                    dir.normalize();
                    gameent *target = game::getclient(tcn), *actor = game::getclient(acn);
                    if(!target || !actor) break;
                    game::damaged(weap, flags, damage, health, armour, target, actor, lastmillis, dir);
                    break;
                }

                case N_RELOAD:
                {
                    int trg = getint(p), weap = getint(p), amt = getint(p), ammo = getint(p), reloads = getint(p);
                    gameent *target = game::getclient(trg);
                    if(!target || !isweap(weap)) break;
                    weapons::weapreload(target, weap, amt, ammo, reloads, false);
                    break;
                }

                case N_REGEN:
                {
                    int trg = getint(p), heal = getint(p), amt = getint(p), armour = getint(p);
                    gameent *f = game::getclient(trg);
                    if(!f) break;
                    if(!amt)
                    {
                        f->impulse[IM_METER] = 0;
                        f->resetresidual();
                    }
                    else if(amt > 0 && (!f->lastregen || lastmillis-f->lastregen >= 500)) playsound(S_REGEN, f->o, f);
                    f->health = heal;
                    f->armour = armour;
                    f->lastregen = lastmillis;
                    break;
                }

                case N_DIED:
                {
                    int vcn = getint(p), deaths = getint(p), acn = getint(p), frags = getint(p), spree = getint(p), style = getint(p), weap = getint(p), flags = getint(p), damage = getint(p);
                    gameent *victim = game::getclient(vcn), *actor = game::getclient(acn);
                    static vector<gameent *> assist; assist.setsize(0);
                    int count = getint(p);
                    loopi(count)
                    {
                        int lcn = getint(p);
                        gameent *log = game::getclient(lcn);
                        if(log) assist.add(log);
                    }
                    if(!actor || !victim) break;
                    victim->deaths = deaths;
                    actor->frags = frags;
                    actor->spree = spree;
                    game::killed(weap, flags, damage, victim, actor, assist, style);
                    victim->lastdeath = lastmillis;
                    victim->weapreset(true);
                    break;
                }

                case N_POINTS:
                {
                    int acn = getint(p), add = getint(p), points = getint(p);
                    gameent *actor = game::getclient(acn);
                    if(!actor) break;
                    actor->lastpoints = add;
                    actor->points = points;
                    break;
                }

                case N_DROP:
                {
                    int trg = getint(p), weap = getint(p), ds = getint(p);
                    gameent *target = game::getclient(trg);
                    bool local = target && (target == game::player1 || target->ai);
                    if(ds) loopj(ds)
                    {
                        int gs = getint(p), drop = getint(p), ammo = getint(p), reloads = getint(p);
                        if(target) projs::drop(target, gs, drop, ammo, reloads, local, j, weap);
                    }
                    if(isweap(weap) && target)
                    {
                        target->weapswitch(weap, lastmillis, weaponswitchdelay);
                        playsound(WSND(weap, S_W_SWITCH), target->o, target, 0, -1, -1, -1, &target->wschan);
                    }
                    break;
                }

                case N_WSELECT:
                {
                    int trg = getint(p), weap = getint(p);
                    gameent *target = game::getclient(trg);
                    if(!target || !isweap(weap)) break;
                    weapons::weapselect(target, weap, G(weaponinterrupts), false);
                    break;
                }

                case N_RESUME:
                {
                    for(;;)
                    {
                        int lcn = getint(p);
                        if(p.overread() || lcn < 0) break;
                        gameent *f = game::newclient(lcn);
                        if(f && f!=game::player1 && !f->ai) f->respawn();
                        parsestate(f, p, true);
                        f->setscale(game::rescale(f), 0, true, game::gamemode, game::mutators);
                    }
                    break;
                }

                case N_ITEMSPAWN:
                {
                    int ent = getint(p), value = getint(p);
                    if(!entities::ents.inrange(ent)) break;
                    gameentity &e = *(gameentity *)entities::ents[ent];
                    entities::setspawn(ent, value);
                    ai::itemspawned(ent, value!=0);
                    if(e.spawned)
                    {
                        int sweap = m_weapon(game::gamemode, game::mutators), attr = e.type == WEAPON ? w_attr(game::gamemode, game::mutators, e.attrs[0], sweap) : e.attrs[0],
                            colour = e.type == WEAPON ? W(attr, colour) : 0xFFFFFF;
                        playsound(e.type == WEAPON && attr >= W_OFFSET ? WSND(attr, S_W_SPAWN) : S_ITEMSPAWN, e.o);
                        if(entities::showentdescs)
                        {
                            vec pos = vec(e.o).add(vec(0, 0, 4));
                            const char *texname = entities::showentdescs >= 2 ? hud::itemtex(e.type, attr) : NULL;
                            if(texname && *texname) part_icon(pos, textureload(texname, 3), game::aboveitemiconsize, 1, -10, 0, game::eventiconfade, colour);
                            else
                            {
                                const char *item = entities::entinfo(e.type, e.attrs, 0);
                                if(item && *item)
                                {
                                    defformatstring(ds)("<emphasis>%s", item);
                                    part_textcopy(pos, ds, PART_TEXT, game::eventiconfade, 0xFFFFFF, 2, 1, -10);
                                }
                            }
                        }
                        regularshape(PART_SPARK, enttype[e.type].radius*1.5f, colour, 53, 50, 350, e.o, 1.5f, 1, 1, 0, 35);
                        if(game::dynlighteffects)
                            adddynlight(e.o, enttype[e.type].radius*2, vec::hexcolor(colour).mul(2.f), 250, 250);
                    }
                    break;
                }

                case N_TRIGGER:
                {
                    int ent = getint(p), st = getint(p);
                    entities::setspawn(ent, st);
                    break;
                }

                case N_ITEMACC:
                { // uses a specific drop so the client knows what to replace
                    int lcn = getint(p), ent = getint(p), ammoamt = getint(p), reloadamt = getint(p), spawn = getint(p),
                        weap = getint(p), drop = getint(p), ammo = getint(p), reloads = getint(p);
                    gameent *target = game::getclient(lcn);
                    if(!target) break;
                    if(entities::ents.inrange(ent) && enttype[entities::ents[ent]->type].usetype == EU_ITEM)
                        entities::useeffects(target, ent, ammoamt, reloadamt, spawn, weap, drop, ammo, reloads);
                    break;
                }

                case N_EDITVAR:
                {
                    int t = getint(p);
                    bool commit = true;
                    getstring(text, p);
                    ident *id = idents.access(text);
                    if(!d || !id || !(id->flags&IDF_WORLD) || id->type != t) commit = false;
                    switch(t)
                    {
                        case ID_VAR:
                        {
                            int val = getint(p);
                            if(commit)
                            {
                                if(val > id->maxval) val = id->maxval;
                                else if(val < id->minval) val = id->minval;
                                setvar(text, val, true);
                                conoutft(CON_EVENT, "\fc%s set worldvar %s to %s", game::colorname(d), id->name, intstr(id));
                            }
                            break;
                        }
                        case ID_FVAR:
                        {
                            float val = getfloat(p);
                            if(commit)
                            {
                                if(val > id->maxvalf) val = id->maxvalf;
                                else if(val < id->minvalf) val = id->minvalf;
                                setfvar(text, val, true);
                                conoutft(CON_EVENT, "\fc%s set worldvar %s to %s", game::colorname(d), id->name, floatstr(*id->storage.f));
                            }
                            break;
                        }
                        case ID_SVAR:
                        {
                            int vlen = getint(p);
                            if(vlen < 0 || vlen > p.remaining()) break;
                            char *val = newstring(vlen);
                            getstring(val, p, vlen+1);
                            if(commit)
                            {
                                setsvar(text, val, true);
                                conoutft(CON_EVENT, "\fc%s set worldvar %s to %s", game::colorname(d), id->name, *id->storage.s);
                            }
                            delete[] val;
                            break;
                        }
                        case ID_ALIAS:
                        {
                            int vlen = getint(p);
                            if(vlen < 0 || vlen > p.remaining()) break;
                            char *val = newstring(vlen);
                            getstring(val, p, vlen+1);
                            if(commit || !id) // set aliases anyway
                            {
                                worldalias(text, val);
                                conoutft(CON_EVENT, "\fc%s set worldalias %s to %s", game::colorname(d), text, val);
                            }
                            delete[] val;
                            break;
                        }
                        default: break;
                    }
                    break;
                }

                case N_CLIPBOARD:
                {
                    int cn = getint(p), unpacklen = getint(p), packlen = getint(p);
                    gameent *d = game::getclient(cn);
                    ucharbuf q = p.subbuf(max(packlen, 0));
                    if(d) unpackeditinfo(d->edit, q.buf, q.maxlen, unpacklen);
                    break;
                }

                case N_EDITF:            // coop editing messages
                case N_EDITT:
                case N_EDITM:
                case N_FLIP:
                case N_COPY:
                case N_PASTE:
                case N_ROTATE:
                case N_REPLACE:
                case N_DELCUBE:
                {
                    if(!d) return;
                    selinfo s;
                    s.o.x = getint(p); s.o.y = getint(p); s.o.z = getint(p);
                    s.s.x = getint(p); s.s.y = getint(p); s.s.z = getint(p);
                    s.grid = getint(p); s.orient = getint(p);
                    s.cx = getint(p); s.cxs = getint(p); s.cy = getint(p), s.cys = getint(p);
                    s.corner = getint(p);
                    int dir, mode, tex, newtex, mat, allfaces;
                    ivec moveo;
                    switch(type)
                    {
                        case N_EDITF: dir = getint(p); mode = getint(p); if(s.validate()) mpeditface(dir, mode, s, false); break;
                        case N_EDITT: tex = getint(p); allfaces = getint(p); if(s.validate()) mpedittex(tex, allfaces, s, false); break;
                        case N_EDITM: mat = getint(p); mode = getint(p); allfaces = getint(p); if(s.validate()) mpeditmat(mat, mode, allfaces, s, false); break;
                        case N_FLIP: if(s.validate()) mpflip(s, false); break;
                        case N_COPY: if(d && s.validate()) mpcopy(d->edit, s, false); break;
                        case N_PASTE: if(d && s.validate()) mppaste(d->edit, s, false); break;
                        case N_ROTATE: dir = getint(p); if(s.validate()) mprotate(dir, s, false); break;
                        case N_REPLACE: tex = getint(p); newtex = getint(p); allfaces = getint(p); if(s.validate()) mpreplacetex(tex, newtex, allfaces!=0, s, false); break;
                        case N_DELCUBE: if(s.validate()) mpdelcube(s, false); break;
                    }
                    break;
                }
                case N_REMIP:
                {
                    if(!d) return;
                    conoutft(CON_MESG, "%s remipped", game::colorname(d));
                    mpremip(false);
                    break;
                }
                case N_EDITENT:            // coop edit of ent
                {
                    if(!d) return;
                    int i = getint(p);
                    float x = getint(p)/DMF, y = getint(p)/DMF, z = getint(p)/DMF;
                    int type = getint(p), numattrs = getint(p);
                    attrvector attrs;
                    attrs.add(0, max(entities::numattrs(type), min(numattrs, MAXENTATTRS)));
                    loopk(numattrs)
                    {
                        int val = getint(p);
                        if(attrs.inrange(k)) attrs[k] = val;
                    }
                    mpeditent(i, vec(x, y, z), type, attrs, false);
                    entities::setspawn(i, 0);
                    break;
                }

                case N_EDITLINK:
                {
                    if(!d) return;
                    int b = getint(p), index = getint(p), node = getint(p);
                    entities::linkents(index, node, b!=0, false, false);
                }

                case N_PONG:
                    addmsg(N_CLIENTPING, "i", game::player1->ping = (game::player1->ping*5+totalmillis-getint(p))/6);
                    break;

                case N_CLIENTPING:
                    if(!d) return;
                    d->ping = getint(p);
                    loopv(game::players) if(game::players[i] && game::players[i]->ownernum == d->clientnum)
                        game::players[i]->ping = d->ping;
                    break;

                case N_TICK:
                    game::timeupdate(getint(p));
                    break;

                case N_SERVMSG:
                {
                    int lev = getint(p);
                    getstring(text, p);
                    conoutft(lev >= 0 && lev < CON_MAX ? lev : CON_INFO, "%s", text);
                    break;
                }

                case N_SENDDEMOLIST:
                {
                    int demos = getint(p);
                    if(demos <= 0) conoutft(CON_MESG, "\frno demos available");
                    else loopi(demos)
                    {
                        getstring(text, p);
                        if(p.overread()) break;
                        conoutft(CON_MESG, "\fa%d. %s", i+1, text);
                    }
                    break;
                }

                case N_DEMOPLAYBACK:
                {
                    int on = getint(p);
                    if(on) game::player1->state = CS_SPECTATOR;
                    else
                    {
                        loopv(game::players) if(game::players[i]) game::clientdisconnected(i);
                    }
                    demoplayback = on!=0;
                    game::player1->clientnum = getint(p);
                    break;
                }

                case N_CURRENTPRIV:
                {
                    int mn = getint(p), priv = getint(p);
                    getstring(text, p);
                    if(mn >= 0)
                    {
                        gameent *m = game::getclient(mn);
                        if(m)
                        {
                            m->privilege = priv;
                            copystring(m->handle, text);
                        }
                    }
                    break;
                }

                case N_EDITMODE:
                {
                    int val = getint(p);
                    if(!d) break;
                    if(val) d->state = CS_EDITING;
                    else
                    {
                        d->state = CS_ALIVE;
                        d->editspawn(game::gamemode, game::mutators);
                    }
                    d->resetinterp();
                    projs::remove(d);
                    break;
                }

                case N_SPECTATOR:
                {
                    int sn = getint(p), val = getint(p);
                    gameent *s = game::newclient(sn);
                    if(!s) break;
                    if(s == game::player1) game::resetfollow();
                    if(val)
                    {
                        if(s == game::player1 && editmode) toggleedit();
                        s->state = CS_SPECTATOR;
                        s->checkpoint = -1;
                        s->cpmillis = 0;
                    }
                    else if(s->state == CS_SPECTATOR)
                    {
                        s->state = CS_WAITING;
                        s->checkpoint = -1;
                        s->cpmillis = 0;
                        if(s != game::player1 && !s->ai) s->resetinterp();
                        game::waiting.removeobj(s);
                    }
                    break;
                }

                case N_WAITING:
                {
                    int sn = getint(p);
                    gameent *s = game::newclient(sn);
                    if(!s) break;
                    if(s == game::player1)
                    {
                        if(editmode) toggleedit();
                        hud::showscores(false);
                        s->stopmoving(true);
                        game::waiting.setsize(0);
                        gameent *d;
                        loopv(game::players) if((d = game::players[i]) && d->aitype == AI_NONE && d->state == CS_WAITING)
                            game::waiting.add(d);
                    }
                    else if(!s->ai) s->resetinterp();
                    game::waiting.removeobj(s);
                    if(s->state == CS_ALIVE) s->lastdeath = lastmillis; // so spawndelay shows properly
                    s->state = CS_WAITING;
                    s->weapreset(true);
                    break;
                }

                case N_SETTEAM:
                {
                    int wn = getint(p), tn = getint(p);
                    gameent *w = game::getclient(wn);
                    if(!w) return;
                    if(w->team != tn)
                    {
                        if(m_isteam(game::gamemode, game::mutators) && w->aitype == AI_NONE && showteamchange >= (w->team != T_NEUTRAL && tn != T_NEUTRAL ? 1 : 2))
                            conoutft(CON_EVENT, "\fa%s is now on team \fs\f[%d]\f(%s)%s", game::colorname(w), TEAM(tn, colour), hud::teamtexname(tn), TEAM(tn, name));
                        w->team = tn;
                        if(w == game::focus) hud::lastteam = 0;
                    }
                    break;
                }

                case N_INFOAFFIN:
                {
                    int flag = getint(p), converted = getint(p),
                            owner = getint(p), enemy = getint(p);
                    if(m_defend(game::gamemode)) defend::updateaffinity(flag, owner, enemy, converted);
                    break;
                }

                case N_SETUPAFFIN:
                {
                    if(m_defend(game::gamemode)) defend::parseaffinity(p);
                    break;
                }

                case N_MAPVOTE:
                {
                    int vn = getint(p);
                    gameent *v = game::getclient(vn);
                    getstring(text, p);
                    filtertext(text, text);
                    int reqmode = getint(p), reqmuts = getint(p);
                    if(!v) break;
                    vote(v, text, reqmode, reqmuts);
                    break;
                }

                case N_CLEARVOTE:
                {
                    int vn = getint(p);
                    gameent *v = game::getclient(vn);
                    if(!v) break;
                    clearvotes(v, true);
                    break;
                }

                case N_CHECKPOINT:
                {
                    int tn = getint(p), ent = getint(p);
                    gameent *t = game::getclient(tn);
                    if(!t) break;
                    if(ent >= 0)
                    {
                        int laptime = getint(p);
                        if(laptime >= 0)
                        {
                            t->cplast = laptime;
                            t->cptime = getint(p);
                            t->cplaps = getint(p);
                            t->cpmillis = t->impulse[IM_METER] = 0;
                            if(showlaptimes > (t != game::focus ? (t->aitype > AI_NONE ? 2 : 1) : 0))
                            {
                                defformatstring(best)("%s", hud::timetostr(t->cptime));
                                conoutft(t != game::player1 ? CON_INFO : CON_SELF, "%s completed in \fs\fg%s\fS (best: \fs\fy%s\fS, laps: \fs\fc%d\fS)", game::colorname(t), hud::timetostr(t->cplast), best, t->cplaps);
                            }
                        }
                        if(entities::ents.inrange(ent) && entities::ents[ent]->type == CHECKPOINT)
                            entities::execlink(t, ent, false);
                    }
                    else
                    {
                        t->checkpoint = -1;
                        t->cpmillis = 0;
                    }
                }

                case N_SCORE:
                {
                    int team = getint(p), total = getint(p);
                    if(m_isteam(game::gamemode, game::mutators))
                    {
                        score &ts = hud::teamscore(team);
                        ts.total = total;
                    }
                    break;
                }

                case N_INITAFFIN:
                {
                    if(m_capture(game::gamemode)) capture::parseaffinity(p);
                    else if(m_bomber(game::gamemode)) bomber::parseaffinity(p);
                    break;
                }

                case N_DROPAFFIN:
                {
                    int ocn = getint(p), tcn = getint(p), flag = getint(p);
                    vec droploc, inertia;
                    loopk(3) droploc[k] = getint(p)/DMF;
                    loopk(3) inertia[k] = getint(p)/DMF;
                    gameent *o = game::newclient(ocn);
                    if(o)
                    {
                        if(m_capture(game::gamemode)) capture::dropaffinity(o, flag, droploc, inertia, tcn);
                        else if(m_bomber(game::gamemode)) bomber::dropaffinity(o, flag, droploc, inertia, tcn);
                    }
                    break;
                }

                case N_SCOREAFFIN:
                {
                    int ocn = getint(p), relayflag = getint(p), goalflag = getint(p), score = getint(p);
                    gameent *o = game::newclient(ocn);
                    if(o)
                    {
                        if(m_capture(game::gamemode)) capture::scoreaffinity(o, relayflag, goalflag, score);
                        else if(m_bomber(game::gamemode)) bomber::scoreaffinity(o, relayflag, goalflag, score);
                    }
                    break;
                }

                case N_RETURNAFFIN:
                {
                    int ocn = getint(p), flag = getint(p);
                    gameent *o = game::newclient(ocn);
                    if(o && m_capture(game::gamemode)) capture::returnaffinity(o, flag);
                    break;
                }

                case N_TAKEAFFIN:
                {
                    int ocn = getint(p), flag = getint(p);
                    gameent *o = game::newclient(ocn);
                    if(o)
                    {
                        if(m_capture(game::gamemode)) capture::takeaffinity(o, flag);
                        else if(m_bomber(game::gamemode)) bomber::takeaffinity(o, flag);
                    }
                    break;
                }

                case N_RESETAFFIN:
                {
                    int flag = getint(p), value = getint(p);
                    if(m_capture(game::gamemode)) capture::resetaffinity(flag, value);
                    else if(m_bomber(game::gamemode)) bomber::resetaffinity(flag, value);
                    break;
                }

                case N_GETMAP:
                {
                    conoutft(CON_MESG, "server has requested we send the map..");
                    if(!needsmap && !gettingmap) sendmap();
                    else
                    {
                        if(!gettingmap) conoutft(CON_MESG, "..we don't have the map though, so asking for it instead");
                        else conoutft(CON_MESG, "..but we're in the process of getting it");
                        addmsg(N_GETMAP, "r");
                    }
                    break;
                }

                case N_SENDMAP:
                {
                    conoutft(CON_MESG, "map data has been uploaded");
                    if(needsmap && !gettingmap)
                    {
                        conoutft(CON_MESG, "\fowe want the map too, so asking for it");
                        addmsg(N_GETMAP, "r");
                    }
                    break;
                }

                case N_FAILMAP:
                {
                    if(needsmap) conoutft(CON_MESG, "everybody failed to get the map..");
                    needsmap = gettingmap = false;
                    break;
                }

                case N_NEWMAP:
                {
                    int size = getint(p);
                    if(size>=0) emptymap(size, true);
                    else enlargemap(true);
                    mapvotes.shrink(0);
                    needsmap = false;
                    if(d && d!=game::player1)
                    {
                        int newsize = 0;
                        while(1<<newsize < getworldsize()) newsize++;
                        conoutft(CON_MESG, size>=0 ? "%s started a new map of size %d" : "%s enlarged the map to size %d", game::colorname(d), newsize);
                    }
                    break;
                }

                case N_INITAI:
                {
                    int bn = getint(p), on = getint(p), at = getint(p), et = getint(p), sk = clamp(getint(p), 1, 101);
                    getstring(text, p);
                    int tm = getint(p), cl = getint(p), md = getint(p);
                    string vanity;
                    getstring(vanity, p);
                    gameent *b = game::newclient(bn);
                    if(!b) break;
                    ai::init(b, at, et, on, sk, bn, text, tm, cl, md, vanity);
                    break;
                }

                case N_AUTHCHAL:
                {
                    uint id = (uint)getint(p);
                    getstring(text, p);
                    if(accountname[0] && accountpass[0])
                    {
                        conoutft(CON_MESG, "aswering account challenge..");
                        vector<char> buf;
                        answerchallenge(accountpass, text, buf);
                        addmsg(N_AUTHANS, "ris", id, buf.getbuf());
                    }
                    break;
                }

                default:
                {
                    defformatstring(err)("type (got %d)", type);
                    neterr(err);
                    return;
                }
            }
        }
    }

    int serverstat(serverinfo *a)
    {
        if(a->attr.length() > 4 && a->numplayers >= a->attr[4])
        {
            return SSTAT_FULL;
        }
        else if(a->attr.length() > 5) switch(a->attr[5])
        {
            case MM_LOCKED:
            {
                return SSTAT_LOCKED;
            }
            case MM_PRIVATE:
            case MM_PASSWORD:
            {
                return SSTAT_PRIVATE;
            }
            default:
            {
                return SSTAT_OPEN;
            }
        }
        return SSTAT_UNKNOWN;
    }

    int servercompare(serverinfo *a, serverinfo *b)
    {
        int ac = 0, bc = 0;
        if(a->address.host == ENET_HOST_ANY || a->ping >= serverinfo::WAITING || a->attr.empty()) ac = -1;
        else
        {
            ac = a->attr[0] == GAMEVERSION ? 0x7FFF : clamp(a->attr[0], 0, 0x7FFF-1);
            ac <<= 16;
            if(a->address.host == masteraddress.host) ac |= 0xFFFF;
            else ac |= clamp(1 + a->priority, 1, 0xFFFF-1);
        }
        if(b->address.host == ENET_HOST_ANY || b->ping >= serverinfo::WAITING || b->attr.empty()) bc = -1;
        else
        {
            bc = b->attr[0] == GAMEVERSION ? 0x7FFF : clamp(b->attr[0], 0, 0x7FFF-1);
            bc <<= 16;
            if(b->address.host == masteraddress.host) bc |= 0xFFFF;
            else bc |= clamp(1 + b->priority, 1, 0xFFFF-1);
        }
        if(ac > bc) return -1;
        if(ac < bc) return 1;

        #define retcp(c) { int cv = (c); if(cv) { return reverse ? -cv : cv; } }
        #define retsw(c,d,e) { \
            int cv = (c), dv = (d); \
            if(cv != dv) \
            { \
                if((e) != reverse ? cv < dv : cv > dv) return -1; \
                if((e) != reverse ? cv > dv : cv < dv) return 1; \
            } \
        }

        if(serversortstyles.empty()) updateserversort();
        loopv(serversortstyles)
        {
            int style = serversortstyles[i];
            serverinfo *aa = a, *ab = b;
            bool reverse = false;
            if(style < 0)
            {
                style = 0-style;
                reverse = true;
            }

            switch(style)
            {
                case SINFO_STATUS:
                {
                    retsw(serverstat(aa), serverstat(ab), true);
                    break;
                }
                case SINFO_DESC:
                {
                    retcp(strcmp(aa->sdesc, ab->sdesc));
                    break;
                }
                case SINFO_MODE:
                {
                    if(aa->attr.length() > 1) ac = aa->attr[1];
                    else ac = 0;

                    if(ab->attr.length() > 1) bc = ab->attr[1];
                    else bc = 0;

                    retsw(ac, bc, true);
                }
                case SINFO_MUTS:
                {
                    if(aa->attr.length() > 2) ac = aa->attr[2];
                    else ac = 0;

                    if(ab->attr.length() > 2) bc = ab->attr[2];
                    else bc = 0;

                    retsw(ac, bc, true);
                    break;
                }
                case SINFO_MAP:
                {
                    retcp(strcmp(aa->map, ab->map));
                    break;
                }
                case SINFO_TIME:
                {
                    if(aa->attr.length() > 3) ac = aa->attr[3];
                    else ac = 0;

                    if(ab->attr.length() > 3) bc = ab->attr[3];
                    else bc = 0;

                    retsw(ac, bc, false);
                    break;
                }
                case SINFO_NUMPLRS:
                {
                    retsw(aa->numplayers, ab->numplayers, false);
                    break;
                }
                case SINFO_MAXPLRS:
                {
                    if(aa->attr.length() > 4) ac = aa->attr[4];
                    else ac = 0;

                    if(ab->attr.length() > 4) bc = ab->attr[4];
                    else bc = 0;

                    retsw(ac, bc, false);
                    break;
                }
                case SINFO_PING:
                {
                    retsw(aa->ping, ab->ping, true);
                    break;
                }
                default: break;
            }
        }
        return strcmp(a->name, b->name);
    }

    void parsepacketclient(int chan, packetbuf &p)  // processes any updates from the server
    {
        if(p.packet->flags&ENET_PACKET_FLAG_UNSEQUENCED) return;
        switch(chan)
        {
            case 0:
                parsepositions(p);
                break;

            case 1:
                parsemessages(-1, NULL, p);
                break;

            case 2:
                receivefile(p.buf, p.maxlen);
                break;
        }
    }

    void getservers(int server, int prop, int idx)
    {
        if(server < 0) intret(servers.length());
        else if(servers.inrange(server))
        {
            serverinfo *si = servers[server];
            if(prop < 0) intret(3);
            else switch(prop)
            {
                case 0:
                    if(idx < 0) intret(8);
                    else switch(idx)
                    {
                        case 0: intret(serverstat(si)); break;
                        case 1: result(si->name); break;
                        case 2: intret(si->port); break;
                        case 3: result(si->sdesc[0] ? si->sdesc : si->name); break;
                        case 4: result(si->map); break;
                        case 5: intret(si->numplayers); break;
                        case 6: intret(si->ping); break;
                        case 7: intret(si->lastinfo); break;
                    }
                    break;
                case 1:
                    if(idx < 0) intret(si->attr.length());
                    else if(si->attr.inrange(idx)) intret(si->attr[idx]);
                    break;
                case 2:
                    if(idx < 0) intret(si->players.length());
                    else if(si->players.inrange(idx)) result(si->players[idx]);
                    break;
            }
        }
    }
    ICOMMAND(0, getserver, "bbb", (int *server, int *prop, int *idx, int *numargs), getservers(*server, *prop, *idx));
}
