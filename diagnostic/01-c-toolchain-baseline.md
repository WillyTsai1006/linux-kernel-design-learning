# Phase 0 診斷 01：C 與 Toolchain 基線

- 日期：2026-10-06
- 狀態：已作答，需補強後複測
- 評分方式：每題 0–2 分；2 分代表能獨立且精確解釋

## 原始回答

### 1. Pointer 基本語意

> x 變數 p x的地址 p* 指針 &p p的地址

### 2. Local variable lifetime

> 回傳 type 是 int return 是地址

### 3. Struct member access

> 一樣吧 只是不同種類的使用

### 4. Compilation pipeline

> .a -> .o
>
> preprocessing 處理 macro 和一些關鍵字，compilation 是變成組合語言， assembly 變成二進位，linking 是連接 library

### 5. User/kernel boundary

> 程式本身在 user space ，getpid 在 kernel

## 評量

| 題目 | 分數 | 已有概念 | 需要補強 |
|---|---:|---|---|
| Pointer | 1/2 | 知道 `p` 保存 `x` 的地址 | `*p` 是 dereference；`&p` 的型別是 `int **` |
| Lifetime | 0/2 | 注意到回傳值與地址有關 | `int *` 與 `&value` 型別相容；真正問題是 local object 已結束 lifetime |
| Struct | 1/2 | 知道兩者能存取同一 member | `.` 用於 object；`->` 用於 pointer，且 `ptr->pid == (*ptr).pid` |
| Toolchain | 1/2 | 大致知道 compile、assemble、link | 完整路徑是 `.c → .i → .s → .o → ELF`；preprocessor 主要處理 directives |
| Boundary | 1/2 | 知道 `getpid()` 需要 kernel 資訊 | `printf()` 的 formatting 多在 libc；真正輸出仍經 `write` 類 system call 進 kernel |

總分：4/10。

## 診斷結論

先插入三個短補強單元，再進入 Process P0：

1. pointer、dereference、pointer-to-pointer 與 object lifetime
2. preprocess、compile、assemble、link 與 ELF
3. libc wrapper、system call 與 user/kernel boundary

## 下一個驗收點

能不用背地址數字，僅憑宣告判斷 `x`、`p`、`*p`、`&p` 的值域、型別及它們指向的物件。
