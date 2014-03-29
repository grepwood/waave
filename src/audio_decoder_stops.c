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

#include "audio_decoder_stops.h"

#include "common.h"


/* init stop list */
void WVAD_initStops(WVADStopList* stopL)
{
  stopL->readPos = 0;
  stopL->writePos = 0;
}


/*******************************/
/* some function to manipulate */
/* the stop list               */
/*******************************/


int WVAD_haveStop(WVADStopList* stopL)
{
  if(stopL->readPos == stopL->writePos)
    return 0;
  else
    return 1;
}


int16_t* WVAD_firstStop(WVADStopList* stopL)
{
  if(!WVAD_haveStop(stopL))
    return NULL;

  return stopL->stopStreamPos[stopL->readPos];
}

int16_t* WVAD_lastStop(WVADStopList* stopL)
{
  if(!WVAD_haveStop(stopL))
    return NULL;

  int writePos = stopL->writePos;
  writePos--;  //the write pos is set after the last stop
  if(writePos < 0)
    writePos = WVAD_STOP_LIST_SIZE - 1;

  return stopL->stopStreamPos[writePos];
}
  

int16_t* WVAD_getStop(WVADStopList* stopL)
{
  if(!WVAD_haveStop(stopL))
    return NULL;

  int16_t* popedStop = stopL->stopStreamPos[stopL->readPos];
  
  stopL->readPos++;
  if(stopL->readPos >= WVAD_STOP_LIST_SIZE)
    stopL->readPos = 0;
  
  return popedStop;
}


void WVAD_putStop(WVADStopList* stopL, int16_t* stop)
{
  /* may this stop is already set */
  if(WVAD_lastStop(stopL) == stop)
    return;
  
  /* else put the stop */ 
  stopL->stopStreamPos[stopL->writePos] = stop;
  stopL->writePos++;
  if(stopL->writePos >= WVAD_STOP_LIST_SIZE)
    stopL->writePos = 0;
}

void WVAD_putStopAtStart(WVADStopList* stopL, int16_t* stop)
{
  /* may this stop is already set */
  if(WVAD_firstStop(stopL) == stop)
    return;

  if(WVAD_haveStop(stopL)){
    int readPos = stopL->readPos;
    readPos--;
    if(readPos < 0)
      readPos = WVAD_STOP_LIST_SIZE - 1;
  
    stopL->stopStreamPos[readPos] = stop;
    stopL->readPos = readPos;
  }

  else{
    stopL->stopStreamPos[stopL->writePos] = stop;
    stopL->writePos++;
    if(stopL->writePos >= WVAD_STOP_LIST_SIZE)
      stopL->writePos = 0;
  }
}

void WVAD_deleteFirstStop(WVADStopList* stopL)
{
  if(WVAD_haveStop(stopL)){
    stopL->readPos++;
    if(stopL->readPos >= WVAD_STOP_LIST_SIZE)
      stopL->readPos = 0;
  }
}


void WVAD_deleteStops(WVADStopList* stopL)
{
  stopL->readPos = 0;
  stopL->writePos = 0;
}


/* replace the stops = or after bufferEnd at the beginning of the buffer */
void WVAD_updateStops(WVADStopList* stopL, int16_t* bufferStart, int16_t* bufferEnd)
{
  int checkedStop = stopL->readPos;
  while(checkedStop != stopL->writePos){
    /* check if the stop out of the buffer  */
    if(stopL->stopStreamPos[checkedStop] >= bufferEnd){
      unsigned int shift = stopL->stopStreamPos[checkedStop] - bufferEnd;
      stopL->stopStreamPos[checkedStop] = bufferStart + shift;
    }

    checkedStop++;
    if(checkedStop >= WVAD_STOP_LIST_SIZE)
      checkedStop = 0;
  }
}


