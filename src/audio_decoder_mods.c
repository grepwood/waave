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

#include "audio_decoder_mods.h"

#include "common.h"


/*******************************/
/* some function to manipulate */
/* the mod list                */
/*******************************/

/* check if we have a mod */
int WVAD_haveMod(WVADModList* modL)
{
  if(modL->readPos == modL->writePos)
    return 0;
  else
    return 1;
}


/* check if we need to change the clock reference */
/* by reading the mod list */
/* !!! the stream must be locked by the mutex !!!*/
int WVAD_needRefMod(WVADModList* modL, int16_t* pos)
{
  if(WVAD_haveMod(modL)){
    if(pos == modL->mod[modL->readPos].pos){  //check if the first mod is at pos
      return 1;
    }
    else
      return 0;
  }
  
  return 0;
}


/* get the first reference clock modificator */
/* and put it out of the list */
uint32_t WVAD_refMod(WVADModList* modL)
{
  /* get the mod */
  uint32_t popedMod = modL->mod[modL->readPos].value;
  
  /* pop the mod */
  modL->readPos++;
  if(modL->readPos >= WVAD_MOD_LIST_SIZE)
    modL->readPos = 0;

  /* return */
  return popedMod;
}


/* put a new mod on the list */
void WVAD_saveRefMod(WVADModList* modL, int16_t* pos, uint32_t value)
{
  /* check if we have a previous mod */
  if(WVAD_haveMod(modL)){
    int previousModPos = modL->writePos - 1;
    if(previousModPos < 0)
      previousModPos = WVAD_MOD_LIST_SIZE - 1;
    
    if(pos < modL->mod[previousModPos].pos)
      return; //we must have encreasing positions
    else if(pos == modL->mod[previousModPos].pos){ //if the new pos is the same the previous
      modL->mod[previousModPos].pos = pos;         //we overwrite the previous mod
      return;
    }
  }

  /* we have encreasing positions */
  modL->mod[modL->writePos].pos = pos;
  modL->mod[modL->writePos].value = value;
  
  /* update position */
  modL->writePos++;
  if(modL->writePos >= WVAD_MOD_LIST_SIZE)
    modL->writePos = 0;
  
}
  

/* delete all the mods */
void WVAD_deleteMods(WVADModList* modL)
{
  modL->readPos = 0;
  modL->writePos = 0;
}


/* keep only the mod at pos */
/* return 1 if a mod was found */
/* else return 0 */
int WVAD_keepOnlyMod(WVADModList* modL, int16_t* pos)
{
  /* search for the pos */
  while(modL->mod[modL->readPos].pos != pos &&\
	modL->readPos != modL->writePos){
    modL->readPos++;
    if(modL->readPos >= WVAD_MOD_LIST_SIZE)
      modL->readPos = 0;
  }

  /* found ? */
  if(modL->readPos != modL->writePos){
    modL->writePos = modL->readPos + 1;
    if(modL->writePos >= WVAD_MOD_LIST_SIZE)
      modL->writePos = 0;

    return 1;
  }
  else
    return 0;
}


/* reset the first mod */
/* !!! check for mods first !!! */
void WVAD_resetModPos(WVADModList* modL, int16_t* pos)
{
  modL->mod[modL->readPos].pos = pos;
}


/* replace the mods = or after bufferEnd at the beginning of the buffer */
void WVAD_updateMods(WVADModList* modL, int16_t* bufferStart, int16_t* bufferEnd)
{
  int checkedMod = modL->readPos;
  while(checkedMod != modL->writePos){
    /* check if the the mod pos is after streamEnd */
    if(modL->mod[checkedMod].pos >= bufferEnd){
      unsigned int shift = modL->mod[checkedMod].pos - bufferEnd;
      modL->mod[checkedMod].pos = bufferStart + shift;
    }

    checkedMod++;
    if(checkedMod >= WVAD_MOD_LIST_SIZE)
      checkedMod = 0;
  }
}

/* !!! doesn't pop the mod !!!*/
/* !!! check for mods first !!!*/
int16_t* WVAD_firstModPos(WVADModList* modL)
{
  return modL->mod[modL->readPos].pos;
}


/* !!! doesn't pop the mod !!!*/
/* !!! check for mods first !!!*/
uint32_t WVAD_firstModValue(WVADModList* modL)
{
  return modL->mod[modL->readPos].value;
}

