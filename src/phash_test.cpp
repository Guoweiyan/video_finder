//
// Created by weiyanguo on 2021/12/24.
//
#define cimg_use_jpeg
#define cimg_display 0
//#include <pHash.h>
#include <CImg-3.0.0/CImg.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

using namespace std;
using namespace cimg_library;
typedef unsigned long long int ulong64;

CImg<float>* ph_dct_matrix(const int N){
    auto *ptr_matrix = new CImg<float>(N,N,1,1,1/sqrt((float)N));
    const float c1 = sqrt(2.0/N);
    for (int x=0;x<N;x++){
        for (int y=1;y<N;y++){
            *ptr_matrix->data(x,y) = c1*cos((cimg::PI/2/N)*y*(2*x+1));
        }
    }
    return ptr_matrix;
}

int main(){
    auto * image = new CImg<unsigned char>("");
    CImg<unsigned char> img;
    CImg<float> meanfilter(7, 7, 1, 1, 1);
    img = image->RGBtoYUV().channel(0).get_convolve(meanfilter);
    img.resize(32,32);
    CImg<float> *C = ph_dct_matrix(32);
    CImg<float> Ctransp = C->get_transpose();

    CImg<float> dctImage = (*C) * img * Ctransp;
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
    std::cout << hash;
    delete image;
    return 0;
}
