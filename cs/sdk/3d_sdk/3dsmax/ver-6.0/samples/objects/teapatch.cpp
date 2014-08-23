#include <stdlib.h>
#include "tea_util.h"

#pragma warning(disable: 4244)

/*
* The comments on patches number the quadrants looking down 
* with the spout at 12 o'clock
*/
int patches[32][4][4] = {
    {{0, 1, 2, 3},             /*  0:  top rim 12 -> 3 */
     {4, 5, 6, 7},
     {8, 9, 10, 11},
     {12, 13, 14, 15}},

    {{3, 16, 17, 18},          /*  1:  top rim 3 -> 6 */
     {7, 19, 20, 21},
     {11, 22, 23, 24},
     {15, 25, 26, 27}},

    {{18, 28, 29, 30},         /*  2:  top rim 6 -> 9 */
     {21, 31, 32, 33},
     {24, 34, 35, 36},
     {27, 37, 38, 39}},

    {{30, 40, 41, 0},          /*  3:  top rim 9 ->12 */
     {33, 42, 43, 4},
     {36, 44, 45, 8},
     {39, 46, 47, 12}},

    {{12, 13, 14, 15},         /*  4:  Top Body 12 -> 3 */
     {48, 49, 50, 51},
     {52, 53, 54, 55},
     {56, 57, 58, 59}},

    {{15, 25, 26, 27},         /*  5:  Top Body 3 -> 6 */
     {51, 60, 61, 62},
     {55, 63, 64, 65},
     {59, 66, 67, 68}},

    {{27, 37, 38, 39},         /*  6:  Top Body 6 -> 9 */
     {62, 69, 70, 71},
     {65, 72, 73, 74},
     {68, 75, 76, 77}},

    {{39, 46, 47, 12},         /*  7:  Top Body 9 -> 12 */
     {71, 78, 79, 48},
     {74, 80, 81, 52},
     {77, 82, 83, 56}},

    {{56, 57, 58, 59},         /*  8:  Bottom Body 12 -> 3 */
     {84, 85, 86, 87},
     {88, 89, 90, 91},
     {92, 93, 94, 95}},

    {{59, 66, 67, 68},         /*  9:  Bottom Body 3 -> 6 */
     {87, 96, 97, 98},
     {91, 99, 100, 101},
     {95, 102, 103, 104}},

    {{68, 75, 76, 77},         /* 10:  Bottom Body 6 -> 9 */
     {98, 105, 106, 107},
     {101, 108, 109, 110},
     {104, 111, 112, 113}},

    {{ 77,  82,  83, 56},      /* 11:  Bottom Body 9 -> 12 */
     {107, 114, 115, 84},
     {110, 116, 117, 88},
     {113, 118, 119, 92}},


    {{ 92,  93,  94,  95},     /* 12: Bottom 12 -> 3 */
     {274, 303, 302, 296},
     {278, 305, 304, 299},
     {269, 269, 269, 269}},

    {{95,  102, 103, 104},     /* 13: Bottom 3 -> 6 */
     {296, 295, 294, 287},
     {299, 298, 297, 290},
     {269, 269, 269, 269}},

    {{104, 111, 112, 113},     /* 14: Bottom 6 -> 9 */
     {287, 286, 285, 277},
     {290, 289, 288, 281},
     {269, 269, 269, 269}},

    {{113, 118, 119,  92},     /* 15: Bottom 9 -> 12 */
     {277, 276, 275, 274},
     {281, 280, 279, 278},
     {269, 269, 269, 269}},





    {{120, 121, 122, 123},     /* 16:  Top Handle RIGHT */
     {124, 125, 126, 127},
     {128, 129, 130, 131},
     {132, 133, 134, 135}},

    {{123, 136, 137, 120},     /* 17:  Top Handle LEFT */
     {127, 138, 139, 124},
     {131, 140, 141, 128},
     {135, 142, 143, 132}},

    {{132, 133, 134, 135},     /* 18:  Bottom Handle RIGHT */
     {144, 145, 146, 147},
     {148, 149, 150, 151},
     {68, 152, 153, 154}},

    {{135, 142, 143, 132},     /* 19:  Bottom Handle LEFT */
     {147, 155, 156, 144},
     {151, 157, 158, 148},
     {154, 159, 160, 68}},





    {{161, 162, 163, 164},     /* 20:  Spout Body RIGHT */
     {165, 166, 167, 168},
     {169, 170, 171, 172},
     {173, 174, 175, 176}},

    {{164, 177, 178, 161},     /* 21:  Spout Body LEFT */
     {168, 179, 180, 165},
     {172, 181, 182, 169},
     {176, 183, 184, 173}},

    {{173, 174, 175, 176},     /* 22:  Spout Tip RIGHT */
     {185, 186, 187, 188},
     {189, 190, 191, 192},
     {193, 194, 195, 196}},

    {{176, 183, 184, 173},     /* 23:  Spout Tip LEFT */
     {188, 197, 198, 185},
     {192, 199, 200, 189},
     {196, 201, 202, 193}},




    {{203, 203, 203, 203},     /* 24:  Lid Handle 12 -> 3 */
     {206, 207, 208, 209},
     {210, 210, 210, 210},
     {211, 212, 213, 214}},

    {{203, 203, 203, 203},     /* 25:  Lid Handle 3 -> 6 */
     {209, 216, 217, 218},
     {210, 210, 210, 210},
     {214, 219, 220, 221}},

    {{203, 203, 203, 203},     /* 26:  Lid Handle 6 -> 9 */
     {218, 223, 224, 225},
     {210, 210, 210, 210},
     {221, 226, 227, 228}},

    {{203, 203, 203, 203},     /* 27:  Lid Handle 9 -> 12 */
     {225, 229, 230, 206},
     {210, 210, 210, 210},
     {228, 231, 232, 211}},

    {{211, 212, 213, 214},     /* 28: Lid 12 -> 3 */
     {233, 234, 235, 236},
     {237, 238, 239, 240},
     {241, 242, 243, 244}},

    {{214, 219, 220, 221},     /* 29: Lid 3 -> 6 */
     {236, 245, 246, 247},
     {240, 248, 249, 250},
     {244, 251, 252, 253}},

    {{221, 226, 227, 228},     /* 30: Lid 6 -> 9 */
     {247, 254, 255, 256},
     {250, 257, 258, 259},
     {253, 260, 261, 262}},

    {{228, 231, 232, 211},     /* 31: Lid 9 -> 12 */
     {256, 263, 264, 233},
     {259, 265, 266, 237},
     {262, 267, 268, 241}}
};

