//
// Created by weiyanguo on 2021/12/25.
//

#ifndef VIDEO_FINDER_BASIC_H
#define VIDEO_FINDER_BASIC_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavcodec/packet.h>
}

//#include <jpeglib.h>
//#include <jerror.h>

#define cimg_display 0 //to avoid unnecessary x11 dependency
#define cimg_use_jpeg //to add jpeg support for cimg
#define cimg_use_png //to add png support for cimg

//#define cimg_plugin "plugins/jpeg_buffer.h"

#include <CImg-3.0.0/CImg.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <mutex>
#include <regex>



typedef unsigned long long int ulong64;
using namespace std;
using namespace cimg_library;

struct phash_general_param{
    CImg<float> * mean_filter;
    CImg<float> * dct_matrix;
    CImg<float> * dct_matrix_tr;
    int resize_size;
};

struct phash_table_item{
    vector<ulong64> * hashlist;
    string path;
    string folder;
    string vidname;
};

struct phash_table{
    vector<phash_table_item> content;
    string table_location;
};

struct vid_info{
    AVFormatContext * fmt_ctx_ptr;
    AVCodecID cdc_id;
    AVCodecContext * cdc_ctx_ptr;
    const AVCodec * cdc_ptr;
    AVStream * stream_ptr;
    int video_stream_index;
    long current_index = 0;
    long next_index = 0;
    AVPixelFormat pixel_format;
    int width, height;
    int fps;
    filesystem::path path;
    int _errno;
    long frame_number;
};



CImg<float> *
get_dct_matrix(const int N){
    auto *ptr_matrix = new CImg<float>(N,N,1,1,1/sqrt((float)N));
    const float c1 = sqrtf(2.0/N);
    for (int x=0;x<N;x++){
        for (int y=1;y<N;y++){
            *ptr_matrix->data(x,y) = c1*cos((cimg::PI/2/N)*y*(2*x+1));
        }
    }
    return ptr_matrix;
}

inline ulong64
compute_pic_hash(CImg<unsigned char> &image,
                 const phash_general_param & pgp){
    const CImg<float> * mean_filter = pgp.mean_filter;
    const CImg<float> * dct_matrix_tr = pgp.dct_matrix_tr;
    const CImg<float> * dct_matrix = pgp.dct_matrix;
    const int resize_size = pgp.resize_size;
//    image = image.RGBtoYCbCr().channel(0).get_convolve(*mean_filter);
// todo: trim the border of videos
    image = image.RGBtoYCbCr().channel(0);
    image.resize(resize_size,resize_size);
    CImg<float> dctImage = (*dct_matrix) * image * (*dct_matrix_tr);
    CImg<float> subsec = dctImage.crop(1,1,8,8).unroll('x');
    float median = subsec.median();
    ulong64 one = 0x0000000000000001;
    ulong64 hash = 0x0000000000000000;
    for (int i=0; i<64; i++){
        float current = subsec(i);
        if (current > median)
            hash |= one;
        one = one << 1;
    }
    return hash;
}

// This function will generate the general parameters for phash computation
phash_general_param *
get_phash_general_param(){
    const int dct_size = 32;
    auto * pgp = new phash_general_param();
    pgp->mean_filter = new CImg<float>(3,3,1,1,1);
    pgp->dct_matrix = get_dct_matrix(dct_size);
    pgp->dct_matrix_tr = new CImg<float>(pgp->dct_matrix->get_transpose());
    pgp->resize_size = dct_size;
//    pgp->mean_filter->save("meanfileter.jpg");
//    pgp->dct_matrix->save("dct.jpg");
//    pgp->dct_matrix_tr->save("dcttr.jpg");


    return pgp;
}

struct hamming_distance_result{
    int min_hamming_dist;
    int min_index;
    string path;
    string folder;
    string vidname;
};


int compute_hamming_distance(ulong64 hash1,ulong64 hash2){
//    ulong64 x = hash1^hash2;
//    const ulong64 m1  = 0x5555555555555555ULL;
//    const ulong64 m2  = 0x3333333333333333ULL;
//    const ulong64 h01 = 0x0101010101010101ULL;
//    const ulong64 m4  = 0x0f0f0f0f0f0f0f0fULL;
//    x -= (x >> 1) & m1;
//    x = (x & m2) + ((x >> 2) & m2);
//    x = (x + (x >> 4)) & m4;
//    return (x * h01)>>56;
    int answer = 0;
    ulong64 hashtmp1 = hash1 & 0xffff000000000000;
    ulong64 hashtmp2 = hash2 & 0xffff000000000000;
    while(hash1!= hash2){
        answer += (hash1 & 1) ^ (hash2 & 1);
        hash1 >>=1;
        hash2 >>=1;
    }

    while(hashtmp1!= hashtmp2){
        answer += (hashtmp1 & 1) ^ (hashtmp2 & 1);
        hashtmp1 >>=1;
        hashtmp2 >>=1;
    }

    return answer;
}



#endif //VIDEO_FINDER_BASIC_H
