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
        if(!canplay(hasflaginfo) || ci->state.actortype >= A_ENEMY) return;
        vec n = inertia.iszero() ? vec(0, 0, G(bomberspeed)/10.f) : inertia;
        loopv(flags) if(flags[i].owner == ci->clientnum)
        {
            ivec p(vec(o).mul(DMF)), q(vec(n).mul(DMF));
            sendf(-1, 1, "ri3i7", N_DROPAFFIN, ci->clientnum, target, i, p.x, p.y, p.z, q.x, q.y, q.z);
            bomberstate::dropaffinity(i, o, n, gamemillis);
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(!canplay(hasflaginfo)) return;
        dropaffinity(ci, ci->state.feetpos(G(bomberdropheight)), vec(ci->state.vel).add(ci->state.falling));
    }

    void dodamage(clientinfo *m, clientinfo *v, int &damage, int &hurt, int &weap, int &flags, int &material, const ivec &hitpush, const ivec &hitvel, float dist)
    {
        //if(weaptype[weap].melee) dropaffinity(m, m->state.o);
    }

    void spawned(clientinfo *ci)
    {
        if(bombertime >= 0) return;
        if(m_team(gamemode, mutators))
        {
            int alive[T_MAX] = {0}, numt = numteams(gamemode, mutators);
            loopv(clients) if(clients[i]->state.state == CS_ALIVE) alive[clients[i]->team]++;
            loopk(numt) if(!alive[k+1]) return;
        }
        else
        {
            int alive = 0;
            loopv(clients) if(clients[i]->state.state == CS_ALIVE) alive++;
            if(alive <= 1) return;
        }
        bombertime = gamemillis+G(bomberdelay);
        loopvj(sents) if(enttype[sents[j].type].usetype == EU_ITEM) setspawn(j, hasitem(j), true, true);
    }

    void died(clientinfo *ci, clientinfo *v)
    {
        if(!canplay(hasflaginfo)) return;
        dropaffinity(ci, ci->state.feetpos(G(bomberdropheight)), vec(ci->state.vel).add(ci->state.falling));
        if(v && m_gsp1(gamemode, mutators) && (!m_team(gamemode, mutators) || ci->team != v->team))
        {
            loopv(flags) if(isbomberaffinity(flags[i]) && flags[i].owner == v->clientnum)
                flags[i].taketime = gamemillis;
        }
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
        int total = 0;
        if(g.team != ci->team)
        {
            givepoints(ci, G(bomberpoints));
            total = addscore(ci->team, 1);
        }
        else
        {
            givepoints(ci, -G(bomberpenalty));
            total = addscore(ci->team, -1);
        }
        bomberstate::returnaffinity(relay, gamemillis, false);
        sendf(-1, 1, "ri5", N_SCOREAFFIN, ci->clientnum, relay, goal, total);
        mutate(smuts, mut->scoreaffinity(ci, g.team != ci->team));
        bombertime = m_duke(gamemode, mutators) ? -1 : gamemillis+G(bomberdelay);
        loopvj(flags) if(flags[j].enabled)
        {
            bomberstate::returnaffinity(j, gamemillis, false);
            sendf(-1, 1, "ri3", N_RESETAFFIN, j, 0);
        }
        if(!m_balance(gamemode, mutators) && G(bomberlimit) && total >= G(bomberlimit))
        {
            ancmsgft(-1, S_V_NOTIFY, CON_EVENT, "\fyscore limit has been reached");
            startintermission();
        }
        if(m_gsp3(gamemode, mutators) && G(bomberattackwinner))
        {
            int numt = numteams(gamemode, mutators);
            if(curbalance == numt-1)
            {
                bool found = false;
                loopi(numt)
                {
                    int t = i+T_FIRST, s = teamscore(t).total;
                    if(t != T_OMEGA && s >= total)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    ancmsgft(-1, S_V_NOTIFY, CON_EVENT, "\fybest score has been reached");
                    startintermission();
                }
            }
        }
    }

    void moved(clientinfo *ci, const vec &oldpos, const vec &newpos)
    {
        if(!canplay(hasflaginfo) || ci->state.actortype >= A_ENEMY) return;
        if(G(bomberthreshold) > 0 && oldpos.dist(newpos) >= G(bomberthreshold))
            dropaffinity(ci, oldpos, vec(ci->state.vel).add(ci->state.falling));
        if(m_gsp1(gamemode, mutators) || (G(bomberbasketonly) && !m_gsp2(gamemode, mutators))) return;
        loopv(flags) if(isbomberaffinity(flags[i]) && flags[i].owner == ci->clientnum) loopvk(flags)
            if(isbombertarg(flags[k], ci->team) && newpos.dist(flags[k].spawnloc) <= enttype[AFFINITY].radius/2) scorebomb(ci, i, k);
    }

    void returnaffinity(int i, int v) // 0 = disable, 1 = return and wait, 2 = return instantly
    {
        bomberstate::returnaffinity(i, gamemillis, v == 2);
        sendf(-1, 1, "ri3", N_RESETAFFIN, i, flags[i].enabled ? v : 0);
        if(v && !flags[i].enabled)
        {
            loopvj(flags) if(flags[j].enabled) returnaffinity(j, 0);
            if(bombertime >= 0) bombertime = gamemillis+G(bomberdelay);
        }
    }

    void takeaffinity(clientinfo *ci, int i)
    {
        if(!canplay(hasflaginfo) || !flags.inrange(i) || ci->state.state!=CS_ALIVE || ci->state.actortype >= A_ENEMY) return;
        flag &f = flags[i];
        if(!isbomberaffinity(f) || f.owner >= 0 || !f.enabled) return;
        if(f.lastowner == ci->clientnum && f.droptime && (G(bomberpickupdelay) < 0 || gamemillis-f.droptime <= max(G(bomberpickupdelay), 500))) return;
        if(m_gsp3(gamemode, mutators) && ci->team != T_OMEGA && G(bomberattackreset))
        {
            if(!f.droptime) return;
            if(!m_nopoints(gamemode, mutators) && (!f.droptime || f.lastowner != ci->clientnum)) givepoints(ci, G(bomberpickuppoints));
            returnaffinity(i, 2);
        }
        else
        {
            bomberstate::takeaffinity(i, ci->clientnum, gamemillis);
            if(!m_nopoints(gamemode, mutators) && (!f.droptime || f.lastowner != ci->clientnum)) givepoints(ci, G(bomberpickuppoints));
            sendf(-1, 1, "ri3", N_TAKEAFFIN, ci->clientnum, i);
        }
    }

    void resetaffinity(clientinfo *ci, int i, bool force = false)
    {
        if(!canplay(hasflaginfo) || !flags.inrange(i) || ci->state.ownernum >= 0) return;
        flag &f = flags[i];
        if(!isbomberaffinity(f) || f.owner >= 0 || !f.droptime || f.votes.find(ci->clientnum) >= 0 || !f.enabled) return;
        f.votes.add(ci->clientnum);
        if(f.votes.length() >= numclients()/2) returnaffinity(i, 2);
    }

    void layout()
    {
        if(!canplay(hasflaginfo)) return;
        bombertime = -1;
        loopv(flags) if(flags[i].owner >= 0 || flags[i].droptime) returnaffinity(i, 0);
        bombertime = gamemillis+G(bomberdelay);
    }

    void update()
    {
        if(!canplay(hasflaginfo) || bombertime < 0) return;
        if(bombertime)
        {
            if(gamemillis < bombertime) return;
            int hasaffinity = 0;
            vector<int> candidates[T_MAX];
            loopv(flags) candidates[flags[i].team].add(i);
            int wants = m_gsp1(gamemode, mutators) ? 1 : (m_gsp3(gamemode, mutators) ? 2 : teamcount(gamemode, mutators));
            loopi(wants)
            {
                int c = candidates[i].length(), r = c > 1 ? rnd(c) : 0;
                if(candidates[i].inrange(r) && flags.inrange(candidates[i][r]) && isteam(gamemode, mutators, flags[candidates[i][r]].team, T_NEUTRAL))
                {
                    bomberstate::returnaffinity(candidates[i][r], gamemillis, true);
                    sendf(-1, 1, "ri3", N_RESETAFFIN, candidates[i][r], 1);
                    hasaffinity++;
                }
            }
            if(hasaffinity < wants)
            {
                if(!candidates[T_NEUTRAL].empty() && !m_gsp1(gamemode, mutators))
                {
                    int muts = mutators;
                    if(muts&(1<<G_M_GSP2)) muts &= ~(1<<G_M_GSP2);
                    muts |= (1<<G_M_GSP1);
                    srvmsgf(-1, "\fzoythis map does have enough goals, switching on hold mutator");
                    sendf(-1, 1, "risi3", N_MAPCHANGE, smapname, 0, gamemode, muts);
                    changemap(smapname, gamemode, muts);
                    return;
                }
                hasflaginfo = false;
                loopv(flags) sendf(-1, 1, "ri3", N_RESETAFFIN, i, 0);
                srvmsgf(-1, "\fs\fzoythis map is not playable in:\fS %s", gamename(gamemode, mutators));
            }
            else ancmsgft(-1, m_duke(gamemode, mutators) ? S_V_BOMBDUEL : S_V_BOMBSTART, CON_INFO, "\fathe \fs\fzwvbomb\fS has been spawned");
            bombertime = 0;
        }
        int t = (gamemillis/G(bomberholdinterval))-((gamemillis-(curtime+scoresec))/G(bomberholdinterval));
        if(t < 1) scoresec += curtime;
        else scoresec = 0;
        loopv(flags) if(isbomberaffinity(flags[i]))
        {
            flag &f = flags[i];
            if(f.owner >= 0)
            {
                clientinfo *ci = (clientinfo *)getinfo(f.owner);
                if(m_gsp1(gamemode, mutators) && t > 0)
                {
                    int score = G(bomberholdpoints)*t;
                    if(score)
                    {
                        int total = ci->state.points;
                        givepoints(ci, score);
                        if(m_team(gamemode, mutators))
                        {
                            total = addscore(ci->team, score);
                            sendf(-1, 1, "ri3", N_SCORE, ci->team, total);
                        }
                        if(!m_balance(gamemode, mutators) && G(bomberholdlimit) && total >= G(bomberholdlimit))
                        {
                            ancmsgft(-1, S_V_NOTIFY, CON_EVENT, "\fyscore limit has been reached");
                            startintermission();
                        }
                    }
                }
                if(ci && carrytime && gamemillis-f.taketime >= carrytime)
                {
                    ci->state.weapshots[W_GRENADE][0].add(1);
                    sendf(-1, 1, "ri7", N_DROP, ci->clientnum, -1, 1, W_GRENADE, -1, -1);
                    dropaffinity(ci, ci->state.feetpos(G(bomberdropheight)), vec(ci->state.vel).add(ci->state.falling));
                    if(m_gsp1(gamemode, mutators) && G(bomberholdpenalty))
                    {
                        givepoints(ci, -G(bomberholdpenalty));
                        if(m_team(gamemode, mutators))
                        {
                            int total = addscore(ci->team, -G(bomberholdpenalty));
                            sendf(-1, 1, "ri3", N_SCORE, ci->team, total);
                        }
                    }
                }
                continue;
            }
            if(f.droptime && gamemillis-f.droptime >= G(bomberresetdelay)) returnaffinity(i, 2);
        }
    }

    void sendaffinity()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        initclient(NULL, p, false);
        sendpacket(-1, 1, p.finalize());
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        if(!hasflaginfo) return;
        putint(p, N_INITAFFIN);
        putint(p, flags.length());
        loopv(flags)
        {
            flag &f = flags[i];
            putint(p, f.team);
            putint(p, f.ent);
            putint(p, f.enabled ? 1 : 0);
            putint(p, f.owner);
            loopj(3) putint(p, int(f.spawnloc[j]*DMF));
            if(f.owner<0)
            {
                putint(p, f.droptime);
                if(f.droptime)
                {
                    loopj(3) putint(p, int(f.droploc[j]*DMF));
                    loopj(3) putint(p, int(f.inertia[j]*DMF));
                }
            }
        }
        loopv(clients)
        {
            clientinfo *oi = clients[i];
            if(!oi->connected || (ci && oi->clientnum == ci->clientnum) || !oi->state.lastbuff) continue;
            putint(p, N_SPHY);
            putint(p, oi->clientnum);
            putint(p, SPHY_BUFF);
            putint(p, 1);
        }
    }

    void regen(clientinfo *ci, int &total, int &amt, int &delay)
    {
        if(!canplay(hasflaginfo) || !G(bomberregenbuff) || !ci->state.lastbuff) return;
        if(G(maxhealth)) total = max(m_maxhealth(gamemode, mutators, ci->state.model), total);
        if(ci->state.lastregen && G(bomberregendelay)) delay = G(bomberregendelay);
        if(G(bomberregenextra)) amt += G(bomberregenextra);
    }

    void checkclient(clientinfo *ci)
    {
        if(!canplay(hasflaginfo) || ci->state.state != CS_ALIVE || m_insta(gamemode, mutators)) return;
        #define bomberbuff1 (G(bomberbuffing)&1 && isbomberhome(f, ci->team) && (G(bomberbuffarea) > 0 ? ci->state.o.dist(f.spawnloc) <= G(bomberbuffarea) : true))
        #define bomberbuff2 ((G(bomberbuffing)&2 || (G(bomberbuffing)&4 && m_gsp3(gamemode, mutators) && ci->team != T_OMEGA)) && isbomberaffinity(f) && f.owner == ci->clientnum)
        if(G(bomberbuffing)) loopv(flags)
        {
            flag &f = flags[i];
            if(f.enabled && (bomberbuff1 || bomberbuff2))
            {
                if(!ci->state.lastbuff) sendf(-1, 1, "ri4", N_SPHY, ci->clientnum, SPHY_BUFF, 1);
                ci->state.lastbuff = gamemillis;
                return;
            }
        }
        if(ci->state.lastbuff && (!G(bomberbuffing) || gamemillis-ci->state.lastbuff >= G(bomberbuffdelay)))
        {
            ci->state.lastbuff = 0;
            sendf(-1, 1, "ri4", N_SPHY, ci->clientnum, SPHY_BUFF, 0);
        }
    }

    void moveaffinity(clientinfo *ci, int cn, int id, const vec &o, const vec &inertia = vec(0, 0, 0))
    {
        if(!canplay(hasflaginfo) || !flags.inrange(id)) return;
        flag &f = flags[id];
        if(!f.droptime || f.owner >= 0 || !isbomberaffinity(f) || f.lastowner != ci->clientnum) return;
        f.distance += f.droploc.dist(o);
        f.droploc = o;
        f.inertia = inertia;
        if(!m_gsp1(gamemode, mutators) && !m_gsp2(gamemode, mutators)) loopv(flags)
        {
            if(isbomberaffinity(flags[i]) || f.droploc.dist(flags[i].spawnloc) > enttype[AFFINITY].radius/2) continue;
            if(G(bomberbasketmindist) > 0 && !flags[id].travel(flags[i].spawnloc, G(bomberbasketmindist))) continue;
            scorebomb(ci, id, i);
            break;
        }
        //sendf(-1, 1, "ri9", N_MOVEAFFIN, ci->clientnum, id, int(f.droploc.x*DMF), int(f.droploc.y*DMF), int(f.droploc.z*DMF), int(f.inertia.x*DMF), int(f.inertia.y*DMF), int(f.inertia.z*DMF));
    }

    void parseaffinity(ucharbuf &p)
    {
        int numflags = getint(p);
        if(numflags)
        {
            loopi(numflags)
            {
                int team = getint(p), ent = getint(p);
                vec o;
                loopj(3) o[j] = getint(p)/DMF;
                if(p.overread()) break;
                if(!hasflaginfo && i < MAXPARAMS) addaffinity(o, team, ent);
            }
            if(!hasflaginfo)
            {
                hasflaginfo = true;
                sendaffinity();
                loopv(clients) if(clients[i]->state.state == CS_ALIVE) entergame(clients[i]);
            }
        }
    }

    int points(clientinfo *m, clientinfo *v)
    {
        bool isteam = m==v || (m_team(gamemode, mutators) && m->team == v->team);
        int p = isteam ? -1 : (m_team(gamemode, mutators) ? 1 : 0), q = p;
        if(p) { loopv(flags) if(flags[i].owner == m->clientnum) p += q; }
        return p;
    }
} bombermode;
