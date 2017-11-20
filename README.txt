# but-how-do-it-know
Let's bring the Scott CPU to life. And program it.

Content:
I.	Introduction 		- line 11
II. 	For the user 		- line 45
III.	For the programmer	- line 261
IV.	Conclusion		- line 312


I. Introduction

I got into programming the way most people do - one day I got curious,
found a tutorial, learned about variables, functions, if-s, and loops.
It was easy to get information on how to make a working program. But
there seemed to be next to no information on how and why exactly
did it work. How did the computer work?

If you are anything like me, you have wondered that yourself. A lot.

So I got into books about hardware. Went through one, then another, 
then another... Found out about control units, and ALUs, and 
registers, and fetch-execute cycles. Learned what all the parts are and what 
they do. But still no explanation of how and why they do it.

At around this time I came across a book called "But how Do it Know?: The Basic Principles 
of Computers for Everyone" by a guy named J Clark Scott. It promised that by the 
end of it you would understand what a computer is and how it functions. People on the 
Internet seemed to agree that it delivered on this promise, so I was hooked.

And guess what? It did. Big time. I'm not here to sell you anything, but if you want
to know how computers work - read this book.

Inside, the author shows you what a NAND gate is, then takes you on the journey of
creating a complete 8 bit CPU with 256 bytes of RAM along with it's very own 
assembly language, step by step. Every part is small enough to understand, but big 
enough to work. The language of the book is simple. The only prerequisite you need 
is English.

After I read the book I decided that it'd be really fun to emulate the CPU, get
a way to program it and see it work. That's how this project came to be.



II. For the user

The project consists of the five programs listed below. In the
/jcp/bin/ directory you can find all of them compiled for
Windows, x86 Linux(compiled and tested on Ubuntu), and Raspbian for the
Raspberry Pi. If you'd like to compile the code yourself see part III.


1. jcpvm - the virtual machine. Runs programs for the computer from the book.
Note: before running increase the height of your cmd/terminal window.

When you run it you'd see a 16x16 grid which is the memory. On the right of that there are
the memory address register(MAR), the instruction address register(IAR), the instruction
register(IR), the carry(C), "a is greater"(A), equal(E), and zero(Z) flags, and the
four general purpose registers R0, R1, R2, and R3. '*' marks where MAR is pointing to,
'@' marks where IAR is pointing to. If they point to the same place only '@' is shown.

Below the memory you'd see the last executed instruction(the one at the top), the instruction
which is going to be executed next(the one the arrow points to), and a number of instructions
that follow in the memory.

Below that is the command line for the machine. Pressing enter executes the next instruction.
All control options are:
---------------------------------------------------------------
Start the vm: jcpvm <file name>
<file name> should be the name of a file compiled for the jcpu
Version:      jcpvm -v
Help:         jcpvm -h

Interactive options:
Execute a single instruction - enter
Jump n instructions ahead    - j <n> + enter
Note: jumping executes n instructions, it does not skip over
n instructions from the code
Reset the cpu                - r + enter
Print screen in decimal      - d + enter
Print help in vm             - h + enter
Quit                         - q + enter
---------------------------------------------------------------


2. jcpasm - the assembler. Compiles the assembly instructions to binary. Runs the preprocessor
first, if available.
Note: jcpasm, preproc, and lang should be in the same directory. jcpasm and lang are not case 
sensitive. All source code is transformed to uppercase and displayed as such in the error messages.

Instruction set:

Memory:
LD RA, RB - loads RB from RAM address in RA
ST RA, RB - stores RB to RAM address in RA 
DATA RB - loads the next byte as data in RB

Jumps:
JMPR RB - unconditional jump register; jumps to address in RB
JMP addr - unconditional jump; jumps to the address in the next byte
J<flag(s)> addr - conditional jumps; jumps to the address in the next byte when
any of the requested flag bits is set(JC, JA, JE, JZ, JAE, and other combinations 
there of)

ALU:
CLF - clears the flags
ADD RA, RB - adds the value in RA to the value in RB in RB; modifies: C, Z
SHR RA, RB - shifts RA one to the right into RB; modifies: C, Z
SHL RA, RB - shifts RA one to the left into RB; modifies: C, Z
NOT RA, RB - sets RB to the reverse bits value of RA; modifies: Z
AND RA, RB - bitwise ands RA and RB in RB; modifies: Z
OR RA, RB -  bitwise ors RA and RB in RB; modifies: Z
XOR RA, RB - bitwise xors RA and RB in RB; modifies: Z
CMP RA, RB - compares RA and RB; modifies: A, E

Labels must begin with a '.'
Comments start with a '#' There is no multi-line comment support.

Usage:
---------------------------------------------------------------
Compile: jcpasm <input text file> -o <output binary file>
Version: jcpasm -v
Help:    jcpasm -h
---------------------------------------------------------------
Example code is in: /jcp/bin/example_code/asm/


3. jcpdis - the disassembler. Decompiles binary files to assembly instructions.
Usage:
---------------------------------------------------------------
Disassemble: jcpdis <input binary file> -o <output text file>
<input binary file> should be the name of a file compiled for the jcpu
Version:     jcpdis -v
Help:        jcpdis -h
---------------------------------------------------------------


4. lang - the compiler. Compiles higher level constructs into assembly.
Runs the preprocessor first if available, compiles, and calls the assembler.

Lang provides assignment, if-s, and a loop structure. Anything that is not
a construct which lang recognizes is assumed to be assembly and is therefore
left to the assembler.

Assignments are in the form of:
<register> = <immediate value>
<register> = <register>

The if structure is:
if <register> <comparison operator> <register>
 <code>
endif

