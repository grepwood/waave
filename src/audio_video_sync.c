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

#include "audio_video_sync.h"

#include "common.h"

#include "sync_object.h"
#include "audio_decoder.h"
#include "video_decoder.h"

/****************************/
/* the standard sync object */
/****************************/

void WV_AVSync_signalStateChange(WVSyncObject* sync)
{
  WV_videoDecoderSignal();
} 



void WV_AVSync_getRefClock(WVSyncObject* sync, WVReferenceClock* refClock) 
{
  /* private contain the audioStreamHdl */
  WVAudioStreamHandle streamHdl = (WVAudioStreamHandle)sync->wvStdPrivate;

  /* get the audio clock */
  *refClock = WV_getAudioClock(streamHdl);
}


int WV_AVSync_play(WVSyncObject* sync)
{
  /* private contain the audioStreamHdl */
  WVAudioStreamHandle streamHdl = (WVAudioStreamHandle)sync->wvStdPrivate;
  
  /* play */
  return WV_playAudio(streamHdl);
}


int WV_AVSync_pause(WVSyncObject* sync)
{
  /* private contain the audioStreamHdl */
  WVAudioStreamHandle streamHdl = (WVAudioStreamHandle)sync->wvStdPrivate;
  
  /* pause */
  return WV_pauseAudio(streamHdl);
}


int WV_AVSync_seek(WVSyncObject* sync, uint32_t targetClock, int seekFlag)
{
  //targetClock is useless, it was given by pts
  //seekFlag is useless, it was given to startAudioSeeking

  /* private contain the audioStreamHdl */
  WVAudioStreamHandle streamHdl = (WVAudioStreamHandle)sync->wvStdPrivate;

  /* seek */
  return WV_seekAudio(streamHdl);
}

  



WVSyncObject* WV_getAVSyncObj(WVAudioStreamHandle streamHdl)
{

  /* alloc the struct */
  WVSyncObject* syncObj;
  
  syncObj = (WVSyncObject*)malloc(sizeof(WVSyncObject)); //private contain the audioStreamHdl

  
  /* fill struct */
  syncObj->signalStateChange = &WV_AVSync_signalStateChange;
  syncObj->getRefClock = &WV_AVSync_getRefClock;
  syncObj->play = &WV_AVSync_play;
  syncObj->pause = &WV_AVSync_pause;
  syncObj->seek = &WV_AVSync_seek;
  syncObj->wvStdPrivate = streamHdl;


  /* return */
  return syncObj;
}




void WV_freeAVSyncObj(WVSyncObject* syncObj)
{
  free(syncObj);
}



  
  
