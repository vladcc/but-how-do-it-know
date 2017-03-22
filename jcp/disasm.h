/* disasm.h -- the header for the disassembler engine */
/* ver. 1.01 */
#ifndef DISASM_H
#define DISASM_H
// the byte type
typedef unsigned char byte;

enum {NO_PREF, PREF_HEX};
char ** disasm_dis(byte * code, int size, int prefhex);
/*
 * returns: A pointer to the string representation of code in jcpu assembly language.
 * 
 * description: Reads a binary array code of size size and returns it's 
 * equivalent in jcpu assembly. All literas are disassembled in hex. 
 * prefhex specifies whether they should be prefixed with 0x or not. 
 * It should be set to NO_PREF or PREF_HEX. */
#endif
