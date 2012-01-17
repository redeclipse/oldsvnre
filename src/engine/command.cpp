// command.cpp: implements the parsing and execution of a tiny script language which
// is largely backwards compatible with the quake console language.

#include "engine.h"

bool interactive = false;

hashset<ident> idents; // contains ALL vars/commands/aliases
vector<ident *> identmap;
ident *dummyident = NULL;

int identflags = 0;

static const int MAXARGS = 25;

VARN(0, numargs, _numargs, MAXARGS, 0, 0);

static inline void freearg(tagval &v)
{
    switch(v.type)
    {
        case VAL_STR: delete[] v.s; break;
        case VAL_CODE: if(v.code[-1] == CODE_START) delete[] (uchar *)&v.code[-1]; break;
    }
}

static inline void forcenull(tagval &v)
{
    switch(v.type)
    {
        case VAL_NULL: return;
    }
    freearg(v);
    v.setnull();
}

static inline float forcefloat(tagval &v)
{
    float f = 0.0f;
    switch(v.type)
    {
        case VAL_INT: f = v.i; break;
        case VAL_STR: f = parsefloat(v.s); break;
        case VAL_MACRO: f = parsefloat(v.s); break;
        case VAL_FLOAT: return v.f;
    }
    freearg(v);
    v.setfloat(f);
    return f;
}

static inline int forceint(tagval &v)
{
    int i = 0;
    switch(v.type)
    {
        case VAL_FLOAT: i = v.f; break;
        case VAL_STR: i = parseint(v.s); break;
        case VAL_MACRO: i = parseint(v.s); break;
        case VAL_INT: return v.i;
    }
    freearg(v);
    v.setint(i);
    return i;
}

static inline const char *forcestr(tagval &v)
{
    const char *s = "";
    switch(v.type)
    {
        case VAL_FLOAT: s = floatstr(v.f); break;
        case VAL_INT: s = intstr(v.i); break;
        case VAL_STR: case VAL_MACRO: return v.s;
    }
    freearg(v);
    v.setstr(newstring(s));
    return s;
}

static inline void forcearg(tagval &v, int type)
{
    switch(type)
    {
        case RET_STR: if(v.type != VAL_STR) forcestr(v); break;
        case RET_INT: if(v.type != VAL_INT) forceint(v); break;
        case RET_FLOAT: if(v.type != VAL_FLOAT) forcefloat(v); break;
    }
}

void tagval::cleanup()
{
    freearg(*this);
}

static inline void freeargs(tagval *args, int &oldnum, int newnum)
{
    for(int i = newnum; i < oldnum; i++) freearg(args[i]);
    oldnum = newnum;
}

static inline void cleancode(ident &id)
{
    if(id.code)
    {
        id.code[0] -= 0x100;
        if(int(id.code[0]) < 0x100) delete[] id.code;
        id.code = NULL;
    }
}

struct nullval : tagval
{
    nullval() { setnull(); }
} nullval;
tagval noret = nullval, *commandret = &noret;

void clear_command()
{
    enumerate(idents, ident, i,
    {
        if(i.type==ID_ALIAS)
        {
            DELETEA(i.name);
            i.forcenull();
            DELETEA(i.code);
        }
    });
}

static bool initedidents = false;
static vector<ident> *identinits = NULL;

static inline ident *addident(const ident &id)
{
    if(!initedidents)
    {
        if(!identinits) identinits = new vector<ident>;
        identinits->add(id);
        return NULL;
    }
    ident &def = idents.access(id.name, id);
    def.index = identmap.length();
    return identmap.add(&def);
}

static bool initidents()
{
    initedidents = true;
    for(int i = 0; i < MAXARGS; i++)
    {
        defformatstring(argname)("arg%d", i+1);
        newident(argname, IDF_ARG);
    }
    dummyident = newident("//dummy", IDF_UNKNOWN);
    if(identinits)
    {
        loopv(*identinits) addident((*identinits)[i]);
        DELETEP(identinits);
    }
    return true;
}
static bool forceinitidents = initidents();

static const char *sourcefile = NULL, *sourcestr = NULL;

static const char *debugline(const char *p, const char *fmt)
{
    if(!sourcestr) return fmt;
    int num = 1;
    const char *line = sourcestr;
    for(;;)
    {
        const char *end = strchr(line, '\n');
        if(!end) end = line + strlen(line);
        if(p >= line && p <= end)
        {
            static string buf;
            char color[] = { '\0', '\0', '\0' };
            if(fmt[0] == '\f') { strncpy(color, fmt, 2); fmt += strlen(color); }
            if(sourcefile) formatstring(buf)("%s%s:%d: %s", color, sourcefile, num, fmt);
            else formatstring(buf)("%s%d: %s", color, num, fmt);
            return buf;
        }
        if(!*end) break;
        line = end + 1;
        num++;
    }
    return fmt;
}

static struct identlink
{
    ident *id;
    identlink *next;
    int usedargs;
    identstack *argstack;
} noalias = { NULL, NULL, (1<<MAXARGS)-1, NULL }, *aliasstack = &noalias;

VAR(0, dbgalias, 0, 4, 1000);

static int nodebug = 0;

static void debugcode(const char *fmt, ...)
{
    if(nodebug) return;

    defvformatstring(msg, fmt, fmt);
    conoutft(CON_MESG, "%s", msg);

    if(!dbgalias) return;
    int total = 0, depth = 0;
    for(identlink *l = aliasstack; l != &noalias; l = l->next) total++;
    for(identlink *l = aliasstack; l != &noalias; l = l->next)
    {
        ident *id = l->id;
        ++depth;
        if(depth < dbgalias) conoutft(CON_MESG, "  %d) %s", total-depth+1, id->name);
        else if(l->next == &noalias) conoutft(CON_MESG, depth == dbgalias ? "  %d) %s" : "  ..%d) %s", total-depth+1, id->name);
    }
}

ICOMMAND(0, nodebug, "e", (uint *body), { nodebug++; executeret(body, *commandret); nodebug--; });

void addident(ident *id)
{
    addident(*id);
}

static inline void pusharg(ident &id, const tagval &v, identstack &stack)
{
    stack.val = id.val;
    stack.valtype = id.valtype;
    stack.next = id.stack;
    id.stack = &stack;
    id.setval(v);
    cleancode(id);
}

static inline void poparg(ident &id)
{
    if(!id.stack) return;
    identstack *stack = id.stack;
    if(id.valtype == VAL_STR) delete[] id.val.s;
    id.setval(*stack);
    cleancode(id);
    id.stack = stack->next;
}

ICOMMAND(0, push, "rte", (ident *id, tagval *v, uint *code),
{
    if(id->type != ID_ALIAS || id->index < MAXARGS) return;
    identstack stack;
    pusharg(*id, *v, stack);
    v->type = VAL_NULL;
    id->flags &= ~IDF_UNKNOWN;
    executeret(code, *commandret);
    poparg(*id);
});

static inline void pushalias(ident &id, identstack &stack)
{
    if(id.type == ID_ALIAS && id.index >= MAXARGS) 
    {
        pusharg(id, nullval, stack);
        id.flags &= ~IDF_UNKNOWN;
    }
}

static inline void popalias(ident &id)
{
    if(id.type == ID_ALIAS && id.index >= MAXARGS) poparg(id);
}

ICOMMAND(0, local, "L", (), {});

static inline bool checknumber(const char *s)
{
    if(isdigit(s[0])) return true;
    else switch(s[0])
    {
        case '+': case '-': return isdigit(s[1]) || (s[1] == '.' && isdigit(s[2]));
        case '.': return isdigit(s[1]) != 0;
        default: return false;
    }
}

ident *newident(const char *name, int flags)
{
    ident *id = idents.access(name);
    if(!id)
    {
        if(checknumber(name))
        {
            debugcode("number %s is not a valid identifier name", name);
            return dummyident;
        }
        ident init(ID_ALIAS, newstring(name), flags);
        id = addident(init);
    }
    return id;
}

ident *writeident(const char *name, int flags)
{
    ident *id = newident(name, flags);
    if(id->index < MAXARGS && !(aliasstack->usedargs&(1<<id->index)))
    {
        pusharg(*id, nullval, aliasstack->argstack[id->index]);
        aliasstack->usedargs |= 1<<id->index;
    }
    return id;
}

ident *readident(const char *name)
{
    ident *id = idents.access(name);
    if(id && id->index < MAXARGS && !(aliasstack->usedargs&(1<<id->index)))
       return NULL;
    return id;
}

static inline void setarg(ident &id, tagval &v)
{
    if(aliasstack->usedargs&(1<<id.index))
    {
        if(id.valtype == VAL_STR) delete[] id.val.s;
        id.setval(v);
        cleancode(id);
    }
    else
    {
        pusharg(id, v, aliasstack->argstack[id.index]);
        aliasstack->usedargs |= 1<<id.index;
    }
}

static inline void setalias(ident &id, tagval &v)
{
    if(id.valtype == VAL_STR) delete[] id.val.s;
    id.setval(v);
    cleancode(id);
    id.flags = (id.flags & (identflags|IDF_WORLD)) | identflags;
#ifndef STANDALONE
    if(id.flags&IDF_WORLD) client::editvar(&id, interactive && !(identflags&IDF_WORLD));
#endif
}

static void setalias(const char *name, tagval &v)
{
    ident *id = idents.access(name);
    if(id)
    {
        if(id->type == ID_ALIAS)
        {
            if(id->index < MAXARGS) setarg(*id, v); else setalias(*id, v);
        }
        else
        {
            debugcode("\frcannot redefine builtin %s with an alias", id->name);
            freearg(v);
        }
    }
    else if(checknumber(name))
    {
        debugcode("cannot alias number %s", name);
        freearg(v);
    }
    else
    {
        ident def(ID_ALIAS, newstring(name), v, identflags);
#ifdef STANDALONE
        addident(def);
#else
        id = addident(def);
        if(id->flags&IDF_WORLD) client::editvar(id, interactive && !(identflags&IDF_WORLD));
#endif
    }
}

void alias(const char *name, const char *str)
{
    tagval v;
    v.setstr(newstring(str));
    setalias(name, v);
}

void alias(const char *name, tagval &v)
{
    setalias(name, v);
}

ICOMMAND(0, alias, "st", (const char *name, tagval *v),
{
    setalias(name, *v);
    v->type = VAL_NULL;
});

void worldalias(const char *name, const char *action)
{
    WITHWORLD(alias(name, action));
}
COMMAND(0, worldalias, "ss");

// variable's and commands are registered through globals, see cube.h

int variable(const char *name, int min, int cur, int max, int *storage, identfun fun, int flags)
{
    ident v(ID_VAR, name, min, cur, max, storage, (void *)fun, flags);
    addident(v);
    return cur;
}

float fvariable(const char *name, float min, float cur, float max, float *storage, identfun fun, int flags)
{
    ident v(ID_FVAR, name, min, cur, max, storage, (void *)fun, flags);
    addident(v);
    return cur;
}

char *svariable(const char *name, const char *cur, char **storage, identfun fun, int flags)
{
    ident v(ID_SVAR, name, newstring(cur), storage, (void *)fun, flags);
    addident(v);
    return newstring(cur);
}

#define _GETVAR(id, vartype, name, retval) \
    ident *id = idents.access(name); \
    if(!id || id->type!=vartype) return retval;
