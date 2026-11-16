; ============================================================
; NanoComputer 冒泡排序 (升序)
;
; 用法: ./build/nanocomputer -> 输入 test.asm
;
; 内存布局:
;   0x00F0 ~ 0x00F7  待排序数组, 共 8 个元素
;   0x00F8           内层统计数 (每轮减 1)
;   0x00F9           外层剩余轮数
;   0x00FA           数组基址 (= 0xF0)
;
; 为什么数组放在 0x00F0:
;   汇编器固定把代码装载到 0x0000, 且 [reg] 间接寻址只能用
;   8 位寄存器当地址(0x00~0xFF), 所以数组只能放在
;   "代码尾之后、0x00FF 之前" 这一小段里。
;   必须保证本文件总长度 < 0x00F0。
;
; 寄存器分配:
;   BH = arr[j] 的地址 (基址 + BL)
;   BL = 内层下标 j
;   CL = 内层剩余比较次数
;   AL = arr[j]      DL = arr[j+1]
;
; 运行结果: 0x00F0~0x00F7 = 04 08 11 17 2A 37 47 5D
;           (即 4, 8, 17, 23, 42, 55, 71, 93)
; ============================================================

start:
; ---------- 准备数据 ----------
    MOV [0x00F0], 42            ; arr[0]
    MOV [0x00F1], 17            ; arr[1]
    MOV [0x00F2], 93            ; arr[2]
    MOV [0x00F3], 8             ; arr[3]
    MOV [0x00F4], 55            ; arr[4]
    MOV [0x00F5], 23            ; arr[5]
    MOV [0x00F6], 71            ; arr[6]
    MOV [0x00F7], 4             ; arr[7]

    MOV [0x00F8], 7             ; 第一轮比较 7 次
    MOV [0x00F9], 7             ; 共 7 轮 (n-1)
    MOV [0x00FA], 0xF0          ; 数组基址

; ---------- 外层循环: 每轮把最大的送到末尾 ----------
outer:
    MOV BL, 0                   ; j = 0
    MOV CL, [0x00F8]            ; 本轮比较次数

; ---------- 内层循环: 比较 arr[j] 与 arr[j+1] ----------
inner:
    MOV BH, [0x00FA]            ; BH = 基址
    ADD BH, BL                  ; BH = &arr[j]
    MOV AL, [BH]                ; AL = arr[j]
    ADD BH, 1                   ; BH = &arr[j+1]
    MOV DL, [BH]                ; DL = arr[j+1]

    CMP AL, DL                  ; 无符号比较, CF = (AL < DL)
    JC  no_swap                 ; arr[j] <  arr[j+1] -> 顺序正确
    JZ  no_swap                 ; arr[j] == arr[j+1] -> 不必交换

    ; ---- 交换: 此时 BH = &arr[j+1] ----
    MOV [BH], AL                ; arr[j+1] = 原来的 arr[j]
    SUB BH, 1                   ; BH = &arr[j]
    MOV [BH], DL                ; arr[j]   = 原来的 arr[j+1]

no_swap:
    ADD BL, 1                   ; j++
    SUB CL, 1
    JNZ inner                   ; 内层未结束

; ---------- 一轮结束: 下一次少比较一次 ----------
    SUB [0x00F8], 1             ; 内层上限 -1
    SUB [0x00F9], 1             ; 剩余轮数 -1
    JZ  done                    ; 轮数减到 0 -> 排序完成
    JMP outer

done:
    HALT
