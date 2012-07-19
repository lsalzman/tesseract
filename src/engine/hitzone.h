struct skelbihnode
{
    short split[2];
    ushort child[2];

    int axis() const { return child[0]>>14; }
    int childindex(int which) const { return child[which]&0x3FFF; }
    bool isleaf(int which) const { return (child[1]&(1<<(14+which)))!=0; }
};

struct skelbih
{
    struct tri : skelmodel::tri
    {
        uchar mesh, id;
        ushort vert[3];
    };

    skelbihnode *nodes;
    tri *tris;

    vec bbmin, bbmax;

    skelbih(skelmodel::skelmeshgroup *m, int numtris, tri *tris);

    ~skelbih()
    {
        DELETEA(nodes);
    }

    static bool triintersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const tri &t, const vec &o, const vec &ray);

    void build(skelmodel::skelmeshgroup *m, vector<skelbihnode> &buildnodes, ushort *indices, int numindices, int depth = 1);

    void intersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const vec &o, const vec &ray);

    vec calccenter() const
    {
        return vec(bbmin).add(bbmax).mul(0.5f);
    }

    float calcradius() const
    {
        return vec(bbmax).sub(bbmin).mul(0.5f).magnitude();
    }
};

bool skelbih::triintersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const tri &t, const vec &o, const vec &ray)
{
    skelmodel::skelmesh *tm = (skelmodel::skelmesh *)m->meshes[t.mesh];
    const skelmodel::vert &va = tm->verts[t.vert[0]], &vb = tm->verts[t.vert[1]], &vc = tm->verts[t.vert[2]];
    vec eb = vb.pos, ec = vc.pos;
    eb.sub(va.pos);
    ec.sub(va.pos);
    vec p;
    p.cross(ray, ec);
    float det = eb.dot(p);
    if(det == 0) return false;
    vec r(o); 
    r.sub(va.pos);
    float u = r.dot(p) / det; 
    if(u < 0 || u > 1) return false;
    vec q; 
    q.cross(r, eb);
    float v = ray.dot(q) / det;
    if(v < 0 || u + v > 1) return false;
    float f = ec.dot(q) / det;
    if(f < 0 || f*skelmodel::intersectscale > skelmodel::intersectdist) return false;
    if(!(skelmodel::intersectmode&RAY_SHADOW) && tm->noclip) return false;
    Texture *tex = s[t.mesh].tex;
    if(tex->bpp==4 && (skelmodel::intersectmode&RAY_ALPHAPOLY)==RAY_ALPHAPOLY && (tex->alphamask || (loadalphamask(tex), tex->alphamask)))
    {
        int si = clamp(int(tex->xs * (va.u + u*(vb.u - va.u) + v*(vc.u - va.u))), 0, tex->xs-1),
            ti = clamp(int(tex->ys * (va.v + u*(vb.v - va.v) + v*(vc.v - va.v))), 0, tex->ys-1);
        if(!(tex->alphamask[ti*((tex->xs+7)/8) + si/8] & (1<<(si%8)))) return false;
    }
    skelmodel::intersectdist = f*skelmodel::intersectscale;
    skelmodel::intersectresult = t.id&0x80 ? -1 : t.id;
    return true;
}

struct skelbihstack
{
    skelbihnode *node;
    float tmin, tmax;
};

