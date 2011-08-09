void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen)
{
    char buf[2*sizeof(string)];
    formatstring(buf)("%d %d ", cn, sessionid);
    copystring(&buf[strlen(buf)], pwd);
    if(!hashstring(buf, result, maxlen)) *result = '\0';
}

bool checkpassword(clientinfo *ci, const char *wanted, const char *given)
{
    string hash;
    hashpassword(ci->clientnum, ci->sessionid, wanted, hash, sizeof(string));
    return !strcmp(hash, given);
}

namespace auth
{
    int lastconnect = 0, lastactivity = 0;
    uint nextauthreq = 1;

    clientinfo *findauth(uint id)
    {
        loopv(clients) if(clients[i]->authreq == id) return clients[i];
        return NULL;
    }

    void reqauth(clientinfo *ci)
    {
        if(!nextauthreq) nextauthreq = 1;
        ci->authreq = nextauthreq++;
        ci->authlevel = -1;
        requestmasterf("reqauth %u %s\n", ci->authreq, ci->authname);
        lastactivity = totalmillis;
        srvmsgft(ci->clientnum, CON_EVENT, "\fyplease wait, requesting credential match..");
    }

    bool tryauth(clientinfo *ci, const char *user)
    {
        if(!ci) return false;
        else if(!connectedmaster())
        {
            srvmsgft(ci->clientnum, CON_EVENT, "\fYnot connected to master server");
            return false;
        }
        else if(ci->authreq)
        {
            srvmsgft(ci->clientnum, CON_EVENT, "\fYwaiting for previous attempt..");
            return true;
        }
        filtertext(ci->authname, user, true, true, false, 100);
        reqauth(ci);
        return true;
    }

    void setmaster(clientinfo *ci, bool val, int flags = 0)
    {
        int privilege = ci->privilege;
        if(val)
        {
            if(ci->privilege >= flags) return;
            privilege = ci->privilege = flags;
        }
        else
        {
            if(!ci->privilege) return;
            ci->privilege = PRIV_NONE;
            int others = 0;
            loopv(clients) if(clients[i]->privilege >= PRIV_MASTER || clients[i]->local) others++;
            if(!others) mastermode = MM_OPEN;
        }
        srvoutf(-2, "\fy%s %s \fs\fc%s\fS", colorname(ci), val ? "claimed" : "relinquished", privname(privilege));
        masterupdate = true;
        if(paused)
        {
            int others = 0;
            loopv(clients) if(clients[i]->privilege >= (GAME(varslock) >= 2 ? PRIV_ADMIN : PRIV_MASTER) || clients[i]->local) others++;
            if(!others) setpause(false);
        }
    }

    int allowconnect(clientinfo *ci, const char *pwd = "", const char *authname = "")
    {
        if(ci->local) return DISC_NONE;
        if(m_local(gamemode)) return DISC_PRIVATE;
        if(ci->privilege >= PRIV_ADMIN) return DISC_NONE;
        if(*authname)
        {
            if(ci->connectauth) return DISC_NONE;
            if(tryauth(ci, authname))
            {
                ci->connectauth = true;
                return DISC_NONE;
            }
        }
        if(*pwd)
        {
            if(adminpass[0] && checkpassword(ci, adminpass, pwd))
            {
                if(GAME(automaster)) setmaster(ci, true, PRIV_ADMIN);
                return DISC_NONE;
            }
            if(serverpass[0] && checkpassword(ci, serverpass, pwd)) return DISC_NONE;
        }
        if(numclients() >= GAME(serverclients)) return DISC_MAXCLIENTS;
        uint ip = getclientip(ci->clientnum);
        if(!ci->privilege && !checkipinfo(allows, ip))
        {
            if(mastermode >= MM_PRIVATE || serverpass[0]) return DISC_PRIVATE;
            if(checkipinfo(bans, ip)) return DISC_IPBAN;
        }
        return DISC_NONE;
    }

