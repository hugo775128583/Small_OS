; 将idt载入idtr

[GLOBAL idt_flush]
idt_flush:
  mov eax, [esp+4]
  lidt [eax]
  ret
.end:


; 定义两个构造中断处理函数的宏有的有错误代码,有的没有

; 用于没有错误代码的中断
%macro ISR_NOERRCODE 1
[GLOBAL isr%1]
isr%1:
	cli                           ; 首先关闭中断
	push 0                        ; push无效的中断错误代码(起到占位作用,便于所有isr函数统一清理)
	push %1                       ; push中断号
	jmp isr_common_stub
%endmacro

; 用于有错误的处理的中断
%macro ISR_ERRCODE 1
[GLOBAL isr%1]
isr%1:
  cli
  push %1
  jmp isr_common_stub
%endmacro


; 定义中断处理函数
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19

; 20-31 intel保留
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; 32-255用户定义中断
ISR_NOERRCODE 255

[GLOBAL isr_common_stub]
[EXTERN isr_handler]
; 中断服务程序
isr_common_stub:
  pusha                         ; pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
  mov ax, ds
  push eax                      ; 保存数据段描述符

  mov ax, 0x10                  ; 加载内核数据段描述符
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  push esp                      ; 此时esp寄存器等价于pt_regs结构体的指针
  call isr_handler              ; 在C语言里面的代码
  add esp, 4                    ; 清除压入的参数

  pop ebx                       ; 恢复原来的数据段
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  popa
  add esp, 8
  iret
.end:

; 构造中断请求的宏
%macro IRQ 2
[GLOBAL irq%1]
irq%1:
  cli
  push byte 0
  push byte %2
  jmp irq_common_stub
%endmacro

IRQ 0, 32                 ; timer 
IRQ 1, 33                 ; keryboard 
IRQ 2, 34                 ; 与IRQ9相连,mpu-401DM使用 
IRQ 3, 33                 ; 串口设备 
IRQ 4, 36                 ; 串口设备 
IRQ 5, 37                 ; 建议声卡使用 
IRQ 6, 38                 ; 软驱传输控制器 
IRQ 7, 39                 ; 打印机传输控制器使用 
IRQ 8, 40                 ; 即使时钟 
IRQ 9, 41                 ; 有IRQ2相连 
IRQ 10, 42                ; 建议网卡使用 
IRQ 11, 43                ; 建议AGP显卡使用 
IRQ 12, 44                ; 建议PS/2鼠标 
IRQ 13, 45                ; 协处理器 
IRQ 14, 46                ; IDE0传输 
IRQ 15, 47                ; IDE1传输

[GLOBAL irq_common_stub]
[EXTERN irq_handler]
irq_common_stub:
  pusha

  mov ax, ds
  push eax                      ; 保存数据段描述符

  mov ax, 0x10                  ; 加载内核数据段描述符
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  push esp
  call irq_handler
  add esp, 4

  pop ebx                       ; 恢复现场
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  popa
  add esp, 8
  iret
.end:
