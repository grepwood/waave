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

#include "eof_signal.h"

#include "common.h"
#include "config_sdl.h"

#include "waave_engine_flags.h"
#include "waave_stream.h"




/* the decoders will send the stream pointer */
void WV_signalEOF(void* streamHdl)
{
  /* check stream */
  if(!streamHdl)
    return;

  WVStream* stream = (WVStream*)streamHdl;
  
  /**********/
  /* signal */
  /**********/
  
  /* with the event system */
  if(stream->eofSignalType == WV_EVENT_EOF_SIGNAL){
    
    SDL_Event event;
  
    event.type = WV_EOF_EVENT;
    event.user.data1 = stream;
    event.user.data2 = stream->eofSignalParam; 
    
    SDL_PushEvent(&event);
  }

  /* with the user function */
  else if(stream->eofSignalType == WV_USER_EOF_SIGNAL && stream->eofSignalCall){
    
    stream->eofSignalCall(stream, stream->eofSignalParam);
  }

  /* else, nothing to do */
  
}






