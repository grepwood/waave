#ifndef WAAVE_CONFIG_FFMPEG_H
#define WAAVE_CONFIG_FFMPEG_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_LIBAVCODEC_AVCODEC_H
#include <libavcodec/avcodec.h>
#endif

#if HAVE_LIBAVFORMAT_AVFORMAT_H
#include <libavformat/avformat.h>
#endif

#if HAVE_LIBSWSCALE_SWSCALE_H
#include <libswscale/swscale.h>
#endif

#if HAVE_LIBSWRESAMPLE_SWRESAMPLE_H
#include <libswresample/swresample.h>
#endif

#endif
