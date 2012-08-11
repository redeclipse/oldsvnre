// console.cpp: the console buffer, its display, and command line control

#include "engine.h"

vector<cline> conlines;
int commandmillis = -1;
string commandbuf;
char *commandaction = NULL, *commandicon = NULL;
enum { CF_COMPLETE = 1<<0, CF_EXECUTE = 1<<1 };
int commandflags = 0, commandpos = -1, commandcolour = 0;

void conline(int type, const char *sf, int n)
{
    cline cl;
    cl.type = type;
    cl.cref = conlines.length() > MAXSTRLEN ? conlines.pop().cref : newstringbuf("");
    cl.reftime = cl.outtime = totalmillis;
    conlines.insert(n, cl);

    int c = 0;
    #define addcref(d) { cl.cref[c] = d; c++; }
    if(n)
    {
        addcref(' ');
        addcref(' ');
        concatstring(cl.cref, sf);
    }
    addcref(0);
    concatstring(cl.cref, sf);

    if(n)
    {
        string sd;
        loopj(2)
        {
            int off = n+j;
            if(conlines.inrange(off))
            {
                if(j) formatstring(sd)("%s\fs", conlines[off].cref);
                else formatstring(sd)("\fS%s", conlines[off].cref);
                copystring(conlines[off].cref, sd);
            }
        }
    }
}

// keymap is defined externally in keymap.cfg
struct keym
{
    enum
    {
        ACTION_DEFAULT = 0,
        ACTION_SPECTATOR,
        ACTION_EDITING,
        ACTION_WAITING,
        NUMACTIONS
    };

    int code;
    char *name;
    char *actions[NUMACTIONS];
    bool pressed, persist[NUMACTIONS];

    keym() : code(-1), name(NULL), pressed(false) { loopi(NUMACTIONS) { actions[i] = newstring(""); persist[i] = false; } }
    ~keym() { DELETEA(name); loopi(NUMACTIONS) { DELETEA(actions[i]); persist[i] = false; } }
};

/*
extern keym *keypressed;
extern char *keyaction;
extern keym *findbind(const char *key);
extern int findactionkey(const char *action, int which, int num);

extern const char *retbind(const char *key, int which);
extern const char *retbindaction(const char *action, int which, int num);
*/

hashtable<int, keym> keyms(128);

void keymap(int *code, char *key)
{
    if(identflags&IDF_WORLD) { conoutf("\frcannot override keymap"); return; }
    keym &km = keyms[*code];
    km.code = *code;
    DELETEA(km.name);
    km.name = newstring(key);
}

COMMAND(0, keymap, "is");

keym *keypressed = NULL;
char *keyaction = NULL;

const char *getkeyname(int code)
{
    keym *km = keyms.access(code);
    return km ? km->name : NULL;
}

#define ADDSEP(comma, conj) do { \
    if(pretty && *pretty) \
    { \
        names.put("\fs", 2); \
        names.put(pretty, strlen(pretty)); \
        if(comma) names.add(comma); \
        names.add(' '); \
        if((conj)[0]) \
        { \
            names.put(conj, strlen(conj)); \
            names.add(' '); \
        } \
        names.put("\fS", 2); \
    } \
    else if(sep && *sep) names.put(sep, strlen(sep)); \
    else names.add(' '); \
} while(0)

void searchbindlist(const char *action, int type, int limit, const char *sep, const char *pretty, vector<char> &names)
{
    const char *name1 = NULL, *name2 = NULL, *lastname = NULL;
    int found = 0;
    enumerate(keyms, keym, km,
    {
        char *act = ((!km.actions[type] || !*km.actions[type]) && type ? km.actions[keym::ACTION_DEFAULT] : km.actions[type]);
        if(!strcmp(act, action))
        {
            if(!name1) name1 = km.name;
            else if(!name2) name2 = km.name;
            else
            {
                if(lastname)
                {
                    ADDSEP(',', "");
                    names.put(lastname, strlen(lastname));
                }
                else
                {
                    names.put(name1, strlen(name1));
                    ADDSEP(',', "");
                    names.put(name2, strlen(name2));
                }
                lastname = km.name;
            }
            ++found;
            if(limit > 0 && found >= limit) break;
        }
    });
    if(lastname)
    {
        ADDSEP(',', sep);
        names.put(lastname, strlen(lastname));
    }
    else
    {
        if(name1) names.put(name1, strlen(name1));
        if(name2)
        {
            ADDSEP('\0', sep);
            names.put(name2, strlen(name2));
        }
    }
    names.add('\0');
}

