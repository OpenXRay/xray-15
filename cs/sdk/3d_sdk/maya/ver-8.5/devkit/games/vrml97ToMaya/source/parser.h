typedef union {
	char *string;
	VrmlField *field;
	VrmlNode *node;
        vector<VrmlNode *> *nodeList;
} YYSTYPE;
#define	IDENTIFIER	258
#define	DEF	259
#define	USE	260
#define	PROTO	261
#define	EXTERNPROTO	262
#define	TO	263
#define	IS	264
#define	ROUTE	265
#define	SFN_NULL	266
#define	EVENTIN	267
#define	EVENTOUT	268
#define	FIELD	269
#define	EXPOSEDFIELD	270
#define	SF_BOOL	271
#define	SF_COLOR	272
#define	SF_FLOAT	273
#define	SF_INT32	274
#define	SF_ROTATION	275
#define	SF_TIME	276
#define	SF_IMAGE	277
#define	SF_STRING	278
#define	SF_VEC2F	279
#define	SF_VEC3F	280
#define	MF_COLOR	281
#define	MF_FLOAT	282
#define	MF_INT32	283
#define	MF_ROTATION	284
#define	MF_STRING	285
#define	MF_VEC2F	286
#define	MF_VEC3F	287
#define	SF_NODE	288
#define	MF_NODE	289


extern YYSTYPE yylval;
