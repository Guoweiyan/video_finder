//
// Created by weiyanguo on 2021/12/25.
//

#ifndef VIDEO_FINDER_IO_AFFAIRS_H
#define VIDEO_FINDER_IO_AFFAIRS_H

//#include "compute_hash.h"
#include "basic.h"

void
save_cstr(char * cstr, ofstream & of){
    const int length = int(strlen(cstr));
    of.write((char*)&length, sizeof(int));
    of.write(cstr, length);
}

//read a single c string from input stream
char *
read_cstr(ifstream & inf){
    int length;
    inf.read((char*)&length, sizeof(int));
    char * cstr = new char[length + 1];
    inf.read(cstr, length);
    cstr[length] = '\0';
    return cstr;
}

void
save_hash_list(const vector<ulong64> & hl, ofstream & of){
    const unsigned long hash_list_length = hl.size();
    of.write((char*) &hash_list_length, sizeof(long));
    for(auto hash:hl){
        of.write((char*)&hash, sizeof(ulong64));
    }
}

void
read_hash_list(ifstream & inf, vector<ulong64> & hl){
    unsigned long hash_list_length;
    inf.read((char*)&hash_list_length, sizeof(long));
    ulong64 hash_tmp;
    hl.reserve(hash_list_length);
    for(unsigned long i = 0; i < hash_list_length; i++){
        inf.read((char *)&hash_tmp, sizeof(ulong64));
        hl.push_back(hash_tmp);
    }
}

void
save_phash_table_content_itm(const phash_table_item & pti, ofstream & of){
    save_cstr((char*)pti.path.c_str(), of);
    save_cstr((char*)pti.folder.c_str(), of);
    save_cstr((char*)pti.vidname.c_str(), of);
    save_hash_list(*pti.hashlist, of);
}

void
read_phash_table_content_itm(ifstream & inf, phash_table_item & pti){
    //todo : Memory leak!
    char * tmpchar;
    tmpchar = read_cstr(inf);
    pti.path = tmpchar;
    delete tmpchar;
    tmpchar = read_cstr(inf);
    pti.folder = tmpchar;
    delete tmpchar;
    tmpchar = read_cstr(inf);
    pti.vidname = tmpchar;
    delete tmpchar;
//    auto hl_ptr = new vector<ulong64>();
//    read_hash_list(inf, *hl_ptr);
//    pti.hashlist = std::move(*hl_ptr);
    read_hash_list(inf, *pti.hashlist);
}

// save the pht.content (vector) to of
void
save_phash_table_content(const phash_table & pht, ofstream & of){
    // content elements count
    const unsigned long content_length = pht.content.size();
    of.write((char*) &content_length, sizeof(long));
    for(const auto& itm: pht.content){
        save_phash_table_content_itm(itm, of);
    }
}

vector<phash_table_item> *
read_phash_table_content(ifstream & inf){
    auto ptc = new vector<phash_table_item>();
    phash_table_item pti_tmp;
    unsigned long content_length;
    inf.read((char*) & content_length, sizeof(long));
    for(unsigned long i = 0; i < content_length; i++){
        pti_tmp.hashlist = new vector<ulong64>();
        read_phash_table_content_itm(inf, pti_tmp);
        ptc->push_back(pti_tmp);
    }
    return ptc;
}

int
save_phash_table(const phash_table & pht, const filesystem::path& path){
    ofstream outfile;
    outfile.open(path, ios::out|ios::trunc|ios::binary);
    // write the table location information: location length + location string
    save_cstr((char*)pht.table_location.c_str(), outfile);
    // write the count of content
    save_phash_table_content(pht, outfile);
    outfile.close();
//    cout << "Saved!" << path;
    return 0;
}

phash_table *
read_phash_table(const filesystem::path & path){
    ifstream inf;
    inf.open(path, ios::binary);
    auto pt = new phash_table;
    pt->table_location = read_cstr(inf);
    pt->content = *read_phash_table_content(inf);
    return pt;
}

#endif //VIDEO_FINDER_IO_AFFAIRS_H
