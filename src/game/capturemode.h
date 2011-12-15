// server-side capture manager
struct captureservmode : capturestate, servmode
{
    bool hasflaginfo;

    captureservmode() : hasflaginfo(false) {}

    void reset(bool empty)
    {
        capturestate::reset();
        hasflaginfo = false;
    }

    void dropaffinity(clientinfo *ci, const vec &o, const vec &inertia = vec(0, 0, 0), int target = -1)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START) return;
        int numflags = 0, iterflags = 0;
        loopv(flags) if(flags[i].owner == ci->clientnum) numflags++;
        vec dir = inertia, olddir = dir;
        if(numflags > 1 && dir.iszero())
        {
            dir.x = -sinf(RAD*ci->state.yaw);
            dir.y = cosf(RAD*ci->state.yaw);
            dir.x = rnd(101)/100.f;
            olddir = dir.normalize().mul(max(dir.magnitude(), 1.f));
        }
        loopv(flags) if(flags[i].owner == ci->clientnum)
        {
            if(numflags > 1)
            {
                float yaw = -45.f+(90/float(numflags+1)*(iterflags+1));
                dir = vec(olddir).rotate_around_z(yaw*RAD);
                iterflags++;
            }
            ivec p(vec(o).mul(DMF)), q(vec(dir).mul(DMF));
            sendf(-1, 1, "ri3i7", N_DROPAFFIN, ci->clientnum, -1, i, p.x, p.y, p.z, q.x, q.y, q.z);
            capturestate::dropaffinity(i, o, dir, gamemillis);
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        dropaffinity(ci, ci->state.o, vec(ci->state.vel).add(ci->state.falling));
    }

    void dodamage(clientinfo *target, clientinfo *actor, int &damage, int &hurt, int &weap, int &flags, const ivec &hitpush)
    {
        //if(weaptype[weap].melee || flags&HIT_CRIT) dropaffinity(target, target->state.o, vec(ci->state.vel).add(ci->state.falling));
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        dropaffinity(ci, ci->state.o, vec(ci->state.vel).add(ci->state.falling));
    }

    int addscore(int team, int points = 1)
    {
        score &cs = teamscore(team);
        cs.total += points;
        return cs.total;
    }

    void moved(clientinfo *ci, const vec &oldpos, const vec &newpos)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START) return;
        if(GAME(capturethreshold) > 0 && oldpos.dist(newpos) >= GAME(capturethreshold))
            dropaffinity(ci, oldpos, vec(ci->state.vel).add(ci->state.falling));
        loopv(flags) if(flags[i].owner == ci->clientnum)
        {
            loopvk(flags)
            {
                flag &f = flags[k];
                if(f.team == ci->team && (f.owner < 0 || (m_gsp1(gamemode, mutators) && f.owner == ci->clientnum && i == k)) && !f.droptime && newpos.dist(f.spawnloc) <= enttype[AFFINITY].radius*2/3)
                {
                    capturestate::returnaffinity(i, gamemillis);
                    givepoints(ci, GAME(capturepoints));
                    if(flags[i].team != ci->team)
                    {
                        ci->state.gscore++;
                        int score = addscore(ci->team);
                        sendf(-1, 1, "ri5", N_SCOREAFFIN, ci->clientnum, i, k, score);
                        if(m_duke(gamemode, mutators))
                        {
                            loopvj(clients) if(clients[j]->state.aitype < AI_START && clients[j]->state.state == CS_ALIVE && clients[j]->team == flags[i].team)
                                waiting(clients[j], 0, 3);
                        }
                        if(GAME(capturelimit) && score >= GAME(capturelimit))
                        {
                            ancmsgft(-1, S_V_NOTIFY, CON_EVENT, "\fyscore limit has been reached");
                            startintermission();
                        }
                    }
                    else sendf(-1, 1, "ri3", N_RETURNAFFIN, ci->clientnum, i);
                }
            }
        }
    }

    void takeaffinity(clientinfo *ci, int i)
    {
        if(!hasflaginfo || !flags.inrange(i) || ci->state.state!=CS_ALIVE || !ci->team || ci->state.aitype >= AI_START) return;
        flag &f = flags[i];
        if(f.owner >= 0 || (f.team == ci->team && !m_gsp3(gamemode, mutators) && (m_gsp2(gamemode, mutators) || !f.droptime))) return;
        if(f.lastowner == ci->clientnum && f.droptime && (GAME(capturepickupdelay) < 0 || lastmillis-f.droptime <= GAME(capturepickupdelay))) return;
        if(!m_gsp(gamemode, mutators) && f.team == ci->team)
        {
            capturestate::returnaffinity(i, gamemillis);
            givepoints(ci, GAME(capturepoints));
            sendf(-1, 1, "ri3", N_RETURNAFFIN, ci->clientnum, i);
        }
        else
        {
            capturestate::takeaffinity(i, ci->clientnum, gamemillis);
            if(f.team != ci->team) givepoints(ci, GAME(capturepickuppoints));
            sendf(-1, 1, "ri3", N_TAKEAFFIN, ci->clientnum, i);
        }
    }

    void resetaffinity(clientinfo *ci, int i)
    {
        if(!hasflaginfo || !flags.inrange(i) || ci->state.ownernum >= 0) return;
        flag &f = flags[i];
        if(f.owner >= 0 || !f.droptime || f.votes.find(ci->clientnum) >= 0) return;
        f.votes.add(ci->clientnum);
        if(f.votes.length() >= numclients()/2)
        {
            capturestate::returnaffinity(i, gamemillis);
            sendf(-1, 1, "ri3", N_RESETAFFIN, i, 2);
        }
    }

    void layout()
    {
        if(!hasflaginfo) return;
        loopv(flags) if(flags[i].owner >= 0 || flags[i].droptime)
        {
            capturestate::returnaffinity(i, gamemillis);
            sendf(-1, 1, "ri3", N_RESETAFFIN, i, 1);
        }
    }

    void update()
    {
        if(!hasflaginfo) return;
        loopv(flags)
        {
            flag &f = flags[i];
            if(m_gsp3(gamemode, mutators) && f.owner >= 0 && f.taketime && gamemillis-f.taketime >= GAME(captureprotectdelay))
            {
                clientinfo *ci = (clientinfo *)getinfo(f.owner);
                if(f.team != ci->team)
                {
                    capturestate::returnaffinity(i, gamemillis);
                    givepoints(ci, GAME(capturepoints));
                    ci->state.gscore++;
                    int score = addscore(ci->team);
                    sendf(-1, 1, "ri5", N_SCOREAFFIN, ci->clientnum, i, -1, score);
                    if(GAME(capturelimit) && score >= GAME(capturelimit))
                    {
                        ancmsgft(-1, S_V_NOTIFY, CON_EVENT, "\fyscore limit has been reached");
                        startintermission();
                    }
                }
            }
            else if(f.owner < 0 && f.droptime && gamemillis-f.droptime >= capturedelay)
            {
                capturestate::returnaffinity(i, gamemillis);
                loopvk(clients) if(f.team == clients[k]->team) givepoints(clients[k], -GAME(capturepenalty));
                sendf(-1, 1, "ri3", N_RESETAFFIN, i, 2);
            }
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
        if(hasflaginfo)
        {
            if(m_gsp2(gamemode, mutators)) loopv(flags)
            {
                flag &f = flags[i];
                if(f.team == ci->team && f.owner < 0 && ci->state.o.dist(f.droptime ? f.droploc : f.spawnloc) <= enttype[AFFINITY].radius*2.f)
                {
                    if(GAME(maxhealth)) total = max(int(m_health(gamemode, mutators)*GAME(maxhealth)), total);
                    if(ci->state.lastregen && GAME(regenguard)) delay = GAME(regenguard);
                    if(GAME(regenextra)) amt += GAME(regenextra);
                    return;
                }
            }
            if(GAME(regenaffinity)) loopv(flags)
            {
                flag &f = flags[i];
                if((f.team == ci->team && f.owner < 0 && !f.droptime && ci->state.o.dist(f.spawnloc) <= enttype[AFFINITY].radius*2.f) || (GAME(regenaffinity) == 2 && f.owner == ci->clientnum))
                {
                    if(GAME(maxhealth)) total = max(int(m_health(gamemode, mutators)*GAME(maxhealth)), total);
                    if(ci->state.lastregen && GAME(regenguard)) delay = GAME(regenguard);
                    if(GAME(regenextra)) amt += GAME(regenextra);
                    return;
                }
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
                if(!hasflaginfo) addaffinity(o, team);
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
} capturemode;