#define GETVAR(id, name, retval) _GETVAR(id, ID_VAR, name, retval)
void setvar(const char *name, int i, bool dofunc, bool def)
{
    GETVAR(id, name, );
    *id->storage.i = clamp(i, id->minval, id->maxval);
    if(def) id->def.i = i;
    if(dofunc) id->changed();
    //if(!(id->flags&IDF_WORLD) && !(id->flags&IDF_REWRITE) && (verbose >= 4 || interactive)) conoutf("\fc%s set to %d", id->name, *id->storage.i);
}
void setfvar(const char *name, float f, bool dofunc, bool def)
{
    _GETVAR(id, ID_FVAR, name, );
    *id->storage.f = clamp(f, id->minvalf, id->maxvalf);
    if(def) id->def.f = f;
    if(dofunc) id->changed();
    //if(!(id->flags&IDF_WORLD) && !(id->flags&IDF_REWRITE) && (verbose >= 4 || interactive)) conoutf("\fc%s set to %s", id->name, floatstr(*id->storage.f));
}
void setsvar(const char *name, const char *str, bool dofunc, bool def)
{
    _GETVAR(id, ID_SVAR, name, );
    delete[] *id->storage.s;
    *id->storage.s = newstring(str);
    if(def)
    {
        delete[] id->def.s;
        id->def.s = newstring(str);
    }
    if(dofunc) id->changed();
    //if(!(id->flags&IDF_WORLD) && !(id->flags&IDF_REWRITE) && (verbose >= 4 || interactive)) conoutf("\fc%s set to %s", id->name, *id->storage.s);
}
int getvar(const char *name)
{
    GETVAR(id, name, 0);
    return *id->storage.i;
}
int getvartype(const char *name)
{
    GETVAR(id, name, 0);
    return id->type;
}
int getvarflags(const char *name)
{
    GETVAR(id, name, 0);
    return id->flags;
}
int getvarmin(const char *name)
{
    GETVAR(id, name, 0);
    return id->minval;
}
int getvarmax(const char *name)
{
    GETVAR(id, name, 0);
    return id->maxval;
}
float getfvarmin(const char *name)
{
    _GETVAR(id, ID_FVAR, name, 0);
    return id->minvalf;
}
float getfvarmax(const char *name)
{
    _GETVAR(id, ID_FVAR, name, 0);
    return id->maxvalf;
}
int getvardef(const char *name)
{
    ident *id = getident(name);
    if(!id) return 0;
    switch(id->type)
    {
        case ID_VAR: return id->def.i;
        case ID_FVAR: return int(id->def.f);
        case ID_SVAR: return atoi(id->def.s);
        case ID_ALIAS: return id->getint();
        default: break;
    }
    return 0;
}

ICOMMAND(0, getvar, "s", (char *n), intret(getvar(n)));
ICOMMAND(0, getvartype, "s", (char *n), intret(getvartype(n)));
ICOMMAND(0, getvarflags, "s", (char *n), intret(getvarflags(n)));
ICOMMAND(0, getvarmin, "s", (char *n), intret(getvarmin(n)));
ICOMMAND(0, getvarmax, "s", (char *n), intret(getvarmax(n)));
ICOMMAND(0, getfvarmin, "s", (char *s), floatret(getfvarmin(s)));
ICOMMAND(0, getfvarmax, "s", (char *s), floatret(getfvarmax(s)));
ICOMMAND(0, getvardef, "s", (char *n), intret(getvardef(n)));

bool identexists(const char *name) { return idents.access(name)!=NULL; }
ident *getident(const char *name) { return idents.access(name); }

void touchvar(const char *name)
{
    ident *id = idents.access(name);
    if(id) switch(id->type)
    {
        case ID_VAR:
        case ID_FVAR:
        case ID_SVAR:
            id->changed();
            break;
    }
}

const char *getalias(const char *name)
{
    ident *i = idents.access(name);
    return i && i->type==ID_ALIAS && (i->index >= MAXARGS || aliasstack->usedargs&(1<<i->index)) ? i->getstr() : "";
}

#ifndef STANDALONE
#define WORLDVAR \
    if(!(identflags&IDF_WORLD) && !editmode && id->flags&IDF_WORLD) \
    { \
        debugcode("\frcannot set world variable %s outside editmode", id->name); \
        return; \
    }
#endif

void setvarchecked(ident *id, int val)
{
    if(id->flags&IDF_READONLY) debugcode("\frvariable %s is read-only", id->name);
    else
    {
#ifndef STANDALONE
        WORLDVAR
#endif
        if(val<id->minval || val>id->maxval)
        {
            val = val<id->minval ? id->minval : id->maxval;                // clamp to valid range
            debugcode(
                id->flags&IDF_HEX ?
                    (id->minval <= 255 ? "\frvalid range for %s is %d..0x%X" : "\frvalid range for %s is 0x%X..0x%X") :
                    "\frvalid range for %s is %d..%d",
                id->name, id->minval, id->maxval);
        }
        *id->storage.i = val;
        id->changed();                                             // call trigger function if available
        //if(!(id->flags&IDF_WORLD) && !(id->flags&IDF_REWRITE) && (verbose >= 4 || interactive)) conoutf("\fc%s set to %d", id->name, *id->storage.i);
#ifndef STANDALONE
        if(id->flags&IDF_WORLD) client::editvar(id, interactive && !(identflags&IDF_WORLD));
#endif
    }
}

void setfvarchecked(ident *id, float val)
{
    if(id->flags&IDF_READONLY) debugcode("\frvariable %s is read-only", id->name);
    else
    {
#ifndef STANDALONE
        WORLDVAR
#endif
        if(val<id->minvalf || val>id->maxvalf)
        {
            val = val<id->minvalf ? id->minvalf : id->maxvalf;                // clamp to valid range
            debugcode("\frvalid range for %s is %s..%s", id->name, floatstr(id->minvalf), floatstr(id->maxvalf));
        }
        *id->storage.f = val;
        id->changed();
        //if(!(id->flags&IDF_WORLD) && !(id->flags&IDF_REWRITE) && (verbose >= 4 || interactive)) conoutf("\fc%s set to %s", id->name, floatstr(*id->storage.f));
#ifndef STANDALONE
        if(id->flags&IDF_WORLD) client::editvar(id, interactive && !(identflags&IDF_WORLD));
#endif
    }
}

void setsvarchecked(ident *id, const char *val)
{
    if(id->flags&IDF_READONLY) debugcode("\frvariable %s is read-only", id->name);
    else
    {
#ifndef STANDALONE
        WORLDVAR
#endif
        delete[] *id->storage.s;
        *id->storage.s = newstring(val);
        id->changed();
        //if(!(id->flags&IDF_WORLD) && !(id->flags&IDF_REWRITE) && (verbose >= 4 || interactive)) conoutf("\fc%s set to %s", id->name, *id->storage.s);
#ifndef STANDALONE
        if(id->flags&IDF_WORLD) client::editvar(id, interactive && !(identflags&IDF_WORLD));
#endif
    }
}

bool addcommand(const char *name, identfun fun, const char *args, int flags)
{
    uint argmask = 0;
    int numargs = 0;
    bool limit = true;
    for(const char *fmt = args; *fmt; fmt++) switch(*fmt)
    {
        case 'i': case 'f': case 't': case 'N': case 'D': case 'R': if(numargs < MAXARGS) numargs++; break;
        case 's': case 'e': case 'r': if(numargs < MAXARGS) { argmask |= 1<<numargs; numargs++; } break;
        case '1': case '2': case '3': case '4': if(numargs < MAXARGS) fmt -= *fmt-'0'+1; break;
        case 'C': case 'V': case 'L': limit = false; break;
        default: fatal("builtin %s declared with illegal type: %s", name, args); break;
    }
    if(limit && numargs > 8) fatal("builtin %s declared with too many args: %d", name, numargs);
    ident c(ID_COMMAND, name, args, argmask, (void *)fun, flags);
    addident(c);
    return false;
}

const char *parsestring(const char *p)
{
    for(; *p; p++) switch(*p)
    {
        case '\r':
        case '\n':
        case '\"':
            return p;
        case '^':
            if(*++p) break;
            return p;
    }
    return p;
}

int unescapestring(char *dst, const char *src, const char *end)
{
    char *start = dst;
    while(src < end)
    {
        int c = *src++;
        if(c == '^')
        {
            if(src >= end) break;
            int e = *src++;
            switch(e)
            {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'f': *dst++ = '\f'; break;
                default: *dst++ = e; break;
            }
        }
        else *dst++ = c;
    }
    return dst - start;
}

static char *conc(vector<char> &buf, tagval *v, int n, bool space, const char *prefix = NULL, int prefixlen = 0)
{
    if(prefix)
    {
        buf.put(prefix, prefixlen);
        if(space && n) buf.add(' ');
    }
    loopi(n)
    {
        const char *s = "";
        int len = 0;
        switch(v[i].type)
        {
            case VAL_INT: s = intstr(v[i].i); break;
            case VAL_FLOAT: s = floatstr(v[i].f); break;
            case VAL_STR: s = v[i].s; break;
            case VAL_MACRO: s = v[i].s; len = v[i].code[-1]>>8; goto haslen;
        }
        len = int(strlen(s));
    haslen:
        buf.put(s, len);
        if(i == n-1) break;
        if(space) buf.add(' ');
    }
    buf.add('\0');
    return buf.getbuf();
}

char *conc(tagval *v, int n, bool space, const char *prefix)
{
    int len = space ? max(prefix ? n : n-1, 0) : 0, prefixlen = 0;
    if(prefix) { prefixlen = strlen(prefix); len += prefixlen; }
    loopi(n) switch(v[i].type)
    {
        case VAL_MACRO: len += v[i].code[-1]>>8; break;
        case VAL_STR: len += int(strlen(v[i].s)); break;
        default:
        {
            vector<char> buf;
            buf.reserve(len);
            conc(buf, v, n, space, prefix, prefixlen);
            return newstring(buf.getbuf(), buf.length()-1);
        }
    }
    char *buf = newstring(prefix ? prefix : "", len);
    if(prefix && space && n) { buf[prefixlen] = ' '; buf[prefixlen] = '\0'; }
    loopi(n)
    {
        strcat(buf, v[i].s);
        if(i==n-1) break;
        if(space) strcat(buf, " ");
    }
    return buf;
}

static inline void skipcomments(const char *&p)
{
    for(;;)
    {
        p += strspn(p, " \t\r");
        if(p[0]!='/' || p[1]!='/') break;
        p += strcspn(p, "\n\0");
    }
}

static inline char *cutstring(const char *&p, int &len)
{
    p++;
    const char *end = parsestring(p);
    char *s = newstring(end - p);
    len = unescapestring(s, p, end);
    s[len] = '\0';
    p = end;
    if(*p=='\"') p++;
    return s;
}

char *parsetext(const char *&p)
{
    p += strspn(p, " \t\r");
    if(*p == '"')
    {
        int len;
        return cutstring(p, len);
    }
    else
    {
        const char *start = p;
        p += strcspn(p, " ;\t\r\n\0");
        return newstring(start, p-start);
    }
}

static inline const char *parseword(const char *p)
{
    const int maxbrak = 100;
    static char brakstack[maxbrak];
    int brakdepth = 0;
    for(;; p++)
    {
        p += strcspn(p, "\"/;()[]{} \t\r\n\0");
        switch(p[0])
        {
            case '"': case ';': case ' ': case '\t': case '\r': case '\n': case '\0': return p;
            case '/': if(p[1] == '/') return p; break;
            case '[': case '{': case '(': if(brakdepth >= maxbrak) return p; brakstack[brakdepth++] = p[0]; break;
            case ']': if(brakdepth <= 0 || brakstack[--brakdepth] != '[') return p; break;
            case '}': if(brakdepth <= 0 || brakstack[--brakdepth] != '{') return p; break;
            case ')': if(brakdepth <= 0 || brakstack[--brakdepth] != '(') return p; break;
        }
    }
    return p;
}

static inline char *cutword(const char *&p, int &len)
{
    const char *word = p;
    p = parseword(p);
    len = p-word;
    if(!len) return NULL;
    return newstring(word, len);
}

static inline void compilestr(vector<uint> &code, const char *word, int len, bool macro = false)
{
    if(len <= 3 && !macro)
    {
        uint op = CODE_VALI|RET_STR;
        for(int i = 0; i < len; i++) op |= uint(uchar(word[i]))<<((i+1)*8);
        code.add(op);
        return;
    }
    code.add((macro ? CODE_MACRO : CODE_VAL|RET_STR)|(len<<8));
    code.put((const uint *)word, len/sizeof(uint));
    size_t endlen = len%sizeof(uint);
    union
    {
        char c[sizeof(uint)];
        uint u;
    } end;
    end.u = 0;
    memcpy(end.c, word + len - endlen, endlen);
    code.add(end.u);
}

static inline void compilestr(vector<uint> &code, const char *word = NULL)
{
    if(!word) { code.add(CODE_VALI|RET_STR); return; }
    compilestr(code, word, int(strlen(word)));
}

