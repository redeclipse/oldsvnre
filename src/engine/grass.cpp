#include "engine.h"

VAR(IDF_PERSIST, grass, 0, 0, 1);
FVAR(IDF_PERSIST, grassstep, 0.5, 2, 8);
FVAR(IDF_PERSIST, grasstaper, 0, 0.1, 1);

VAR(0, dbggrass, 0, 0, 1);
VAR(IDF_PERSIST, grassdist, 0, 256, 10000);
VAR(IDF_WORLD, grassheight, 1, 4, 64);

struct grasswedge
{
    vec dir, edge1, edge2;
    plane bound1, bound2;

    void init(int i, int n)
    {
        dir = vec(2*M_PI*(i+0.5f)/float(n), 0);
        edge1 = vec(2*M_PI*i/float(n), 0).div(cos(M_PI/n));
        edge2 = vec(2*M_PI*(i+1)/float(n), 0).div(cos(M_PI/n));
        bound1 = plane(vec(2*M_PI*(i/float(n) - 0.5f), 0), 0);
        bound2 = plane(vec(2*M_PI*((i+1)/float(n) + 0.5f), 0), 0);
    }
};
grasswedge *grasswedges = NULL;
void resetgrasswedges(int n)
{
    DELETEA(grasswedges);
    grasswedges = new grasswedge[n];
    loopi(n) grasswedges[i].init(i, n);
}
VARFN(IDF_PERSIST, grasswedges, numgrasswedges, 8, 8, 1024, resetgrasswedges(numgrasswedges));

struct grassvert
{
    vec pos;
    uchar color[4];
    float u, v, lmu, lmv;
};

static vector<grassvert> grassverts;

struct grassgroup
{
    const grasstri *tri;
    float dist;
    int tex, lmtex, offset, numquads, scale, height;
};

static vector<grassgroup> grassgroups;

float *grassoffsets = NULL, *grassanimoffsets = NULL;
void resetgrassoffsets(int n)
{
    DELETEA(grassoffsets);
    DELETEA(grassanimoffsets);
    grassoffsets = new float[n];
    grassanimoffsets = new float[n];
    loopi(n) grassoffsets[i] = rnd(0x1000000)/float(0x1000000);
}
VARFN(IDF_PERSIST, grassoffsets, numgrassoffsets, 8, 32, 1024, resetgrassoffsets(numgrassoffsets));

void initgrass()
{
    if(!grasswedges) resetgrasswedges(numgrasswedges);
    if(!grassoffsets) resetgrassoffsets(numgrassoffsets);
}

static int lastgrassanim = -1;

VAR(IDF_WORLD, grassanimmillis, 0, 3000, 60000);
FVAR(IDF_WORLD, grassanimscale, 0, 0.03f, 1);

static void animategrass()
{
    loopi(numgrassoffsets) grassanimoffsets[i] = grassanimscale*sinf(2*M_PI*(grassoffsets[i] + lastmillis/float(grassanimmillis)));
    lastgrassanim = lastmillis;
}

static inline bool clipgrassquad(const grasstri &g, vec &p1, vec &p2)
{
#define CLIPEDGE(n) { \
        float dist1 = g.e[n].dist(p1), dist2 = g.e[n].dist(p2); \
        if(dist1 <= 0) \
        { \
            if(dist2 <= 0) return false; \
            p1.add(vec(p2).sub(p1).mul(dist1 / (dist1 - dist2))); \
        } \
        else if(dist2 <= 0) \
            p2.add(vec(p1).sub(p2).mul(dist2 / (dist2 - dist1))); \
    }
    CLIPEDGE(0)
    CLIPEDGE(1)
    CLIPEDGE(2)
    if(g.numv > 3) CLIPEDGE(3)
    return true;
}

VAR(IDF_WORLD, grassscale, 1, 2, 64);
bvec grasscolor(255, 255, 255);
VARF(IDF_HEX|IDF_WORLD, grasscolour, 0, 0xFFFFFF, 0xFFFFFF,
{
    if(!grasscolour) grasscolour = 0xFFFFFF;
    grasscolor = bvec((grasscolour>>16)&0xFF, (grasscolour>>8)&0xFF, grasscolour&0xFF);
});
FVAR(IDF_WORLD, grassblend, 0, 1, 1);

