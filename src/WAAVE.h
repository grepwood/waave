#ifndef WAAVE_LIBRARY_H
#define WAAVE_LIBRARY_H


#include <libavutil/pixfmt.h>
#include <stdint.h>
#include <SDL.h>


/**
 * \mainpage WAAVE documentation
 *
 * The waave main website is http://waave.sourceforge.net
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif


/*||||||||||||||||||||||||||||||||||||*/
/*                                    */
/*         THE STREAMING OBJECT       */
/*                                    */
/*||||||||||||||||||||||||||||||||||||*/


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


/**
 * \brief Frame buffer descriptor 
 *
 * Decribe frame buffers where the engine decode
 * video frames. This is the type returned by
 * the getBuffer method of the streaming object.
 *
 */
typedef struct WVStreamingBuffer{
  
  int width;               /**< \brief The frame width */ 
  int height;              /**< \brief The frame height */
  enum PixelFormat format; /**< \brief The frame pixel format, see ffmpeg documentation */    

  /** 
   * \brief The frame planes
   * If you have just one plane use data[0]
   */ 
  uint8_t* data[4];   

  /** 
   * \brief The corresponding  plane linesizein bytes 
   * If you have just one plane use linesize[0]
   */
  int linesize[4];

}WVStreamingBuffer;



/**
 * \brief Streaming method descriptor
 *
 * Describe all the methods needed to decode and display
 * video frames. This object must be given as stream parameter
 * to play any video data. However waave library come with
 * a standard streaming object for sdl surface. See corresponding
 * documentation. Note that all object methods will be called by
 * the main thread if there are not declared thread safe. 
 */ 