static inline void compileint(vector<uint> &code, int i)
{
    if(abs(i) <= 0x7FFFFF)
        code.add(CODE_VALI|RET_INT|int(i)<<8);
    else
    {
        code.add(CODE_VAL|RET_INT);
        code.add(i);
    }
}

static inline void compilenull(vector<uint> &code)
{
    code.add(CODE_VALI|RET_NULL);
}

static inline void compileblock(vector<uint> &code)
{
    int start = code.length();
    code.add(CODE_BLOCK);
    code.add(CODE_OFFSET|((start+2)<<8));
    code.add(CODE_EXIT);
    code[start] |= uint(code.length() - (start + 1))<<8;
}

static inline void compileident(vector<uint> &code, const char *word = NULL)
{
    ident *id = newident(word ? word : "//dummy", IDF_UNKNOWN);
    code.add((id->index < MAXARGS ? CODE_IDENTARG : CODE_IDENT)|(id->index<<8));
}

static inline void compileint(vector<uint> &code, const char *word = NULL)
{
    return compileint(code, word ? parseint(word) : 0);
}

static inline void compilefloat(vector<uint> &code, float f)
{
    union
    {
        float f;
        uint u;
    } conv;
    conv.f = f;
    if(floor(conv.f) == conv.f && fabs(conv.f) <= 0x7FFFFF)
        code.add(CODE_VALI|RET_FLOAT|int(conv.f)<<8);
    else
    {
        code.add(CODE_VAL|RET_FLOAT);
        code.add(conv.u);
    }
}

static inline void compilefloat(vector<uint> &code, const char *word = NULL)
{
    return compilefloat(code, word ? parsefloat(word) : 0.0f);
}

static bool compilearg(vector<uint> &code, const char *&p, int wordtype);
static void compilestatements(vector<uint> &code, const char *&p, int rettype, int brak = '\0');

static inline void compileval(vector<uint> &code, int wordtype, char *word, int wordlen)
{
    switch(wordtype)
    {
        case VAL_STR: compilestr(code, word, wordlen, true); break;
        case VAL_ANY: compilestr(code, word, wordlen); break;
        case VAL_FLOAT: compilefloat(code, word); break;
        case VAL_INT: compileint(code, word); break;
        case VAL_CODE:
        {
            int start = code.length();
            code.add(CODE_BLOCK);
            code.add(CODE_OFFSET|((start+2)<<8));
            const char *p = word;
            compilestatements(code, p, VAL_ANY);
            code.add(CODE_EXIT|RET_STR);
            code[start] |= uint(code.length() - (start + 1))<<8;
            break;
        }
        case VAL_IDENT: compileident(code, word); break;
        default:
            break;
    }
}

static bool compileword(vector<uint> &code, const char *&p, int wordtype, char *&word, int &wordlen);

static bool compilelookup(vector<uint> &code, const char *&p, int ltype)
{
    char *lookup = NULL;
    int lookuplen = 0;
    switch(*++p)
    {
        case '(':
        case '[':
        case '{':
            if(!compileword(code, p, VAL_STR, lookup, lookuplen)) return false;
            break;
        case '$':
            if(!compilelookup(code, p, VAL_STR)) return false;
            break;
        default:
        {
            lookup = cutword(p, lookuplen);
            if(!lookup) return false;
            ident *id = newident(lookup, IDF_UNKNOWN);
            if(id) switch(id->type)
            {
            case ID_VAR: code.add(CODE_IVAR|((ltype >= VAL_ANY ? VAL_INT : ltype)<<CODE_RET)|(id->index<<8)); goto done;
            case ID_FVAR: code.add(CODE_FVAR|((ltype >= VAL_ANY ? VAL_FLOAT : ltype)<<CODE_RET)|(id->index<<8)); goto done;
            case ID_SVAR: code.add(CODE_SVAR|((ltype >= VAL_ANY ? VAL_STR : ltype)<<CODE_RET)|(id->index<<8)); goto done;
            case ID_ALIAS: code.add((id->index < MAXARGS ? CODE_LOOKUPARG : CODE_LOOKUP)|((ltype >= VAL_ANY ? VAL_STR : ltype)<<CODE_RET)|(id->index<<8)); goto done;
            }
            compilestr(code, lookup, lookuplen, true);
            break;
        }
    }
    code.add(CODE_LOOKUPU|((ltype < VAL_ANY ? ltype<<CODE_RET : 0)));
done:
    delete[] lookup;
    switch(ltype)
    {
        case VAL_CODE: code.add(CODE_COMPILE); break;
        case VAL_IDENT: code.add(CODE_IDENTU); break;
    }

    return true;
}

static bool compileblockstr(vector<uint> &code, const char *str, const char *end, bool macro, int braktype = ']')
{
    int start = code.length();
    code.add(macro ? CODE_MACRO : CODE_VAL|RET_STR);
    char *buf = (char *)code.reserve((end-str)/sizeof(uint)+1).buf;
    int len = 0;
    while(str < end)
    {
        int n = braktype == ']' ? strcspn(str, "\r/\"@]\0") : strcspn(str, "\r/\"}\0");
        memcpy(&buf[len], str, n);
        len += n;
        str += n;
        switch(*str)
        {
            case '\r': str++; break;
            case '\"':
            {
                const char *start = str;
                str = parsestring(str+1);
                if(*str=='\"') str++;
                memcpy(&buf[len], start, str-start);
                len += str-start;
                break;
            }
            case '/':
                if(str[1] == '/') str += strcspn(str, "\n\0");
                else buf[len++] = *str++;
                break;
            case '@':
            case ']':
            case '}':
                if(str < end) { buf[len++] = *str++; break; }
            case '\0': goto done;
        }
    }
done:
    memset(&buf[len], '\0', sizeof(uint)-len%sizeof(uint));
    code.advance(len/sizeof(uint)+1);
    code[start] |= len<<8;
    return true;
}

static bool compileblocksub(vector<uint> &code, const char *&p)
{
    switch(*p)
    {
        case '(':
            if(!compilearg(code, p, VAL_STR)) return false;
            break;
        case '[':
        case '{':
            if(!compilearg(code, p, VAL_STR)) return false;
            code.add(CODE_LOOKUPU|RET_STR);
            break;
        default:
        {
            const char *start = p;
            while(iscubealnum(*p) || *p=='_') p++;
            if(p <= start) return false;
            char *lookup = newstring(start, p-start);
            ident *id = newident(lookup, IDF_UNKNOWN);
            if(id) switch(id->type)
            {
            case ID_VAR: code.add(CODE_IVAR|RET_STR|(id->index<<8)); goto done;
            case ID_FVAR: code.add(CODE_FVAR|RET_STR|(id->index<<8)); goto done;
            case ID_SVAR: code.add(CODE_SVAR|RET_STR|(id->index<<8)); goto done;
            case ID_ALIAS: code.add((id->index < MAXARGS ? CODE_LOOKUPARG : CODE_LOOKUP)|RET_STR|(id->index<<8)); goto done;
            }
            compilestr(code, lookup, p-start, true);
            code.add(CODE_LOOKUPU|RET_STR);
        done:
            delete[] lookup;
            break;
        }
    }
    return true;
}

static bool compileblock(vector<uint> &code, const char *&p, int wordtype, int braktype = ']')
{
    const char *line = p, *start = p;
    int concs = 0;
    for(int brak = 1; brak;)
    {
        p += braktype == ']' ? strcspn(p, "@\"/[]\0") : strcspn(p, "\"/{}\0");
        int c = *p++;
        switch(c)
        {
            case '\0':
                debugcode(debugline(line, braktype == ']' ? "\frmissing \"]\"" : "\frmissing \"}\""));
                p--;
                return false;
            case '\"':
                p = parsestring(p);
                if(*p=='\"') p++;
                break;
            case '/':
                if(*p=='/') p += strcspn(p, "\n\0");
                break;
            case '[': case '{': brak++; break;
            case ']': case '}': brak--; break;
            case '@':
            {
                const char *esc = p;
                while(*p == '@') p++;
                int level = p - (esc - 1);
                if(brak > level) continue;
                else if(brak < level) debugcode(debugline(line, "\frtoo many @s"));
                if(!concs) code.add(CODE_ENTER);
                if(concs + 2 > MAXARGS)
                {
                    code.add(CODE_CONCW|RET_STR|(concs<<8));
                    concs = 1;
                }
                if(compileblockstr(code, start, esc-1, true)) concs++;
                if(compileblocksub(code, p)) concs++;
                if(!concs) code.pop();
                else start = p;
                break;
            }
        }
    }
    if(p-1 > start)
    {
        if(!concs) switch(wordtype)
        {
            case VAL_CODE:
            {
                p = start;
                int inst = code.length();
                code.add(CODE_BLOCK);
                code.add(CODE_OFFSET|((inst+2)<<8));
                compilestatements(code, p, VAL_ANY, braktype);
                code.add(CODE_EXIT);
                code[inst] |= uint(code.length() - (inst + 1))<<8;
                return true;
            }
            case VAL_IDENT:
            {
                char *name = newstring(start, p-1-start);
                compileident(code, name);
                delete[] name;
                return true;
            }
        }
        compileblockstr(code, start, p-1, concs > 0, braktype);
        if(concs > 1) concs++;
    }
    if(concs)
    {
        code.add(CODE_CONCM|(wordtype < VAL_ANY ? wordtype<<CODE_RET : RET_STR)|(concs<<8));
        code.add(CODE_EXIT|(wordtype < VAL_ANY ? wordtype<<CODE_RET : RET_STR));
    }
    switch(wordtype)
    {
        case VAL_CODE: if(!concs && p-1 <= start) compileblock(code); else code.add(CODE_COMPILE); break;
        case VAL_IDENT: if(!concs && p-1 <= start) compileident(code); else code.add(CODE_IDENTU); break;
        case VAL_STR: case VAL_NULL: case VAL_ANY:
            if(!concs && p-1 <= start) compilestr(code);
            break;
        default:
            if(!concs)
            {
                if(p-1 <= start) compileval(code, wordtype, NULL, 0);
                else code.add(CODE_FORCE|(wordtype<<CODE_RET));
            }
            break;
    }
    return true;
}

static bool compileword(vector<uint> &code, const char *&p, int wordtype, char *&word, int &wordlen)
{
    skipcomments(p);
    switch(*p)
    {
        case '\"': word = cutstring(p, wordlen); break;
        case '$': return compilelookup(code, p, wordtype);
        case '(':
            p++;
            code.add(CODE_ENTER);
            compilestatements(code, p, VAL_ANY, ')');
            code.add(CODE_EXIT|(wordtype < VAL_ANY ? wordtype<<CODE_RET : 0));
            switch(wordtype)
            {
                case VAL_CODE: code.add(CODE_COMPILE); break;
                case VAL_IDENT: code.add(CODE_IDENTU); break;
            }
            return true;
        case '[':
            p++;
            return compileblock(code, p, wordtype, ']');
        case '{':
            p++;
            return compileblock(code, p, wordtype, '}');
        default: word = cutword(p, wordlen); break;
    }
    return word!=NULL;
}

static inline bool compilearg(vector<uint> &code, const char *&p, int wordtype)
{
    char *word = NULL;
    int wordlen = 0;
    bool more = compileword(code, p, wordtype, word, wordlen);
    if(!more) return false;
    if(word)
    {
        compileval(code, wordtype, word, wordlen);
        delete[] word;
    }
    return true;
}

