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

#include "waave_stream.h"
#include "waave_engine_flags.h"
#include "waave_ffmpeg.h"
#include "sync_object.h"
#include "streaming_object.h"
#include "packet_feeder.h"
#include "audio_decoder.h"
#include "video_decoder.h"
#include "audio_video_sync.h"
#include "clock_video_sync.h"
#include "eof_signal.h"


#define WAAVE_INIT_NONE 0
#define WAAVE_INIT_AUDIO 1
#define WAAVE_INIT_VIDEO 2

static int packetFeederStartedFlag = 0;
static int audioDecoderStartedFlag = 0;
static int videoDecoderStartedFlag = 0;


int WV_waaveInit(int flag)
{
  /* launch ffmpeg */
  av_register_all();
  
  /* init waave engine flags */
  #if SDL_VERSION_ATLEAST(2,0,0)
  initWaaveEngineFlags();
  #endif

  /* launch packet feeder */
  if(flag & WAAVE_INIT_AUDIO || flag & WAAVE_INIT_VIDEO){
    if(!packetFeederStartedFlag){
      WV_initPacketFeeder();
      packetFeederStartedFlag = 1;
    }
  }

  /* launch audio decoder */
  if(flag & WAAVE_INIT_AUDIO){
    if(!audioDecoderStartedFlag){
      WV_initAudioDecoder();
      audioDecoderStartedFlag = 1;
    }
  }

  /* launch video decoder */
  if(flag & WAAVE_INIT_VIDEO){
    if(!videoDecoderStartedFlag){
      WV_initVideoDecoder();
      videoDecoderStartedFlag = 1;
    }
  }

  return 0;
}


int WV_waaveClose(void)
{
  /* close video decoder */
  if(videoDecoderStartedFlag)
    WV_videoDecoderShutdown();

  /* close audio decoder */
  if(audioDecoderStartedFlag)
    WV_audioDecoderShutdown();

  /* close packet feeder */
  if(packetFeederStartedFlag)
    WV_packetFeederShutdown();

  /* close ffmpeg */
  //seems there are no function to close av_register_all


  return 0; 

}



WVStream* WV_closeStream(WVStream* stream)
{
  /* close video stream */
  if(stream->videoStreamHdl){
    WV_delVideoStream(stream->videoStreamHdl);
    if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
      WV_stopAudioMasterSync(stream->audioStreamHdl);
    }
    stream->videoStreamHdl = NULL;
  }
  
  /* close audio stream */
  if(stream->audioStreamHdl){
    WV_delAudioStream(stream->audioStreamHdl);
    stream->audioStreamHdl = NULL;
  }


  /* close queues */
  if(stream->audioQueueHdl || stream->videoQueueHdl){
    WV_delFeederContext(stream->formatCtx);
    stream->audioQueueHdl = NULL;
    stream->videoQueueHdl = NULL;
  }

  
  /* close codec */
  if(stream->videoCodecCtx){
    avcodec_close(stream->videoCodecCtx);
    stream->videoCodecCtx = NULL;
  }

  if(stream->audioCodecCtx){
    avcodec_close(stream->audioCodecCtx);
    stream->audioCodecCtx = NULL;
  }

  /* close sync object */
  /* if the user set it, he free it */
  if(!stream->userSyncFlag){
    if(stream->type == WV_STREAM_TYPE_VIDEO){
      WV_freeCVSyncObj(stream->syncObj);
    }
    else if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
      WV_freeAVSyncObj(stream->syncObj);
    }
  }

  
  /* close stream */
  stream->audioStreamIdx = -1;
  stream->videoStreamIdx = -1;


  /* close format */
  if(stream->formatCtx){
    avformat_close_input(&stream->formatCtx); //set formatCtx to NULL
  }


  /* close WVStream */
  free(stream);

  return NULL;
}



