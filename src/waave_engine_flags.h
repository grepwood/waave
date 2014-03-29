#ifndef WAAVE_ENGINE_FLAGS_H
#define WAAVE_ENGINE_FLAGS_H

/*
 *  waave, a modular audio/video engine
 * 
 *  Copyright (C) 2012  Baptiste Pellegrin
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "common.h"
#include "config_sdl.h"


/*********************/
/* THE PACKET FEEDER */
/*********************/

//The maximum number of pkt in a queue
//the feeder may put more packet if needed 
//to avoid locking packetQueueGet
#define WV_PACKET_FEEDER_MAX_QUEUE_SIZE 6  

//the maximum number of simultaneous context 
#define WV_PACKET_FEEDER_MAX_CONTEXT 10


/*********************/
/* THE AUDIO DECODER */
/*********************/

/* the audio fmt */
/*!!!  can't be modified actually !!!*/
#define WV_DECODER_SAMPLE_RATE 44100
#define WV_DECODER_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define WV_DECODER_CHANNELS 2

/* the size in byte of the wanted audio buffer */
/* small values increase response */
/* high values increase stability */ 
#define WV_WANTED_AUDIO_BLOCK_SIZE 4096

/* when we have a volume near 1.0 we don't apply it */
/* it will be faster to play */
#define WV_VOLUME_SKIP_LOW_THRESHOLD 0.9
#define WV_VOLUME_SKIP_HIGH_THRESHOLD 1.1


/* audio decoder use a mean filter */
/* to thread audio callback irregularity */
/* this is the size of the filter in ms */
#define WV_CALLBACK_DENOISE_FILTER_SIZE 200;

/* the maximum number of simultaneous loaded audio streams */
#define WV_AUDIO_DECODER_MAX_STREAMS 30



/*********************/
/* THE VIDEO DECODER */
/*********************/

//this is the size we give to the decode function 
//so this is the maximum size we can get in one decode
//ideally this is a size where we can decode all the packets
//but in reallity a pkt can be as big the encoder want

//!!! need to be >= than AVCODEC_MAX_AUDIO_FRAME_SIZE
#define WV_DECODE_TARGET_SIZE AVCODEC_MAX_AUDIO_FRAME_SIZE
/* we use stereo s16 audio */ 
#define WV_DECODE_TARGET_SAMPLES AVCODEC_MAX_AUDIO_FRAME_SIZE/4


/* the default WV_REFRESH_EVENT value */
#if SDL_VERSION_ATLEAST(2,0,0)

extern int dynamic_wv_refresh_event;  //dynamic, see waave_engine_flags.c
#define WV_REFRESH_EVENT dynamic_wv_refresh_event

#else

#define WV_REFRESH_EVENT SDL_NUMEVENTS - 1

#endif

/* the decoder store refresh durations for A/V sync */
/* and filter it with a median filter */
#define WV_REFRESH_DURATION_LIST_SIZE 3
#define WV_REFRESH_DURATION_MED_POS 1 
//usually (size-1)/2

/* the ffmpeg filter used to scale video frames */
#define WV_VIDEO_DECODER_SCALE_FILTER SWS_BICUBIC

/* the maximum number of simultaneous loaded video streams */
#define WV_VIDEO_DECODER_MAX_STREAMS 30

/* the sdl granularity */
#define WV_TIMER_GRANULARITY 10


/**************/
/* SIGNAL EOF */
/**************/
#if SDL_VERSION_ATLEAST(2,0,0)

extern int dynamic_wv_eof_event;  //dynamic, see waave_engine_flags.c
#define WV_EOF_EVENT dynamic_wv_eof_event

#else

#define WV_EOF_EVENT SDL_NUMEVENTS - 2

#endif


/**********/
/* VOLUME */
/**********/
#define WAAVE_DEFAULT_VOLUME_DB_PITCHE 1.5





/*********************************/
/* ||||||||||||||||||||||||||||| */
/*  initialize the dynamic flags */
/* ||||||||||||||||||||||||||||| */
/*********************************/
#if SDL_VERSION_ATLEAST(2,0,0)
void initWaaveEngineFlags(void);
#endif




#endif
