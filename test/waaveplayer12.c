#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <WAAVE.h>

/* input file */
char* filename;


/* screen proprieties */
int screenWidth;
int screenHeight;
int bpp;


/* window surface */
int flags;
SDL_Surface* screen;
int winWidth;
int winHeight;
int fullscreenFlag = 0;


/* playing stream */
WVStream* stream = NULL;
WVStreamingObject* streamObj = NULL;


/* close sequence */
void closePlayer(void)
{
  /* close the opened stream. not mandatory  */
  if(stream)
    WV_closeStream(stream);

  /* close the streaming object */
  if(streamObj)
    WV_freeStreamOverlayObj(streamObj);
  
  /* close the waave engine */
  WV_waaveClose();

  /* close sdl */
  SDL_Quit();
  exit(0);
}


/* check key */
void keyPressed( SDL_keysym* keysym  ) 
{
 
  switch ( keysym->sym ) {

  case SDLK_ESCAPE:
    closePlayer();  //close
    break;
    
  case SDLK_SPACE:
    WV_playStream(stream); //play
    break;

  case SDLK_BACKSPACE:
    WV_pauseStream(stream);  //pause
    break;

  case SDLK_KP_PLUS:
    WV_shiftDBVolume(stream, +1); //encrease volume
    break;

  case SDLK_KP_MINUS:
    WV_shiftDBVolume(stream, -1); //decrease volume
    break;

  case SDLK_f:
    if(!fullscreenFlag){   //toggle fullscreen
      fullscreenFlag = 1;
      SDL_ShowCursor(SDL_DISABLE);
      screen = SDL_SetVideoMode(screenWidth, screenHeight, bpp, flags ^ SDL_FULLSCREEN);
      if(streamObj)
	WV_resetStreamOverlayOutput(streamObj, screen, NULL); 
    }
    else{
      fullscreenFlag = 0;
      SDL_ShowCursor(SDL_ENABLE);
      screen = SDL_SetVideoMode(winWidth, winHeight, bpp, flags);
      if(streamObj)
	WV_resetStreamOverlayOutput(streamObj, screen, NULL);
      
    }
    break;

    
    /********/
    /* seek */
    /********/
    uint32_t seekShift; //will be converted in milliseconds
    int seekDirection;

  case SDLK_RIGHT:
    seekShift = 10;
    seekDirection = WV_SEEK_FORWARD; 
    goto do_seek;

  case SDLK_LEFT:
    seekShift = 10;
    seekDirection = WV_SEEK_BACKWARD;
    goto do_seek;

  case SDLK_UP:
    seekShift = 60;
    seekDirection = WV_SEEK_FORWARD; 
    goto do_seek;

  case SDLK_DOWN:
    seekShift = 60;
    seekDirection = WV_SEEK_BACKWARD; 
    goto do_seek;

  do_seek:
    seekShift *= 1000;
    WV_rseekStream(stream, seekShift, seekDirection);
    break;
     
  default:
    break;
  }
}


/* sdl events */
void processEvents(void)
{
  SDL_Event event;
  while( SDL_WaitEvent( &event ) ) {
    switch( event.type ) {
    
    case SDL_KEYDOWN:
      keyPressed( &event.key.keysym );
      break;
      
    case SDL_VIDEORESIZE:
      if(!fullscreenFlag){
	winWidth = event.resize.w;
	winHeight = event.resize.h;
	screen = SDL_SetVideoMode(winWidth, winHeight, bpp, flags);
	if(streamObj)
	  WV_resetStreamOverlayOutput(streamObj, screen, NULL);
      }  
      break;
      
    case WV_REFRESH_EVENT:
      WV_refreshVideoFrame(&event);
      break;
      
    case WV_EOF_EVENT:
      closePlayer();
      break;

    case SDL_QUIT:
      closePlayer();
      break;
    }
  }
}



int main(int argc, char** argv)
{
  if(argc != 2){
    printf("usage : %s vidfilename\n", argv[0]);
    return 0;
  }

  /* parse arg */
  filename = argv[1];
  

  /* the window will be resized */
  /* after reading video size */
  winWidth = 640;
  winHeight = 480;

  /************/
  /* init sdl */
  /************/
  if( SDL_Init( SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER ) < 0 ) {
    fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
    SDL_Quit( );
    exit(0);
  }

  /* get video information */
  const SDL_VideoInfo* info = NULL;
  info = SDL_GetVideoInfo( );
  if( !info ) {
    fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
    SDL_Quit( );
    exit(0);
  }

  /* set windows attributes */
  bpp = info->vfmt->BitsPerPixel;
  screenWidth = info->current_w;
  screenHeight = info->current_h;
  flags = SDL_RESIZABLE;

  /* create display */
  if( !(screen = SDL_SetVideoMode(winWidth, winHeight, bpp, flags) ) ) {
    fprintf( stderr, "Video mode set failed: %s\n", SDL_GetError( ) );
    SDL_Quit( );
    exit(0);
  }
  
  /**************/
  /* init waave */
  /**************/

  /* start waave and load file */
  WV_waaveInit(WAAVE_INIT_AUDIO|WAAVE_INIT_VIDEO);
  stream = WV_getStream(filename);
  
  /* build a streaming object for the screen surface */
  int streamType = WV_getStreamType(stream);
  if(streamType == WV_STREAM_TYPE_VIDEO || streamType == WV_STREAM_TYPE_AUDIOVIDEO){
    streamObj = WV_getStreamOverlayObj(screen, NULL);
    WV_setStreamingMethod(stream, streamObj);
  }

  /* load the stream */
  WV_loadStream(stream);
  
  /* reset window size for video */
  if(streamObj){
    winWidth = streamObj->srcWidth;
    winHeight = streamObj->srcHeight;
    screen = SDL_SetVideoMode(winWidth, winHeight, bpp, flags);
    WV_resetStreamOverlayOutput(streamObj, screen, NULL);
  }

  /* at start the stream is paused */
  /* so we launch playback */
  WV_playStream(stream);
  
  /*****************/
  /* display loop  */
  /*****************/
  while( 1 ) {
    processEvents( );
  }
  return(0);
}




