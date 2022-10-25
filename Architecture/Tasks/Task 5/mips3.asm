# the maximal digit in ID - 1 = 9 - 1 = 8
addi $s6, $0, 8
# cases constant
addi $t1, $0, 1
addi $t2, $0, 2
addi $t3, $0, 3
# constant
addi $t5, $0, 5
# switch case
beq $s6, $t1, case1
beq $s6, $t2, case2
beq $s6, $t3, case3
j case4
# case 1
case1:
addi $s6, $s6, 0x20
j done
# case 2
case2:
sll $s6, $s6, 3
j done
# case 3
case3:
# another option is to use mul $s6, $s6, $t5
mult $s6, $t5
mflo $s6
addi $s6, $s6, 20
j done
# case 4 ($t3 = 3)
case4:
div $s6, $s6, $t3
j done
done:
