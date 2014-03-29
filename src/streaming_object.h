#ifndef STREAMING_OBJECT_H
#define STREAMING_OBJECT_H

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
#include "config_ffmpeg.h"


/* the get buffer method */
//the buffers doesn't change, get is used one time per slot
#define WV_STATIC_GET 0
//we need to get buffer each time we decode a frame   
#define WV_DYNAMIC_GET 1  

/* the Get/Lock/Release method */
//just after the frame was displayed we reload the slot
#define WV_SYNC_GLR  0  
//we load the slot just before the next slot was displayed
#define WV_ASYNC_GLR 1  

/* Get/Lock/Release thread safety */
//the function is thread safe
#define WV_THREAD_SAFE 1    
//the function is not thread safe
#define WV_NO_THREAD_SAFE 0 


/***************************/
/* frame buffer descriptor */
/***************************/
typedef struct WVStreamingBuffer{
  
  /* the frame size and format */
  int width;
  int height;
  enum PixelFormat format;  //see ffmpeg format

  /* the frame plane */
  uint8_t* data[4];

  /* the corresponding  plane linesize */
  int linesize[4];

}WVStreamingBuffer;


/*******************/
/* the main object */
/*******************/
typedef struct WVStreamingObject{
  /* the src format, initialized by the decoder */
  /* init can use it to adjust his buffers      */
  int srcWidth;
  int srcHeight;
  enum PixelFormat srcFormat;  //ffmpeg src format 
  
  /* the size of the list where we decode frames */
  int nbSlots;


  /****************************/
  /* streaming object methods */
  /****************************/
  
  /* method type */
  int getBufferMethod;  //STATIC or DYNAMIC
  int GLRMethod;        //SYNC or ASYNC get/lock/release
  int getThreadSafety;  //get thread safety
  int LRThreadSafety;   //Lock and release thread safety
  

  /* init the object */
  int (*init)(struct WVStreamingObject* streamObj);

  /* get a frame buffer */
  /* if we have a static get, this is done one time per slot */
  /* if we have a dynamic get, we get each time we decode a new frame in a slot */
  /* !!! NULL if useless !!!*/
  WVStreamingBuffer (*getBuffer)(struct WVStreamingObject* streamObj, int slotIdx);

  /* lock the buffer before writing */
  /* !!! NULL if useless !!! */
  int (*lockBuffer)(struct WVStreamingObject* streamObj, int slotIdx);

  /* filter the buffer with the user method */
  /* !!! need to be thread safe !!! */
  /* else filter in releaseBuffer */
  int (*filterBuffer)(struct WVStreamingObject* streamObj, int slotIdx, WVStreamingBuffer* buffer);

  /* release the buffer after writing */
  /* !!! NULL if useless !!! */
  int (*releaseBuffer)(struct WVStreamingObject* streamObj, int slotIdx);

  /* refresh the frame by displaying his content */
  int (*refreshFrame)(struct WVStreamingObject* streamObj, int slotIdx);

  /* close the object */
  int (*close)(struct WVStreamingObject* streamObj);


  /****************/
  /* private data */
  /****************/
  void* objPrivate;

}WVStreamingObject;



  
#endif