static void compilestatements(vector<uint> &code, const char *&p, int rettype, int brak)
{
    const char *line = p;
    char *idname = NULL;
    int idlen = 0;
    ident *id = NULL;
    int numargs = 0;
    for(;;)
    {
        skipcomments(p);
        idname = NULL;
        bool more = compileword(code, p, VAL_ANY, idname, idlen);
        if(!more) goto endstatement;
        skipcomments(p);
        if(p[0] == '=') switch(p[1])
        {
            case '/':
                if(p[2] != '/') break;
            case ';': case ' ': case '\t': case '\r': case '\n': case '\0':
                p++;
                if(idname)
                {
                    id = newident(idname, IDF_UNKNOWN);
                    if(!id || id->type != ID_ALIAS) { compilestr(code, idname, idlen, true); id = NULL; }
                    delete[] idname;
                }
                if(!(more = compilearg(code, p, VAL_ANY))) compilestr(code);
                code.add(idname && id ? (id->index < MAXARGS ? CODE_ALIASARG : CODE_ALIAS)|(id->index<<8) : CODE_ALIASU);
                goto endstatement;
        }
        numargs = 0;
        if(!idname)
        {
        noid:
            while(numargs < MAXARGS && (more = compilearg(code, p, VAL_ANY))) numargs++;
            code.add(CODE_CALLU);
        }
        else
        {
            id = idents.access(idname);
            if(!id || id->flags&IDF_REWRITE)
            {
                if(!checknumber(idname)) { compilestr(code, idname, idlen); delete[] idname; goto noid; }
                char *end = idname;
                int val = int(strtol(idname, &end, 0));
                if(*end) compilestr(code, idname, idlen);
                else compileint(code, val);
                code.add(CODE_RESULT);
            }
            else switch(id->type)
            {
                case ID_ALIAS:
                    while(numargs < MAXARGS && (more = compilearg(code, p, VAL_ANY))) numargs++;
                    code.add(CODE_CALL|(id->index<<8));
                    break;
                case ID_COMMAND:
                {
                    int comtype = CODE_COM, fakeargs = 0;
                    bool rep = false;
                    for(const char *fmt = id->args; *fmt; fmt++) switch(*fmt)
                    {
                    case 's':
                        if(more) more = compilearg(code, p, VAL_STR);
                        if(!more)
                        {
                            if(rep) break;
                            compilestr(code, NULL, 0, true);
                            fakeargs++;
                        }
                        else if(!fmt[1])
                        {
                            int numconc = 0;
                            while(numargs + numconc < MAXARGS && (more = compilearg(code, p, VAL_STR))) numconc++;
                            if(numconc > 0) code.add(CODE_CONC|RET_STR|((numconc+1)<<8));
                        }
                        numargs++;
                        break;
                    case 'i': if(more) more = compilearg(code, p, VAL_INT); if(!more) { if(rep) break; compileint(code); fakeargs++; } numargs++; break;
                    case 'f': if(more) more = compilearg(code, p, VAL_FLOAT); if(!more) { if(rep) break; compilefloat(code); fakeargs++; } numargs++; break;
                    case 't': if(more) more = compilearg(code, p, VAL_ANY); if(!more) { if(rep) break; compilenull(code); fakeargs++; } numargs++; break;
                    case 'e': if(more) more = compilearg(code, p, VAL_CODE); if(!more) { if(rep) break; compileblock(code); fakeargs++; } numargs++; break;
                    case 'r': if(more) more = compilearg(code, p, VAL_IDENT); if(!more) { if(rep) break; compileident(code); fakeargs++; } numargs++; break;
                    case 'N': compileint(code, numargs-fakeargs); numargs++; break;
#ifndef STANDALONE
                    case 'D': comtype = CODE_COMD; numargs++; break;
#endif
                    case 'C': comtype = CODE_COMC; if(more) while(numargs < MAXARGS && (more = compilearg(code, p, VAL_ANY))) numargs++; numargs = 1; goto endfmt;
                    case 'V': comtype = CODE_COMV; if(more) while(numargs < MAXARGS && (more = compilearg(code, p, VAL_ANY))) numargs++; numargs = 2; goto endfmt;
                    case '1': case '2': case '3': case '4': if(more) { fmt -= *fmt-'0'+1; rep = true; } break;
                    case 'L': 
                        if(more) while(numargs < MAXARGS && (more = compilearg(code, p, VAL_IDENT))) numargs++; 
                        if(more) while((more = compilearg(code, p, VAL_ANY))) code.add(CODE_POP); 
                        code.add(CODE_LOCAL); 
                        goto endcmd;
                    }
                endfmt:
                    code.add(comtype|(rettype < VAL_ANY ? rettype<<CODE_RET : 0)|(id->index<<8));
                endcmd:
                    break;
                }
                case ID_VAR:
                    if(!(more = compilearg(code, p, VAL_INT))) code.add(CODE_PRINT|(id->index<<8));
                    else if(!(id->flags&IDF_HEX) || !(more = compilearg(code, p, VAL_INT))) code.add(CODE_IVAR1|(id->index<<8));
                    else if(!(more = compilearg(code, p, VAL_INT))) code.add(CODE_IVAR2|(id->index<<8));
                    else code.add(CODE_IVAR3|(id->index<<8));
                    break;
                case ID_FVAR:
                    if(!(more = compilearg(code, p, VAL_FLOAT))) code.add(CODE_PRINT|(id->index<<8));
                    else code.add(CODE_FVAR1|(id->index<<8));
                    break;
                case ID_SVAR:
                    if(!(more = compilearg(code, p, VAL_STR))) code.add(CODE_PRINT|(id->index<<8));
                    else
                    {
                        int numconc = 0;
                        while(numconc+1 < MAXARGS && (more = compilearg(code, p, VAL_ANY))) numconc++;
                        if(numconc > 0) code.add(CODE_CONC|RET_STR|((numconc+1)<<8));
                        code.add(CODE_SVAR1|(id->index<<8));
                    }
                    break;
            }
            delete[] idname;
        }
    endstatement:
        if(more) while(compilearg(code, p, VAL_ANY)) code.add(CODE_POP);
        p += strcspn(p, ")}];/\n\0");
        int c = *p++;
        switch(c)
        {
            case '\0':
                if(c != brak) debugcode(debugline(line, "\frmissing \"%c\""), brak);
                return;

            case ')':
            case ']':
            case '}':
                if(c == brak) return;
                debugcode(debugline(line, "\frunexpected \"%c\""), c);
                break;

            case '/':
                if(*p == '/') p += strcspn(p, "\n\0");
                goto endstatement;
        }
    }
}

static void compilemain(vector<uint> &code, const char *p, int rettype = VAL_ANY)
{
    code.add(CODE_START);
    compilestatements(code, p, VAL_ANY);
    code.add(CODE_EXIT|(rettype < VAL_ANY ? rettype<<CODE_RET : 0));
}

uint *compilecode(const char *p)
{
    vector<uint> buf;
    buf.reserve(64);
    compilemain(buf, p);
    uint *code = new uint[buf.length()];
    memcpy(code, buf.getbuf(), buf.length()*sizeof(uint));
    code[0] += 0x100;
    return code;
}

void keepcode(uint *code)
{
    if(!code) return;
    switch(*code&CODE_OP_MASK)
    {
        case CODE_START:
            *code += 0x100;
            return;
    }
    switch(code[-1]&CODE_OP_MASK)
    {
        case CODE_START:
            code[-1] += 0x100;
            break;
        case CODE_OFFSET:
            code -= int(code[-1]>>8);
            *code += 0x100;
            break;
    }
}

void freecode(uint *code)
{
    if(!code) return;
    switch(*code&CODE_OP_MASK)
    {   
        case CODE_START:
            *code -= 0x100;
            if(int(*code) < 0x100) delete[] code;
            return;
    }
    switch(code[-1]&CODE_OP_MASK)
    {
        case CODE_START:
            code[-1] -= 0x100;
            if(int(code[-1]) < 0x100) delete[] &code[-1];
            break;
        case CODE_OFFSET:
            code -= int(code[-1]>>8);
            *code -= 0x100;
            if(int(*code) < 0x100) delete[] code;
            break;
    }
}

static void printvar(ident *id)
{
    switch(id->type)
    {
        case ID_VAR:
        {
            int i = *id->storage.i;
            if(i < 0) conoutft(CON_MESG, "%s = %d", id->name, i);
            else if(id->flags&IDF_HEX && id->maxval==0xFFFFFF)
                conoutft(CON_MESG, "%s = 0x%.6X (%d, %d, %d)", id->name, i, (i>>16)&0xFF, (i>>8)&0xFF, i&0xFF);
            else
                conoutft(CON_MESG, id->flags&IDF_HEX ? "%s = 0x%X" : "%s = %d", id->name, i);
            break;
        }
        case ID_FVAR:
            conoutft(CON_MESG, "%s = %s", id->name, floatstr(*id->storage.f));
            break;
        case ID_SVAR:
            conoutft(CON_MESG, strchr(*id->storage.s, '"') ? "%s = [%s]" : "%s = \"%s\"", id->name, *id->storage.s);
            break;
    }
}

