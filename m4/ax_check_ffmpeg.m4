AC_DEFUN([AX_CHECK_FFMPEG],
[ax_ffmpeg_save_CFLAGS="${CFLAGS}"
 ax_ffmpeg_save_LIBS="${LIBS}"
 
 AC_CHECK_HEADERS([windows.h])
 FFMPEG_CFLAGS=" " 

 dnl -----------------------
 dnl check for ffmpeg headers
 dnl -----------------------
 
 AC_ARG_WITH(ffmpeg-headers,
    AC_HELP_STRING([--with-ffmpeg-headers=DIR],
                   [Directory where ffmpeg headers are (optional)]),
    ffmpeg_headers_dir="$withval")

dnl -- add user given dir
 if test "X${ffmpeg_headers_dir}" != "X"; then
   FFMPEG_CFLAGS="-I${ffmpeg_headers_dir}"
 fi

 CFLAGS="${FFMPEG_CFLAGS} ${CFLAGS}"

dnl-- check 
 ax_cv_check_ffmpeg_headers=yes
 AC_CHECK_HEADERS([libavcodec/avcodec.h],[ ], ax_cv_check_ffmpeg_headers=no)
 AC_CHECK_HEADERS([libavformat/avformat.h],[ ], ax_cv_check_ffmpeg_headers=no)
 AC_CHECK_HEADERS([libswscale/swscale.h])
 AC_CHECK_HEADERS([libswresample/swresample.h])
 AC_CHECK_HEADERS([libavcodec/audioconvert.h])

 if test "X${ax_cv_check_ffmpeg_headers}" = "Xno"; then
echo "------------------------------"
echo " !!!                     !!!  "
echo "  cannot find libav headers   "
echo "     check include paths      "
echo " !!!                     !!!  "
echo "------------------------------"    
(exit 1); exit 1;
 fi

dnl -----------------------
dnl check ffmpeg library
dnl -----------------------
FFMPEG_LIBS=" "
AC_CHECK_LIB([avutil], [av_malloc],[FFMPEG_LIBS="-lavutil ${FFMPEG_LIBS}"])
AC_CHECK_LIB([avformat], [avformat_version],[FFMPEG_LIBS="-lavformat ${FFMPEG_LIBS}"],[ ],[-lavcodec])
AC_CHECK_LIB([avcodec], [avcodec_alloc_frame],[FFMPEG_LIBS="-lavcodec ${FFMPEG_LIBS}"])
AC_CHECK_LIB([swscale], [sws_scale],[FFMPEG_LIBS="-lswscale ${FFMPEG_LIBS}"])
AC_CHECK_LIB([swresample], [swr_convert],[FFMPEG_LIBS="-lswresample ${FFMPEG_LIBS}"])

dnl-- check AVFORMAT
AC_CHECK_TYPES([enum AVMediaType], [], [], [[#include <libavcodec/avcodec.h>]])
AC_CHECK_LIB([avformat], [avformat_open_input],[AC_DEFINE([HAVE_AVFORMAT_OPEN_INPUT], [1],[Define if avformat_open_input exist in libavformat])],[ ],[-lavcodec])
dnl-- AC_CHECK_LIB([avformat], [av_open_input_file],[AC_DEFINE([HAVE_AV_OPEN_INPUT_FILE], [1],[Define if av_open_input_file exist in libavformat])],[ ],[-lavcodec])
AC_CHECK_LIB([avformat], [avformat_close_input],[AC_DEFINE([HAVE_AVFORMAT_CLOSE_INPUT], [1],[Define if avformat_close_input exist in libavformat])],[ ],[-lavcodec])
AC_CHECK_LIB([avformat], [avformat_find_stream_info], [AC_DEFINE([HAVE_AVFORMAT_FIND_STREAM_INFO], [1],[Define if avformat_find_stream_info exist in libavformat])],[ ],[-lavcodec])
AC_CHECK_LIB([avformat], [av_find_best_stream],[AC_DEFINE([HAVE_AV_FIND_BEST_STREAM], [1],[Define if av_find_best_stream exist in libavformat])],[ ],[-lavcodec])
AC_CHECK_LIB([avformat], [avformat_free_context],[AC_DEFINE([HAVE_AVFORMAT_FREE_CONTEXT], [1],[Define if avformat_free_context exist in libavformat])],[ ],[-lavcodec])
AC_CHECK_LIB([avformat], [av_dump_format],[AC_DEFINE([HAVE_AV_DUMP_FORMAT], [1],[Define if av_dump_format exist in libavformat])],[ ],[-lavcodec])


dnl-- check AVCODEC
AC_CHECK_TYPES([enum AVSampleFormat], [], [], [[#include <libavcodec/avcodec.h>]])
AC_CHECK_LIB([avcodec], [avcodec_open2],[AC_DEFINE([HAVE_AVCODEC_OPEN_TWO], [1],[Define if avcodec_open2 exist in libavcodec])])

dnl-- check decode/pts method
AC_CHECK_LIB([avutil], [av_opt_ptr],[AC_DEFINE([HAVE_AV_OPT_PTR], [1],[Define if av_opt_ptr exist in libavutil])])
AC_CHECK_LIB([avcodec], [avcodec_get_frame_class],[AC_DEFINE([HAVE_AVCODEC_GET_FRAME_CLASS], [1],[Define if avcodec_get_frame_class exist in libavcodec])])
AC_CHECK_MEMBERS([AVFrame.best_effort_timestamp],[AC_DEFINE([HAVE_BEST_EFFORT_TIMESTAMP], [1],[Define if AVFrame.best_effort_timestamp exist in libavcodec]) ], [ ], [[#include <libavcodec/avcodec.h>]])
AC_CHECK_LIB([avcodec], [avcodec_decode_video2],[AC_DEFINE([HAVE_AVCODEC_DECODE_VIDEO_TWO], [1],[Define if avcodec_decode_video2 exist in libavcodec])])

dnl-- check for audio resample method 
AC_CHECK_MEMBERS([AVFrame.nb_samples],[AC_DEFINE([HAVE_FRAME_NB_SAMPLES], [1],[Define if AVFrame.nb_samples exist in libavcodec]) ], [ ], [[#include <libavcodec/avcodec.h>]])
AC_CHECK_MEMBERS([AVFrame.channels],[AC_DEFINE([HAVE_FRAME_CHANNELS], [1],[Define if AVFrame.nb_samples exist in libavcodec]) ], [ ], [[#include <libavcodec/avcodec.h>]])
AC_CHECK_MEMBERS([AVFrame.format],[AC_DEFINE([HAVE_FRAME_FORMAT], [1],[Define if AVFrame.format exist in libavcodec]) ], [ ], [[#include <libavcodec/avcodec.h>]])
AC_CHECK_LIB([swresample], [swr_alloc_set_opts],[AC_DEFINE([HAVE_AUDIO_DECODE_RESAMPLE], [1],[Define if have decode_audio4/libswresample])])
AC_CHECK_LIB([avcodec], [avcodec_decode_audio3],[AC_DEFINE([HAVE_AVCODEC_DECODE_AUDIO_THREE], [1],[Define if avcodec_decode_audio3 exist in libavcodec])])

dnl-- check format size calculation
AC_CHECK_LIB([avcodec], [av_get_bytes_per_sample],[AC_DEFINE([HAVE_AV_GET_BYTES_PER_SAMPLE], [1],[Define if av_get_bytes_per_sample exist in libavcodec])])

CFLAGS=${ax_ffmpeg_save_CFLAGS}
LIBS=${ax_ffmpeg_save_LIBS}

AC_SUBST([FFMPEG_CFLAGS])   
AC_SUBST([FFMPEG_LIBS])

])

