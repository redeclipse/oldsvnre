#ifdef GAMESERVER
struct duelservmode : servmode
{
    int duelround, dueltime, duelcheck, dueldeath, duelwinner, duelwins;
    vector<clientinfo *> duelqueue, allowed, playing;

    duelservmode() {}

    void position(clientinfo *ci)
    {
        if(allowbroadcast(ci->clientnum) && ci->state.aitype < AI_START)
        {
            int n = duelqueue.find(ci);
            if(n >= 0)
            {
                if(m_survivor(gamemode, mutators))
                    srvmsgft(ci->clientnum, CON_EVENT, "\fyyou are now \fs\fzgyqueued\fS for your \fs\fgnext match\fS");
                else
                {
                    if(n) srvmsgft(ci->clientnum, CON_EVENT, "\fyyou are \fs\fzcy#%d\fS in the \fs\fgduel queue\fS", n+1);
                    else srvmsgft(ci->clientnum, CON_EVENT, "\fyyou are \fs\fzcrNEXT\fS in the \fs\fgduel queue\fS");
                }
            }
        }
    }

    void queue(clientinfo *ci, bool top = false, bool wait = true, bool clean = false)
    {
        if(ci->online && ci->state.state != CS_SPECTATOR && ci->state.state != CS_EDITING && ci->state.aitype < AI_START)
        {
            int n = duelqueue.find(ci);
            if(top)
            {
                if(n >= 0) duelqueue.remove(n);
                duelqueue.insert(0, ci);
            }
            else if(n < 0) duelqueue.add(ci);
            if(wait && ci->state.state != CS_WAITING) waiting(ci, 0, DROP_RESET);
            if(!clean) position(ci);
        }
    }

