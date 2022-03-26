//
// Created by weiyanguo on 2021/12/24.
//
#include "compute_hash.h"
#include "io_affairs.h"
#include "video_affairs.h"

int
main(int argc, char ** argv){
    if (argc < 3){
        cout << "Usage:\ncompute_hash dir database_location\n";
        return -1;
    }
    filesystem::path path{argv[1]};
    if(! filesystem::is_directory(path)){
        cout << "Your input is not a directory!\n";
        return -1;
    }
    // The dataset location
    filesystem::path dataset_path{argv[2]};
    auto res = get_dir_video_list(path);
    printf("Totally %d videos to process!\n", int(res->size()));
//    printf("%d", res->size());
    // Phash general parameter for later hash computation
    const phash_general_param * pgp = get_phash_general_param();
    // The structure saves all the hash infomation
    phash_table phash_info;
    phash_info.table_location = path;
    //construct vector segments
    int process_num = 4;
    int seg_length = static_cast<int>(res->size()) / (process_num - 1);
//    printf("Segmentation length is: %d\n", seg_length);
    vector<filesystem::path> vss[process_num];
    for(int i = 0; i < process_num - 1; i++){
        vss[i] = vector<filesystem::path>(res->begin()+ i*seg_length,
                                          res->begin() + (i+1)*seg_length);
    }
    vss[process_num-1] = vector<filesystem::path>(res->begin() + (process_num-1)*seg_length,
                                                  res->end());

    mutex g_mutex;
    thread threads[process_num];
    for(int i = 0; i < process_num; i++){
        threads[i] = thread(compute_vid_in_dataset_mp, ref(g_mutex), ref(phash_info),
                            ref(vss[i]), ref(*pgp));
    }
    for(int i = 0; i < process_num; i++){
        threads[i].join();
    }


    save_phash_table(phash_info, dataset_path);
//    save_phash_table(phash_info, path/"phash.out");

//    auto pht_ptr = read_phash_table(path/"phash.out");
//    cout << pht_ptr->table_location << endl;
//    cout << phash_info.content[0].hashlist[0] << endl;
    return 0;
}


