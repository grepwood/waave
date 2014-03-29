AC_DEFUN([AX_CHECK_SDL],
[ax_sdl_save_CFLAGS="${CFLAGS}"
 ax_sdl_save_LIBS="${LIBS}"
 
 AC_CHECK_HEADERS([windows.h])
 
 dnl -----------------------
 dnl check for sdl headers
 dnl -----------------------
 ax_cv_check_sdl_headers=no
 
 AC_ARG_WITH(SDL-headers,
    AC_HELP_STRING([--with-SDL-headers=DIR],
                   [Directory where SDL headers are (optional)]),
    SDL_headers_dir="$withval")

dnl -- try user given dir
 if test "X${SDL_headers_dir}" != "X"; then
   AC_CHECK_HEADERS([${SDL_headers_dir}/SDL.h],[
     ax_cv_check_sdl_headers=yes
     SDL_CFLAGS="-I${SDL_headers_dir}"])
 fi

dnl-- try with no added dir 
 if test "X${ax_cv_check_sdl_headers}" = "Xno"; then 
   AC_CHECK_HEADERS([SDL.h], [ax_cv_check_sdl_headers=yes])
 fi

dnl-- try SDL2/ dir
 if test "X${ax_cv_check_sdl_headers}" = "Xno"; then 
   AC_CHECK_HEADERS([SDL2/SDL.h], [ax_cv_check_sdl_headers=yes])
 fi

dnl-- try SDL/ dir 
 if test "X${ax_cv_check_sdl_headers}" = "Xno"; then 
   AC_CHECK_HEADERS([SDL/SDL.h], [ax_cv_check_sdl_headers=yes])
 fi
 
dnl -- signal on error 
if test "X${ax_cv_check_sdl_headers}" = "Xno"; then
echo "-------------------------------"
echo " !!!                       !!! "
echo "    cannot find SDL headers    "
echo "  check your SDL installation  "
echo " !!!                       !!! "
echo "-------------------------------"
(exit 1); exit 1;
fi


 if test "X${ax_cv_check_sdl_headers}" = "Xyes"; then 
   dnl ---------------------------
   dnl    check for sdl library 
   dnl ---------------------------
   CFLAGS="${SDL_CFLAGS} ${ax_sdl_save_CFLAGS}"

   AC_CACHE_CHECK([for SDL library], [ax_cv_check_libsdl],
   [ax_cv_check_libsdl="no"
	
dnl --try without flags

   AC_LINK_IFELSE([[
#if HAVE_WINDOWS_H && defined(_WIN32)
#   include <windows.h>
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

int main(int argc, char** argv)
{
    SDL_Init( SDL_INIT_VIDEO );
}
]],[ax_cv_check_libsdl="yes"])


dnl -- try with -lSDL2
if test "X${ax_cv_check_libsdl}" = "Xno"; then

   LIBS="-lSDL2 ${ax_sdl_save_LIBS}"	

   AC_LINK_IFELSE([[
#if HAVE_WINDOWS_H && defined(_WIN32)
#   include <windows.h>
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

int main(int argc, char** argv)
{
    SDL_Init( SDL_INIT_VIDEO );
}
]],[ax_cv_check_libsdl="yes"
    SDL_LIBS="-lSDL2"])

fi

dnl -- try with -lSDL
if test "X${ax_cv_check_libsdl}" = "Xno"; then

   LIBS="-lSDL ${ax_sdl_save_LIBS}"	

   AC_LINK_IFELSE([[
#if HAVE_WINDOWS_H && defined(_WIN32)
#   include <windows.h>
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

int main(int argc, char** argv)
{
    SDL_Init( SDL_INIT_VIDEO );
}
]],[ax_cv_check_libsdl="yes"
    SDL_LIBS="-lSDL"])

fi

dnl -- try with -lmingw32 -lSDLmain -lSDL -mwindows
if test "X${ax_cv_check_libsdl}" = "Xno"; then

   LIBS="-lmingw32 -lSDLmain -lSDL -mwindows ${ax_sdl_save_LIBS}"	

   AC_LINK_IFELSE([[
#if HAVE_WINDOWS_H && defined(_WIN32)
#   include <windows.h>
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

int main(int argc, char** argv)
{
    SDL_Init( SDL_INIT_VIDEO );
}
]],[ax_cv_check_libsdl="yes"
    SDL_LIBS="-lSDL"])

fi
])

fi

if test "X${ax_cv_check_libsdl}" = "Xno"; then
echo "-------------------------------"
echo " !!!                       !!! "
echo "  cannot find the SDL library  "
echo "  check your SDL installation  "
echo " !!!                       !!! "
echo "-------------------------------"
(exit 1); exit 1;
fi


CFLAGS=${ax_sdl_save_CFLAGS}
LIBS=${ax_sdl_save_LIBS}

AC_SUBST([SDL_CFLAGS])
AC_SUBST([SDL_LIBS])


])



