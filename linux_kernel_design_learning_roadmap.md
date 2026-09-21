# Linux Kernel Design & Systems Programming — 完整可執行學習路徑

> 目標：從 Linux user space 一路學到 Linux Kernel internals、核心子系統、除錯、eBPF、container internals、driver、performance，以及實際 kernel engineering。
>
> 適用環境：Fedora 44 Linux Lab
>
> 目前狀態：Stage 0 已完成，正式從 Stage 1 開始。

---

# 0. 最終目標

這條路線不是要你「會用 Linux」，而是要讓你最後能做到：

- 能解釋 user space 與 kernel space 的界線
- 能理解 system call 如何進入 kernel
- 能讀 Linux Kernel source code
- 能編譯與啟動自訂 kernel
- 能使用 QEMU + GDB debug kernel
- 能理解 process / scheduler / virtual memory / VFS / driver / network stack
- 能撰寫 kernel module
- 能理解 namespace / cgroup / container runtime
- 能使用 perf / ftrace / eBPF 分析系統
- 能找 kernel bug、重現、定位、修正
- 能閱讀 kernel patch
- 最終能嘗試提交 upstream patch

核心能力路線：

```text
User Program
    ↓
glibc / runtime
    ↓
System Call
    ↓
Kernel Entry
    ↓
Kernel Subsystem
    ↓
Hardware
```

完整階段：

```text
Stage 0  Linux Lab Environment
Stage 1  Linux System Programming
Stage 2  Computer Systems Fundamentals
Stage 3  Operating System Design with xv6
Stage 4  Linux Kernel Fundamentals
Stage 5  Linux Kernel Core Subsystems
Stage 6  Kernel Observability & Debugging
Stage 7  eBPF / Containers / Performance
Stage 8  Kernel Engineering
Stage 9  Specialization
```

---

# Stage 0 — Linux Lab Environment

## 目標

建立一台可以：

- 編譯
- 除錯
- 跑服務
- 做 kernel 實驗
- 做 QEMU VM
- 做系統觀察

的 Linux Lab。

## 目前已完成

你的 Fedora Lab：

```text
Fedora 44
Kernel 7.1.x
GCC
Clang
GDB
LLDB
Make
CMake
Git
Python
uv
Podman
podman-compose
strace
tcpdump
nmap
btop
htop
tmux
ripgrep
jq
SSH
/data
```

專案結構：

```text
~/projects
├── ai
├── csapp
├── linux
├── playground
└── tools
```

## 驗收標準

以下都能執行：

```bash
gcc --version
clang --version
gdb --version
git --version
strace --version
ssh localhost
```

Stage 0：

```text
STATUS: DONE
```

---

# Stage 1 — Linux System Programming

## Stage 1 核心目的

不要把這階段當成 API 教學。

真正目標：

> 理解 Linux kernel 提供給 user-space program 的基本 abstraction。

你要理解：

```text
Process
File Descriptor
Signal
Virtual Memory
Thread
Socket
IPC
Event Loop
```

---

# 1.0 建立學習目錄

執行：

```bash
cd ~/projects/linux

mkdir -p system-programming/{process-lab,fd-lab,ipc-lab,signal-lab,memory-lab,thread-lab,network-lab,epoll-lab}
```

確認：

```bash
tree -L 2 ~/projects/linux/system-programming
```

預期：

```text
system-programming
├── epoll-lab
├── fd-lab
├── ipc-lab
├── memory-lab
├── network-lab
├── process-lab
├── signal-lab
└── thread-lab
```

建立主 README：

```bash
cd ~/projects/linux/system-programming
vim README.md
```

建議內容：

```markdown
# Linux System Programming Lab

## Goal

Understand how user-space programs interact with the Linux kernel.

## Topics

- Process
- File descriptor
- IPC
- Signal
- Memory
- Thread
- Socket
- epoll
```

---

# 1.1 Process Lab

## 要理解的概念

- Program vs Process
- PID
- PPID
- UID / GID
- Parent / Child
- `fork()`
- `exec()`
- `wait()`
- `exit()`
- Zombie
- Orphan
- Process state
- `/proc/<pid>`

最終你要回答：

> Shell 執行一個指令時，Linux 到底做了什麼？

---

## Lab 1 — Process Identity

進入：

```bash
cd ~/projects/linux/system-programming/process-lab
```

建立：

```bash
vim 01-process-info.c
```

