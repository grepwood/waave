#ifndef SYNC_OBJECT_H
#define SYNC_OBJECT_H

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


/*****************************/
/* the clock data structure  */
/*****************************/
typedef struct WVReferenceClock{
  
  uint32_t clock;   //the clock value
  int pauseFlag;    //say if the clock is paused
  int modIdx;       //this count the number of clock modification
                    //this is used to know if we get the clock for the 
                    //good chunk of the stream
}WVReferenceClock;




/* we have two way for seeking */
//if the clock is paused, seek cause restart playing
#define WV_PLAYING_SEEK 0
//if the clock is paused, it stay paused after the seek
#define WV_BLOCKING_SEEK 1

/* we have two way for thread EOF */
// if clock reach stream end, pause */
#define WV_BLOCKING_STREAM 0
// if clock reach stream end, loop */
#define WV_LOOPING_STREAM 1



/*******************/
/* the sync object */
/*******************/
typedef struct WVSyncObject{
  
  /************/
  /* EOF FLAG */
  /************/
  /* This is set by the engine but must be read */
  /* by the method to thread EOF */
  int loopingFlag;

  
  /*************/
  /* SLAVE DEF */
  /*************/
  /* signalStateChange is called by the clock generator */
  /* to signal the slave for clock modification */
  /* usually it signal play, seek and eof */
  /* but it does not signal pause because it can be read with pause flag */ 
  void (*signalStateChange)(struct WVSyncObject* sync); 

  /**************/
  /* MASTER DEF */
  /**************/

  /*!!! getRefClock and play are mandatory !!!*/
  /*!!! signalStateChange for play and seek but not for pause !!!*/ 

  /* GETREFCLOCK */
  /* return the master clock */
  /* the slave fill refClock->clock and refClock->modIdx with */
  /* it's current stream position, so master clock can readjust */
  /* itself  when modIdx change */
  /*!!! master get UINT32_MAX if the slave can't determine his stream pos !!!*/ 
  /*!!! if master give modIdx=0 the getRefClock must return his clock value !!!*/
  void (*getRefClock)(struct WVSyncObject* sync, WVReferenceClock* refClock); 

  
  /* PLAY */
  /* launch the master clock */
  /* !!! return -1 if play is useless (already playing) !!!*/
  int (*play)(struct WVSyncObject* sync);

  /* PAUSE */
  /* pause the master clock */
  /* return -1 if pause is useless  */
  int (*pause)(struct WVSyncObject* sync);


  /* SEEK */
  /* seek on master clock */
  /* return >= 0 on success */
  /* get the seekFlag (WV_PLAYING_SEEK or WV_BLOCKING_SEEK) */
  int (*seek)(struct WVSyncObject* sync, uint32_t clock, int seekFlag);
  
  /***********/
  /* PRIVATE */
  /***********/

  void* objPrivate;            //private vars for the methods
  
  void* wvStdPrivate;    //don't use it ! 
                         //it is for waave methods

}WVSyncObject;


#endif
