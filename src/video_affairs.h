//
// Created by weiyanguo on 2021/12/25.
//

#ifndef VIDEO_FINDER_VIDEO_AFFAIRS_H
#define VIDEO_FINDER_VIDEO_AFFAIRS_H

#include "basic.h"

inline int
get_stream_fps(AVStream * stream_ptr){
    int num = stream_ptr->r_frame_rate.num;
    int den = stream_ptr->r_frame_rate.den;
    int fps = den==0 ? num/den : 0;
    return fps;
}

long
get_stream_frames_number(AVStream * stream_ptr){
    long frames_number = stream_ptr->nb_frames;
    if(frames_number > 0){
        return frames_number;
    }else{
        frames_number = (long) av_index_search_timestamp(stream_ptr, stream_ptr->duration, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
//        if(frames_number < 0) {
//            int time_base = stream_ptr->time_base.den / stream_ptr->time_base.num;
//            frames_number = stream_ptr->duration / time_base;

//        }
        if(frames_number < 0){
            frames_number = 0x0fffffffffffffff;
        }
    }
    return frames_number;
}

int
find_video_stream(AVFormatContext * fmt_ctx){
    for(int i = 0; i < fmt_ctx->nb_streams; i ++){
        if(fmt_ctx->streams[i]->codecpar->width != 0
        && fmt_ctx->streams[i]->codecpar->height != 0)
            return i;
    }
    return -1;
}

int
close_video_file(vid_info * vi_ptr){
    avcodec_close(vi_ptr->cdc_ctx_ptr);
    avformat_close_input(&(vi_ptr->fmt_ctx_ptr));
    vi_ptr->fmt_ctx_ptr = nullptr;
    vi_ptr->cdc_ctx_ptr = nullptr;
    vi_ptr->height = -1;
    vi_ptr->width = -1;
    return 0;
}


vid_info *
open_video_file(const filesystem::path & path){
    auto * vi_ptr = new vid_info;
    vi_ptr->_errno = 0;
//    AVFormatContext * fmt_ctx_ptr = nullptr;
    vi_ptr->fmt_ctx_ptr = avformat_alloc_context();
    if(!vi_ptr->fmt_ctx_ptr){
        printf("Error allocating format context");
        vi_ptr->_errno = -1;
        return vi_ptr;
    }
    int errcode = 0;
    if((errcode = avformat_open_input(&(vi_ptr->fmt_ctx_ptr), path.c_str(), nullptr, nullptr)) < 0){
        printf("Error opening video file: \n");
        vi_ptr->_errno = -1;
        return vi_ptr;
    }
//    vi_ptr->fmt_ctx_ptr = fmt_ctx_ptr;
    if(avformat_find_stream_info(vi_ptr->fmt_ctx_ptr, nullptr)){
        printf("Error get stream info!\n");
    }
    vi_ptr->video_stream_index = av_find_best_stream(vi_ptr->fmt_ctx_ptr, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0) < 0;
    if(vi_ptr->video_stream_index < 0){
        vi_ptr->video_stream_index = find_video_stream(vi_ptr->fmt_ctx_ptr);
        if(vi_ptr->video_stream_index < 0){
            printf("No valid stream exists!\n");
            vi_ptr->_errno = -3;
            return vi_ptr;
        }
    }
//    printf("vid stream is : %d\nopening path: %s\n", vi_ptr->video_stream_index, path.c_str());
    vi_ptr->stream_ptr = vi_ptr->fmt_ctx_ptr->streams[vi_ptr->video_stream_index];
    vi_ptr->height = vi_ptr->stream_ptr->codecpar->height;
    vi_ptr->width = vi_ptr->stream_ptr->codecpar->width;
    // If the stream we find is not right ,then manually select it
    if(vi_ptr->height == 0 && vi_ptr->width == 0){
        vi_ptr->video_stream_index = find_video_stream(vi_ptr->fmt_ctx_ptr);
        if(vi_ptr->video_stream_index < 0){
            printf("No valid stream exists!\n");
            vi_ptr->_errno = -3;
            return vi_ptr;
        }
        vi_ptr->stream_ptr = vi_ptr->fmt_ctx_ptr->streams[vi_ptr->video_stream_index];
        vi_ptr->height = vi_ptr->stream_ptr->codecpar->height;
        vi_ptr->width = vi_ptr->stream_ptr->codecpar->width;
    }
//    printf("video stream index: %d\n", vi_ptr->video_stream_index);
//    printf("width: %d, height: %d\n", vi_ptr->width, vi_ptr->height);

    vi_ptr->fps = get_stream_fps(vi_ptr->stream_ptr);
//    printf("Getting fps!\n");
    vi_ptr->cdc_id = vi_ptr->stream_ptr->codecpar->codec_id;
    vi_ptr->cdc_ptr = avcodec_find_decoder(vi_ptr->cdc_id);
    if (!vi_ptr->cdc_ptr){
        printf("Error finding a decoder!\n");
        vi_ptr->_errno = -1;
        return vi_ptr;
    }

    vi_ptr->cdc_ctx_ptr = avcodec_alloc_context3(vi_ptr->cdc_ptr);
    if (!vi_ptr->cdc_ctx_ptr){
        printf("Error allocating the context for codec!\n");
        vi_ptr->_errno = -1;
        return vi_ptr;
    }

    avcodec_parameters_to_context(vi_ptr->cdc_ctx_ptr, vi_ptr->stream_ptr->codecpar);
    if(avcodec_open2(vi_ptr->cdc_ctx_ptr, vi_ptr->cdc_ptr, 0) < 0){
        printf("Cannot open codec\n");
        vi_ptr->_errno = -2;
        close_video_file(vi_ptr);
        return vi_ptr;
    }

    vi_ptr->pixel_format = AV_PIX_FMT_NONE;
//    vi_ptr->pixel_format = vi_ptr->cdc_ctx_ptr->pix_fmt;
//    printf("The video pixel format is: %d", vi_ptr->pixel_format);
    vi_ptr->frame_number = get_stream_frames_number(vi_ptr->stream_ptr);

    return vi_ptr;
}


int
get_video_frames_at_interval(vid_info * vi_ptr, vector<CImg<unsigned char>> * frame_img_list, int frame_interval){
    av_log_set_level(AV_LOG_FATAL);
    int target_frame_size = 128;
    AVPixelFormat frame_pixel_format = AV_PIX_FMT_RGB24;
    printf("getting video frames\n");
    AVFrame * frame_ptr;
//    AVFrame * converted_frame_ptr;
    frame_ptr = av_frame_alloc();
    if(frame_ptr == nullptr)
        return -1;

    unsigned char *dst_data[4];
    int dst_linesize[4];
    const int channels = 3;
    int result = 0;

    vi_ptr->current_index = 0;
    vi_ptr->next_index = 0;
    printf("Video frame total: %lu\n", vi_ptr->frame_number);
    AVPacket * packet_ptr = av_packet_alloc();
    if(packet_ptr == nullptr){
        printf("Error to allocate packet memory!");
        av_frame_free(&frame_ptr);
        return -1;
    }
    int i = 0;
    av_image_alloc(dst_data, dst_linesize,
                   target_frame_size, target_frame_size,
                   frame_pixel_format,
                   1);
    SwsContext * sws_ctx_ptr = nullptr;
    while(result >= 0){
        result = av_read_frame(vi_ptr->fmt_ctx_ptr, packet_ptr);
        if(result >= 0 && packet_ptr->stream_index != vi_ptr->video_stream_index){
            av_packet_unref(packet_ptr);
            continue;
        }
        if(result < 0)
            result = avcodec_send_packet(vi_ptr->cdc_ctx_ptr, nullptr);
        else{
            if(packet_ptr->pts == AV_NOPTS_VALUE)
                packet_ptr->pts = packet_ptr->dts = i;
            result = avcodec_send_packet(vi_ptr->cdc_ctx_ptr, packet_ptr);
        }
        av_packet_unref(packet_ptr);
        if(result < 0){
            printf("Error submitting a packet for decoding\n");
            goto finish;
        }
        while(result >= 0){
            result = avcodec_receive_frame(vi_ptr->cdc_ctx_ptr, frame_ptr);
            if(result == AVERROR_EOF){
                result = 0;
                goto finish;
            }
            else if (result == AVERROR(EAGAIN)){
                result = 0;
                break;
            } else if (result < 0) {
                printf("Error decoding frame\n");
                result = 0;
                break;
            }

            if(frame_ptr->key_frame || (i % frame_interval == 0)){
                CImg<unsigned char> next_image;
                vi_ptr->pixel_format = vi_ptr->cdc_ctx_ptr->pix_fmt;
                if(vi_ptr->pixel_format == AV_PIX_FMT_NONE){
                    printf("Pixel Format Reading Error! \n");
                    goto finish;
                }
                if(sws_ctx_ptr == nullptr)
                    sws_ctx_ptr = sws_getContext(vi_ptr->width, vi_ptr->height, vi_ptr->pixel_format,
                                                 target_frame_size, target_frame_size, frame_pixel_format, SWS_BILINEAR,
                                                 nullptr, nullptr, nullptr);
                sws_scale(sws_ctx_ptr, frame_ptr->data, frame_ptr->linesize,
                        0, vi_ptr->cdc_ctx_ptr->height,
                        dst_data, dst_linesize);

                next_image.assign((dst_data[0] + dst_linesize[0]), channels,
                                  target_frame_size, target_frame_size,
                                  1, false);
                next_image.permute_axes("yzcx");

                frame_img_list->push_back(next_image);
                av_frame_unref(frame_ptr);
            }
            i++;
            vi_ptr->current_index ++;
        }
    }
// clean up
finish:
    sws_freeContext(sws_ctx_ptr);
    av_packet_free(&packet_ptr);
    av_frame_free(&frame_ptr);
    av_freep(&dst_data[0]);
    return result;
}

int
get_video_hashlist_at_interval(vid_info * vi_ptr, vector<ulong64> * hash_list, int frame_interval, const phash_general_param & pgp){
    av_log_set_level(AV_LOG_FATAL);
    int frame_size = 128;
    AVPixelFormat frame_pixel_format = AV_PIX_FMT_RGB24;
    AVFrame * frame_ptr;
    frame_ptr = av_frame_alloc();
    if(frame_ptr == nullptr)
        return -1;
    unsigned char *dst_data[4];
    int dst_linesize[4];
    const int channels = 3;
    int result = 0;
    vi_ptr->current_index = 0;
    vi_ptr->next_index = 0;
    AVPacket * packet_ptr = av_packet_alloc();
    if(packet_ptr == nullptr){
        printf("Error to allocate packet memory!");
        av_frame_free(&frame_ptr);
        return -1;
    }
    int i = 0;
    av_image_alloc(dst_data, dst_linesize,
                   frame_size, frame_size,
                   frame_pixel_format,
                   1);
    SwsContext * sws_ctx_ptr = nullptr;
    while(result >= 0){
        result = av_read_frame(vi_ptr->fmt_ctx_ptr, packet_ptr);
//        if(result < 0){
//            printf("Error reading frame packet\n");
//            break;
//        }
        if(result >= 0 && packet_ptr->stream_index != vi_ptr->video_stream_index){
            av_packet_unref(packet_ptr);
            continue;
        }
        if(result < 0)
            result = avcodec_send_packet(vi_ptr->cdc_ctx_ptr, nullptr);
        else{
            if(packet_ptr->pts == AV_NOPTS_VALUE)
                packet_ptr->pts = packet_ptr->dts = i;
            result = avcodec_send_packet(vi_ptr->cdc_ctx_ptr, packet_ptr);
        }
        av_packet_unref(packet_ptr);

        if(result < 0){
            printf("Error submitting a packet for decoding\n");
//            result = -1;
//            goto finish;
            result = 0;
            continue;

//            return result;
        }

        while(result >= 0){
            result = avcodec_receive_frame(vi_ptr->cdc_ctx_ptr, frame_ptr);
            if(result == AVERROR_EOF){
                result = 0;
                goto finish;
            }
            else if (result == AVERROR(EAGAIN)){
                result = 0;
                break;
            } else if (result < 0) {
                printf("Error decoding frame\n");
//                return result;
//                result = -1;
                goto finish;
            }

            if((frame_ptr->key_frame || (i % frame_interval == 0))){
                CImg<unsigned char> next_image;
                if(vi_ptr->pixel_format == AV_PIX_FMT_NONE) {
//                    printf("Getting pixel format!\n");
                    if(frame_ptr->format == -1){
//                        result = -1;
//                        goto finish;
                        printf("Some frames has wrong pixel format\n");
                        break;
                    }
                    vi_ptr->pixel_format = AVPixelFormat(frame_ptr->format);
                }
                if(vi_ptr->pixel_format == AV_PIX_FMT_NONE){
                    printf("Some frame has wrong pixel format\n");
                    break;
                }
//                printf("Frame width:%d, height:%d\n", frame_ptr->width, frame_ptr->height);
                if(sws_ctx_ptr == nullptr)
                    sws_ctx_ptr = sws_getContext(vi_ptr->width,vi_ptr->height, vi_ptr->pixel_format,
                                                 frame_size,frame_size, frame_pixel_format, SWS_BILINEAR,
                                                 nullptr, nullptr, nullptr);
                sws_scale(sws_ctx_ptr, frame_ptr->data, frame_ptr->linesize,
                          0, vi_ptr->height,
                          dst_data, dst_linesize);

                next_image.assign((dst_data[0] + dst_linesize[0]), channels,
                                  frame_size, frame_size,
                                  1, false);
                next_image.permute_axes("yzcx");
                ulong64 res_t = compute_pic_hash(next_image, pgp);
                hash_list->push_back(res_t);
                av_frame_unref(frame_ptr);
            }
            i++;
            vi_ptr->current_index ++;
        }
    }
    result = 0;

    finish:
    if(sws_ctx_ptr != nullptr)
        sws_freeContext(sws_ctx_ptr);
    av_packet_free(&packet_ptr);
    av_frame_free(&frame_ptr);
    av_freep(&dst_data[0]);
    return result;
}



#endif //VIDEO_FINDER_VIDEO_AFFAIRS_H
