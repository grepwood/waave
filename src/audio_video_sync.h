#ifndef AUDIO_VIDEO_SYNC_H
#define AUDIO_VIDEO_SYNC_H

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

#include "sync_object.h"
#include "audio_decoder.h"
#include "video_decoder.h"

/* standard audio video sync */
WVSyncObject* WV_getAVSyncObj(WVAudioStreamHandle streamHdl);
void WV_freeAVSyncObj(WVSyncObject* syncObj);


/* standard sync methods */
void WV_AVSync_signalStateChange(WVSyncObject* sync);
void WV_AVSync_getRefClock(WVSyncObject* sync, WVReferenceClock* refClock);
int WV_AVSync_play(WVSyncObject* sync);
int WV_AVSync_pause(WVSyncObject* sync);
int WV_AVSync_seek(WVSyncObject* sync, uint32_t targetClock, int seekFlag);


#endif