typedef struct WVStreamingObject{
  
  /** 
   * \brief The video native width (Set by decoder)
   * 
   * This is initialized by the decoder and
   * give the video native width. This can be used 
   * by user methods to adjust the frame buffers
   * relative to native format.
   */
  int srcWidth;
  
  /**
   * \brief The video native height (Set by decoder)
   *
   * This is initialized by the decoder and
   * give the video native height. This can be used 
   * by user methods to adjust the frame buffers
   * relative to native format.
   */
  int srcHeight;

  /** 
   * \brief The video native pixel format (Set by decoder)
   *
   * This is initialized by the decoder and
   * give the video native format (see ffmpeg documentation).
   * This can be used by user methods to adjust
   * the frame buffers relative to native format.
   */
  enum PixelFormat srcFormat;  //ffmpeg src format 
  

  /**
   * \brief The number of decoding buffers (Set by user)
   *
   * A slot is a place where we can decode 
   * a video frame independently of the others.
   * The user decides the number of slots that will
   * be built by setting the *nbSlots* var at object 
   * creation or with the *init* method. This 
   * must be bet !
   */
  int nbSlots;


  /**
   * \brief Type of buffer returned by the getBuffer method (Set by user)
   *
   * Must be set by user at objet creation or by the init method.
   * Say if the *getBuffer* method will be called one time 
   * per slots so we can reuse the obtained buffers. Or if 
   * it is needed to reget a buffer each time we want 
   * to decode a frame.
   *
   * Possible values :
   *
   * Get buffer method | Description 
   * ------------------|------------
   * WV_STATIC_GET     | The buffer will be allocated just one time, so the decoder can reuse it.
   * WV_DYNAMIC_GET    | The buffer will be allocated  each time the decoder decode a frame.
   *
   */
  int getBufferMethod;  //STATIC or DYNAMIC


  /**
   * \brief Get/Lock/Release synchronization (Set by user)
   *
   * Must be set by user at objet creation or by the init method.
   * Say when the decoder can reuse a slot to decode a frame.
   * 
   * GLR method   | Description
   * -------------|-------------
   * WV_SYNC_GLR  | Just after the frame was displayed, reload the slot
   * WV_ASYNC_GLR | We reload the slot just before the next slot was displayed
   *
   * The choice depend on method thread safety and Get/Lock/Release behavior.
   *
   */
  int GLRMethod;        //SYNC or ASYNC get/lock/release
  
  /**
   * \brief Get buffer method thread safety (Set by user)
   *
   * Must be set by user at objet creation or by the init method.
   * Say if the *getBuffer* method is thread safe and can be 
   * called directly by the decoder.
   *
   * Get Thread Safety | Description 
   * ------------------|---------------
   * WV_THREAD_SAFE    | Thread safe getBuffer
   * WV_NO_THREAD_SAFE | No thread safe getBuffer 
   *
   */
  int getThreadSafety;  //get thread safety



  /**
   * \brief Lock/Release method thread safety (Set by user)
   *
   * Must be set by user at objet creation or by the init method.
   * Say if the *lockBuffer* and *releaseBuffer* methods are thread safe
   * and can be called directly by the decoder.
   *
   * L/R Thread Safety | Description 
   * ------------------|---------------
   * WV_THREAD_SAFE    | Thread safe lock/release methods
   * WV_NO_THREAD_SAFE | No thread safe lock/release methods
   *
   */
  int LRThreadSafety;   //Lock and release thread safety
  

  /**
   * \brief Initialize object parameters and build decoding slots
   *
   * \param streamObj The streaming object 
   *
   * The init method must set all the object user variables if this
   * has not been done at object creation. It initialize the decoding
   * slot by reading the video native format and prepare decoding 
   * buffer creation. This method is always called by the main thread.
   *
   */
  int (*init)(struct WVStreamingObject* streamObj);

  /**
   * \brief Give a frame buffer
   *
   * \param streamObj The streaming object 
   * \param slotIdx The associated slot index 
   *
   * Return a decoding frame buffer. If static get method is used then 
   * this will be called one time per slots. If dynamic get is used then 
   * this will be called each time we decode a new frame in a slot.
   * Set to NULL if useless !
   */
  WVStreamingBuffer (*getBuffer)(struct WVStreamingObject* streamObj, int slotIdx);

  /**
   * \brief Lock the frame buffer
   *
   * \param streamObj The streaming object 
   * \param slotIdx The associated slot index    
   *
   * Lock the frame buffer so the decoder can write data on it.
   * If this action is not needed set to NULL.
   */
  int (*lockBuffer)(struct WVStreamingObject* streamObj, int slotIdx);



  /**
   * \brief Filter the buffer with a user defined method
   *
   * \param streamObj The streaming object 
   * \param slotIdx The associated slot index   
   * \param buffer The buffer previously given
   *
   * Apply a filter after the decoding step. **Need to be thread safe** it
   * will be called by the video decoder thread. If it's not, put the 
   * method in the releaseBuffer step. If this action is not needed set to NULL.
   *
   */
  int (*filterBuffer)(struct WVStreamingObject* streamObj, int slotIdx, WVStreamingBuffer* buffer);
  


  /**
   * \brief Release the buffer after writing
   *
   * \param streamObj The streaming object 
   * \param slotIdx The associated slot index    
   *   
   * Release the buffer after the decoder have writed 
   * data on it. If this action is not needed set to NULL.
   *
   */
  int (*releaseBuffer)(struct WVStreamingObject* streamObj, int slotIdx);

  /**
   * \brief Refresh the frame by displaying its content
   *
   * \param streamObj The streaming object 
   * \param slotIdx The associated slot index    
   *
   * Method used to display a decoded frame. This will always called by
   * the main thread by the *WV_refreshVideoFrame* function.
   * 
   */
  int (*refreshFrame)(struct WVStreamingObject* streamObj, int slotIdx);

  

  /**
   * \brief Close the object 
   *
   * \param streamObj The streaming object 
   *
   * Close the object internal data by freeing all the slots.
   */
  int (*close)(struct WVStreamingObject* streamObj);


  /****************/
  /* private data */
  /****************/
  
  /**
   * \brief Private data of the object
   *
   * Private data used by the methods of the object. 
   *
   */
  void* objPrivate;

}WVStreamingObject;



  







/*|||||||||||||||||||||||||||||||||*/
/*                                 */
/*        THE SYNC OBJECT          */
/*                                 */
/*|||||||||||||||||||||||||||||||||*/


/*****************************/
/* the clock data structure  */
/*****************************/

/**
 * \brief Clock status descriptor
 *
 * Give the status of a refence clock. Used in  
 * the *getRefClock* function of the sync object 
 * to obtain reference clock state.
 *
 */