WVStream* WV_getStream(const char* filename)
{
  /* alloc the struct and set default value */
  WVStream* newStream = (WVStream*)malloc(sizeof(WVStream));

  newStream->type = WV_STREAM_TYPE_NONE;
  
  newStream->formatCtx = NULL;
  newStream->audioStreamIdx = -1;
  newStream->videoStreamIdx = -1;
  
  newStream->streamObj = NULL;
  newStream->syncObj = NULL;
  newStream->userSyncFlag = 0;
  
  newStream->audioCodecCtx = NULL;
  newStream->audioCodec = NULL;
  newStream->audioQueueHdl = NULL;
  newStream->audioStreamHdl = NULL;
  
  newStream->videoCodecCtx = NULL;
  newStream->videoCodec = NULL;
  newStream->videoQueueHdl = NULL;
  newStream->videoStreamHdl = NULL;
  
  newStream->volume = 1.0;
  newStream->volumeDBValue = 0;
  newStream->volumeDBPitche = WAAVE_DEFAULT_VOLUME_DB_PITCHE; 
  
  newStream->lastSeekModIdx = -1;
  newStream->lastSeekTargetClock = UINT32_MAX;

  newStream->loopingFlag = WV_BLOCKING_STREAM;
  newStream->seekFlag = WV_BLOCKING_SEEK;
  newStream->playFlag = WV_NEUTRAL_PLAY;
  
  newStream->eofSignalType = WV_EVENT_EOF_SIGNAL;
  newStream->eofSignalParam = NULL;
  newStream->eofSignalCall = NULL;

  /* open file */
  AVFormatContext* formatCtx = avformat_alloc_context();
  if(formatCtx == NULL)
    return NULL;

  if(avformat_open_input(&formatCtx, filename, 0, NULL) < 0){
    /* error */
    avformat_free_context(formatCtx);
    //close stream is useless 
    return NULL;
  }
  
  newStream->formatCtx = formatCtx;

  /* read stream info */
  if( avformat_find_stream_info(formatCtx, NULL) < 0 ){
    /* error */
    WV_closeStream(newStream);
    return NULL;
  }

  av_dump_format(formatCtx, 0, filename, 0);

  
  /* find streams */
  int audioStreamIdx = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
  int videoStreamIdx = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);


  /* set WVStream type */
  if(audioStreamIdx == AVERROR_STREAM_NOT_FOUND){
    if(videoStreamIdx == AVERROR_STREAM_NOT_FOUND){
      /* error */
      WV_closeStream(newStream);
      return NULL;
    }
    else{
      newStream->type = WV_STREAM_TYPE_VIDEO;
    }
  }
  else{
    if(videoStreamIdx == AVERROR_STREAM_NOT_FOUND){
      newStream->type = WV_STREAM_TYPE_AUDIO;
    }
    else{
      newStream->type = WV_STREAM_TYPE_AUDIOVIDEO;
    }
  }

  /* save stream */
  newStream->audioStreamIdx = audioStreamIdx;
  newStream->videoStreamIdx = videoStreamIdx;


  /* give the AVStream */
  return newStream;
}



int WV_getStreamType(WVStream* stream)
{
  if(!stream)
    return -1;
  
  return stream->type;
}

int WV_getStreamWidth(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* check if we have video */
  int type = stream->type;
  if(type != WV_STREAM_TYPE_VIDEO && type != WV_STREAM_TYPE_AUDIOVIDEO)
    return -1;

  /* get the video stream */
  AVStream* ffmpegStream = stream->formatCtx->streams[stream->videoStreamIdx];

  /* return width */
  return ffmpegStream->codec->width;
}


int WV_getStreamHeight(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* check if we have video */
  int type = stream->type;
  if(type != WV_STREAM_TYPE_VIDEO && type != WV_STREAM_TYPE_AUDIOVIDEO)
    return -1;

  /* get the video stream */
  AVStream* ffmpegStream = stream->formatCtx->streams[stream->videoStreamIdx];

  /* return width */
  return ffmpegStream->codec->height;
}


int WV_disableAudio(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* disable audio */
  stream->audioStreamIdx = -1;

  if(stream->type == WV_STREAM_TYPE_AUDIO){
    stream->type = WV_STREAM_TYPE_NONE;
  }
  else if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    stream->type = WV_STREAM_TYPE_VIDEO;
  }
  
  return 0;
}


