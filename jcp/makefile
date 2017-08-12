# Compile for Windows(default)/Linux
WIN=Windows
LIN=Linux
OS=$(WIN)

# Compiler
CC=gcc
CFLAGS=-s -Wall

# Common
JCPU=jcpu
MCODE=mach_code

ifeq ($(OS),$(WIN))
EXEC=.exe
else
EXEC=.bin
endif

OBJ=o
CMDIR=.
# RM=del for cmd
RM=rm

# All
all: vm preproc asm dis lang

# The virtual machine
VMDIR=$(CMDIR)/jcpvm
VM=$(VMDIR)/jcpvm
DISPLAY=$(CMDIR)/display
DISASM=$(CMDIR)/disasm

VMOBJ=$(VM).$(OBJ) $(DISPLAY).$(OBJ) $(JCPU).$(OBJ) $(DISASM).$(OBJ) $(MCODE).$(OBJ)

vm: $(VMOBJ)
	$(CC) $(VMOBJ) -o jcpvm$(EXEC) $(CFLAGS)
	
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

# Abstract data types
AADTDIR=$(CMDIR)/adt
LIST=$(AADTDIR)/list
HTBL=$(AADTDIR)/chtbl

$(HTBL).$(OBJ): $(HTBL).c $(HTBL).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
$(LIST).$(OBJ): $(LIST).c $(LIST).h
	$(CC) $< -c -o $@ $(CFLAGS)

# The preprocessor
PPDIR=$(CMDIR)/preproc
PP=$(PPDIR)/preproc
PPO=$(PP).$(OBJ) $(HTBL).$(OBJ) $(LIST).$(OBJ)

# Remove the dot from here and add it to EXEC
preproc: $(PPO)
	$(CC) $(PPO) -o $@$(EXEC) $(CFLAGS)

$(PP).$(OBJ): $(PP).c $(PP).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
# The disassembler
DISDIR=$(CMDIR)/jcpdis
DIS=$(DISDIR)/jcpdis
DISO=$(DIS).$(OBJ) $(DISASM).$(OBJ) $(MCODE).$(OBJ)

dis: $(DISO)
	$(CC) $(DISO) -o jcp$@$(EXEC) $(CFLAGS)

$(DIS).$(OBJ): $(DIS).c
	$(CC) $< -c -o $@ $(CFLAGS)
	
# The assembler
ASMDIR=$(CMDIR)/jcpasm
ASM=$(ASMDIR)/jcpasm
LEXR=$(ASMDIR)/lexjcpa
ASMO=$(ASM).$(OBJ) $(LEXR).$(OBJ) $(HTBL).$(OBJ) $(LIST).$(OBJ) $(MCODE).$(OBJ)

asm: $(ASMO)
	$(CC) $(ASMO) -o jcp$@$(EXEC) $(CFLAGS)

$(ASM).$(OBJ): $(ASM).c $(ASM).h $(MCODE).h $(LEXR).h
	$(CC) $< -c -o $@ $(CFLAGS)

$(LEXR).$(OBJ): $(LEXR).c $(LEXR).h $(MCODE).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
# The compiler
LANGDIR=$(CMDIR)/lang
LANG=$(LANGDIR)/lang
LANGLEX=$(LANGDIR)/lexlang
LANGO=$(LANG).$(OBJ) $(LANGLEX).$(OBJ) $(MCODE).$(OBJ)

lang: $(LANGO)
	$(CC) $(LANGO) -o $@$(EXEC) $(CFLAGS)
	
$(LANG).$(OBJ): $(LANG).c $(MCODE).h $(LANGLEX).h
	$(CC) $< -c -o $@ $(CFLAGS)

$(LANGLEX).$(OBJ): $(LANGLEX).c $(LANGLEX).h $(MCODE).h
	$(CC) $< -c -o $@ $(CFLAGS)
	
clean:	
	$(RM) $(CMDIR)/*.$(OBJ)
	$(RM) $(VMDIR)/*.$(OBJ)
	$(RM) $(PPDIR)/*.$(OBJ)
	$(RM) $(ASMDIR)/*.$(OBJ)
	$(RM) $(LANGDIR)/*.$(OBJ)
	$(RM) $(DISDIR)/*.$(OBJ)
	$(RM) $(AADTDIR)/*.$(OBJ)
	$(RM) $(CMDIR)/*$(EXEC)
