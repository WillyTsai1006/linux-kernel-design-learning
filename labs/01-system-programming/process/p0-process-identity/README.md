# P0 — Process Identity

## 這次只回答一個問題

> 一個正在執行的程式，如何從 Linux 取得自己的 process identity？

這不是 API 背誦題。你要親手建立一個 process，再用程式輸出、`ps` 與 `/proc` 三個觀察來源交叉驗證。

## 開始前的預測

執行任何命令前，先在 `notes.md` 寫下你的預測：

1. 同一個程式連續執行兩次，PID 會不會相同？
2. PID 和 PPID 哪一個代表目前這個程式？
3. 程式印出的 PID，是否應和 `/proc/<PID>` 的目錄名稱一致？

預測可以錯，但不能事後改寫成正確答案。

## 你的任務

打開 `src/process_identity.c`，完成四個 `TODO`。

這次允許使用的介面：

```c
getpid()
getppid()
getuid()
getgid()
```

不要加入其他功能，也不要複製 roadmap 中的完成版。

## Build

```bash
make
```

編譯選項：

- `-Wall -Wextra -Wpedantic`：要求 compiler 顯示常見問題。
- `-std=c11`：使用 C11 語言標準。
- `-g`：保留之後給 GDB 使用的 debug information。

成功時會產生：

```text
build/process-identity
```

若 compiler 報錯，先保留完整訊息，不要只截最後一行。

## 第一個觀察點

先執行：

```bash
./build/process-identity
```

程式會印出 identity，然後等待 60 秒。先不要執行 `strace`。

在第二個 terminal，以程式印出的 PID 取代 `<PID>`：

```bash
ps -o pid,ppid,uid,gid,stat,comm -p <PID>
```

再執行：

```bash
grep -E '^(Name|State|Pid|PPid|Uid|Gid):' /proc/<PID>/status
```

把三份真實輸出貼回來：

1. `./build/process-identity`
2. `ps ...`
3. `grep ... /proc/<PID>/status`

我們會先判讀這三份證據，再開放下一個觀察點 `strace`。

## 驗收標準

- [ ] 四個 TODO 都由你完成。
- [ ] `make` 沒有 warning 或 error。
- [ ] 程式、`ps`、`/proc` 的 PID/PPID/UID/GID 能互相對應。
- [ ] 能說明 PID 與 PPID 各代表誰。
- [ ] 能分開描述觀察事實和自己的推論。