typedef void (__cdecl *comfun)();
typedef void (__cdecl *comfun1)(void *);
typedef void (__cdecl *comfun2)(void *, void *);
typedef void (__cdecl *comfun3)(void *, void *, void *);
typedef void (__cdecl *comfun4)(void *, void *, void *, void *);
typedef void (__cdecl *comfun5)(void *, void *, void *, void *, void *);
typedef void (__cdecl *comfun6)(void *, void *, void *, void *, void *, void *);
typedef void (__cdecl *comfun7)(void *, void *, void *, void *, void *, void *, void *);
typedef void (__cdecl *comfun8)(void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (__cdecl *comfunv)(tagval *, int);

static const uint *runcode(const uint *code, tagval &result)
{
    ident *id = NULL;
    int numargs = 0;
    tagval args[MAXARGS+1], *prevret = commandret;
    result.setnull();
    commandret = &result;
    for(;;)
    {
        uint op = *code++;
        switch(op&0xFF)
        {
            case CODE_START: case CODE_OFFSET: continue;

            case CODE_POP:
                freearg(args[--numargs]);
                continue;
            case CODE_ENTER:
                code = runcode(code, args[numargs++]);
                continue;
            case CODE_EXIT|RET_NULL: case CODE_EXIT|RET_STR: case CODE_EXIT|RET_INT: case CODE_EXIT|RET_FLOAT:
                forcearg(result, op&CODE_RET_MASK);
                goto exit;
            case CODE_PRINT:
                printvar(identmap[op>>8]);
                continue;
            case CODE_LOCAL:
            {
                identstack locals[MAXARGS];
                freearg(result);
                loopi(numargs) pushalias(*args[i].id, locals[i]);
                code = runcode(code, result);
                loopi(numargs) popalias(*args[i].id);
                goto exit;
            }
        
            case CODE_MACRO:
            {
                uint len = op>>8;
                args[numargs++].setmacro(code);
                code += len/sizeof(uint) + 1;
                continue;
            }

            case CODE_VAL|RET_STR:
            {
                uint len = op>>8;
                args[numargs++].setstr(newstring((const char *)code, len));
                code += len/sizeof(uint) + 1;
                continue;
            }
            case CODE_VALI|RET_STR:
            {
                char s[4] = { (op>>8)&0xFF, (op>>16)&0xFF, (op>>24)&0xFF, '\0' };
                args[numargs++].setstr(newstring(s));
                continue;
            }
            case CODE_VAL|RET_NULL:
            case CODE_VALI|RET_NULL: args[numargs++].setnull(); continue;
            case CODE_VAL|RET_INT: args[numargs++].setint(int(*code++)); continue;
            case CODE_VALI|RET_INT: args[numargs++].setint(int(op)>>8); continue;
            case CODE_VAL|RET_FLOAT: args[numargs++].setfloat(*(const float *)code++); continue;
            case CODE_VALI|RET_FLOAT: args[numargs++].setfloat(float(int(op)>>8)); continue;

            case CODE_FORCE|RET_STR: forcestr(args[numargs-1]); continue;
            case CODE_FORCE|RET_INT: forceint(args[numargs-1]); continue;
            case CODE_FORCE|RET_FLOAT: forcefloat(args[numargs-1]); continue;

            case CODE_RESULT|RET_NULL: case CODE_RESULT|RET_STR: case CODE_RESULT|RET_INT: case CODE_RESULT|RET_FLOAT:
            litval:
                freearg(result);
                result = args[0];
                forcearg(result, op&CODE_RET_MASK);
                args[0].setnull();
                freeargs(args, numargs, 0);
                continue;

            case CODE_BLOCK:
            {
                uint len = op>>8;
                args[numargs++].setcode(code+1);
                code += len;
                continue;
            }
            case CODE_COMPILE:
            {
                tagval &arg = args[numargs-1];
                vector<uint> buf;
                switch(arg.type)
                {
                    case VAL_INT: buf.reserve(8); buf.add(CODE_START); compileint(buf, arg.i); buf.add(CODE_RESULT); buf.add(CODE_EXIT); break;
                    case VAL_FLOAT: buf.reserve(8); buf.add(CODE_START); compilefloat(buf, arg.f); buf.add(CODE_RESULT); buf.add(CODE_EXIT); break;
                    case VAL_STR: case VAL_MACRO: buf.reserve(64); compilemain(buf, arg.s); freearg(arg); break;
                    default: buf.reserve(8); buf.add(CODE_START); compilenull(buf); buf.add(CODE_RESULT); buf.add(CODE_EXIT); break;
                }
                arg.setcode(buf.getbuf()+1);
                buf.disown();
                continue;
            }

            case CODE_IDENT:
                args[numargs++].setident(identmap[op>>8]);
                continue;
            case CODE_IDENTARG:
            {
                ident *id = identmap[op>>8];
                if(!(aliasstack->usedargs&(1<<id->index)))
                {
                    pusharg(*id, nullval, aliasstack->argstack[id->index]);
                    aliasstack->usedargs |= 1<<id->index;
                }
                args[numargs++].setident(id);
                continue;
            }
            case CODE_IDENTU:
            {
                tagval &arg = args[numargs-1];
                ident *id = newident(arg.type == VAL_STR || arg.type == VAL_MACRO ? arg.s : "//dummy", IDF_UNKNOWN);
                if(id->index < MAXARGS && !(aliasstack->usedargs&(1<<id->index)))
                {
                    pusharg(*id, nullval, aliasstack->argstack[id->index]);
                    aliasstack->usedargs |= 1<<id->index;
                }
                freearg(arg);
                arg.setident(id);
                continue;
            }

            case CODE_LOOKUPU|RET_STR:
                #define LOOKUPU(aval, sval, ival, fval, nval) { \
                    tagval &arg = args[numargs-1]; \
                    if(arg.type != VAL_STR && arg.type != VAL_MACRO) continue; \
                    id = idents.access(arg.s); \
                    if(id) switch(id->type) \
                    { \
                        case ID_ALIAS: \
                            if(id->flags&IDF_UNKNOWN) break; \
                            freearg(arg); \
                            if(id->index < MAXARGS && !(aliasstack->usedargs&(1<<id->index))) { nval; continue; } \
                            aval; \
                            continue; \
                        case ID_SVAR: freearg(arg); sval; continue; \
                        case ID_VAR: freearg(arg); ival; continue; \
                        case ID_FVAR: freearg(arg); fval; continue; \
                    } \
                    debugcode("\frunknown alias lookup: %s", arg.s); \
                    freearg(arg); \
                    nval; \
                    continue; \
                }
                LOOKUPU(arg.setstr(newstring(id->getstr())),
                        arg.setstr(newstring(*id->storage.s)),
                        arg.setstr(newstring(intstr(*id->storage.i))),
                        arg.setstr(newstring(floatstr(*id->storage.f))),
                        arg.setstr(newstring("")));
            case CODE_LOOKUP|RET_STR:
                #define LOOKUP(aval) { \
                    id = identmap[op>>8]; \
                    if(id->flags&IDF_UNKNOWN) debugcode("\frunknown alias lookup: %s", id->name); \
                    aval; \
                    continue; \
                }
                LOOKUP(args[numargs++].setstr(newstring(id->getstr())));
            case CODE_LOOKUPARG|RET_STR:
                #define LOOKUPARG(aval, nval) { \
                    id = identmap[op>>8]; \
                    if(!(aliasstack->usedargs&(1<<id->index))) { nval; continue; } \
                    aval; \
                    continue; \
                }
                LOOKUPARG(args[numargs++].setstr(newstring(id->getstr())), args[numargs++].setstr(newstring("")));
            case CODE_LOOKUPU|RET_INT:
                LOOKUPU(arg.setint(id->getint()),
                        arg.setint(parseint(*id->storage.s)),
                        arg.setint(*id->storage.i),
                        arg.setint(int(*id->storage.f)),
                        arg.setint(0));
            case CODE_LOOKUP|RET_INT:
                LOOKUP(args[numargs++].setint(id->getint()));
            case CODE_LOOKUPARG|RET_INT:
                LOOKUPARG(args[numargs++].setint(id->getint()), args[numargs++].setint(0));
            case CODE_LOOKUPU|RET_FLOAT:
                LOOKUPU(arg.setfloat(id->getfloat()),
                        arg.setfloat(parsefloat(*id->storage.s)),
                        arg.setfloat(float(*id->storage.i)),
                        arg.setfloat(*id->storage.f),
                        arg.setfloat(0.0f));
            case CODE_LOOKUP|RET_FLOAT:
                LOOKUP(args[numargs++].setfloat(id->getfloat()));
            case CODE_LOOKUPARG|RET_FLOAT:
                LOOKUPARG(args[numargs++].setfloat(id->getfloat()), args[numargs++].setfloat(0.0f));
            case CODE_LOOKUPU|RET_NULL:
                LOOKUPU(id->getval(arg),
                        arg.setstr(newstring(*id->storage.s)),
                        arg.setint(*id->storage.i),
                        arg.setfloat(*id->storage.f),
                        arg.setnull());
            case CODE_LOOKUP|RET_NULL:
                LOOKUP(id->getval(args[numargs++]));
            case CODE_LOOKUPARG|RET_NULL:
                LOOKUPARG(id->getval(args[numargs++]), args[numargs++].setnull());

            case CODE_SVAR|RET_STR: case CODE_SVAR|RET_NULL: args[numargs++].setstr(newstring(*identmap[op>>8]->storage.s)); continue;
            case CODE_SVAR|RET_INT: args[numargs++].setint(parseint(*identmap[op>>8]->storage.s)); continue;
            case CODE_SVAR|RET_FLOAT: args[numargs++].setfloat(parsefloat(*identmap[op>>8]->storage.s)); continue;
            case CODE_SVAR1: setsvarchecked(identmap[op>>8], args[0].s); freeargs(args, numargs, 0); continue;

            case CODE_IVAR|RET_INT: case CODE_IVAR|RET_NULL: args[numargs++].setint(*identmap[op>>8]->storage.i); continue;
            case CODE_IVAR|RET_STR: args[numargs++].setstr(newstring(intstr(*identmap[op>>8]->storage.i))); continue;
            case CODE_IVAR|RET_FLOAT: args[numargs++].setfloat(float(*identmap[op>>8]->storage.i)); continue;
            case CODE_IVAR1: setvarchecked(identmap[op>>8], args[0].i); numargs = 0; continue;
            case CODE_IVAR2: setvarchecked(identmap[op>>8], (args[0].i<<16)|(args[1].i<<8)); numargs = 0; continue;
            case CODE_IVAR3: setvarchecked(identmap[op>>8], (args[0].i<<16)|(args[1].i<<8)|args[2].i); numargs = 0; continue;

            case CODE_FVAR|RET_FLOAT: case CODE_FVAR|RET_NULL: args[numargs++].setfloat(*identmap[op>>8]->storage.f); continue;
            case CODE_FVAR|RET_STR: args[numargs++].setstr(newstring(floatstr(*identmap[op>>8]->storage.f))); continue;
            case CODE_FVAR|RET_INT: args[numargs++].setint(int(*identmap[op>>8]->storage.f)); continue;
            case CODE_FVAR1: setfvarchecked(identmap[op>>8], args[0].f); numargs = 0; continue;

            case CODE_COM|RET_NULL: case CODE_COM|RET_STR: case CODE_COM|RET_FLOAT: case CODE_COM|RET_INT:
                id = identmap[op>>8];
#ifndef STANDALONE
            callcom:
#endif
                #define CALLCOM \
                    switch(numargs) \
                    { \
                        case 0: ((comfun)id->fun)(); break; \
                        case 1: ((comfun1)id->fun)(ARG(0)); break; \
                        case 2: ((comfun2)id->fun)(ARG(0), ARG(1)); break; \
                        case 3: ((comfun3)id->fun)(ARG(0), ARG(1), ARG(2)); break; \
                        case 4: ((comfun4)id->fun)(ARG(0), ARG(1), ARG(2), ARG(3)); break; \
                        case 5: ((comfun5)id->fun)(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4)); break; \
                        case 6: ((comfun6)id->fun)(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5)); break; \
                        case 7: ((comfun7)id->fun)(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5), ARG(6)); break; \
                        case 8: ((comfun8)id->fun)(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5), ARG(6), ARG(7)); break; \
                    }
                forcenull(result);
                #define ARG(n) (id->argmask&(1<<n) ? (void *)args[n].s : (void *)&args[n].i)
                CALLCOM
                #undef ARG
            forceresult:
                freeargs(args, numargs, 0);
                forcearg(result, op&CODE_RET_MASK);
                continue;
#ifndef STANDALONE
            case CODE_COMD|RET_NULL: case CODE_COMD|RET_STR: case CODE_COMD|RET_FLOAT: case CODE_COMD|RET_INT:
                id = identmap[op>>8];
                args[numargs].setint(addreleaseaction(conc(args, numargs, true, id->name)) ? 1 : 0);
                numargs++;
                goto callcom;
