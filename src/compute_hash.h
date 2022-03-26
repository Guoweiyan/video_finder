//
// Created by weiyanguo on 2021/12/24.
//

#ifndef VIDEO_FINDER_COMPUTE_HASH_H
#define VIDEO_FINDER_COMPUTE_HASH_H

#include "basic.h"
#include "video_affairs.h"



vector<ulong64> *
compute_video(const filesystem::path & pt, const phash_general_param & pgp){
    auto res = new vector<ulong64>();
//    printf("Opening file\n");
    vid_info * vi_ptr = open_video_file(pt);
    if(vi_ptr->_errno < 0){
        printf("Error opening file: %s", pt.c_str());
        close_video_file(vi_ptr);
        delete vi_ptr;
        return res;
    }
//    auto frame_img_list_ptr = new vector<CImg<unsigned char>>();
    const int frame_interval = 30;
    printf("Reading video: %s\n", pt.c_str());
//    if(get_video_frames_at_interval(vi_ptr, frame_img_list_ptr, frame_interval) < 0)
    if(get_video_hashlist_at_interval(vi_ptr, res, frame_interval, pgp) < 0)
        printf("Error getting frames\n");
    close_video_file(vi_ptr);
//    printf("the number of hashes is: %lu\n",res->size());
//    int i = 0;
//    for(auto & img: *frame_img_list_ptr){
//        ulong64 res_t = compute_pic_hash(img,pgp);
//        res->push_back(res_t);
//    }
    delete vi_ptr;
//    delete frame_img_list_ptr;
    return res;
}


vector<filesystem::path> *
get_dir_video_list(const filesystem::path & pt){
    auto res = new vector<filesystem::path>();
    regex video_regex(".+\\.((mp4)|(avi)|(wmv)|(rmvb))$", regex::icase|regex::nosubs|regex::optimize);
    for(auto const & dir_entry : filesystem::recursive_directory_iterator(pt, filesystem::directory_options::skip_permission_denied)){
        if(dir_entry.is_directory())
            continue;
        string path(dir_entry.path());

        if(regex_match(path, video_regex)){
            res->push_back(path);
        }
    }
    return res;
}

void
compute_vid_in_dataset_mp(std::mutex & g_mutex, phash_table & pt, vector<filesystem::path> & vid_list,
                          const phash_general_param & pgp){
    phash_table_item current_item;
    for (auto & itm: vid_list){
        current_item.hashlist = compute_video(itm, pgp);
        current_item.path = itm.string();
        current_item.vidname = itm.filename();
        current_item.folder = itm.remove_filename().string();
        const lock_guard<mutex> lock(g_mutex);
        pt.content.push_back(current_item);
        printf("%d videos file processed!\n", int(pt.content.size()));
    }
}

#endif //VIDEO_FINDER_COMPUTE_HASH_H