const char *searchbind(const char *action, int type)
{
    enumerate(keyms, keym, km,
    {
        char *act = ((!km.actions[type] || !*km.actions[type]) && type ? km.actions[keym::ACTION_DEFAULT] : km.actions[type]);
        if(!strcmp(act, action)) return km.name;
    });
    return NULL;
}

void getkeypressed(int limit, const char *sep, const char *pretty, vector<char> &names)
{
    const char *name1 = NULL, *name2 = NULL, *lastname = NULL;
    int found = 0;
    enumerate(keyms, keym, km,
    {
        if(km.pressed)
        {
            if(!name1) name1 = km.name;
            else if(!name2) name2 = km.name;
            else
            {
                if(lastname)
                {
                    ADDSEP(',', "");
                    names.put(lastname, strlen(lastname));
                }
                else
                {
                    names.put(name1, strlen(name1));
                    ADDSEP(',', "");
                    names.put(name2, strlen(name2));
                }
                lastname = km.name;
            }
            ++found;
            if(limit > 0 && found >= limit) break;
        }
    });
    if(lastname)
    {
        ADDSEP(',', sep);
        names.put(lastname, strlen(lastname));
    }
    else
    {
        if(name1) names.put(name1, strlen(name1));
        if(name2)
        {
            ADDSEP('\0', sep);
            names.put(name2, strlen(name2));
        }
    }
    names.add('\0');
}

int findkeycode(char *key)
{
    enumerate(keyms, keym, km,
    {
        if(!strcasecmp(km.name, key)) return i;
    });
    return 0;
}

keym *findbind(char *key)
{
    enumerate(keyms, keym, km,
    {
        if(!strcasecmp(km.name, key)) return &km;
    });
    return NULL;
}

void getbind(char *key, int type)
{
    keym *km = findbind(key);
    result(km ? ((!km->actions[type] || !*km->actions[type]) && type ? km->actions[keym::ACTION_DEFAULT] : km->actions[type]) : "");
}

int changedkeys = 0;

void bindkey(char *key, char *action, int state, const char *cmd)
{
    if(identflags&IDF_WORLD) { conoutf("\frcannot override %s \"%s\"", cmd, key); return; }
    keym *km = findbind(key);
    if(!km) { conoutf("\frunknown key \"%s\"", key); return; }
    char *&binding = km->actions[state];
    bool *persist = &km->persist[state];
    if(!keypressed || keyaction!=binding) delete[] binding;
    // trim white-space to make searchbinds more reliable
    while(iscubespace(*action)) action++;
    int len = strlen(action);
    while(len>0 && iscubespace(action[len-1])) len--;
    binding = newstring(action, len);
    *persist = interactive;
    changedkeys = totalmillis;
}

ICOMMAND(0, bind,     "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_DEFAULT, "bind"));
ICOMMAND(0, specbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_SPECTATOR, "specbind"));
ICOMMAND(0, editbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_EDITING, "editbind"));
ICOMMAND(0, waitbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_WAITING, "waitbind"));
ICOMMAND(0, getbind,     "s", (char *key), getbind(key, keym::ACTION_DEFAULT));
ICOMMAND(0, getspecbind, "s", (char *key), getbind(key, keym::ACTION_SPECTATOR));
ICOMMAND(0, geteditbind, "s", (char *key), getbind(key, keym::ACTION_EDITING));
ICOMMAND(0, getwaitbind, "s", (char *key), getbind(key, keym::ACTION_WAITING));
ICOMMAND(0, searchbinds,     "siss", (char *action, int *limit, char *sep, char *pretty), { vector<char> list; searchbindlist(action, keym::ACTION_DEFAULT, max(*limit, 0), sep, pretty, list); result(list.getbuf()); });
ICOMMAND(0, searchspecbinds, "siss", (char *action, int *limit, char *sep, char *pretty), { vector<char> list; searchbindlist(action, keym::ACTION_SPECTATOR, max(*limit, 0), sep, pretty, list); result(list.getbuf()); });
ICOMMAND(0, searcheditbinds, "siss", (char *action, int *limit, char *sep, char *pretty), { vector<char> list; searchbindlist(action, keym::ACTION_EDITING, max(*limit, 0), sep, pretty, list); result(list.getbuf()); });
ICOMMAND(0, searchwaitbinds, "siss", (char *action, int *limit, char *sep, char *pretty), { vector<char> list; searchbindlist(action, keym::ACTION_WAITING, max(*limit, 0), sep, pretty, list); result(list.getbuf()); });

