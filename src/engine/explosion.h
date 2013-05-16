VARP(softexplosion, 0, 1, 1);
VARP(softexplosionblend, 1, 16, 64);

//cache our unit hemisphere
namespace hemisphere
{
    GLushort *indices = NULL;
    vec *verts = NULL;
    int numverts = 0, numindices = 0;
    GLuint vbuf = 0, ebuf = 0;

    void subdivide(int depth, int face);

    void genface(int depth, int i1, int i2, int i3)
    {
        int face = numindices; numindices += 3;
        indices[face]   = i1;
        indices[face+1] = i2;
        indices[face+2] = i3;
        subdivide(depth, face);
    }

    void subdivide(int depth, int face)
    {
        if(depth-- <= 0) return;
        int idx[6];
        loopi(3) idx[i] = indices[face+i];
        loopi(3)
        {
            int curvert = numverts++;
            verts[curvert] = vec(verts[idx[i]]).add(verts[idx[(i+1)%3]]).normalize(); //push on to unit sphere
            idx[3+i] = curvert;
            indices[face+i] = curvert;
        }
        subdivide(depth, face);
        loopi(3) genface(depth, idx[i], idx[3+i], idx[3+(i+2)%3]);
    }

    //subdiv version wobble much more nicely than a lat/longitude version
    void init(int hres, int depth)
    {
        const int tris = hres << (2*depth);
        numverts = numindices = 0;
        verts = new vec[tris+1];
        indices = new GLushort[tris*3];
        verts[numverts++] = vec(0.0f, 0.0f, 1.0f); //build initial 'hres' sided pyramid
        loopi(hres) verts[numverts++] = vec(sincos360[(360*i)/hres], 0.0f);
        loopi(hres) genface(depth, 0, i+1, 1+(i+1)%hres);

        if(!vbuf) glGenBuffers_(1, &vbuf);
        glBindBuffer_(GL_ARRAY_BUFFER, vbuf);
        glBufferData_(GL_ARRAY_BUFFER, numverts*sizeof(vec), verts, GL_STATIC_DRAW);
        DELETEA(verts);

        if(!ebuf) glGenBuffers_(1, &ebuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebuf);
        glBufferData_(GL_ELEMENT_ARRAY_BUFFER, numindices*sizeof(GLushort), indices, GL_STATIC_DRAW);
        DELETEA(indices);
    }

    void cleanup()
    {
        if(vbuf) { glDeleteBuffers_(1, &vbuf); vbuf = 0; }
        if(ebuf) { glDeleteBuffers_(1, &ebuf); ebuf = 0; }
    }

    void enable()
    {
        if(!vbuf) init(5, 2);
        glBindBuffer_(GL_ARRAY_BUFFER, vbuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebuf);

        gle::vertexpointer(sizeof(vec), verts);
        gle::enablevertex();
    }

    void draw()
    {
        glDrawRangeElements_(GL_TRIANGLES, 0, numverts-1, numindices, GL_UNSIGNED_SHORT, indices);
        xtraverts += numindices;
        glde++;
    }
    