void skelbih::intersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const vec &o, const vec &ray)
{
    vec invray(ray.x ? 1/ray.x : 1e16f, ray.y ? 1/ray.y : 1e16f, ray.z ? 1/ray.z : 1e16f);
    float tmin, tmax;
    float t1 = (bbmin.x - o.x)*invray.x, 
          t2 = (bbmax.x - o.x)*invray.x;
    if(invray.x > 0) { tmin = t1; tmax = t2; } else { tmin = t2; tmax = t1; }
    t1 = (bbmin.y - o.y)*invray.y;
    t2 = (bbmax.y - o.y)*invray.y;
    if(invray.y > 0) { tmin = max(tmin, t1); tmax = min(tmax, t2); } else { tmin = max(tmin, t2); tmax = min(tmax, t1); }
    t1 = (bbmin.z - o.z)*invray.z;
    t2 = (bbmax.z - o.z)*invray.z;
    if(invray.z > 0) { tmin = max(tmin, t1); tmax = min(tmax, t2); } else { tmin = max(tmin, t2); tmax = min(tmax, t1); }
    tmax = min(tmax, skelmodel::intersectdist/skelmodel::intersectscale);
    if(tmin >= tmax) return;

    if(!nodes)
    {
        triintersect(m, s, tris[0], o, ray);
        return;
    }

    static vector<skelbihstack> stack;
    stack.setsize(0);

    ivec order(ray.x>0 ? 0 : 1, ray.y>0 ? 0 : 1, ray.z>0 ? 0 : 1);
    skelbihnode *curnode = &nodes[0];
    for(;;)
    {
        int axis = curnode->axis();
        int nearidx = order[axis], faridx = nearidx^1;
        float nearsplit = (curnode->split[nearidx] - o[axis])*invray[axis],
              farsplit = (curnode->split[faridx] - o[axis])*invray[axis];

        if(nearsplit <= tmin)
        {
            if(farsplit < tmax)
            {
                if(!curnode->isleaf(faridx))
                {
                    curnode = &nodes[curnode->childindex(faridx)];
                    tmin = max(tmin, farsplit);
                    continue;
                }
                else if(triintersect(m, s, tris[curnode->childindex(faridx)], o, ray)) 
                    tmax = min(tmax, skelmodel::intersectdist/skelmodel::intersectscale);
            }
        }
        else if(curnode->isleaf(nearidx))
        {
            if(triintersect(m, s, tris[curnode->childindex(nearidx)], o, ray)) tmax = min(tmax, skelmodel::intersectdist/skelmodel::intersectscale); 
            if(farsplit < tmax)
            {
                if(!curnode->isleaf(faridx))
                {
                    curnode = &nodes[curnode->childindex(faridx)];
                    tmin = max(tmin, farsplit);
                    continue;
                }
                else if(triintersect(m, s, tris[curnode->childindex(faridx)], o, ray)) 
                    tmax = min(tmax, skelmodel::intersectdist/skelmodel::intersectscale);
            }
        }
        else
        {
            if(farsplit < tmax)
            {
                if(!curnode->isleaf(faridx))
                {
                    skelbihstack &save = stack.add();
                    save.node = &nodes[curnode->childindex(faridx)];
                    save.tmin = max(tmin, farsplit);
                    save.tmax = tmax;
                }
                else if(triintersect(m, s, tris[curnode->childindex(faridx)], o, ray)) 
                    tmax = min(tmax, skelmodel::intersectdist/skelmodel::intersectscale);
            }
            curnode = &nodes[curnode->childindex(nearidx)];
            tmax = min(tmax, nearsplit);
            continue;
        }
        if(stack.empty()) return;
        skelbihstack &restore = stack.pop();
        curnode = restore.node;
        tmin = restore.tmin;
        tmax = restore.tmax;
    }
}

