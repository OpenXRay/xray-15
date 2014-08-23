/**********************************************************************
 *<
	FILE: type1.h

	DESCRIPTION:  Adobe Type 1 bezier font file header

	CREATED BY: Tom Hudson

	HISTORY: created 2 November 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __TYPE1_H__

#define __TYPE1_H__

typedef int Errcode;

#define Success 0

/* general errors */

#define Err_nogood		-1 /* something generally went wrong */
#define Err_no_memory	-2 /* no memory available */
#define Err_bad_input	-3 /* bad input data */
#define Err_format		-4 /* general bad data format */
#define Err_no_vram 	-5 /* out of video display memory */
#define Err_no_stack	-6 /* out of stack space */
#define Err_reported	-7 /* still an error but was reported below */
#define Err_unimpl		-8 /* feature unimplemented */
#define Err_overflow	-9 /* data overflow for size of result */
#define Err_not_found	-10 /* object not found */
#define Err_bad_magic	-11 /* bad magic id number on data or record */
#define Err_abort		-12 /* user abort request */
#define Err_timeout 	-13 /* the call waiting timed out */
#define Err_wrong_res	-14 /* Can't deal with object of this dimension */
#define Err_too_big 	-15 /* object too big to handle */
#define Err_version 	-16 /* correct file type but new version */
#define Err_bad_record	-17 /* record magic number is bad */
#define Err_uninit		-18 /* subsystem not initialized */
#define Err_wrong_type	-19 /* object found but not requested type */
#define Err_spline_points -20 /* too many points in spline */
#define Err_widget		-21 /* hardware lock not found */
#define Err_rgb_convert -22 /* only CONVERT program can load rgb image files */
#define Err_pic_unknown -23 /* picture file format unknown */
#define Err_range		-24 /* value out of range */
#define Err_no_message	-25 /* No message except for "context" part */
#define Err_internal_pointer   -26	  /* internal routine detected bad pointer */
#define Err_internal_parameter -27	  /* internal routine detected bad parm value */

/* system errors */
#define SYSERR -50
#define Err_stdio		(SYSERR -0) /* error occurred in stdio routine */
/* end SYSERR */

/* io and file errors */

#define FERR -100
#define Err_no_file 	(FERR -0) /* file not found */
#define Err_no_path 	(FERR -1) /* path not found */
#define Err_no_device	(FERR -2) /* device not found */
#define Err_write		(FERR -3) /* write error */
#define Err_read		(FERR -4) /* read error */
#define Err_seek		(FERR -5) /* seek error */
#define Err_eof 		(FERR -6) /* end of file */
#define Err_in_use		(FERR -7) /* file in use */
#define Err_extant		(FERR -8) /* file exists */
#define Err_create		(FERR -9) /* file creation error */
#define Err_truncated	(FERR -10) /* file data truncated */
#define Err_corrupted	(FERR -11) /* file data corrupted or invalid */
#define Err_no_space	(FERR -12) /* out of space writing to device */
#define Err_disabled	(FERR -13) /* the device,window,file etc. disabled */
#define Err_invalid_id	(FERR -14) /* invalid id value */
#define Err_file_not_open  (FERR -15) /* file is not open */
#define Err_suffix		(FERR-16) /* unrecognized file suffix */
#define Err_too_many_files (FERR-17) /* can't open file cause too many open */
#define Err_access		(FERR-18)	/* Don't have permission to use this file */
#define Err_sys_mem_bad (FERR-19)	/* MS-DOS memory block corrupted */
#define Err_bad_env 	(FERR-20)	/* MS-DOS environment block bad */
#define Err_bad_address (FERR-21)  /* "Invalid memory-block address", */
#define Err_disk_format (FERR-22)	/* Invalid format */
#define Err_file_access (FERR-23)	/* Bad file access code (internal) */
#define Err_data		(FERR-24)	/* Invalid data */
#define Err_no_more_files (FERR-25)  /* No more files */
#define Err_write_protected (FERR-26)  /* Disk write protected. */
#define Err_disk_not_ready (FERR-27) /* No disk in drive */
#define Err_not_dos_disk (FERR-28)	 /* Not an MS-DOS disk */
#define Err_disk_data	(FERR-29)		/* Disk data error */
#define Err_sector		(FERR-30)		/* Disk sector not found */
#define Err_no_paper	(FERR-31)		/* Printer out of paper */
#define Err_general_failure (FERR-32) /* MS-DOS general failure */
#define Err_critical	(FERR-33)		/* critical error */
#define Err_network 	(FERR-34)		/* General network failure */
#define Err_file_share	(FERR-35)	/* file sharing error */
#define Err_file_lock	(FERR-36)		/* file lock error */
#define Err_disk_change (FERR-37)	/* invalid disk  change */
#define Err_no_remote	(FERR-38)		/* remote computer not listening */
#define Err_network_busy (FERR-39)	/* Network busy */
#define Err_share_pause (FERR-40)	/* Sharing temporarily paused */
#define Err_redirect_pause (FERR-41) /* File/printer redirection paused */
#define Err_directory_entry (FERR-42) /* Can't create directory entry */
#define Err_dir_too_long	(FERR-43) /* Directory name too big for ms-dos */
#define Err_dir_name_err	(FERR-44) /* Directory name malformed */
#define Err_file_name_err	(FERR-45) /* FIle name malformed (extra .?) */
#define Err_no_temp_devs (FERR-46) /* Temp files path doen't have any devs. */
#define Err_macrosync	 (FERR-47) /* Input macro out of sync */
#define Err_no_record	(FERR-48)	 /* Data record not found */
#define Err_end_of_record	(FERR-49)	 /* End of data record */
#define Err_no_temp_space	(FERR-50) /* out of space writing to temp device */

