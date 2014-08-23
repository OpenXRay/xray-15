// Structures for Sprite Icons for Sliders Custom Object
// Arnie Cachelin, Copyright 2001 NewTek, Inc.
// 

typedef struct {
	int		 resW, resH;
	unsigned char	*rgba;
} HUD_Icon;

typedef enum {ICON_DRAG=0, ICON_SIZE, ICON_UNFOLD, ICON_FOLD, ICON_ENV, ICON_KNOB, ICON_MAX} IconTypes;