內容：

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    printf("PID  = %d\n", getpid());
    printf("PPID = %d\n", getppid());
    printf("UID  = %d\n", getuid());
    printf("GID  = %d\n", getgid());

    return 0;
}
```

編譯：

```bash
gcc -Wall -Wextra -g 01-process-info.c -o process-info
```

執行：

```bash
./process-info
```

觀察：

```bash
strace -e getpid,getppid,getuid,getgid ./process-info
```

再執行：

```bash
ps -ef | head
```

學會看 process tree：

```bash
pstree -p
```

若沒有：

```bash
sudo dnf install psmisc
```

### 驗收問題

你必須能回答：

1. Program 和 Process 差在哪？
2. PID 是什麼？
3. PPID 是什麼？
4. 同一個 program 能不能同時有多個 PID？
5. UID/GID 為什麼屬於 process？

---

## Lab 2 — fork()

建立：

```bash
vim 02-fork-demo.c
```

內容：

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t pid;

    printf("before fork: PID=%d\n", getpid());

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("child: PID=%d PPID=%d\n", getpid(), getppid());
    } else {
        printf("parent: PID=%d childPID=%d\n", getpid(), pid);
    }

    return 0;
}
```

編譯：

```bash
gcc -Wall -Wextra -g 02-fork-demo.c -o fork-demo
```

執行：

```bash
./fork-demo
```

追蹤：

```bash
strace -f ./fork-demo
```

觀察 child：

```bash
strace -f -e clone,clone3,fork,vfork ./fork-demo
```

### 要理解

`fork()` 後：

```text
Parent Process
      |
      fork
      |
  +---+---+
  |       |
Parent   Child
```

兩個 process 從 fork 後的位置繼續。

### 驗收問題

1. `fork()` 回傳值在 parent 和 child 分別是多少？
2. child PID 是否與 parent 相同？
3. fork 後記憶體是不是馬上完整複製？
4. Copy-on-write 是什麼？

---

## Lab 3 — exec()

建立：

```bash
vim 03-exec-demo.c
```

內容：

```c
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("before exec\n");

    execl("/bin/ls", "ls", "-l", NULL);

    perror("execl");

    return 1;
}
```

編譯：

```bash
gcc -Wall -Wextra -g 03-exec-demo.c -o exec-demo
```

執行：

```bash
./exec-demo
```

追：

```bash
strace -e execve ./exec-demo
```

### 要理解

exec 不會建立新的 process。

它是：

```text
same PID
   ↓
replace process image
   ↓
new program
```

### 驗收問題

1. exec 會不會產生新 PID？
2. exec 成功後為什麼不會回來？
3. shell 為什麼通常 fork 後再 exec？

---

## Lab 4 — wait()

建立：

```bash
vim 04-wait-demo.c
```

內容：

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("child running\n");
        sleep(2);
        return 42;
    }

    int status;

    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("child exit code = %d\n", WEXITSTATUS(status));
    }

    return 0;
}
```

編譯：

```bash
gcc -Wall -Wextra -g 04-wait-demo.c -o wait-demo
```

執行：

```bash
./wait-demo
```

---

## Lab 5 — Zombie

建立 zombie：

```bash
vim 05-zombie-demo.c
```

內容：

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        printf("child exit\n");
        exit(0);
    }

    printf("parent PID=%d child=%d\n", getpid(), pid);

    sleep(30);

    return 0;
}
```

執行：

```bash
gcc -Wall -Wextra -g 05-zombie-demo.c -o zombie-demo
./zombie-demo
```

另一個 terminal：

```bash
ps -o pid,ppid,state,cmd
```

觀察 `Z`。

### 核心理解

Zombie：

```text
child terminated
      ↓
exit status 尚未被 parent wait()
      ↓
kernel 保留少量 process metadata
```

---

## Process Lab Final

作品：

```text
mini-process-launcher
```

功能：

```bash
./launcher ls -l
```

內部：

```text
fork
  ↓
child exec
  ↓
parent wait
```

---

# 1.2 File Descriptor Lab

## 核心概念

理解：

```text
Process
  |
  +-- fd 0 -> stdin
  +-- fd 1 -> stdout
  +-- fd 2 -> stderr
  +-- fd 3 -> file
```

API：

```text
open
read
write
close
dup
dup2
```

---

## Lab 1 — open/read/write

建立：

```bash
cd ~/projects/linux/system-programming/fd-lab
vim 01-read-file.c
```

