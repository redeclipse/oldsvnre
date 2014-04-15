#include "engine.h"

int uimillis = -1;

VAR(IDF_READONLY, guilayoutpass, 1, 0, -1);
bool guiactionon = false;
int mouseaction[2] = {0}, guibound[2] = {0};

static float firstx, firsty;

enum {FIELDCOMMIT, FIELDABORT, FIELDEDIT, FIELDSHOW, FIELDKEY};
static int fieldmode = FIELDSHOW;
static bool fieldsactive = false;

VAR(IDF_PERSIST, guishadow, 0, 2, 8);
VAR(IDF_PERSIST, guiclicktab, 0, 1, 1);
VAR(IDF_PERSIST, guitextblend, 1, 255, 255);
VAR(IDF_PERSIST, guitextfade, 1, 200, 255);
VAR(IDF_PERSIST, guilinesize, 1, 36, 128);
VAR(IDF_PERSIST, guisepsize, 1, 10, 128);
VAR(IDF_PERSIST, guiscaletime, 0, 250, VAR_MAX);
VAR(IDF_PERSIST|IDF_HEX, guibgcolour, -1, 0x000000, 0xFFFFFF);
FVAR(IDF_PERSIST, guibgblend, 0, 0.5f, 1);
VAR(IDF_PERSIST|IDF_HEX, guibordercolour, -1, 0x000000, 0xFFFFFF);
FVAR(IDF_PERSIST, guiborderblend, 0, 1.f, 1);
SVAR(0, guistatustext, "");
FVAR(IDF_PERSIST, guihoverscale, 0, 0.3f, 1);
SVAR(0, guitooltiptext, "");
VAR(IDF_PERSIST, guitooltiptime, 0, 350, VAR_MAX);
VAR(IDF_PERSIST|IDF_HEX, guitooltipcolour, -1, 0x100000, 0xFFFFFF);
FVAR(IDF_PERSIST, guitooltipblend, 0, 0.9f, 1);
VAR(IDF_PERSIST|IDF_HEX, guitooltipbordercolour, -1, 0x100000, 0xFFFFFF);
FVAR(IDF_PERSIST, guitooltipborderblend, 0, 1.f, 1);

static bool needsinput = false, hastitle = true, hasbgfx = true;
static char *tooltip = NULL;
static int lasttooltip = 0;

#include "textedit.h"
struct gui : guient
{
    struct list { int parent, w, h, springs, curspring, mouse[2]; };

    int nextlist;
    static vector<list> lists;
    static float hitx, hity;
    static int curdepth, curlist, xsize, ysize, curx, cury, fontdepth, mergelist, mergedepth;
    static bool hitfx;

    static void reset()
    {
        lists.shrink(0);
        mergelist = mergedepth = -1;
    }

    static int ty, tx, tpos, *tcurrent, tcolor; //tracking tab size and position since uses different layout method...

    void allowhitfx(bool on) { hitfx = on; }
    bool visibletab() { return !tcurrent || tpos == *tcurrent; }
    bool visible() { return !guilayoutpass && visibletab(); }

