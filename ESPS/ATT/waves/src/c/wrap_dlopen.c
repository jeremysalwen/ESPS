#include <dlfcn.h>
#include <stdio.h>

void * __wrap_dlopen(const char *filename, int flag) {
fprintf(stderr, "wrapper: dlopen(%s,%i)\n", filename, flag);
return __real_dlopen(filename,flag);
}
