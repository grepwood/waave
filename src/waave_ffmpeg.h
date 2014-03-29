#ifndef WAAVE_FFMPEG_H
#define WAAVE_FFMPEG_H

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

#include "waave_engine_flags.h"

#if !HAVE_AUDIO_DECODE_RESAMPLE && HAVE_LIBAVCODEC_AUDIOCONVERT_H
#include <libavcodec/audioconvert.h>
#endif



/***************************/
/*       AVFORMAT          */
/***************************/
#ifndef AVERROR_STREAM_NOT_FOUND
#define AVERROR_STREAM_NOT_FOUND -1
#endif


#if !HAVE_ENUM_AVMEDIATYPE

#define AVMEDIA_TYPE_UNKNOWN CODEC_TYPE_UNKNOWN
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO

#endif


#if !HAVE_AVFORMAT_OPEN_INPUT
#define avformat_open_input(formatCtx, filename, fmt, options)  av_open_input_file(formatCtx, filename, fmt, 0, NULL)
#endif

#if !HAVE_AVFORMAT_CLOSE_INPUT
#define avformat_close_input(formatCtx) av_close_input_file(*formatCtx)
#endif

#if !HAVE_AVFORMAT_FIND_STREAM_INFO
#define avformat_find_stream_info(formatCtx, option)  av_find_stream_info(formatCtx)
#endif

#if !HAVE_AV_FIND_BEST_STREAM
#define av_find_best_stream(formatCtx, type, streamNb, related, decoder, flags) WV_findBestStream(formatCtx, type)
#endif


int WV_findBestStream(AVFormatContext* formatCtx, int type);

#if !HAVE_AVFORMAT_FREE_CONTEXT
#define avformat_free_context av_free
#endif

#if !HAVE_AV_DUMP_FORMAT
#define av_dump_format dump_format
#endif


/***********/
/* AVCODEC */
/***********/
#if !HAVE_AVCODEC_OPEN_TWO
#define avcodec_open2(codecCtx, codec,  options)  avcodec_open(codecCtx, codec)
#endif



/************************/
/*    AVCODEC video     */
/************************/
#if !HAVE_ENUM_AVSAMPLEFORMAT

#define AV_SAMPLE_FMT_S16 SAMPLE_FMT_S16 

#endif


int WV_decodeVideo(AVCodecContext* codecCtx, AVFrame* frame, int* got_picture, AVPacket* pkt);
int64_t WV_getFramePts(AVFrame* frame);




/************************/
/*   AVCODEC audio      */
/************************/
/* better method ? */
#if HAVE_FRAME_NB_SAMPLES
#define WVnb_samples nb_samples
#else
#define WVnb_samples linesize[0]
#endif

//never have
#if HAVE_FRAME_CHANNELS
#define WVchannels channels
#else
#define WVchannels linesize[1]
#endif

#if HAVE_FRAME_FORMAT
#define WVformat format
#else
#define WVformat linesize[2]
#endif


#if !HAVE_AUDIO_DECODE_RESAMPLE
#define SwrContext AVAudioConvert
#endif

#if !HAVE_AV_GET_BYTES_PER_SAMPLE
#define av_get_bytes_per_sample(format) (av_get_bits_per_sample_format(format)/8)
#endif


struct SwrContext* WV_getResampleContext(AVCodecContext* codec);
void WV_freeResampleContext(struct SwrContext** convertCtx);
void WV_getDecodeFrame(AVCodecContext* codec, AVFrame* frame, int16_t* data);
void WV_freeDecodeFrame(AVFrame* frame);
int WV_decodeAudio(AVCodecContext* codec, AVFrame* frame, int* gotFrame, AVPacket* pkt);
int WV_resampleAudio(struct SwrContext *swrCtx, uint8_t* destBuffer, AVFrame* inputFrame);


#endif