ICOMMAND(0, keyspressed, "iss", (int *limit, char *sep, char *pretty), { vector<char> list; getkeypressed(max(*limit, 0), sep, pretty, list); result(list.getbuf()); });

void inputcommand(char *init, char *action = NULL, char *icon = NULL, int colour = 0, char *flags = NULL) // turns input to the command line on or off
{
    commandmillis = init ? totalmillis : -totalmillis;
    SDL_EnableUNICODE(commandmillis > 0 ? 1 : 0);
    keyrepeat(commandmillis > 0);
    copystring(commandbuf, init ? init : "");
    DELETEA(commandaction);
    DELETEA(commandicon);
    commandpos = -1;
    if(action && action[0]) commandaction = newstring(action);
    if(icon && icon[0]) commandicon = newstring(icon);
    commandcolour = colour;
    commandflags = 0;
    if(flags) while(*flags) switch(*flags++)
    {
        case 'c': commandflags |= CF_COMPLETE; break;
        case 'x': commandflags |= CF_EXECUTE; break;
        case 's': commandflags |= CF_COMPLETE|CF_EXECUTE; break;
    }
    else if(init) commandflags |= CF_COMPLETE|CF_EXECUTE;
}

ICOMMAND(0, saycommand, "C", (char *init), inputcommand(init));
ICOMMAND(0, inputcommand, "sssis", (char *init, char *action, char *icon, int *colour, char *flags), inputcommand(init, action, icon, *colour, flags));

#if !defined(WIN32) && !defined(__APPLE__)
#include <X11/Xlib.h>
#endif

void pasteconsole()
{
#ifdef WIN32
    UINT fmt = CF_UNICODETEXT;
    if(!IsClipboardFormatAvailable(fmt))
    {
        fmt = CF_TEXT;
        if(!IsClipboardFormatAvailable(fmt)) return;
    }
    if(!OpenClipboard(NULL)) return;
    HANDLE h = GetClipboardData(fmt);
    size_t commandlen = strlen(commandbuf);
    int cblen = int(GlobalSize(h)), decoded = 0;
    ushort *cb = (ushort *)GlobalLock(h);
    switch(fmt)
    {
        case CF_UNICODETEXT:
            decoded = min(int(sizeof(commandbuf)-1-commandlen), cblen/2);
            loopi(decoded) commandbuf[commandlen++] = uni2cube(cb[i]);
            break;
        case CF_TEXT:
            decoded = min(int(sizeof(commandbuf)-1-commandlen), cblen);
            memcpy(&commandbuf[commandlen], cb, decoded);
            break;
    }
    commandbuf[commandlen + decoded] = '\0';
    GlobalUnlock(cb);
    CloseClipboard();
#elif defined(__APPLE__)
    extern char *mac_pasteconsole(int *cblen);
    int cblen = 0;
    uchar *cb = (uchar *)mac_pasteconsole(&cblen);
    if(!cb) return;
    size_t commandlen = strlen(commandbuf);
    int decoded = decodeutf8((uchar *)&commandbuf[commandlen], int(sizeof(commandbuf)-1-commandlen), cb, cblen);
    commandbuf[commandlen + decoded] = '\0';
    free(cb);
    #else
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    wminfo.subsystem = SDL_SYSWM_X11;
    if(!SDL_GetWMInfo(&wminfo)) return;
    int cbsize;
    uchar *cb = (uchar *)XFetchBytes(wminfo.info.x11.display, &cbsize);
    if(!cb || !cbsize) return;
    size_t commandlen = strlen(commandbuf);
    for(uchar *cbline = cb, *cbend; commandlen + 1 < sizeof(commandbuf) && cbline < &cb[cbsize]; cbline = cbend + 1)
    {
        cbend = (uchar *)memchr(cbline, '\0', &cb[cbsize] - cbline);
        if(!cbend) cbend = &cb[cbsize];
        int cblen = int(cbend-cbline), commandmax = int(sizeof(commandbuf)-1-commandlen);
        loopi(cblen) if((cbline[i]&0xC0) == 0x80)
        {
            commandlen += decodeutf8((uchar *)&commandbuf[commandlen], commandmax, cbline, cblen);
            goto nextline;
        }
        cblen = min(cblen, commandmax);
        loopi(cblen) commandbuf[commandlen++] = uni2cube(*cbline++);
    nextline:
        commandbuf[commandlen] = '\n';
        if(commandlen + 1 < sizeof(commandbuf) && cbend < &cb[cbsize]) ++commandlen;
        commandbuf[commandlen] = '\0';
    }
    XFree(cb);
#endif
}