void skelbih::build(skelmodel::skelmeshgroup *m, vector<skelbihnode> &buildnodes, ushort *indices, int numindices, int depth)
{
    vec vmin(1e16f, 1e16f, 1e16f), vmax(-1e16f, -1e16f, -1e16f);
    loopi(numindices)
    {
        tri &tri = tris[indices[i]];
        skelmodel::skelmesh *tm = (skelmodel::skelmesh *)m->meshes[tri.mesh];
        const vec &ta = tm->verts[tri.vert[0]].pos, &tb = tm->verts[tri.vert[1]].pos, &tc = tm->verts[tri.vert[2]].pos;
        loopk(3)
        {
            float amin = min(ta[k], min(tb[k], tc[k])),
                  amax = max(ta[k], max(tb[k], tc[k]));
            vmin[k] = min(vmin[k], amin);
            vmax[k] = max(vmax[k], amax);
        }
    }
    if(depth==1)
    {
        bbmin = vmin;
        bbmax = vmax;
    }
    if(numindices <= 1)
        return;

    int axis = 2;
    loopk(2) if(vmax[k] - vmin[k] > vmax[axis] - vmin[axis]) axis = k;

    float splitleft, splitright;
    int left, right;
    loopk(3)
    {
        float split = 0.5f*(vmax[axis] + vmin[axis]);
        for(left = 0, right = numindices, splitleft = SHRT_MIN, splitright = SHRT_MAX; left < right;)
        {
            tri &tri = tris[indices[left]];
            skelmodel::skelmesh *tm = (skelmodel::skelmesh *)m->meshes[tri.mesh];
            float ta = tm->verts[tri.vert[0]].pos[axis], tb = tm->verts[tri.vert[1]].pos[axis], tc = tm->verts[tri.vert[2]].pos[axis];
            float amin = min(ta, min(tb, tc)),
                  amax = max(ta, max(tb, tc));
            if(max(split - amin, 0.0f) > max(amax - split, 0.0f)) 
            {
                ++left;
                splitleft = max(splitleft, amax);
            }
            else 
            {    
                --right; 
                swap(indices[left], indices[right]); 
                splitright = min(splitright, amin);
            }
        }
        if(left > 0 && right < numindices) break;
        axis = (axis+1)%3;
    }

    if(!left || right==numindices) 
    {
        left = right = numindices/2;
        splitleft = SHRT_MIN;
        splitright = SHRT_MAX;
        loopi(numindices)
        {
            tri &tri = tris[indices[i]];
            skelmodel::skelmesh *tm = (skelmodel::skelmesh *)m->meshes[tri.mesh];
            float ta = tm->verts[tri.vert[0]].pos[axis], tb = tm->verts[tri.vert[1]].pos[axis], tc = tm->verts[tri.vert[2]].pos[axis];
            if(i < left) splitleft = max(splitleft, max(ta, max(tb, tc)));
            else splitright = min(splitright, min(ta, min(tb, tc)));
        }
    }

    int node = buildnodes.length();
    buildnodes.add();
    buildnodes[node].split[0] = short(ceil(splitleft));
    buildnodes[node].split[1] = short(floor(splitright));

    if(left==1) buildnodes[node].child[0] = (axis<<14) | indices[0];
    else
    {
        buildnodes[node].child[0] = (axis<<14) | buildnodes.length();
        build(m, buildnodes, indices, left, depth+1);
    }

    if(numindices-right==1) buildnodes[node].child[1] = (1<<15) | (left==1 ? 1<<14 : 0) | indices[right];
    else 
    {
        buildnodes[node].child[1] = (left==1 ? 1<<14 : 0) | buildnodes.length();
        build(m, buildnodes, &indices[right], numindices-right, depth+1);
    }
}

skelbih::skelbih(skelmodel::skelmeshgroup *m, int numtris, tri *tris)
  : nodes(NULL), tris(tris), bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f)
{
    vector<skelbihnode> buildnodes;
    ushort *indices = new ushort[numtris];
    loopi(numtris) indices[i] = i;

    build(m, buildnodes, indices, numtris);

    delete[] indices;

    if(buildnodes.length())
    {
        nodes = new skelbihnode[buildnodes.length()];
        memcpy(nodes, buildnodes.getbuf(), buildnodes.length()*sizeof(skelbihnode));
    }
}

struct skelhitzone
{
    typedef skelbih::tri tri;

    int numparents, numchildren;
    skelhitzone **parents, **children;
    vec center, animcenter;
    float radius;
    int visited;
    union
    {
        int blend;
        int numtris;
    };
    union
    {
        tri *tris;
        skelbih *bih;
    };

    skelhitzone() : numparents(0), numchildren(0), parents(NULL), children(NULL), center(0, 0, 0), animcenter(0, 0, 0), radius(0), visited(-1)
    {
        blend = -1;
        bih = NULL;
    }

    ~skelhitzone()
    {
        if(!numchildren) { DELETEP(bih); }
        else { DELETEA(tris); }
    }

