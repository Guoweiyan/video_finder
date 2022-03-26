//
// Created by weiyanguo on 2021/12/24.
//
#define cimg_use_jpeg
#include <CImg-3.0.0/CImg.h>
#include <iostream>

using namespace cimg_library;
using std::cout;
using std::cin;
int main(int argc, char ** argv){

    CImg<unsigned char> image("/Users/weiyanguo/Code/video_finder/test.jpg");

    CImgDisplay main_disp(image, "Image show");
    while(!main_disp.is_closed()){
        main_disp.wait();
    }

    cout << "Pixel at (20,20,1) is: " << int(image(20, 20, 0, 1)) << std::endl;
    cout << "The image width and height are: " << image.width() << ", " << image.height() << std::endl;
    cout << "The memory of image is: " << image.size() << "Byte";

    CImg<unsigned char> * newCimage = new CImg<unsigned char>(image.width(), image.height(), 1, 3, 0);
    const unsigned char red[] = {255, 0, 0}, green[]={0,0,255}, blue[]={0,0,255};

    CImgDisplay * disp1 = new CImgDisplay();
    newCimage->fill(0).display(*disp1);
    while (!disp1->is_closed()){
        disp1->wait();
    }
//    main_disp.assign();
    return 0;
}
