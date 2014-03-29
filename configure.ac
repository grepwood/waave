#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([waave], [3.01], [waave-user@lists.sourceforge.net])
AM_INIT_AUTOMAKE
LT_INIT

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/audio_decoder_stops.c])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_INSTALL

#Checks for SDL library.
AX_CHECK_SDL 

#check for the ffmpeg library
AX_CHECK_FFMPEG


# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC

#set cflags and libs  
CFLAGS="${SDL_CFLAGS} ${FFMPEG_CFLAGS} ${CFLAGS}"
LIBS="${SDL_LIBS} ${FFMPEG_LIBS} ${LIBS}" 


AC_CONFIG_FILES([Makefile
                 src/Makefile])


AC_OUTPUT

AC_MSG_RESULT([
----------------------------------

Waave will be build with :

SDL_CFLAGS = $SDL_CFLAGS
SDL_LIBS = $SDL_LIBS

FFMPEG_CFLAGS = $FFMPEG_CFLAGS
FFMPEG_LIBS = $FFMPEG_LIBS

CFLAGS = $CFLAGS

----------------------------------
])


