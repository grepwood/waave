#ifndef AUDIO_DECODER_EOFS_H
#define AUDIO_DECODER_EOFS_H

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
/* the EOF list  */
/*****************/

// we cannot have more than 3 eof; 
#define WVAD_EOF_LIST_SIZE 3 

typedef struct WVADEOFList{
  int16_t* eofStreamPos[WVAD_EOF_LIST_SIZE]; //this is a list of stream position
                                             // where the reader need to signal eof
  int readPos;
  int writePos;
}WVADEOFList;


/* init eof list */
void WVAD_initEOFS(WVADEOFList* eofL);



/*******************************/
/* some function to manipulate */
/* the eof list                */
/*******************************/
int WVAD_haveEOF(WVADEOFList* eofL);
int16_t* WVAD_firstEOF(WVADEOFList* eofL);
int16_t* WVAD_lastEOF(WVADEOFList* eofL);
int16_t* WVAD_getEOF(WVADEOFList* eofL);
void WVAD_putEOF(WVADEOFList* eofL, int16_t* eofPos);
void WVAD_deleteFirstEOF(WVADEOFList* eofL);
void WVAD_deleteEOFS(WVADEOFList* eofL);


/* replace the eof = or after bufferEnd at the beginning of the buffer */
void WVAD_updateEOFS(WVADEOFList* eofL, int16_t* bufferStart, int16_t* bufferEnd);


#endif