typedef struct WVReferenceClock{
  
  /**
   * \brief The clock value in milliseconds
   *
   * The time value of a reference clock in millisecond. Generally
   * the time elapsed from the begining of the stream.
   *
   */  
  uint32_t clock;

  /** 
   *\brief Give the play/pause status
   *
   * The pauseFlag if set if the clock is paused
   * so its time value remain the same.
   *
   */
  int pauseFlag;    
  
  /**
   * \brief Clock modification counter
   *
   * Count the number of clock modification since
   * clock initialisation. This occur when decoder reach 
   * stream end, seek on the stream or change the playing stream.
   * This is used to know if we get the clock for the good chunk of the stream
   * by identifying each chunk to its index.
   */
  int modIdx;       

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

/** 
 *
 * \brief Synchronization method descriptor
 *
 * Describe the master and slave synchronization method. Classic use is 
 * audio/video sync but you can sync video to you own clock (master redefined)
 * or sync an object to the audio clock (slave redefined).
 */ 
typedef struct WVSyncObject{
  
  /************/
  /* EOF FLAG */
  /************/
  
  /**
   * \brief Looping method
   *
   * Set by the waave engine to follow stream parameters. This
   * flag give to the object methods if looping playback is 
   * enabled. Must be read to thread *End of File*.
   * Possible values are :
   *
   * looping flag       |   Description 
   * -------------------|--------------------------------
   * WV_BLOCKING_STREAM | When the stream reach *end of file* it stops 
   * WV_LOOPING_STREAM  | When the stream reach *end of file* it restart playing at the beginning
   */
  int loopingFlag;

  
  /*************/
  /* SLAVE DEF */
  /*************/

  /**
   * \brief Signal state change to the slave
   *
   * \param sync The sync object
   *
   * Defined by the slave for thread synchronization,
   * signalStateChange is called by
   * the clock generator to signal the slave for clock modification.
   * Usually it signal play, seek and EOF but it does not signal
   * pause because it can be read with pause flag.
   */ 
  void (*signalStateChange)(struct WVSyncObject* sync); 

  /**************/
  /* MASTER DEF */
  /**************/

  /* !!! getRefClock and play are mandatory !!! */
  /* !!! signalStateChange for play and seek but not for pause !!! */ 

  /* GETREFCLOCK */
  
  /**
   * \brief Return the master clock 
   *
   * \param sync The sync object 
   * \param refClock input/ouput clock descriptor 
   *   
   * Defined by the master it give the reference clock.
   * The slave fill refClock->clock and refClock->modIdx with 
   * its current stream position, so master clock can readjust
   * itself when modIdx change (When playing video only, it's the
   * video part, the slave, that receive the EOF or seek signal.
   * So it's give it to the master by setting the modIdx index.
   *
   * **Be carefull !** Master will get a UINT32_MAX clock if the slave can't determine his stream pos
   * **Be carefull !** If teh slave give modIdx=0, the getRefClock must return its clock value
   */
  void (*getRefClock)(struct WVSyncObject* sync, WVReferenceClock* refClock); 

  
  /* PLAY */

  /**
   * \brief Start master clock
   *
   * \param sync The sync object 
   *
   *
   * Defined by the master it start master clock if it was paused
   * or return -1 if the play command is useless (the clock already running)
   * It need to signal state change to the slave.
   *
   */
  int (*play)(struct WVSyncObject* sync);


  /* PAUSE */
  
  /** 
   * \brief Pause master clock
   *
   * \param sync The sync object 
   *
   * Defined by the master it pause master clock
   * or return -1 if the pause command is useless (the clock already paused)
   * This does not signal state change to the slave. This done with the
   * *pauseFlag* of teh reference clock.
   *
   */
  int (*pause)(struct WVSyncObject* sync);

  
  /* SEEK */
  
  /**
   * \brief Seek on master clock 
   *
   * \param sync The sync object 
   * \param clock The seek target clock
   * \param seekFlag Method used if clock is paused
   *
   * Defined by the master it realise seek operation
   * on master clock. This method need to signal state 
   * change to the slave and need to thread the given 
   * seek flag. 
   *
   * Seek flag        |   Description 
   * -----------------|--------------------------------
   * WV_BLOCKING_SEEK | If the clock is paused it stay paused after the seek 
   * WV_PLAYING_SEEK  | If the clock is paused it restart when a *seek* command is sent
   *
   */
  int (*seek)(struct WVSyncObject* sync, uint32_t clock, int seekFlag);
  

  /***********/
  /* PRIVATE */
  /***********/

  /**
   * \brief Method's private data
   *
   * Private data used by the methods.
   *
   */  
  void* objPrivate;            //private vars for the methods
  
  /**
   * \brief Private space for the waave engine
   *
   * Used by the waave engine to complete the user given
   * methods with standard methods like audio/video sync.
   * Don't access it.
   */
  void* wvStdPrivate;    


}WVSyncObject;





/*|||||||||||||||||||||||||||||||||||||||||||*/
/*                                           */
/*          THE STREAM PARAMETERS            */
/*                                           */
/*|||||||||||||||||||||||||||||||||||||||||||*/

/* the stream type */
#define WV_STREAM_TYPE_NONE 0
#define WV_STREAM_TYPE_AUDIO 1
#define WV_STREAM_TYPE_VIDEO 2
#define WV_STREAM_TYPE_AUDIOVIDEO 3

/* the play method */
#define WV_NEUTRAL_PLAY 0
#define WV_LOOPING_PLAY 1

/* the seek method */
#define WV_SEEK_BACKWARD -1
#define WV_SEEK_FORWARD 1


/* the stream struct */
struct WVStream;
typedef struct WVStream  WVStream;

/* to signal eof */
typedef  int (*WVEOFSignalCall)(struct WVStream* stream, void* param);


#define WAAVE_INIT_NONE 0
#define WAAVE_INIT_AUDIO 1
#define WAAVE_INIT_VIDEO 2



/*|||||||||||||||||||||||||||||||||||||||||||*/
/*                                           */
/*             THE WAAVE API                 */
/*                                           */
/*|||||||||||||||||||||||||||||||||||||||||||*/



/***********************************/
/* Start and stop the Waave engine */
/***********************************/

/** 
 * \defgroup stengine Start and stop the Waave engine
 *
 * Before using the Waave engine build a SDL context with support for video, audio an timers.
 * You can then start or stop earch component of the Waave engine. 
 *
 * @{
 */


/**
 * \brief Init the Waave engine
 *
 * \param flag Set the components to start, the others are stopped if running
 *
 * Use *WV_waaveInit* to start the Waave engine with the subsystems specified by flags. Here the  
 * possible flag value :
 *
 * Flag value       | Description
 * -----------------|----------------------------- 
 * WAAVE_INIT_NONE  | Nothing need to be started  
 * WAAVE_INIT_AUDIO | Audio engine need to be started
 * WAAVE_INIT_VIDEO | Video engine need to be started
 *
 * So :
 *
 *     WV_waaveInit(WAAVE_INIT_AUDIO|WAAVE_INIT_VIDEO)
 * 
 * will start all the engine components. 
 *
 */
int WV_waaveInit(int flag);


/**
 * \brief Close the Waave engine
 *
 * All the opened streams will be closed and the engine will be stopped. If you 
 * want to close just an engine component relaunch *WV_waaveInit* with the correct flags
 *
 */ 
int WV_waaveClose(void);


/** @} */






/***********************/
/* Stream manipulation */
/***********************/

/** 
 * \defgroup streammap Stream manipulation
 *
 * Loading an audio or video stream in the Waave engine is done in three steps. First **open** a file and check it's type.
 * Next you can **set** stream attributes to control the stream behavior. Finally **load** the stream so you can play it.  
 * @{
 */

/**
 * \brief Open an audio/video file and give a stream handle.
 *
 * \param filename The file you want to open
 *
 * To open a new stream just do a :
 *
 * \code
 * WVStream* newStream;
 * newStream = WV_getStream(filename);
 * \endcode
 *
 */
WVStream* WV_getStream(const char* filename);


/**
 * \brief Close an opened stream
 *
 * \param stream The stream you want to release
 *
 * The *WV_getStream* function return NULL on success, so the classic way to free a stream is :
 * \code
 * myStream = WV_closeStream(myStream)
 * \endcode
 */
WVStream* WV_closeStream(WVStream* stream);


/**
 * \brief Get the stream type : audio, video ...
 *
 * \param stream The stream you want the type
 *
 * Return the stream type. Possible returned values are :
 *
 * Flag value                | Description
 * --------------------------|----------------------------- 
 * WV_STREAM_TYPE_NONE       | Unknow stream type  
 * WV_STREAM_TYPE_AUDIO      | An audio only stream
 * WV_STREAM_TYPE_VIDEO      | An video only stream
 * WV_STREAM_TYPE_AUDIOVIDEO | A stream with audio and video
 *
 */
int WV_getStreamType(WVStream* stream);

/**
 * \brief Get the stream width 
 *
 * \param stream The stream you want the width
 * 
 * Return the stream width or -1 if the stream doesn't contain video
 *
 */
int WV_getStreamWidth(WVStream* stream);

/**
 * \brief Get the stream height 
 *
 * \param stream The stream you want the height
 * 
 * Return the stream width or -1 if the stream doesn't contain video
 *
 */
int WV_getStreamHeight(WVStream* stream);


/**
 * \brief Load the stream in the Waave engine
 *
 * \param stream The stream to load.
 *
 * Before playing a stream it need to be loaded in the Waave engine. *WV_closeStream* will unload it if needed.
 *
 */
int WV_loadStream(WVStream* stream);


/** @} */




/************************/
/* Display video frames */
/************************/

/** 
 * \defgroup  displayvideo Display video frames
 *
 * The methods to display the video frames is described
 * in the streaming object. But to display each of them
 * you need to call the *WV_refreshVideoFrame* when you
 * receive the SDL *WV_REFRESH_EVENT*
 *   
 * @{
 */

/* the default WV_REFRESH_EVENT value */
#if SDL_VERSION_ATLEAST(2,0,0)

extern int dynamic_wv_refresh_event;  //dynamic, see waave_engine_flags.c
#define WV_REFRESH_EVENT dynamic_wv_refresh_event

#else

#define WV_REFRESH_EVENT SDL_NUMEVENTS - 1

#endif


/**
 * \brief Display a video frame
 *
 * \param refreshEvent The reveived refresh event
 *
 * Display a video frame of a stream using the method
 * discribed in the corresponding streaming object.
 * Call this function each time you receive the 
 * *WV_REFRESH_EVENT* to play video. Use a simple 
 * test like this :
 * \code
 * SDL_Event event;
 * while( SDL_PollEvent( &event ) ) {
 *     switch( event.type ) {
 *     case WV_REFRESH_EVENT:
 *     WV_refreshVideoFrame(&event);
 *     break;
 *     }
 * }
 *
 * \endcode
 */
void WV_refreshVideoFrame(SDL_Event* refreshEvent);

/** @} */



/*************************/
/* Set stream parameters */
/*************************/


/** 
 * \defgroup streamparam Set stream parameters
 *
 * Before loading a stream in the Waave engine you can set parameters that
 * control the stream behavior. 
 * **Be carefull !** For a video stream, setting the streaming method is mandatory !
 * 
 *
 * @{
 */

/**
 * \brief Disable audio playing in a stream
 *
 * \param stream The stream where the audio will be disabled.
 *
 * You can use the *WV_disableAudio* function when you want only play the video data of an audio/video file.
 * Check the stream type before doing this, it will change after the call.
 * 
 */
int WV_disableAudio(WVStream* stream);

/**
 * \brief Disable video playing in a stream
 *
 * \param stream The stream where the video will be disabled.
 *
 * You can use the *WV_disableAudio* function when you want only play the audio data of an audio/video file.
 * Check the stream type before doing this, it will change after the call.
 * 
 */
int WV_disableVideo(WVStream* stream);

/**
 * \brief Set the stream behavior when a *play* command is sent 
 *
 * \param stream The stream you want to set the play method
 * \param playFlag The wanted *play* behavior
 *
 * This function set the playing behavior when the *play* command is sent to the stream. It can be useful when you have 
 * an audio file used as sound effect and you want it to be played each time an event occur. 
 *
 * Possible behavior are :
 *
 * Play method     |   Description 
 * ----------------|--------------------------------
 * WV_NEUTRAL_PLAY | If the stream already playing, the *play* command do nothing (default)
 * WV_LOOPING_PLAY | Each time the *play* command is sent, the stream restart to the beginning
 *
 */
int WV_setPlayMethod(WVStream* stream, int playFlag);



/**
 * \brief Set the stream behavior when a *seek* command is sent 
 *
 * \param stream The stream you want to set the seek method
 * \param seekFlag The wanted *seek* behavior
 *
 * This function set the seeking behavior when the *seek* command is sent to the stream. 
 * Possible behavior are :
 *
 * Seek method      |   Description 
 * -----------------|--------------------------------
 * WV_BLOCKING_SEEK | If the stream is paused it stay paused after the seek (default)
 * WV_PLAYING_SEEK  | If the stream is paused it restart playing when a *seek* command is sent
 *
 */
int WV_setSeekMethod(WVStream* stream, int seekFlag);


/* the eof signal */
#if SDL_VERSION_ATLEAST(2,0,0)

extern int dynamic_wv_eof_event;  //dynamic, see waave_engine_flags.c
#define WV_EOF_EVENT dynamic_wv_eof_event

#else

#define WV_EOF_EVENT SDL_NUMEVENTS - 2

#endif


/**
 * \brief Set the stream behavior when *end of file* is reached
 *
 * \param stream  The stream you want to set the EOF method
 * \param loopingFlag  The wanted *EOF* behavior
 *
 * This function set the stream behavior when the *end of file* is reached. 
 * Possible behavior are :
 *
 * EOF method         |   Description 
 * -------------------|--------------------------------
 * WV_BLOCKING_STREAM | When the stream reach *end of file* it stops (default)
 * WV_LOOPING_STREAM  | When the stream reach *end of file* it restart playing at the beginning
 *
 */
int WV_setEOFMethod(WVStream* stream, int loopingFlag);



/**
 * \brief Associate a streaming object with an opened stream.
 *
 * \param stream A video stream
 * \param streamObj The streaming object to associate with the stream
 *
 * For a stream that contain video data, it's absolutly mandatory to give a streaming object that describe
 * the methods to displaying video frames ! Waave library comes with standard streaming objects 
 * (see relative documentation). **Be carefull** to only use **one** streaming object instance for 
 * **one** video stream ! Check the object documentation to know how to create your own streaming
 * objects.
 *
 */   
int WV_setStreamingMethod(WVStream* stream, WVStreamingObject* streamObj);



/**
 * \brief Return the streaming object associated with the stream.
 *
 * \param stream A video stream
 * 
 * Give the streaming object associated with a stream by the ::WV_setStreamingMethod
 * command.
 *
 */
 WVStreamingObject* WV_getStreamingMethod(WVStream* stream);


/**
 * \brief Apply a user defined synchronisation method to a stream
 *
 * \param stream The synchronized stream
 * \param syncObj The user defined sync method
 *
 * To know how to create you own streaming object check the object documentation.
 *
 */
int WV_setSyncMethod(WVStream* stream, WVSyncObject* syncObj);


/**
 * \brief Return the synchronisation method associated with the stream.
 *
 * \param stream A synchronized stream
 * 
 * Give the synchronisation method associated with a stream with the ::WV_setSyncMethod
 * command.
 *
 */
 WVSyncObject* WV_getSyncMethod(WVStream* stream);


/**
 * \brief Set the parameter that will be sent with the stream when *end of file* is reached 
 *
 * \param stream The stream you set the parameter
 * \param param The sended pointer to the user defined data
 *
 * With the *WV_setSyncMethod* you can set you own stream descriptor that will be 
 * returned to you when *EOF* is reached. If the stream signal *EOF* by sending an 
 * WV_EVENT_EOF_SIGNAL event then the parameter will be found in **event.user.data2**.
 * Else, if the stream signal *EOF* with a user defined function, then *param* will
 * be passed to this function.
 *
 */
int WV_setEOFSignalParam(WVStream* stream, void* param);


/**
 * \brief Enable callback to signal *end of file*
 *
 * \param stream  The stream where eof signal callback will be enabled
 * \param eofCall The callback function that will be called when EOF is reached
 *
 * Set a callback function that will be called when the *end of file* of a stream
 * is reached. Be carefull that, once done, the WV_EVENT_EOF_SIGNAL is no longer sended.
 * The *eofCall* as to be of type :
 *
 *     int (*WVEOFSignalCall)(struct WVStream* stream, void* param)
 *
 * You can set the *param* that will be sent to the callback function 
 * with *WV_setEOFSignalParam*.
 *
 * **Be carefull !** The callback function 
 * will be called by the waave engine thread ! To call your function with
 * the main thread, use the WV_EVENT_EOF_SIGNAL.
 *
 */
int WV_setEOFSignalCall(WVStream* stream, WVEOFSignalCall eofCall);


/**
 * \brief Set the volume exponentiation factor
 *
 * \param stream A stream with audio data
 * \param DBPitche The exponentiation factor used to compute DB volume
 *
 * To apply volume, audio data is multiplied by a volume factor \f$f_v\f$ which depend
 * of the DBvolume \f$v\f$ and the internal DBPitche factor \f$f_{db}\f$ with 
 * the above fomula :
 *
 * \f[ f_v = \left( f_{db} \right)^{v} \f]  
 */
int WV_setVolumeDBPitche(WVStream* stream, double DBPitche);


/**
 * \brief Get the volume exponentiation factor
 *
 * \param stream The stream with get exponentiation factor.
 *
 * Return the stream's volume DB pitche.
 *
 */
double WV_getVolumeDBPitche(WVStream* stream);

/** @} */






/***************************/
/* Stream playback control */
/***************************/

/** 
 * \defgroup streamcontrol Stream playback control
 *
 * Here the functions that control stream playback. Note that the commands behavior
 * is influenced by the stream parameters.
 *  
 * @{
 */

/**
 * \brief Start stream playback
 *
 * \param stream The stream that receive the play commmand
 *
 * Start stream playback if it was paused. If playback already running, the play command
 * behavior depend of the selected play method. See ::WV_setPlayMethod. The function return
 * -1 if the play command was useless and 0 otherwise.
 *
 */
int WV_playStream(WVStream* stream);


/**
 * \brief Pause stream playback
 *
 * \param stream The stream that receive the pause commmand
 *
 * Pause stream playback while keeping the current reading position. Note that 
 * send the pause command again will not relaunch playback. But depending
 * on the seek method, seeking can unpause the stream. See ::WV_setSeekMethod.
 *
 */
int WV_pauseStream(WVStream* stream);


 /**
  * \brief Stop stream playback and return to start
  *
  * \param stream The stopped stream
  *
  * Stop stream playback and put the reading position at the stream beginning.
  * The stream will be paused, even if you have set the WV_PLAYING_SEEK flag.
  *
  */
int WV_stopStream(WVStream* stream);


/**
 * \brief Move playback to the stream beginning
 *
 * \param stream The stream that receive the rewind commmand
 *
 * Reset playback position at the stream beginning. If the stream was
 * paused the command behavior depend on the seek method. See ::WV_setSeekMethod.
 */
int WV_rewindStream(WVStream* stream);


/**
 * \brief Relative seek command
 *
 * \param stream The stream that receive the relative seek command
 * \param seekShift The seek step in milliseconds
 * \param seekDirection The direction where we move playback position (see below)
 *
 * Seek on the stream relative to the current playback position. The *seekShift* parameter
 * control for how many millisecond we seek and the two possible seeking direction 
 * are given by the folowing values :
 *
 * Seek direction   | Description
 * -----------------|---------------------------------------
 * WV_SEEK_BACKWARD | seek in the reverse playback direction
 * WV_SEEK_FORWARD  | seek in the playback direction
 *
 * If the stream was paused the command behavior depend 
 * on the seek method. See ::WV_setSeekMethod. 
 * 
 */
int WV_rseekStream(WVStream* stream, uint32_t seekShift, int seekDirection);


/**
 * \brief The seek command
 *
 * \param stream The stream that receive the seek command
 * \param clock The stream position where playback is moved in milliseconds
 *
 * Seek on the stream giving the absolute position. Use ::WV_getStreamDuration to 
 * know the maximum clock value and ::WV_getStreamClock to know the current 
 * playback position. If the given stream position is after the stream end 
 * then playback restart to the beginning.
 *
 */
int WV_seekStream(WVStream* stream, uint32_t clock);


/**
 * \brief Give total stream duration 
 *
 * \param stream The stream
 * 
 * Give the total stream duration in milliseconds.
 *
 */
uint32_t WV_getStreamDuration(WVStream* stream);


/**
 * \brief Give the current playback position
 *
 * \param stream The stream
 *
 * Give the current playback position in milliseconds.
 * 
 */

 
uint32_t WV_getStreamClock(WVStream* stream);


/** @} */






/*************************/
/* Stream volume control */
/*************************/

/** 
 * \defgroup volumecontrol Stream volume control
 *
 * Here the functions that control volume on streams that contain audio data.
 * The volume is the multiplicator coefficient that is applied to audio samples.
 * So a 1.0 volume value doesn't affect the original data.
 * Note that the commands behavior is influenced by the stream parameters.
 *  
 * @{
 */


/**
 * \brief Set stream volume
 *
 * \param stream The stream where we adjust volume
 * \param volume The new value of volume
 *
 * Change the volume value of a stream. 
 *
 */
int WV_setVolume(WVStream* stream, double volume);


/**
 * \brief Get stream volume
 *
 * \param stream The stream we get volume 
 *
 * Get the current volume of a stream.
 *
 */
  double WV_getVolume(WVStream* stream);


/**
 * \brief Change the volume in db unit
 *
 * \param stream The stream where we adjust volume
 * \param DBVolume The new volume in db
 *
 * Change the stream volume using the db unit. This volume setting manner 
 * take on account the logarithmic human audio perception. So a 0.0
 * value mean that volume stay unchanged. Positive value encrease the original volume
 * and negative value decrease it. 
 *
 * This command if influenced by the choice of the db pitche. See ::WV_setVolumeDBPitche.
 *
 */
int WV_setDBVolume(WVStream* stream, int DBVolume);


/**
 * \brief Get stream volume in db unit
 *
 * \param stream The stream we get db volume 
 *
 * Get the current volume of a stream in db unit.
 *
 */
int WV_getDBVolume(WVStream* stream);


/**
 * \brief Increase of decrease the volume in db unit
 *
 * \param stream The stream where we adjust volume
 * \param shift The encreasing or decreasing value
 *
 * Encrease on decrease the volume db value by the value given in *shift*.
 *
 */
int WV_shiftDBVolume(WVStream* stream, int shift);


/** @} */







/**************************************/
/* Standard SDL 1.2 streaming objects */
/**************************************/

/** 
 * \defgroup strandardoverlay Standard SDL 1.2 streaming objects
 *
 * There function permit to create standard streaming objects to stream
 * video on sdl surface using YUV **overlay** or simply using **surface**. Only for SDL 1.2 !
 *  
 * @{
 */

#if !SDL_VERSION_ATLEAST(2,0,0)

  /****************/
  /*   overlays   */
  /****************/

/**
 * \brief Get a new overlay streaming object
 * 
 * \param targetSurface The sdl surface where we want to stream video data.
 * ** Need to be the screen surface ! **
 * \param destRect The destination rectangle. All the surface if NULL;
 * 
 * Create a new streaming object for the sdl **screen** surface. Be carefull to 
 * use it only for one video stream !
 *
 */
WVStreamingObject* WV_getStreamOverlayObj(SDL_Surface* targetSurface, SDL_Rect* destRect);

/**
 * \brief Change the target sdl surface of a overlay streaming object
 *
 * \param streamObj The modified streaming object
 * \param targetSurface The new target surface
 * \param destRect The new destination rectangle. May be NULL.
 *
 * It's possible to change the target surface of a streaming object without
 * recreate it. This can be done at any time ! Even if the stream is playing.
 *
 */
void WV_resetStreamOverlayOutput(WVStreamingObject* streamObj, SDL_Surface* targetSurface, SDL_Rect* destRect);



/**
 * \brief Free a overlay streaming object
 *
 * \param streamObj The released streaming object
 *
 * Free a streaming object created with ::WV_getStreamOverlayObj. Be carefull
 * that the streaming object pointer is not reset.
 *
 */
void WV_freeStreamOverlayObj(WVStreamingObject* streamObj);



  /****************/
  /*   surface    */
  /****************/

/**
 * \brief Get a new surface streaming object
 * 
 * \param targetSurface The sdl surface where we want to stream video data.
 * ** The surface you want ! **
 * \param destRect The destination rectangle. All the surface if NULL;
 * \param updateFlag Set it if you want that waave update the target surface with SDL_UpdateRect 
 * 
 * Create a new streaming object for an sdl surface. Be carefull to 
 * use it only for one video stream !
 *
 */
WVStreamingObject* WV_getStreamSurfaceObj(SDL_Surface* targetSurface, SDL_Rect* destRect, int updateFlag);

/**
 * \brief Change the target sdl surface of a surface streaming object
 *
 * \param streamObj The modified streaming object
 * \param targetSurface The new target surface
 * \param destRect The new destination rectangle. May be NULL.
 *
 * It's possible to change the target surface of a streaming object without
 * recreate it. This can be done at any time ! Even if the stream is playing.
 *
 */
void WV_resetStreamSurfaceOutput(WVStreamingObject* streamObj, SDL_Surface* targetSurface, SDL_Rect* destRect);


/**
 * \brief Free an surface streaming object
 *
 * \param streamObj The released streaming object
 *
 * Free a streaming object created with ::WV_getStreamSurfaceObj. Be carefull
 * that the streaming object pointer is not reset.
 *
 */
void WV_freeStreamSurfaceObj(WVStreamingObject* streamObj);



#endif

/** @} */




/**************************************/
/* Standard SDL 2.0 streaming objects */
/**************************************/

/** 
 * \defgroup strandardrenderer Standard SDL 2.0 streaming objects
 *
 * There functions permit to create standard streaming objects to stream
 * video with a sdl renderer using textures. The renderer is not cleared 
 * by the object so do it one time before start streaming. 
 *
 * Don't forget to reset the object and clear the renderer when the 
 * renderer's viewport is modified !
 * 
 * Only for SDL 2.0 or greater !
 * 
 *  
 * @{
 */


#if SDL_VERSION_ATLEAST(2,0,0)


/**
 * \brief Get a new renderer streaming object
 * 
 * \param targetRenderer The sdl renderer we use to stream video data
 * \param destRect The destination rectangle. The entire rendering target if NULL;
 * \param updateFlag Set it if you want that waave update the target surface with SDL_RenderPresent 
 * 
 * Create a new streaming object for a sdl renderer. Be carefull to 
 * use it only for one video stream !
 *
 */
  WVStreamingObject* WV_getStreamRendererObj(SDL_Renderer* targetRenderer, SDL_Rect* destRect, int updateFlag);


/**
 * \brief Change/update the renderer of a renderer streaming object
 *
 * \param streamObj The modified/updated streaming object
 * \param targetRenderer The new/updated renderer
 * \param destRect The new/updated destination rectangle. May be NULL.
 *
 * Call this function when the renderer's viewport is modified !
 * It's also possible to change the renderer of a streaming object without
 * recreate it. This can be done at any time ! Even if the stream is playing.
 *
 */
void WV_resetStreamRendererOutput(WVStreamingObject* streamObj, SDL_Renderer* targetRenderer, SDL_Rect* destRect);



/**
 * \brief Free a renderer streaming object
 *
 * \param streamObj The released streaming object
 *
 * Free a streaming object created with ::WV_getStreamRendererObj. Be carefull
 * that the streaming object pointer is not reset.
 *
 */
void WV_freeStreamRendererObj(WVStreamingObject* streamObj);

#endif

/** @} */



#ifdef __cplusplus
}
#endif


#endif