#endif
            case CODE_COMV|RET_NULL: case CODE_COMV|RET_STR: case CODE_COMV|RET_FLOAT: case CODE_COMV|RET_INT:
                id = identmap[op>>8];
                forcenull(result);
                ((comfunv)id->fun)(args, numargs);
                goto forceresult;
            case CODE_COMC|RET_NULL: case CODE_COMC|RET_STR: case CODE_COMC|RET_FLOAT: case CODE_COMC|RET_INT:
                id = identmap[op>>8];
                forcenull(result);
                {
                    vector<char> buf;
                    ((comfun1)id->fun)(conc(buf, args, numargs, true));
                }
                goto forceresult;

            case CODE_CONC|RET_NULL: case CODE_CONC|RET_STR: case CODE_CONC|RET_FLOAT: case CODE_CONC|RET_INT:
            case CODE_CONCW|RET_NULL: case CODE_CONCW|RET_STR: case CODE_CONCW|RET_FLOAT: case CODE_CONCW|RET_INT:
            {
                int numconc = op>>8;
                char *s = conc(&args[numargs-numconc], numconc, (op&CODE_OP_MASK)==CODE_CONC);
                freeargs(args, numargs, numargs-numconc);
                args[numargs++].setstr(s);
                forcearg(args[numargs-1], op&CODE_RET_MASK);
                continue;
            }

            case CODE_CONCM|RET_NULL: case CODE_CONCM|RET_STR: case CODE_CONCM|RET_FLOAT: case CODE_CONCM|RET_INT:
            {
                int numconc = op>>8;
                char *s = conc(&args[numargs-numconc], numconc, false);
                freeargs(args, numargs, numargs-numconc);
                result.setstr(s);
                forcearg(result, op&CODE_RET_MASK);
                continue;
            }

            case CODE_ALIAS:
                setalias(*identmap[op>>8], args[--numargs]);
                freeargs(args, numargs, 0);
                continue;
            case CODE_ALIASARG:
                setarg(*identmap[op>>8], args[--numargs]);
                freeargs(args, numargs, 0);
                continue;
            case CODE_ALIASU:
                forcestr(args[0]);
                setalias(args[0].s, args[--numargs]);
                freeargs(args, numargs, 0);
                continue;

            case CODE_CALL|RET_NULL: case CODE_CALL|RET_STR: case CODE_CALL|RET_FLOAT: case CODE_CALL|RET_INT:
                #define CALLALIAS(offset) { \
                    identstack argstack[MAXARGS]; \
                    for(int i = 0; i < numargs-offset; i++) \
                        pusharg(*identmap[i], args[i+offset], argstack[i]); \
                    int oldargs = _numargs, newargs = numargs-offset; \
                    _numargs = newargs; \
                    identlink aliaslink = { id, aliasstack, (1<<newargs)-1, argstack }; \
                    aliasstack = &aliaslink; \
                    if(!id->code) id->code = compilecode(id->getstr()); \
                    uint *code = id->code; \
                    code[0] += 0x100; \
                    runcode(code+1, result); \
                    code[0] -= 0x100; \
                    if(int(code[0]) < 0x100) delete[] code; \
                    aliasstack = aliaslink.next; \
                    for(int i = 0; i < newargs; i++) \
                        poparg(*identmap[i]); \
                    for(int argmask = aliaslink.usedargs&(~0<<newargs), i = newargs; argmask; i++) \
                        if(argmask&(1<<i)) { poparg(*identmap[i]); argmask &= ~(1<<i); } \
                    forcearg(result, op&CODE_RET_MASK); \
                    _numargs = oldargs; \
                    numargs = 0; \
                }
                forcenull(result);
                id = identmap[op>>8];
                if(id->flags&IDF_UNKNOWN)
                {
                    debugcode("\frunknown command: %s", id->name);
                    goto forceresult;
                }
                CALLALIAS(0);
                continue;
            case CODE_CALLARG|RET_NULL: case CODE_CALLARG|RET_STR: case CODE_CALLARG|RET_FLOAT: case CODE_CALLARG|RET_INT:
                forcenull(result);
                id = identmap[op>>8];
                if(!(aliasstack->usedargs&(1<<id->index))) goto forceresult;
                CALLALIAS(0);
                continue;

            case CODE_CALLU|RET_NULL: case CODE_CALLU|RET_STR: case CODE_CALLU|RET_FLOAT: case CODE_CALLU|RET_INT:
                if(args[0].type != VAL_STR) goto litval;
                id = idents.access(args[0].s);
                if(!id || id->flags&IDF_REWRITE)
                {
                noid:
                    if(checknumber(args[0].s)) goto litval;
                    if(!id || id->flags&IDF_REWRITE)
                    {
                        if(server::rewritecommand(id, args, numargs)) goto forceresult;
                    }
                    debugcode("\frunknown command: %s", args[0].s);
                    forcenull(result);
                    goto forceresult;
                }
                forcenull(result);
                switch(id->type)
                {
                    case ID_COMMAND:
                    {
                        int i = 1, maxargs = MAXARGS, fakeargs = 1;
                        for(const char *fmt = id->args; *fmt && i < maxargs; fmt++, i++) switch(*fmt)
                        {
                            case 'i': if(numargs <= i) { args[numargs++].setint(0); fakeargs++; } else forceint(args[i]); break;
                            case 'f': if(numargs <= i) { args[numargs++].setfloat(0.0f); fakeargs++; } else forcefloat(args[i]); break;
                            case 's': if(numargs <= i) { args[numargs++].setstr(newstring("")); fakeargs++; } else forcestr(args[i]); break;
                            case 't': if(numargs <= i) { args[numargs++].setnull(); fakeargs++; } break;
                            case 'e':
                                if(numargs <= i)
                                {
                                    uint *buf = new uint[2];
                                    buf[0] = CODE_START;
                                    buf[1] = CODE_EXIT;
                                    args[numargs++].setcode(buf);
                                    fakeargs++;
                                }
                                else
                                {
                                    vector<uint> buf;
                                    buf.reserve(64);
                                    compilemain(buf, numargs <= i ? "" : args[i].getstr());
                                    freearg(args[i]);
                                    args[i].setcode(buf.getbuf()+1);
                                    buf.disown();
                                }
                                break;
                            case 'r':
                                if(numargs <= i) { args[numargs++].setident(newident("//dummy", IDF_UNKNOWN)); fakeargs++; }
                                else
                                {
                                    ident *id = newident(args[i].type==VAL_STR ? args[i].s : "//dummy", IDF_UNKNOWN);
                                    freearg(args[i]);
                                    args[i].setident(id);
                                }
                                break;
                            case 'N': args[numargs].setint(numargs-fakeargs); numargs++; fakeargs++; break;
#ifndef STANDALONE
                            case 'D': args[numargs].setint(addreleaseaction(conc(args, numargs, true)) ? 1 : 0); numargs++; fakeargs++; break;
#endif
                            case 'C':
                            {
                                vector<char> buf;
                                ((comfun1)id->fun)(conc(buf, args+1, numargs-1, true));
                                goto forceresult;
                            }
                            case 'V': ((comfunv)id->fun)(args+1, numargs-1); goto forceresult;
                            case '1': case '2': case '3': case '4': if(numargs <= i) { fmt -= *fmt-'0'+1; maxargs = numargs; } break;
                            case 'L':
                            {
                                identstack locals[MAXARGS];
                                freearg(args[0]);
                                loopj(numargs-1)
                                {
                                    tagval &v = args[j+1];
                                    if(v.type != VAL_IDENT)
                                    {
                                        ident *id = newident(v.type==VAL_STR ? v.s : "//dummy", IDF_UNKNOWN);
                                        freearg(v);
                                        v.setident(id);
                                    }
                                    pushalias(*v.id, locals[j]);
                                }
                                code = runcode(code, result);
                                loopj(numargs-1) popalias(*args[j+1].id);
                                goto exit;
                            }
                        }
                        #define ARG(n) (id->argmask&(1<<n) ? (void *)args[n+1].s : (void *)&args[n+1].i)
                        CALLCOM
                        #undef ARG
                        goto forceresult;
                    }
                    case ID_VAR:
                        if(numargs <= 1) printvar(id);
                        else
                        {
                            int val = forceint(args[1]);
                            if(id->flags&IDF_HEX && numargs > 2)
                            {
                                val = (val << 16) | (forceint(args[2])<<8);
                                if(numargs > 3) val |= forceint(args[3]);
                            }
                            setvarchecked(id, val);
                        }
                        goto forceresult;
                    case ID_FVAR:
                        if(numargs <= 1) printvar(id); else setfvarchecked(id, forcefloat(args[1]));
                        goto forceresult;
                    case ID_SVAR:
                        if(numargs <= 1) printvar(id); else setsvarchecked(id, forcestr(args[1]));
                        goto forceresult;
                    case ID_ALIAS:
                        if(id->index < MAXARGS && !(aliasstack->usedargs&(1<<id->index))) goto forceresult;
                        if(id->valtype==VAL_NULL) goto noid;
                        freearg(args[0]);
                        CALLALIAS(1);
                        continue;
                    default:
                        goto forceresult;
                }
        }
    }
exit:
    commandret = prevret;
    return code;
}

void executeret(const uint *code, tagval &result)
{
    runcode(code, result);
}

void executeret(const char *p, tagval &result)
{
    vector<uint> code;
    code.reserve(64);
    compilemain(code, p, VAL_ANY);
    runcode(code.getbuf()+1, result);
    if(int(code[0]) >= 0x100) code.disown();
}

char *executestr(const uint *code)
{
    tagval result;
    runcode(code, result);
    if(result.type == VAL_NULL) return NULL;
    forcestr(result);
    return result.s;
}

char *executestr(const char *p)
{
    tagval result;
    executeret(p, result);
    if(result.type == VAL_NULL) return NULL;
    forcestr(result);
    return result.s;
}

int execute(const uint *code)
{
    tagval result;
    runcode(code, result);
    int i = result.getint();
    freearg(result);
    return i;
}

int execute(const char *p)
{
    vector<uint> code;
    code.reserve(64);
    compilemain(code, p, VAL_INT);
    tagval result;
    runcode(code.getbuf()+1, result);
    if(int(code[0]) >= 0x100) code.disown();
    int i = result.getint();
    freearg(result);
    return i;
}

int execute(const char *p, bool nonworld)
{
    int oldflags = identflags;
    if(nonworld) identflags &= ~IDF_WORLD;
    int result = execute(p);
    if(nonworld) identflags = oldflags;
    return result;
}

static inline bool getbool(const char *s)
{
    switch(s[0])
    {
        case '+': case '-': 
            switch(s[1])
            {
                case '0': break;
                case '.': return !isdigit(s[2]) || parsefloat(s) != 0;
                default: return true;
            }
            // fall through
        case '0':
        {      
            char *end;
            int val = strtol((char *)s, &end, 0);
            if(val) return true;
            switch(*end)
            {
                case 'e': case '.': return parsefloat(s) != 0;
                default: return false;
            }
        }
        case '.': return !isdigit(s[1]) || parsefloat(s) != 0;
        case '\0': return false;
        default: return true;
    }
}

static inline bool getbool(const tagval &v)
{
    switch(v.type)
    {
        case VAL_FLOAT: return v.f!=0;
        case VAL_INT: return v.i!=0;
        case VAL_STR: case VAL_MACRO: return getbool(v.s);
        default: return false;
    }
}

bool executebool(const uint *code)
{
    tagval result;
    runcode(code, result);
    bool b = getbool(result);
    freearg(result);
    return b;
}

bool executebool(const char *p)
{
    tagval result;
    executeret(p, result);
    bool b = getbool(result);
    freearg(result);
    return b;
}

bool execfile(const char *cfgfile, bool msg, bool nonworld)
{
    string s;
    copystring(s, cfgfile);
    char *buf = loadfile(s, NULL);
    if(!buf)
    {
        if(msg) conoutf("\frcould not read %s", cfgfile);
        return false;
    }
    int oldflags = identflags;
    if(nonworld) identflags &= ~IDF_WORLD;
    const char *oldsourcefile = sourcefile, *oldsourcestr = sourcestr;
    sourcefile = cfgfile;
    sourcestr = buf;
    execute(buf);
    sourcefile = oldsourcefile;
    sourcestr = oldsourcestr;
    if(nonworld) identflags = oldflags;
    delete[] buf;
    if(verbose >= 3) conoutf("\faloaded script %s", cfgfile);
    return true;
}

const char *escapestring(const char *s)
{
    static vector<char> strbuf[3];
    static int stridx = 0;
    stridx = (stridx + 1)%3;
    vector<char> &buf = strbuf[stridx];
    buf.setsize(0);
    buf.add('"');
    for(; *s; s++) switch(*s)
    {
        case '\n': buf.put("^n", 2); break;
        case '\t': buf.put("^t", 2); break;
        case '\f': buf.put("^f", 2); break;
        case '"': buf.put("^\"", 2); break;
        case '^': buf.put("^^", 2); break;
        default: buf.add(*s); break;
    }
    buf.put("\"\0", 2);
    return buf.getbuf();
}

const char *escapeid(const char *s)
{
    const char *end = s + strcspn(s, "\"/;()[]{} \f\t\r\n\0");
    return *end ? escapestring(s) : s;
}

bool validateblock(const char *s)
{
    const int maxbrak = 100;
    static char brakstack[maxbrak];
    int brakdepth = 0;
    for(; *s; s++) switch(*s)
    {
        case '[': case '{': case '(': if(brakdepth >= maxbrak) return false; brakstack[brakdepth++] = *s; break;
        case ']': if(brakdepth <= 0 || brakstack[--brakdepth] != '[') return false; break;
        case '}': if(brakdepth <= 0 || brakstack[--brakdepth] != '{') return false; break;
        case ')': if(brakdepth <= 0 || brakstack[--brakdepth] != '(') return false; break;
        case '"': s = parsestring(s + 1); if(*s != '"') return false; break;
        case '/': if(s[1] == '/') return false; break;
        case '\f': return false;
    }
    return brakdepth == 0;
}

// below the commands that implement a small imperative language. thanks to the semantics of
// () and [] expressions, any control construct can be defined trivially.

static string retbuf[3];
static int retidx = 0;

const char *intstr(int v)
{
    retidx = (retidx + 1)%3;
    formatstring(retbuf[retidx])("%d", v);
    return retbuf[retidx];
}

void intret(int v)
{
    commandret->setint(v);
}

const char *floatstr(float v)
{
    retidx = (retidx + 1)%3;
    formatstring(retbuf[retidx])(v==int(v) ? "%.1f" : "%.7g", v);
    return retbuf[retidx];
}

void floatret(float v)
{
    commandret->setfloat(v);
}

#undef ICOMMANDNAME
#define ICOMMANDNAME(name) _stdcmd

ICOMMAND(0, do, "e", (uint *body), executeret(body, *commandret));
ICOMMAND(0, if, "tee", (tagval *cond, uint *t, uint *f), executeret(getbool(*cond) ? t : f, *commandret));
ICOMMAND(0, ?, "ttt", (tagval *cond, tagval *t, tagval *f), result(*(getbool(*cond) ? t : f)));

