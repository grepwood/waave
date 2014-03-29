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

#include "waave_ffmpeg.h"

#include "common.h"
#include "config_ffmpeg.h"

#include "waave_engine_flags.h"

#if !HAVE_AUDIO_DECODE_RESAMPLE && HAVE_LIBAVCODEC_AUDIOCONVERT_H
#include <libavcodec/audioconvert.h>
#endif


/***************************/
/*       AVFORMAT          */
/***************************/
int WV_findBestStream(AVFormatContext* formatCtx, int type)
{
  int i;
  for(i=0; i<formatCtx->nb_streams; i++){
    if(formatCtx->streams[i]->codec->codec_type == type)
      return i;
  }

  /* cannot find */
  return -1;
}




/************************/
/*    AVCODEC video     */
/************************/

int WV_decodeVideo(AVCodecContext* codecCtx, AVFrame* frame, int* got_picture, AVPacket* pkt)
{
  /* save pts and dts if needed */
#if !(HAVE_AV_OPT_PTR && HAVE_AVCODEC_GET_FRAME_CLASS) && !HAVE_BEST_EFFORT_TIMESTAMP
  codecCtx->reordered_opaque = pkt->pts;
#endif
  
  /* decode */
  int decodedBytes;
#if HAVE_AVCODEC_DECODE_VIDEO_TWO
  decodedBytes = avcodec_decode_video2(codecCtx, frame, got_picture, pkt);
#else
  decodedBytes = avcodec_decode_video(codecCtx, frame, got_picture, pkt->data, pkt->size);
#endif
  
  /* save pts */
#if !(HAVE_AV_OPT_PTR && HAVE_AVCODEC_GET_FRAME_CLASS) && !HAVE_BEST_EFFORT_TIMESTAMP
  frame->pts = pkt->dts;
#endif

  /* return */
  return 0;
}


int64_t WV_getFramePts(AVFrame* frame)
{
#if HAVE_AV_OPT_PTR && HAVE_AVCODEC_GET_FRAME_CLASS
  return *(int64_t*)av_opt_ptr(avcodec_get_frame_class(), frame, "best_effort_timestamp");
#elif HAVE_BEST_EFFORT_TIMESTAMP
  return frame->best_effort_timestamp;
#else
  if(frame->reordered_opaque != AV_NOPTS_VALUE)
    return frame->reordered_opaque;
  else if(frame->pts != AV_NOPTS_VALUE)
    return frame->pts;
  else
    return 0;
    
#endif
} 


/************************/
/*   AVCODEC audio      */
/************************/

struct SwrContext* WV_getResampleContext(AVCodecContext* codec)
{
#if HAVE_AUDIO_DECODE_RESAMPLE
  struct SwrContext* newCtx;
  int64_t srcChannelLayout = (codec->channel_layout && codec->channels == av_get_channel_layout_nb_channels(codec->channel_layout)) ? codec->channel_layout : av_get_default_channel_layout(codec->channels);
  newCtx = swr_alloc_set_opts(NULL,	\
			      AV_CH_LAYOUT_STEREO,			\
			      WV_DECODER_SAMPLE_FORMAT,			\
			      WV_DECODER_SAMPLE_RATE,			\
			      srcChannelLayout,				\
			      codec->sample_fmt,			\
			      codec->sample_rate,			\
			      0, NULL);

  swr_init(newCtx);
  return newCtx;
#else
  av_audio_convert_alloc(WV_DECODER_SAMPLE_FORMAT, 1, codec->sample_fmt, 1, NULL, 0);
#endif
}



void WV_freeResampleContext(struct SwrContext** convertCtx)
{
#if HAVE_AUDIO_DECODE_RESAMPLE
  swr_free(convertCtx);
#else
  av_audio_convert_free(*convertCtx);
  *convertCtx = NULL;
#endif
}


/* !!! frame need to be allocated !!! */
/* try direct rendring */
void WV_getDecodeFrame(AVCodecContext* codec, AVFrame* frame, int16_t* data)
{
#if HAVE_AUDIO_DECODE_RESAMPLE
  avcodec_get_frame_defaults(frame);
#else
  frame->data[0] = (uint8_t*)data;
  if(codec->sample_fmt != WV_DECODER_SAMPLE_FORMAT ||	\
     codec->sample_rate != WV_DECODER_SAMPLE_RATE || \
     codec->channels != WV_DECODER_CHANNELS){
    if(!frame->data[1])
      frame->data[1] = (uint8_t*)av_malloc(WV_DECODE_TARGET_SIZE);
  }
  else{
    if(frame->data[1]){
      av_free(frame->data[1]);
      frame->data[1] = NULL;
    }
  }
#endif
}



void WV_freeDecodeFrame(AVFrame* frame)
{
#if HAVE_AUDIO_DECODE_RESAMPLE
  av_free(frame);
#else
  if(frame->data[1]){
    av_free(frame->data[1]);
    frame->data[1] = NULL;
  }

  av_free(frame);
#endif
}



int WV_decodeAudio(AVCodecContext* codec, AVFrame* frame, int* gotFrame, AVPacket* pkt)
{
#if HAVE_AUDIO_DECODE_RESAMPLE
  return avcodec_decode_audio4(codec, frame, gotFrame,	pkt);
#else
  /* check direct rendering */
  int16_t* destBuffer;
  if(frame->data[1])
    destBuffer = (int16_t*)frame->data[1];
  else
    destBuffer = (int16_t*)frame->data[0];

  /* decode */
  int decodedBytes;
  frame->WVnb_samples = WV_DECODE_TARGET_SIZE;  //will be converted in samples

#if HAVE_AVCODEC_DECODE_AUDIO_THREE
  decodedBytes = avcodec_decode_audio3(codec, destBuffer, &frame->WVnb_samples, pkt);
#else
  decodedBytes = avcodec_decode_audio2(codec, destBuffer, &frame->WVnb_samples, pkt->data, pkt->size);
#endif

  if(frame->WVnb_samples){
    *gotFrame = 1;
    frame->WVnb_samples /= codec->channels*av_get_bytes_per_sample(codec->sample_fmt); 
    frame->WVformat = codec->sample_fmt;
    frame->WVchannels = codec->channels;
  }
  else{
    *gotFrame = 0;
  }
  return decodedBytes;
#endif
}



int WV_resampleAudio(struct SwrContext *swrCtx, uint8_t* destBuffer, AVFrame* frame)
{
#if HAVE_AUDIO_DECODE_RESAMPLE
  const uint8_t** in = (const uint8_t **)frame->extended_data;
  uint8_t* out[] = {destBuffer};
  return swr_convert(swrCtx, out, WV_DECODE_TARGET_SAMPLES, in, frame->WVnb_samples);
#else
  /* only if no direct rendering */ 
  if(frame->data[1]){
    const uint8_t* in[] = {frame->data[1]};
    uint8_t* out[] = {destBuffer};

    int istride[6]= {av_get_bytes_per_sample(frame->WVformat)};
    int ostride[6]= {2};  //s16 audio
    av_audio_convert(swrCtx, out, ostride, in, istride, frame->WVnb_samples*frame->WVchannels); 
  }
  return frame->WVnb_samples;

#endif    
}