    template<class B>
    static bool triintersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const B *bdata1, const B *bdata2, int numblends, const tri &t, const vec &o, const vec &ray);

    bool shellintersect(const vec &o, const vec &ray)
    {
        vec c(animcenter);
        c.sub(o);
        float v = c.dot(ray), inside = radius*radius - c.squaredlen();
        if(inside < 0 && v < 0) return false;
        float d = inside + v*v;
        if(d < 0) return false;
        v -= skelmodel::intersectdist/skelmodel::intersectscale;
        return v < 0 || d >= v*v;
    }

    template<class B>
    void intersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const B *bdata1, const B *bdata2, int numblends, const vec &o, const vec &ray)
    {
        if(!numchildren)
        {
            if(bih)
            {
                const B &b = blend < numblends ? bdata2[blend] : bdata1[m->skel->bones[blend - numblends].interpindex];
                vec bo = b.transposedtransform(o), bray = b.transposedtransformnormal(ray);
                bih->intersect(m, s, bo, bray);
            }
        }
        else if(shellintersect(o, ray))
        {
            loopi(numtris) triintersect(m, s, bdata1, bdata2, numblends, tris[i], o, ray);
            loopi(numchildren) if(children[i]->visited != visited) 
            {
                children[i]->visited = visited;
                children[i]->intersect(m, s, bdata1, bdata2, numblends, o, ray);
            }
        }
    }

    template<class B>
    void propagate(skelmodel::skelmeshgroup *m, const B *bdata1, const B *bdata2, int numblends)
    {
        if(!numchildren)
        {
            const B &b = blend < numblends ? bdata2[blend] : bdata1[m->skel->bones[blend - numblends].interpindex];
            animcenter = b.transform(center);
        }
        else 
        {
            animcenter = children[numchildren-1]->animcenter;
            radius = children[numchildren-1]->radius;
            loopi(numchildren-1)
            {
                skelhitzone *child = children[i];
                vec n = child->animcenter;
                n.sub(animcenter);
                float dist = n.magnitude();
                if(child->radius >= dist + radius)
                {
                    animcenter = child->animcenter;
                    radius = child->radius;
                }
                else if(radius < dist + child->radius)
                {
                    float newradius = 0.5f*(radius + dist + child->radius);
                    animcenter.add(n.mul((newradius - radius)/dist));
                    radius = newradius;
                }
            }
        }
    } 
};

template<class B>
bool skelhitzone::triintersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const B *bdata1, const B *bdata2, int numblends, const tri &t, const vec &o, const vec &ray)
{
    skelmodel::skelmesh *tm = (skelmodel::skelmesh *)m->meshes[t.mesh];
    const skelmodel::vert &va = tm->verts[t.vert[0]], &vb = tm->verts[t.vert[1]], &vc = tm->verts[t.vert[2]];
    vec a = (va.blend < numblends ? bdata2[va.blend] : bdata1[m->blendcombos[va.blend].interpbones[0]]).transform(va.pos),
        eb = (vb.blend < numblends ? bdata2[vb.blend] : bdata1[m->blendcombos[vb.blend].interpbones[0]]).transform(vb.pos).sub(a),
        ec = (vc.blend < numblends ? bdata2[vc.blend] : bdata1[m->blendcombos[vc.blend].interpbones[0]]).transform(vc.pos).sub(a);
    vec p;
    p.cross(ray, ec);
    float det = eb.dot(p);
    if(det == 0) return false;
    vec r(o);
    r.sub(a);
    float u = r.dot(p) / det;
    if(u < 0 || u > 1) return false;
    vec q;
    q.cross(r, eb);
    float v = ray.dot(q) / det;
    if(v < 0 || u + v > 1) return false;
    float f = ec.dot(q) / det;
    if(f < 0 || f*skelmodel::intersectscale > skelmodel::intersectdist) return false;
    if(!(skelmodel::intersectmode&RAY_SHADOW) && tm->noclip) return false;
    Texture *tex = s[t.mesh].tex;
    if(tex->bpp==4 && (skelmodel::intersectmode&RAY_ALPHAPOLY)==RAY_ALPHAPOLY && (tex->alphamask || (loadalphamask(tex), tex->alphamask)))
    {
        int si = clamp(int(tex->xs * (va.u + u*(vb.u - va.u) + v*(vc.u - va.u))), 0, tex->xs-1),
            ti = clamp(int(tex->ys * (va.v + u*(vb.v - va.v) + v*(vc.v - va.v))), 0, tex->ys-1);
        if(!(tex->alphamask[ti*((tex->xs+7)/8) + si/8] & (1<<(si%8)))) return false;
    }
    skelmodel::intersectdist = f*skelmodel::intersectscale;
    skelmodel::intersectresult = t.id&0x80 ? -1 : t.id;
    return true;
}