    void disable()
    {
        gle::disablevertex();

        glBindBuffer_(GL_ARRAY_BUFFER, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

static GLuint expmodtex[2] = {0, 0};
static GLuint lastexpmodtex = 0;

static GLuint createexpmodtex(int size, float minval)
{
    uchar *data = new uchar[size*size], *dst = data;
    loop(y, size) loop(x, size)
    {
        float dx = 2*float(x)/(size-1) - 1, dy = 2*float(y)/(size-1) - 1;
        float z = max(0.0f, 1.0f - dx*dx - dy*dy);
        if(minval) z = sqrtf(z);
        else loopk(2) z *= z;
        *dst++ = uchar(max(z, minval)*255);
    }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    createtexture(tex, size, size, data, 3, 2, hasTRG ? GL_R8 : GL_LUMINANCE8);
    delete[] data;
    return tex;
}

namespace sphere
{
    struct vert
    {
        vec pos;
        float s, t;
    } *verts = NULL;
    GLushort *indices = NULL;
    int numverts = 0, numindices = 0;
    GLuint vbuf = 0, ebuf = 0;

    void init(int slices, int stacks)
    {
        numverts = (stacks+1)*(slices+1);
        verts = new vert[numverts];
        float ds = 1.0f/slices, dt = 1.0f/stacks, t = 1.0f;
        loopi(stacks+1)
        {
            float rho = M_PI*(1-t), s = 0.0f;
            loopj(slices+1)
            {
                float theta = j==slices ? 0 : 2*M_PI*s;
                vert &v = verts[i*(slices+1) + j];
                v.pos = vec(-sin(theta)*sin(rho), cos(theta)*sin(rho), cos(rho));
                v.s = s;
                v.t = t;
                s += ds;
            }
            t -= dt;
        }

        numindices = stacks*slices*3*2;
        indices = new ushort[numindices];
        GLushort *curindex = indices;
        loopi(stacks)
        {
            loopk(slices)
            {
                int j = i%2 ? slices-k-1 : k;

                *curindex++ = i*(slices+1)+j;
                *curindex++ = (i+1)*(slices+1)+j;
                *curindex++ = i*(slices+1)+j+1;

                *curindex++ = i*(slices+1)+j+1;
                *curindex++ = (i+1)*(slices+1)+j;
                *curindex++ = (i+1)*(slices+1)+j+1;
            }
        }

        if(!vbuf) glGenBuffers_(1, &vbuf);
        glBindBuffer_(GL_ARRAY_BUFFER, vbuf);
        glBufferData_(GL_ARRAY_BUFFER, numverts*sizeof(vert), verts, GL_STATIC_DRAW);
        DELETEA(verts);

        if(!ebuf) glGenBuffers_(1, &ebuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebuf);
        glBufferData_(GL_ELEMENT_ARRAY_BUFFER, numindices*sizeof(GLushort), indices, GL_STATIC_DRAW);
        DELETEA(indices);
    }

    void cleanup()
    {
        if(vbuf) { glDeleteBuffers_(1, &vbuf); vbuf = 0; }
        if(ebuf) { glDeleteBuffers_(1, &ebuf); ebuf = 0; }
    }

    void enable()
    {
        if(!vbuf) init(12, 6);

        glBindBuffer_(GL_ARRAY_BUFFER, vbuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebuf);

        gle::vertexpointer(sizeof(vert), &verts->pos);
        gle::texcoord0pointer(sizeof(vert), &verts->s);
        gle::enablevertex();
        gle::enabletexcoord0();
    }

    void draw()
    {
        glDrawRangeElements_(GL_TRIANGLES, 0, numverts-1, numindices, GL_UNSIGNED_SHORT, indices);
        xtraverts += numindices;
        glde++;
    }

    void disable()
    {
        gle::disablevertex();
        gle::disabletexcoord0();

        glBindBuffer_(GL_ARRAY_BUFFER, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

VARP(explosion2d, 0, 0, 1);

static void setupexplosion()
{
    if(!expmodtex[0]) expmodtex[0] = createexpmodtex(64, 0);
    if(!expmodtex[1]) expmodtex[1] = createexpmodtex(64, 0.25f);
    lastexpmodtex = 0;

    if(softparticles && softexplosion)
    {
        if(explosion2d) SETSHADER(explosion2dsoft); else SETSHADER(explosion3dsoft);
    }
    else if(explosion2d) SETSHADER(explosion2d); else SETSHADER(explosion3d);

    if(explosion2d) hemisphere::enable();
    else sphere::enable();
}

static void drawexplosion(bool inside, float r, float g, float b, float a)
{
    if(lastexpmodtex != expmodtex[inside ? 1 : 0])
    {
        glActiveTexture_(GL_TEXTURE1);
        lastexpmodtex = expmodtex[inside ? 1 :0];
        glBindTexture(GL_TEXTURE_2D, lastexpmodtex);
        glActiveTexture_(GL_TEXTURE0);
    }
    if(!explosion2d)
    {
        LOCALPARAMF(side, (inside ? -1 : 1));
        loopi(inside ? 2 : 1)
        {
            gle::colorf(r, g, b, i ? a/2 : a);
            if(i) glDepthFunc(GL_GEQUAL);
            sphere::draw();
            if(i) glDepthFunc(GL_LESS);
        }
        return;
    }
    loopi(inside ? 2 : 1)
    {
        gle::colorf(r, g, b, i ? a/2 : a);
        LOCALPARAMF(side, (1));
        if(i) glDepthFunc(GL_GEQUAL);
        if(inside)
        {
            glCullFace(GL_FRONT);
            hemisphere::draw();
            glCullFace(GL_BACK);
            LOCALPARAMF(side, (-1));
        }
        hemisphere::draw();
        if(i) glDepthFunc(GL_LESS);
    }
}

static void cleanupexplosion()
{
    if(explosion2d) hemisphere::disable();
    else sphere::disable();
}

static void deleteexplosions()
{
    loopi(2) if(expmodtex[i]) { glDeleteTextures(1, &expmodtex[i]); expmodtex[i] = 0; }
    hemisphere::cleanup();
    sphere::cleanup();
}

static const float WOBBLE = 1.25f;

struct fireballrenderer : listrenderer
{
    fireballrenderer(const char *texname)
        : listrenderer(texname, 0, PT_FIREBALL|PT_SHADER)
    {}

    void startrender()
    {
        setupexplosion();
    }

    void endrender()
    {
        cleanupexplosion();
    }

    void cleanup()
    {
        deleteexplosions();
    }

    void seedemitter(particleemitter &pe, const vec &o, const vec &d, int fade, float size, int gravity)
    {
        pe.maxfade = max(pe.maxfade, fade);
        pe.extendbb(o, (size+1+pe.ent->attr2)*WOBBLE); 
    }

    void renderpart(listparticle *p, const vec &o, const vec &d, int blend, int ts)
    {
        float pmax = p->val,
              size = p->fade ? float(ts)/p->fade : 1,
              psize = p->size + pmax * size;

        if(isfoggedsphere(psize*WOBBLE, p->o)) return;

        glmatrix m;
        m.identity();
        m.translate(o);

        bool inside = o.dist(camera1->o) <= psize*WOBBLE;
        vec oc(o);
        oc.sub(camera1->o);

        float yaw = inside ? camera1->yaw : atan2(oc.y, oc.x)/RAD - 90,
        pitch = (inside ? camera1->pitch : asin(oc.z/oc.magnitude())/RAD) - 90;
        vec rotdir;
        float rotangle = lastmillis/1000.0f*143;
        if(explosion2d)
        {
            m.rotate_around_z(yaw*RAD);
            m.rotate_around_x(pitch*RAD);
            rotdir = vec(0, 0, 1);
        }
        else
        {
            vec s(1, 0, 0), t(0, 1, 0);
            s.rotate_around_x(pitch*-RAD);
            s.rotate_around_z(yaw*-RAD);
            t.rotate_around_x(pitch*-RAD);
            t.rotate_around_z(yaw*-RAD);

            rotdir = vec(-1, 1, -1).normalize();
            s.rotate(rotangle*-RAD, rotdir);
            t.rotate(rotangle*-RAD, rotdir);

            LOCALPARAMF(texgenS, (0.5f*s.x, 0.5f*s.y, 0.5f*s.z, 0.5f));
            LOCALPARAMF(texgenT, (0.5f*t.x, 0.5f*t.y, 0.5f*t.z, 0.5f));
        }

        m.rotate(rotangle*RAD, vec(-rotdir.x, rotdir.y, -rotdir.z));
        m.scale(-psize, psize, -psize);
        m.mul(camprojmatrix, glmatrix(m));
        LOCALPARAM(explosionmatrix, m);

        LOCALPARAM(center, o);
        LOCALPARAMF(animstate, (size, psize, pmax, lastmillis/1000.0f));
        if(2*(p->size + pmax)*WOBBLE >= softexplosionblend)
        {
            LOCALPARAMF(softparams, (-1.0f/softexplosionblend, 0, inside ? blend/(2*255.0f) : 0));
        }
        else
        {
            LOCALPARAMF(softparams, (0, -1, inside ? blend/(2*255.0f) : 0));
        }

        drawexplosion(inside, p->color.r*ldrscaleb, p->color.g*ldrscaleb, p->color.b*ldrscaleb, blend/255.0f);
    }
};
static fireballrenderer fireballs("packages/particles/explosion.png"), bluefireballs("packages/particles/plasma.png");

