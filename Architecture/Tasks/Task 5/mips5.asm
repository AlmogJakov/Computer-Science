main:
# set the arguments
addi $a0, $0, 0x0A
addi $a1, $0, 0x10
addi $a2, $0, 0x12
jal func1
# save the returned value to $s2
addi $s2, $v0, 0
j done
# ------------ func1 ------------
func1:
# save first address to returned to
addi $sp, $sp, -4
sw $ra, 0($sp)
# $t0 = sum variable
add $t0, $a0, $a1
sub $t0, $t0, $a2
# copy sum to last param ($a3)
addi $a3, $t0, 0
jal func2
# add the returned value to sum
add $t0, $t0, $v0
# copy sum to returned value
addi $v0, $t0, 0
# get first address to returned to
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra
# ------------func2 ------------
func2:
add $t1, $a0, $a1
add $t1, $t1, $a2
# another option is to use mul $a0, $a0, $t1
mult $a3, $t1
mflo $a3
addi $v0, $a3, 1
jr $ra
# ------------ done ------------
done:
