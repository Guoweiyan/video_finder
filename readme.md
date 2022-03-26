# Video Finder: 本地视频数据库建立并依据截图查找
这是我在休学期间突发奇想写的一个小工具，之前写过 python 版本，但是速度太慢，且依赖较多，
因此用 c++（准确来说是 c with class hhh）重写了一遍。cmake 文件以及代码都挺乱的，以后有机会慢慢修正（咕咕。
## 功能
1. 对本地指定目录内的视频文件进行处理，生成一个非常小的数据库
2. 可依据截图搜索到数据库中对应视频的信息以及截图在视频中的时间点

适合电影、短视频等资源囤积户用于搜索某一截图出处。
## 使用流程
1. 首先使用数据库生成工具：compute_hash 对本地某一目录下的所有视频文件处理生成数据库
    ```
   Usage:
   compute_hash dir database_location
   ```
2. 使用搜寻工具： finder 在选定数据库中搜寻你想要寻找的截图出处
    ```
   Usage:
   finder pic_path dataset_path
    ```

pic_path 可以是一个 URL 或者本地图片文件路径。

3. 若有多个数据库文件，可以将他们放置在同一文件夹内，之后通过 merge_dataset 合并此文件夹

⚠️：需要修改cmake 文件中的一些路径才可以正确编译，包括：curl,ffmpeg,libjpeg,libpng等库的头文件以及链接库文件路径

## 思路介绍
核心思路为对每个视频中的关键帧以及隔 30 帧的图像计算phash，之后将视频名称、路径、整个视频的 phash列表等内容记录到数据库中；在搜寻截图出处时，对比截图的 phash
与数据库中记录的各个 phash 列表，找到汉明距离最短的 phash，进而索引到对应的视频与时间点。

使用 ffmpeg 处理视频数据；使用 CImg 对帧图像数据进行处理，计算；使用 curl 从URL载入图像内容；

## 特别感谢
_防火长城长_ 提供的很多优化思路

_避难所_ 
