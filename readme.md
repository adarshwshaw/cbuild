# CBUILD
This is a header only libary. Basic build tool for c programs.
This assumes you are using clang.

Usages:
- Get the header file to use
- create a c file
```c
#define CBUILD_IMPLEMENTATION
#include "./cbuild.h"
int main(int argc, char const *argv[])
{
    GO_REBUILD_URSELF();
    CMD("clang","main.c")
    return 0;
}
```
- In shell build
```sh
clang cbuild.c
./cbuild.exe
```