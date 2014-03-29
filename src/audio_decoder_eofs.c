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

#include "audio_decoder_eofs.h"

#include "common.h"


/* init eof list */
void WVAD_initEOFS(WVADEOFList* eofL)
{
  eofL->readPos = 0;
  eofL->writePos = 0;
}


/*******************************/
/* some function to manipulate */
/* the eof list                */
/*******************************/


int WVAD_haveEOF(WVADEOFList* eofL)
{
  if(eofL->readPos == eofL->writePos)
    return 0;
  else
    return 1;
}


int16_t* WVAD_firstEOF(WVADEOFList* eofL)
{
  if(!WVAD_haveEOF(eofL))
    return NULL;

  return eofL->eofStreamPos[eofL->readPos];
}

int16_t* WVAD_lastEOF(WVADEOFList* eofL)
{
  if(!WVAD_haveEOF(eofL))
    return NULL;

  int writePos = eofL->writePos;
  writePos--;  //the write pos is set after the last eof
  if(writePos < 0)
    writePos = WVAD_EOF_LIST_SIZE - 1;

  return eofL->eofStreamPos[writePos];
}
  

int16_t* WVAD_getEOF(WVADEOFList* eofL)
{
  if(!WVAD_haveEOF(eofL))
    return NULL;

  int16_t* popedEOF = eofL->eofStreamPos[eofL->readPos];
  
  eofL->readPos++;
  if(eofL->readPos >= WVAD_EOF_LIST_SIZE)
    eofL->readPos = 0;
  
  return popedEOF;
}


void WVAD_putEOF(WVADEOFList* eofL, int16_t* eofPos)
{
  /* may this eof is already set */
  if(WVAD_lastEOF(eofL) == eofPos)
    return;
  
  /* else put the eof */ 
  eofL->eofStreamPos[eofL->writePos] = eofPos;
  eofL->writePos++;
  if(eofL->writePos >= WVAD_EOF_LIST_SIZE)
    eofL->writePos = 0;
}



void WVAD_deleteFirstEOF(WVADEOFList* eofL)
{
  if(WVAD_haveEOF(eofL)){
    eofL->readPos++;
    if(eofL->readPos >= WVAD_EOF_LIST_SIZE)
      eofL->readPos = 0;
  }
}


void WVAD_deleteEOFS(WVADEOFList* eofL)
{
  eofL->readPos = 0;
  eofL->writePos = 0;
}


/* replace the EOFs = or after bufferEnd at the beginning of the buffer */
void WVAD_updateEOFS(WVADEOFList* eofL, int16_t* bufferStart, int16_t* bufferEnd)
{
  int checkedEOF = eofL->readPos;
  while(checkedEOF != eofL->writePos){
    /* check if the eof is out of the buffer  */
    if(eofL->eofStreamPos[checkedEOF] >= bufferEnd){
      unsigned int shift = eofL->eofStreamPos[checkedEOF] - bufferEnd;
      eofL->eofStreamPos[checkedEOF] = bufferStart + shift;
    }

    checkedEOF++;
    if(checkedEOF >= WVAD_EOF_LIST_SIZE)
      checkedEOF = 0;
  }
}


