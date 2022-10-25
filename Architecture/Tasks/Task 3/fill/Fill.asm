// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.
@SCREEN
D=A

@addr
M=D

@KBD
D=A

@endaddr
M=D-1

(LOOP_L_B)
	// init i
	@addr
	D=M
	@i
	M=D
	// check keyboard press
	@KBD
	D=M
	@LOOP_B
	D;JNE
	// continute loop
	@LOOP_L_B
	0;JMP

(LOOP_B)
	// stop condition
	@i
	D=M
	@endaddr
	D=D-M
	@LOOP_L_W
	D;JGT
	// print black
	@i
	A=M
	M=-1
	@i
	M=M+1
	// continute loop
	@LOOP_B
	0;JMP
	

(LOOP_L_W)
	// init i
	@addr
	D=M
	@i
	M=D
	// check keyboard press
	@KBD
	D=M
	@LOOP_W
	D;JEQ
	// continute loop
	@LOOP_L_W
	0;JMP

(LOOP_W)
	// stop condition
	@i
	D=M
	@endaddr
	D=D-M
	@LOOP_L_B
	D;JGT
	// print white
	@i
	A=M
	M=0
	@i
	M=M+1
	// continute loop
	@LOOP_W
	0;JMP