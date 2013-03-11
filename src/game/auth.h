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

struct localop
{
    char *name, *flags;

    localop() : name(NULL), flags(NULL) {}
    localop(const char *n, const char *f) : name(newstring(n)), flags(newstring(f)) {}
    ~localop()
    {
        if(name) delete[] name;
        if(flags) delete[] flags;
    }
};
vector<localop> localops;

void localopreset()
{
    loopvrev(localops) localops.remove(i);
}
ICOMMAND(0, resetlocalop, "", (), localopreset());

void localopadd(const char *name, const char *flags)
{
    if(!name || !flags) return;
    localop &o = localops.add();
    o.name = newstring(name);
    o.flags = newstring(flags);
}
ICOMMAND(0, addlocalop, "ss", (char *n, char *f), localopadd(n, f));

namespace auth
{
    int lastconnect = 0, lastactivity = 0;
    uint nextauthreq = 1;

    clientinfo *findauth(uint id)
    {
        loopv(clients) if(clients[i]->authreq == id) return clients[i];
        loopv(connects) if(connects[i]->authreq == id) return connects[i];
        return NULL;
    }

    void reqauth(clientinfo *ci)
    {
        if(!nextauthreq) nextauthreq = 1;
        ci->authreq = nextauthreq++;
        requestmasterf("reqauth %u %s\n", ci->authreq, ci->authname);
        lastactivity = totalmillis;
        srvmsgft(ci->clientnum, CON_EVENT, "\fyplease wait, requesting credential match..");
    }

    bool tryauth(clientinfo *ci, const char *user)
    {
        if(!ci) return false;
        else if(!connectedmaster())
        {
            srvmsgft(ci->clientnum, CON_EVENT, "\founable to verify, not connected to master server");
            return false;
        }
        else if(ci->authreq)
        {
            srvmsgft(ci->clientnum, CON_EVENT, "\foplease wait, still processing previous attempt..");
            return true;
        }
        filtertext(ci->authname, user, true, true, false, 100);
        reqauth(ci);
        return true;
    }

    void setprivilege(clientinfo *ci, bool val, int flags = 0, bool authed = false, bool local = true)
    {
        int privilege = ci->privilege;
        mkstring(msg);
        if(val)
        {
            if(ci->privilege >= flags) return;
            privilege = ci->privilege = flags;
            if(authed)
            {
                formatstring(msg)("\fy%s identified as \fs\fc%s\fS", colourname(ci), ci->authname);
                if(ci->privilege > PRIV_PLAYER)
                {
                    defformatstring(msgx)(" (\fs\fc%s\fS)", privname(privilege));
                    concatstring(msg, msgx);
                }
                copystring(ci->handle, ci->authname);
            }
            else formatstring(msg)("\fy%s elevated to \fs\fc%s\fS", colourname(ci), privname(privilege, true));
            if(local) concatstring(msg, " [\fs\falocal\fS]");
        }
        else
        {
            if(!ci->privilege) return;
            ci->privilege = PRIV_NONE;
            ci->handle[0] = 0;
            int others = 0;
            loopv(clients) if(clients[i]->privilege >= PRIV_MODERATOR || clients[i]->local) others++;
            if(!others) mastermode = MM_OPEN;
            if(privilege >= PRIV_ELEVATED) formatstring(msg)("\fy%s is no longer \fs\fc%s\fS", colourname(ci), privname(privilege, true));
        }
        if(*msg) srvoutforce(ci, -2, "%s", msg);
        privupdate = true;
        if(paused)
        {
            int others = 0;
            loopv(clients) if(clients[i]->privilege >= PRIV_ADMINISTRATOR || clients[i]->local) others++;
            if(!others) setpause(false);
        }
    }

    int allowconnect(clientinfo *ci, bool connecting = true, const char *pwd = "", const char *authname = "")
    {
        if(ci->local) return DISC_NONE;
        if(m_local(gamemode)) return DISC_PRIVATE;
        int minpriv = max(int(PRIV_ELEVATED), G(connectlock));
        if(haspriv(ci, minpriv)) return DISC_NONE;
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
                if(G(autoadmin)) setprivilege(ci, true, PRIV_ADMINISTRATOR);
                return DISC_NONE;
            }
            if(serverpass[0] && checkpassword(ci, serverpass, pwd)) return DISC_NONE;
        }
        // above here are short circuits
        if(numclients() >= G(serverclients)) return DISC_MAXCLIENTS;
        uint ip = getclientip(ci->clientnum);
        if(!ip || !checkipinfo(control, ipinfo::ALLOW, ip))
        {
            if(mastermode >= MM_PRIVATE || serverpass[0] || (G(connectlock) && !haspriv(ci, G(connectlock)))) return DISC_PRIVATE;
            if(checkipinfo(control, ipinfo::BAN, ip)) return DISC_IPBAN;
        }
        return DISC_NONE;
    }

    void authfailed(uint id)
    {
        clientinfo *ci = findauth(id);
        if(!ci) return;
        ci->authreq = ci->authname[0] = 0;
        srvmsgft(ci->clientnum, CON_EVENT, "\foauthority request failed, please check your credentials");
        if(ci->connectauth)
        {
            ci->connectauth = false;
            int disc = allowconnect(ci, false);
            if(disc) { disconnect_client(ci->clientnum, disc); return; }
            connected(ci);
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
            case 'c': case 'C': n = PRIV_CREATOR; break;
            case 'd': case 'D': n = PRIV_DEVELOPER; break;
            case 'a': case 'A': n = PRIV_ADMINISTRATOR; break;
            case 'o': case 'O': n = PRIV_OPERATOR; break;
            case 'm': case 'M': n = PRIV_MODERATOR; break;
            case 's': case 'S': n = PRIV_SUPPORTER; break;
            case 'u': case 'U': n = PRIV_PLAYER; break;
            default: break;
        }
        bool local = false;
        loopv(localops) if(!strcmp(localops[i].name, name))
        {
            int o = -1;
            for(const char *c = localops[i].flags; *c; c++) switch(*c)
            {
                case 'a': case 'A': o = PRIV_ADMINISTRATOR; break;
                case 'o': case 'O': o = PRIV_OPERATOR; break;
                case 'm': case 'M': o = PRIV_MODERATOR; break;
                default: break;
            }
            if(o > n)
            {
                n = o;
                local = true;
            }
        }
        if(n > PRIV_NONE) setprivilege(ci, true, n, true, local);
        else ci->authname[0] = 0;
        if(ci->connectauth)
        {
            ci->connectauth = false;
            int disc = allowconnect(ci, false);
            if(disc) { disconnect_client(ci->clientnum, disc); return; }
            connected(ci);
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
        else loopj(ipinfo::MAXTYPES) if(!strcmp(w[0], ipinfotypes[j]))
        {
            ipinfo &p = control.add();
            p.ip = uint(atoi(w[1]));
            p.mask = uint(atoi(w[2]));
            p.type = j;
            p.flag = ipinfo::GLOBAL; // master info
            p.time = totalmillis ? totalmillis : 1;
            updatecontrols = true;
            break;
        }
        loopj(numargs) if(w[j]) delete[] w[j];
    }

    void regserver()
    {
        loopvrev(control) if(control[i].flag == ipinfo::GLOBAL) control.remove(i);
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