int WV_disableVideo(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* disable audio */
  stream->videoStreamIdx = -1;

  if(stream->type == WV_STREAM_TYPE_VIDEO){
    stream->type = WV_STREAM_TYPE_NONE;
  }
  else if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    stream->type = WV_STREAM_TYPE_AUDIO;
  }
  
  return 0;
}


int WV_setStreamingMethod(WVStream* stream, WVStreamingObject* streamObj)
{
  /* check stream and streamObj */
  if(!stream || !streamObj)
    return -1;

  /* check if stream Obj is correctly initialized */
  if(!streamObj->getBuffer || !streamObj->refreshFrame)
    return -1;

  /* save the streaming object */
  stream->streamObj = streamObj;
  return 0;
}


WVStreamingObject* WV_getStreamingMethod(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return NULL;

  /* return the streaming object */
  return stream->streamObj;
}



int WV_setSyncMethod(WVStream* stream, WVSyncObject* syncObj)
{
  /* check stream and syncObj */
  if(!stream || !syncObj)
    return -1;

  /* save the sync object */
  stream->syncObj = syncObj;
  stream->userSyncFlag = 1;
  return 0;
}


WVSyncObject* WV_getSyncMethod(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return NULL;

  /* return the sync object */
  return stream->syncObj;
}


int WV_setEOFMethod(WVStream* stream, int loopingFlag)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* set looping flag */
  stream->loopingFlag = loopingFlag;

  if(stream->syncObj)
    stream->syncObj->loopingFlag = loopingFlag;
  else if(stream->audioStreamHdl)
    setAudioDefaultLoopingFlag(stream->audioStreamHdl, stream->loopingFlag);

  return 0;
}


int WV_setSeekMethod(WVStream* stream, int seekFlag)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* set looping flag */
  stream->seekFlag = seekFlag;

  return 0;
}


int WV_setPlayMethod(WVStream* stream, int playFlag)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* set looping flag */
  stream->playFlag = playFlag;

  return 0;
}


int WV_setEOFSignalParam(WVStream* stream, void* param)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* set looping flag */
  stream->eofSignalParam = param;

  return 0;
}



int WV_setEOFSignalCall(WVStream* stream, WVEOFSignalCall eofCall)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* set looping flag */
  stream->eofSignalType = WV_USER_EOF_SIGNAL;
  stream->eofSignalCall = eofCall;

  return 0;
}


int WV_setVolume(WVStream* stream, double volume)
{
  /* check stream */
  if(!stream)
    return -1;

  /* set volume */
  stream->volume = volume;

  if(stream->audioStreamHdl)
    WV_setAudioVolume(stream->audioStreamHdl, volume);
    
  return 0; 
}


double WV_getVolume(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1.0;

  /* return volume */
  return stream->volume;
}


int WV_setDBVolume(WVStream* stream, int DBVolume)
{
  /* check stream */
  if(!stream)
    return -1;

  /* save db volume */
  stream->volumeDBValue = DBVolume;

  /* compute volume */
  double volumeDBPitche = stream->volumeDBPitche;
  if(volumeDBPitche <= 0)
    return -1;

  double volume = 1.0;
  if(DBVolume >= 0){
    while(DBVolume > 0){
      volume *= volumeDBPitche;
      DBVolume--;
    }
  }
  else{
    while(DBVolume < 0){
      volume /= volumeDBPitche;
      DBVolume++;
    }
  }

  /* save and set volume */
  stream->volume = volume;
  
  if(stream->audioStreamHdl)
    WV_setAudioVolume(stream->audioStreamHdl, volume);

  return 0;
}


double WV_getDBVolume(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* return volume */
  return stream->volumeDBValue;
}



int WV_setVolumeDBPitche(WVStream* stream, double DBPitche)
{
  /* check stream */
  if(!stream)
    return -1;

  /* set DBPitche */
  stream->volumeDBPitche = DBPitche;
  
  /* reset volume */
  WV_setDBVolume(stream, stream->volumeDBValue);

  return 0;
}


double WV_getVolumeDBPitche(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1.0;

  /* set DBPitche */
  return stream->volumeDBPitche;
} 
 


int WV_shiftDBVolume(WVStream* stream, int shift)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* update DBVolume */
  stream->volumeDBValue += shift;

  /* reset volume */
  WV_setDBVolume(stream, stream->volumeDBValue);

  return 0;
}




