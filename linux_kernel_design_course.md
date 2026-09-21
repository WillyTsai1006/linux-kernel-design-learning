# Linux Kernel Design 實作課程

> 依據 `linux_kernel_design_learning_roadmap.md` 設計。這是一套以 mastery gate（能力關卡）推進的互動課程，不以固定週數趕進度。

## 1. 課程目標

完成主線後，你應能獨立完成四件事：

1. 從 user-space 程式追到 system call、kernel data structure 與硬體行為。
2. 解釋 Linux 的 process、scheduler、memory、VFS、concurrency、interrupt、driver 與 network 設計。
3. 在 QEMU 中編譯、啟動、觀察、除錯及安全地修改 kernel。
4. 用可重現的實驗與 patch 證明結論，而不是只憑印象描述原始碼。

這門課的「學會」不等於看完章節；每一階段都要留下程式、觀察紀錄、除錯證據與口頭解釋。

## 2. 已知條件與假設

- Stage 0 已完成，主要實驗環境是 Fedora 44 Linux Lab。
- 你能使用 shell，但不假設你已熟悉 C、assembly、computer architecture 或 OS internals。
- 課程主線先完成通用 kernel fundamentals，再選 specialization；不現在替你決定 Track A、B 或 C。
- xv6-riscv 用來看清 OS 設計；Linux 的 x86-64 實驗用來追真實 kernel。兩者概念相通，但 register、trap entry 與 page table 細節不可直接互套。
- 自訂 kernel、會造成 crash 的 module、sanitizer 與 driver 實驗一律先在 QEMU guest 中進行，不直接拿主要 Fedora 主機試錯。

## 3. 先備知識地圖

```text
C 基礎 ───────────────┬─> Linux system programming ─> syscall boundary
pointer / struct      │                               │
編譯與 Makefile ──────┘                               v
                                                  xv6 design
binary / ELF / ABI ─────> assembly / GDB ────────────┤
CPU / cache / VM ────────────────────────────────────┘
                                                       v
                                            Linux kernel fundamentals
                                                       v
                   process / scheduler / memory / VFS / concurrency
                                                       v
                         observability / debugging / engineering
                                                       v
                              specialization + upstream-quality project
```

若診斷顯示 C、pointer、binary 或 GDB 有缺口，就插入補強單元；這不是退級，而是避免在 kernel code 裡同時猜語言與設計。

## 4. 教學方式

每堂課固定使用以下節奏：

1. **Concept**：先建立一個可畫出的 mental model，並定義名詞。
2. **Example**：閱讀或執行最小範例，先預測再觀察。
3. **Quiz**：用 3–5 題短答檢查模型，不靠背誦指令。
4. **Practice**：你完成有 TODO 的核心實作；課程提供介面、限制與驗收，不直接代寫核心答案。
5. **Evidence**：保留命令、真實輸出、異常現象及你的解釋。
6. **Kernel connection**：把現象連到 system call、資料結構或 source path。

一次只引入一個主要觀察點。例如先證明 process 存在，再看 state；不會一次傾倒十個指令。你貼出真實結果後，才進行解讀與下一步。

### 每堂課的通過條件

- 不看資料，以自己的話回答核心問題。
- 能在執行前預測至少一個結果，並解釋預測錯誤的原因。
- 實驗可以從乾淨目錄重現。
- README 清楚分開「觀察到的事實」和「根據事實做出的推論」。
- 變更前後都有驗證；失敗輸出也要保留。

## 5. 課程主線

時間只作負荷參考；是否晉級由 gate 決定。

