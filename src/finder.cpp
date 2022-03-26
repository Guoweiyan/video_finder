//
// Created by weiyanguo on 2021/12/26.
//

#include "finder.h"

int main(int argc, char ** argv){
    if(argc < 3){
        printf("Usage:\nfinder pic_path dataset_path\n");
        return -1;
    }
    CImg<unsigned char> img_to_find;
    // if URL we download the content of the file
    if(string(argv[1]).starts_with("http")){
        auto * ms = new MemoryStruct;
        FILE * pic_file;
        if(download_img_to_buffer(argv[1], ms) < 0){
            printf("Error downloading your picture!\n");
            return -1;
        }
        pic_file = fmemopen(ms->memory, ms->size, "rb");
        if(!pic_file) {
            printf("Error opening buffer!\n");
            return -1;
        }
        string url_str = string(argv[1]);
        regex jpeg_reg(".+\\.((jpeg)|(jpg)).*", regex::icase);
        regex png_reg(".+\\.(png).*", regex::icase);

        if(regex_match(url_str, jpeg_reg)){
            img_to_find.load_jpeg(pic_file);
        }else if(regex_match(url_str, png_reg)){
            img_to_find.load_png(pic_file);
        }else{
            printf("Error file format!\n");
            return -1;
        }
        printf("Image downloaded!\n");
    }else{
        char * pic_path = argv[1];
        img_to_find.load(pic_path);
    }
    if(img_to_find.spectrum() == 4){
        printf("Your input has alpha channel and we will ignore it\n");
        img_to_find.channels(0,2);
    }
    printf("Image loaded!\n");


    const filesystem::path dataset_path{argv[2]};
    if(!exists(dataset_path)){
        printf("Your input dataset file is not existed!\n");
        return -1;
    }
    phash_table * pt = read_phash_table(dataset_path);

    phash_general_param * pgp = get_phash_general_param();
    ulong64 pic_hash = compute_pic_hash(img_to_find, *pgp);
    printf("The hash of the image is: %#16llx\n", pic_hash);

    vector<hamming_distance_result> results;
    for(const auto& pti: pt->content){
        hamming_distance_result hdr;
        int min_hamming_dist = 64;
        int min_index = 0;
        int index = 0;
        for(const auto & hash: *pti.hashlist){
            const int hamming_dist = compute_hamming_distance(pic_hash, hash);
            if(hamming_dist < min_hamming_dist){
                min_index = index;
                min_hamming_dist = hamming_dist;
            }
            index ++;
        }
        hdr.min_hamming_dist = min_hamming_dist;
        hdr.min_index = min_index;
        hdr.path = pti.path;
        hdr.folder = pti.folder;
        hdr.vidname = pti.vidname;
        results.push_back(hdr);
    }
    sort(results.begin(), results.end(), [](const hamming_distance_result & hdr1, const hamming_distance_result & hdr2){
        return hdr1.min_hamming_dist < hdr2.min_hamming_dist;
    });

    printf("The most possible video is :\nFolder: %s\nVideo name: %s\nHamming distance: %d\n",
           results[0].folder.c_str(), results[0].vidname.c_str(), results[0].min_hamming_dist);
//    int i = 0;
//    for(auto & result : results){
//        printf("%s\t%d\t%d\n", result.vidname.c_str(), result.min_hamming_dist, result.min_index);
//        if(i > 3)
//            break;
//        i++;
//    }

    delete pgp;
    delete pt;
    return 0;
}