static void gengrassquads(grassgroup *&group, const grasswedge &w, const grasstri &g, Texture *tex, const vec &col, float blend, int scale, int height)
{
    float t = camera1->o.dot(w.dir);
    int tstep = int(ceil(t/grassstep));
    float tstart = tstep*grassstep, tfrac = tstart - t;

    float t1 = w.dir.dot(g.v[0]), t2 = w.dir.dot(g.v[1]), t3 = w.dir.dot(g.v[2]),
          tmin = min(t1, min(t2, t3)),
          tmax = max(t1, max(t2, t3));
    if(g.numv>3)
    {
        float t4 = w.dir.dot(g.v[3]);
        tmin = min(tmin, t4);
        tmax = max(tmax, t4);
    }

    if(tmax < tstart || tmin > t + grassdist) return;

    int minstep = max(int(ceil(tmin/grassstep)) - tstep, 1),
        maxstep = int(floor(min(tmax, t + grassdist)/grassstep)) - tstep,
        numsteps = maxstep - minstep + 1,
        gs = scale > 0 ? scale : grassscale,
        gh = height > 0 ? height : grassheight;

    float texscale = (gs*tex->ys)/float(gh*tex->xs), animscale = gh*texscale;
    vec tc;
    tc.cross(g.surface, w.dir).mul(texscale);

    int color = tstep + maxstep;
    if(color < 0) color = numgrassoffsets - (-color)%numgrassoffsets;
    color += numsteps + numgrassoffsets - numsteps%numgrassoffsets;

    float taperdist = grassdist*grasstaper,
          taperscale = 1.0f / (grassdist - taperdist);
    bvec gcol = col.iszero() ? grasscolor : bvec(uchar(col.x*255), uchar(col.y*255), uchar(col.z*255));
    if(blend <= 0) blend = grassblend;
    vec e1(camera1->o.x, camera1->o.y, g.surface.zintersect(camera1->o)), e2 = e1,
        de1(w.edge1.x, w.edge1.y, g.surface.zdelta(w.edge1)),
        de2(w.edge2.x, w.edge2.y, g.surface.zdelta(w.edge2));
    float dist = maxstep*grassstep + tfrac;
    e1.add(vec(de1).mul(dist));
    e2.add(vec(de2).mul(dist));
    de1.mul(grassstep);
    de2.mul(grassstep);
    for(int i = maxstep; i >= minstep; i--, color--, e1.sub(de1), e2.sub(de2), dist -= grassstep)
    {
        vec p1 = e1, p2 = e2;
        if(!clipgrassquad(g, p1, p2)) continue;

        if(!group)
        {
            group = &grassgroups.add();
            group->tri = &g;
            group->tex = tex->id;
            group->lmtex = lightmaptexs.inrange(g.lmid) ? lightmaptexs[g.lmid].id : notexture->id;
            group->offset = grassverts.length();
            group->numquads = 0;
            group->scale = gs;
            group->height = gh;
            if(lastgrassanim!=lastmillis) animategrass();
        }

        group->numquads++;

        float offset = grassoffsets[color%numgrassoffsets],
              animoffset = animscale*grassanimoffsets[color%numgrassoffsets],
              tc1 = tc.dot(p1) + offset, tc2 = tc.dot(p2) + offset,
              lm1u = g.tcu.dot(p1), lm1v = g.tcv.dot(p1),
              lm2u = g.tcu.dot(p2), lm2v = g.tcv.dot(p2),
              fade = dist > taperdist ? (grassdist - dist)*taperscale : 1,
              height = gh * fade;
        uchar color[4] = { gcol.x, gcol.y, gcol.z, uchar(fade*blend*255) };

        #define GRASSVERT(n, tcv, modify) { \
            grassvert &gv = grassverts.add(); \
            gv.pos = p##n; \
            memcpy(gv.color, color, sizeof(color)); \
            gv.u = tc##n; gv.v = tcv; \
            gv.lmu = lm##n##u; gv.lmv = lm##n##v; \
            modify; \
        }

        GRASSVERT(2, 0, { gv.pos.z += height; gv.u += animoffset; });
        GRASSVERT(1, 0, { gv.pos.z += height; gv.u += animoffset; });
        GRASSVERT(1, 1, );
        GRASSVERT(2, 1, );
    }
}