    //tab is always at top of page
    void tab(const char *name, int color, bool front)
    {
        if(curdepth != 0) return;
        tpos++;
        if(front && tcurrent && *tcurrent != tpos) *tcurrent = tpos;
        if(!hastitle)
        {
            if(guilayoutpass)
            {
                ty = max(ty, ysize);
                ysize = 0;
            }
            else cury = -ysize;
            return;
        }
        if(color) tcolor = color;
        if(!name) name = intstr(tpos);
        gui::pushfont("super");
        int w = text_width(name);
        if(guilayoutpass)
        {
            ty = max(ty, ysize);
            ysize = 0;
        }
        else
        {
            cury = -ysize;
            int x1 = curx+tx, x2 = x1+w+guibound[0]*2, y1 = cury-guibound[1]*2, y2 = cury-guibound[1]/2, alpha = guitextblend;
            if(!visibletab())
            {
                if(tcurrent && hitx>=x1 && hity>=y1 && hitx<x2 && hity<y2)
                {
                    if(!guiclicktab || mouseaction[0]&GUI_UP) *tcurrent = tpos; // switch tab
                    tcolor = 0xFF4444;
                    alpha = max(alpha, guitextfade);
                }
                else tcolor = vec::hexcolor(tcolor).mul(0.25f).tohexcolor();
            }
            if(hasbgfx && guibgcolour >= 0)
            {
                notextureshader->set();
                glDisable(GL_TEXTURE_2D);
                glColor4f((guibgcolour>>16)/255.f, ((guibgcolour>>8)&0xFF)/255.f, (guibgcolour&0xFF)/255.f, guibgblend);
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2f(x1, y1);
                glVertex2f(x2, y1);
                glVertex2f(x1, y2);
                glVertex2f(x2, y2);
                glEnd();
                defaultshader->set();
                glEnable(GL_TEXTURE_2D);
            }
            if(hasbgfx && guibordercolour >= 0)
            {
                lineshader->set();
                glDisable(GL_TEXTURE_2D);
                glColor4f((guibordercolour>>16)/255.f, ((guibordercolour>>8)&0xFF)/255.f, (guibordercolour&0xFF)/255.f, guiborderblend);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x1, y1);
                glVertex2f(x2, y1);
                glVertex2f(x2, y2);
                glVertex2f(x1, y2);
                glEnd();
                defaultshader->set();
                glEnable(GL_TEXTURE_2D);
            }
            x1 += guibound[0];
            y1 += guibound[1]-FONTH/3*2;
            text_(name, x1, y1, tcolor, alpha, visible());
        }
        tx += w+guibound[0]*3;
        gui::popfont();
    }

    void uibuttons()
    {
        if(guilayoutpass) return;
        cury = -ysize;
        int x1 = curx+max(xsize-guibound[1]*3/2, tx), x2 = x1+guibound[1]*2, y1 = cury-guibound[1]*2, y2 = cury-guibound[1]/2;
        #define uibtn(a,b) \
        { \
            bool hit = false; \
            if(hitx>=x1 && hity>=y1 && hitx<x2 && hity<y2) \
            { \
                if(mouseaction[0]&GUI_UP) { b; } \
                hit = true; \
            } \
            if(hasbgfx && guibgcolour >= 0) \
            { \
                notextureshader->set(); \
                glDisable(GL_TEXTURE_2D); \
                glColor4f((guibgcolour>>16)/255.f, ((guibgcolour>>8)&0xFF)/255.f, (guibgcolour&0xFF)/255.f, guibgblend); \
                glBegin(GL_TRIANGLE_STRIP); \
                glVertex2f(x1, y1); \
                glVertex2f(x2, y1); \
                glVertex2f(x1, y2); \
                glVertex2f(x2, y2); \
                glEnd(); \
                defaultshader->set(); \
                glEnable(GL_TEXTURE_2D); \
            } \
            if(hasbgfx && guibordercolour >= 0) \
            { \
                lineshader->set(); \
                glDisable(GL_TEXTURE_2D); \
                glColor4f((guibordercolour>>16)/255.f, ((guibordercolour>>8)&0xFF)/255.f, (guibordercolour&0xFF)/255.f, guiborderblend); \
                glBegin(GL_LINE_LOOP); \
                glVertex2f(x1, y1); \
                glVertex2f(x2, y1); \
                glVertex2f(x2, y2); \
                glVertex2f(x1, y2); \
                glEnd(); \
                defaultshader->set(); \
                glEnable(GL_TEXTURE_2D); \
            } \
            x1 += guibound[1]/2; \
            y1 += guibound[1]/4; \
            icon_(a, false, x1, y1, guibound[1], hit, 0xFFFFFF); \
            y1 += guibound[1]*3/2; \
        }
        if(!exittex) exittex = textureload(guiexittex, 3, true, false); \
        uibtn(exittex, cleargui(1));
    }

    bool ishorizontal() const { return curdepth&1; }
    bool isvertical() const { return !ishorizontal(); }

    void pushlist(bool merge)
    {
        if(guilayoutpass)
        {
            if(curlist>=0)
            {
                lists[curlist].w = xsize;
                lists[curlist].h = ysize;
            }
            list &l = lists.add();
            l.parent = curlist;
            l.springs = 0;
            curlist = lists.length()-1;
            l.mouse[0] = l.mouse[1] = xsize = ysize = 0;
        }
        else
        {
            curlist = nextlist++;
            if(curlist >= lists.length()) // should never get here unless script code doesn't use same amount of lists in layout and render passes
            {
                list &l = lists.add();
                l.parent = curlist;
                l.springs = 0;
                l.w = l.h = l.mouse[0] = l.mouse[1] = 0;
            }
            list &l = lists[curlist];
            l.curspring = 0;
            if(l.springs > 0)
            {
                if(ishorizontal()) xsize = l.w; else ysize = l.h;
            }
            else
            {
                xsize = l.w;
                ysize = l.h;
            }
        }
        curdepth++;
        if(!guilayoutpass && visible() && ishit(xsize, ysize)) loopi(2) lists[curlist].mouse[i] = mouseaction[i]|GUI_ROLLOVER;
        if(merge)
        {
            mergelist = curlist;
            mergedepth = curdepth;
        }
    }

    int poplist()
    {
        if(!lists.inrange(curlist)) return 0;
        list &l = lists[curlist];
        if(guilayoutpass)
        {
            l.w = xsize;
            l.h = ysize;
        }
        curlist = l.parent;
        curdepth--;
        if(mergelist >= 0 && curdepth < mergedepth) mergelist = mergedepth = -1;
        if(lists.inrange(curlist))
        {
            int w = xsize, h = ysize;
            if(ishorizontal()) cury -= h; else curx -= w;
            list &p = lists[curlist];
            xsize = p.w;
            ysize = p.h;
            if(!guilayoutpass && p.springs > 0)
            {
                list &s = lists[p.parent];
                if(ishorizontal()) xsize = s.w; else ysize = s.h;
            }
            return layout(w, h);
        }
        return 0;
    }

    int text  (const char *text, int color, const char *icon, int icolor) { return button_(text, color, icon, icolor, false, false); }
    int button(const char *text, int color, const char *icon, int icolor, bool faded) { return button_(text, color, icon, icolor, true, faded); }
    int title (const char *text, int color, const char *icon, int icolor) { return button_(text, color, icon, icolor, false, false, "emphasis"); }

    void separator() { line_(guisepsize); }

    //use to set min size (useful when you have progress bars)
    void strut(float size) { layout(isvertical() ? int(size*guibound[0]) : 0, isvertical() ? 0 : int(size*guibound[1])); }
    //add space between list items
    void space(float size) { layout(isvertical() ? 0 : int(size*guibound[0]), isvertical() ? int(size*guibound[1]) : 0); }

    void pushfont(const char *font) { ::pushfont(font); fontdepth++; }
    void popfont() { if(fontdepth) { ::popfont(); fontdepth--; } }

    void spring(int weight)
    {
        if(curlist < 0) return;
        list &l = lists[curlist];
        if(guilayoutpass) { if(l.parent >= 0) l.springs += weight; return; }
        int nextspring = min(l.curspring + weight, l.springs);
        if(nextspring <= l.curspring) return;
        if(ishorizontal())
        {
            int w = xsize - l.w;
            layout((w*nextspring)/l.springs - (w*l.curspring)/l.springs, 0);
        }
        else
        {
            int h = ysize - l.h;
            layout(0, (h*nextspring)/l.springs - (h*l.curspring)/l.springs);
        }
        l.curspring = nextspring;
    }

    int layout(int w, int h)
    {
        if(guilayoutpass)
        {
            if(ishorizontal())
            {
                xsize += w;
                ysize = max(ysize, h);
            }
            else
            {
                xsize = max(xsize, w);
                ysize += h;
            }
        }
        else
        {
            bool hit = ishit(w, h);
            if(ishorizontal()) curx += w;
            else cury += h;
            if(hit && visible()) return mouseaction[0]|GUI_ROLLOVER;
        }
        return 0;
    }

    bool ishit(int w, int h, int x = curx, int y = cury)
    {
        if(passthrough) return false;
        if(mergelist >= 0 && curdepth >= mergedepth && lists[mergelist].mouse[0]) return true;
        if(ishorizontal()) h = ysize;
        else w = xsize;
        return hitx>=x && hity>=y && hitx<x+w && hity<y+h;
    }

    int image(Texture *t, float scale, bool overlaid, int icolor)
    {
        if(scale == 0) scale = 1;
        int size = (int)(scale*2*guibound[1])-guishadow;
        if(visible()) icon_(t, overlaid, curx, cury, size, ishit(size+guishadow, size+guishadow), icolor);
        return layout(size+guishadow, size+guishadow);
    }

    int texture(VSlot &vslot, float scale, bool overlaid)
    {
        if(scale==0) scale = 1;
        int size = (int)(scale*2*guibound[1])-guishadow;
        if(visible()) previewslot(vslot, overlaid, curx, cury, size, ishit(size+guishadow, size+guishadow));
        return layout(size+guishadow, size+guishadow);
    }

    int playerpreview(int model, int color, int team, int weap, const char *vanity, float sizescale, bool overlaid, float scale, float blend)
    {
        if(sizescale==0) sizescale = 1;
        int size = (int)(sizescale*2*guibound[1])-guishadow;
        if(visible())
        {
            bool hit = ishit(size+guishadow, size+guishadow);
            float xs = size, ys = size, xi = curx, yi = cury, xpad = 0, ypad = 0;
            if(overlaid)
            {
                xpad = xs/32;
                ypad = ys/32;
                xi += xpad;
                yi += ypad;
                xs -= 2*xpad;
                ys -= 2*ypad;
            }
            int x1 = int(floor(screen->w*(xi*uiscale.x+uiorigin.x))), y1 = int(floor(screen->h*(1 - ((yi+ys)*uiscale.y+uiorigin.y)))),
                x2 = int(ceil(screen->w*((xi+xs)*uiscale.x+uiorigin.x))), y2 = int(ceil(screen->h*(1 - (yi*uiscale.y+uiorigin.y))));
            glViewport(x1, y1, x2-x1, y2-y1);
            glScissor(x1, y1, x2-x1, y2-y1);
            glEnable(GL_SCISSOR_TEST);
            glDisable(GL_BLEND);
            modelpreview::start(overlaid);
            game::renderplayerpreview(model, color, team, weap, vanity, scale, blend);
            modelpreview::end();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glDisable(GL_SCISSOR_TEST);
            glViewport(0, 0, screen->w, screen->h);
            if(overlaid)
            {
                if(!overlaytex) overlaytex = textureload(guioverlaytex, 3, true, false);
                const vec &ocolor = hit && hitfx ? vec(1, 0.25f, 0.25f) : vec(1, 1, 1);
                glColor3fv(ocolor.v);
                glBindTexture(GL_TEXTURE_2D, overlaytex->id);
                rect_(xi - xpad, yi - ypad, xs + 2*xpad, ys + 2*ypad, 0);
            }
        }
        return layout(size+guishadow, size+guishadow);
    }

    int modelpreview(const char *name, int anim, float sizescale, bool overlaid, float scale, float blend)
    {
        if(sizescale==0) sizescale = 1;
        int size = (int)(sizescale*2*guibound[1])-guishadow;
        if(visible())
        {
            bool hit = ishit(size+guishadow, size+guishadow);
            float xs = size, ys = size, xi = curx, yi = cury, xpad = 0, ypad = 0;
            if(overlaid)
            {
                xpad = xs/32;
                ypad = ys/32;
                xi += xpad;
                yi += ypad;
                xs -= 2*xpad;
                ys -= 2*ypad;
            }
            int x1 = int(floor(screen->w*(xi*uiscale.x+uiorigin.x))), y1 = int(floor(screen->h*(1 - ((yi+ys)*uiscale.y+uiorigin.y)))),
                x2 = int(ceil(screen->w*((xi+xs)*uiscale.x+uiorigin.x))), y2 = int(ceil(screen->h*(1 - (yi*uiscale.y+uiorigin.y))));
            glViewport(x1, y1, x2-x1, y2-y1);
            glScissor(x1, y1, x2-x1, y2-y1);
            glEnable(GL_SCISSOR_TEST);
            glDisable(GL_BLEND);
            modelpreview::start(overlaid);
            model *m = loadmodel(name);
            if(m)
            {
                entitylight light;
                light.color = vec(1, 1, 1);
                light.dir = vec(0, -1, 2).normalize();
                vec center, radius;
                m->boundbox(center, radius);
                float dist =  2.0f*max(radius.magnitude2(), 1.1f*radius.z),
                      yaw = fmod(lastmillis/10000.0f*360.0f, 360.0f);
                vec o(-center.x, dist - center.y, -0.1f*dist - center.z);
                rendermodel(&light, name, anim, o, yaw, 0, 0, NULL, NULL, 0, scale, blend);
            }
            modelpreview::end();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glDisable(GL_SCISSOR_TEST);
            glViewport(0, 0, screen->w, screen->h);
            if(overlaid)
            {
                if(!overlaytex) overlaytex = textureload(guioverlaytex, 3, true, false);
                const vec &ocolor = hit && hitfx ? vec(1, 0.25f, 0.25f) : vec(1, 1, 1);
                glColor3fv(ocolor.v);
                glBindTexture(GL_TEXTURE_2D, overlaytex->id);
                rect_(xi - xpad, yi - ypad, xs + 2*xpad, ys + 2*ypad, 0);
            }
        }
        return layout(size+guishadow, size+guishadow);
    }

    int slice(Texture *t, float scale, float start, float end, const char *text)
    {
        if(scale == 0) scale = 1;
        int size = (int)(scale*2*guibound[1]);
        if(t!=notexture && visible()) slice_(t, curx, cury, size, start, end, text);
        return layout(size, size);
    }

    void progress(float percent, float scale)
    {
        if(scale == 0) scale = 1;
        int size = (int)(scale*2*guibound[1]);
        slice_(textureload(hud::progringtex, 3, true, false), curx, cury, size, (SDL_GetTicks()%1000)/1000.f, 0.1f);
        string s; if(percent > 0) formatstring(s)("\fg%d%%", int(percent*100)); else formatstring(s)("\fg...");
        slice_(textureload(hud::progresstex, 3, true, false), curx, cury, size, 0, percent, s);
        layout(size, size);
    }

    void slider(int &val, int vmin, int vmax, int color, const char *label, bool reverse, bool scroll)
    {
        int x = curx;
        int y = cury;
        int space = line_(guilinesize, 1.0f, ishorizontal() ? guibound[0]*3 : guibound[1]);
        if(visible())
        {
            pushfont("emphasis");
            if(!label) label = intstr(val);
            int w = text_width(label);

            bool hit = false, forcecolor = false;
            int px, py;
            if(ishorizontal())
            {
                hit = ishit(guilinesize, ysize, x + space/2 - guilinesize/2, y);
                px = x + space/2 - w/2;
                py = ((ysize-guibound[1])*(val-vmin))/max(vmax-vmin, 1);
                if(reverse) py += y; //vmin at top
                else py = y + (ysize-guibound[1]) - py; //vmin at bottom
            }
            else
            {
                hit = ishit(xsize, guilinesize, x, y + space/2 - guilinesize/2);
                px = ((xsize-w)*(val-vmin))/max(vmax-vmin, 1);
                if(reverse) px = x + (xsize-guibound[0]/2-w/2) - px; //vmin at right
                else px += x + guibound[0]/2 - w/2; //vmin at left
                py = y + space/2 - FONTH/2;
            }
            if(hit && hitfx) { forcecolor = true; color = 0xFF4444; }
            text_(label, px, py, color, hit && hitfx ? guitextblend : guitextfade, hit && mouseaction[0]&GUI_DOWN, forcecolor);
            if(hit)
            {
                if(mouseaction[0]&GUI_PRESSED)
                {
                    int vnew = vmax-vmin+1;
                    if(ishorizontal()) vnew = int((vnew*(reverse ? hity-y-guibound[1]/2 : y+ysize-guibound[1]/2-hity))/(ysize-guibound[1]));
                    else vnew = int((vnew*(reverse ? x+xsize-guibound[0]/2-hitx : hitx-x-guibound[0]/2))/(xsize-w));
                    vnew += vmin;
                    vnew = clamp(vnew, vmin, vmax);
                    if(vnew != val) val = vnew;
                }
                else if(mouseaction[1]&GUI_UP)
                {
                    int vval = val+(reverse == !(mouseaction[1]&GUI_ALT) ? -1 : 1),
                        vnew = clamp(vval, vmin, vmax);
                    if(vnew != val) val = vnew;
                }
            }
            else if(scroll && lists[curlist].mouse[1]&GUI_UP)
            {
                int vval = val+(reverse == !(lists[curlist].mouse[1]&GUI_ALT) ? -1 : 1),
                    vnew = clamp(vval, vmin, vmax);
                if(vnew != val) val = vnew;
            }
            popfont();
        }
    }

    char *field(const char *name, int color, int length, int height, const char *initval, int initmode, bool focus, const char *parent)
    {
        return field_(name, color, length, height, initval, initmode, FIELDEDIT, focus, parent, "console");
    }

    char *keyfield(const char *name, int color, int length, int height, const char *initval, int initmode, bool focus, const char *parent)
    {
        return field_(name, color, length, height, initval, initmode, FIELDKEY, focus, parent, "console");
    }

    char *field_(const char *name, int color, int length, int height, const char *initval, int initmode, int fieldtype = FIELDEDIT, bool focus = false, const char *parent = NULL, const char *font = "")
    {
        if(font && *font) gui::pushfont(font);
        editor *e = useeditor(name, initmode, false, initval, parent); // generate a new editor if necessary
        if(guilayoutpass)
        {
            if(initval && e->mode==EDITORFOCUSED && (e!=currentfocus() || fieldmode == FIELDSHOW))
            {
                if(strcmp(e->lines[0].text, initval)) e->clear(initval);
            }
            e->linewrap = (length<0);
            e->maxx = (e->linewrap) ? -1 : length;
            e->maxy = (height<=0)?1:-1;
            e->pixelwidth = abs(length)*FONTW;
            if(e->linewrap && e->maxy==1)
            {
                int temp;
                text_bounds(e->lines[0].text, temp, e->pixelheight, e->pixelwidth); //only single line editors can have variable height
            }
            else
                e->pixelheight = FONTH*max(height, 1);
        }
        int h = e->pixelheight, hpad = 0, w = e->pixelwidth, wpad = guibound[0];
        if((h+hpad)%guibound[1]) hpad += guibound[1] - (h+hpad)%guibound[1];
        h += hpad;
        if((w+wpad)%guibound[0]) wpad += guibound[0] - (w+wpad)%guibound[0];
        w += wpad;

        bool wasvertical = isvertical();
        if(wasvertical && e->maxy != 1) pushlist(false);

        char *result = NULL;
        if(visible())
        {
            e->rendered = true;
            if(focus && e->unfocus) focus = false;
            bool hit = ishit(w, h) && e->mode!=EDITORREADONLY, clrs = fieldtype == FIELDKEY,
                 editing = (fieldmode != FIELDSHOW) && e==currentfocus() && e->mode!=EDITORREADONLY;
            if(mouseaction[0]&GUI_UP && mergedepth >= 0 && hit) mouseaction[0] &= ~GUI_UP;
            if(mouseaction[0]&GUI_DOWN) //mouse request focus
            {
                if(hit)
                {
                    focus = true;
                    if(mouseaction[0]&GUI_ALT) clrs = true;
                    if(e->unfocus) e->unfocus = false;
                }
                else if(editing) fieldmode = FIELDCOMMIT;
            }
            if(focus)
            {
                if(clrs) e->clear();
                useeditor(e->name, initmode, true, initval, parent);
                e->mark(false);
                if(fieldmode != FIELDCOMMIT && fieldmode != FIELDABORT) fieldmode = fieldtype;
            }
            if(hit && editing && (mouseaction[0]&GUI_PRESSED)!=0 && fieldtype==FIELDEDIT)
                e->hit(int(floor(hitx-(curx+wpad/2))), int(floor(hity-(cury+hpad/2))), (mouseaction[0]&GUI_DRAGGED)!=0); //mouse request position
            if(editing && (fieldmode==FIELDCOMMIT || fieldmode==FIELDABORT)) // commit field if user pressed enter
            {
                if(fieldmode==FIELDCOMMIT) result = e->currentline().text;
                e->active = (e->mode!=EDITORFOCUSED);
                fieldmode = FIELDSHOW;
            }
            else fieldsactive = true;

            e->draw(curx+wpad/2, cury+hpad/2, color, editing);

            lineshader->set();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            if(editing) glColor3f(0.75f, 0.25f, 0.25f);
            else glColor3ub(color>>16, (color>>8)&0xFF, color&0xFF);
            rect_(curx, cury, w, h, true);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            defaultshader->set();
        }
        else if(e->unfocus) e->unfocus = false;
        layout(w, h);
        if(e->maxy != 1)
        {
            int slines = e->limitscrolly();
            if(slines > 0)
            {
                int oldpos = e->scrolly == editor::SCROLLEND ? slines : e->scrolly, newpos = oldpos;
                slider(newpos, 0, slines, color, NULL, true, true);
                if(oldpos != newpos)
                {
                    e->cy = newpos;
                    e->scrolly = e->mode == EDITORREADONLY && newpos >= slines ? editor::SCROLLEND : newpos;
                }
            }
            if(wasvertical) poplist();
        }
        if(font && *font) gui::popfont();
        return result;
    }

    void rect_(float x, float y, float w, float h, bool lines = false)
    {
        glBegin(lines ? GL_LINE_LOOP : GL_TRIANGLE_STRIP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        if(lines) glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        if(!lines) glVertex2f(x + w, y + h);
        glEnd();
        xtraverts += 4;
    }

    void rect_(float x, float y, float w, float h, int usetc)
    {
        glBegin(GL_TRIANGLE_STRIP);
        static const GLfloat tc[5][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}};
        glTexCoord2fv(tc[usetc]); glVertex2f(x, y);
        glTexCoord2fv(tc[usetc+1]); glVertex2f(x + w, y);
        glTexCoord2fv(tc[usetc+3]); glVertex2f(x, y + h);
        glTexCoord2fv(tc[usetc+2]); glVertex2f(x + w, y + h);
        glEnd();
        xtraverts += 4;
    }

    void text_(const char *text, int x, int y, int color, int alpha, bool shadow, bool force = false)
    {
        if(FONTH != guibound[1]) y += (guibound[1]-FONTH)/2;
        if(shadow) draw_text(text, x+guishadow, y+guishadow, 0x00, 0x00, 0x00, -0xC0*alpha/255);
        draw_text(text, x, y, color>>16, (color>>8)&0xFF, color&0xFF, force ? -alpha : alpha);
    }

    void background(int color, int inheritw, int inherith)
    {
        if(!visible()) return;
        glDisable(GL_TEXTURE_2D);
        notextureshader->set();
        glColor4ub(color>>16, (color>>8)&0xFF, color&0xFF, 0x80);
        int w = xsize, h = ysize;
        if(inheritw>0)
        {
            int parentw = curlist, parentdepth = 0;
            for(;parentdepth < inheritw && lists[parentw].parent>=0; parentdepth++)
                parentw = lists[parentw].parent;
            list &p = lists[parentw];
            w = p.springs > 0 && (curdepth-parentdepth)&1 ? lists[p.parent].w : p.w;
        }
        if(inherith>0)
        {
            int parenth = curlist, parentdepth = 0;
            for(;parentdepth < inherith && lists[parenth].parent>=0; parentdepth++)
                parenth = lists[parenth].parent;
            list &p = lists[parenth];
            h = p.springs > 0 && !((curdepth-parentdepth)&1) ? lists[p.parent].h : p.h;
        }
        rect_(curx, cury, w, h, false);
        glEnable(GL_TEXTURE_2D);
        defaultshader->set();
    }

    void border(int color, int inheritw, int inherith, int offsetx, int offsety)
    {
        if(!visible()) return;
        glDisable(GL_TEXTURE_2D);
        lineshader->set();
        glColor4ub(color>>16, (color>>8)&0xFF, color&0xFF, 0x80);
        int w = xsize, h = ysize;
        if(inheritw>0)
        {
            int parentw = curlist, parentdepth = 0;
            for(;parentdepth < inheritw && lists[parentw].parent>=0; parentdepth++)
                parentw = lists[parentw].parent;
            list &p = lists[parentw];
            w = p.springs > 0 && (curdepth-parentdepth)&1 ? lists[p.parent].w : p.w;
        }
        if(inherith>0)
        {
            int parenth = curlist, parentdepth = 0;
            for(;parentdepth < inherith && lists[parenth].parent>=0; parentdepth++)
                parenth = lists[parenth].parent;
            list &p = lists[parenth];
            h = p.springs > 0 && !((curdepth-parentdepth)&1) ? lists[p.parent].h : p.h;
        }
        rect_(curx+offsetx, cury+offsety, w-offsetx*2, h-offsety*2, true);
        glEnable(GL_TEXTURE_2D);
        defaultshader->set();
    }

    void icon_(Texture *t, bool overlaid, int x, int y, int size, bool hit, int icolor)
    {
        static const float tc[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        float xs = 0, ys = 0;
        int textureid = -1;
        if(t)
        {
            float scale = float(size)/max(t->xs, t->ys); //scale and preserve aspect ratio
            xs = t->xs*scale;
            ys = t->ys*scale;
            x += int((size-xs)/2);
            y += int((size-ys)/2);
            textureid = t->id;
        }
        else
        {
            if(lightmapping && lmprogtex)
            {
                float scale = float(size)/256; //scale and preserve aspect ratio
                xs = 256*scale; ys = 256*scale;
                x += int((size-xs)/2);
                y += int((size-ys)/2);
                textureid = lmprogtex;
            }
            else
            {
                defformatstring(texname)("%s", mapname);
                if((t = textureload(texname, 3, true, false)) == notexture) t = textureload(emblemtex, 3, true, false);
                float scale = float(size)/max(t->xs, t->ys); //scale and preserve aspect ratio
                xs = t->xs*scale; ys = t->ys*scale;
                x += int((size-xs)/2);
                y += int((size-ys)/2);
                textureid = t->id;
            }
        }
        float xi = x, yi = y, xpad = 0, ypad = 0;
        if(overlaid)
        {
            xpad = xs/32;
            ypad = ys/32;
            xi += xpad;
            yi += ypad;
            xs -= 2*xpad;
            ys -= 2*ypad;
        }
        if(hit && hitfx)
        {
            float offx = xs*guihoverscale, offy = ys*guihoverscale;
            if(!hovertex) hovertex = textureload(guihovertex, 3, true, false);
            glBindTexture(GL_TEXTURE_2D, hovertex->id);
            glColor3f(1.f, 1.f, 1.f);
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2fv(tc[0]); glVertex2f(xi-offx,    yi-offy);
            glTexCoord2fv(tc[1]); glVertex2f(xi+xs+offx, yi-offy);
            glTexCoord2fv(tc[3]); glVertex2f(xi-offx,    yi+ys+offy);
            glTexCoord2fv(tc[2]); glVertex2f(xi+xs+offx, yi+ys+offy);
            glEnd();
        }
        glBindTexture(GL_TEXTURE_2D, textureid);
        vec color = vec::hexcolor(icolor);
        //if(hit && hitfx && !overlaid) color.div(2);
        glColor3fv(color.v);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2fv(tc[0]); glVertex2f(xi,    yi);
        glTexCoord2fv(tc[1]); glVertex2f(xi+xs, yi);
        glTexCoord2fv(tc[3]); glVertex2f(xi,    yi+ys);
        glTexCoord2fv(tc[2]); glVertex2f(xi+xs, yi+ys);
        glEnd();
        if(overlaid)
        {
            if(!overlaytex) overlaytex = textureload(guioverlaytex, 3, true, false);
            const vec &ocolor = hit && hitfx ? vec(1, 0.25f, 0.25f) : vec(1, 1, 1);
            glColor3fv(ocolor.v);
            glBindTexture(GL_TEXTURE_2D, overlaytex->id);
            rect_(xi - xpad, yi - ypad, xs + 2*xpad, ys + 2*ypad, 0);
        }
    }

    void previewslot(VSlot &vslot, bool overlaid, int x, int y, int size, bool hit)
    {
        Slot &slot = *vslot.slot;
        if(slot.sts.empty()) return;
        VSlot *layer = NULL;
        Texture *t = NULL, *glowtex = NULL, *layertex = NULL;
        if(slot.loaded)
        {
            t = slot.sts[0].t;
            if(t == notexture) return;
            Slot &slot = *vslot.slot;
            if(slot.texmask&(1<<TEX_GLOW)) { loopvj(slot.sts) if(slot.sts[j].type==TEX_GLOW) { glowtex = slot.sts[j].t; break; } }
            if(vslot.layer)
            {
                layer = &lookupvslot(vslot.layer);
                if(!layer->slot->sts.empty()) layertex = layer->slot->sts[0].t;
            }
        }
        else if(slot.thumbnail && slot.thumbnail != notexture) t = slot.thumbnail;
        else return;
        float xt = min(1.0f, t->xs/(float)t->ys), yt = min(1.0f, t->ys/(float)t->xs), xs = size, ys = size;
        float xi = x, yi = y, xpad = 0, ypad = 0;
        if(overlaid)
        {
            xpad = xs/32;
            ypad = ys/32;
            xi += xpad;
            yi += ypad;
            xs -= 2*xpad;
            ys -= 2*ypad;
        }
        SETSHADER(rgbonly);
        const vec &color = hit && hitfx && !overlaid ? vec(1.25f, 1.25f, 1.25f) : vec(1, 1, 1);
        float tc[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        int xoff = vslot.xoffset, yoff = vslot.yoffset;
        if(vslot.rotation)
        {
            if((vslot.rotation&5) == 1) { swap(xoff, yoff); loopk(4) swap(tc[k][0], tc[k][1]); }
            if(vslot.rotation >= 2 && vslot.rotation <= 4) { xoff *= -1; loopk(4) tc[k][0] *= -1; }
            if(vslot.rotation <= 2 || vslot.rotation == 5) { yoff *= -1; loopk(4) tc[k][1] *= -1; }
        }
        loopk(4) { tc[k][0] = tc[k][0]/xt - float(xoff)/t->xs; tc[k][1] = tc[k][1]/yt - float(yoff)/t->ys; }
        if(slot.loaded)
        {
            vec colorscale = vslot.getcolorscale();
            glColor3f(color.x*colorscale.x, color.y*colorscale.y, color.z*colorscale.z);
        }
        else glColor3fv(color.v);
        glBindTexture(GL_TEXTURE_2D, t->id);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2fv(tc[0]); glVertex2f(xi,    yi);
        glTexCoord2fv(tc[1]); glVertex2f(xi+xs, yi);
        glTexCoord2fv(tc[3]); glVertex2f(xi,    yi+ys);
        glTexCoord2fv(tc[2]); glVertex2f(xi+xs, yi+ys);
        glEnd();
        if(glowtex)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBindTexture(GL_TEXTURE_2D, glowtex->id);
            vec glowcolor = vslot.getglowcolor();
            if(hit || overlaid) glColor3f(color.x*glowcolor.x, color.y*glowcolor.y, color.z*glowcolor.z);
            else glColor3fv(glowcolor.v);
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2fv(tc[0]); glVertex2f(x,    y);
            glTexCoord2fv(tc[1]); glVertex2f(x+xs, y);
            glTexCoord2fv(tc[3]); glVertex2f(x,    y+ys);
            glTexCoord2fv(tc[2]); glVertex2f(x+xs, y+ys);
            glEnd();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if(layertex)
        {
            vec layerscale = layer->getcolorscale();
            glBindTexture(GL_TEXTURE_2D, layertex->id);
            glColor3f(color.x*layerscale.x, color.y*layerscale.y, color.z*layerscale.z);
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2fv(tc[0]); glVertex2f(x+xs/2, y+ys/2);
            glTexCoord2fv(tc[1]); glVertex2f(x+xs,   y+ys/2);
            glTexCoord2fv(tc[3]); glVertex2f(x+xs/2, y+ys);
            glTexCoord2fv(tc[2]); glVertex2f(x+xs,   y+ys);
            glEnd();
        }

        defaultshader->set();
        if(overlaid)
        {
            if(!overlaytex) overlaytex = textureload(guioverlaytex, 3, true, false);
            const vec &ocolor = hit && hitfx ? vec(1.f, 0.25f, 0.25f) : vec(1, 1, 1);
            glColor3fv(ocolor.v);
            glBindTexture(GL_TEXTURE_2D, overlaytex->id);
            rect_(xi - xpad, yi - ypad, xs + 2*xpad, ys + 2*ypad, 0);
        }
    }

    void slice_(Texture *t, int x, int y, int size, float start = 0, float end = 1, const char *text = NULL)
    {
        float scale = float(size)/max(t->xs, t->ys), xs = t->xs*scale, ys = t->ys*scale, fade = 1;
        if(start == end) { end = 1; fade = 0.5f; }
        glBindTexture(GL_TEXTURE_2D, t->id);
        glColor4f(1, 1, 1, fade);
        int s = max(xs,ys)/2;
        drawslice(start, end, x+s/2, y+s/2, s);
        if(text && *text)
        {
            int w = text_width(text);
            text_(text, x+s/2-w/2, y+s/2-FONTH/2, 0xFFFFFF, guitextblend, false);
        }
    }

    int line_(int size, float percent = 1.0f, int space = 0)
    {
        space = max(max(space, guibound[0]), size);
        if(visible())
        {
            if(!slidertex) slidertex = textureload(guislidertex, 3, true, false);
            glBindTexture(GL_TEXTURE_2D, slidertex->id);
            if(percent < 0.99f)
            {
                glColor4f(0.5f, 0.5f, 0.5f, 0.375f);
                if(ishorizontal())
                    rect_(curx + space/2 - size/2, cury, size, ysize, 0);
                else
                    rect_(curx, cury + space/2 - size/2, xsize, size, 1);
            }
            glColor3f(0.5f, 0.5f, 0.5f);
            if(ishorizontal())
                rect_(curx + space/2 - size/2, cury + ysize*(1-percent), size, ysize*percent, 0);
            else
                rect_(curx, cury + space/2 - size/2, xsize*percent, size, 1);
        }
        layout(ishorizontal() ? space : 0, ishorizontal() ? 0 : space);
        return space;
    }

    int button_(const char *text, int color, const char *icon, int icolor, bool clickable, bool faded, const char *font = "")
    {
        if(font && *font) gui::pushfont(font);
        int w = 0, h = max((int)FONTH, guibound[1]);
        if(icon) w += guibound[1];
        if(icon && text) w += 8;
        if(text) w += text_width(text);

        if(visible())
        {
            bool hit = ishit(w, FONTH), forcecolor = false;
            if(hit && hitfx && clickable) { forcecolor = true; color = 0xFF4444; }
            int x = curx;
            if(icon)
            {
                const char *tname = strstr(icon, "textures/") ? icon : makerelpath("textures", icon);
                icon_(textureload(tname, 3, true, false), false, x, cury, guibound[1], clickable && hit, icolor);
                x += guibound[1];
            }
            if(icon && text) x += 8;
            if(text) text_(text, x, cury, color, (hit && hitfx) || !faded || !clickable ? guitextblend : guitextfade, hit && clickable, forcecolor);
        }
        if(font && *font) gui::popfont();
        return layout(w, h);
    }

    static Texture *overlaytex, *slidertex, *exittex, *hovertex;

    vec uiorigin, uiscale;
    guicb *cb;

    static float basescale, maxscale;
    static bool passthrough;

    void adjustscale()
    {
        int w = xsize + guibound[0]*8, h = ysize + guibound[1]*6;
        float aspect = forceaspect ? 1.0f/forceaspect : float(screen->h)/float(screen->w), fit = 1.0f;
        if(w*aspect*basescale>1.0f) fit = 1.0f/(w*aspect*basescale);
        if(h*basescale*fit>maxscale) fit *= maxscale/(h*basescale*fit);
        uiscale = vec(aspect*uiscale.x*fit, uiscale.y*fit, 1);
        uiorigin = vec(0.5f - ((w-xsize)/2 - (guibound[0]*4))*uiscale.x, 0.5f + (0.5f*h-(guibound[1]*2))*uiscale.y, 0);
        //uiorigin = vec(0.5f - (guibound[0]*2)*uiscale.x, 0.5f + (h-guibound[1]*2)*uiscale.y, 0);
    }

    void start(int starttime, float initscale, int *tab, bool allowinput, bool wantstitle, bool wantsbgfx)
    {
        initscale *= 0.025f;
        basescale = initscale;
        if(guilayoutpass)
            uiscale.x = uiscale.y = uiscale.z = guiscaletime ? min(basescale*(totalmillis-starttime)/float(guiscaletime), basescale) : basescale;
        needsinput = allowinput;
        hastitle = wantstitle;
        hasbgfx = wantsbgfx;
        passthrough = !allowinput;
        fontdepth = 0;
        gui::pushfont("reduced");
        curdepth = curlist = mergedepth = mergelist = -1;
        tpos = tx = ty = 0;
        tcurrent = tab;
        tcolor = 0xFFFFFF;
        pushlist(false);
        if(guilayoutpass) nextlist = curlist;
        else
        {
            if(tcurrent && !*tcurrent) tcurrent = NULL;
            cury = -ysize;
            curx = -xsize/2;

            glPushMatrix();
            glTranslatef(uiorigin.x, uiorigin.y, uiorigin.z);
            glScalef(uiscale.x, uiscale.y, uiscale.z);
            int x = curx-guibound[0]*1, y = cury-guibound[1]/2, w = xsize+guibound[0]*2, h = ysize+guibound[1];
            #if 0
            if(hastitle)
            {
                y -= guibound[1]*3/2;
                h += guibound[1]*3/2;
            }
            #endif
            if(hasbgfx && guibgcolour >= 0)
            {
                notextureshader->set();
                glDisable(GL_TEXTURE_2D);
                glColor4f((guibgcolour>>16)/255.f, ((guibgcolour>>8)&0xFF)/255.f, (guibgcolour&0xFF)/255.f, guibgblend);
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2f(x, y);
                glVertex2f(x+w, y);
                glVertex2f(x, y+h);
                glVertex2f(x+w, y+h);
                glEnd();
                defaultshader->set();
                glEnable(GL_TEXTURE_2D);
            }
            if(hasbgfx && guibordercolour >= 0)
            {
                lineshader->set();
                glDisable(GL_TEXTURE_2D);
                glColor4f((guibordercolour>>16)/255.f, ((guibordercolour>>8)&0xFF)/255.f, (guibordercolour&0xFF)/255.f, guiborderblend);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x+w, y);
                glVertex2f(x+w, y+h);
                glVertex2f(x, y+h);
                glEnd();
                defaultshader->set();
                glEnable(GL_TEXTURE_2D);
            }
        }
    }

    void end()
    {
        if(guilayoutpass)
        {
            xsize = max(tx, xsize);
            ysize = max(ty, ysize);
            ysize = max(ysize, guibound[1]);

            if(tcurrent) *tcurrent = max(1, min(*tcurrent, tpos));
            adjustscale();

            if(!passthrough)
            {
                hitx = (cursorx - uiorigin.x)/uiscale.x;
                hity = (cursory - uiorigin.y)/uiscale.y;
                if((mouseaction[0]&GUI_PRESSED) && (fabs(hitx-firstx) > 2 || fabs(hity - firsty) > 2)) mouseaction[0] |= GUI_DRAGGED;
            }
        }
        else
        {
            if(*guistatustext)
            {
                gui::pushfont("little");
                int w = text_width(guistatustext)+guibound[0]*2, h = guibound[1]/2+FONTH, x = -w/2, y = guibound[1];
                if(hasbgfx && guibgcolour >= 0)
                {
                    notextureshader->set();
                    glDisable(GL_TEXTURE_2D);
                    glColor4f((guibgcolour>>16)/255.f, ((guibgcolour>>8)&0xFF)/255.f, (guibgcolour&0xFF)/255.f, guibgblend);
                    glBegin(GL_TRIANGLE_STRIP);
                    glVertex2f(x, y);
                    glVertex2f(x+w, y);
                    glVertex2f(x, y+h);
                    glVertex2f(x+w, y+h);
                    glEnd();
                    defaultshader->set();
                    glEnable(GL_TEXTURE_2D);
                }
                if(hasbgfx && guibordercolour >= 0)
                {
                    lineshader->set();
                    glDisable(GL_TEXTURE_2D);
                    glColor4f((guibordercolour>>16)/255.f, ((guibordercolour>>8)&0xFF)/255.f, (guibordercolour&0xFF)/255.f, guiborderblend);
                    glBegin(GL_LINE_LOOP);
                    glVertex2f(x, y);
                    glVertex2f(x+w, y);
                    glVertex2f(x+w, y+h);
                    glVertex2f(x, y+h);
                    glEnd();
                    defaultshader->set();
                    glEnable(GL_TEXTURE_2D);
                }
                x += guibound[0];
                y += guibound[1]/4;
                draw_text(guistatustext, x, y);
                gui::popfont();
            }
            if(needsinput && hastitle) uibuttons();
            if(*guitooltiptext)
            {
                if(!tooltip || !lasttooltip || strcmp(tooltip, guitooltiptext))
                {
                    if(tooltip) DELETEA(tooltip);
                    tooltip = newstring(guitooltiptext);
                    lasttooltip = totalmillis;
                }
                if(totalmillis-lasttooltip >= guitooltiptime)
                {
                    gui::pushfont("little");
                    int w = text_width(guitooltiptext)+guibound[0]*2, h = guibound[1]/2+FONTH, x = hitx, y = hity-guibound[1]-FONTH/2;
                    if(hasbgfx && guitooltipcolour >= 0)
                    {
                        notextureshader->set();
                        glDisable(GL_TEXTURE_2D);
                        glColor4f((guitooltipcolour>>16)/255.f, ((guitooltipcolour>>8)&0xFF)/255.f, (guitooltipcolour&0xFF)/255.f, guitooltipblend);
                        glBegin(GL_TRIANGLE_STRIP);
                        glVertex2f(x, y);
                        glVertex2f(x+w, y);
                        glVertex2f(x, y+h);
                        glVertex2f(x+w, y+h);
                        glEnd();
                        defaultshader->set();
                        glEnable(GL_TEXTURE_2D);
                    }
                    if(hasbgfx && guitooltipbordercolour >= 0)
                    {
                        lineshader->set();
                        glDisable(GL_TEXTURE_2D);
                        glColor4f((guitooltipbordercolour>>16)/255.f, ((guitooltipbordercolour>>8)&0xFF)/255.f, (guitooltipbordercolour&0xFF)/255.f, guitooltipborderblend);
                        glBegin(GL_LINE_LOOP);
                        glVertex2f(x, y);
                        glVertex2f(x+w, y);
                        glVertex2f(x+w, y+h);
                        glVertex2f(x, y+h);
                        glEnd();
                        defaultshader->set();
                        glEnable(GL_TEXTURE_2D);
                    }
                    x += guibound[0];
                    y += guibound[1]/4;
                    draw_text(tooltip, x, y);
                    gui::popfont();
                }
            }
            else if(tooltip)
            {
                DELETEA(tooltip);
                lasttooltip = 0;
            }
            glPopMatrix();
        }
        poplist();
        while(fontdepth) gui::popfont();
    }
};

