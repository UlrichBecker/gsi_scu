	.file	"asm_test.c"
	.section	.text
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align 4
.LC0:
	.string	"x = %u\n"
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC1:
	.long	.LC0
	.section	.text
	.align 4
	.global	sig
	.type	sig, @function
sig:
	addi     sp, sp, -4
	sw       (sp+4), ra
	orhi     r2, r0, hi(.LC1)
	ori      r2, r2, lo(.LC1)
	lw       r1, (r2+0)
	calli    mprintf
	lw       ra, (sp+4)
	addi     sp, sp, 4
	b        ra
	.size	sig, .-sig
	.section	.rodata.cst4
	.align 4
.LC2:
	.long	counter
	.section	.text
	.align 4
	.global	incCounter
	.type	incCounter, @function
incCounter:
	orhi     r1, r0, hi(.LC2)
	ori      r1, r1, lo(.LC2)
	lw       r2, (r1+0)
	lw       r1, (r2+0)
	addi     r1, r1, 1
	sw       (r2+0), r1
	b        ra
	.size	incCounter, .-incCounter
	.align 4
	.global	decCounter
	.type	decCounter, @function
decCounter:
#APP
# 33 "asm_test.c" 1
	orhi r1, r0, hi(counter) 
	ori  r1, r1, lo(counter) 
	lw       r2, (r1+0)     
	lw       r1, (r2+0)     
	addi     r1, r1, -1     
	sw       (r2+0), r1     
	
# 0 "" 2
#NO_APP
	b        ra
	.size	decCounter, .-decCounter
	.section	.rodata.str1.4
	.align 4
.LC3:
	.string	"Inline assembler test\n"
	.align 4
.LC4:
	.string	"Noch was...\n"
	.align 4
.LC5:
	.string	"counter = %d\n"
	.align 4
.LC6:
	.string	"counter = %u\n"
	.section	.rodata.cst4
	.align 4
.LC7:
	.long	.LC3
	.align 4
.LC8:
	.long	.LC4
	.align 4
.LC9:
	.long	counter
	.align 4
.LC10:
	.long	.LC5
	.align 4
.LC11:
	.long	.LC6
	.section	.text
	.align 4
	.global	main
	.type	main, @function
main:
	addi     sp, sp, -12
	sw       (sp+12), r11
	sw       (sp+8), r12
	sw       (sp+4), ra
	orhi     r2, r0, hi(.LC7)
	ori      r2, r2, lo(.LC7)
	lw       r1, (r2+0)
	calli    mprintf
	orhi     r3, r0, hi(.LC8)
	ori      r3, r3, lo(.LC8)
	lw       r1, (r3+0)
	calli    mprintf
	orhi     r1, r0, hi(.LC9)
	ori      r1, r1, lo(.LC9)
	lw       r11, (r1+0)
	lw       r2, (r11+0)
	orhi     r3, r0, hi(.LC10)
	ori      r3, r3, lo(.LC10)
	lw       r1, (r3+0)
	calli    mprintf
	calli    incCounter
	lw       r2, (r11+0)
	orhi     r1, r0, hi(.LC11)
	ori      r1, r1, lo(.LC11)
	lw       r12, (r1+0)
	or       r1, r12, r0
	calli    mprintf
	lw       r2, (r11+0)
	or       r1, r12, r0
	calli    mprintf
.L6:
	bi       .L6
	.size	main, .-main
	.section	.bss
	.type	counter, @object
	.size	counter, 4
	.align 4
counter:
	.zero	4
	.ident	"GCC: (GSI) 9.2.0"
