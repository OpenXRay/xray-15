/*****************************************************************************
 * PJPROTOS.H - Function prototypes for the client<->FlicLib interfaces.
 *
 *	This file contains prototypes for all the most commonly-used functions.
 *	Additional prototypes for less-commonly used functions are in:
 *
 *		PJCUSTOM.H	- For working with custom rasters.
 *		PJGFX.H 	- For using the graphics library to draw on rasters.
 *
 ****************************************************************************/

#ifndef PJPROTOS_H
#define PJPROTOS_H

#ifndef PJLTYPES_H
	#include "pjltypes.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * clock-related functions...
 *--------------------------------------------------------------------------*/

Boolean 		__cdecl pj_clock_init(void);
void			__cdecl pj_clock_cleanup(void);
unsigned long	__cdecl pj_clock_1000(void);
unsigned long	__cdecl pj_clock_jiffies(void);
unsigned long	__cdecl pj_clock_jiffies2ms(unsigned long jiffies);
unsigned long	__cdecl pj_clock_ms2jiffies(unsigned long milliseconds);

/*----------------------------------------------------------------------------
 * general-purpose utility functions...
 *--------------------------------------------------------------------------*/

Boolean 	__cdecl pj_key_is(void);	/* has key been hit? */
int 		__cdecl pj_key_in(void);	/* get key input (will wait) */

void *		__cdecl pj_malloc(size_t amount);
void *		__cdecl pj_zalloc(size_t amount);
void		__cdecl pj_free(void *pblock);
void		__cdecl pj_freez(void *ppblock);

void		__cdecl pj_doserr_install_handler(void);
void		__cdecl pj_doserr_remove_handler(void);

char *		__cdecl pj_error_get_message(Errcode err);
Errcode 	__cdecl pj_error_internal(Errcode err, char *module_name, int line_number);

/*----------------------------------------------------------------------------
 * video-related data and functions...
 *--------------------------------------------------------------------------*/

extern LocalVdevice *pj_vdev_supervga;
extern LocalVdevice *pj_vdev_vesa;
extern LocalVdevice *pj_vdev_8514;
extern LocalVdevice *pj_vdev_mcga;

void		__cdecl pj_video_add(LocalVdevice *pldev);
void		__cdecl pj_video_add_all(void);
Errcode 	__cdecl pj_video_detect(LocalVdevice **ppldev);
void		__cdecl pj_video_close(FlicRaster **prast);
Errcode 	__cdecl pj_video_open(LocalVdevice *pldev, int mode, FlicRaster **pprast);
Errcode 	__cdecl pj_video_find_open(int width, int height, FlicRaster **pprast);
Errcode 	__cdecl pj_video_find(LocalVdevice **ppldev, int *pmode,
						int width, int height);
Errcode 	__cdecl pj_video_mode_info(LocalVdevice *pldev, int mode,
						int *pwidth, int *pheight);
Errcode 	__cdecl pj_video_get_current(LocalVdevice **ppdev, int *pmode,
						FlicRaster **pprast);

/*----------------------------------------------------------------------------
 * Raster functions...
 *	 (functions related to custom raster types are in pjcustom.h)
 *--------------------------------------------------------------------------*/

Errcode 	__cdecl pj_raster_make_ram(FlicRaster **pprast, int width, int height);
Errcode 	__cdecl pj_raster_free_ram(FlicRaster **pprast);

Errcode 	__cdecl pj_raster_bind_ram(FlicRaster **pprast,
							int width, int height, Pixel *pbuf);
Errcode 	__cdecl pj_raster_unbind_ram(FlicRaster **pprast);

Errcode 	__cdecl pj_raster_make_centered(FlicRaster **pprast, FlicRaster *proot,
							Flic *pflic);
Errcode 	__cdecl pj_raster_free_centered(FlicRaster **pprast);

