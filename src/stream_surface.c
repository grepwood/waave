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

#include "stream_surface.h"

#include "common.h"
#include "config_sdl.h"

#include "streaming_object.h"

/*!!!!!!!!!!!!!!!!!!!!!!!*/
/*    only for SDL 1.2   */
/*!!!!!!!!!!!!!!!!!!!!!!!*/
#if !SDL_VERSION_ATLEAST(2,0,0)


typedef struct StreamSurfacePrivate{
  /* user params */
  SDL_Surface* targetSurface;
  SDL_Rect userDestRect;
  int updateFlag;

  /* the object surface */
  SDL_Rect targetRect;
  SDL_Surface** slotSurface;

  /* mutex  */
  /* for the slot surface */
  SDL_mutex* surfaceMutex;

}StreamSurfacePrivate;


/* !!! need objPrivate->targetSurface, objPrivate->userDestRect !!! */
/* !!! and an initialized streaming object for srcWidth and srcHeight !!! */
static void setOutputRect(WVStreamingObject* streamObj)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;
  
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
  
    

static int init_streamSurface(WVStreamingObject* streamObj)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;
  
  /* set output rect */
  setOutputRect(streamObj);
  SDL_Rect* targetRect = &(objPrivate->targetRect);

  /* alloc space for the surface */
  objPrivate->slotSurface = (SDL_Surface**)malloc(streamObj->nbSlots*sizeof(SDL_Surface*));
  
  int i;
  for(i=0; i<streamObj->nbSlots; i++)
    objPrivate->slotSurface[i] = NULL;

  /* create the surfaces */
  for(i=0; i<streamObj->nbSlots; i++){
    objPrivate->slotSurface[i] = SDL_CreateRGBSurface(SDL_HWSURFACE, targetRect->w, targetRect->h, 32, 0, 0, 0, 0);
    if(!objPrivate->slotSurface[i])
      return -1;
  }

  /* create the mutex */
  objPrivate->surfaceMutex = SDL_CreateMutex();

  return 0;
}



static WVStreamingBuffer getBuffer_streamSurface(WVStreamingObject* streamObj, int slotIdx)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;

  /* lock the mutex */
  /* we can change the surfaces before release */
  SDL_mutexP(objPrivate->surfaceMutex);
    
  /* create the buffer */
  WVStreamingBuffer newBuff;
  SDL_Surface* surface = objPrivate->slotSurface[slotIdx];
  
  newBuff.width = surface->w;
  newBuff.height = surface->h;
  newBuff.format = PIX_FMT_RGB32;
  
  /* fill frame plane */
  newBuff.data[0] = surface->pixels;
  
  /* fill plane linesize */
  newBuff.linesize[0] = surface->w * 4;
  
  /* return the buffer */
  return newBuff;
}


static int lockBuffer_streamSurface(WVStreamingObject* streamObj, int slotIdx)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;

  /* lock the surface */
  /* seems thread safe */
  SDL_LockSurface(objPrivate->slotSurface[slotIdx]);

  return 0;
}


static int releaseBuffer_streamSurface(WVStreamingObject* streamObj, int slotIdx)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;

  /* release the surface */
  /* seems thread safe */
  SDL_UnlockSurface(objPrivate->slotSurface[slotIdx]);

  /* we can now change the surface if needed */
  SDL_mutexV(objPrivate->surfaceMutex);

  return 0;
}


static int refreshFrame_streamSurface(WVStreamingObject* streamObj, int slotIdx)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;
  SDL_Rect* targetRect = &(objPrivate->targetRect);

  /* blit the surface */
  SDL_BlitSurface(objPrivate->slotSurface[slotIdx], NULL, objPrivate->targetSurface, &(objPrivate->targetRect));
  if(objPrivate->updateFlag)
    SDL_UpdateRect(objPrivate->targetSurface, targetRect->x, targetRect->y, targetRect->w, targetRect->h);
  
  return 0;
}