static inline void setiter(ident &id, int i, identstack &stack)
{
    if(i)
    {
        if(id.valtype != VAL_INT)
        {
            if(id.valtype == VAL_STR) delete[] id.val.s;
            cleancode(id);
            id.valtype = VAL_INT;
        }
        id.val.i = i;
    }
    else
    {
        tagval zero;
        zero.setint(0);
        pusharg(id, zero, stack);
        id.flags &= ~IDF_UNKNOWN;
    }
}
ICOMMAND(0, loop, "rie", (ident *id, int *n, uint *body),
{
    if(*n <= 0 || id->type!=ID_ALIAS) return;
    identstack stack;
    loopi(*n)
    {
        setiter(*id, i, stack);
        execute(body);
    }
    poparg(*id);
});
ICOMMAND(0, loopwhile, "riee", (ident *id, int *n, uint *cond, uint *body),
{
    if(*n <= 0 || id->type!=ID_ALIAS) return;
    identstack stack;
    loopi(*n)
    {
        setiter(*id, i, stack);
        if(!executebool(cond)) break;
        execute(body);
    }
    poparg(*id);
});
ICOMMAND(0, while, "ee", (uint *cond, uint *body), while(executebool(cond)) execute(body));
ICOMMAND(0, loopconcat, "rie", (ident *id, int *n, uint *body),
{
    if(*n <= 0 || id->type!=ID_ALIAS) return;
    identstack stack;
    vector<char> s;
    loopi(*n)
    {
        setiter(*id, i, stack);
        tagval v;
        executeret(body, v);
        const char *vstr = v.getstr();
        int len = strlen(vstr);
        if(!s.empty()) s.add(' ');
        s.put(vstr, len);
        freearg(v);
    }
    poparg(*id);
    s.add('\0');
    commandret->setstr(newstring(s.getbuf(), s.length()-1));
});

void concat(tagval *v, int n)
{
    commandret->setstr(conc(v, n, true));
}

void concatword(tagval *v, int n)
{
    commandret->setstr(conc(v, n, false));
}

void result(tagval &v)
{
    *commandret = v;
    v.type = VAL_NULL;
}

void stringret(char *s)
{
    commandret->setstr(s);
}

void result(const char *s)
{
    commandret->setstr(newstring(s));
}

void format(tagval *args, int numargs)
{
    vector<char> s;
    const char *f = args[0].getstr();
    while(*f)
    {
        int c = *f++;
        if(c == '%')
        {
            int i = *f++;
            if(i >= '1' && i <= '9')
            {
                i -= '0';
                const char *sub = i < numargs ? args[i].getstr() : "";
                while(*sub) s.add(*sub++);
            }
            else s.add(i);
        }
        else s.add(c);
    }
    s.add('\0');
    result(s.getbuf());
}

ICOMMAND(0, result, "t", (tagval *v),
{
    *commandret = *v;
    v->type = VAL_NULL;
});

COMMAND(0, concat, "V");
COMMAND(0, concatword, "V");
COMMAND(0, format, "V");

static const char *liststart = NULL, *listend = NULL, *listquotestart = NULL, *listquoteend = NULL;

static inline void skiplist(const char *&p)
{
    for(;;)
    {
        p += strspn(p, " \t\r\n");
        if(p[0]!='/' || p[1]!='/') break;
        p += strcspn(p, "\n\0");
    }
}

static bool parselist(const char *&s, const char *&start = liststart, const char *&end = listend, const char *&quotestart = listquotestart, const char *&quoteend = listquoteend)
{
    skiplist(s);
    switch(*s)
    {
        case '"': quotestart = s++; start = s; s = parsestring(s); end = s; if(*s == '"') s++; quoteend = s; break;
        case '(': case '[': case '{':
            quotestart = s;
            start = s+1;
            for(int braktype = *s++, brak = 1;;)
            {
                s += strcspn(s, "\"/;()[]{}\0");
                int c = *s++;
                switch(c)
                {
                    case '\0': s--; quoteend = end = s; return true;
                    case '"': s = parsestring(s); if(*s == '"') s++; break;
                    case '/': if(*s == '/') s += strcspn(s, "\n\0"); break;
                    case '(': case '[': case '{': if(c == braktype) brak++; break;
                    case ')': if(braktype == '(' && --brak <= 0) goto endblock; break;
                    case ']': if(braktype == '[' && --brak <= 0) goto endblock; break;
                    case '}': if(braktype == '{' && --brak <= 0) goto endblock; break;
                }
            }
        endblock:
            end = s-1;
            quoteend = s;
            break;
        case '\0': case ')': case ']': case '}': return false;
        default: quotestart = start = s; s = parseword(s); quoteend = end = s; break;
    }
    skiplist(s);
    if(*s == ';') s++;
    return true;
}

void explodelist(const char *s, vector<char *> &elems, int limit)
{
    const char *start, *end;
    while((limit < 0 || elems.length() < limit) && parselist(s, start, end))
        elems.add(newstring(start, end-start));
}

char *indexlist(const char *s, int pos)
{
    loopi(pos) if(!parselist(s)) return newstring("");
    const char *start, *end;
    return parselist(s, start, end) ? newstring(start, end-start) : newstring("");
}

int listlen(const char *s)
{
    int n = 0;
    while(parselist(s)) n++;
    return n;
}

const char *indexlist(const char *s, int pos, int &len)
{
    loopi(pos) if(!parselist(s)) break;
    const char *start = s, *end = s;
    parselist(s, start, end);
    len = end-start;
    return start;
}

void at(char *s, int *pos)
{
    commandret->setstr(indexlist(s, *pos));
}

void substr(char *s, int *start, int *count, int *numargs)
{
    int len = strlen(s), offset = clamp(*start, 0, len);
    commandret->setstr(newstring(&s[offset], *numargs >= 3 ? clamp(*count, 0, len - offset) : len - offset));
}

void sublist(const char *s, int *skip, int *count, int *numargs)
{
    int offset = max(*skip, 0), len = *numargs >= 3 ? max(*count, 0) : -1;
    loopi(offset) if(!parselist(s)) break;
    if(len < 0) { if(offset > 0) skiplist(s); commandret->setstr(newstring(s)); return; }
    const char *list = s, *start, *end, *qstart, *qend = s;
    if(len > 0 && parselist(s, start, end, list, qend)) while(--len > 0 && parselist(s, start, end, qstart, qend));
    commandret->setstr(newstring(list, qend - list));
}

void getalias_(char *s)
{
    result(getalias(s));
}

ICOMMAND(0, exec, "si", (char *file, int *n), execfile(file, true, *n!=0));
COMMAND(0, at, "si");
ICOMMAND(0, escape, "s", (char *s), result(escapestring(s)));
ICOMMAND(0, unescape, "s", (char *s),
{
    int len = strlen(s);
    char *d = newstring(len);
    d[unescapestring(d, s, &s[len])] = '\0';
    stringret(d);
});
COMMAND(0, substr, "siiN");
COMMAND(0, sublist, "siiN");
ICOMMAND(0, listlen, "s", (char *s), intret(listlen(s)));
ICOMMAND(0, indexof, "ss", (char *list, char *elem), intret(listincludes(list, elem, strlen(elem))));
ICOMMAND(0, shrinklist, "ssi", (char *s, char *t, int *n), commandret->setstr(shrinklist(s, t, *n)));
COMMANDN(0, getalias, getalias_, "s");

void looplist(ident *id, const char *list, const uint *body, bool search)
{
    if(id->type!=ID_ALIAS) { if(search) intret(-1); return; }
    identstack stack;
    int n = 0;
    for(const char *s = list, *start, *end; parselist(s, start, end);)
    {
        char *val = newstring(start, end-start);
        if(n++)
        {
            if(id->valtype == VAL_STR) delete[] id->val.s;
            else id->valtype = VAL_STR;
            cleancode(*id);
            id->val.s = val;
        }
        else
        {
            tagval t;
            t.setstr(val);
            pusharg(*id, t, stack);
            id->flags &= ~IDF_UNKNOWN;
        }
        if(executebool(body) && search) { intret(n-1); search = false; break; }
    }
    if(search) intret(-1);
    if(n) poparg(*id);
}

void prettylist(const char *s, const char *conj)
{
    vector<char> p;
    const char *start, *end;
    for(int len = listlen(s), n = 0; parselist(s, start, end); n++)
    {
        p.put(start, end - start);
        if(n+1 < len)
        {
            if(len > 2 || !conj[0]) p.add(',');
            if(n+2 == len && conj[0])
            {
                p.add(' ');
                p.put(conj, strlen(conj));
            }
            p.add(' ');
        }
    } 
    p.add('\0');
    result(p.getbuf());
}
COMMAND(0, prettylist, "ss");

int listincludes(const char *list, const char *needle, int needlelen)
{
    int offset = 0;
    for(const char *s = list, *start, *end; parselist(s, start, end);)
    {
        int len = end - start;
        if(needlelen == len && !strncmp(needle, start, len)) return offset;
        offset++;
    }
    return -1;
}

char *listdel(const char *s, const char *del)
{
    vector<char> p;
    for(const char *start, *end, *qstart, *qend; parselist(s, start, end, qstart, qend);)
    {
        if(listincludes(del, start, end-start) < 0)
        {
            if(!p.empty()) p.add(' ');
            p.put(qstart, qend-qstart);
        }
    }
    p.add('\0');
    return newstring(p.getbuf(), p.length()-1);
}

char *shrinklist(const char *list, const char *limit, int failover)
{
    vector<char> p;
    for(const char *s = list, *start, *end, *qstart, *qend; parselist(s, start, end, qstart, qend);)
    {
        if(listincludes(limit, start, end-start) >= 0)
        {
            if(!p.empty()) p.add(' ');
            p.put(qstart, qend-qstart);
        }
    }
    if(failover && p.empty())
    {
        const char *all = "";
        switch(failover)
        {
            case 2: all = *limit ? limit : list; break;
            case 1: default: all = *list ? list : limit; break;
        }
        return newstring(all);
    }
    p.add('\0');
    return newstring(p.getbuf(), p.length()-1);
}

void listsplice(const char *s, const char *vals, int *skip, int *count, int *numargs)
{
    int offset = max(*skip, 0), len = *numargs >= 4 ? max(*count, 0) : -1;
    const char *list = s, *start, *end, *qstart, *qend = s;
    loopi(offset) if(!parselist(s, start, end, qstart, qend)) break;
    vector<char> p;
    if(qend > list) p.put(list, qend-list);
    if(*vals)
    {
        if(!p.empty()) p.add(' ');
        p.put(vals, strlen(vals));
    }
    while(len-- > 0) if(!parselist(s)) break;
    skiplist(s);
    switch(*s)
    {
        case '\0': case ')': case ']': case '}': break;
        default:
            if(!p.empty()) p.add(' ');
            p.put(s, strlen(s));
            break;
    }
    p.add('\0');
    commandret->setstr(newstring(p.getbuf(), p.length()-1));
}
COMMAND(0, listsplice, "ssiiN");

ICOMMAND(0, listdel, "ss", (char *list, char *del), commandret->setstr(listdel(list, del)));
ICOMMAND(0, shrinklist, "ssi", (char *s, char *t, int *n), commandret->setstr(shrinklist(s, t, *n)));
ICOMMAND(0, listfind, "rse", (ident *id, char *list, uint *body), looplist(id, list, body, true));
ICOMMAND(0, looplist, "rse", (ident *id, char *list, uint *body), looplist(id, list, body, false));
ICOMMAND(0, loopfiles, "rsse", (ident *id, char *dir, char *ext, uint *body),
{
    if(id->type!=ID_ALIAS) return;
    identstack stack;
    vector<char *> files;
    listfiles(dir, ext[0] ? ext : NULL, files);
    loopv(files)
    {
        char *file = files[i];
//        bool redundant = false;
//        loopj(i) if(!strcmp(files[j], file)) { redundant = true; break; }
//        if(redundant) { delete[] file; continue; }
        if(i)
        {
            if(id->valtype == VAL_STR) delete[] id->val.s;
            else id->valtype = VAL_STR;
            id->val.s = file;
        }
        else
        {
            tagval t;
            t.setstr(file);
            pusharg(*id, t, stack);
            id->flags &= ~IDF_UNKNOWN;
        }
        execute(body);
    }
    if(files.length()) poparg(*id);
});