int WV_loadStream(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* check type */
  if(stream->type == WV_STREAM_TYPE_NONE)
    return -1;

  /* check engine */
  if(stream->type == WV_STREAM_TYPE_AUDIO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    if(!audioDecoderStartedFlag)
      return -1;
  }

  if(stream->type == WV_STREAM_TYPE_VIDEO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    if(!videoDecoderStartedFlag)
      return -1;
  }


  /******************************/
  /* CHECK THE STREAMING OBJECT */
  /******************************/

  /* check if we have streaming method for the video */
  if(stream->type == WV_STREAM_TYPE_VIDEO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    if(!stream->streamObj)
      return -1;
  }




  /**********************/
  /* TRY TO OPEN CODECS */
  /**********************/
  
  /*********/
  /* audio */
  /*********/
  if(stream->type == WV_STREAM_TYPE_AUDIO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){

    /* find corresponding codec */
    AVCodecContext* audioCodecCtx = stream->formatCtx->streams[stream->audioStreamIdx]->codec;
    AVCodec* audioCodec = avcodec_find_decoder(audioCodecCtx->codec_id);
    if(!audioCodec)
      return -1;
    
    /* open codec */
    if( avcodec_open2(audioCodecCtx, audioCodec, NULL) < 0 ){
      free(audioCodec);  //to be verified 
      return -1;
    }

    /* save codec */
    stream->audioCodecCtx = audioCodecCtx;
    stream->audioCodec = audioCodec;
  }
  
  /*********/
  /* video */
  /*********/
  if(stream->type == WV_STREAM_TYPE_VIDEO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){

    /* find corresponding codec */
    AVCodecContext* videoCodecCtx = stream->formatCtx->streams[stream->videoStreamIdx]->codec;
    AVCodec* videoCodec = avcodec_find_decoder(videoCodecCtx->codec_id);
    if(!videoCodec)
      return -1;
    
    /* open codec */
    if( avcodec_open2(videoCodecCtx, videoCodec, NULL) < 0 ){
      free(videoCodec);   //to be verified 
      return -1;
    }

    /* save codec */
    stream->videoCodecCtx = videoCodecCtx;
    stream->videoCodec = videoCodec;
  }


  

  /*******************/
  /* LOAD THE QUEUES */
  /*******************/
  
  /* audio */
  if(stream->type == WV_STREAM_TYPE_AUDIO){
    if(WV_buildFeederContext(stream->formatCtx, 1) < 0)  //just one audio stream
      return -1;
    
    stream->audioQueueHdl = WV_getStreamQueue(stream->audioStreamIdx);
    WV_addFeederContext();
  }

  /* video */
  else if(stream->type == WV_STREAM_TYPE_VIDEO){
    if(WV_buildFeederContext(stream->formatCtx, 1) < 0)  //just one video stream
      return -1;
    
    stream->videoQueueHdl = WV_getStreamQueue(stream->videoStreamIdx);
    WV_addFeederContext();
  }

  /* audio video */
  else if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    if(WV_buildFeederContext(stream->formatCtx, 2) < 0)  //audio and video stream
      return -1;
    
    stream->audioQueueHdl = WV_getStreamQueue(stream->audioStreamIdx);
    stream->videoQueueHdl = WV_getStreamQueue(stream->videoStreamIdx);
    WV_addFeederContext();
  }

  
  /**************/
  /* LOAD AUDIO */
  /**************/
  if(stream->type == WV_STREAM_TYPE_AUDIO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    stream->audioStreamHdl = WV_addAudioStream(stream->audioQueueHdl,	\
					       stream->audioCodecCtx,		\
					       stream->formatCtx->streams[stream->audioStreamIdx]->time_base, \
					       stream->volume);

  }


  /*********************/
  /* CHECK SYNC OBJECT */
  /*********************/

  /* if no sync object is given */
  /* put the default sync object */
  if(!stream->syncObj){

    /* video only */
    if(stream->type == WV_STREAM_TYPE_VIDEO){
      stream->syncObj = WV_getCVSyncObj();  //hardware clock to sync video
    }

    /* audio with video */
    else if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
      stream->syncObj =  WV_getAVSyncObj(stream->audioStreamHdl);
    }

  }


  /* if sync is given */
  /* overwrite the default methods */
  /* check if the the other methods are given */
  else{
    
    /* audio only */
    /* the user fill the slave part of the obj */
    /* the signalStateChange function */
    /* so fill the other with standard methods */
    if(stream->type == WV_STREAM_TYPE_AUDIO){
      WVSyncObject* syncObj = stream->syncObj;

      /* check user */
      if(!syncObj->signalStateChange)
	return -1;

      /* overrite other */
      syncObj->getRefClock = &WV_AVSync_getRefClock;
      syncObj->play = &WV_AVSync_play;
      syncObj->pause = &WV_AVSync_pause;
      syncObj->seek = &WV_AVSync_seek;
      syncObj->wvStdPrivate = (void*)stream->audioStreamHdl;
    }

    /* video only */
    /* the user fill the master part of the obj */
    /* the getRefClock, play, pause, seek */
    /* so fill the others with standard methods */
    else if(stream->type == WV_STREAM_TYPE_VIDEO){
      WVSyncObject* syncObj = stream->syncObj;

      /* check user */
      /* only getRefClock and play are mandatory */
      if(!syncObj->getRefClock || !syncObj->play)
	return -1;
      
      /* overwrite others */
      syncObj->signalStateChange = &WV_AVSync_signalStateChange;
    }

    /* error on other case */
    else{
      return -1;
    }
  }


  /* !!!!!!!!!!! !!!!!!!!! */
  /* put the looping flag  */
  /* !!!!!!!!!!!!!!!!!!!!! */
  if(stream->syncObj)
    stream->syncObj->loopingFlag = stream->loopingFlag;

  
    
  /*****************************/
  /* PUT THE AUDIO SYNC OBJECT */
  /*****************************/
  
  /* audio video */
  /* always have a sync object */
  if(stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    WV_setAudioMasterSync(stream->audioStreamHdl, stream->syncObj);
  }

  /* audio only */
  /* put the syncObject if we have one */
  /* else set the default looping flag */
  else if(stream->type == WV_STREAM_TYPE_AUDIO){
    if(stream->syncObj){
      WV_setAudioMasterSync(stream->audioStreamHdl, stream->syncObj);
    }
    else{
      setAudioDefaultLoopingFlag(stream->audioStreamHdl, stream->loopingFlag);
    }
  }

  /************************/
  /* PUT AUDIO EOF SIGNAL */
  /************************/
  /* if we have audio we use it for signal eof */
  if(stream->type == WV_STREAM_TYPE_AUDIO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO)
    WV_setAudioEOFSignalHandle(stream->audioStreamHdl, stream);

    

  /**************/
  /* LOAD VIDEO */
  /**************/
  
  /* video always have a sync and a streaming object */
  if(stream->type == WV_STREAM_TYPE_VIDEO || stream->type == WV_STREAM_TYPE_AUDIOVIDEO){
    stream->videoStreamHdl = WV_addVideoStream(stream->videoQueueHdl,	\
					       stream->videoCodecCtx,	\
					       stream->formatCtx->streams[stream->videoStreamIdx]->time_base, \
					       stream->streamObj,		\
					       stream->syncObj);

  
  }
  
  /************************/
  /* PUT VIDEO EOF SIGNAL */
  /*   (if needed)        */
  /************************/
  if(stream->type == WV_STREAM_TYPE_VIDEO)
    WV_setVideoEOFSignalHandle(stream->videoStreamHdl, stream);
  

  /* loading finished */
  return 0;
}




