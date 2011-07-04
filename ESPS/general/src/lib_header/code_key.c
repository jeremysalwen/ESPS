static char alpha[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxzy._012";
/*                   012345678901234567890123456789012345678901234567890123456*/
/*                             11111111112222222222333333333344444444445555555*/



#include <stdio.h>
main(argc,argv)
int argc;
char **argv;

{
	char *name;
	int i,j;

	if(argc != 2) {
	  printf("usage: code_key key\n");
	  exit(1);
	}
	
        name = argv[1];
	j=strlen(name);
	printf("char %s[] = {",name);
	for(i=0;i<j;i++)
	  printf("%d, ",code(name[i]));
	printf("-1}\n");
}

code(c)
char c;
{
	int i;
	for(i=0;i<strlen(alpha);i++) 
		if(c == alpha[i]) return i;
	return -1;
}
