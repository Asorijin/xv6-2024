#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc,char* argv[]){
	if(argc==2){
		int time = atoi(argv[1]);
		sleep(time);
		exit(0);
	}
	else{
		write(1,"usage sleep {sleepTime}\n",24);
		exit(1);
	} 

}