int WV_rewindStream(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* if the user provide master syncObj */
  /* check if the seek function is defined */
  if(!stream->audioStreamHdl && stream->syncObj && !stream->syncObj->seek)
    return -1;
  
  /* prepare seek */
  if(stream->audioStreamHdl)
    WV_startAudioSeeking(stream->audioStreamHdl, stream->seekFlag);
      
  if(stream->videoStreamHdl)
    WV_startVideoSeeking(stream->videoStreamHdl);

  /* packet feeder seek */
  WV_contextSeek(stream->formatCtx, -1, 0, AVSEEK_FLAG_BACKWARD );
      
  /* master seek (audio or user) */
  if(stream->audioStreamHdl)
    WV_seekAudio(stream->audioStreamHdl);
  else if(stream->syncObj)
    stream->syncObj->seek(stream->syncObj, 0, stream->seekFlag);
  
  /* slave seek */
  if(stream->videoStreamHdl)
    WV_seekVideo(stream->videoStreamHdl);
     
  return 0;
}



int WV_playStream(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* try to play */
  int playRet;
  if(stream->syncObj && stream->syncObj->play)
    playRet = stream->syncObj->play(stream->syncObj);
  else if(stream->audioStreamHdl)
    playRet = WV_playAudio(stream->audioStreamHdl);
  else
    return -1;

  /* check playFlag */
  if(stream->playFlag == WV_LOOPING_PLAY){
    
    /* seek to start if play is useless */
    if(playRet < 0)
      WV_rewindStream(stream);
  }

  return 0;
}



