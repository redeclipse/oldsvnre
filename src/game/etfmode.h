// server-side etf manager
struct etfservmode : etfstate, servmode
{
    bool hasflaginfo;

    etfservmode() : hasflaginfo(false) {}

    void reset(bool empty)
    {
        etfstate::reset();
        hasflaginfo = false;
    }

    void dropaffinity(clientinfo *ci, const vec &o)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START) return;
        loopv(flags) if(flags[i].owner == ci->clientnum)
        {
            ivec p(vec(ci->state.o.dist(o) > enttype[AFFINITY].radius ? ci->state.o : o).mul(DMF));
            sendf(-1, 1, "ri6", N_DROPAFFIN, ci->clientnum, i, p.x, p.y, p.z);
            etfstate::dropaffinity(i, p.tovec().div(DMF), gamemillis);
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        dropaffinity(ci, ci->state.o);
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

    void moved(clientinfo *ci, const vec &oldpos, const vec &newpos)
    {
        if(!hasflaginfo || ci->state.aitype >= AI_START) return;
        loopv(flags) if(isetfaffinity(flags[i]) && flags[i].owner == ci->clientnum)
        {
            loopvk(flags)
            {
                flag &f = flags[k];
                if(isetftarg(f, ci->team) && newpos.dist(f.spawnloc) <= enttype[AFFINITY].radius*2/3)
                {
                    etfstate::returnaffinity(i, gamemillis);
                    givepoints(ci, 5);
                    if(flags[i].team != ci->team)
                    {
                        ci->state.flags++;
                        int score = addscore(ci->team);
                        sendf(-1, 1, "ri5", N_SCOREAFFIN, ci->clientnum, i, k, score);
                        if(GAME(etflimit) && score >= GAME(etflimit))
                        {
                            sendf(-1, 1, "ri3s", N_ANNOUNCE, S_GUIBACK, CON_MESG, "\fycpature limit has been reached");
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
        if(!isetfaffinity(f) || f.owner >= 0) return;
        etfstate::takeaffinity(i, ci->clientnum, gamemillis);
        givepoints(ci, 3);
        sendf(-1, 1, "ri3", N_TAKEAFFIN, ci->clientnum, i);
    }

    void resetaffinity(clientinfo *ci, int i)
    {
        if(!hasflaginfo || !flags.inrange(i) || ci->state.ownernum >= 0) return;
        flag &f = flags[i];
        if(!isetfaffinity(f) || f.owner >= 0 || !f.droptime || f.votes.find(ci->clientnum) >= 0) return;
        f.votes.add(ci->clientnum);
        if(f.votes.length() >= numclients()/2)
        {
            etfstate::returnaffinity(i, gamemillis);
            sendf(-1, 1, "ri2", N_RESETAFFIN, i);
        }
    }

    void update()
    {
        if(!hasflaginfo) return;
        loopv(flags) if(isetfaffinity(flags[i]))
        {
            flag &f = flags[i];
            if(f.owner < 0 && f.droptime && gamemillis-f.droptime >= GAME(etfresetdelay))
            {
                etfstate::returnaffinity(i, gamemillis);
                sendf(-1, 1, "ri2", N_RESETAFFIN, i);
            }
            break;
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
                }
            }
        }
    }

    void regen(clientinfo *ci, int &total, int &amt, int &delay)
    {
        if(hasflaginfo && GAME(regenflag)) loopv(flags)
        {
            flag &f = flags[i];
            bool insidehome = (isetfhome(f, ci->team) && f.owner < 0 && !f.droptime && ci->state.o.dist(f.spawnloc) <= enttype[AFFINITY].radius*2.f);
            if(insidehome || (GAME(regenflag) == 2 && f.owner == ci->clientnum))
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
} etfmode;
