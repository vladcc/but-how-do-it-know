# naive multiplication test
# multiplies reg a times reg b, result is left in reg a
#%define a		r0	# rag a; holds result at the end
#%define a		rr 	# error
#%define b		r1	# reg b
#%define one		r2	# holds one
#%define save	r3	# save a
#%define mplcand	2
#%define mplier	5

	data r2, 1
	data r0, 2
	data r1, 5
	cmp r2, r1	# if a * 0
	ja .mulz
	cmp r1, r2	# if a * 1
	je .done
	clf
	# mov a, save
	xor r3, r3 
	or r0, r3
.mult:
	add r3, r0
	clf
	# two's compliment
	add r2, r1
	not r2, r2
	add r2, r1
	not r2, r2
	clf
	cmp r2, r1
	je .done
	jmp .mult
	
.mulz:
	xor r0, r0
.done:
	jmp .done
