#define DEGREE				3
#define DUCK_COUNT			306
#define PATCH_COUNT			32

#define FIRST_BODY_PATCH	0
#define FIRST_HANDLE_PATCH	16
#define FIRST_SPOUT_PATCH	20
#define FIRST_LID_PATCH		24

#define BODY_PATCHES		16
#define HANDLE_PATCHES		4
#define SPOUT_PATCHES		4
#define LID_PATCHES			8

typedef struct {
    float    x,y,z;
} Teapoint;

typedef enum {
    GenAll,
    ShareAll,
    ShareBegin,
    ShareBeginSingularityEnd,
    GenSingularityBegin
} EdgeType;

typedef struct {
    /* first the connectivity data */
    int patch0, edge0,  // which patch,
        patch1, edge1,  // which edge,
        patch2, edge2,  // does this patch use from another
        patch3, edge3;

    EdgeType    first, center, last; // how to build edges or center verts

	float bU, bV, eU, eV;
} TeaEdges;

typedef struct {
	int *left,
		*top,
		*right,
		*bottom;
} TeaShare;

extern Teapoint verts[];
extern int patches[PATCH_COUNT][4][4];
extern TeaEdges edges[PATCH_COUNT];