| Phase | 單元 | 建議投入 | 主要成果 | Gate |
|---|---|---:|---|---|
| 0 | 入學診斷與安全規則 | 2–4 小時 | 診斷紀錄、lab policy | 知道何時用 host、container、QEMU |
| 1 | Linux system programming | 45–70 小時 | mini shell、epoll server | 能從 API 解釋 kernel abstraction |
| 2 | Machine & binary fundamentals | 35–60 小時 | ABI/ELF/GDB、cache/VM labs | 能從 C 追到 instruction 與 memory |
| 3 | OS design with xv6 | 50–80 小時 | syscall、scheduler、COW、FS 修改 | 能解釋設計不變量及修改後果 |
| 4 | Linux kernel fundamentals | 25–45 小時 | 可重現的 QEMU custom kernel lab | 能 build、boot、debug、回復 |
| 5 | Linux core subsystems | 100–180 小時 | 4 個 subsystem investigations | 能由現象追 source path 和 data structure |
| 6 | Observability & debugging | 35–60 小時 | 同一問題的多工具診斷報告 | 能選對工具並說明觀測成本 |
| 7 | eBPF、container、performance | 50–90 小時 | monitor 或 mini runtime | 能連結 isolation、resource、event path |
| 8 | Kernel engineering | 30–60 小時 | bug reproduction + reviewable patch | 能做 bisect、test、checkpatch、review |
| 9 | 專精與 capstone | 80+ 小時 | 一個有深度的公開作品 | 有 benchmark、限制、可重現證據 |

### Phase 0：入學診斷

不先上課，先用小題確認起點：

- C：pointer、array、struct、function pointer、lifetime、undefined behavior。
- Toolchain：preprocess、compile、assemble、link 各自產生什麼。
- Debug：能在 GDB breakpoint 後查看 stack、register、memory。
- Linux：能區分 process、thread、file descriptor、socket。
- Architecture：能說明 privilege level、virtual address、interrupt 的直覺模型。

產出：`diagnostic/README.md`。診斷只用來安排補強，不計分。

### Phase 1：Linux system programming

順序與成果：

1. Process identity 與 `/proc` → process observation note
2. `fork`、copy-on-write → process tree experiment
3. `exec`、ELF image replacement → exec tracer
4. `wait`、zombie、orphan → lifecycle experiment
5. FD、open file description、redirection → mini-cat / mini-cp
6. pipe 與 IPC → two-process pipeline
7. signal 與 async constraints → signal-safe controller
8. virtual address space、`mmap`、page fault → maps/fault notebook
9. pthread、race、mutex、condition variable → bounded queue
10. socket、TCP state → echo client/server
11. blocking、poll、epoll → concurrent echo/chat server
12. 整合 → mini shell

Phase 1 gate：現場解釋並展示 mini shell 的 `fork → dup2/pipe → exec → wait` 路徑；能重現 zombie、處理 Ctrl+C，並說明 foreground/background job 的差異。

### Phase 2：Machine & binary fundamentals

1. x86-64 registers、stack frame、instruction stepping
2. System V AMD64 calling convention
3. ELF sections、segments、symbol、relocation、GOT/PLT
4. static/dynamic linking 與 loader
5. cache locality、cache line、false sharing
6. virtual address、page table、TLB、page fault
7. atomic operation 與 memory ordering 的入門模型

Phase 2 gate：選一個小型 C 程式，從 source、assembly、ELF、loader mapping 一路解釋到 runtime memory；再用 benchmark 證明一個 locality 或 false-sharing 現象。

### Phase 3：OS design with xv6

1. boot、privilege、trap path
2. syscall dispatch 與 argument passing
3. process lifecycle、sleep/wakeup
4. scheduler 與 context switch
5. page table、allocator、lazy allocation
6. copy-on-write 與 reference count invariant
7. buffer cache、inode、directory、logging
8. console/block device driver path

Phase 3 gate：新增 syscall、完成 COW 類修改、修改一項 scheduler 行為與一項 filesystem feature；每項都要有 failure case 與 regression test。若採用 MIT 6.S081 labs，先確認版本與題目授權要求。

### Phase 4：Linux kernel fundamentals

