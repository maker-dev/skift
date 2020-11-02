
global gdt_flush
gdt_flush:
    push rbp
    mov rbp, rsp
    lgdt [rdi]

    mov ax, 16  
    mov ss, ax
    mov ds, ax
    mov es, ax

    push qword 0x10 ; data segment
    push rsp        ; rsp
    pushf           ; rflags
    push qword 0x8  ; code segment
    push trampoline ; rip
    iretq           ; pop everything


trampoline:
    pop rbp ; set rbp
    ret
global idt_flush
idt_flush:
    lidt [rdi]
    ret