SVAR(0, commandbuffer, "");

struct hline
{
    char *buf, *action, *icon;
    int colour, flags;

    hline() : buf(NULL), action(NULL), icon(NULL), colour(0), flags(0) {}
    ~hline()
    {
        DELETEA(buf);
        DELETEA(action);
        DELETEA(icon);
    }

    void restore()
    {
        copystring(commandbuf, buf);
        if(commandpos >= (int)strlen(commandbuf)) commandpos = -1;
        DELETEA(commandaction);
        DELETEA(commandicon);
        if(action) commandaction = newstring(action);
        if(icon) commandicon = newstring(icon);
        commandcolour = colour;
        commandflags = flags;
    }

    bool shouldsave()
    {
        return strcmp(commandbuf, buf) ||
               (commandaction ? !action || strcmp(commandaction, action) : action!=NULL) ||
               (commandicon ? !icon || strcmp(commandicon, icon) : icon!=NULL) ||
               commandcolour != colour ||
               commandflags != flags;
    }

    void save()
    {
        buf = newstring(commandbuf);
        if(commandaction) action = newstring(commandaction);
        if(commandicon) icon = newstring(commandicon);
        colour = commandcolour;
        flags = commandflags;
    }

    void run()
    {
        if(flags&CF_EXECUTE && buf[0]=='/') execute(buf+1); // above all else
        else if(action)
        {
            setsvar("commandbuffer", buf, true);
            execute(action);
        }
        else client::toserver(0, buf);
    }
};
vector<hline *> history;
int histpos = 0;

VAR(IDF_PERSIST, maxhistory, 0, 1000, 10000);

void history_(int *n)
{
    static bool inhistory = false;
    if(!inhistory && history.inrange(*n))
    {
        inhistory = true;
        history[history.length()-*n-1]->run();
        inhistory = false;
    }
}

COMMANDN(0, history, history_, "i");

struct releaseaction
{
    keym *key;
    char *action;
};
vector<releaseaction> releaseactions;

const char *addreleaseaction(char *s)
{
    if(!keypressed) return NULL;
    releaseaction &ra = releaseactions.add();
    ra.key = keypressed;
    ra.action = s;
    return keypressed->name;
}

void onrelease(const char *s)
{
    addreleaseaction(newstring(s));
}

COMMAND(0, onrelease, "s");

void execbind(keym &k, bool isdown)
{
    loopv(releaseactions)
    {
        releaseaction &ra = releaseactions[i];
        if(ra.key==&k)
        {
            if(!isdown) execute(ra.action);
            delete[] ra.action;
            releaseactions.remove(i--);
        }
    }
    if(isdown)
    {

        int state = keym::ACTION_DEFAULT;
        switch(client::state())
        {
            case CS_ALIVE: case CS_DEAD: default: break;
            case CS_SPECTATOR: state = keym::ACTION_SPECTATOR; break;
            case CS_EDITING: state = keym::ACTION_EDITING; break;
            case CS_WAITING: state = keym::ACTION_WAITING; break;
        }
        char *&action = k.actions[state][0] ? k.actions[state] : k.actions[keym::ACTION_DEFAULT];
        keyaction = action;
        keypressed = &k;
        execute(keyaction);
        keypressed = NULL;
        if(keyaction!=action) delete[] keyaction;
    }
    k.pressed = isdown;
}

