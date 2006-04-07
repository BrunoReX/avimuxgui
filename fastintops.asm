.686
.MMX
.model flat, stdcall


public shr64
public Int2VSUInt_asm
public zerobits64

_CODE SEGMENT PARA PUBLIC USE32 "Data"


shr64 proc i, j
    push esi
	mov  esi, i
	mov  ecx, j
	mov  edx, [esi+4]
	mov  eax, [esi]
	or   edx, edx
	jz   @1

	shrd eax, edx, cl
	shr  edx, cl
	mov  [esi], eax
	mov  [esi+4], edx
	pop  esi
	ret

@1: shr  eax, cl
 	mov  [esi], eax
	pop  esi
	ret
shr64 endp

; __int64  FSSInt2Int (char* x, int iLen)

FSSInt2Int proc dwX, dwL
    push esi
	push ebx
    mov  esi, dwX
	mov  ecx, dwL
	xor  eax, eax
	xor  edx, edx
	xor  ebx, 0
	mov  bl, [esi]
	bt   ebx, 7
	mov  ebx, 0
	sbb  ebx, 0
	mov  eax, ebx
	mov  edx, ebx

@1: shld edx, eax, 8
    shl  eax, 8
	mov  al, [esi]
	inc  esi
	dec  cx
	jnz  @1
    bt   ebx, 0
	jnc  @2

@2:
    pop  ebx
    pop  esi
	ret

FSSInt2Int endp


; normal integer to variable size unsigned ebml format big endian int
Int2VSUInt_asm proc dwX, dwY, dwL  ;  (__int64* x, char* y, int iLen)
	push ebx
    mov  ebx, dwX
	push esi
	push edi
	
    mov  edi, dwY
; EDX:EAX
	mov  eax, [ebx]
	mov  edx, [ebx+4]
	add  eax, 1
	adc  edx, 0

	xor  ecx, ecx

	mov  ecx, dwL
	or   ecx, ecx
	jnz  @3

	cmp  eax, 0
	jne  @1
	cmp  edx, 0
	jne  @1
    mov  edi, dwY
	mov  al, 80h
	mov  [edi], al
	mov  eax, 1
	jmp  @5
@1: ; !!*x
	mov  cl, 0
@2: ; shift value by 7 bits and check for zero
	inc  cl
	clc
	shrd eax, edx, 7
	shr  edx, 7
    cmp  eax, 0
	jne  @2
	cmp  edx, 0
	jne  @2
	
@3: ; cl = length in ebml
    add  ebx, ecx
	dec  ebx
    mov  esi, ecx
	or   edi, edi
	jnz  @6
	xor  eax, eax
	mov  al, cl
	jmp  @5
@6:
	mov  edi, dwY
@4:
    mov  ch, [ebx]
	mov  [edi], ch
	inc  edi
	dec  ebx
	dec  cl
	jnz  @4

	mov  ecx, 8
	sub  ecx, esi
    
	mov  edx, 0
	bts  edx, ecx

    mov  ebx, dwY
	or   byte ptr [ebx], dl
	mov  eax, esi
@5:
	pop  edi
	pop  esi
	xor  edx, edx
    pop  ebx
	ret	
Int2VSUInt_asm endp

zerobits64 proc dwX, dwB
    mov  eax, dwX
	mov  edx, 0
	mov  ecx, dwB
	bts  edx, ecx
	dec  edx
	xor  edx, 0FFFFFFFFh
	and  [eax], edx
	ret
zerobits64 endp

shift_vals DD 3Fh, 1FFFh, 0FFFFFh, 0EFFFFFFh;

swap_byteorder proc dwX: QWORD, dwLen
    push ebx
    mov  ebx, dwLen
	jmp  [swap_table + 4*ebx]

@8:
    mov  eax, dword ptr dwX
	bswap eax
	mov  edx, eax
	mov  eax, dword ptr dwX+4
	bswap eax
	ret
@4:
    mov  eax, dword ptr dwX
    bswap eax
	pop  ebx
	ret

@2:
	mov  ax, word ptr dwX
	xchg ah, al
	pop  ebx
	ret

@1:
    pop  ebx
    ret
	swap_table DD @1, @1, @2, @1, @4, @1, @1, @1, @8
swap_byteorder endp




_CODE ENDS

END