int WV_pauseStream(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;

  /* if the user provide master syncObj */
  /* check if the pause function is defined */
  if(stream->syncObj && !stream->syncObj->pause)
    return -1;

  /* pause */
  if(stream->syncObj)
    stream->syncObj->pause(stream->syncObj);
  else if(stream->audioStreamHdl)
    WV_pauseAudio(stream->audioStreamHdl);
  
  return 0;
}


int WV_stopStream(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return -1;
  
  /* pause stream */
  WV_pauseStream(stream);

  /* seek to beginning */
  /* keep pause */
  /* if the user provide master syncObj */
  /* check if the seek function is defined */
  if(!stream->audioStreamHdl && stream->syncObj && !stream->syncObj->seek)
    return -1;
  
  /* prepare seek */
  if(stream->audioStreamHdl)
    WV_startAudioSeeking(stream->audioStreamHdl, WV_BLOCKING_SEEK);
      
  if(stream->videoStreamHdl)
    WV_startVideoSeeking(stream->videoStreamHdl);

  /* packet feeder seek */
  WV_contextSeek(stream->formatCtx, -1, 0, AVSEEK_FLAG_BACKWARD );
      
  /* master seek (audio or user) */
  if(stream->audioStreamHdl)
    WV_seekAudio(stream->audioStreamHdl);
  else if(stream->syncObj)
    stream->syncObj->seek(stream->syncObj, 0, WV_BLOCKING_SEEK);
  
  /* slave seek */
  if(stream->videoStreamHdl)
    WV_seekVideo(stream->videoStreamHdl);

  /* return */
  return 0;
  
}


int WV_rseekStream(WVStream* stream, uint32_t seekShift, int seekDirection)
{
  /* check stream */
  if(!stream)
    return -1;

  /* if the user provide master syncObj */
  /* check if the seek function is defined */
  if(!stream->audioStreamHdl && stream->syncObj && !stream->syncObj->seek)
    return -1;

  /* get ref clock */
  WVReferenceClock refClock;
  refClock.modIdx = 0;
  
  if(stream->syncObj)
    stream->syncObj->getRefClock(stream->syncObj, &refClock);
  else if(stream->audioStreamHdl)
    refClock = WV_getAudioClock(stream->audioStreamHdl);
  else
    return -1;
  
  /************************/
  /* compute target clock */
  /************************/
  uint32_t targetClock = refClock.clock;
  
  /* check if previous seek was done */
  /* else put clock to previous targetClock */
  if(stream->lastSeekModIdx == refClock.modIdx){
    
    /* the previous seek not done yet, overwrite */
    targetClock = stream->lastSeekTargetClock;
    refClock.modIdx++; //do the modIdx encrease to save the corect value 
  }


  /* apply shift */
  if(seekDirection < 0){
    if(seekShift > targetClock){
      targetClock = 0;
    }
    else{
      targetClock -= seekShift;
    }
  }
  else{
    targetClock += seekShift;
  }

  /* save last seek params */
  stream->lastSeekModIdx = refClock.modIdx;
  stream->lastSeekTargetClock = targetClock;
  

  /* compute clock in AV_TIME_BASE unit */
  uint64_t TBClock = targetClock;
  
  TBClock *= AV_TIME_BASE;
  TBClock /= 1000;

  /* check if we seek after stream end */
  if(TBClock >= stream->formatCtx->duration){
    TBClock = 0;    //go to start
    targetClock = 0;
    /* signal that we reach end */
    WV_signalEOF(stream);
  }
    
  
  /* prepare seek */
  if(stream->audioStreamHdl)
    WV_startAudioSeeking(stream->audioStreamHdl, stream->seekFlag);
      
  if(stream->videoStreamHdl)
    WV_startVideoSeeking(stream->videoStreamHdl);

  /* packet feeder seek */
  if(seekDirection < 0)
    WV_contextSeek(stream->formatCtx, -1, TBClock, AVSEEK_FLAG_BACKWARD);
  else
    WV_contextSeek(stream->formatCtx, -1, TBClock, 0);
      
  /* master seek (audio or user) */
  if(stream->audioStreamHdl)
    WV_seekAudio(stream->audioStreamHdl);
  else if(stream->syncObj)
    stream->syncObj->seek(stream->syncObj, targetClock, stream->seekFlag);
  
  /* slave seek */
  if(stream->videoStreamHdl)
    WV_seekVideo(stream->videoStreamHdl);

  return 0;
}


