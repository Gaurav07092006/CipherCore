global _encrypt_char
global _decrypt_char

section .text

_encrypt_char:
    mov eax, [esp+4]  
    mov edx, [esp+8]  

    ; Check if Number ('0'=48, '9'=57)
    cmp eax, 48
    jl .done_enc
    cmp eax, 57
    jle .do_num_enc

    ; Check if Uppercase ('A'=65, 'Z'=90)
    cmp eax, 65
    jl .done_enc
    cmp eax, 90
    jle .do_upper_enc

    ; Check if Lowercase ('a'=97, 'z'=122)
    cmp eax, 97
    jl .done_enc
    cmp eax, 122
    jle .do_lower_enc

    jmp .done_enc

.do_num_enc:
    add eax, edx
.wrap_num_enc:
    cmp eax, 57
    jle .done_enc
    sub eax, 10
    jmp .wrap_num_enc

.do_upper_enc:
    add eax, edx
.wrap_upper_enc:
    cmp eax, 90
    jle .done_enc
    sub eax, 26
    jmp .wrap_upper_enc

.do_lower_enc:
    add eax, edx
.wrap_lower_enc:
    cmp eax, 122
    jle .done_enc
    sub eax, 26
    jmp .wrap_lower_enc

.done_enc:
    ret               


_decrypt_char:
    mov eax, [esp+4]  
    mov edx, [esp+8]  

    ; Check if Number ('0'=48, '9'=57)
    cmp eax, 48
    jl .done_dec
    cmp eax, 57
    jle .do_num_dec

    ; Check if Uppercase ('A'=65, 'Z'=90)
    cmp eax, 65
    jl .done_dec
    cmp eax, 90
    jle .do_upper_dec

    ; Check if Lowercase ('a'=97, 'z'=122)
    cmp eax, 97
    jl .done_dec
    cmp eax, 122
    jle .do_lower_dec

    jmp .done_dec

.do_num_dec:
    sub eax, edx
.wrap_num_dec:
    cmp eax, 48
    jge .done_dec
    add eax, 10
    jmp .wrap_num_dec

.do_upper_dec:
    sub eax, edx
.wrap_upper_dec:
    cmp eax, 65
    jge .done_dec
    add eax, 26
    jmp .wrap_upper_dec

.do_lower_dec:
    sub eax, edx
.wrap_lower_dec:
    cmp eax, 97
    jge .done_dec
    add eax, 26
    jmp .wrap_lower_dec

.done_dec:
    ret