void consolekey(int code, bool isdown, int cooked)
{
    if(isdown)
    {
        switch(code)
        {
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                break;

            case SDLK_HOME:
                if(strlen(commandbuf)) commandpos = 0;
                break;

            case SDLK_END:
                commandpos = -1;
                break;

            case SDLK_DELETE:
            {
                int len = (int)strlen(commandbuf);
                if(commandpos<0) break;
                memmove(&commandbuf[commandpos], &commandbuf[commandpos+1], len - commandpos);
                resetcomplete();
                if(commandpos>=len-1) commandpos = -1;
                break;
            }

            case SDLK_BACKSPACE:
            {
                int len = (int)strlen(commandbuf), i = commandpos>=0 ? commandpos : len;
                if(i<1) break;
                memmove(&commandbuf[i-1], &commandbuf[i], len - i + 1);
                resetcomplete();
                if(commandpos>0) commandpos--;
                else if(!commandpos && len<=1) commandpos = -1;
                break;
            }

            case SDLK_LEFT:
                if(commandpos>0) commandpos--;
                else if(commandpos<0) commandpos = (int)strlen(commandbuf)-1;
                break;

            case SDLK_RIGHT:
                if(commandpos>=0 && ++commandpos>=(int)strlen(commandbuf)) commandpos = -1;
                break;

            case SDLK_UP:
                if(histpos > history.length()) histpos = history.length();
                if(histpos > 0) history[--histpos]->restore();
                break;

            case SDLK_DOWN:
                if(histpos + 1 < history.length()) history[++histpos]->restore();
                break;

            case SDLK_TAB:
                if(commandflags&CF_COMPLETE)
                {
                    complete(commandbuf, commandflags&CF_EXECUTE ? "/" : NULL);
                    if(commandpos>=0 && commandpos>=(int)strlen(commandbuf)) commandpos = -1;
                }
                break;

            case SDLK_f:
                if(SDL_GetModState()&MOD_KEYS) cooked = '\f';
                // fall through

            case SDLK_v:
                if(SDL_GetModState()&MOD_KEYS)
                {
                    pasteconsole();
                    break;
                }
                // fall through

            case SDLK_F4:
                if(SDL_GetModState()&MOD_KEYS)
                {
                    quit();
                    break;
                }
                // fall through

            default:
                resetcomplete();
                if(cooked)
                {
                    size_t len = (int)strlen(commandbuf);
                    if(len+1<sizeof(commandbuf))
                    {
                        if(commandpos<0) commandbuf[len] = cooked;
                        else
                        {
                            memmove(&commandbuf[commandpos+1], &commandbuf[commandpos], len - commandpos);
                            commandbuf[commandpos++] = cooked;
                        }
                        commandbuf[len+1] = '\0';
                    }
                }
                break;
        }
    }
    else
    {
        if(code==SDLK_RETURN || code==SDLK_KP_ENTER)
        {
            hline *h = NULL;
            if(commandbuf[0])
            {
                if(history.empty() || history.last()->shouldsave())
                {
                    if(maxhistory && history.length() >= maxhistory)
                    {
                        loopi(history.length()-maxhistory+1) delete history[i];
                        history.remove(0, history.length()-maxhistory+1);
                    }
                    history.add(h = new hline)->save();
                }
                else h = history.last();
            }
            histpos = history.length();
            inputcommand(NULL);
            if(h)
            {
                interactive = true;
                h->run();
                interactive = false;
            }
        }
        else if(code==SDLK_ESCAPE || code < 0)
        {
            histpos = history.length();
            inputcommand(NULL);
        }
    }
}

void keypress(int code, bool isdown, int cooked)
{
    keym *haskey = keyms.access(code);
    if(haskey && haskey->pressed) execbind(*haskey, isdown); // allow pressed keys to release
    else if(commandmillis > 0) consolekey(code, isdown, cooked);
    else if(!hud::keypress(code, isdown, cooked) && haskey) execbind(*haskey, isdown);
}

