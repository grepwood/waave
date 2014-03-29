#ifndef WAAVE_CONFIG_SDL_H
#define WAAVE_CONFIG_SDL_H

#if HAVE_CONFIG_H
#include <config.h>
#endif 

#if HAVE_SDL_H
#include <SDL.h>
#elif HAVE_SDL2_SDL_H
#include <SDL2/SDL.h> 
#elif HAVE_SDL_SDL_H
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#endif