struct skelzonekey
{
    int blend;
    uchar bones[12];

    skelzonekey() : blend(-1) { memset(bones, 0xFF, sizeof(bones)); }
    skelzonekey(int bone) : blend(INT_MAX) { bones[0] = bone; memset(&bones[1], 0xFF, sizeof(bones)-1); }
    skelzonekey(skelmodel::skelmesh *m, const skelmodel::tri &t)
      : blend(-1)
    {
        memset(bones, 0xFF, sizeof(bones));
        addbones(m, t);
    }

    bool includes(const skelzonekey &o)
    {
        int j = 0;
        loopi(sizeof(bones))
        {
            if(bones[i] > o.bones[j]) return false;
            if(bones[i] == o.bones[j]) j++;
        }
        return j < (int)sizeof(bones) ? o.bones[j] == 0xFF : blend < 0 || blend == o.blend;
    }

    void subtract(const skelzonekey &o)
    {
        int len = 0, j = 0;
        loopi(sizeof(bones))
        {
        retry:
            if(j >= (int)sizeof(o.bones) || bones[i] < o.bones[j]) { bones[len++] = bones[i]; continue; }
            if(bones[i] == o.bones[j]) { j++; continue; }
            do j++; while(j < (int)sizeof(o.bones) && bones[i] > o.bones[j]); 
            goto retry;
        }
        memset(&bones[len], 0xFF, sizeof(bones) - len); 
    }

    bool hasbone(int n)
    {
        loopi(sizeof(bones))
        {
            if(bones[i] == n) return true;
            if(bones[i] == 0xFF) break;
        }
        return false;
    }

    int numbones()
    {
        loopi(sizeof(bones)) if(bones[i] == 0xFF) return i;
        return sizeof(bones);
    }

    void addbone(int n)
    {
        loopi(sizeof(bones))
        {
            if(n <= bones[i])
            {
                if(n < bones[i])
                {
                    memmove(&bones[i+1], &bones[i], sizeof(bones) - (i+1));
                    bones[i] = n;
                }
                return;
            }
        }
    }

    void addbones(skelmodel::skelmesh *m, const skelmodel::tri &t)
    {
        skelmodel::skelmeshgroup *g = (skelmodel::skelmeshgroup *)m->group;
        int b0 = m->verts[t.vert[0]].blend, b1 = m->verts[t.vert[1]].blend, b2 = m->verts[t.vert[1]].blend;
        const skelmodel::blendcombo &c0 = g->blendcombos[b0];
        loopi(4) if(c0.weights[i]) addbone(c0.bones[i]);
        if(b0 != b1 || b0 != b2)
        {
            const skelmodel::blendcombo &c1 = g->blendcombos[b1];
            loopi(4) if(c1.weights[i]) addbone(c1.bones[i]);
            const skelmodel::blendcombo &c2 = g->blendcombos[b2];
            loopi(4) if(c2.weights[i]) addbone(c2.bones[i]);
        }
        else blend = b0;
    }
};

struct skelzonebounds
{
    int owner;
    vec bbmin, bbmax;

