#include <dlfcn.h>
#include <stdio.h>

void * __wrap_dlsym(void *handle, const char *symbol) {
    fprintf(stderr, "dlerror: %s\n", dlerror());
    fprintf(stderr, "dlopen called for %s\n", symbol);
    fprintf(stderr, "Handle%s null\n", handle == NULL? "": " not");
    return __real_dlsym(handle, symbol);
}
