/* disasm.h -- the header for the disassembler engine */
/* ver. 1.0 */
#ifndef DISASM_H
#define DISASM_H
// the byte type
typedef unsigned char byte;

char ** disasm_dis(byte * code, int size);
/*
 * returns: A pointer to the string representation of code in jcpu assembly.
 * 
 * description: Reads a binary array code of size size and returns it's 
 * equivalent in jcpu assembly. */
#endif
