#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
 
#define __NR_hello 312 //349 if you are running a 32bit kernel and following my tutorial
 
long test_syscall(pid_t p)
{
    return syscall(337,p);
}
long test_syscall_s(int limit){
   return syscall(338,limit);
}
long test_syscall_ss(){
   return syscall(339);
}
int main(int argc, char *argv[])
{
    long int a = test_syscall(1);
    long int b = test_syscall_s(10);
    long int c = test_syscall_ss();
    long int d = test_syscall_s(5);
    printf("System call returned %ld\n", a);
    printf("System call returned %ld\n", b);
    printf("System call returned %ld\n", d);
    return 0;
}

