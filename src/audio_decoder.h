#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

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
#include "audio_decoder_stops.h"
#include "audio_decoder_eofs.h"
#include "audio_decoder_mods.h"




/* abstract pointer for the user */
typedef void* WVAudioStreamHandle;



/****************************/
/* INIT                     */
/* first launch the decoder */
/****************************/
int WV_initAudioDecoder(void);


/****************************************/
/* ADD / DEL                            */
/* you can now add audio stream         */
/* !!! at start the stream is paused !!!*/
/****************************************/

/* add a stream and return a stream handle */
WVAudioStreamHandle WV_addAudioStream(WVQueueHandle queueHdl,\
				      AVCodecContext* codec,	\
				      AVRational timeBase,	\
				      double volume);     
				 


/* del a stream */
int WV_delAudioStream(WVAudioStreamHandle streamHdl);


/********************************/
/* AVSync                       */
/* set the A/V synchronisation  */
/********************************/
void WV_setAudioMasterSync(WVAudioStreamHandle streamHdl, WVSyncObject* AVSync);

void WV_stopAudioMasterSync(WVAudioStreamHandle streamHdl);


/************************************/
/* Default looping flag             */
/* if audio master sync is not set  */
/************************************/
void setAudioDefaultLoopingFlag(WVAudioStreamHandle streamHdl, int loopingFlag);


/*****************************/
/* EOF signal                */
/* set the EOF signal handle */
/*****************************/
void WV_setAudioEOFSignalHandle(WVAudioStreamHandle streamHdl, void* eofSignalHandle);

void WV_stopAudioEOFSignalHandle(WVAudioStreamHandle streamHdl);


/****************/
/* PLAY / PAUSE */
/****************/

/* return <0 if the play is useless (the stream is not paused) */
int WV_playAudio(WVAudioStreamHandle streamHdl);

/* pause the stream immediately */
int WV_pauseAudio(WVAudioStreamHandle streamHdl);


/************/
/* SEEKING  */
/************/

/*!!! to seek you need to follow three step !!!*/

/* 1) set if stream will preserve pause (blockingFlag) */
void WV_startAudioSeeking(WVAudioStreamHandle streamHdl, int blockingFlag);

/* 2) send a seek command to the packet feeder */
// it's here you set where seek

/* 3) send a seek command to the audio decoder */
int WV_seekAudio(WVAudioStreamHandle streamHdl);


/**************/
/* GET CLOCK  */
/**************/
WVReferenceClock WV_getAudioClock(WVAudioStreamHandle streamHdl);


/*****************/
/* CHANGE VOLUME */
/*****************/
int WV_setAudioVolume(WVAudioStreamHandle streamHdl, double volume);



/**********/
/* CLOSE  */
/**********/
int WV_audioDecoderShutdown(void);





#endif