    skelzonebounds() : owner(-1), bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f) {}

    void addvert(const vec &p)
    {
        bbmin.x = min(bbmin.x, p.x);
        bbmin.y = min(bbmin.y, p.y);
        bbmin.z = min(bbmin.z, p.z);
        bbmax.x = max(bbmax.x, p.x);
        bbmax.y = max(bbmax.y, p.y);
        bbmax.z = max(bbmax.z, p.z);
    }

    bool empty() const { return bbmin.x > bbmax.x; }

    vec calccenter() const
    {
        return vec(bbmin).add(bbmax).mul(0.5f);
    }

    float calcradius() const
    {
        return vec(bbmax).sub(bbmin).mul(0.5f).magnitude();
    }
};

struct skelzoneinfo
{
    int index, parents, conflicts;
    skelzonekey key;
    vector<skelzoneinfo *> children;
    vector<skelhitzone::tri> tris;

    skelzoneinfo() : index(-1), parents(0), conflicts(0) {}
    skelzoneinfo(const skelzonekey &key) : index(-1), parents(0), conflicts(0), key(key) {}
};

static inline bool htcmp(const skelzonekey &x, const skelzoneinfo &y) { return !memcmp(x.bones, y.key.bones, sizeof(x.bones)) && (x.bones[1] == 0xFF || x.blend == y.key.blend); }

static inline uint hthash(const skelzonekey &k)
{
    union { uint i[3]; uchar b[12]; } conv;
    memcpy(conv.b, k.bones, sizeof(conv.b));
    return conv.i[0]^conv.i[1]^conv.i[2];
}

struct skelhitdata
{
    int numzones, rootzones, visited;
    skelhitzone *zones;
    skelhitzone **links;
    skelhitzone::tri *tris;
    int numblends;
    skelmodel::blendcacheentry blendcache;

    skelhitdata() : numzones(0), rootzones(0), visited(0), zones(NULL), links(NULL), tris(NULL), numblends(0) {}
    ~skelhitdata() 
    { 
        DELETEA(zones); 
        DELETEA(links); 
        DELETEA(tris); 
        if(blendcache.bdata) 
        { 
            delete[] (uchar *)blendcache.bdata; 
            blendcache.bdata = NULL; 
            blendcache.mdata = NULL; 
        } 
    }

    uchar chooseid(skelmodel::skelmeshgroup *g, skelmodel::skelmesh *m, const skelmodel::tri &t, const uchar *ids);
    void build(skelmodel::skelmeshgroup *g, const uchar *ids);

    void cleanup()
    {
        blendcache.owner = -1;
    }

    template<class B>
    void propagate(skelmodel::skelmeshgroup *m, const B *bdata1, B *bdata2)
    {
        visited = 0;
        loopi(numzones) 
        {
            zones[i].visited = -1;
            zones[i].propagate(m, bdata1, bdata2, numblends);
        }
    }

    template<class B>
    void intersect(skelmodel::skelmeshgroup *m, skelmodel::skin *s, const B *bdata1, B *bdata2, const vec &o, const vec &ray)
    {
        if(++visited < 0)
        {
            visited = 0;
            loopi(numzones) zones[i].visited = -1;
        }
        for(int i = numzones - rootzones; i < numzones; i++) 
        {
            zones[i].visited = visited;
            zones[i].intersect(m, s, bdata1, bdata2, numblends, o, ray);
        }
    }
};

void skelmodel::skelmeshgroup::cleanuphitdata()
{
    if(hitdata) hitdata->cleanup();
}

void skelmodel::skelmeshgroup::deletehitdata()
{
    DELETEP(hitdata);
}

void skelmodel::skelmeshgroup::intersect(skelhitdata *z, part *p, const skelmodel::skelcacheentry &sc, const vec &o, const vec &ray)
{
    int owner = &sc - &skel->skelcache[0];
    skelmodel::blendcacheentry &bc = z->blendcache;
    if(bc.owner != owner || bc != sc) 
    {
        bc.owner = owner;
        bc.millis = lastmillis;
        (animcacheentry &)bc = sc;
        if(skel->usematskel) 
        {
            blendbones(sc.mdata, bc.mdata, blendcombos.getbuf(), z->numblends);
            z->propagate(this, sc.mdata, bc.mdata);
        }
        else 
        {
            blendbones(sc.bdata, bc.bdata, blendcombos.getbuf(), z->numblends);
            z->propagate(this, sc.bdata, bc.bdata);
        }
    }
    if(skel->usematskel) z->intersect(this, p->skins.getbuf(), sc.mdata, bc.mdata, o, ray);
    else z->intersect(this, p->skins.getbuf(), sc.bdata, bc.bdata, o, ray);
}