TeaEdges
edges[PATCH_COUNT] = {
    {                           /*  0:  top rim 12 -> 3 */
        -1,     -1,
        -1,     -1,
        -1,     -1,
        -1,     -1,
        GenAll, GenAll, GenAll,
		2.0f, 2.0f, 1.0f, 1.9f
    },
    {                           /*  1:  top rim 3 -> 6 */
        0,      2,
        -1,     -1,
        -1,     -1,
        -1,     -1,
        ShareAll, GenAll, GenAll,
		1.0f, 2.0f, 0.0f, 1.9f
    },
    {                           /*  2:  top rim 6 -> 9 */
        1,      2,
        -1,     -1,
        -1,     -1,
        -1,     -1,
        ShareAll, GenAll, GenAll,
		2.0f, 2.0f, 1.0f, 1.9f
    },
    {                           /*  3:  top rim 9 ->12 */
        2,      2,
        -1,     -1,
        0,      0,
        -1,     -1,
        ShareAll, GenAll, ShareAll,
		1.0f, 2.0f, 0.0f, 1.9f
    },

    {                           /*  4:  Top Body 12 -> 3 */
        0,      0,
        -1,     -1,
        -1,     -1,
        0,      1,
        ShareBegin, ShareBegin, ShareBegin,
		2.0f, 1.9f, 1.0f, 1.0f
    },
    {                           /*  5:  Top Body 3 -> 6 */
        4,      2,
        -1,     -1,
        -1,     -1,
        1,      1,
        ShareAll, ShareBegin, ShareBegin,
		1.0f, 1.9f, 0.0f, 1.0f
    },
    {                           /*  6:  Top Body 6 -> 9 */
        5,      2,
        -1,     -1,
        -1,     -1,
        2,      1,
        ShareAll, ShareBegin, ShareBegin,
		2.0f, 1.9f, 1.0f, 1.0f
    },
    {                           /*  7:  Top Body 9 -> 12 */
        6,      2,
        -1,     -1,
        4,      0,
        3,      1,
        ShareAll, ShareBegin, ShareAll,
		1.0f, 1.9f, 0.0f, 1.0f
    },

    {                           /*  8:  Bottom Body 12 -> 3 */
        4,      0,
        -1,     -1,
        -1,     -1,
        4,      1,
        ShareBegin, ShareBegin, ShareBegin,
		2.0f, 1.0f, 1.0f, 0.4f
    },
    {                           /*  9:  Bottom Body 3 -> 6 */
        8,      2,
        -1,     -1,
        -1,     -1,
        5,      1,
        ShareAll, ShareBegin, ShareBegin,
		1.0f, 1.0f, 0.0f, 0.4f
    },
    {                           /* 10:  Bottom Body 6 -> 9 */
        9,      2,
        -1,     -1,
        -1,     -1,
        6,      1,
        ShareAll, ShareBegin, ShareBegin,
		2.0f, 1.0f, 1.0f, 0.4f
    },
    {                           /* 11:  Bottom Body 9 -> 12 */
        10,     2,
        -1,     -1,
        8,      0,
        7,      1,
        ShareAll, ShareBegin, ShareAll,
		1.0f, 1.0f, 0.0f, 0.4f
    },

    {                           /* 12: Bottom 12 -> 3 */
        8,      0,
        -1,     -1,
        -1,     -1,
        8,      1,
        ShareBegin, ShareBeginSingularityEnd, ShareBeginSingularityEnd,
		2.0f, 0.4f, 1.0f, 0.0f
    },
    {                           /* 13: Bottom 3 -> 6 */
        12,     2,
        -1,     -1,
        -1,     -1,
        9,      1,
        ShareAll, ShareBeginSingularityEnd, ShareBeginSingularityEnd,
		1.0f, 0.4f, 0.0f, 0.0f
    },
    {                           /* 14: Bottom 6 -> 9 */
        13,     2,
        -1,     -1,
        -1,     -1,
        10,     1,
        ShareAll, ShareBeginSingularityEnd, ShareBeginSingularityEnd,
		2.0f, 0.4f, 1.0f, 0.0f
    },
    {                           /* 15: Bottom 9 -> 12 */
        14,     2,
        -1,     -1,
        12,     0,
        11,     1,
        ShareAll, ShareBeginSingularityEnd, ShareAll,
		1.0f, 0.4f, 0.0f, 0.0f
    },





    {                           /* 16:  Top Handle RIGHT */
        -1,     -1,
        -1,     -1,
        -1,     -1,
        -1,     -1,
        GenAll, GenAll, GenAll,
		1.0f, 1.0f, 0.5f, 0.5f
    },
    {                           /* 17:  Top Handle LEFT */
        16,     2,
        -1,     -1,
        16,     0,
        -1,     -1,
        ShareAll, GenAll, ShareAll,
		0.5f, 1.0f, 0.0f, 0.5f
    },
    {                           /* 18:  Bottom Handle RIGHT */
        16,     0,
        -1,     -1,
        -1,     -1,
        16,     1,
        ShareBegin, ShareBegin, ShareBegin,
		1.0f, 0.5f, 0.5f, 0.0f
    },
    {                           /* 19:  Bottom Handle LEFT */
        18,     2,
        -1,     -1,
        18,     0,
        17,     1,
        ShareAll, ShareBegin, ShareAll,
		0.5f, 0.5f, 0.0f, 0.0f
	},




    {                           /* 20:  Spout Body RIGHT */
        -1, -1,
        -1, -1,
        -1, -1,
        -1, -1,
        GenAll, GenAll, GenAll,
		0.5f, 0.0f, 1.0f, 0.9f
    },
    {                           /* 21:  Spout Body LEFT */
        20,     2,
        -1,     -1,
        20,     0,
        -1,     -1,
        ShareAll, GenAll, ShareAll,
		0.0f, 0.0f, 0.5f, 0.9f
    },
    {                           /* 22:  Spout Tip RIGHT */
        20,     0,
        -1,     -1,
        -1,     -1,
        20,     1,
        ShareBegin, ShareBegin, ShareBegin,
		0.5f, 0.9f, 1.0f, 1.0f
    },
    {                           /* 23:  Spout Tip LEFT */
        22,     2,
        -1,     -1,
        22,     0,
        21,     1,
        ShareAll, ShareBegin, ShareAll,
  		0.0f, 0.9f, 0.5f, 1.0f
    },





        
    {                           /* 24:  Lid Handle 12 -> 3 */
        -1,     -1,
        -1,     -1,
        -1,     -1,
        -1,     -1,
        GenAll, GenSingularityBegin, GenSingularityBegin,
		1.0f, 1.0f, 0.5f, 0.0f
    },
    {                           /* 25:  Lid Handle 3 -> 6 */
        24,     2,
        -1,     -1,
        -1,     -1,
        -1,     -1,
        ShareAll, GenSingularityBegin, GenSingularityBegin,
		0.5f, 1.0f, 0.0f, 0.0f
    },
    {                           /* 26:  Lid Handle 6 -> 9 */
        25,     2,
        -1,     -1,
        -1,     -1,
        -1, -1,
        ShareAll, GenSingularityBegin, GenSingularityBegin,
		1.0f, 1.0f, 0.5f, 0.0f
    },
    {                           /* 27:  Lid Handle 9 -> 12 */
        26,     2,
        -1,     -1,
        24,     0,
        -1,     -1,
        ShareAll, GenSingularityBegin, ShareAll,
		0.5f, 1.0f, 0.0f, 0.0f
    },

    {                           /* 28: Lid 12 -> 3 */
        24,     0,
        -1,     -1,
        -1,     -1,
        24,     1,
        ShareBegin, ShareBegin, ShareBegin,
		1.0f, 1.0f, 0.5f, 0.0f
    },
    {                           /* 29: Lid 3 -> 6 */                            
        28,     2,
        -1,     -1,
        -1,     -1,
        25,     1,
        ShareAll, ShareBegin, ShareBegin,
		0.5f, 1.0f, 0.0f, 0.0f
    },
    {                           /* 30: Lid 6 -> 9 */
        29,     2,
        -1,     -1,
        -1,     -1,
        26,     1,
        ShareAll, ShareBegin, ShareBegin,
		1.0f, 1.0f, 0.5f, 0.0f
    },
    {                           /* 31: Lid 9 -> 12 */
        30,     2,
        -1,     -1,
        28,     0,
        27,     1,
        ShareAll, ShareBegin, ShareAll,
		0.5f, 1.0f, 0.0f, 0.0f
    },
};

