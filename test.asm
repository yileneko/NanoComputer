; ============================================================
; NanoComputer 全指令测试
;
; 用法:  ./build/nanocomputer  ->  输入 test.asm
;
; 这个 CPU 一共实现了 24 个操作码, 本文件逐条覆盖:
;   传送 01 07 08 09 0A 16 17      加法 02 0B 0C 0D 0E
;   减法 03 0F 10 11 12            比较跳转 04 05 06 13 14 15
;   停机 FF
;
; 判定方式: 每条测试把结果写进 0x0100~0x010F 对应格子,
;          程序跑完对照下面表格即可。全对则结果区应为:
;           AA 12 12 5A A5 0F 2C 07 0E 06 07 04 06 00 0F 77
;
; 内存布局(注意约束):
;   0x0000~  代码(本文件 < 0x00F0 字节)
;   0x00F0  寄存器间接寻址的缓冲区(只能放在 0x00~0xFF,
;           因为 [reg] 用 8 位寄存器当指针)
;   0x0100~  结果区
;
; 结果表:
;   0x0100 08 MOV [addr],imm     期望 AA
;   0x0101 01 MOV reg,imm    + 09 MOV [addr],reg        期望 12
;   0x0102 0A MOV reg,[addr] + 07 MOV reg,reg + 09      期望 12
;   0x0103 16 MOV reg,[reg]  + 09                       期望 5A
;   0x0104 17 MOV [reg],reg  + 16 + 09                  期望 A5
;   0x0105 02 ADD reg,imm                               期望 0F
;   0x0106 0B ADD reg,reg   (200+100 回绕)              期望 2C
;   0x0107 0C ADD [addr],imm + 0D ADD [addr],reg        期望 07
;   0x0108 0E ADD reg,[addr]                            期望 0E
;   0x0109 03 SUB reg,imm                               期望 06
;   0x010A 0F SUB reg,reg                               期望 07
;   0x010B 10 SUB [addr],imm + 11 SUB [addr],reg        期望 04
;   0x010C 12 SUB reg,[addr]                            期望 06
;   0x010D 06 CMP + 13 JZ + 14 JC + 15 JNC 位掩码       期望 00
;   0x010E 04 JNZ (1+2+3+4+5)                           期望 0F
;   0x010F 05 JMP (必须跳过写 FF 的陷阱)                期望 77
;   停机    FF HALT
; ============================================================

start:
; ============================================================
; 一、数据传送  01 07 08 09 0A 16 17
; ============================================================
    MOV [0x0100], 0xAA          ; 08 MOV [addr],imm
    MOV AL, 0x12                ; 01 MOV reg,imm
    MOV [0x0101], AL            ; 09 MOV [addr],reg   -> 期望 12
    MOV BL, [0x0101]            ; 0A MOV reg,[addr]   BL = 12
    MOV CL, BL                  ; 07 MOV reg,reg      CL = BL
    MOV [0x0102], CL            ; 09                  -> 期望 12

; ---- 间接寻址: 指针只能用 8 位寄存器, 缓冲区放 0x00F0 ----
    MOV [0x00F0], 0x5A
    MOV BH, 0x00F0              ; BH = 指针
    MOV DH, [BH]                ; 16 MOV reg,[reg]    DH = 5A
    MOV [0x0103], DH            ; 09                  -> 期望 5A
    MOV AH, 0xA5
    MOV [BH], AH                ; 17 MOV [reg],reg    [F0] = A5
    MOV DH, [BH]                ; 16  读回验证
    MOV [0x0104], DH            ; 09                  -> 期望 A5

; ============================================================
; 二、加法  02 0B 0C 0D 0E
; ============================================================
    MOV AL, 10
    ADD AL, 5                   ; 02 ADD reg,imm      = 15
    MOV [0x0105], AL            ;                     -> 期望 0F
    MOV AL, 200
    MOV BL, 100
    ADD AL, BL                  ; 0B ADD reg,reg      300 -> 2C
    MOV [0x0106], AL            ;                     -> 期望 2C
    MOV [0x0107], 1
    ADD [0x0107], 2             ; 0C ADD [addr],imm   = 3
    MOV BL, 4
    ADD [0x0107], BL            ; 0D ADD [addr],reg   = 7
                                ;                     -> 期望 07
    MOV AL, [0x0107]            ; 0A                  AL = 7
    ADD AL, [0x0107]            ; 0E ADD reg,[addr]   = 14
    MOV [0x0108], AL            ;                     -> 期望 0E

; ============================================================
; 三、减法  03 0F 10 11 12
; ============================================================
    MOV AL, 10
    SUB AL, 4                   ; 03 SUB reg,imm      = 6
    MOV [0x0109], AL            ;                     -> 期望 06
    MOV AL, 10
    MOV BL, 3
    SUB AL, BL                  ; 0F SUB reg,reg      = 7
    MOV [0x010A], AL            ;                     -> 期望 07
    MOV [0x010B], 10
    SUB [0x010B], 4             ; 10 SUB [addr],imm   = 6
    MOV BL, 2
    SUB [0x010B], BL            ; 11 SUB [addr],reg   = 4
                                ;                     -> 期望 04
    MOV AL, 10
    SUB AL, [0x010B]            ; 12 SUB reg,[addr]   = 6
    MOV [0x010C], AL            ;                     -> 期望 06

; ============================================================
; 四、比较与跳转  06 13 14 15 04 05
; ============================================================
; 做法: 三个条件跳转各自"跳对"则不加分, 跳错就给 0x010D
;       加上一个位标记(1/2/4), 最终应为 0
    MOV [0x010D], 0
    MOV AL, 5
    MOV BL, 5
    CMP AL, BL                  ; 06 CMP reg,reg   5 == 5 -> ZF=1
    JZ  jc_test                 ; 13 JZ            应跳转
    ADD [0x010D], 1             ;                  跳错则 +1
jc_test:
    MOV AL, 3
    CMP AL, BL                  ; 3 < 5 -> CF=1
    JC  jnc_test                ; 14 JC            应跳转
    ADD [0x010D], 2             ;                  跳错则 +2
jnc_test:
    MOV AL, 9
    CMP AL, BL                  ; 9 > 5 -> CF=0
    JNC jnz_test                ; 15 JNC           应跳转
    ADD [0x010D], 4             ;                  跳错则 +4
                                ;  -> 期望 0x010D = 00
jnz_test:
    MOV AL, 0
    MOV BL, 1
    MOV CL, 5
sum_loop:
    ADD AL, BL                  ; 1+2+3+4+5 = 15
    ADD BL, 1
    SUB CL, 1
    JNZ sum_loop                ; 04 JNZ
    MOV [0x010E], AL            ;  -> 期望 0F

    JMP jmp_ok                  ; 05 JMP
    MOV [0x010F], 0xFF          ; 陷阱: 被执行则结果变 FF
jmp_ok:
    MOV [0x010F], 0x77          ;  -> 期望 77

; ============================================================
; 五、停机  FF
; ============================================================
    HALT