    void authfailed(uint id)
    {
        clientinfo *ci = findauth(id);
        if(!ci) return;
        ci->authreq = ci->authname[0] = 0;
        ci->authlevel = -1;
        srvmsgft(ci->clientnum, CON_EVENT, "\fYauthority request failed, please check your credentials");
        if(ci->connectauth)
        {
            ci->connectauth = false;
            int disc = allowconnect(ci);
            if(disc) { disconnect_client(id, disc); return; }
        }
    }

    void authsucceeded(uint id, const char *name, const char *flags)
    {
        clientinfo *ci = findauth(id);
        if(!ci) return;
        ci->authreq = 0;
        int n = -1;
        for(const char *c = flags; *c; c++) switch(*c)
        {
            case 'a': n = PRIV_ADMIN; break;
            case 'm': n = PRIV_MASTER; break;
            case 'u': n = PRIV_NONE; break;
        }
        ci->authlevel = n;
        srvoutf(-2, "\fy%s identified as '\fs\fc%s\fS'", colorname(ci), name);
        if(ci->authlevel > PRIV_NONE && GAME(automaster)) setmaster(ci, true, ci->authlevel);
        if(ci->connectauth)
        {
            ci->connectauth = false;
            if(ci->authlevel < 0)
            {
                int disc = allowconnect(ci);
                if(disc) { disconnect_client(id, disc); return; }
            }
        }
    }

    void authchallenged(uint id, const char *val)
    {
        clientinfo *ci = findauth(id);
        if(!ci) return;
        sendf(ci->clientnum, 1, "riis", N_AUTHCHAL, id, val);
    }

    void answerchallenge(clientinfo *ci, uint id, char *val)
    {
        if(ci->authreq != id) return;
        for(char *s = val; *s; s++)
        {
            if(!isxdigit(*s)) { *s = '\0'; break; }
        }
        requestmasterf("confauth %u %s\n", id, val);
        lastactivity = totalmillis;
    }

    void processinput(const char *p)
    {
        const int MAXWORDS = 8;
        char *w[MAXWORDS];
        int numargs = MAXWORDS;
        loopi(MAXWORDS)
        {
            w[i] = (char *)"";
            if(i > numargs) continue;
            char *s = parsetext(p);
            if(s) w[i] = s;
            else numargs = i;
        }
        if(!strcmp(w[0], "error")) conoutf("master server error: %s", w[1]);
        else if(!strcmp(w[0], "echo")) conoutf("master server reply: %s", w[1]);
        else if(!strcmp(w[0], "failauth")) authfailed((uint)(atoi(w[1])));
        else if(!strcmp(w[0], "succauth")) authsucceeded((uint)(atoi(w[1])), w[2], w[3]);
        else if(!strcmp(w[0], "chalauth")) authchallenged((uint)(atoi(w[1])), w[2]);
        else if(!strcmp(w[0], "ban") || !strcmp(w[0], "allow"))
        {
            ipinfo &p = (strcmp(w[0], "ban") ? allows : bans).add();
            p.ip = (uint)(atoi(w[1]));
            p.mask = (uint)(atoi(w[2]));
            p.time = -2; // master info
        }
        //else if(w[0]) conoutf("authserv sent invalid command: %s", w[0]);
        loopj(numargs) if(w[j]) delete[] w[j];
    }

    void regserver()
    {
        loopvrev(bans) if(bans[i].time == -2) bans.remove(i);
        loopvrev(allows) if(allows[i].time == -2) allows.remove(i);
        conoutf("updating master server");
        requestmasterf("server %d\n", serverport);
        lastactivity = totalmillis;
    }

    void update()
    {
        if(servertype < 2)
        {
            if(connectedmaster()) disconnectmaster();
            return;
        }
        else if(!connectedmaster() && (!lastconnect || totalmillis-lastconnect > 60*1000))
        {
            lastconnect = totalmillis;
            if(connectmaster() == ENET_SOCKET_NULL) return;
            regserver();
            loopv(clients) if(clients[i]->authreq) reqauth(clients[i]);
        }

        if(totalmillis-lastactivity > 30*60*1000) regserver();
    }
}

void disconnectedmaster()
{
}

void processmasterinput(const char *cmd, int cmdlen, const char *args)
{
    auth::processinput(cmd);
}



