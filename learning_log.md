# Learning Log

這份文件記錄課程里程碑。各實驗的完整命令、輸出與分析會保存在對應 lab 的 README 或 notes 中。

## 2026-09-22 — 建立課程基線

### 完成

- 保存原始 Linux Kernel Design 學習地圖。
- 建立 mastery-gated 實作課程。
- 定義 Phase 0–9、評量規則與實驗安全邊界。
- 將 Process 單元拆成 P0–P9，作為第一輪正式課程。

### 尚未驗證

- Fedora Lab 的實際 OS、kernel 與 toolchain 版本。
- C、pointer、toolchain、GDB 與 Linux process 的目前能力基線。

### 下一步

- 執行 Phase 0 入學診斷。
- 依診斷結果直接進入 P0，或先插入必要的補強單元。

## 2026-10-06 — Phase 0 診斷 01

### 完成

- 完成 C pointer、object lifetime、struct access、toolchain 與 user/kernel boundary 的五題診斷。
- 基線為 4/10；已有基本直覺，但精確型別、lifetime、完整編譯流程與 system-call boundary 尚未達 gate。

### 決定

- 先補強 pointer/lifetime、toolchain、user/kernel boundary，再進 Process P0。
- 保留原始回答與逐題評量於 `diagnostic/01-c-toolchain-baseline.md`。

### 下一步

- 完成 pointer 與 object lifetime 的第一個小單元及複測。

## 2026-10-06 — 改為實作驅動學習

### 回饋

- 紙筆式補強無法讓學習者在真實 Linux 行為中建立 kernel design 理解。

### 調整

- 改用「實作 → 觀察 → 解釋 → 追入 kernel」作為主循環。
- C、toolchain 與 system-call 概念改成 lab 中需要時即時補充。
- 建立第一個 `P0 Process Identity` lab；核心 identity API 保留為學習者 TODO。

### 下一步

- 在 Fedora Lab 完成四個 TODO。
- 交叉比對程式、`ps` 與 `/proc/<PID>/status` 的真實輸出。

