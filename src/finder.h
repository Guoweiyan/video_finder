//
// Created by weiyanguo on 2021/12/26.
//

#ifndef VIDEO_FINDER_FINDER_H
#define VIDEO_FINDER_FINDER_H

#include "io_affairs.h"
#include "compute_hash.h"
#include "basic.h"
extern "C"{
#include <curl.h>
};


struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    auto *mem = (MemoryStruct *)userp;

    char *ptr = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
    if(!ptr) {
        /* out of memory! */
        printf("not "
               "enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int
download_img_to_buffer(char * url, MemoryStruct * ms){
    CURL * curl_handle;
    CURLcode res;
    ms->memory = (char*)malloc(1);
    ms->size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)ms);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl_handle);
    if(res != CURLE_OK){
        return -1;
    }
//    printf("The image size is %lu bytes", (unsigned long)ms->size);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 0;

}


static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int
download_img_to_file(char * url, const char * filename){
    CURL * curl_handle;
    CURLcode res;
//    filename = new char[]{"img.jpg"};
    FILE * pagefile;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
//    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)ms);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    pagefile = fopen(filename, "wb");
    if(pagefile) {
        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
        /* get it! */
        res = curl_easy_perform(curl_handle);
        /* close the header file */
        fclose(pagefile);
    }
    if(res != CURLE_OK){
        return -1;
    }
//    printf("The image size is %lu bytes", (unsigned long)ms->size);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 0;

}




#endif //VIDEO_FINDER_FINDER_H