typedef struct type1_box
	{
	int xmin,ymin,xmax,ymax;
	} Type1_box;

#define BYTE_MAX 256

#define NCdefs  300                   /* Number of Charstring definitions */


class Type1_font
/* This is all the info on the font that is the same regardless of the
 * size we render it at. */
	{
	public:
		char fullname[256];
		int minx,miny,maxx,maxy;
		unsigned char **letter_defs;	 	 /* Character definitions */
		char **letter_names;				 /* Character names. */
		int letter_count;					 /* Number of letters. */
		unsigned char *ascii_defs[BYTE_MAX]; /* Character defs in ASCII order */
		int sub_count;                    	 /* Number of subroutines */
		unsigned char **subrs;         		 /* Subroutine definitions */
		char **encoding;              		 /* Active mapping vector. */
		int  encoding_count;				 /* Number of entries in encoding */
		Type1_box letter_bounds[BYTE_MAX];	 /* Bounds of letters (unscaled) */
		Type1_box font_bounds;				 /* Unscaled bounds of font. */
		int letter_width[BYTE_MAX];			 /* Width of letters (unscaled) */
		int font_widest;					 /* Width of widest (unscaled) */
		Type1_font() {
			fullname[0]=0;
			minx=miny=maxx=maxy=0;
			letter_defs = NULL;
			letter_names = NULL;
			letter_count = 0;
			sub_count = 0;
			subrs = NULL;
			encoding = NULL;
			encoding_count = 0;
			font_widest = 0;
			Type1_box empty = {0,0,0,0};
			font_bounds = empty;
			for(int i = 0; i < BYTE_MAX; ++i) {
				ascii_defs[i] = NULL;
				letter_bounds[i] = empty;
				letter_width[i] = 0;
				}
			}
	};

typedef struct type1_output
	{
	/* This guy tells the interpreter where to send drawing commands. */
	Errcode (*letter_open)(struct type1_output *fo);
	Errcode (*letter_close)(struct type1_output *fo);
	Errcode (*shape_open)(struct type1_output *fo, double x, double y);
	Errcode (*shape_close)(struct type1_output *fo);
	Errcode (*shape_point)(struct type1_output *fo, double x, double y);
	void *data;		/* Hang data specific implementations of letter_open, etc.
					 * might want here. */
	} Type1_output;

#endif // __TYPE1_H__
