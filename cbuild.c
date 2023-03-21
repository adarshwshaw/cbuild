#define CBUILD_IMPLEMENTATION
#include "./cbuild.h"
int main(int argc, char const *argv[])
{
    GO_REBUILD_URSELF();
    printf("%s  ac\n", argv[0]);
    return 0;
}
