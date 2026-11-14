# crtend.o for IRIX n32 (PathScale); see crtbegin.s.
#
# Ends the .ctors and .dtors lists, and its .init fragment, which runs
# after crtbegin.o's, calls the static constructors. As with GCC, they
# run from the end of .ctors backwards to crtbegin.o's -1 marker.

	.abicalls
	.option	pic2
	.set	noreorder

	.section .ctors,"aw",@progbits
	.align	2
	.type	__CTOR_END__, @object
__CTOR_END__:
	.word	0

	.section .dtors,"aw",@progbits
	.align	2
	.type	__DTOR_END__, @object
__DTOR_END__:
	.word	0

	.text
	.align	3
	.ent	__pscrt_do_global_ctors
	.type	__pscrt_do_global_ctors, @function
__pscrt_do_global_ctors:
	.frame	$sp, 32, $31
	# gp is callee-saved in n32: save the caller's before setting ours.
	addiu	$sp, $sp, -32
	sd	$31, 0($sp)
	sd	$16, 8($sp)
	sd	$28, 16($sp)
	lui	$28, %hi(%neg(%gp_rel(__pscrt_do_global_ctors)))
	addiu	$28, $28, %lo(%neg(%gp_rel(__pscrt_do_global_ctors)))
	addu	$28, $28, $25
	sd	$28, 24($sp)
	lw	$16, %got_page(__CTOR_END__)($28)
	addiu	$16, $16, %got_ofst(__CTOR_END__)
1:	addiu	$16, $16, -4
	lw	$25, 0($16)
	li	$8, -1
	beq	$25, $8, 2f		# crtbegin.o's marker
	nop
	jalr	$25
	nop
	b	1b
	ld	$28, 24($sp)		# our gp, in case the callee lost it
2:	ld	$31, 0($sp)
	ld	$16, 8($sp)
	ld	$28, 16($sp)
	jr	$31
	addiu	$sp, $sp, 32
	.end	__pscrt_do_global_ctors
	.size	__pscrt_do_global_ctors, .-__pscrt_do_global_ctors

	.section .init,"ax",@progbits
	addiu	$sp, $sp, -16
	sd	$31, 0($sp)
	sd	$28, 8($sp)
	lui	$25, %hi(__pscrt_do_global_ctors)
	addiu	$25, $25, %lo(__pscrt_do_global_ctors)
	jalr	$25
	nop
	ld	$31, 0($sp)
	ld	$28, 8($sp)
	addiu	$sp, $sp, 16