Texture *gui::overlaytex = NULL, *gui::slidertex = NULL, *gui::exittex = NULL, *gui::hovertex = NULL;
TVARN(IDF_PERSIST|IDF_PRELOAD, guioverlaytex, "textures/guioverlay", gui::overlaytex, 0);
TVARN(IDF_PERSIST|IDF_PRELOAD, guislidertex, "textures/guislider", gui::slidertex, 0);
TVARN(IDF_PERSIST|IDF_PRELOAD, guiexittex, "textures/guiexit", gui::exittex, 0);
TVARN(IDF_PERSIST|IDF_PRELOAD, guihovertex, "textures/guihover", gui::hovertex, 0);

vector<gui::list> gui::lists;
float gui::basescale, gui::maxscale = 1, gui::hitx, gui::hity;
bool gui::passthrough, gui::hitfx = true;
int gui::curdepth, gui::fontdepth, gui::curlist, gui::xsize, gui::ysize, gui::curx, gui::cury, gui::mergelist, gui::mergedepth;
int gui::ty, gui::tx, gui::tpos, *gui::tcurrent, gui::tcolor;
static vector<gui> guis;

namespace UI
{
    bool isopen = false, ready = false;

    void setup()
    {
        pushfont("reduced");
        loopk(2) guibound[k] = (k ? FONTH : FONTW);
        popfont();
        ready = true;
    }

