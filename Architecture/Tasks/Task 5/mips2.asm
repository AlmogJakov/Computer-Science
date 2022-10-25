# $s0 = i
addi $s0, $0, 1
addi $t0, $0, 8
# result
addi $t2, $0, 0
# constant
addi $t1, $0, -1
# $t1 = A, $t2 = B, $t3 = C
addi $t3, $0, 1
addi $t4, $0, 1
addi $t5, $0, 1
loop: 
# stop codition
slt $t7, $s0, $t0
beq $t7, $0, done
# sum
# another option is to use mul $t6, $t3, $t4
mult $t3, $t4
mflo $t6
# another option is to use mul $t6, $t6, $t5
mult $t6, $t5
mflo $t6
add $t2, $t2, $t6
# increase i
addi $s0, $s0, 1
# set A
addi $t3, $t3, 2
# set B
# another option is to use mul $t4, $s0, $s0
mult $s0, $s0
mflo $t4
# set C
# another option is to use mul mul $t5, $t5, $t1
mult $t5, $t1
mflo $t5
j loop
done:
