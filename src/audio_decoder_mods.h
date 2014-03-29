#ifndef AUDIO_DECODER_MODS_H
#define AUDIO_DECODER_MODS_H

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


/****************/
/* the mod list */
/****************/

// we cannot have more than 3 ref mod; two blocks + 1block latency
#define WVAD_MOD_LIST_SIZE 3   


// this is a indicator for when we need to change the reference of the audio clock
typedef struct WVADClockRefMod{
  int16_t* pos;    //position of the change
  uint32_t value;  //the new reference
}WVADClockRefMod;


//the list
typedef struct WVADModList{
  WVADClockRefMod mod[WVAD_MOD_LIST_SIZE];  //a list of clock reference modification
  int readPos;
  int writePos;
}WVADModList;


/*******************************/
/* some function to manipulate */
/* the mod list                */
/*******************************/

/* check if we have a mod */
int WVAD_haveMod(WVADModList* modL);


/* check if we need to change the clock reference */
/* by reading the mod list */
/* !!! the stream must be locked by the mutex !!!*/
int WVAD_needRefMod(WVADModList* modL, int16_t* pos);


/* get the first reference clock modificator */
/* and put it out of the list */
uint32_t WVAD_refMod(WVADModList* modL);


/* put a new mod on the list */
void WVAD_saveRefMod(WVADModList* modL, int16_t* pos, uint32_t value);
 

/* delete all the mods */
void WVAD_deleteMods(WVADModList* modL);


/* keep only the mod at pos */
/* return 1 if a mod was found */
/* else return 0 */
int WVAD_keepOnlyMod(WVADModList* modL, int16_t* pos);


/* reset the first mod */
/* !!! check for mods first !!! */
void WVAD_resetModPos(WVADModList* modL, int16_t* pos);


/* replace the mods = or after bufferEnd at the beginning of the buffer */
void WVAD_updateMods(WVADModList* modL, int16_t* bufferStart, int16_t* bufferEnd);


/* !!! doesn't pop the mod !!!*/
/* !!! check for mods first !!!*/
int16_t* WVAD_firstModPos(WVADModList* modL);


/* !!! doesn't pop the mod !!!*/
/* !!! check for mods first !!!*/
uint32_t WVAD_firstModValue(WVADModList* modL);



#endif
