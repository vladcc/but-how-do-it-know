# Compiler
CC=gcc
CFLAGS=-s -Wall

# Common
JCPU=jcpu
MCODE=mach_code
EXEC=exe
OBJ=o
CMDIR=.
# RM=del for cmd
RM=rm

# All
all: vm asm dis

# The virtual machine
VMDIR=$(CMDIR)/jcpvm
VM=$(VMDIR)/jcpvm
DISPLAY=$(CMDIR)/display
DISASM=$(CMDIR)/disasm

VMOBJ=$(VM).$(OBJ) $(DISPLAY).$(OBJ) $(JCPU).$(OBJ) $(DISASM).$(OBJ) $(MCODE).$(OBJ)

vm: $(VMOBJ)
	$(CC) $(VMOBJ) -o jcpvm.$(EXEC) $(CFLAGS)
	
$(VM).$(OBJ): $(VM).c
	$(CC) $< -c -o $@ $(CFLAGS)

$(DISPLAY).$(OBJ): $(DISPLAY).c $(DISPLAY).h
	$(CC) $< -c -o $@ $(CFLAGS)

$(DISASM).$(OBJ): $(DISASM).c $(DISASM).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
$(JCPU).$(OBJ): $(JCPU).c $(JCPU).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
$(MCODE).$(OBJ): $(MCODE).c $(MCODE).h
	$(CC) $< -c -o $@ $(CFLAGS)

# The disassembler
DISDIR=$(CMDIR)/jcpdis
DIS=$(DISDIR)/jcpdis
DISO=$(DIS).$(OBJ) $(DISASM).$(OBJ) $(MCODE).$(OBJ)

dis: $(DISO)
	$(CC) $(DISO) -o jcp$@.$(EXEC)

# The assembler
ASMDIR=$(CMDIR)/jcpasm
LIST=$(ASMDIR)/list
HTBL=$(ASMDIR)/chtbl
ASM=$(ASMDIR)/jcpasm
LEXR=$(ASMDIR)/lexjcpa
ASMO=$(ASM).$(OBJ) $(LEXR).$(OBJ) $(HTBL).$(OBJ) $(LIST).$(OBJ) $(MCODE).$(OBJ)

asm: $(ASMO)
	$(CC) $(ASMO) -o jcp$@.$(EXEC)

$(ASM).$(OBJ): $(ASM).c $(MCODE).h $(LEXR).h
	$(CC) $< -c -o $@ $(CFLAGS)

$(LEXR).$(OBJ): $(LEXR).c $(LEXR).h $(MCODE).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
$(HTBL).$(OBJ): $(HTBL).c $(HTBL).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
$(LIST).$(OBJ): $(LIST).c $(LIST).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
clean:
	$(RM) $(CMDIR)/*.$(EXEC)
	$(RM) $(CMDIR)/*.$(OBJ)
	$(RM) $(VMDIR)/*.$(OBJ)
	$(RM) $(ASMDIR)/*.$(OBJ)
