#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  char *end = sbrk(PGSIZE * 128);
  for(char *p = end; p < end + PGSIZE * 128; p += PGSIZE){
    for(int o = 0; o < PGSIZE - 32; o++){
      if(p[o] == 's' && p[o+1] == 'e' && p[o+2] == 'c' && p[o+3] == 'r' &&
         p[o+4] == 'e' && p[o+5] == 't' && p[o+6] == ' ' && p[o+7] == 'p' &&
         p[o+8] == 'w' && p[o+9] == ' ' && p[o+10] == 'i' && p[o+11] == 's'){
        write(2, p + o + 14, 8);
        exit(0);
      }
    }
  }
  exit(1);
}
