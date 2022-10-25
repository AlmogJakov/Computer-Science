# $s0 = base address
lui $s0, 0x1001
ori $s0, $s0, 0x0000
# $s1 = i
addi $s1, $0, 1
addi $t0, $0, 6
# num
addi $t1, $0, 1
loop: 
# stop codition
slt $t7, $s1, $t0
beq $t7, $0, done
# set num value to the address
sw  $t1, 0($s0)
# increase num
sll $t1, $t1, 3
# increase address
addi $s0, $s0, 12
# increase i
addi $s1, $s1, 1
# sll $t1, $t1, 3
j loop
done: