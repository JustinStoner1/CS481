#include <stdio.h>
#include <stdlib.h>
int main(void) {
	FILE *fd ; float pval=3.14159; int i,k;
	if ((fd= fopen("myTstFile","r+"))==NULL)
		printf("\n Program Failed, figure out why...\n");
	else {
		printf("\n Simple pie value %1.8f\n", pval);
		for (i=0; i<100; i++) {
			k = rand()%10;
			if (fprintf(fd, "%f\n",pval+i*k) == -1) perror("write err"); fflush(fd);
			printf("."); fflush(stdout);
			}
		fclose(fd); printf("\n Program successful ends\n");
		}
	}