uchar skelhitdata::chooseid(skelmodel::skelmeshgroup *g, skelmodel::skelmesh *m, const skelmodel::tri &t, const uchar *ids)
{
    int numused = 0;
    uchar used[12];
    float weights[12];
    loopk(3)
    {
        const skelmodel::vert &v = m->verts[t.vert[k]];
        const skelmodel::blendcombo &c = g->blendcombos[v.blend];
        loopl(4) if(c.weights[l]) 
        {
            uchar id = ids[c.bones[l]];
            loopi(numused) if(used[i] == id) { weights[i] += c.weights[l]; goto nextbone; }
            used[numused] = id;
            weights[numused] = c.weights[l];
            numused++;
        nextbone:;
        }
    }
    uchar bestid = 0xFF;
    float bestweight = 0;
    loopi(numused) if(weights[i] > bestweight || (weights[i] == bestweight && used[i] < bestid))
    {
        bestid = used[i];
        bestweight = weights[i];
    }
    return bestid;
}

void skelhitdata::build(skelmodel::skelmeshgroup *g, const uchar *ids)
{
    if(numzones) return;

    hashset<skelzoneinfo> infomap(256);
    vector<skelzoneinfo *> info;
    skelzonebounds *bounds = new skelzonebounds[g->skel->numbones];
    numblends = g->blendcombos.length();
    loopv(g->blendcombos) if(!g->blendcombos[i].weights[1]) { numblends = i; break; }
    uchar *cdata = numblends > 0 ? new uchar[numblends*max(sizeof(dualquat), sizeof(matrix3x4))] : NULL;
    blendcache.bdata = (dualquat *)cdata;
    blendcache.mdata = (matrix3x4 *)cdata;
    loopi(min(g->meshes.length(), 0x100))
    {
        skelmodel::skelmesh *m = (skelmodel::skelmesh *)g->meshes[i];
        loopj(m->numtris)
        {
            const skelmodel::tri &t = m->tris[j];
            loopk(3)
            {
                const skelmodel::vert &v = m->verts[t.vert[k]];
                const skelmodel::blendcombo &c = g->blendcombos[v.blend];
                loopl(4) if(c.weights[l]) bounds[c.bones[l]].addvert(v.pos);
            }
            skelzonekey key(m, t);
            skelzoneinfo &zi = infomap.access(key, key);
            if(key.blend >= numblends && zi.index < 0) 
            { 
                bounds[key.bones[0]].owner = zi.index = info.length(); 
                info.add(&zi); 
            }
            skelhitzone::tri &zt = zi.tris.add();
            zt.mesh = i;
            zt.id = chooseid(g, m, t, ids);
            memcpy(zt.vert, t.vert, sizeof(zt.vert));
        }
    }
    loopi(g->skel->numbones)
    {
        if(bounds[i].empty() || bounds[i].owner >= 0) continue;
        skelzonekey key(i);
        skelzoneinfo &zi = infomap.access(key, key);
        zi.index = info.length();
        info.add(&zi);
    } 
    int leafzones = info.length();
    enumerate(infomap, skelzoneinfo, zi, { if(zi.index < 0) info.add(&zi); });
    for(int i = leafzones; i < info.length(); i++)
    {
        skelzoneinfo &zi = *info[i];    
        if(zi.key.blend >= 0) continue;
        loopvj(info) if(i != j && zi.key.includes(info[j]->key))
        {
            skelzoneinfo &zj = *info[j];
            loopvk(zi.children) 
            {
                skelzoneinfo &zk = *zi.children[k];
                if(zk.key.includes(zj.key)) goto nextzone;
                if(zj.key.includes(zk.key)) 
                {
                    zk.parents--;
                    zj.parents++;
                    zi.children[k] = &zj;
                    while(++k < zi.children.length()) if(zj.key.includes(zi.children[k]->key)) { zi.children[k]->parents--; zi.children.removeunordered(k--); }
                    goto nextzone;
                }
            }
            zj.parents++;
            zi.children.add(&zj);
        nextzone:;
        }
        skelzonekey deps = zi.key;
        loopvj(zi.children)
        {
            skelzoneinfo &zj = *zi.children[j];
            if(zj.key.blend < 0 || zj.key.blend >= numblends) deps.subtract(zj.key);
        }
        loopj(sizeof(deps.bones)) 
        {
            if(deps.bones[j]==0xFF) break;
            skelzonekey dep(deps.bones[j]);
            skelzoneinfo &zj = infomap.access(dep, dep);
            zj.parents++;
            zi.children.add(&zj);
        }
    }
    for(int i = leafzones; i < info.length(); i++)
    {
        skelzoneinfo &zi = *info[i];
        loopvj(zi.children)
        {
            skelzoneinfo &zj = *zi.children[j];
            if(zj.tris.length() <= 2 && zj.parents == 1)
            {
                zi.tris.put(zj.tris.getbuf(), zj.tris.length());
                zj.tris.setsize(0);
                if(zj.index < 0)
                {
                    zj.parents = 0;
                    zi.children.removeunordered(j--);
                }
                zi.children.put(zj.children.getbuf(), zj.children.length());
                zj.children.setsize(0);
            }
        }
    }
    int numlinks = 0, numtris = 0;
    loopvrev(info)
    {
        skelzoneinfo &zi = *info[i];
        if(zi.parents || zi.tris.empty()) info.removeunordered(i);
        zi.conflicts = zi.parents;
        numlinks += zi.parents + zi.children.length();
        numtris += zi.tris.length();
    }    
    rootzones = info.length();
    loopv(info)
    {
        skelzoneinfo &zi = *info[i];
        zi.index = i;
        loopvj(zi.children)
        {
            skelzoneinfo &zj = *zi.children[j];
            if(!--zj.conflicts) info.add(&zj);
        }
    }
    numzones = info.length();
    zones = new skelhitzone[numzones];
    links = numlinks ? new skelhitzone *[numlinks] : NULL;
    tris = new skelhitzone::tri[numtris];
    skelhitzone **curlink = links;
    skelhitzone::tri *curtris = tris;
    loopi(numzones)
    {
        skelhitzone &z = zones[i];
        skelzoneinfo &zi = *info[info.length()-1 - i];
        memcpy(curtris, zi.tris.getbuf(), zi.tris.length()*sizeof(skelhitzone::tri));
        if(zi.key.blend >= numblends)
        {
            z.blend = zi.key.bones[0] + numblends;
            if(zi.tris.length()) z.bih = new skelbih(g, zi.tris.length(), curtris);
            const skelzonebounds &b = bounds[zi.key.bones[0]];
            z.center = b.calccenter();
            z.radius = b.calcradius();
        }
        else if(zi.key.blend >= 0)
        {
            z.blend = zi.key.blend;
            z.bih = new skelbih(g, zi.tris.length(), curtris);
            z.center = z.bih->calccenter();
            z.radius = z.bih->calcradius();
        }
        else
        {
            z.numtris = zi.tris.length();
            z.tris = curtris;
        }
        curtris += zi.tris.length();
        z.parents = curlink;
        curlink += zi.parents;
        z.numchildren = zi.children.length();
        z.children = curlink;
        loopvj(zi.children) z.children[j] = &zones[info.length()-1 - zi.children[j]->index];
        curlink += zi.children.length();
    }
    loopi(numzones)
    {
        skelhitzone &z = zones[i];
        loopj(z.numchildren) z.children[j]->parents[z.children[j]->numparents++] = &z;
    }
    delete[] bounds;
}

void skelmodel::skelmeshgroup::buildhitdata(const uchar *hitzones)
{
    if(hitdata) return;
    hitdata = new skelhitdata;
    hitdata->build(this, hitzones);
}   

