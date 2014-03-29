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

#include "stream_overlay.h"

#include "common.h"
#include "config_sdl.h"

#include "streaming_object.h"

/*!!!!!!!!!!!!!!!!!!!!!!!*/
/*    only for SDL 1.2   */
/*!!!!!!!!!!!!!!!!!!!!!!!*/
#if !SDL_VERSION_ATLEAST(2,0,0)

typedef struct StreamOverlayPrivate{
  /* user params */
  SDL_Surface* targetSurface;
  SDL_Rect userDestRect;

  /* the object overlays */
  SDL_Rect targetRect;
  SDL_Overlay** slotOverlay;
}StreamOverlayPrivate;


/* !!! need objPrivate->targetSurface, objPrivate->userDestRect !!! */
/* !!! and an initialized streaming object for srcWidth and srcHeight !!! */
static void setOutputRect(WVStreamingObject* streamObj)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;
  
  /* compute ratios */
  int outputWidth;
  int outputHeight;

  if(objPrivate->userDestRect.w && objPrivate->userDestRect.h){ //user give rect ?
    outputWidth = objPrivate->userDestRect.w;
    outputHeight = objPrivate->userDestRect.h;
  }
  else{
    outputWidth = objPrivate->targetSurface->w;
    outputHeight = objPrivate->targetSurface->h;
  }

  double vidRatio = (double)streamObj->srcWidth/(double)streamObj->srcHeight;
  double outputRatio = (double)outputWidth/(double)outputHeight;
  int rem;
  
  if(vidRatio >= outputRatio){
    objPrivate->targetRect.w = outputWidth;
    vidRatio = (double)outputWidth / vidRatio; //the corresponding height
    objPrivate->targetRect.h = (int)vidRatio;

    rem = outputHeight - objPrivate->targetRect.h;
    rem /= 2;
    
    if(objPrivate->userDestRect.w && objPrivate->userDestRect.h){ //user give rect ?
      objPrivate->targetRect.x = objPrivate->userDestRect.x;
      objPrivate->targetRect.y = objPrivate->userDestRect.y + rem;
    }
    else{
      objPrivate->targetRect.x = 0;
      objPrivate->targetRect.y = rem;
    }
  }
  else{
    objPrivate->targetRect.h = outputHeight;
    vidRatio *= (double)outputHeight;    //the corresponding width
    objPrivate->targetRect.w = (int)vidRatio;

    rem = outputWidth - objPrivate->targetRect.w;
    rem /= 2;
    
    if(objPrivate->userDestRect.w && objPrivate->userDestRect.h){ //user give rect ?
      objPrivate->targetRect.y = objPrivate->userDestRect.y;
      objPrivate->targetRect.x = objPrivate->userDestRect.x + rem;
    }
    else{
      objPrivate->targetRect.y = 0;
      objPrivate->targetRect.x = rem;
    }
  }
}
  
    

static int init_streamOverlay(WVStreamingObject* streamObj)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;
  
  /* set output rect */
  setOutputRect(streamObj);

  /* alloc space for the overlays */
  objPrivate->slotOverlay = (SDL_Overlay**)malloc(streamObj->nbSlots*sizeof(SDL_Overlay*));
  
  int i;
  for(i=0; i<streamObj->nbSlots; i++)
    objPrivate->slotOverlay[i] = NULL;

  return 0;
}


static WVStreamingBuffer getBuffer_streamOverlay(WVStreamingObject* streamObj, int slotIdx)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;

  /* create an overlay */
  /* we use native size */
  SDL_Overlay* newOverlay;
  
  if(!objPrivate->slotOverlay[slotIdx]){
    
    newOverlay = SDL_CreateYUVOverlay(streamObj->srcWidth, streamObj->srcHeight, \
						 SDL_YV12_OVERLAY, objPrivate->targetSurface);
    /* save */
    objPrivate->slotOverlay[slotIdx] = newOverlay;
  }
  else{
    newOverlay = objPrivate->slotOverlay[slotIdx];
  }
  
  /* create the buffer */
  WVStreamingBuffer newBuff;
  
  newBuff.width = streamObj->srcWidth;
  newBuff.height = streamObj->srcHeight;
  newBuff.format = PIX_FMT_YUV420P;
  
  /* fill frame plane */
  newBuff.data[0] = newOverlay->pixels[0];
  newBuff.data[1] = newOverlay->pixels[2];
  newBuff.data[2] = newOverlay->pixels[1];

  /* fill plane linesize */
  newBuff.linesize[0] = newOverlay->pitches[0];
  newBuff.linesize[1] = newOverlay->pitches[2];
  newBuff.linesize[2] = newOverlay->pitches[1];

  /* return the buffer */
  return newBuff;
}


