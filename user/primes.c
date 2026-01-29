#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
	int nowDrop=2,passNum=2;
	int pl,pr;
	int p[2];
	pipe(p);
	pl=p[0],pr=p[1];
	if(fork()==0){
		pl=p[0];
		p[1]=0;
		nowDrop=3;
		sleep(1);
		printf("prime 3\n");
		while(read(pl,&passNum,4)>=0){
			if(passNum%nowDrop==0){
				continue;
			}
			else{
				if(p[1]==0){
					pipe(p);
					pr=p[1];
					p[1]=0;
					if(fork()==0){
						printf("prime %d\n",passNum);
						pl=p[0];
						nowDrop = passNum;
						continue;
					}
					p[1]=pr;
				}
				if(pr!=0)
				write(pr,&passNum,4);
			}
		}
		close(pr);
		close(pl);
		exit(0);
	}
	else{
		printf("prime %d\n",nowDrop);
		for(int i=2;i<=280;i++){
			if(i%nowDrop==0){
				continue;
			}
			else{
				sleep(1);
				write(pr,&i,4);
			}
		}
		close(pr);
	}
	exit(0);
}
