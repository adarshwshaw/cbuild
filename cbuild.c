#define CBUILD_IMPLEMENTATION
#include "./cbuild.h"
int main(int argc, char const *argv[])
{
    GO_REBUILD_URSELF();
    cstr_array arr=cstr_array_init(NULL);
    adcb_getFiles("./*.c",&arr);
    for (int i=0;i<arr.count;i++){
        printf("%s\n",arr.elem[i]);
    }
    printf("%d\n",arr.count);
    cstr_array_free(arr);
    return 0;
}