char *getcurcommand()
{
    return commandmillis > 0 ? commandbuf : (char *)NULL;
}

void clear_console()
{
    keyms.clear();
}

static inline bool sortbinds(keym *x, keym *y)
{
    return strcmp(x->name, y->name) < 0;
}

void writebinds(stream *f)
{
    static const char *cmds[4] = { "bind", "specbind", "editbind", "waitbind" };
    vector<keym *> binds;
    enumerate(keyms, keym, km, binds.add(&km));
    binds.sort(sortbinds);
    loopj(4)
    {
        bool found = false;
        loopv(binds)
        {
            keym &km = *binds[i];
            if(km.persist[j] && *km.actions[j])
            {
                if(validateblock(km.actions[j])) f->printf("%s %s [%s]\n", cmds[j], escapestring(km.name), km.actions[j]);
                else f->printf("%s %s %s\n", cmds[j], escapestring(km.name), escapestring(km.actions[j]));
                found = true;
            }
        }
        if(found) f->printf("\n");
    }
}

// tab-completion of all idents and base maps

enum { FILES_DIR = 0, FILES_LIST };

struct fileskey
{
    int type;
    const char *dir, *ext;

    fileskey() {}
    fileskey(int type, const char *dir, const char *ext) : type(type), dir(dir), ext(ext) {}
};

struct filesval
{
    int type;
    char *dir, *ext;
    vector<char *> files;
    int millis;

    filesval(int type, const char *dir, const char *ext) : type(type), dir(newstring(dir)), ext(ext && ext[0] ? newstring(ext) : NULL), millis(-1) {}
    ~filesval() { DELETEA(dir); DELETEA(ext); loopv(files) DELETEA(files[i]); files.shrink(0); }

    static bool comparefiles(const char *x, const char *y) { return strcmp(x, y) < 0; }

    void update()
    {
        if(type!=FILES_DIR || millis >= commandmillis) return;
        files.deletearrays();
        listfiles(dir, ext, files);
        files.sort(comparefiles);
        loopv(files) if(i && !strcmp(files[i], files[i-1])) delete[] files.remove(i--);
        millis = totalmillis;
    }
};

static inline bool htcmp(const fileskey &x, const fileskey &y)
{
    return x.type==y.type && !strcmp(x.dir, y.dir) && (x.ext == y.ext || (x.ext && y.ext && !strcmp(x.ext, y.ext)));
}

static inline uint hthash(const fileskey &k)
{
    return hthash(k.dir);
}

static hashtable<fileskey, filesval *> completefiles;
static hashtable<char *, filesval *> completions;

int completesize = 0;
string lastcomplete;

void resetcomplete() { completesize = 0; }

void addcomplete(char *command, int type, char *dir, char *ext)
{
    if(identflags&IDF_WORLD)
    {
        conoutf("\frcannot override complete %s", command);
        return;
    }
    if(!dir[0])
    {
        filesval **hasfiles = completions.access(command);
        if(hasfiles) *hasfiles = NULL;
        return;
    }
    if(type==FILES_DIR)
    {
    int dirlen = (int)strlen(dir);
    while(dirlen > 0 && (dir[dirlen-1] == '/' || dir[dirlen-1] == '\\'))
        dir[--dirlen] = '\0';
        if(ext)
        {
    if(strchr(ext, '*')) ext[0] = '\0';
            if(!ext[0]) ext = NULL;
        }
    }
    fileskey key(type, dir, ext);
    filesval **val = completefiles.access(key);
    if(!val)
    {
        filesval *f = new filesval(type, dir, ext);
        if(type==FILES_LIST) explodelist(dir, f->files);
        val = &completefiles[fileskey(type, f->dir, f->ext)];
        *val = f;
    }
    filesval **hasfiles = completions.access(command);
    if(hasfiles) *hasfiles = *val;
    else completions[newstring(command)] = *val;
}

void addfilecomplete(char *command, char *dir, char *ext)
{
    addcomplete(command, FILES_DIR, dir, ext);
}

void addlistcomplete(char *command, char *list)
{
    addcomplete(command, FILES_LIST, list, NULL);
}

