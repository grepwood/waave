#ifndef AUDIO_DECODER_STOPS_H
#define AUDIO_DECODER_STOPS_H

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


/*****************/
/* the stop list */
/*****************/

// we cannot have more than 3 stop; two blocks + pause 
#define WVAD_STOP_LIST_SIZE 3 

typedef struct WVADStopList{
  int16_t* stopStreamPos[WVAD_STOP_LIST_SIZE]; //this is a list of stream position
                                               // where the reader need to stop
  int readPos;
  int writePos;
}WVADStopList;


/* init stop list */
void WVAD_initStops(WVADStopList* stopL);



/*******************************/
/* some function to manipulate */
/* the stop list               */
/*******************************/
int WVAD_haveStop(WVADStopList* stopL);
int16_t* WVAD_firstStop(WVADStopList* stopL);
int16_t* WVAD_lastStop(WVADStopList* stopL);
int16_t* WVAD_getStop(WVADStopList* stopL);
void WVAD_putStop(WVADStopList* stopL, int16_t* stop);
void WVAD_putStopAtStart(WVADStopList* stopL, int16_t* stop);
void WVAD_deleteFirstStop(WVADStopList* stopL);
void WVAD_deleteStops(WVADStopList* stopL);


/* replace the stops = or after bufferEnd at the beginning of the buffer */
void WVAD_updateStops(WVADStopList* stopL, int16_t* bufferStart, int16_t* bufferEnd);


#endif