1. source tree 與文件導航
2. Kconfig、`.config`、Kbuild
3. 建立 minimal initramfs
4. QEMU boot、自動化 console capture
5. `vmlinux` symbols、GDB remote debugging
6. module lifecycle 與 kernel log
7. kernel coding style 與最小 patch

Phase 4 gate：從空白 build directory 產出 kernel + rootfs，在 QEMU 開機進 shell，命中一個 kernel breakpoint，載入/卸載安全的 hello module，並保存完整重現步驟。

### Phase 5：Linux core subsystems

依賴順序：

1. `task_struct`、clone/exec/exit
2. scheduler：runqueue、class、EEVDF、affinity、load balance
3. memory：VMA、fault、buddy、SLUB、reclaim、OOM、NUMA
4. VFS：mount、superblock、inode、dentry、file
5. concurrency：atomic、spinlock、mutex、RCU、barrier
6. interrupt、softirq、workqueue、timer
7. character device 與 Linux device model
8. network receive/transmit path、`sk_buff`、NAPI、socket

每個子系統都做同一套 investigation：

```text
使用者可見現象
→ trace point / call stack
→ 核心資料結構
→ state transition / invariant
→ concurrency rule
→ design trade-off
→ 最小修改或故障注入
→ 驗證與回復
```

Phase 5 gate：從 scheduler、memory、VFS/driver、network 各完成至少一份 investigation，其中至少一份包含安全的 kernel 修改。

### Phase 6：Observability & debugging

依問題選工具，而不是把工具當清單背：

- `dmesg`、`/proc`、sysfs：狀態與事件的第一層證據。
- perf：counter、sampling、scheduler timeline。
- ftrace / trace-cmd：function 與 tracepoint 路徑。
- bpftrace / eBPF：可程式化的動態觀察。
- GDB / KGDB：停止世界後檢查控制流與資料。
- KASAN、KCSAN、UBSAN、lockdep：針對特定 bug class。
- crash / drgn：dump 與 live-kernel data structure inspection。

Phase 6 gate：針對同一個 latency、race 或 memory bug，至少使用兩種工具交叉驗證，說明每種證據能證明什麼、不能證明什麼，以及 instrumentation overhead。

### Phase 7：eBPF、container 與 performance

1. BPF verifier、program type、map、tracepoint/kprobe/uprobe
2. syscall / scheduler / network observability programs
3. namespace、cgroup v2、capability、seccomp
4. mount、pivot_root、overlayfs
5. veth、bridge、network namespace
6. benchmark design：warm-up、variance、baseline、CPU affinity

Phase 7 gate：二選一完成主要作品：

- eBPF system monitor：有事件模型、drop/overhead 說明與對照驗證。
- mini container runtime：能建立隔離環境並清楚說明它不是完整安全邊界。

### Phase 8：Kernel engineering

1. 閱讀 commit history 與 mailing-list discussion
2. 建立最小 reproducer
3. `git bisect` 找 regression
4. patch 拆分、commit message、`checkpatch.pl`
5. sparse、Coccinelle 與 compiler diagnostics
6. reviewer mindset：correctness、concurrency、ABI、performance
7. syzkaller 基礎與 crash triage

Phase 8 gate：交付一個可重現 bug report、一個有測試的最小 patch，以及一份模擬 review 回應。是否真的寄 upstream 要另外確認目標 subsystem 的流程與 maintainer 要求。

### Phase 9：專精與 capstone

到 Phase 8 再共同選方向：

- Track A — Scheduler / Performance：適合 HPC、runtime、latency。
- Track B — Driver / AI Accelerator：適合 PCIe、DMA、IOMMU、accelerator runtime。
- Track C — eBPF / Cloud / AI Infra：適合 observability、container、networking。

選擇依據不是哪條最熱門，而是前三階段的實作證據：你最常主動深入什麼、在哪類 bug 上表現最好、願意長期維護哪種作品。

Capstone 必須包含：問題定義、architecture、最小 v1、測試、benchmark、profiling、已知限制、reproduction script 與 demo。

## 6. 評量規則

