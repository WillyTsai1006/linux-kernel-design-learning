#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = /* TODO 1: 取得目前 process 的 PID */;
    pid_t ppid = /* TODO 2: 取得 parent process 的 PID */;
    uid_t uid = /* TODO 3: 取得目前 process 的 UID */;
    gid_t gid = /* TODO 4: 取得目前 process 的 GID */;

    printf("PID  = %ld\n", (long)pid);
    printf("PPID = %ld\n", (long)ppid);
    printf("UID  = %ld\n", (long)uid);
    printf("GID  = %ld\n", (long)gid);
    printf("Waiting 60 seconds for observation...\n");

    fflush(stdout);
    sleep(60);

    return 0;
}

