// server-side bomber manager
struct bomberservmode : bomberstate, servmode
{
    bool hasflaginfo;
    int bombertime, scoresec;

    bomberservmode() : hasflaginfo(false), bombertime(-1) {}

    void reset(bool empty)
    {
        bomberstate::reset();
        hasflaginfo = false;
        bombertime = -1;
    }

    void dropaffinity(clientinfo *ci, const vec &o, const vec &inertia = vec(0, 0, 0), int target = -1)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START) return;
        vec n = inertia.iszero() ? vec(0, 0, GAME(bomberspeed)/10.f) : inertia;
        loopv(flags) if(flags[i].owner == ci->clientnum)
        {
            ivec p(vec(o).mul(DMF)), q(vec(n).mul(DMF));
            sendf(-1, 1, "ri3i7", N_DROPAFFIN, ci->clientnum, target, i, p.x, p.y, p.z, q.x, q.y, q.z);
            bomberstate::dropaffinity(i, o, n, gamemillis);
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        dropaffinity(ci, ci->state.o, vec(ci->state.vel).add(ci->state.falling));
    }

    void dodamage(clientinfo *target, clientinfo *actor, int &damage, int &hurt, int &weap, int &flags, const ivec &hitpush)
    {
        //if(weaptype[weap].melee || flags&HIT_CRIT) dropaffinity(target, target->state.o);
    }

    void spawned(clientinfo *ci)
    {
        if(bombertime >= 0) return;
        if(m_team(gamemode, mutators))
        {
            int alive[TEAM_MAX] = {0}, numt = numteams(gamemode, mutators);
            loopv(clients) if(clients[i]->state.state == CS_ALIVE) alive[clients[i]->team]++;
            loopk(numt) if(!alive[k+1]) return;
        }
        else
        {
            int alive = 0;
            loopv(clients) if(clients[i]->state.state == CS_ALIVE) alive++;
            if(alive <= 1) return;
        }
        bombertime = gamemillis+GAME(bomberdelay);
        if(!m_duke(gamemode, mutators)) loopvj(sents) if(enttype[sents[j].type].usetype == EU_ITEM) setspawn(j, hasitem(j), true, true);
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        dropaffinity(ci, ci->state.o, vec(ci->state.vel).add(ci->state.falling));
    }

    int addscore(int team, int points)
    {
        score &cs = teamscore(team);
        cs.total += points;
        return cs.total;
    }

    void scorebomb(clientinfo *ci, int relay, int goal)
    {
        flag g = flags[goal];
        if(!g.enabled) return;
        bomberstate::returnaffinity(relay, gamemillis, false);
        int score = 0;
        if(g.team != ci->team)
        {
            givepoints(ci, GAME(bomberpoints));
            ci->state.gscore++;
            score = addscore(ci->team, 1);
        }
        else
        {
            givepoints(ci, -GAME(bomberpenalty));
            ci->state.gscore--;
            score = addscore(ci->team, -1);
        }
        sendf(-1, 1, "ri5", N_SCOREAFFIN, ci->clientnum, relay, goal, score);
        if(GAME(bomberreset))
        {
            loopvj(clients) if(clients[j]->state.state != CS_SPECTATOR && clients[j]->state.aitype < AI_START)
            {
                if((GAME(bomberreset) >= 2 || clients[j]->team == ci->team) && (clients[j]->state.state == CS_ALIVE || !m_duke(gamemode, mutators)))
                    waiting(clients[j], 0, 3);
            }
            bombertime = -1;
        }
        else bombertime = gamemillis+GAME(bomberdelay);
        loopvj(flags) if(flags[j].enabled)
        {
            bomberstate::returnaffinity(j, gamemillis, false);
            sendf(-1, 1, "ri3", N_RESETAFFIN, j, 0);
        }
        if(GAME(bomberlimit) && score >= GAME(bomberlimit))
        {
            ancmsgft(-1, S_GUIBACK, CON_EVENT, "\fyscore limit has been reached");
            startintermission();
        }
    }

    void scoreaffinity(clientinfo *ci, int relay, int goal)
    {
        if(!m_team(gamemode, mutators) || !m_gsp1(gamemode, mutators) || !flags.inrange(relay) || !flags.inrange(goal) || flags[relay].lastowner != ci->clientnum || !flags[relay].droptime) return;
        scorebomb(ci, relay, goal);
    }

    void moved(clientinfo *ci, const vec &oldpos, const vec &newpos)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START || !m_team(gamemode, mutators) || m_gsp2(gamemode, mutators)) return;
        if(GAME(bomberthreshold) > 0 && oldpos.dist(newpos) >= GAME(bomberthreshold))
            dropaffinity(ci, oldpos, vec(ci->state.vel).add(ci->state.falling));
        loopv(flags) if(isbomberaffinity(flags[i]) && flags[i].owner == ci->clientnum)
        {
            loopvk(flags)
            {
                flag &f = flags[k];
                if(isbombertarg(f, ci->team) && newpos.dist(f.spawnloc) <= enttype[AFFINITY].radius/2) scorebomb(ci, i, k);
            }
        }
    }

    void takeaffinity(clientinfo *ci, int i)
    {
        if(!hasflaginfo || !flags.inrange(i) || ci->state.state!=CS_ALIVE || ci->state.aitype >= AI_START) return;
        flag &f = flags[i];
        if(!isbomberaffinity(f) || f.owner >= 0 || !f.enabled) return;
        if(f.lastowner == ci->clientnum && f.droptime && (GAME(bomberpickupdelay) < 0 || lastmillis-f.droptime <= GAME(bomberpickupdelay))) return;
        bomberstate::takeaffinity(i, ci->clientnum, gamemillis);
        givepoints(ci, GAME(bomberpickuppoints));
        sendf(-1, 1, "ri3", N_TAKEAFFIN, ci->clientnum, i);
    }

    void returnaffinity(int i, int v)
    {
        bomberstate::returnaffinity(i, gamemillis, false);
        sendf(-1, 1, "ri3", N_RESETAFFIN, i, flags[i].enabled ? v : 0);
        if(v && !flags[i].enabled)
        {
            loopvj(flags) if(flags[j].enabled) returnaffinity(j, 0);
            if(bombertime >= 0) bombertime = gamemillis+GAME(bomberdelay);
        }
    }

    void resetaffinity(clientinfo *ci, int i, bool force = false)
    {
        if(!hasflaginfo || !flags.inrange(i) || ci->state.ownernum >= 0) return;
        flag &f = flags[i];
        if(!isbomberaffinity(f) || f.owner >= 0 || !f.droptime || f.votes.find(ci->clientnum) >= 0 || !f.enabled) return;
        f.votes.add(ci->clientnum);
        if(f.votes.length() >= numclients()/2) returnaffinity(i, 2);
    }

    void layout()
    {
        if(!hasflaginfo) return;
        bombertime = -1;
        loopv(flags) if(flags[i].owner >= 0 || flags[i].droptime) returnaffinity(i, 0);
        bombertime = gamemillis+GAME(bomberdelay);
    }

    void update()
    {
        if(!hasflaginfo || bombertime < 0) return;
        if(bombertime)
        {
            if(gamemillis < bombertime) return;
            int hasaffinity = 0;
            vector<int> candidates[TEAM_MAX];
            loopv(flags) candidates[flags[i].team].add(i);
            int wants = teamcount(gamemode, mutators);
            loopi(wants)
            {
                int c = candidates[i].length(), r = c > 1 ? rnd(c) : 0;
                if(candidates[i].inrange(r) && flags.inrange(candidates[i][r]) && isteam(gamemode, mutators, flags[candidates[i][r]].team, TEAM_NEUTRAL))
                {
                    bomberstate::returnaffinity(candidates[i][r], gamemillis, true);
                    sendf(-1, 1, "ri3", N_RESETAFFIN, candidates[i][r], 1);
                    hasaffinity++;
                }
            }
            if(hasaffinity < wants)
            {
                if(hasaffinity && !m_gsp2(gamemode, mutators)) changemap(smapname, gamemode, mutators|G_M_GSP2);
                else hasflaginfo = false;
                return;
            }
            if(!hasaffinity) { hasflaginfo = false; return; }
            ancmsgft(-1, S_V_BOMBSTART, CON_INFO, "\fathe \fs\fwbomb\fS has been spawned");
            bombertime = 0;
        }
        int t = (gamemillis/GAME(bomberholdinterval))-((gamemillis-(curtime+scoresec))/GAME(bomberholdinterval));
        if(t < 1) scoresec += curtime;
        else scoresec = 0;
        loopv(flags) if(isbomberaffinity(flags[i]))
        {
            flag &f = flags[i];
            if(f.owner >= 0)
            {
                clientinfo *ci = (clientinfo *)getinfo(f.owner);
                if((!m_team(gamemode, mutators) || m_gsp2(gamemode, mutators)) && t > 0)
                {
                    int score = GAME(bomberholdpoints)*t;
                    if(score)
                    {
                        int total = 0;
                        givepoints(ci, score);
                        if(m_team(gamemode, mutators))
                        {
                            total = addscore(ci->team, score);
                            sendf(-1, 1, "ri3", N_SCORE, ci->team, total);
                        }
                        else total = ci->state.points;
                        if(GAME(bomberholdlimit) && total >= GAME(bomberholdlimit))
                        {
                            ancmsgft(-1, S_GUIBACK, CON_EVENT, "\fyscore limit has been reached");
                            startintermission();
                        }
                    }
                }
                if(ci && GAME(bombercarrytime) && gamemillis-f.taketime >= GAME(bombercarrytime))
                {
                    ci->state.weapshots[WEAP_GRENADE][0].add(1);
                    sendf(-1, 1, "ri7", N_DROP, ci->clientnum, -1, 1, WEAP_GRENADE, -1, -1);
                    dropaffinity(ci, ci->state.o, vec(ci->state.vel).add(ci->state.falling));
                    if((!m_team(gamemode, mutators) || m_gsp2(gamemode, mutators)) && GAME(bomberholdpenalty))
                    {
                        int total = 0;
                        givepoints(ci, 0-GAME(bomberholdpenalty));
                        if(m_team(gamemode, mutators))
                        {
                            total = addscore(ci->team, 0-GAME(bomberholdpenalty));
                            sendf(-1, 1, "ri3", N_SCORE, ci->team, total);
                        }
                        else total = ci->state.points;
                    }
                }
                continue;
            }
            if(f.droptime && gamemillis-f.droptime >= GAME(bomberresetdelay)) returnaffinity(i, 2);
        }
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_INITAFFIN);
        putint(p, flags.length());
        loopv(flags)
        {
            flag &f = flags[i];
            putint(p, f.team);
            putint(p, f.owner);
            if(f.owner<0)
            {
                putint(p, f.droptime);
                if(f.droptime)
                {
                    putint(p, int(f.droploc.x*DMF));
                    putint(p, int(f.droploc.y*DMF));
                    putint(p, int(f.droploc.z*DMF));
                    putint(p, int(f.inertia.x*DMF));
                    putint(p, int(f.inertia.y*DMF));
                    putint(p, int(f.inertia.z*DMF));
                }
            }
        }
    }

    void regen(clientinfo *ci, int &total, int &amt, int &delay)
    {
        if(hasflaginfo && GAME(regenaffinity)) loopv(flags)
        {
            flag &f = flags[i];
            bool insidehome = (isbomberhome(f, ci->team) && f.owner < 0 && !f.droptime && ci->state.o.dist(f.spawnloc) <= enttype[AFFINITY].radius);
            if(insidehome || (GAME(regenaffinity) == 2 && f.owner == ci->clientnum))
            {
                if(GAME(maxhealth)) total = max(int(m_health(gamemode, mutators)*GAME(maxhealth)), total);
                if(ci->state.lastregen && GAME(regenguard)) delay = GAME(regenguard);
                if(GAME(regenextra)) amt += GAME(regenextra);
                return;
            }
        }
    }

    void parseaffinity(ucharbuf &p)
    {
        int numflags = getint(p);
        if(numflags)
        {
            loopi(numflags)
            {
                int team = getint(p);
                vec o;
                loopk(3) o[k] = getint(p)/DMF;
                if(!hasflaginfo) addaffinity(o, team, i);
            }
            hasflaginfo = true;
        }
    }

    int points(clientinfo *victim, clientinfo *actor)
    {
        bool isteam = victim==actor || (m_team(gamemode, mutators) && victim->team == actor->team);
        int p = isteam ? -1 : (m_team(gamemode, mutators) ? 1 : 0), v = p;
        if(p) { loopv(flags) if(flags[i].owner == victim->clientnum) p += v; }
        return p;
    }
} bombermode;
