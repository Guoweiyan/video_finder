//
// Created by weiyanguo on 2021/12/30.
//
#include "io_affairs.h"
#include "basic.h"


int main(int argc, char ** argv){
    if(argc < 2){
        printf("Usage:\nmerge_datasets dir\n");
        return -1;
    }
    phash_table total_pt;
    total_pt.table_location = "Merged: ";
    filesystem::path path{argv[1]};
    int count = 0;
    for(const auto & file: filesystem::directory_iterator{path}){
        if(!string(file.path()).ends_with(".out"))
            continue;
        phash_table * pt = read_phash_table(file);
        total_pt.content.insert(total_pt.content.end(), pt->content.begin(),
                                pt->content.end());
        total_pt.table_location += (pt->table_location + "   ");
        count ++;
    }
    printf("Totally %d datasets merged\n", count);
    save_phash_table(total_pt, path/"merged_dataset.out");

}