static void gengrassquads(vtxarray *va)
{
    loopv(va->grasstris)
    {
        grasstri &g = va->grasstris[i];
        if(isfoggedsphere(g.radius, g.center)) continue;
        float dist = g.center.dist(camera1->o);
        if(dist - g.radius > grassdist) continue;

        Slot &s = *lookupvslot(g.texture, false).slot;
        if(!s.grasstex)
        {
            if(!s.texgrass) continue;
            s.grasstex = textureload(makerelpath(NULL, s.texgrass, NULL, "<ffskip><premul>"), 2);
        }

        grassgroup *group = NULL;
        loopi(numgrasswedges)
        {
            grasswedge &w = grasswedges[i];
            if(w.bound1.dist(g.center) > g.radius || w.bound2.dist(g.center) > g.radius) continue;
            gengrassquads(group, w, g, s.grasstex, s.grasscolor, s.grassblend, s.grassscale, s.grassheight);
        }
        if(group) group->dist = dist;
    }
}

static inline int comparegrassgroups(const grassgroup *x, const grassgroup *y)
{
    if(x->dist > y->dist) return -1;
    else if(x->dist < y->dist) return 1;
    else return 0;
}

void generategrass()
{
    if(!grass || !grassdist) return;

    initgrass();

    grassgroups.setsize(0);
    grassverts.setsize(0);

    loopi(numgrasswedges)
    {
        grasswedge &w = grasswedges[i];
        w.bound1.offset = -camera1->o.dot(w.bound1);
        w.bound2.offset = -camera1->o.dot(w.bound2);
    }

    extern vtxarray *visibleva;
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(va->grasstris.empty() || va->occluded >= OCCLUDE_GEOM) continue;
        if(va->distance > grassdist) continue;
        if(reflecting || refracting>0 ? va->o.z+va->size<reflectz : va->o.z>=reflectz) continue;
        gengrassquads(va);
    }

    grassgroups.sort(comparegrassgroups);
}

void rendergrass()
{
    if(!grass || !grassdist || grassgroups.empty() || dbggrass) return;

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(renderpath==R_FIXEDFUNCTION ? GL_SRC_ALPHA : GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    SETSHADER(grass);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(grassvert), grassverts[0].pos.v);

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(grassvert), grassverts[0].color);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(grassvert), &grassverts[0].u);

    if(renderpath!=R_FIXEDFUNCTION || maxtmus>=2)
    {
        glActiveTexture_(GL_TEXTURE1_ARB);
        glClientActiveTexture_(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(grassvert), &grassverts[0].lmu);
        if(renderpath==R_FIXEDFUNCTION) setuptmu(1, "P * T x 2");
        glClientActiveTexture_(GL_TEXTURE0_ARB);
        glActiveTexture_(GL_TEXTURE0_ARB);
    }

    int texid = -1, lmtexid = -1;
    loopv(grassgroups)
    {
        grassgroup &g = grassgroups[i];

        if(reflecting || refracting)
        {
            if(refracting < 0 ?
                min(g.tri->numv>3 ? min(g.tri->v[0].z, g.tri->v[3].z) : g.tri->v[0].z, min(g.tri->v[1].z, g.tri->v[2].z)) > reflectz :
                max(g.tri->numv>3 ? max(g.tri->v[0].z, g.tri->v[3].z) : g.tri->v[0].z, max(g.tri->v[1].z, g.tri->v[2].z)) + g.height < reflectz)
                continue;
            if(isfoggedsphere(g.tri->radius, g.tri->center)) continue;
        }

        if(texid != g.tex)
        {
            glBindTexture(GL_TEXTURE_2D, g.tex);
            texid = g.tex;
        }
        if(lmtexid != g.lmtex)
        {
            if(renderpath!=R_FIXEDFUNCTION || maxtmus>=2)
            {
                glActiveTexture_(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_2D, g.lmtex);
                glActiveTexture_(GL_TEXTURE0_ARB);
            }
            lmtexid = g.lmtex;
        }

        glDrawArrays(GL_QUADS, g.offset, 4*g.numquads);
        xtravertsva += 4*g.numquads;
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if(renderpath!=R_FIXEDFUNCTION || maxtmus>=2)
    {
        glActiveTexture_(GL_TEXTURE1_ARB);
        glClientActiveTexture_(GL_TEXTURE1_ARB);
        if(renderpath==R_FIXEDFUNCTION) resettmu(1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
        glClientActiveTexture_(GL_TEXTURE0_ARB);
        glActiveTexture_(GL_TEXTURE0_ARB);
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

