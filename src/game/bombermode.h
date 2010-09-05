// server-side bomber manager
struct bomberservmode : bomberstate, servmode
{
    bool hasflaginfo;
    int bombertime;

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
        loopv(flags) if(flags[i].owner == ci->clientnum)
        {
            ivec p(vec(o).mul(DMF)), q((inertia.iszero() ? vec(0, 0, enttype[AFFINITY].radius/2) : vec(inertia)).mul(DMF));
            sendf(-1, 1, "ri3i7", N_DROPAFFIN, ci->clientnum, target, i, p.x, p.y, p.z, q.x, q.y, q.z);
            bomberstate::dropaffinity(i, o, inertia, gamemillis);
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        dropaffinity(ci, ci->state.o);
    }

    void dodamage(clientinfo *target, clientinfo *actor, int &damage, int &weap, int &flags, const ivec &hitpush)
    {
        if(weaptype[weap].melee || flags&HIT_CRIT) dropaffinity(target, target->state.o);
    }

    void spawned(clientinfo *ci)
    {
        if(bombertime < 0)
        {
            int alive[TEAM_MAX] = {0};
            loopv(clients) if(clients[i]->state.state == CS_ALIVE) alive[clients[i]->team]++;
            if(alive[TEAM_ALPHA] && alive[TEAM_BETA]) bombertime = gamemillis+GAME(bomberdelay);
        }
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        dropaffinity(ci, ci->state.o);
    }

    int addscore(int team)
    {
        score &cs = findscore(team);
        cs.total++;
        return cs.total;
    }

    int subscore(int team)
    {
        score &cs = findscore(team);
        cs.total--;
        return cs.total;
    }

    void scorebomb(clientinfo *ci, int relay, int goal)
    {
        flag &f = flags[relay], g = flags[goal];
        bomberstate::returnaffinity(relay, gamemillis, true, false);
        int score = 0;
        if(g.team != ci->team)
        {
            givepoints(ci, 5);
            ci->state.flags++;
            score = addscore(ci->team);
        }
        else
        {
            givepoints(ci, -5);
            ci->state.flags--;
            score = subscore(ci->team);
        }
        sendf(-1, 1, "ri5", N_SCOREAFFIN, ci->clientnum, relay, goal, score);
        loopvj(clients) if(clients[j]->state.state != CS_SPECTATOR && clients[j]->state.aitype < AI_START)
        {
            bool kamikaze = clients[j]->state.state == CS_ALIVE && clients[j]->team == f.team;
            if(kamikaze || !m_duke(gamemode, mutators)) waiting(clients[j], 0, kamikaze ? 3 : 1);
        }
        if(!m_duke(gamemode, mutators)) loopvj(sents) if(enttype[sents[j].type].usetype == EU_ITEM) setspawn(j, hasitem(j));
        loopvj(flags) if(flags[j].enabled)
        {
            bomberstate::returnaffinity(j, gamemillis, true, false);
            sendf(-1, 1, "ri3", N_RESETAFFIN, j, 0);
        }
        if(GAME(bomberlimit) && score >= GAME(bomberlimit))
        {
            sendf(-1, 1, "ri3s", N_ANNOUNCE, S_GUIBACK, CON_MESG, "\fyscore limit has been reached");
            startintermission();
        }
        bombertime = -1;
    }

    void scoreaffinity(clientinfo *ci, int relay, int goal)
    {
        if(!flags.inrange(relay) || !flags.inrange(goal) || flags[relay].lastowner != ci->clientnum || !flags[relay].droptime) return;
        scorebomb(ci, relay, goal);
    }

    void moved(clientinfo *ci, const vec &oldpos, const vec &newpos)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START) return;
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
        if(!hasflaginfo || !flags.inrange(i) || ci->state.state!=CS_ALIVE || !ci->team || ci->state.aitype >= AI_START) return;
        flag &f = flags[i];
        if(!isbomberaffinity(f) || f.owner >= 0) return;
        bomberstate::takeaffinity(i, ci->clientnum, gamemillis);
        givepoints(ci, 3);
        sendf(-1, 1, "ri3", N_TAKEAFFIN, ci->clientnum, i);
    }

    void resetaffinity(clientinfo *ci, int i, bool force = false)
    {
        if(!hasflaginfo || !flags.inrange(i) || ci->state.ownernum >= 0) return;
        flag &f = flags[i];
        if(!isbomberaffinity(f) || f.owner >= 0 || !f.droptime || f.votes.find(ci->clientnum) >= 0) return;
        f.votes.add(ci->clientnum);
        if(f.votes.length() >= numclients()/2)
        {
            bomberstate::returnaffinity(i, gamemillis, true, true);
            sendf(-1, 1, "ri3", N_RESETAFFIN, i, 1);
        }
    }

    void update()
    {
        if(!hasflaginfo || bombertime < 0) return;
        if(bombertime)
        {
            if(gamemillis < bombertime) return;
            vector<int> candidates[TEAM_MAX];
            loopv(flags) candidates[flags[i].team].add(i);
            loopi(TEAM_COUNT)
            {
                int c = candidates[i].length(), r = c > 1 ? rnd(c) : 0;
                if(candidates[i].inrange(r) && flags.inrange(candidates[i][r]))
                {
                    bomberstate::returnaffinity(candidates[i][r], gamemillis, true, true);
                    sendf(-1, 1, "ri3", N_RESETAFFIN, candidates[i][r], 1);
                }
                else return;
            }
            sendf(-1, 1, "ri3s", N_ANNOUNCE, S_V_FIGHT, CON_MESG, "\fwnew round starting");
            bombertime = 0;
        }
        loopv(flags) if(isbomberaffinity(flags[i]))
        {
            flag &f = flags[i];
            if(f.owner >= 0)
            {
                clientinfo *ci = (clientinfo *)getinfo(f.owner);
                if(ci && GAME(bomberholdtime) && gamemillis-f.taketime >= GAME(bomberholdtime))
                {
                    ci->state.weapshots[WEAP_GRENADE][0].add(1);
                    sendf(-1, 1, "ri7", N_DROP, ci->clientnum, -1, 1, WEAP_GRENADE, -1, -1);
                    dropaffinity(ci, ci->state.o);
                }
                continue;
            }
            if(f.droptime && gamemillis-f.droptime >= GAME(bomberresetdelay))
            {
                bomberstate::returnaffinity(i, gamemillis, true, true);
                sendf(-1, 1, "ri3", N_RESETAFFIN, i, 1);
            }
        }
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        if(connecting)
        {
            loopv(scores)
            {
                score &cs = scores[i];
                putint(p, N_SCORE);
                putint(p, cs.team);
                putint(p, cs.total);
            }
        }
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
                if(GAME(extrahealth)) total = max(GAME(extrahealth), total);
                if(ci->state.lastregen && GAME(regenguard)) delay = GAME(regenguard);
                if(GAME(regenextra)) amt = GAME(regenextra);
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
        bool isteam = victim==actor || victim->team == actor->team;
        int p = isteam ? -1 : 1, v = p;
        loopv(flags) if(flags[i].owner == victim->clientnum) p += v;
        return p;
    }
} bombermode;
