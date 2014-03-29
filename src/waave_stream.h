#ifndef WAAVE_STREAM_H
#define WAAVE_STREAM_H

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
#include "config_ffmpeg.h"

#include "streaming_object.h"
#include "sync_object.h"
#include "packet_feeder.h"
#include "audio_decoder.h"
#include "video_decoder.h"


/* the stream type */
#define WV_STREAM_TYPE_NONE 0
#define WV_STREAM_TYPE_AUDIO 1
#define WV_STREAM_TYPE_VIDEO 2
#define WV_STREAM_TYPE_AUDIOVIDEO 3

/* the play method */
#define WV_NEUTRAL_PLAY 0
#define WV_LOOPING_PLAY 1

/* the seek method */
#define WV_SEEK_BACKWARD -1
#define WV_SEEK_FORWARD 1

/* the signal eof type */
//no signal
#define WV_NO_EOF_SIGNAL 0

//push a WV_EOF_EVENT when eof
#define WV_EVENT_EOF_SIGNAL 1

//launch user function when eof
#define WV_USER_EOF_SIGNAL 2



/* to signal eof */
struct WVStream;
typedef  int (*WVEOFSignalCall)(struct WVStream* stream, void* param);

/* the main waave struct */
typedef struct WVStream{
  
  /* the stream type : audio, video, audiovideo */
  int type;

  /* the ffmpeg streams */
  AVFormatContext* formatCtx;
  int audioStreamIdx;
  int videoStreamIdx;

  /* the WAAVE objects */
  WVStreamingObject* streamObj;  //how stream the video
  WVSyncObject* syncObj;         //how sync audio/video
  int userSyncFlag;              //say the the user give the sync object 

  /* the audio instance */
  AVCodecContext* audioCodecCtx;
  AVCodec* audioCodec;
  WVQueueHandle audioQueueHdl;        //the packet feeder queue
  WVAudioStreamHandle audioStreamHdl; //the audio decoder handle
  

  /* the video instance */
  AVCodecContext* videoCodecCtx;
  AVCodec* videoCodec;
  WVQueueHandle videoQueueHdl;        //the packet feeder queue
  WVVideoStreamHandle videoStreamHdl; //the video decoder handle
  
  
  /* volume */
  double volume;
  int volumeDBValue;
  double volumeDBPitche;

  /* seek info */
  /* this avoid doing the same seek two times */
  int lastSeekModIdx;
  uint32_t lastSeekTargetClock;

  
  /* stream flags */
  int loopingFlag;   //at stream end continue playing ? WV_BLOCKING_STREAM or WV_LOOPING_STREAM
  int seekFlag; //at seek, preserve pause or restart playing ? WV_PLAYING_SEEK or WV_BLOCKING_SEEK
  int playFlag; //when play is useless, go to the beginning ? WV_NEUTRAL_PLAY or WV_LOOPING_PLAY

  /* eof signal */
  int eofSignalType;
  void* eofSignalParam;
  WVEOFSignalCall eofSignalCall;

}WVStream;

#endif