ICOMMAND(0, +, "ii", (int *a, int *b), intret(*a + *b));
ICOMMAND(0, *, "ii", (int *a, int *b), intret(*a * *b));
ICOMMAND(0, -, "ii", (int *a, int *b), intret(*a - *b));
ICOMMAND(0, +f, "ff", (float *a, float *b), floatret(*a + *b));
ICOMMAND(0, *f, "ff", (float *a, float *b), floatret(*a * *b));
ICOMMAND(0, -f, "ff", (float *a, float *b), floatret(*a - *b));
ICOMMAND(0, =, "ii", (int *a, int *b), intret((int)(*a == *b)));
ICOMMAND(0, !=, "ii", (int *a, int *b), intret((int)(*a != *b)));
ICOMMAND(0, <, "ii", (int *a, int *b), intret((int)(*a < *b)));
ICOMMAND(0, >, "ii", (int *a, int *b), intret((int)(*a > *b)));
ICOMMAND(0, <=, "ii", (int *a, int *b), intret((int)(*a <= *b)));
ICOMMAND(0, >=, "ii", (int *a, int *b), intret((int)(*a >= *b)));
ICOMMAND(0, =f, "ff", (float *a, float *b), intret((int)(*a == *b)));
ICOMMAND(0, !=f, "ff", (float *a, float *b), intret((int)(*a != *b)));
ICOMMAND(0, <f, "ff", (float *a, float *b), intret((int)(*a < *b)));
ICOMMAND(0, >f, "ff", (float *a, float *b), intret((int)(*a > *b)));
ICOMMAND(0, <=f, "ff", (float *a, float *b), intret((int)(*a <= *b)));
ICOMMAND(0, >=f, "ff", (float *a, float *b), intret((int)(*a >= *b)));
ICOMMAND(0, ^, "ii", (int *a, int *b), intret(*a ^ *b));
ICOMMAND(0, !, "t", (tagval *a), intret(!getbool(*a)));
ICOMMAND(0, &, "ii", (int *a, int *b), intret(*a & *b));
ICOMMAND(0, |, "ii", (int *a, int *b), intret(*a | *b));
ICOMMAND(0, ~, "i", (int *a), intret(~*a));
ICOMMAND(0, ^~, "ii", (int *a, int *b), intret(*a ^ ~*b));
ICOMMAND(0, &~, "ii", (int *a, int *b), intret(*a & ~*b));
ICOMMAND(0, |~, "ii", (int *a, int *b), intret(*a | ~*b));
ICOMMAND(0, <<, "ii", (int *a, int *b), intret(*a << *b));
ICOMMAND(0, >>, "ii", (int *a, int *b), intret(*a >> *b));
ICOMMAND(0, &&, "e1V", (tagval *args, int numargs),
{
    if(!numargs) intret(1);
    else loopi(numargs)
    {
        if(i) freearg(*commandret);
        executeret(args[i].code, *commandret);
        if(!getbool(*commandret)) break;
    }
});
ICOMMAND(0, ||, "e1V", (tagval *args, int numargs),
{
    if(!numargs) intret(0);
    else loopi(numargs)
    {
        if(i) freearg(*commandret);
        executeret(args[i].code, *commandret);
        if(getbool(*commandret)) break;
    }
});

ICOMMAND(0, div, "ii", (int *a, int *b), intret(*b ? *a / *b : 0));
ICOMMAND(0, mod, "ii", (int *a, int *b), intret(*b ? *a % *b : 0));
ICOMMAND(0, divf, "ff", (float *a, float *b), floatret(*b ? *a / *b : 0));
ICOMMAND(0, modf, "ff", (float *a, float *b), floatret(*b ? fmod(*a, *b) : 0));
ICOMMAND(0, sin, "f", (float *a), floatret(sin(*a*RAD)));
ICOMMAND(0, cos, "f", (float *a), floatret(cos(*a*RAD)));
ICOMMAND(0, tan, "f", (float *a), floatret(tan(*a*RAD)));
ICOMMAND(0, asin, "f", (float *a), floatret(asin(*a)/RAD));
ICOMMAND(0, acos, "f", (float *a), floatret(acos(*a)/RAD));
ICOMMAND(0, atan, "f", (float *a), floatret(atan(*a)/RAD));
ICOMMAND(0, sqrt, "f", (float *a), floatret(sqrt(*a)));
ICOMMAND(0, pow, "ff", (float *a, float *b), floatret(pow(*a, *b)));
ICOMMAND(0, loge, "f", (float *a), floatret(log(*a)));
ICOMMAND(0, log2, "f", (float *a), floatret(log(*a)/M_LN2));
ICOMMAND(0, log10, "f", (float *a), floatret(log10(*a)));
ICOMMAND(0, exp, "f", (float *a), floatret(exp(*a)));
ICOMMAND(0, min, "V", (tagval *args, int numargs),
{
    int val = numargs > 0 ? args[numargs - 1].getint() : 0;
    loopi(numargs - 1) val = min(val, args[i].getint());
    intret(val);
});
ICOMMAND(0, max, "V", (tagval *args, int numargs),
{
    int val = numargs > 0 ? args[numargs - 1].getint() : 0;
    loopi(numargs - 1) val = max(val, args[i].getint());
    intret(val);
});
ICOMMAND(0, minf, "V", (tagval *args, int numargs),
{
    float val = numargs > 0 ? args[numargs - 1].getfloat() : 0.0f;
    loopi(numargs - 1) val = min(val, args[i].getfloat());
    floatret(val);
});
ICOMMAND(0, maxf, "V", (tagval *args, int numargs),
{
    float val = numargs > 0 ? args[numargs - 1].getfloat() : 0.0f;
    loopi(numargs - 1) val = max(val, args[i].getfloat());
    floatret(val);
});
ICOMMAND(0, precf, "fi", (float *a, int *b),
{
    defformatstring(format)("%%.%df", max(*b, 0));
    defformatstring(retval)(format, *a);
    result(retval);
});

ICOMMAND(0, cond, "ee2V", (tagval *args, int numargs),
{
    for(int i = 0; i < numargs; i += 2)
    {
        if(executebool(args[i].code))
        {
            if(i+1 < numargs) executeret(args[i+1].code, *commandret);
            break;
        }
    }
});
#define CASECOMMAND(name, fmt, type, acc, compare) \
    ICOMMAND(0, name, fmt "te2V", (tagval *args, int numargs), \
    { \
        type val = acc; \
        int i; \
        for(i = 1; i+1 < numargs; i += 2) \
        { \
            if(compare) \
            { \
                executeret(args[i+1].code, *commandret); \
                return; \
            } \
        } \
    })
CASECOMMAND(case, "i", int, args[0].getint(), args[i].type == VAL_NULL || args[i].getint() == val);
CASECOMMAND(casef, "f", float, args[0].getfloat(), args[i].type == VAL_NULL || args[i].getfloat() == val);
CASECOMMAND(cases, "s", const char *, args[0].getstr(), args[i].type == VAL_NULL || !strcmp(args[i].getstr(), val));

ICOMMAND(0, rnd, "ii", (int *a, int *b), intret(*a - *b > 0 ? rnd(*a - *b) + *b : *b));
ICOMMAND(0, strcmp, "ss", (char *a, char *b), intret(strcmp(a,b)==0));
ICOMMAND(0, =s, "ss", (char *a, char *b), intret(strcmp(a,b)==0));
ICOMMAND(0, !=s, "ss", (char *a, char *b), intret(strcmp(a,b)!=0));
ICOMMAND(0, <s, "ss", (char *a, char *b), intret(strcmp(a,b)<0));
ICOMMAND(0, >s, "ss", (char *a, char *b), intret(strcmp(a,b)>0));
ICOMMAND(0, <=s, "ss", (char *a, char *b), intret(strcmp(a,b)<=0));
ICOMMAND(0, >=s, "ss", (char *a, char *b), intret(strcmp(a,b)>=0));
ICOMMAND(0, strcasecmp, "ss", (char *a, char *b), intret(strcasecmp(a,b)==0));
ICOMMAND(0, strncmp, "ssi", (char *a, char *b, int *n), intret(strncmp(a,b,*n)==0));
ICOMMAND(0, strncasecmp, "ssi", (char *a, char *b, int *n), intret(strncasecmp(a,b,*n)==0));
ICOMMAND(0, echo, "C", (char *s), conoutft(CON_MESG, "%s", s));
ICOMMAND(0, strstr, "ss", (char *a, char *b), { char *s = strstr(a, b); intret(s ? s-a : -1); });
ICOMMAND(0, strlen, "s", (char *s), intret(strlen(s)));

char *strreplace(const char *s, const char *oldval, const char *newval)
{
    vector<char> buf;

    int oldlen = strlen(oldval);
    for(;;)
    {
        const char *found = strstr(s, oldval);
        if(found)
        {
            while(s < found) buf.add(*s++);
            for(const char *n = newval; *n; n++) buf.add(*n);
            s = found + oldlen;
        }
        else
        {
            while(*s) buf.add(*s++);
            buf.add('\0');
            return newstring(buf.getbuf(), buf.length());
        }
    }
}

ICOMMAND(0, strreplace, "sss", (char *s, char *o, char *n), commandret->setstr(strreplace(s, o, n)));

void nonworld(uint *contents)
{
    int oldflags = identflags;
    identflags &= ~IDF_WORLD;
    execute(contents);
    identflags = oldflags;
}
COMMAND(0, nonworld, "e");

struct sleepcmd
{
    int delay, millis;
    char *command;
    int flags;
};
vector<sleepcmd> sleepcmds;

void addsleep(int *msec, char *cmd)
{
    sleepcmd &s = sleepcmds.add();
    s.delay = max(*msec, 1);
    s.millis = lastmillis;
    s.command = newstring(cmd);
    s.flags = identflags;
}

ICOMMAND(0, sleep, "is", (int *a, char *b), addsleep(a, b));

void checksleep(int millis)
{
    loopv(sleepcmds)
    {
        sleepcmd &s = sleepcmds[i];
        if(millis - s.millis >= s.delay)
        {
            int oldflags = identflags;
            identflags = s.flags;
            char *cmd = s.command; // execute might create more sleep commands
            s.command = NULL;
            execute(cmd);
            delete[] cmd;
            identflags = oldflags;
            if(sleepcmds.inrange(i) && !sleepcmds[i].command) sleepcmds.remove(i--);
        }
    }
}

void clearsleep(bool clearworlds)
{
    int len = 0;
    loopv(sleepcmds) if(sleepcmds[i].command)
    {
        if(!clearworlds || sleepcmds[i].flags&IDF_WORLD)
            delete[] sleepcmds[i].command;
        else sleepcmds[len++] = sleepcmds[i];
    }
    sleepcmds.shrink(len);
}

ICOMMAND(0, clearsleep, "i", (int *worlds), clearsleep(*worlds!=0 || identflags&IDF_WORLD));
ICOMMAND(0, exists, "ss", (char *a, char *b), intret(fileexists(a, *b ? b : "r")));
ICOMMAND(0, getmillis, "i", (int *total), intret(*total ? totalmillis : lastmillis));

void getvariable(int num)
{
    mkstring(text); num--;
    static vector<ident *> ids;
    static int lastupdate = 0;
    if(ids.empty() || !lastupdate || totalmillis-lastupdate >= 60000)
    {
        ids.setsize(0);
        enumerate(idents, ident, id, ids.add(&id));
        lastupdate = totalmillis;
    }
    if(ids.inrange(num))
    {
        ids.sort(ident::compare);
        formatstring(text)("%s", ids[num]->name);
    }
    else formatstring(text)("%d", ids.length());
    result(text);
}
ICOMMAND(0, getvariable, "i", (int *n), getvariable(*n));

void hexcolour(int *n)
{
    defformatstring(s)(*n >= 0 && *n <= 0xFFFFFF ? "0x%.6X" : "%d", *n);
    result(s);
}

COMMAND(0, hexcolour, "i");

