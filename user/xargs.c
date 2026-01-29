#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc,char* argv[]){
	char* ags[MAXARG];
	int cnts = argc-1;
	for(int i=1;i<argc;i++){
		ags[i-1] = argv[i];
	}
	while(1){
		char buf[512];
		char *ch;
		ch=buf;
		int stat=1;
		while(1){
			stat = read(0,ch,1);
			if(stat==0)
				exit(0);
			if(*ch=='\n'){
				*ch=0;
				break;
			}
			ch++;
		}
		ags[cnts]=malloc(sizeof(buf));
		memmove(ags[cnts],buf,sizeof(buf));
		int pid = fork();
		if(pid == 0){
			exec(ags[0],ags);
			exit(0);
		}
		else{
			wait(&pid);
		}
	}
}

