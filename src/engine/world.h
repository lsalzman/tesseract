
enum                            // hardcoded texture numbers
{
    DEFAULT_SKY = 0,
    DEFAULT_GEOM
};

#define MAPVERSION 33           // bump if map format changes, see worldio.cpp

struct octaheader
{
    char magic[4];              // "OCTA"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int lightmaps;
    int blendmap;
    int numvars;
    int numvslots;
};
    
#define WATER_AMPLITUDE 0.4f
#define WATER_OFFSET 1.1f

enum 
{ 
    MATSURF_NOT_VISIBLE = 0,
    MATSURF_VISIBLE,
    MATSURF_EDIT_ONLY
};

#define TEX_SCALE 16.0f

struct vertex { vec pos; bvec norm; uchar reserved; vec2 tc; bvec tangent; uchar bitangent; };
 
