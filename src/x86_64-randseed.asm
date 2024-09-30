bits 64
global random_seed_x86_64
random_seed_x86_64:
    push rbp
    mov rbp, rsp
    .rdrand:
	mov eax, 1
	xor ecx,ecx
	cpuid
	bt ecx, 30
	jnc .rdseed
	rdrand rax
	jnc .rdrand
	jmp .done
    .rdseed:
    	mov eax, 7
    	xor ecx,ecx
    	cpuid
    	bt ebx, 18
    	jnc .done
    	rdseed rax
    	jnc .rdseed
    .done:
    leave
    ret