static int lockBuffer_streamOverlay(WVStreamingObject* streamObj, int slotIdx)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;

  /* lock the overlay */
  /* seems thread safe */
  SDL_LockYUVOverlay(objPrivate->slotOverlay[slotIdx]);

  return 0;
}


static int releaseBuffer_streamOverlay(WVStreamingObject* streamObj, int slotIdx)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;

  /* release the overlay */
  /* seems thread safe */
  SDL_UnlockYUVOverlay(objPrivate->slotOverlay[slotIdx]);

  return 0;
}


static int refreshFrame_streamOverlay(WVStreamingObject* streamObj, int slotIdx)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;

  /* blit the overlay */
  return SDL_DisplayYUVOverlay(objPrivate->slotOverlay[slotIdx], &(objPrivate->targetRect));
}


static int close_streamOverlay(WVStreamingObject* streamObj)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;
  
  /* close all the overlays */
  int i;
  for(i=0; i<streamObj->nbSlots; i++){
    if(objPrivate->slotOverlay[i]){
      SDL_FreeYUVOverlay(objPrivate->slotOverlay[i]);
      objPrivate->slotOverlay[i] = NULL;
    }
  }

  /* free the overlays buffer */
  free(objPrivate->slotOverlay);

  return 0;
}


WVStreamingObject* WV_getStreamOverlayObj(SDL_Surface* targetSurface, SDL_Rect* userDestRect)
{
  /* alloc the struct */
  WVStreamingObject* streamObj;
  streamObj = (WVStreamingObject*)malloc(sizeof(WVStreamingObject) + sizeof(StreamOverlayPrivate));
  
  void* structP = (void*)streamObj;
  structP += sizeof(WVStreamingObject);
  streamObj->objPrivate = structP;

  
  /* save user params */
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;
  
  objPrivate->targetSurface = targetSurface;
  if(userDestRect)
    objPrivate->userDestRect = *userDestRect;
  else{
    objPrivate->userDestRect.w = 0; //this say we doesn't have a user rect
    objPrivate->userDestRect.h = 0;
  }

  /**********************/
  /* fill object params */
  /**********************/
  
  /* we use 3 slots */
  streamObj->nbSlots = 3;
  
  /* methods params */
  streamObj->getBufferMethod = WV_STATIC_GET;  //get the buffers one time
  streamObj->GLRMethod = WV_ASYNC_GLR;         //async
  streamObj->getThreadSafety = WV_NO_THREAD_SAFE;
  streamObj->LRThreadSafety = WV_THREAD_SAFE;   //assume that lockSurface is thread safe
  
  
  /* methods */
  streamObj->init = &init_streamOverlay;
  streamObj->getBuffer = &getBuffer_streamOverlay;
  streamObj->lockBuffer = &lockBuffer_streamOverlay;
  streamObj->filterBuffer = NULL;
  streamObj->releaseBuffer = &releaseBuffer_streamOverlay;
  streamObj->refreshFrame = &refreshFrame_streamOverlay;
  streamObj->close = &close_streamOverlay;


  /*********************/
  /* return the object */
  /*********************/
  return streamObj;
}


void WV_freeStreamOverlayObj(WVStreamingObject* streamObj)
{
  free(streamObj);
}


void WV_resetStreamOverlayOutput(WVStreamingObject* streamObj, SDL_Surface* targetSurface, SDL_Rect* userDestRect)
{
  StreamOverlayPrivate* objPrivate = (StreamOverlayPrivate*)streamObj->objPrivate;
  
  objPrivate->targetSurface = targetSurface;
  if(userDestRect)
    objPrivate->userDestRect = *userDestRect;
  else{
    objPrivate->userDestRect.w = 0; //this say we doesn't have a user rect
    objPrivate->userDestRect.h = 0;
  }
  
  /* reset output rect */
  setOutputRect(streamObj);
}


#endif



  



