//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//
//  Audio.cpp
//    contributed by Kumaran Santhanam

/*=========================================================================
| CONSTANTS
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/
#include <stdio.h>
#include <string.h>

#include "Audio.h"
#include "Doc.h"


/*=========================================================================
| TYPES
 ========================================================================*/
typedef unsigned char   byte;
typedef unsigned short  two_bytes;
typedef unsigned int    four_bytes;


/*=========================================================================
| TYPES
 ========================================================================*/
enum AudioFileType
{
    AudioFile_UNKNOWN,
    AudioFile_WAV
};


/*=========================================================================
| TYPES
 ========================================================================*/
struct WaveHeader
{
    byte           riff_id[4];
    four_bytes     riff_size;
    byte           wave_id[4];
    byte           format_id[4];
    four_bytes     format_size;
    two_bytes      format_tag;
    two_bytes      num_channels;
    four_bytes     num_samples_per_sec;
    four_bytes     num_avg_bytes_per_sec;
    two_bytes      num_block_align;
    two_bytes      bits_per_sample;
    byte           data_id[4];
    four_bytes     num_data_bytes;
};

#define WAVE_FORMAT_PCM   1


/*=========================================================================
| audioFileType
|
|--------------------------------------------------------------------------
| Determine the audio file type
|
|--------------------------------------------------------------------------
| ARGUMENTS
|     1. URL string
|     2. File handle
|
| RETURNS
|     AudioFileType
|
|--------------------------------------------------------------------------
| REVISION HISTORY:
| Rev     Date      Who         Description
| 0.8     11Nov98   kumaran     Created
 ========================================================================*/
// FILE * is included in case this function is to be updated to
// peek at the file header.  - ks 11Nov98
static AudioFileType audioFileType(const char *url, FILE *)
{
    const char *suffix = strrchr(url, '.');
    if (suffix) ++suffix;

    if (strcmp (suffix,"wav") == 0 ||
        strcmp (suffix,"WAV") == 0)
        return AudioFile_WAV;

    else
        return AudioFile_UNKNOWN;
}


/*=========================================================================
| PUBLIC METHODS
 ========================================================================*/

/*=========================================================================
| Audio::Audio
| Audio::~Audio
|
|--------------------------------------------------------------------------
| CONSTRUCTOR
| DESTRUCTOR
|
|--------------------------------------------------------------------------
| ARGUMENTS
|     1. URL string
|     2. Doc object
|
| RETURNS
|     None
|
|--------------------------------------------------------------------------
| REVISION HISTORY:
| Rev     Date      Who         Description
| 0.8     11Nov98   kumaran     Created
 ========================================================================*/
Audio::Audio(const char *url, Doc *relative)
    : _doc(0),
      _encoding(AUDIO_LINEAR),
      _channels(0),
      _bits_per_sample(0),
      _samples_per_sec(0),
      _sample_blocksize(0),
      _num_samples(0),
      _samples(0)
{
    setURL (url, relative);
}


Audio::~Audio()
{
    delete _doc;
    delete _samples;
}


/*=========================================================================
| Audio::setURL
|
|--------------------------------------------------------------------------
| Set the URL of the audio file and read it from the document object.
|
|--------------------------------------------------------------------------
| ARGUMENTS
|     1. URL string
|     2. Doc object
|
| RETURNS
|     True if the URL was read, false if it was not
|
|--------------------------------------------------------------------------
| REVISION HISTORY:
| Rev     Date      Who         Description
| 0.8     11Nov98   kumaran     Created
 ========================================================================*/
bool Audio::setURL(const char *url, Doc *relative)
{
    if (url == 0)
        return false;

    delete _doc;
    _doc = new Doc (url, relative);
    FILE *fp = _doc->fopen ("rb");

    bool success = false;
    if (fp)
    {
        switch (audioFileType (url, fp))
        {
          case AudioFile_WAV:
            success = wavread (fp);
            break;

          default:
            fprintf (stderr,
                     "Error: unrecognized audio file format (%s).\n", url);

            // Suppress the error message below
            success = true;
            break;
        }

        if (success == false)
            fprintf (stderr,
                     "Error: unable to read audio file (%s).\n", url);
        
        _doc->fclose ();
    }

    else
        fprintf (stderr,
                 "Error: unable to find audio file (%s).\n", url);

    return (_num_samples > 0);
}


/*=========================================================================
| Audio::tryURLs
|
|--------------------------------------------------------------------------
| Try a list of URLs
|
|--------------------------------------------------------------------------
| ARGUMENTS
|     1. Number of URLs to try
|     2. List of URLs
|     3. Document object
|
| RETURNS
|     True if one of the URLs succeeded, false if they all failed
|
|--------------------------------------------------------------------------
| REVISION HISTORY:
| Rev     Date      Who         Description
| 0.8     11Nov98   kumaran     Created
 ========================================================================*/
bool Audio::tryURLs (int nUrls, const char * const *urls, Doc *relative)
{
    int i;

    for (i = 0; i < nUrls; ++i)
        if (setURL (urls[i], relative))
            return true;

    return false;
}


/*=========================================================================
| Audio::url
|
|--------------------------------------------------------------------------
| Return the url of this clip
|
|--------------------------------------------------------------------------
| ARGUMENTS
|     None
|
| RETURNS
|     URL if one exists
|
|--------------------------------------------------------------------------
| REVISION HISTORY:
| Rev     Date      Who         Description
| 0.8     11Nov98   kumaran     Created
 ========================================================================*/
const char *Audio::url() const
{
    return (_doc ? _doc->url() : 0);
}


/*=========================================================================
| PRIVATE METHODS
 ========================================================================*/

/*=========================================================================
| Audio::wavread
|
|--------------------------------------------------------------------------
| Read a WAV file
|
|--------------------------------------------------------------------------
| ARGUMENTS
|     1. File handle
|
| RETURNS
|     True if the read succeeded, false if not
|
|--------------------------------------------------------------------------
| REVISION HISTORY:
| Rev     Date      Who         Description
| 0.8     11Nov98   kumaran     Created
 ========================================================================*/
bool Audio::wavread (FILE *fp)
{
    WaveHeader wave_header;

    fread (&wave_header, sizeof(WaveHeader), 1, fp);
    rewind (fp);

    // Do all sorts of sanity checks
    if (strncmp ((const char*)wave_header.riff_id, "RIFF", 4) != 0)
        return false;

    if (strncmp ((const char*)wave_header.wave_id, "WAVE", 4) != 0)
        return false;

    if (strncmp ((const char*)wave_header.format_id, "fmt ", 4) != 0)
        return false;

    if (strncmp ((const char*)wave_header.data_id, "data", 4) != 0)
        return false;

    if (wave_header.format_tag != WAVE_FORMAT_PCM)
        return false;


    // Allocate the memory required
    delete _samples;
    _samples = new unsigned char[wave_header.num_data_bytes];
    if (_samples == 0)
        return false;

    // Now, we are ready to read the data
    fseek (fp, sizeof(WaveHeader), SEEK_SET);
    int bytes_read = (int)fread (_samples, 1, wave_header.num_data_bytes, fp);

    _encoding         = AUDIO_LINEAR;
    _channels         = wave_header.num_channels;
    _bits_per_sample  = wave_header.bits_per_sample;
    _samples_per_sec  = wave_header.num_samples_per_sec;
    _sample_blocksize = wave_header.num_block_align;
    _num_samples      = bytes_read / _sample_blocksize;


    return true;
}
