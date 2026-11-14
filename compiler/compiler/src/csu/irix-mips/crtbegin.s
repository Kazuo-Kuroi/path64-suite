# crtbegin.o for IRIX n32 (PathScale).
#
# IRIX has no crtbegin.o: its crt1.o and crtn.o bracket the program, and
# crt1.o's __istart runs the .init fragments in between before main.
# This object, linked right after crt1.o, provides what GCC-style code
# expects from crtbegin.o:
#
#  - the start of the .ctors and .dtors lists (crtend.o ends them);
#  - __dso_handle, used by __cxa_atexit;
#  - an .init fragment that registers the static destructors with
#    atexit; crtend.o's fragment then runs the static constructors.
#
# Having writable data also makes GNU ld start a separate data segment.
# Without it, a program with no initialized data got a single RWE
# segment, which IRIX rld cannot load (it crashes in
# find_symbol_in_object).

	.abicalls
	.option	pic2
	.set	noreorder

	.section .ctors,"aw",@progbits
	.align	2
	.type	__CTOR_LIST__, @object
__CTOR_LIST__:
	.word	-1

	.section .dtors,"aw",@progbits
	.align	2
	.type	__DTOR_LIST__, @object
__DTOR_LIST__:
	.word	-1

	.data
	.align	2
	.globl	__dso_handle
	.hidden	__dso_handle
	.type	__dso_handle, @object
	.size	__dso_handle, 4
__dso_handle:
	.word	0

	.text
	.align	3

# Run the static destructors, in .dtors order.
	.ent	__pscrt_do_global_dtors
	.type	__pscrt_do_global_dtors, @function
__pscrt_do_global_dtors:
	.frame	$sp, 32, $31
	# gp is callee-saved in n32: save the caller's before setting ours.
	addiu	$sp, $sp, -32
	sd	$31, 0($sp)
	sd	$16, 8($sp)
	sd	$28, 16($sp)
	lui	$28, %hi(%neg(%gp_rel(__pscrt_do_global_dtors)))
	addiu	$28, $28, %lo(%neg(%gp_rel(__pscrt_do_global_dtors)))
	addu	$28, $28, $25
	sd	$28, 24($sp)
	lw	$16, %got_page(__DTOR_LIST__)($28)
	addiu	$16, $16, %got_ofst(__DTOR_LIST__)
	addiu	$16, $16, 4		# skip the -1 marker
1:	lw	$25, 0($16)
	beq	$25, $0, 2f
	nop
	jalr	$25
	addiu	$16, $16, 4
	b	1b
	ld	$28, 24($sp)		# our gp, in case the callee lost it
2:	ld	$31, 0($sp)
	ld	$16, 8($sp)
	ld	$28, 16($sp)
	jr	$31
	addiu	$sp, $sp, 32
	.end	__pscrt_do_global_dtors
	.size	__pscrt_do_global_dtors, .-__pscrt_do_global_dtors

# atexit (__pscrt_do_global_dtors), called from the .init fragment.
	.ent	__pscrt_init_begin
	.type	__pscrt_init_begin, @function
__pscrt_init_begin:
	.frame	$sp, 16, $31
	addiu	$sp, $sp, -16
	sd	$31, 0($sp)
	sd	$28, 8($sp)		# the caller's gp
	lui	$28, %hi(%neg(%gp_rel(__pscrt_init_begin)))
	addiu	$28, $28, %lo(%neg(%gp_rel(__pscrt_init_begin)))
	addu	$28, $28, $25
	lw	$4, %got_page(__pscrt_do_global_dtors)($28)
	addiu	$4, $4, %got_ofst(__pscrt_do_global_dtors)
	lw	$25, %call16(atexit)($28)
	jalr	$25
	nop
	ld	$31, 0($sp)
	ld	$28, 8($sp)
	jr	$31
	addiu	$sp, $sp, 16
	.end	__pscrt_init_begin
	.size	__pscrt_init_begin, .-__pscrt_init_begin

# .init fragment. __istart has no frame and gp is not set up here, so
# save ra and gp and call through an absolute address (this object is
# only linked into executables, which are not position independent).
	.section .init,"ax",@progbits
	addiu	$sp, $sp, -16
	sd	$31, 0($sp)
	sd	$28, 8($sp)
	lui	$25, %hi(__pscrt_init_begin)
	addiu	$25, $25, %lo(__pscrt_init_begin)
	jalr	$25
	nop
	ld	$31, 0($sp)
	ld	$28, 8($sp)
	addiu	$sp, $sp, 16