COMMANDN(0, complete, addfilecomplete, "sss");
COMMANDN(0, listcomplete, addlistcomplete, "ss");

void complete(char *s, const char *cmdprefix)
{
    int cmdlen = 0;
    if(cmdprefix)
    {
        cmdlen = strlen(cmdprefix);
        if(strncmp(s, cmdprefix, cmdlen))
        {
            defformatstring(cmd)("%s%s", cmdprefix, s);
            copystring(s, cmd);
        }
    }
    if(!s[cmdlen]) return;
    if(!completesize) { completesize = (int)strlen(&s[cmdlen]); lastcomplete[0] = '\0'; }

    filesval *f = NULL;
    if(completesize)
    {
        char *end = strchr(&s[cmdlen], ' ');
        if(end)
        {
            string command;
            copystring(command, &s[cmdlen], min(size_t(end-&s[cmdlen]+1), sizeof(command)));
            filesval **hasfiles = completions.access(command);
            if(hasfiles) f = *hasfiles;
        }
    }

    const char *nextcomplete = NULL;
    string prefix;
    if(f) // complete using filenames
    {
        int commandsize = strchr(&s[cmdlen], ' ')+1-s;
        copystring(prefix, s, min(size_t(commandsize+1), sizeof(prefix)));
        f->update();
        loopv(f->files)
        {
            if(strncmp(f->files[i], &s[commandsize], completesize+cmdlen-commandsize)==0 &&
                strcmp(f->files[i], lastcomplete) > 0 && (!nextcomplete || strcmp(f->files[i], nextcomplete) < 0))
                nextcomplete = f->files[i];
        }
    }
    else // complete using command names
    {
        if(cmdprefix) copystring(prefix, cmdprefix); else prefix[0] = '\0';
        enumerate(idents, ident, id,
            if(id.flags&IDF_COMPLETE && strncmp(id.name, &s[cmdlen], completesize)==0 &&
               strcmp(id.name, lastcomplete) > 0 && (!nextcomplete || strcmp(id.name, nextcomplete) < 0))
                nextcomplete = id.name;
        );
    }
    if(nextcomplete)
    {
        formatstring(s)("%s%s", prefix, nextcomplete);
        copystring(lastcomplete, nextcomplete);
    }
    else lastcomplete[0] = '\0';
}

void setidflag(const char *s, const char *v, int flag, const char *msg, bool alias)
{
    ident *id = idents.access(s);
    if(!id || (alias && id->type != ID_ALIAS)) conoutf("\fradding %s of %s failed as it is not an alias", msg, s);
    else
    {
        bool on = false;
        if(isnumeric(*v)) on = atoi(v) != 0;
        else if(!strcasecmp("false", v)) on = false;
        else if(!strcasecmp("true", v)) on = true;
        else if(!strcasecmp("on", v)) on = true;
        else if(!strcasecmp("off", v)) on = false;
        if(on && !(id->flags&flag)) id->flags |= flag;
        else if(!on && id->flags&flag) id->flags &= ~flag;
    }
}

ICOMMAND(0, setcomplete, "ss", (char *s, char *t), setidflag(s, t, IDF_COMPLETE, "complete", false));
ICOMMAND(0, setpersist, "ss", (char *s, char *t), setidflag(s, t, IDF_PERSIST, "persist", true));

static inline bool sortcompletions(const char *x, const char *y)
{
    return strcmp(x, y) < 0;
}

void writecompletions(stream *f)
{
    vector<char *> cmds;
    enumeratekt(completions, char *, k, filesval *, v, { if(v) cmds.add(k); });
    cmds.sort(sortcompletions);
    loopv(cmds)
    {
        char *k = cmds[i];
        filesval *v = completions[k];
        if(v->type==FILES_LIST)
        {
            if(validateblock(v->dir)) f->printf("    listcomplete %s [%s]\n", escapeid(k), v->dir);
            else f->printf("    listcomplete %s %s\n", escapeid(k), escapestring(v->dir));
        }
        else f->printf("    complete %s %s %s\n", escapeid(k), escapestring(v->dir), escapestring(v->ext ? v->ext : "*"));
    }
}