Errcode 	__cdecl pj_raster_make_offset(FlicRaster **pprast, FlicRaster *proot,
							Flic *pflic, int x, int y);
Errcode 	__cdecl pj_raster_free_offset(FlicRaster **pprast);

Errcode 	__cdecl pj_raster_copy(FlicRaster *psource, FlicRaster *pdest);
Errcode 	__cdecl pj_raster_clear(FlicRaster *prast);

/*----------------------------------------------------------------------------
 * color map functions...
 *--------------------------------------------------------------------------*/

Errcode 	__cdecl pj_col_load(char *path, PjCmap *cmap);
Errcode 	__cdecl pj_col_save(char *path, PjCmap *cmap);

Errcode 	__cdecl pj_cmap_update(FlicRaster *prast, PjCmap *cmap);

/*----------------------------------------------------------------------------
 * flic read/play functions...
 *--------------------------------------------------------------------------*/

Errcode 	__cdecl pj_playoptions_init(FlicPlayOptions *poptions);

Errcode 	__cdecl pj_flic_play(char *filename, FlicPlayOptions *options);
Errcode 	__cdecl pj_flic_play_once(char *filename, FlicPlayOptions *options);
Errcode 	__cdecl pj_flic_play_timed(char *filename, FlicPlayOptions *options,
							unsigned long for_milliseconds);
Errcode 	__cdecl pj_flic_play_until(char *filename, FlicPlayOptions *options,
							UserEventFunc *puserfunc, void *userdata);

Errcode 	__cdecl pj_flic_play_next(Flic *pflic, FlicRaster *display_raster);
Errcode 	__cdecl pj_flic_play_frames(Flic *pflic, FlicRaster *display_raster,
							int count);

Errcode 	__cdecl pj_flic_close(Flic *pflic);
Errcode 	__cdecl pj_flic_open(char *filename, Flic *pflic);
Errcode 	__cdecl pj_flic_open_info(char *filename, Flic *pflic, AnimInfo *pinfo);
Errcode 	__cdecl pj_flic_file_info(char *filename, AnimInfo *pinfo);
Errcode 	__cdecl pj_flic_info(Flic *pflic, AnimInfo *pinfo);

Errcode 	__cdecl pj_flic_rewind(Flic *pflic);

Errcode 	__cdecl pj_flic_set_speed(Flic *pflic, int speed);

/*----------------------------------------------------------------------------
 * flic creation functions...
 *--------------------------------------------------------------------------*/

Errcode 	__cdecl pj_animinfo_init(AnimInfo *pinfo);

Errcode 	__cdecl pj_flic_complete_filename(char *filename, AnimInfo *pinfo,
							Boolean force_type);

Errcode 	__cdecl pj_flic_create(char *filename, Flic *pflic, AnimInfo *pinfo);
Errcode 	__cdecl pj_flic_write_first(Flic *pflic, FlicRaster *firstframe);
Errcode 	__cdecl pj_flic_write_next(Flic *pflic, FlicRaster *firstframe,
							FlicRaster *priorframe);
Errcode 	__cdecl pj_flic_write_finish(Flic *pflic, FlicRaster *lastframe);

/*
   Low-level file I/O functions
*/

long		__cdecl fli_seek(Flic *flic,long pos);
long		__cdecl fli_tell(Flic *flic);

/*
	Flic routines
*/

int			__cdecl pj_flic_frame(Flic *flic);
int			__cdecl pj_clone_flic(Flic *to,Flic *from);
void		__cdecl pj_free_flic(Flic *flic);

#ifdef	__cplusplus
	}
#endif

