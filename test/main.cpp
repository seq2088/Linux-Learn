#include <stdio.h>
#include <sys/utsname.h>

int main(void)
{
    struct utsname sys_info;
    if(uname(&sys_info) == 0)
    {
        printf("Hello,Embedded Linux from Raspberry Pi5!\n");
        printf("Syetem %s %s\n",sys_info.sysname,sys_info.machine);

    }
    else
    {

        perror("uname failed");

    }

    return 0;

}