    bool keypress(int code, bool isdown, int cooked)
    {
        editor *e = currentfocus();
        if(fieldmode == FIELDKEY && e && e->mode != EDITORREADONLY)
        {
            switch(code)
            {
                case SDLK_ESCAPE:
                    if(isdown) fieldmode = FIELDCOMMIT;
                    return true;
            }
            const char *keyname = getkeyname(code);
            if(keyname && isdown)
            {
                if(e->lines.length()!=1 || !e->lines[0].empty()) e->insert(" ");
                e->insert(keyname);
            }
            return true;
        }

        if(code<0) switch(code)
        { // fall-through-o-rama
            case -5: mouseaction[1] |= GUI_ALT;
            case -4: mouseaction[1] |= isdown ? GUI_DOWN : GUI_UP;
                if(active()) return true;
                break;
            case -3: mouseaction[0] |= GUI_ALT;
            case -1: mouseaction[0] |= (guiactionon=isdown) ? GUI_DOWN : GUI_UP;
                if(isdown) { firstx = gui::hitx; firsty = gui::hity; }
                if(active()) return true;
                break;
            case -2:
                if(isdown) cleargui(1);
                if(active()) return true;
            default: break;
        }

        if(fieldmode == FIELDSHOW || !e || e->mode == EDITORREADONLY) return false;
        switch(code)
        {
            case SDLK_ESCAPE: //cancel editing without commit
                if(isdown)
                {
                    fieldmode = FIELDABORT;
                    e->unfocus = true;
                }
                return true;
            case SDLK_RETURN:
            case SDLK_TAB:
                if(cooked && (e->maxy != 1)) break;
            case SDLK_KP_ENTER:
                if(isdown) fieldmode = FIELDCOMMIT; //signal field commit (handled when drawing field)
                return true;
            case SDLK_HOME:
            case SDLK_END:
            case SDLK_DELETE:
            case SDLK_BACKSPACE:
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                break;
            case SDLK_PAGEUP:
            case SDLK_PAGEDOWN:
            case -4:
            case -5:
                if(e->parent && *e->parent)
                { // pass input on to parent
                    editor *f = findeditor(e->parent);
                    if(f) e = f;
                }
                break;
            default:
                if(!cooked || (code<32)) return false;
                break;
        }
        if(!isdown) return true;
        e->key(code, cooked);
        return true;
    }

