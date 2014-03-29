#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "common.h"
#include "config_sdl.h"
#include "config_ffmpeg.h"


#include "waave_engine_flags.h"
#include "waave_ffmpeg.h"
#include "packet_feeder.h"
#include "sync_object.h"
#include "streaming_object.h"


/* abstract pointer for the user */
typedef void* WVVideoStreamHandle;


/********************/
/* THE EVENT SYSTEM */
/********************/

/* !!! thread the WV_REFRESH_EVENT !!! */
/* you need to thread the event sended by the video system */
/* by calling the refresh function */
/* when you receive WV_REFRESH_EVENT */

void WV_refreshVideoFrame(SDL_Event* refreshEvent);




/****************************/
/* INIT                     */
/* first launch the decoder */
/****************************/
int WV_initVideoDecoder(void);



/****************************************/
/* ADD / DEL                            */
/* you can now add audio stream         */
/* !!! at start the stream is paused !!!*/
/****************************************/

/* add a stream and return a stream handle */
WVVideoStreamHandle WV_addVideoStream(WVQueueHandle queueHdl,  \
				      AVCodecContext* codec,   \
				      AVRational timeBase,		\
				      WVStreamingObject* streamObj,	\
				      WVSyncObject* VSync);

/* del a stream */
int WV_delVideoStream(WVVideoStreamHandle streamHdl);



/*****************************/
/* EOF signal                */
/* set the EOF signal handle */
/*****************************/
void WV_setVideoEOFSignalHandle(WVVideoStreamHandle streamHdl, void* eofSignalHandle);

void WV_stopVideoEOFSignalHandle(WVVideoStreamHandle streamHdl);




/************/
/* SEEKING  */
/************/

/*!!! to seek you need to follow three step !!!*/

/* 1) prepare seeking  */
void WV_startVideoSeeking(WVVideoStreamHandle streamHdl);

/* 2) send a seek command to the packet feeder */
// it's here you set where seek

/* 3) send a seek command to the audio decoder */
int WV_seekVideo(WVVideoStreamHandle streamHdl);



/**********/
/* CLOSE  */
/**********/
int WV_videoDecoderShutdown(void);




/******************/
/* used by clocks */
/******************/
void WV_videoDecoderSignal(void);

#endif 
