#include <stdio.h>

void align_check(), align_free();
int debug_level=1;

main()

{
fprintf(stderr,"Calling align_check()\n");
align_check("test");

getchar();

fprintf(stderr,"Calling align_free()\n");
align_free("test");

fprintf(stderr,"Now test align_check2()\n");
align_check2("test");
}