The if allows for only one comparison between two registers. 
The comparison operators are:
RA == RB - checks if the value in RA is equal to the value in RB
RA < RB  - checks if the value in RA is less than the value in RB
RA <= RB - checks if the value in RA is less or equal to the value in RB
RA > RB  - checks if the value in RA is greater than the value in RB
RA >= RB - checks if the value in RA is greater or equal to the value in RB
RA != RB - checks if the values in RA and RB are different
The flags cannot be explicitly checked with if-s.

The loops structure is:
loop
	<code>
endloop

By default it loops forever. Use if with a break statement to break out of it. Also,
continue can be used to immediately start the next loop iteration. Example:

loop
	if <condition>
		break
	endif
	
	if <other condition>
		continue
	endif
	
	<code>
endloop

You can still use labels and jumps in the source.
Lang always leaves behind a file ending in .jasm. This is the case even if you
feed it purse assembly. This file is the input for the assembler.

Usage:
---------------------------------------------------------------
Compile: lang <input text file> -o <output binary file>
Version: lang -v
Help:    lang -h
---------------------------------------------------------------
Example code is in: /jcp/bin/example_code/lang/


5. preproc - the preprocessor. Performs textual substitution based on a "%define cat dog"
directive. Given this directive preproc will run through your file and substitute every occurrence
of the word "cat" with the word "dog". All the %define directives should be placed at the start 
of the line and at the beginning of the file. You can use the preprocessor by itself, but its main 
purpose is to be called by lang and jcpasm.

Preproc always leaves a file ending in .pp This file has the same layout as the input file, with
the difference that substitutions have been made and all %define directives are commented out.
This is so because this file gets fed back in the program which called preproc(either lang or
jcpasm). Therefore any error position in it will correspond to any error position in your 
original source. Also, helped debugging a lot.

Usage:
---------------------------------------------------------------
Use:  preproc.exe <input file>
Help: preproc.exe -h

Description
preproc.exe processes a text file according to a set of directives.
If preproc.exe finds a directive inside the input file, it
processes the file, leaving the result in another file named
<input file>.pp, returning 0 to the OS.
If preproc.exe does not find any directives, it returns non-zero and exits.
If an error occurs, preproc.exe prints the error message and quits with
EXIT_FAILURE.
Regardless of outcome, the input file is never modified.
All directives begin with a '%' immediately followed by the
directive name. The directives must be gathered at the beginning of
the file, placed at the start of the line, one directive per line.
They can only be preceded by comment lines beginning with a '#'
The maximum supported line length is 255 characters.

Supported directives
1. %define
Syntax: %define <a> <b>
Substitutes <a> with <b>
<a> and <b> are defined as any sequence of printable characters
delimited by a space or a comma.
---------------------------------------------------------------

Compilation and running example:

First, get your source file to the directory of the executables. We'll use 
stack_lang.txt Then open your cmd/terminal window and go to that directory. 
After that compile with:

lang stack_lang.txt -o stack_bin

And run it with:

jcpvm stack_bin
Note: if you are on Linux you'd need to type "./lang" and "./jcpvm" instead 
of only "lang" and "jcpvm". Also, if the screen gets all weird and mangled up
when you press enter, increase the height of your cmd/terminal window and run 
jcpvm again.

If you want to compile assembly code only you can do that with:

jcpasm <input file> -o output_binary

Again, "./jcpasm" for Linux.



III. For the programmer

If you'd like to change and play with the code here's what you need to know:

First - please feel welcome to do so.
Second - everything is done in C.

The directory tree goes like this:

/jcp/ - important files:

disasm.c - the disassembler engine. It is used by jcpdis for disassembling 
and by the jcpvm to show you the readable instructions on the screen.

display.c - interface functions for jcpvm.

jcpu.c - the CPU emulator. Used by jcpvm.

mach_code.c - the table of the machine code, the registers, and their mnemonics.

os_def.h - let's you specify if you'd like to compile for Windows or Linux.
Used by display.c, jcpasm, jcpvm, and preproc. This must be changed according to
your target platform.

makefile - the make script. Before you compile make sure you change the OS variable
at the start to WIN or LIN accordingly. "make" or "make all" compiles the whole project. 
You can compile the virtual machine, the preprocessor, the disassembler, the assembler, 
and lang with "make vm", "make preproc", "make dis", "make asm", and "make lang" respectively.
"make clean" removes all binary/object files. It does not touch anything inside /jcp/bin/

All other files in /jcp/ are pretty self-explanatory.

/jcp/adt/ - the abstract data types directory. Has a chained hash table and a linked list
implementations used by the assembler and the preprocessor. The adt-s are based on the 
examples in Kyle Loudon's "Mastering Algorithms with C". Great read, btw.

/jcp/bin/ - contains the compiled binaries for Windows, Linux, and the RPi, along with code
examples and some old tests for the assembler.

/jcp/jcpasm/ - the assembler lives here. You'll find the source for jcpasm and its lexer.

/jcp/jcpdis/ - the disassembler's place. Only its one, lonely file.

/jcp/jcpvm/ - contains the source for the virtual machine.

/jcp/lang/ - home of the lang compiler and its lexer.

/jcp/preproc/ - home of the preprocessor.



IV. Conclusion

This information should be enough to get you started. If you have any suggestions, questions,
or bug reports, please feel welcome to drop me a line.

Also, I am very happy to announce that this project is now on the book's official website:

http://buthowdoitknow.com/

Particularly, you can find it here:

http://buthowdoitknow.com/virtual_machine.html

Also, you don't want to miss the other CPU representations here:

http://buthowdoitknow.com/get_excel.html

and here:

http://buthowdoitknow.com/cpu_model_intro.html
