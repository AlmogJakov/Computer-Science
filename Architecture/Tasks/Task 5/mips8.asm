# ------------ Hanoi ------------
# getting argument from $a0
Hanoi:
# save address to returned to
addi $sp, $sp, -8
sw $a0, 4($sp)
sw $ra, 0($sp)
# stop codition if a<=1 (equal to a<2) dont jump to else
addi $t0, $0, 2
slt $t0, $a0, $t0
beq $t0, $0, else
# if so (a<=1)
addi $v0, $0, 1
# ignore last args from stack and go back
addi $sp, $sp, 8
jr $ra
# else (a>1)
else:
addi $a0, $a0, -1
# recursive call
jal Hanoi
# return value to prev call
# load address to returned to
lw $ra, 0($sp)
lw $a0, 4($sp)
addi $sp, $sp, 8
# 2*Hanoi(n-1)
sll $v0, $v0, 1
# 2*Hanoi(n-1)+1
addi $v0, $v0, 1
jr $ra
# ------------ done ------------