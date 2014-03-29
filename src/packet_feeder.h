#ifndef PACKET_FEEDER_H
#define PACKET_FEEDER_H

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
#include "config_sdl.h"
#include "config_ffmpeg.h"

#include "waave_engine_flags.h"


/*********************************/
/* first, launch the pkt feeder  */
/* this launch a new thread      */
/*********************************/
void WV_initPacketFeeder(void);

/*****************************************/
/* next build a feeder context           */
/* (the feeder support multiple context) */  
/*****************************************/

/* give the Format Context and say the number of streams you want to queue */
int WV_buildFeederContext(AVFormatContext* formatCtx, int nbStreams);

/* for each stream index, query the corresponding queue Handle  */
/* (the return value) */
/*!!! you need to call this function nbStreams times !!!*/
typedef void* WVQueueHandle;

WVQueueHandle WV_getStreamQueue(int streamIdx);

/* start feeding the context */
int WV_addFeederContext(void);


/******************************************************/
/* you can now start getting the pkts from the queues */
/* this is thread safe                                */
/******************************************************/

/* get a packet pointer */
/* !!! If waitFlag = QUEUE_GET_WAIT this function wait if there are not pkt to get !!!*/
/* !!! If waitFlag = QUEUE_GET_DOESNT_WAIT this function return NULL if there are not pkt !!!*/


/* !!! be carefull !!! */
/* if you get pkts only on one queue */
/* this will encrease all the other queue size */
/* because the feeder need to put there pkts */
/* to get the others */

#define WV_QUEUE_GET_WAIT 1
#define WV_QUEUE_GET_DOESNT_WAIT 0

AVPacket* WV_packetQueueGet(WVQueueHandle queueHdl, int waitFlag);

/* !!! free the pkt with !!!*/
/* av_free_packet(pkt) */
/* free(pkt) */


/************************************/
/* the feeder send you specials pkt */
/* if he need to send a message     */
/************************************/
/* if pkt->data == NULL check the pkt->flags */

//when the feeder reach end of file
#define WV_PACKET_FLAG_EOF 1  
//when the feeder seek
#define WV_PACKET_FLAG_SEEK 2 

/* !!! free those pkts with !!!*/
/* free(pkt) */



/****************/
/* you can seek */
/****************/
int WV_contextSeek(AVFormatContext* formatCtx, int streamIdx, uint64_t timestamp, int flags);




/****************************************/
/* you can remove a context at any time */
/****************************************/
/*!!! be sure you are not getting pkt from the context you want to delete !!!*/
int WV_delFeederContext(AVFormatContext* formatCtx);


/*******************/
/* or just a queue */
/*******************/
int WV_delFeederQueue(AVFormatContext* formatCtx, WVQueueHandle queueHdl);



/*********************************************/
/* when the job is done, shutdown the feeder */
/*********************************************/
int WV_packetFeederShutdown(void);




#endif
