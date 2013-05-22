// server-side ai manager
namespace aiman
{
    int oldbotskillmin = -1, oldbotskillmax = -1, oldcoopskillmin = -1, oldcoopskillmax = -1, oldenemyskillmin = -1, oldenemyskillmax = -1,
        oldbotbalance = -2, oldnumplayers = -1, oldbotlimit = -1, oldbotoffset = 0, oldenemylimit = -1;
    float oldcoopbalance = -1, oldcoopmultibalance = -1;

    clientinfo *findaiclient(clientinfo *exclude = NULL)
    {
        clientinfo *least = NULL;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->clientnum < 0 || ci->state.aitype > AI_NONE || !ci->ready || ci == exclude) continue;
            if(!least || ci->bots.length() < least->bots.length()) least = ci;
        }
        return least;
    }

    void getskillrange(int type, int &m, int &n)
    {
        switch(type)
        {
            case AI_BOT:
                if(m_coop(gamemode, mutators))
                {
                    m = max(G(coopskillmax), G(coopskillmin));
                    n = min(G(coopskillmin), m);
                }
                else
                {
                    m = max(G(botskillmax), G(botskillmin));
                    n = min(G(botskillmin), m);
                }
                break;
            default:
                m = max(G(enemyskillmax), G(enemyskillmin));
                n = min(G(enemyskillmin), m);
                break;
        }
    }

    bool addai(int type, int ent, int skill)
    {
        int numbots = 0, numenemies = 0;
        loopv(clients)
        {
            if(type == AI_BOT && numbots >= G(botlimit)) return false;
            if(type >= AI_START && numenemies >= G(enemylimit)) return false;
            if(clients[i]->state.aitype == type)
            {
                clientinfo *ci = clients[i];
                if(ci->state.ownernum < 0)
                { // reuse a slot that was going to removed
                    clientinfo *owner = findaiclient();
                    if(!owner) return false;
                    ci->state.ownernum = owner->clientnum;
                    owner->bots.add(ci);
                    ci->state.aireinit = 1;
                    ci->state.aitype = type;
                    ci->state.aientity = ent;
                    return true;
                }
                if(type == AI_BOT) numbots++;
                if(type >= AI_START) numenemies++;
            }
        }
        if(type == AI_BOT && numbots >= G(botlimit)) return false;
        if(type >= AI_START && numenemies >= G(enemylimit)) return false;
        int cn = addclient(ST_REMOTE);
        if(cn >= 0)
        {
            clientinfo *ci = (clientinfo *)getinfo(cn);
            if(ci)
            {
                int s = skill, m = 100, n = 1;
                getskillrange(type, m, n);
                if(skill > m || skill < n) s = (m != n ? rnd(m-n) + n + 1 : m);
                ci->clientnum = cn;
                clientinfo *owner = findaiclient();
                ci->state.ownernum = owner ? owner->clientnum : -1;
                if(owner) owner->bots.add(ci);
                ci->state.aireinit = 2;
                ci->state.aitype = type;
                ci->state.aientity = ent;
                ci->state.skill = clamp(s, 1, 101);
                clients.add(ci);
                ci->state.lasttimeplayed = lastmillis;
                ci->state.colour = rnd(0xFFFFFF);
                ci->state.model = rnd(PLAYERTYPES);
                ci->state.setvanity(ci->state.model ? G(botfemalevanities) : G(botmalevanities)); // the first slot is special
                copystring(ci->name, aistyle[ci->state.aitype].name, MAXNAMELEN);
                if(ci->state.aitype == AI_BOT)
                {
                    const char *list = ci->state.model ? G(botfemalenames) : G(botmalenames);
                    int len = listlen(list);
                    if(len > 0)
                    {
                        int r = rnd(len);
                        char *name = indexlist(list, r);
                        if(name)
                        {
                            if(*name) copystring(ci->name, name, MAXNAMELEN);
                            delete[] name;
                        }
                    }
                }
                ci->state.state = CS_DEAD;
                ci->team = type == AI_BOT ? T_NEUTRAL : T_ENEMY;
                ci->online = ci->connected = ci->ready = true;
                return true;
            }
            delclient(cn);
        }
        return false;
    }

    void deleteai(clientinfo *ci)
    {
        if(ci->state.aitype == AI_NONE) return;
        int cn = ci->clientnum;
        loopv(clients) if(clients[i] != ci)
        {
            loopvk(clients[i]->state.fraglog) if(clients[i]->state.fraglog[k] == ci->clientnum)
                clients[i]->state.fraglog.remove(k--);
        }
        if(smode) smode->leavegame(ci, true);
        mutate(smuts, mut->leavegame(ci, true));
        ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        savescore(ci);
        sendf(-1, 1, "ri3", N_DISCONNECT, cn, DISC_NONE);
        clientinfo *owner = (clientinfo *)getinfo(ci->state.ownernum);
        if(owner) owner->bots.removeobj(ci);
        clients.removeobj(ci);
        delclient(cn);
        dorefresh = max(dorefresh, 1);
    }

    bool delai(int type, bool skip)
    {
        bool retry = false;
        loopvrev(clients) if(clients[i]->state.aitype == type && clients[i]->state.ownernum >= 0)
        {
            if(!skip || clients[i]->state.state == CS_DEAD || clients[i]->state.state == CS_WAITING)
            {
                deleteai(clients[i]);
                return true;
            }
            else if(skip && !retry) retry = true;
        }
        if(skip && retry) delai(type, false);
        return false;
    }

    void reinitai(clientinfo *ci)
    {
        if(ci->state.aitype == AI_NONE) return;
        if(ci->state.ownernum < 0) deleteai(ci);
        else if(ci->state.aireinit >= 1)
        {
            if(ci->state.aireinit == 2) loopk(W_MAX) loopj(2) ci->state.weapshots[k][j].reset();
            sendf(-1, 1, "ri6si3s", N_INITAI, ci->clientnum, ci->state.ownernum, ci->state.aitype, ci->state.aientity, ci->state.skill, ci->name, ci->team, ci->state.colour, ci->state.model, ci->state.vanity);
            if(ci->state.aireinit == 2)
            {
                waiting(ci, DROP_RESET);
                if(smode) smode->entergame(ci);
                mutate(smuts, mut->entergame(ci));
            }
            ci->state.aireinit = 0;
        }
    }

    void shiftai(clientinfo *ci, clientinfo *owner = NULL)
    {
        clientinfo *prevowner = (clientinfo *)getinfo(ci->state.ownernum);
        if(prevowner) prevowner->bots.removeobj(ci);
        if(!owner) { ci->state.aireinit = 0; ci->state.ownernum = -1; }
        else if(ci->state.ownernum != owner->clientnum) { ci->state.aireinit = 1; ci->state.ownernum = owner->clientnum; owner->bots.add(ci); }
    }

    void removeai(clientinfo *ci, bool complete)
    { // either schedules a removal, or someone else to assign to
        loopvrev(ci->bots) shiftai(ci->bots[i], complete ? NULL : findaiclient(ci));
    }

    bool reassignai(clientinfo *exclude)
    {
        clientinfo *hi = NULL, *lo = NULL;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->clientnum < 0 || ci->state.aitype > AI_NONE || !ci->ready || ci == exclude)
                continue;
            if(!lo || ci->bots.length() < lo->bots.length()) lo = ci;
            if(!hi || hi->bots.length() > hi->bots.length()) hi = ci;
        }
        if(hi && lo && hi->bots.length() - lo->bots.length() > 1)
        {
            loopvrev(hi->bots)
            {
                shiftai(hi->bots[i], lo);
                return true;
            }
        }
        return false;
    }

    void checksetup()
    {
        int m = 100, n = 1, numbots = 0, numenemies = 0;
        loopv(clients) if(clients[i]->state.aitype > AI_NONE && clients[i]->state.ownernum >= 0)
        {
            clientinfo *ci = clients[i];
            getskillrange(clients[i]->state.aitype, m, n);
            if(ci->state.skill > m || ci->state.skill < n)
            { // needs re-skilling
                ci->state.skill = (m != n ? rnd(m-n) + n + 1 : m);
                if(!ci->state.aireinit) ci->state.aireinit = 1;
            }
            if(ci->state.aitype == AI_BOT && ++numbots >= G(botlimit)) shiftai(ci, NULL);
            if(ci->state.aitype >= AI_START && ++numenemies >= G(enemylimit)) shiftai(ci, NULL);
        }

        int balance = 0, people = numclients(-1, true, -1), numt = numteams(gamemode, mutators);
#ifdef CAMPAIGN
        if(m_campaign(gamemode)) balance = G(campaignplayers); // campaigns strictly obeys player setting
        else
#endif
        if(m_coop(gamemode, mutators))
        {
            numt--; // filter out the human team
            balance = people+int(ceilf(people*numt*(m_multi(gamemode, mutators) ? G(coopmultibalance) : G(coopbalance))));
        }
        else if(m_bots(gamemode) && G(botlimit) > 0)
        {
            switch(G(botbalance))
            {
                case -1: balance = max(people, m_duel(gamemode, mutators) ? 2 : G(numplayers)); break; // use distributed map players
                case  0: balance = 0; break; // no bots
                default: balance = max(people, m_duel(gamemode, mutators) ? 2 : G(botbalance)); break; // balance to at least this
            }
            if(m_team(gamemode, mutators) && balance > 0)
            { // skew this if teams are unbalanced
                int plrs[T_TOTAL] = {0}, highest = -1; // we do this because humans can unbalance in odd ways
                loopv(clients) if(clients[i]->state.aitype == AI_NONE && clients[i]->team >= T_FIRST && isteam(gamemode, mutators, clients[i]->team, T_FIRST))
                {
                    int team = clients[i]->team-T_FIRST;
                    plrs[team]++;
                    if(highest < 0 || plrs[team] > plrs[highest]) highest = team;
                }
                if(highest >= 0)
                {
                    int bots = balance-people;
                    loopi(numt) if(i != highest && plrs[i] < plrs[highest]) loopj(plrs[highest]-plrs[i])
                    {
                        if(bots > 0) bots--;
                        else balance++;
                    }
                }
            }
        }
        balance += G(botoffset)*numt;
        int bots = balance-people;
        if(bots > G(botlimit)) balance -= bots-G(botlimit);
        if(balance > 0)
        {
            while(numclients(-1, true, AI_BOT) < balance) if(!addai(AI_BOT)) break;
            while(numclients(-1, true, AI_BOT) > balance) if(!delai(AI_BOT)) break;
            if(m_team(gamemode, mutators)) loopvrev(clients)
            {
                clientinfo *ci = clients[i];
                if(ci->state.aitype == AI_BOT && ci->state.ownernum >= 0)
                {
                    int teamb = chooseteam(ci, ci->team);
                    if(ci->team != teamb) setteam(ci, teamb, TT_DFINFO);
                }
            }
        }
        else clearai(1);
    }

    void checkenemies()
    {
        if(m_enemies(gamemode, mutators))
        {
            loopvj(sents) if(sents[j].type == ACTOR && sents[j].attrs[0] >= 0 && sents[j].attrs[0] < AI_TOTAL && gamemillis >= sents[j].millis && (sents[j].attrs[5] == triggerid || !sents[j].attrs[5]) && m_check(sents[j].attrs[3], sents[j].attrs[4], gamemode, mutators))
            {
                bool allow = true;
                loopv(clients) if(clients[i]->state.aitype < AI_START)
                {
                    float dist = clients[i]->state.o.dist(sents[j].o);
                    if(dist > G(enemyspawndistmax) || dist < G(enemyspawndistmin))
                    {
                        allow = false;
                        break;
                    }
                }
                if(!allow) continue;
                int count = 0, numenemies = 0;
                loopvrev(clients) if(clients[i]->state.aitype >= AI_START)
                {
                    if(clients[i]->state.aientity == j && ++count > G(enemybalance))
                    {
                        deleteai(clients[i]);
                        count--;
                        continue;
                    }
                    numenemies++;
                }
                if(numenemies < G(enemylimit) && count < G(enemybalance))
                {
                    int amt = min(G(enemybalance)-count, G(enemylimit)-numenemies);
                    loopk(amt) addai(sents[j].attrs[0]+AI_START, j);
                }
            }
        }
        else clearai(2);
    }

    void clearai(int type)
    { // clear and remove all ai immediately
        loopvrev(clients) if(!type || (type == 2 ? clients[i]->state.aitype >= AI_START : clients[i]->state.aitype == AI_BOT))
            deleteai(clients[i]);
    }

    void checkai()
    {
        if(!m_demo(gamemode) && numclients())
        {
            if(hasgameinfo && !interm && !gamewait)
            {
                if(!dorefresh)
                {
                    #define checkold(n) if(old##n != G(n)) { dorefresh = 1; old##n = G(n); }
                    if(m_enemies(gamemode, mutators))
                    {
                        checkold(enemyskillmin);
                        checkold(enemyskillmax);
                    }
                    if(m_coop(gamemode, mutators))
                    {
                        checkold(coopskillmin);
                        checkold(coopskillmax);
                        if(m_multi(gamemode, mutators)) { checkold(coopmultibalance); }
                        else { checkold(coopbalance); }
                    }
                    else
                    {
                        checkold(botskillmin);
                        checkold(botskillmax);
                        checkold(botbalance);
                    }
                    checkold(botlimit);
                    checkold(botoffset);
                    checkold(enemylimit);
                    checkold(numplayers);
                }
                if(dorefresh)
                {
                    dorefresh -= curtime;
                    if(dorefresh <= 0) { dorefresh = 0; checksetup(); }
                }
                checkenemies();
                loopvrev(clients) if(clients[i]->state.aitype > AI_NONE) reinitai(clients[i]);
                while(true) if(!reassignai()) break;
            }
        }
        else clearai();
    }
}
