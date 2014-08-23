//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  sound.c
//    contributed by Kumaran Santhanam
//
//  Play sound files without tying up system resources.

#include "config.h"
#include "sound.h"

#if HAVE_SOUND

#include <sys/soundcard.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define AUDIO_DEVICE           "/dev/dsp"
#define MAX_AUDIO_BUFFERSIZE   65536

#endif /* HAVE_SOUND */


int openSound (int channels,
               int bits_per_sample,
               int speed)
{
    int  audio_fd = -1;

#if HAVE_SOUND
    int  format = AFMT_U8;
    int  stereo = channels -1;
    int  err;
    int  o_format;
    int  o_stereo;
    int  o_speed;

    audio_buf_info  info;
    int             fragment_size;
    int             ssss;
    int             frag_arg;

    if (bits_per_sample == 8)
        format = AFMT_U8;
    else if (bits_per_sample == 16)
        format = AFMT_S16_LE;
    else  {
        fprintf (stderr,
                 "Error: %d bits per sample not supported.\n",
                 bits_per_sample);
        return -1;
    }


    // Silently fail if we can't open the device.  This is because
    // there may be multiple audio clips playing, and the behavior
    // is to only play as many as the system allows.
    if ((audio_fd = open (AUDIO_DEVICE, O_WRONLY, 0)) == -1)
        return -1;


    err = ioctl (audio_fd, SNDCTL_DSP_RESET);

    o_format = format;
    o_stereo = stereo;
    o_speed  = speed;
    if (!err) err = ioctl (audio_fd, SNDCTL_DSP_SETFMT, &format);
    if (!err) err = ioctl (audio_fd, SNDCTL_DSP_STEREO, &stereo);
    if (!err) err = ioctl (audio_fd, SNDCTL_DSP_SPEED,  &speed);
    if (!err) err = ioctl (audio_fd, SNDCTL_DSP_NONBLOCK);

    if (err)
    {
        fprintf (stderr, "Error: couldn't set audio ioctl()\n");
        close (audio_fd);
        return -1;
    }

    if (format != o_format || stereo != o_stereo || speed != o_speed)
    {
        fprintf (stderr,
                 "Error: no audio support for %d bit %dHz %s samples.\n",
                 o_format, o_speed, o_stereo ? "STEREO" : "MONO");
        close (audio_fd);
        return -1;
    }


    // Adjust the audio buffer size and fragments
    ioctl (audio_fd, SNDCTL_DSP_GETOSPACE, &info);

    // Calculate about a 1/4 second fragment.  Since
    // the output routine keeps the buffer filled with
    // about 2 fragments, this will keep the latency
    // down to 1/2 second.
    fragment_size = ((bits_per_sample == 8) ? 1 : 2)
        * (stereo ? 2 : 1)
        * (speed / 4);

    if (fragment_size > MAX_AUDIO_BUFFERSIZE)
        fragment_size = MAX_AUDIO_BUFFERSIZE;

    // The ioctl argument is:
    //     0x0002ssss
    // Where the 0x0002 signifies 2 buffer fragments
    // And 2**ssss signifies the fragment size.
    for (ssss = 0; ssss < 65536; ++ssss)
        if ((1 << ssss) >= fragment_size)
            break;

    frag_arg = 0x00020000 | ssss;
    ioctl (audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag_arg);


#endif /* HAVE_SOUND */

    return audio_fd;
}



int outputSoundChunk (int num_sample_bytes,
                      const unsigned char *samples,
                      int bits_per_sample,
                      int byte_index,
                      int loop,
                      double intensity,
                      int fd)
{
#if HAVE_SOUND
    unsigned char   buffer[MAX_AUDIO_BUFFERSIZE];
    int             i, j;
    int             bytes_written;
    audio_buf_info  info;
    int             buffersize;

    // Check for all invalid conditions
    if (num_sample_bytes == 0 || samples == 0    ||
        fd < 0                ||
        byte_index < 0        || byte_index >= num_sample_bytes)
        return byte_index;

    // If there is already enough data in the sound buffer,
    // just return.  Only fill the sound buffer with more
    // data if there is less than one fragment of data.
    ioctl (fd, SNDCTL_DSP_GETOSPACE, &info);
    if (info.fragstotal*info.fragsize - info.bytes > info.fragsize)
        return byte_index;

    buffersize = info.fragsize;


    // Scale the intensity appropriately using the CPU.
    // We don't want to use the mixer, because it is
    // best left under user control.  This calculation
    // is simple and shouldn't take too much of the
    // system's resources.
    for (i = 0, j = byte_index; i < buffersize && j < num_sample_bytes; ++i)
    {
        // Implement real intensity functionality here,
        // keeping in mind that 16-bit samples are little-endian
        // and cannot be cast as unsigned short on big-endian
        // machines.
        // NOT YET IMPLEMENTED
        switch (bits_per_sample)
        {
          case 8:
            if (intensity > 0.0)
                buffer[i] = samples[j];
            else
                buffer[i] = 0;
            break;
          case 16:
            if (intensity > 0.0)
                buffer[i] = samples[j];
            else
                buffer[i] = 0;
            break;
        }

        ++j;

        // If we're looping, reset the index to the beginning of
        // the sample buffer.
        if (j >= num_sample_bytes && loop != 0)
            j -= num_sample_bytes;
    }

    bytes_written  = write (fd, buffer, i);
    byte_index    += bytes_written;

    if (loop)
        byte_index %= num_sample_bytes;

    return byte_index;
#else
    return -1;
#endif /* HAVE_SOUND */
}



int closeSound (int audio_fd)
{
#if HAVE_SOUND
    ioctl (audio_fd, SNDCTL_DSP_RESET);
    close (audio_fd);
#endif /* HAVE_SOUND */
    return -1;
}