static int close_streamSurface(WVStreamingObject* streamObj)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;
  
  /* close all the surfaces */
  SDL_mutexP(objPrivate->surfaceMutex);

  int i;
  for(i=0; i<streamObj->nbSlots; i++){
    if(objPrivate->slotSurface[i]){
      SDL_FreeSurface(objPrivate->slotSurface[i]);
      objPrivate->slotSurface[i] = NULL;
    }
  }

  SDL_mutexV(objPrivate->surfaceMutex);

  /* close the mutex */
  SDL_DestroyMutex(objPrivate->surfaceMutex);

  /* free the surface buffer */
  free(objPrivate->slotSurface);

  

  return 0;
}


WVStreamingObject* WV_getStreamSurfaceObj(SDL_Surface* targetSurface, SDL_Rect* userDestRect, int updateFlag)
{
  /* alloc the struct */
  WVStreamingObject* streamObj;
  streamObj = (WVStreamingObject*)malloc(sizeof(WVStreamingObject) + sizeof(StreamSurfacePrivate));
  
  void* structP = (void*)streamObj;
  structP += sizeof(WVStreamingObject);
  streamObj->objPrivate = structP;

  
  /* save user params */
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;
  
  objPrivate->targetSurface = targetSurface;
  if(userDestRect)
    objPrivate->userDestRect = *userDestRect;
  else{
    objPrivate->userDestRect.w = 0; //this say we doesn't have a user rect
    objPrivate->userDestRect.h = 0;
  }

  objPrivate->updateFlag = updateFlag;

  /**********************/
  /* fill object params */
  /**********************/
  
  /* we use 3 slots */
  streamObj->nbSlots = 3;
  
  /* methods params */
  streamObj->getBufferMethod = WV_DYNAMIC_GET;    //buffer size may change
  streamObj->GLRMethod = WV_ASYNC_GLR;            //async
  streamObj->getThreadSafety = WV_THREAD_SAFE;
  streamObj->LRThreadSafety = WV_THREAD_SAFE;     //assume that lockSurface is thread safe
  
  
  /* methods */
  streamObj->init = &init_streamSurface;
  streamObj->getBuffer = &getBuffer_streamSurface;
  streamObj->lockBuffer = &lockBuffer_streamSurface;
  streamObj->filterBuffer = NULL;
  streamObj->releaseBuffer = &releaseBuffer_streamSurface;
  streamObj->refreshFrame = &refreshFrame_streamSurface;
  streamObj->close = &close_streamSurface;


  /*********************/
  /* return the object */
  /*********************/
  return streamObj;
}


void WV_freeStreamSurfaceObj(WVStreamingObject* streamObj)
{
  free(streamObj);
}


void WV_resetStreamSurfaceOutput(WVStreamingObject* streamObj, SDL_Surface* targetSurface, SDL_Rect* userDestRect)
{
  StreamSurfacePrivate* objPrivate = (StreamSurfacePrivate*)streamObj->objPrivate;
  
  objPrivate->targetSurface = targetSurface;
  if(userDestRect)
    objPrivate->userDestRect = *userDestRect;
  else{
    objPrivate->userDestRect.w = 0; //this say we doesn't have a user rect
    objPrivate->userDestRect.h = 0;
  }
  
  /* reset output rect */
  setOutputRect(streamObj);
  SDL_Rect* targetRect = &(objPrivate->targetRect);

  /* update the slot's surfaces */
  /* !! lock the mutex !! */
  SDL_mutexP(objPrivate->surfaceMutex);

  int i;
  for(i=0; i<streamObj->nbSlots; i++){
   
    SDL_Surface* currentSurface = objPrivate->slotSurface[i];

    if(currentSurface->w != targetRect->w || currentSurface->h != targetRect->h){
      SDL_LockSurface(currentSurface);
      SDL_UnlockSurface(currentSurface);
      SDL_FreeSurface(currentSurface);
      objPrivate->slotSurface[i] = SDL_CreateRGBSurface(SDL_HWSURFACE, targetRect->w, targetRect->h, 32, 0, 0, 0, 0);
    }
  }
  
  /* !! release !! */
  SDL_mutexV(objPrivate->surfaceMutex);
}





#endif  