    bool active(bool pass) { return guis.length() && (!pass || needsinput); }
    void limitscale(float scale) {  gui::maxscale = scale; }

    void addcb(guicb *cb)
    {
        gui &g = guis.add();
        g.cb = cb;
        g.adjustscale();
    }

    void update()
    {
        bool p = active(false);
        if(isopen != p) uimillis = (isopen = p) ? totalmillis : -totalmillis;
        setsvar("guistatustext", "", true);
        setsvar("guitooltiptext", "", true);
        setsvar("guirollovername", "", true);
        setsvar("guirolloveraction", "", true);
        setsvar("guirolloverimgpath", "", true);
        setsvar("guirolloverimgaction", "", true);
    }

    void render()
    {
        if(guiactionon) mouseaction[0] |= GUI_PRESSED;

        gui::reset();
        guis.shrink(0);

        // call all places in the engine that may want to render a gui from here, they call addcb()
        if(progressing) progressmenu();
        else
        {
            texturemenu();
            hud::gamemenus();
            mainmenu();
        }

        readyeditors();
        bool wasfocused = (fieldmode!=FIELDSHOW);
        fieldsactive = false;

        needsinput = false;
        hastitle = hasbgfx = true;

        if(!guis.empty())
        {
            guilayoutpass = 1;
            //loopv(guis) guis[i].cb->gui(guis[i], true);
            guis.last().cb->gui(guis.last(), true);
            guilayoutpass = 0;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, 1, 1, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            //loopvrev(guis) guis[i].cb->gui(guis[i], false);
            guis.last().cb->gui(guis.last(), false);

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            glDisable(GL_BLEND);
        }

        flusheditors();
        if(!fieldsactive) fieldmode = FIELDSHOW; //didn't draw any fields, so lose focus - mainly for menu closed
        if((fieldmode!=FIELDSHOW) != wasfocused)
        {
            SDL_EnableUNICODE(fieldmode!=FIELDSHOW);
            keyrepeat(fieldmode!=FIELDSHOW);
        }
        loopi(2) mouseaction[i] = 0;
    }

    editor *geteditor(const char *name, int mode, const char *init, const char *parent)
    {
        return useeditor(name, mode, false, init, parent);
    }

    void editorline(editor *e, const char *str, int limit)
    {
        if(!e) return;
        if(e->lines.length() != 1 || !e->lines[0].empty()) e->lines.add();
        e->lines.last().set(str);
        if(limit >= 0 && e->lines.length() > limit)
        {
            int n = e->lines.length()-limit;
            e->removelines(0, n);
            e->cy = max(e->cy - n, 0);
            if(e->scrolly != editor::SCROLLEND) e->scrolly = max(e->scrolly - n, 0);
        }
        e->mark(false);
    }

    void editorclear(editor *e, const char *init)
    {
        if(!e) return;
        e->clear(init);
    }

    void editoredit(editor *e)
    {
        if(!e) return;
        useeditor(e->name, e->mode, true);
        e->clear();
        fieldmode = FIELDEDIT;
    }
};