內容：

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {
    int fd = open("/etc/hostname", O_RDONLY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    char buf[128];

    ssize_t n = read(fd, buf, sizeof(buf));

    if (n > 0) {
        write(STDOUT_FILENO, buf, n);
    }

    close(fd);

    return 0;
}
```

編譯：

```bash
gcc -Wall -Wextra -g 01-read-file.c -o read-file
```

追蹤：

```bash
strace -e openat,read,write,close ./read-file
```

---

## Lab 2 — stdout redirection

建立：

```bash
vim 02-redirect.c
```

使用：

```c
dup2()
```

理解：

```bash
ls > output.txt
```

背後的本質不是 shell 神奇處理，而是：

```text
open output.txt
dup2(file_fd, STDOUT_FILENO)
exec ls
```

---

## Final Project

做：

```text
mini-cat
mini-cp
```

---

# 1.3 IPC Lab

## 學習內容

- Anonymous pipe
- Named pipe
- Shared memory
- Unix domain socket

---

## Pipe 基本 Lab

```bash
cd ~/projects/linux/system-programming/ipc-lab
vim 01-pipe.c
```

實作：

```text
Parent
  |
 pipe
  |
Child
```

最終理解：

```bash
ls | grep linux
```

本質：

```text
pipe
fork
dup2
exec
```

---

# 1.4 Signal Lab

學：

```text
SIGINT
SIGTERM
SIGKILL
SIGCHLD
```

API：

```text
kill()
sigaction()
```

觀察：

```bash
sleep 100
```

另一 terminal：

```bash
ps aux | grep sleep
kill PID
```

觀察：

```bash
kill -SIGSTOP PID
kill -SIGCONT PID
kill -SIGTERM PID
kill -SIGKILL PID
```

### 驗收問題

1. SIGTERM 與 SIGKILL 差在哪？
2. Ctrl+C 是什麼 signal？
3. SIGCHLD 何時產生？
4. 為什麼 SIGKILL 不能被 handler 捕捉？

---

# 1.5 Memory Lab

## 核心

先理解 process virtual address space：

```text
high address
+------------------+
| stack            |
+------------------+
| mmap area        |
+------------------+
| heap             |
+------------------+
| bss              |
+------------------+
| data             |
+------------------+
| text             |
+------------------+
low address
```

建立：

```bash
cd ~/projects/linux/system-programming/memory-lab
vim 01-memory-layout.c
```

程式中：

- global variable
- static variable
- local variable
- malloc pointer
- function pointer

印出 address。

同時：

```bash
cat /proc/$(pidof your-program)/maps
```

研究：

```text
stack
heap
shared library
ELF mapping
```

---

# 1.6 Thread Lab

使用：

```text
pthread_create
pthread_join
pthread_mutex
pthread_cond
```

Lab：

```text
01-thread-basic
02-race-condition
03-mutex
04-producer-consumer
05-thread-pool
```

### 核心問題

1. Thread 和 Process 差在哪？
2. Thread 共享什麼？
3. Thread 不共享什麼？
4. Race condition 是什麼？
5. Mutex 為什麼能解決問題？

---

# 1.7 Network / Socket Lab

做：

```text
TCP echo server
TCP client
```

API：

```text
socket
bind
listen
accept
connect
send
recv
```

觀察：

```bash
ss -lntp
```

抓封包：

```bash
sudo tcpdump -i any port 8080
```

---

# 1.8 epoll Lab

先比較：

```text
blocking
select
poll
epoll
```

最後做：

```text
epoll-chat-server
```

核心問題：

> 一個單 thread server 怎麼處理幾千條 connection？

---

# Stage 1 Final Project — Mini Shell

目標：

```bash
./mysh
```

支援：

```bash
ls
ls -l
cd ..
pwd
cat file
cat file | grep linux
ls > output.txt
sleep 10 &
Ctrl+C
```

你必須使用：

```text
fork
exec
wait
pipe
dup2
signal
file descriptor
```

### Stage 1 驗收

你必須能清楚回答：

- Program vs Process
- fork
- exec
- wait
- zombie
- FD
- pipe
- signal
- virtual memory
- thread
- socket
- epoll

---

# Stage 2 — Computer Systems Fundamentals

這階段補 kernel 必須的 CPU 與 binary 基礎。

建議搭配：

```text
CSAPP
```

---

# 2.1 x86-64 Assembly

學：

```text
rax
rbx
rcx
rdx
rsp
rbp
rip
```

指令：

```text
mov
lea
push
pop
call
ret
cmp
jmp
```

實驗：

```bash
gcc -O0 -S hello.c
```

看 assembly：

```bash
less hello.s
```

反組譯：

```bash
objdump -d ./hello
```

GDB：

```bash
gdb ./hello
```

在 GDB：

```text
break main
run
disassemble main
info registers
x/16gx $rsp
```

---

# 2.2 Calling Convention

理解 System V AMD64 ABI：

參數：

```text
rdi
rsi
rdx
rcx
r8
r9
```

return：

```text
rax
```

核心問題：

> C function call 到底如何變成 machine instruction？

---

# 2.3 ELF

工具：

```bash
readelf -h ./program
readelf -S ./program
readelf -l ./program
nm ./program
objdump -d ./program
```

理解：

```text
.text
.data
.bss
.rodata
GOT
PLT
```

---

# 2.4 Linker / Loader

理解流程：

```text
source.c
 ↓
compiler
 ↓
assembly
 ↓
assembler
 ↓
object file
 ↓
linker
 ↓
ELF
 ↓
loader
 ↓
process
```

實驗：

```bash
gcc -c a.c
gcc -c b.c
gcc a.o b.o -o app
```

---

# 2.5 Cache

學：

```text
cache line
L1
L2
L3
locality
false sharing
```

做 Cache Lab。

---

# 2.6 Virtual Memory

理解：

```text
virtual address
 ↓
page table
 ↓
physical address
```

學：

```text
page
PTE
TLB
page fault
```

---

# Stage 2 建議實作

```text
CSAPP Bomb Lab
CSAPP Attack Lab
CSAPP Cache Lab
CSAPP Shell Lab
CSAPP Malloc Lab
```

---

# Stage 3 — Operating System Design with xv6

## 為什麼先學 xv6

Linux Kernel 太大。

xv6 可以讓你完整看到：

```text
boot
trap
syscall
scheduler
memory
filesystem
driver
```

---

# 3.1 xv6 Setup

建立：

```bash
cd ~/projects
mkdir -p os
cd os
```

clone xv6：

```bash
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv
```

Fedora 套件名稱可能不同。

需要：

```text
RISC-V GCC
QEMU
make
```

啟動：

```bash
make qemu
```

---

# 3.2 Trap / Syscall

追：

```text
user code
 ↓
ecall
 ↓
trap
 ↓
syscall dispatcher
 ↓
kernel function
```

新增自己的 syscall。

例如：

```text
getreadcount()
```

---

# 3.3 Scheduler

讀：

```text
kernel/proc.c
```

理解：

```text
RUNNABLE
RUNNING
SLEEPING
ZOMBIE
```

修改 scheduler。

---

# 3.4 Virtual Memory

讀：

```text
kernel/vm.c
```

Lab：

```text
page table printer
lazy allocation
copy-on-write
```

---

# 3.5 Filesystem

理解：

```text
inode
directory
buffer cache
block
```

---

# Stage 3 Final

必須做到：

```text
新增 syscall
修改 scheduler
完成 COW
理解 page table
修改 filesystem feature
```

---

# Stage 4 — Linux Kernel Fundamentals

現在正式進 Linux Kernel。

---

# 4.1 Kernel Source Tree

clone：

```bash
cd ~/projects/linux
git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git kernel-src
cd kernel-src
```

熟悉：

```text
arch/
drivers/
fs/
include/
kernel/
mm/
net/
security/
```

搜尋：

```bash
rg "task_struct"
```

---

# 4.2 Kernel Build

Fedora 安裝必要 dependency。

先：

```bash
sudo dnf builddep kernel
```

若需要其他套件：

```bash
sudo dnf install \
  ncurses-devel \
  flex \
  bison \
  openssl-devel \
  elfutils-libelf-devel \
  dwarves
```

產生 config：

```bash
cp /boot/config-$(uname -r) .config
```

更新：

```bash
make olddefconfig
```

menuconfig：

```bash
make menuconfig
```

編譯：

```bash
make -j$(nproc)
```

---

# 4.3 QEMU Kernel Lab

不要直接拿實體 Fedora 當 kernel crash test。

建立：

```text
QEMU
 + Linux kernel
 + BusyBox rootfs
```

目標：

```text
QEMU
 ↓
custom kernel
 ↓
initramfs
 ↓
BusyBox shell
```

---

# 4.4 Kernel Module

hello module：

```c
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
    pr_info("hello kernel\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("bye kernel\n");
}

module_init(hello_init);
module_exit(hello_exit);
```

Makefile：

```make
obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

build：

```bash
make
```

load：

```bash
sudo insmod hello.ko
```

看：

```bash
dmesg | tail
```

remove：

```bash
sudo rmmod hello
```

---

# Stage 5 — Linux Kernel Core Subsystems

這是整個 roadmap 核心。

---

# 5A — Process Management

讀：

```text
include/linux/sched.h
kernel/fork.c
kernel/exit.c
fs/exec.c
```

核心：

```text
task_struct
```

追 fork：

```text
user fork
 ↓
glibc
 ↓
clone
 ↓
kernel_clone
 ↓
copy_process
 ↓
task_struct
```

Lab：

```text
Process Lifecycle Tracer
```

---

# 5B — Scheduler

讀：

```text
kernel/sched/
```

學：

```text
runqueue
scheduler class
context switch
CPU affinity
load balancing
EEVDF
RT
deadline
```

觀察：

```bash
perf sched record
perf sched timehist
```

Final Project：

```text
Linux Scheduler Visualizer
```

---

# 5C — Memory Management

讀：

```text
mm/
```

學：

```text
struct page
VMA
page table
page fault
buddy allocator
SLUB
reclaim
swap
OOM
NUMA
```

觀察：

```bash
cat /proc/meminfo
cat /proc/buddyinfo
cat /proc/slabinfo
```

Final：

```text
Page Fault Tracer
Memory Allocation Analyzer
```

---

# 5D — VFS / Filesystem

核心：

```text
super_block
inode
dentry
file
```

讀：

```text
fs/
```

實驗：

```text
procfs module
simple pseudo filesystem
```

---

# 5E — Kernel Concurrency

學：

```text
atomic
spinlock
mutex
rwlock
RCU
memory barrier
```

理解：

```text
SMP
preemption
interrupt context
process context
```

工具：

```text
lockdep
KCSAN
```

---

# 5F — Interrupt / Deferred Work

學：

```text
IRQ
softirq
tasklet
workqueue
timer
```

核心問題：

> 為什麼 interrupt handler 要短？

---

# 5G — Device Driver

先：

```text
character device
```

再：

```text
device model
platform driver
PCI
USB
MMIO
interrupt
DMA
IOMMU
```

強烈建議 Final Project：

```text
QEMU Virtual Device
+
Linux Kernel Driver
```

---

# 5H — Network Stack

核心：

```text
socket
sk_buff
TCP
UDP
IP
routing
Netfilter
NAPI
```

追：

```text
NIC
 ↓
driver
 ↓
NAPI
 ↓
network stack
 ↓
socket
 ↓
process
```

Final：

```text
Packet Path Tracer
```

---

# Stage 6 — Kernel Observability & Debugging

你要會：

```text
dmesg
/proc
/sys
perf
ftrace
trace-cmd
bpftrace
eBPF
GDB
KGDB
crash
drgn
```

---

# 6.1 perf

基本：

```bash
perf stat ./program
```

record：

```bash
perf record ./program
```

report：

```bash
perf report
```

---

# 6.2 ftrace

確認：

```bash
sudo mount -t debugfs none /sys/kernel/debug
```

看 tracer：

```bash
cat /sys/kernel/debug/tracing/available_tracers
```

---

# 6.3 bpftrace

Fedora：

```bash
sudo dnf install bpftrace
```

測：

```bash
sudo bpftrace -e 'tracepoint:syscalls:sys_enter_execve { printf("%s\n", comm); }'
```

---

# 6.4 Debug Sanitizers

學：

```text
KASAN
KCSAN
UBSAN
lockdep
```

---

# Stage 7 — eBPF / Containers / Performance

---

# 7.1 eBPF

學：

```text
BPF program
BPF map
tracepoint
kprobe
uprobe
XDP
```

作品：

```text
syscall monitor
scheduler tracer
network monitor
```

---

# 7.2 Container Internals

學：

```text
namespace
cgroup
capability
seccomp
overlayfs
veth
bridge
```

不要直接從 Kubernetes 開始。

Final Project：

```text
Mini Container Runtime
```

理想介面：

```bash
mycontainer run /bin/bash
```

內部：

```text
clone
namespace
mount
pivot_root
cgroup
seccomp
```

---

# Stage 8 — Kernel Engineering

開始真正工程化。

學：

```text
git
patch
bisect
regression
code review
mailing list
```

---

# 8.1 Git workflow

看 log：

```bash
git log --oneline --graph
```

找特定檔案歷史：

```bash
git log -- kernel/sched/core.c
```

blame：

```bash
git blame kernel/sched/core.c
```

---

# 8.2 git bisect

```bash
git bisect start
git bisect bad
git bisect good <commit>
```

---

# 8.3 checkpatch

```bash
./scripts/checkpatch.pl patch.diff
```

---

# 8.4 Static Analysis

學：

```text
sparse
Coccinelle
clang static analyzer
```

---

# 8.5 syzkaller

進階 fuzzing：

```text
syscall fuzz
kernel crash
reproducer
```

---

# Stage 9 — Specialization

最後分三條。

---

# Track A — Scheduler / Performance

深入：

```text
EEVDF
CPU topology
NUMA
scheduler latency
CPU isolation
real-time
cache behavior
```

適合：

```text
HPC
AI Runtime
AI Infrastructure
```

---

# Track B — Driver / AI Accelerator

深入：

```text
PCIe
DMA
IOMMU
MMIO
interrupt
GPU/NPU runtime
```

目標：

```text
Linux
 ↓
Driver
 ↓
Runtime
 ↓
Accelerator
```

---

# Track C — eBPF / Cloud / AI Infra

深入：

```text
eBPF
XDP
container runtime
cgroup
observability
network performance
```

適合：

```text
Cloud Infrastructure
AI Infrastructure
MLOps Infrastructure
```

---

# 最終 GitHub Project Map

建議最後至少有：

```text
01-linux-system-programming-lab
02-mini-shell
03-csapp-labs
04-xv6-kernel-lab
05-custom-linux-kernel-lab
06-linux-kernel-modules
07-scheduler-visualizer
08-memory-tracer
09-ebpf-system-monitor
10-mini-container-runtime
11-qemu-virtual-device
12-linux-device-driver
```

---

# 每個主題固定學習模板

之後每個主題都照：

```text
1. 問題
2. 建立 mental model
3. 寫最小 C program
4. gcc 編譯
5. 執行
6. strace
7. gdb
8. /proc / sysfs 觀察
9. 找 kernel source
10. 追 code path
11. 解釋 design
12. 修改行為
13. 寫 README
14. 驗收問題
```

---

# Git 學習紀錄格式

每個 lab：

```text
lab-name/
├── README.md
├── src/
├── Makefile
└── notes/
```

README：

```markdown
# Topic

## Goal

## Mental Model

## Experiment

## Commands

## Observations

## Kernel Connection

## Questions

## Conclusion
```

commit：

```bash
git add .
git commit -m "lab: understand fork process creation"
```

---

# 建議實際學習節奏

不要按「時間」硬趕。

用 mastery gate。

每個 Stage 必須完成：

```text
概念理解
+
Lab
+
觀察
+
Debug
+
README
+
Final Project
```

才能進下一階段。

---

# 現在立刻開始

目前你在：

```text
Stage 1
└── Process Lab
```

第一輪順序：

```text
01 process-info
02 fork
03 exec
04 wait
05 zombie
06 orphan
07 process-state
08 /proc analysis
```

完成後：

```text
Process Final
    ↓
mini-process-launcher
```

然後才進：

```text
File Descriptor Lab
```

---

# 第一階段核心驗收

完成 Process Lab 後，你必須不看資料回答：

1. Program 和 Process 差在哪？
2. kernel 如何識別 process？
3. PID / PPID 是什麼？
4. fork 的核心語意是什麼？
5. exec 和 fork 最大差異？
6. 為什麼 shell 要 fork + exec？
7. wait 的作用？
8. zombie 為什麼存在？
9. orphan 誰接手？
10. process 在 Linux kernel 裡主要由什麼資料結構表示？

最後一題的答案會把你帶進：

```text
task_struct
```

也就是 Linux Kernel Process Management 的真正入口。

---

# 完整主線一句話

```text
Linux API
→ System Call
→ Kernel Data Structure
→ Kernel Subsystem
→ Hardware
→ Design Trade-off
```

你未來所有 Linux Kernel 學習，都沿著這條主線進行。