uint32_t WV_getStreamDuration(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return 0;

  /* get duration */
  uint64_t duration = stream->formatCtx->duration;
  duration *= 1000;
  duration /= AV_TIME_BASE;

  /* return */
  return (uint32_t)duration;
}


uint32_t WV_getStreamClock(WVStream* stream)
{
  /* check stream */
  if(!stream)
    return 0;

  //sync objects always have a getRefClock

  /* get clock */
  WVReferenceClock refClock;
  refClock.modIdx = 0;
  
  if(stream->syncObj)
    stream->syncObj->getRefClock(stream->syncObj, &refClock);
  else if(stream->audioStreamHdl)
    refClock = WV_getAudioClock(stream->audioStreamHdl);
  else
    return -1;

  /* return clock value */
  return refClock.clock;
}

int WV_seekStream(WVStream* stream, uint32_t clock)
{
  /* check stream */
  if(!stream)
    return -1;

  /* if the user provide master syncObj */
  /* check if the seek function is defined */
  if(!stream->audioStreamHdl && stream->syncObj && !stream->syncObj->seek)
    return -1;

  /* get ref clock */
  WVReferenceClock refClock;
  refClock.modIdx = 0;
  
  if(stream->syncObj)
    stream->syncObj->getRefClock(stream->syncObj, &refClock);
  else if(stream->audioStreamHdl)
    refClock = WV_getAudioClock(stream->audioStreamHdl);
  else
    return -1;

  /* get current clock */
  uint32_t currentClock = refClock.clock;
  
  /* compute clock in AV_TIME_BASE unit */
  uint64_t TBClock = clock;
  
  TBClock *= AV_TIME_BASE;
  TBClock /= 1000;

  /* check if we seek after stream end */
  if(TBClock >= stream->formatCtx->duration){
    TBClock = 0;   //go to start
    clock = 0;
    /* signal that we reach end */
    WV_signalEOF(stream);
  }
    
  
  /* prepare seek */
  if(stream->audioStreamHdl)
    WV_startAudioSeeking(stream->audioStreamHdl, stream->seekFlag);
      
  if(stream->videoStreamHdl)
    WV_startVideoSeeking(stream->videoStreamHdl);

  /* packet feeder seek */
  if(clock <= currentClock)
    WV_contextSeek(stream->formatCtx, -1, TBClock, AVSEEK_FLAG_BACKWARD);
  else
    WV_contextSeek(stream->formatCtx, -1, TBClock, 0);
      
  /* master seek (audio or user) */
  if(stream->audioStreamHdl)
    WV_seekAudio(stream->audioStreamHdl);
  else if(stream->syncObj)
    stream->syncObj->seek(stream->syncObj, clock, stream->seekFlag);
  
  /* slave seek */
  if(stream->videoStreamHdl)
    WV_seekVideo(stream->videoStreamHdl);

  return 0;

}
      