/*----------------------------------------------------------------------------
 * these pragmas allow -3r clients to use our -3s style functions...
 *	 the FLICLIB3S alias is defined in PJSTYPES.H.
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
	#pragma aux (FLICLIB3S) pj_clock_init;
	#pragma aux (FLICLIB3S) pj_clock_cleanup;
	#pragma aux (FLICLIB3S) pj_clock_1000;
	#pragma aux (FLICLIB3S) pj_clock_jiffies;
	#pragma aux (FLICLIB3S) pj_clock_jiffies2ms;
	#pragma aux (FLICLIB3S) pj_clock_ms2jiffies;
	#pragma aux (FLICLIB3S) pj_key_is;
	#pragma aux (FLICLIB3S) pj_key_in;
	#pragma aux (FLICLIB3S) pj_malloc;
	#pragma aux (FLICLIB3S) pj_zalloc;
	#pragma aux (FLICLIB3S) pj_free;
	#pragma aux (FLICLIB3S) pj_freez;
	#pragma aux (FLICLIB3S) pj_doserr_install_handler;
	#pragma aux (FLICLIB3S) pj_doserr_remove_handler;
	#pragma aux (FLICLIB3S) pj_error_get_message;
	#pragma aux (FLICLIB3S) pj_error_internal;
	#pragma aux (FLICLIB3S) pj_video_add;
	#pragma aux (FLICLIB3S) pj_video_add_all;
	#pragma aux (FLICLIB3S) pj_video_detect;
	#pragma aux (FLICLIB3S) pj_video_close;
	#pragma aux (FLICLIB3S) pj_video_open;
	#pragma aux (FLICLIB3S) pj_video_find_open;
	#pragma aux (FLICLIB3S) pj_video_find;
	#pragma aux (FLICLIB3S) pj_video_mode_info;
	#pragma aux (FLICLIB3S) pj_video_get_current;
	#pragma aux (FLICLIB3S) pj_raster_make_ram;
	#pragma aux (FLICLIB3S) pj_raster_free_ram;
	#pragma aux (FLICLIB3S) pj_raster_bind_ram;
	#pragma aux (FLICLIB3S) pj_raster_unbind_ram;
	#pragma aux (FLICLIB3S) pj_raster_make_centered;
	#pragma aux (FLICLIB3S) pj_raster_free_centered;
	#pragma aux (FLICLIB3S) pj_raster_make_offset;
	#pragma aux (FLICLIB3S) pj_raster_free_offset;
	#pragma aux (FLICLIB3S) pj_raster_copy;
	#pragma aux (FLICLIB3S) pj_raster_clear;
	#pragma aux (FLICLIB3S) pj_col_load;
	#pragma aux (FLICLIB3S) pj_col_save;
	#pragma aux (FLICLIB3S) pj_cmap_update;
	#pragma aux (FLICLIB3S) pj_playoptions_init;
	#pragma aux (FLICLIB3S) pj_flic_play;
	#pragma aux (FLICLIB3S) pj_flic_play_once;
	#pragma aux (FLICLIB3S) pj_flic_play_timed;
	#pragma aux (FLICLIB3S) pj_flic_play_until;
	#pragma aux (FLICLIB3S) pj_flic_play_next;
	#pragma aux (FLICLIB3S) pj_flic_play_frames;
	#pragma aux (FLICLIB3S) pj_flic_close;
	#pragma aux (FLICLIB3S) pj_flic_open;
	#pragma aux (FLICLIB3S) pj_flic_open_info;
	#pragma aux (FLICLIB3S) pj_flic_file_info;
	#pragma aux (FLICLIB3S) pj_flic_info;
	#pragma aux (FLICLIB3S) pj_flic_rewind;
	#pragma aux (FLICLIB3S) pj_flic_set_speed;
	#pragma aux (FLICLIB3S) pj_animinfo_init;
	#pragma aux (FLICLIB3S) pj_flic_complete_filename;
	#pragma aux (FLICLIB3S) pj_flic_create;
	#pragma aux (FLICLIB3S) pj_flic_write_first;
	#pragma aux (FLICLIB3S) pj_flic_write_next;
	#pragma aux (FLICLIB3S) pj_flic_write_finish;
#endif
#endif /* PJPROTOS_H */
