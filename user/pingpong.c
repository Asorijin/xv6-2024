#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
	int p1[2],p2[2],nowPid;
	pipe(p1);
	pipe(p2);
	char buf[6];
	if(fork()==0){
		nowPid = getpid();
		close(0);
		dup(p1[0]);
		read(p1[0],buf,5);
		printf("%d: received ping\n",nowPid);
		write(p2[1],"pong\n",5);
		close(p1[0]);
		close(p1[1]);
	}
	else{
		nowPid = getpid();
		close(0);
		dup(p2[0]);
		write(p1[1],"ping\n",5);
		read(p2[1],buf,5);
		printf("%d: received pong\n",nowPid);
		close(p2[0]);
		close(p2[1]);
	}
	exit(0);
}