每個能力用 0–2 分：

- 0：無法解釋或結果不可重現。
- 1：能完成，但需提示，或無法解釋例外與限制。
- 2：能獨立預測、實作、觀察、解釋並指出 trade-off。

每個 Phase 的必要能力全部達 2 才晉級。漂亮 README 不取代正確性；static check 不等於 compile，compile 不等於 boot，boot 不等於行為正確，單次 benchmark 不等於效能結論。

## 7. 實驗紀錄結構

不必一開始建立十二個 repository。先用一個 learning lab 累積證據，作品成熟後再拆分。

```text
linux-kernel-design-lab/
├── README.md
├── diagnostic/
├── 01-system-programming/
├── 02-machine-fundamentals/
├── 03-xv6/
├── 04-linux-kernel/
├── investigations/
└── capstone/
```

每個 lab：

```text
lab-name/
├── README.md
├── src/
├── Makefile
├── expected/
└── notes/
```

README 最少包含：

```markdown
# Question
## Prediction
## Mental Model
## Environment
## Build and Run
## Observations
## Evidence
## Kernel Connection
## What Failed
## Conclusion and Limits
```

## 8. 第一輪課程：Process 單元

先只開放以下內容，通過後才進 FD：

| Lesson | 核心問題 | 主要觀察點 | 實作 | 驗收 |
|---|---|---|---|---|
| P0 | program 如何成為 process？ | shell 啟動一個 foreground process | process-info | 分清 executable、process、PID |
| P1 | kernel 保存哪些 identity？ | `/proc/<pid>/status` | proc reporter | 解釋 PID/PPID/UID/GID |
| P2 | `fork` 到底複製什麼？ | parent/child 分支與 page fault | fork experiment | 解釋 return value 與 COW |
| P3 | `exec` 為何不建立新 PID？ | 同 PID、不同 mappings | exec experiment | 解釋 process image replacement |
| P4 | parent 如何取得 child 結果？ | blocking `waitpid` 與 exit status | launcher v1 | 正確處理 normal/error exit |
| P5 | zombie 為何存在？ | `Z` state 與 unreaped status | zombie lab | 不把 zombie 誤解成仍在執行 |
| P6 | orphan 由誰接手？ | PPID transition | orphan lab | 以實測辨識 subreaper/PID 1 情況 |
| P7 | process state 代表什麼？ | R/S/T/Z 的觸發條件 | state lab | 分清 state snapshot 與完整歷史 |
| P8 | user API 如何進入 kernel？ | `strace` 的 process syscalls | annotated trace | 對應 API、syscall、source entry |
| P9 | 如何整合生命週期？ | fork/exec/wait error paths | mini-process-launcher | 通過功能與 failure tests |

### P0 的互動界線

第一堂不會一次跑完 process lab。流程是：

1. 先畫出 `source → executable → process`，回答三題預測題。
2. 建立最小 `process-info.c`，逐行說明 header、return type 與 system interface。
3. 編譯時說明 `-Wall -Wextra -g`，先看 compiler 是否成功。
4. 執行後只觀察 PID/PPID/UID/GID，貼出真實輸出。
5. 根據輸出釐清 identity 後，下一堂才把它與 `/proc`、`ps`、`strace` 交叉驗證。

P0 通過問題：

1. 原始碼、executable file、process 三者的差異是什麼？
2. 同一個 executable 能否同時對應多個 process？如何證明？
3. PID 是程式本身的屬性，還是一次執行實例的屬性？
4. 為什麼 UID/GID 要存在 process credential 中？

## 9. 現在的下一步

先做 Phase 0 的短診斷，再開始 P0。不要先建立全部 lab、clone Linux kernel 或安裝一長串套件；第一輪只確認 Fedora Lab 的實際版本、C 基礎與 process observation 能力。

診斷結果會決定：

- 直接進 P0；或
- 先插入 1–3 個 C / toolchain 補強課，再回 P0。

