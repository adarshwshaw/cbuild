/* 
MIT License

Copyright (c) 2023 Adarsh Shaw

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#ifndef AD_INCLUDE_CBUILD_H_
#define AD_INCLUDE_CBUILD_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef ADCB_API
    #define ADCB_API static
#endif

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    typedef struct {
        WIN32_FIND_DATA fileinfo;
        char basePath[MAX_PATH];
    } ADCB_FILEINFO;
#endif
typedef const char* cstr;
typedef struct {
    cstr* elem;
    int count;
} cstr_array;

// desc: Execute shell command and prints output to stdout
#define CMD(...) \
do{\
    cstr_array res = cstr_array_init(__VA_ARGS__,NULL);\
    char jres[4096]={0};\
    cstr_array_join(" ",res,jres,sizeof(jres));\
    printf("RUNNING %s\n",jres);\
    __cmd(jres);\
    cstr_array_free(res);\
}while(0)

// Moves file from one location to another
#define MOVE(SRC,DST) CMD("MOVE",SRC,DST)

// rebuild the it self if there is a change in source
#define GO_REBUILD_URSELF() \
do{\
    const char* cfile = __FILE__;\
    const char* bfile = argv[0];\
    int res = is_f1_modified_after_f2(cfile,bfile);\
    ADCB_FILEINFO fi;\
    _Bool b=adcb_getFile(bfile,&fi);\
    if (b!=0 && res){\
        printf("rebuilding cbuild");\
        char newpath[260]={0};\
        strcpy(newpath,bfile);\
        memcpy(newpath+strlen(newpath),".old\0",4);\
        MOVE(bfile,newpath);\
        CMD("clang",cfile,"-o",bfile, "-D _CRT_SECURE_NO_WARNINGS");\
        CMD(bfile);\
        exit(1);\
    }\
}while(0)
// desc: execute the command in shell and prints the output to console
ADCB_API int __cmd(const char* cmd);
// desc: create a string array from variadic args
ADCB_API cstr_array cstr_array_init(cstr first, ...);
ADCB_API void cstr_array_free(cstr_array arr);

// desc: join all strings with delimiter in between
// in:
//     delimiter: delimiter to join with should be given as string and should be single character
//     arr: array of string
//     out: result string it should be pre allocated
//     mac_len: len of pre allocated string for string safety.
ADCB_API int cstr_array_join(const char* delimiter,cstr_array arr,char* out,size_t max_len);
//
// dirent
// 

//get fileinfo from directory
ADCB_API _Bool adcb_getFile(const char* filepath, ADCB_FILEINFO* fi);

// get file list from given path. supports wildcard charactors
ADCB_API _Bool adcb_getFiles(const char* dirpath, cstr_array* arr)

// create a directory
ADCB_API _Bool adcb_createDir(const char* dirpath);

// delete file or directory(recursive)
ADCB_API _Bool adcb_deletefile(char* filepath);


#endif //AD_INCLUDE_CBUILD_H_

#ifdef CBUILD_IMPLEMENTATION

// runs shell command
ADCB_API int __cmd(const char* cmd){

    char buffer[512];
    FILE *fd;
    if ((fd = _popen(cmd,"rt")) == NULL){
        fprintf(stdout,"ERROR: shell integration failed.\n");
        return -1;
    }
    while(fgets(buffer,511,fd)){
        fprintf(stdout,"%s",buffer);
    }
    int eof = feof(fd);
    int closeReturnedVal = _pclose(fd);
    return closeReturnedVal;
}

// create a string array from variadic arguments
ADCB_API cstr_array cstr_array_init(cstr first, ...){
    cstr_array res={0};
    va_list va;
    va_start(va,first);
    cstr arg = first;
    while(arg){
        res.count++;
        arg = va_arg(va,cstr);
    }
    va_end(va);
    res.elem = calloc(res.count,sizeof(cstr));
    va_start(va,first);
    arg = first;
    for (int i=0;i<res.count;i++){
        res.elem[i]=_strdup(arg);
        arg = va_arg(va,cstr);
    }
    va_end(va);
    return res;
}
ADCB_API void cstr_array_add(cstr_array* arr, const char* str){
    arr->count++;
    arr->elem = realloc(arr->elem,arr->count*sizeof(cstr));
    arr->elem[arr->count-1]=_strdup(str);
}

// free the cstr_array
ADCB_API void cstr_array_free(cstr_array arr){
    for (int i=0;i<arr.count;i++)
        free((void*)(arr.elem[i]));
    free(arr.elem);
}

// join list of string with delimiter
ADCB_API int cstr_array_join(const char* delimiter,cstr_array arr,char* out,size_t max_len){
    int res_len = arr.count+1;
    for (int i=0;i<arr.count;i++){
        res_len += strlen(arr.elem[i]);
    }
    if (max_len <  res_len){
        printf("ERROR: output buffer not big enough.\n");
        return 1;
    }
    int out_idx=0;
    for (int i=0;i<arr.count;i++){
        memcpy(out+out_idx,arr.elem[i],strlen(arr.elem[i]));
        out_idx+=strlen(arr.elem[i]);
        if (i< arr.count-1){
            if (strlen(delimiter)!=0)
                out[out_idx++]=delimiter[0];
        }
        else
            out[out_idx]='\0';
    }
    return 0;
}

// 
// dirent
// 

// compares file last write time
ADCB_API int is_f1_modified_after_f2(const char* f1, const char* f2){
    ADCB_FILEINFO fi1,fi2;
    if(adcb_getFile(f1,&fi1)==0){
        printf("ERROR: could not locate file %s\n",f1);
        exit(1);
    }
    if(adcb_getFile(f2,&fi2)==0){
        printf("ERROR: could not locate file %s\n",f2);
        exit(1);
    }
    int res = CompareFileTime(&(fi1.fileinfo.ftLastWriteTime),&(fi2.fileinfo.ftLastWriteTime))==1?1:0;
    return res;
}

ADCB_API _Bool adcb_getFiles(const char* dirpath, cstr_array* arr){
    ADCB_FILEINFO fi;
    HANDLE pid = FindFirstFile(dirpath,&(fi.fileinfo));
    if(pid==INVALID_HANDLE_VALUE){
        printf("ERROR: could not locate directory or file\n");
        return 0;
    }

    do{
        if (!strcmp(".",fi.fileinfo.cFileName) || !strcmp("..",fi.fileinfo.cFileName))
            continue;
        GetFullPathName(fi.fileinfo.cFileName,MAX_PATH,fi.basePath,NULL);
        cstr_array_add(arr,fi.basePath);
    }while(FindNextFile(pid,&(fi.fileinfo))!=0);
    return 1;
}
// get fileinfo
ADCB_API _Bool adcb_getFile(const char* filepath, ADCB_FILEINFO* fi){
    if(FindFirstFile(filepath,&(fi->fileinfo))==INVALID_HANDLE_VALUE){
        printf("ERROR: could not find file\n");
        return 0;
    }

    GetFullPathName(fi->fileinfo.cFileName,MAX_PATH,fi->basePath,NULL);
    return 1;
}
// create empty directory
ADCB_API _Bool adcb_createDir(const char* dirpath){
    // doesnot work recursively;
    return CreateDirectory(dirpath,0);
}

// delete file or directory
ADCB_API _Bool adcb_deletefile(char* filepath){
    // ADCB_fixPath((char*)filepath);
    ADCB_FILEINFO fi;
    if (adcb_getFile(filepath,&fi)==0){
        printf("ERROR: could not locate the file\n");
        return 0;
    }
    
    if (fi.fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY){
        CMD("rmdir","/S","/Q",fi.basePath);
        return 1;
    }    
    else
        return DeleteFile(filepath);
}
#endif //CBUILD_IMPLEMENTATION