    void entergame(clientinfo *ci) { queue(ci); }
    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(duelwinner == ci->clientnum)
        {
            duelwinner = -1;
            duelwins = 0;
        }
        duelqueue.removeobj(ci);
        allowed.removeobj(ci);
        playing.removeobj(ci);
    }

    bool damage(clientinfo *target, clientinfo *actor, int damage, int weap, int flags, const ivec &hitpush)
    {
        if(dueltime >= 0 && target->state.aitype < AI_START) return false;
        return true;
    }

    bool canspawn(clientinfo *ci, bool tryspawn = false)
    {
        if(allowed.find(ci) >= 0 || ci->state.aitype >= AI_START) return true;
        if(tryspawn && dueltime < 0 && dueldeath < 0) queue(ci, false, duelround > 0 || duelqueue.length() > 1);
        return false; // you spawn when we want you to buddy
    }

    void spawned(clientinfo *ci) { allowed.removeobj(ci); }

    void died(clientinfo *ci, clientinfo *at) {}

    void layout()
    {
        loopvj(clients) if(clients[j]->state.aitype < AI_START)
        {
            vector<int> shots;
            loop(a, WEAP_MAX) loop(b, 2) loopvrev(clients[j]->state.weapshots[a][b].projs)
            {
                shots.add(clients[j]->state.weapshots[a][b].projs[i].id);
                clients[j]->state.weapshots[a][b].remove(i);
            }
            if(!shots.empty()) sendf(-1, 1, "ri2iv", N_DESTROY, clients[j]->clientnum, shots.length(), shots.length(), shots.getbuf());

        }
        if(m_survivor(gamemode, mutators) || GAME(duelclear))
            loopv(sents) if(enttype[sents[i].type].usetype == EU_ITEM) setspawn(i, hasitem(i), true, true);
    }

    void cleanup()
    {
        loopvrev(duelqueue) if(duelqueue[i]->state.state != CS_DEAD && duelqueue[i]->state.state != CS_WAITING) duelqueue.remove(i);
        loopvrev(allowed) if(allowed[i]->state.state != CS_DEAD && allowed[i]->state.state != CS_WAITING) allowed.remove(i);
    }

    void clear()
    {
        duelcheck = dueldeath = -1;
        dueltime = gamemillis+GAME(duellimit);
        playing.shrink(0);
    }

    void update()
    {
        if(interm || !hasgameinfo || numclients(-1, true, AI_BOT) <= 1) return;
        #if 0
        if(dueltime < 0)
        {
            if(duelqueue.length() >= 2) clear();
            else
            {
                loopv(clients) queue(clients[i]); // safety
                return;
            }
        }
        else cleanup();
        #else
        cleanup();
        #endif
        if(dueltime >= 0)
        {
            if(gamemillis >= dueltime)
            {
                bool resetwinner = false;
                if(m_duel(gamemode, mutators) && GAME(duelcycle)&(m_isteam(gamemode, mutators) ? 2 : 1) && duelwinner >= 0 && duelwins > 0)
                {
                    clientinfo *ci = (clientinfo *)getinfo(duelwinner);
                    if(ci)
                    {
                        int numwins = GAME(duelcycles), numplrs = 0;
                        loopv(clients)
                            if(clients[i]->state.aitype < AI_START && clients[i]->state.state != CS_SPECTATOR && clients[i]->team == ci->team)
                                numplrs++;
                        if(numplrs > (m_isteam(gamemode, mutators) ? 1 : 2))
                        {
                            if(!numwins) numwins = numplrs;
                            if(duelwins >= numwins) resetwinner = true;
                        }
                    }
                    else
                    {
                        duelwinner = -1;
                        duelwins = 0;
                    }
                }
                loopv(clients) if(clients[i]->state.aitype < AI_START)
                    queue(clients[i], !resetwinner && clients[i]->state.state == CS_ALIVE, resetwinner || GAME(duelreset) || clients[i]->state.state != CS_ALIVE, true);
                allowed.shrink(0);
                playing.shrink(0);
                if(!duelqueue.empty())
                {
                    vector<clientinfo *> alive;
                    loopv(duelqueue)
                    {
                        if(m_duel(gamemode, mutators) && alive.length() >= 2) break;
                        clientinfo *ci = duelqueue[i];
                        if(ci->state.state != CS_ALIVE)
                        {
                            if(ci->state.state != CS_WAITING) waiting(ci, 0, DROP_RESET);
                            if(ci->state.aitype < AI_START && m_duel(gamemode, mutators) && m_isteam(gamemode, mutators))
                            {
                                bool skip = false;
                                loopv(alive) if(ci->team == alive[i]->team) { skip = true; break; }
                                if(skip) continue;
                            }
                            if(allowed.find(ci) < 0) allowed.add(ci);
                        }
                        else
                        {
                            ci->state.health = m_health(gamemode, mutators);
                            ci->state.lastregen = gamemillis;
                            ci->state.lastburn = ci->state.lastburntime = ci->state.lastbleed = ci->state.lastbleedtime = 0;
                            sendf(-1, 1, "ri4", N_REGEN, ci->clientnum, ci->state.health, 0); // amt = 0 regens impulse
                        }
                        alive.add(ci);
                        playing.add(ci);
                    }
                    if(alive.length() >= 2)
                    {
                        if(smode) smode->layout();
                        mutate(smuts, mut->layout());
                        loopv(clients) if(clients[i]->state.aitype < AI_START) position(clients[i]);
                        duelround++;
                        string fight;
                        if(m_duel(gamemode, mutators))
                        {
                            defformatstring(namea)("%s", colorname(alive[0]));
                            defformatstring(nameb)("%s", colorname(alive[1]));
                            formatstring(fight)("\fwduel between %s and %s, round \fs\fr#%d\fS", namea, nameb, duelround);
                        }
                        else if(m_survivor(gamemode, mutators))
                            formatstring(fight)("\fwsurvivor, round \fs\fr#%d\fS", duelround);
                        loopv(playing) if(allowbroadcast(playing[i]->clientnum))
                            ancmsgft(playing[i]->clientnum, S_V_FIGHT, CON_EVENT, fight);
                        dueltime = dueldeath = -1;
                        duelcheck = gamemillis+5000;
                    }
                }
            }
        }
        else if(duelround > 0)
        {
            bool cleanup = false;
            vector<clientinfo *> alive;
            loopv(clients)
                if(clients[i]->state.aitype < AI_START && clients[i]->state.state == CS_ALIVE && clients[i]->state.aitype < AI_START)
                    alive.add(clients[i]);
            if(!allowed.empty() && duelcheck >= 0 && gamemillis >= duelcheck) loopvrev(allowed)
            {
                if(alive.find(allowed[i]) < 0) spectator(allowed[i]);
                allowed.remove(i);
                cleanup = true;
            }
            if(allowed.empty())
            {
                if(m_survivor(gamemode, mutators) && m_isteam(gamemode, mutators) && !alive.empty())
                {
                    bool found = false;
                    loopv(alive) if(i && alive[i]->team != alive[i-1]->team) { found = true; break; }
                    if(!found)
                    {
                        if(dueldeath < 0) dueldeath = gamemillis+DEATHMILLIS;
                        else if(gamemillis >= dueldeath)
                        {
                            if(!cleanup)
                            {
                                srvmsgf(-1, "\fyteam \fs\f[%d]%s\fS are the victors", TEAM(alive[0]->team, colour), TEAM(alive[0]->team, name));
                                bool teampoints = true;
                                loopv(playing)
                                {
                                    if(playing[i]->team == alive[0]->team)
                                    {
                                        if(allowbroadcast(playing[i]->clientnum))
                                            ancmsgft(playing[i]->clientnum, S_V_YOUWIN, -1, "");
                                        if(alive.find(playing[i]) >= 0)
                                        {
                                            givepoints(playing[i], 1, teampoints);
                                            teampoints = false;
                                        }
                                    }
                                    else if(allowbroadcast(playing[i]->clientnum))
                                        ancmsgft(playing[i]->clientnum, S_V_YOULOSE, -1, "");
                                }
                            }
                            clear();
                        }
                    }
                }
                else switch(alive.length())
                {
                    case 0:
                    {
                        if(!cleanup)
                        {
                            srvmsgf(-1, "\fyeveryone died, \fzoyepic fail");
                            loopv(playing) if(allowbroadcast(playing[i]->clientnum))
                                ancmsgft(playing[i]->clientnum, S_V_DRAW, -1, "");
                            duelwinner = -1;
                            duelwins = 0;
                        }
                        clear();
                        break;
                    }
                    case 1:
                    {
                        if(dueldeath < 0) dueldeath = gamemillis+DEATHMILLIS;
                        else if(gamemillis >= dueldeath)
                        {
                            if(!cleanup)
                            {
                                if(duelwinner != alive[0]->clientnum)
                                {
                                    duelwinner = alive[0]->clientnum;
                                    duelwins = 1;
                                    srvmsgf(-1, "\fy%s was the victor", colorname(alive[0]));
                                }
                                else
                                {
                                    duelwins++;
                                    srvmsgf(-1, "\fy%s was the victor (\fs\fc%d\fS in a row)", colorname(alive[0]), duelwins);
                                }
                                loopv(playing)
                                {
                                    if(playing[i] == alive[0])
                                    {
                                        if(allowbroadcast(playing[i]->clientnum))
                                            ancmsgft(playing[i]->clientnum, S_V_YOUWIN, -1, "");
                                        givepoints(playing[i], 1);
                                    }
                                    else if(allowbroadcast(playing[i]->clientnum))
                                        ancmsgft(playing[i]->clientnum, S_V_YOULOSE, -1, "");
                                }
                            }
                            clear();
                        }
                        break;
                    }
                    default: break;
                }
            }
        }
    }

    void reset(bool empty)
    {
        duelround = duelwins = 0;
        duelwinner = -1;
        allowed.shrink(0);
        duelqueue.shrink(0);
        playing.shrink(0);
        clear();
    }
} duelmutator;
#endif
