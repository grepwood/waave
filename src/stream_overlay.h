#ifndef STREAM_OVERLAY_H
#define STREAM_OVERLAY_H

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

#include "streaming_object.h"

/*!!!!!!!!!!!!!!!!!!!!!!!*/
/*    only for SDL 1.2   */
/*!!!!!!!!!!!!!!!!!!!!!!!*/
#if !SDL_VERSION_ATLEAST(2,0,0)

/* get the object */
WVStreamingObject* WV_getStreamOverlayObj(SDL_Surface* targetSurface, SDL_Rect* destRect);

/* reset */
void WV_resetStreamOverlayOutput(WVStreamingObject* streamObj, SDL_Surface* targetSurface, SDL_Rect* destRect);


/* free the object */
void WV_freeStreamOverlayObj(WVStreamingObject* streamObj);

#endif


#endif
