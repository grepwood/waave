#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <WAAVE.h>

/* screen proprieties */
int screenWidth;
int screenHeight;
int bpp;


/* window surface */
int flags;
SDL_Surface* screen;
int winWidth;
int winHeight;


/* check key */
void keyPressed( SDL_keysym* keysym  ) 
{
 
  switch ( keysym->sym ) {

  case SDLK_ESCAPE:
    /* close sdl */
    SDL_Quit();
    exit(0);
    break;
      
  default:
    break;
  }
}


/* sdl events */
void processEvents(void)
{
  SDL_Event event;
  while( SDL_PollEvent( &event ) ) {
    switch( event.type ) {
    
    case SDL_KEYDOWN:
      keyPressed( &event.key.keysym );
      break;
      
    case SDL_VIDEORESIZE:
      winWidth = event.resize.w;
      winHeight = event.resize.h;
      screen = SDL_SetVideoMode(winWidth, winHeight, bpp, flags);
      break;
      
    case SDL_QUIT:
      /* close sdl */
      SDL_Quit();
      exit(0);
      break;
    }
  }
}



int main(int argc, char** argv)
{

  /* default window size */
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
  
    
  /*****************/
  /* display loop  */
  /*****************/
  while( 1 ) {
    processEvents( );
  }
  return(0);
}




