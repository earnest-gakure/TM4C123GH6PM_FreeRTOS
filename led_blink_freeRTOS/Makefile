##
#  @file     Makefile
#  @brief    General makefile for TM4C123
#  @version  V1.0
#  @date     31/10/2016
#
#  @note     CMSIS library used
#
#  @note  make options
#
#  @param build        generate image file in $(OBJDIR)
#  @param flash        write to flash using $(FLASHER)
#  @param clean        delete all generated files
#  @param disassembly  disassemble with output file into $(OBJDIR)/$(PROGNAME).S
#  @param dump         generate a hexadecimal dump file into $(OBJDIR)/$(PROGNAME).dump
#  @param nm           list symbol table in standard output
#  @param size         list size of output file
#  @param term         open a terminal for serial communication to board
#  @param openocd      start an openocd proxy
#  @param gdbserver    start debug daemon (Start before debug session)
#  @param debug        enter a debug session (one of below)
#  @param  gdb         enter a debug session using gdb
#  @param  ddd         enter a debug session using ddd (GUI)
#  @param  nemiver     enter a debug session using nemiver (GUI)
#  @param  tui         enter a debug session using gdb in text UI
#  @param doxygen      generate doc files (alias=docs)
#  @param help         print options (default)

###############################################################################
# Main parameters                                                             #
###############################################################################

#
# Program name
#
PROGNAME=main

#
# Defines the part type that this project uses.
#
PART=TM4C123GH6PM
# Used to define part in header file
PARTCLASS=TM4C123GH6PM
# Used to find correct CMSIS include file
PARTCLASSCMSIS=TM4C123

#
# Source files
#
SRCFILES=libs/startup_tm4c123.c libs/system_tm4c123.c src/main.c

#
# Include the common make definitions.
#
PREFIX:=arm-none-eabi

#
# Main target
#
default: build

#
# Verbose output for Make (Comment to have verbose output)
#
#MAKEFLAGS+= --silent --no-print-directory
.SILENT:

#
# Folder for object files
#
OBJDIR=gcc

#
# Set the compiler CPU/FPU options.
#
CPUFLAGS=-mcpu=cortex-m4

#
# FPU Options
#
FPUFLAGS=-mfpu=fpv4-sp-d16 -mfloat-abi=hard -DENABLE_FPU

#
# Specs script (modification of compiler and linker flags)
#
SPECFLAGS=--specs=nano.specs

#
# Serial terminal communication
#
TTYTERM=/dev/ttyACM0
TTYBAUD=9600

#
# Serial terminal emulator
#
TTYPROG=minicom
TTYPARMS=-D $(TTYTERM) -b $(TTYBAUD)

#
# The command to flash the device
#
FLASHER=lm4flash
#FLASHER=openocd
#
# gdbserver command
#
GDBSERVER=openocd
OPENOCDDIR=/opt/openocd

#
# Terminal application (used to open new windows in debug)
#
TERMAPP=gnome-terminal

#
# GUI Editor
#
GUIEDITOR=gedit

###############################################################################
# Generally it is not needed to modify the lines below                       #
###############################################################################

###############################################################################
#   Commands                                                                  #
###############################################################################

#
# The command for calling the compiler.
#
CC=${PREFIX}-gcc

#
# The command for calling the library archiver.
#
AR=${PREFIX}-ar

#
# The command for extracting images from the linked executables.
#
OBJCOPY=${PREFIX}-objcopy

#
# The command for disassembly
#
OBJDUMP=${PREFIX}-objdump

#
# The command for listing size of code
#
OBJSIZE=${PREFIX}-size

#
# The command for listing symbol table
#
OBJNM=${PREFIX}-nm

#
# Debugger
#
GDB=$(PREFIX)-gdb

#
# GDB servers
#
OPENOCD=openocd
LMICDI=lmicdi

###############################################################################
#   Command parameters                                                        #
###############################################################################

#
# The flags passed to the debugger
#
GDBFLAGS=-x $(GDBINIT) -n

#
# GDB initialization file
#
GDBINIT=$(OBJDIR)/gdbinit

#
# Flags for disassembler
#
ODDISFLAGS= -x -S -D
ODDUMPFLAGS= -x -s

#
# Parameters for OpenOCD
#
OPENOCDBOARD=/usr/share/openocd/scripts/board/ti_ek-tm4c123gxl.cfg
#OPENOCDBOARD=debug/gdb_openocd.cfg
OPENOCDFLASHSCRIPT=$(OBJDIR)/flash.ocd

###############################################################################
# Compilation parameters                                                      #
###############################################################################

#
# Object files
#
OBJFILES=$(addprefix ${OBJDIR}/,$(SRCFILES:.c=.o))

#
# Include path
#
INCLUDEPATH=CMSIS inc

#
# The command for calling the linker
#
LD=${PREFIX}-gcc $(CFLAGS) -nostartfiles -Wl,-Map,$(OBJDIR)/$(PROGNAME).map -Wl,$(LDFLAGS)

#
# The flags passed to the assembler.
#
AFLAGS=-mthumb \
       ${CPUFLAGS}  \
       ${FPUFLAGS}  \
       $(addprefix -I ,${INCLUDEPATH})

#
# The flags passed to the compiler.
#
CFLAGS=-mthumb             \
       ${CPUFLAGS}         \
       ${FPUFLAGS}         \
       $(addprefix -I ,${INCLUDEPATH}) \
       -D${PARTCLASS}      \
       -ffunction-sections \
       -fdata-sections     \
       -std=c11            \
       -pedantic           \
       -DPART_${PART}      \
       $(SPECFLAGS)

#
# The flags passed to the linker.
#
LDFLAGS=--gc-sections

#
# Get the location of libgcc.a from the GCC front-end.
#
LIBGCC:=${shell ${CC} ${CFLAGS} -print-libgcc-file-name}

#
# Get the location of libc.a from the GCC front-end.
#
LIBC:=${shell ${CC} ${CFLAGS} -print-file-name=libc.a}

#
# Get the location of libm.a from the GCC front-end.
#
LIBM:=${shell ${CC} ${CFLAGS} -print-file-name=libm.a}

#
# Generate debug version if DEBUG is set
#
ifneq ($DEBUG,)
CFLAGS+=-g -D DEBUG -O0
else
CFLAGS+=-Os
endif

#
# Additional Flags
#
CFLAGS+= -Wuninitialized -Werror

#
# Controlling dependencies on header files
#
DEPFLAGS=-MT $@  -MMD -MP -MF ${OBJDIR}/$*.d

#
# Linker script
#
LINKERSCRIPT=ld/tm4c123.ld

#
# Entry Point
#
ENTRY=Reset_Handler

###############################################################################
#   Rules                                                                     #
###############################################################################

#
# The rule for building the object file from each C source file.
#
${OBJDIR}/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  Compiling           ${<}";
	${CC} -c ${CFLAGS} -D${OBJDIR} ${DEPFLAGS} -o ${@} ${<}

#
# The rule for building the object file from each assembly source file.
#
${OBJDIR}/%.o: %.S
	@echo "  Assembling          ${<}";
	${CC} -c ${AFLAGS} -D${OBJDIR} -o ${@} -c ${<}

#
# The rule for creating an object library.
#
${OBJDIR}/%.a:
	@echo "  Archiving           ${@}";
	${AR} -cr ${@} ${^}

#
# The rule for linking the application.
#
${OBJDIR}/%.axf:  $(OBJFILES)
	@echo "  Linking             ${@} ";
	${LD}   -T $(LINKERSCRIPT) \
			--entry $(ENTRY)   \
			-o ${@}            \
			$(OBJFILES)        \
			-lm -lc -lgcc

${OBJDIR}/%.bin: ${OBJDIR}/%.axf
	@echo "  Generating binary   ${@} ";
	${OBJCOPY} -O binary ${^} ${@}

###############################################################################
#   Targets                                                                   #
###############################################################################

#
# Help menu: Default target
#
help: usage
usage:
	@echo "Usage:"
	@echo "make build        generate image file in $(OBJDIR)"
	@echo "make flash        write to flash using $(FLASHER)"
	@echo "make clean        delete all generated files"
	@echo "make disassembly  disassemble with output file into $(OBJDIR)/$(PROGNAME).S"
	@echo "make dump         generate a hexadecimal dump file into $(OBJDIR)/$(PROGNAME).dump"
	@echo "make nm           list symbol table in standard output"
	@echo "make size         list size of output file"
	@echo "make term         open a terminal for serial communication to board"
	@echo "make gdbserver    start debug daemon (Start before debug session)"
	@echo "make debug        enter a debug session (one of below)"
	@echo "make  gdb         enter a debug session using gdb"
	@echo "make  ddd         enter a debug session using ddd (GUI)"
	@echo "make  nemiver     enter a debug session using nemiver (GUI)"
	@echo "make  tui         enter a debug session using gdb in text UI"
	@echo "doxygen           generate doc files (alias=docs)"
	@echo "help              print options (default)"

#
# The default rule, which causes the $(PROGNAME) example to be built.
#
build: ${OBJDIR} ${OBJDIR}/$(PROGNAME).bin
	@echo "Done"

#
# Clean out all the generated files
#
clean:
	rm -rf ${OBJDIR} ${wildcard *~} html latex docs  && echo "Done."

#
# Transfer binary to board
#
flash: deploy
burn: deploy
ifeq ($(FLASHER),lm4flash)
deploy: ${OBJDIR}/$(PROGNAME).bin
	sudo ${FLASHER} $^
else
deploy: ${OBJDIR}/$(PROGNAME).bin $(OPENOCDFLASHSCRIPT)
	${OPENOCD} -f $(OPENOCDBOARD) -f $(OPENOCDFLASHSCRIPT)
endif

#
# Disassembling/Dump
#
dis: disassembly
disassembly: $(OBJDIR)/$(PROGNAME).axf
	@echo "  Disassembling ${^} and storing output in $(OBJDIR)/$(PROGNAME).S"
	$(OBJDUMP) $(ODDISFLAGS) $^ > $(OBJDIR)/$(PROGNAME).S

dump: $(OBJDIR)/$(PROGNAME).axf
	@echo "  Dumping ${^} and storing output in $(OBJDIR)/$(PROGNAME).dump"
	$(OBJDUMP) $(ODDUMPFLAGS) $^ > $(OBJDIR)/$(PROGNAME).dump
imagedump: $(OBJDIR)/$(PROGNAME).bin
	od -x $^ > $(OBJDIR)/$(PROGNAME).hexdump

#
# List size
#
size: $(OBJDIR)/$(PROGNAME).axf
	$(OBJSIZE) -A -x $^

#
# List size
#
nm: $(OBJDIR)/$(PROGNAME).axf
	$(OBJNM) $^

#
# The rule to create the target directory.
#
${OBJDIR}:
	mkdir -p ${OBJDIR}

#
# Rules for building the $(PROGNAME) example.
#
${OBJDIR}/$(PROGNAME).bin: ${OBJDIR}/$(PROGNAME).axf
${OBJDIR}/$(PROGNAME).axf: ${OBJFILES}

#
# Open files in editor windows
#
edit:
	$(GUIEDITOR) Makefile *.c *.h *.ld &

#
# Debug command
#
debug: gdb
gdb: $(OBJDIR)/$(PROGNAME).axf $(GDBINIT) openocd
	$(GDB) $(GDBFLAGS) $(OBJDIR)/$(PROGNAME).axf

#
# iDebug command with text UI
#
tui: $(OBJDIR)/$(PROGNAME).axf $(GDBINIT)
	$(GDB) -tui $(GDBFLAGS) $(OBJDIR)/$(PROGNAME).axf

#
# iDebug command with text UI
#
cgdb: $(OBJDIR)/$(PROGNAME).axf $(GDBINIT)
	cgdb -d `which $(GDB)`  -x $(OBJDIR)/gdbinit $(OBJDIR)/$(PROGNAME).axf

#
# Debug using GUI
#
ddd: $(OBJDIR)/$(PROGNAME).axf $(GDBINIT)
	ddd --debugger "$(GDB) $(GDBFLAGS)" $(OBJDIR)/$(PROGNAME).axf

#
# Debug using nemiver GUI
#
nemiver: $(OBJDIR)/$(PROGNAME).axf $(GDBINIT)
	nemiver  --remote=localhost:4242  --gdb-binary=`which $(GDB)`     $(OBJDIR)/$(PROGNAME).axf

#
# Start debug daemon (gdb proxy)
#
gdbserver: $(GDBSERVER)
openocd:
	if [ "`pidof openocd`" != "" ]; then kill `pidof openocd`; fi
	if [ -c $(TTYTERM) ] ; \
	then \
		echo "Starting OpenOCD"; \
		$(TERMAPP) -e "sh -c \" $(OPENOCD) -f $(OPENOCDBOARD) \" " ; \
	else \
		echo "Could not open $(OPENOCD)"; fi

lmicdi:
	if [ "`pidof lmicdi`" != "" ]; then kill `pidof lmicdi`; fi
	if [ -c $(TTYTERM) ] ; \
	then \
		echo "Starting lmicdi"; \
		$(TERMAPP) -e "sh -c \" $(LMICDI)  \" " ; \
	else \
		echo "Could not open $(OPENOCD)"; fi

#
# Create gdbinit (debugger initialization scripts)
#
$(GDBINIT):
	echo "# Run this script using gdb source command" > $(GDBINIT)
	echo "target remote localhost:3333" >> $(GDBINIT)
	echo "break main" >> $(GDBINIT)
	echo "continue" >> $(GDBINIT)

$(OPENOCDFLASHSCRIPT):
	echo "reset halt" > $(OPENOCDFLASHSCRIPT)
	echo "flash probe 0" >> $(OPENOCDFLASHSCRIPT)
	echo "flash write_image erase ${OBJDIR}/$(PROGNAME).bin 0" >> $(OPENOCDFLASHSCRIPT)
	echo "reset run" >> $(OPENOCDFLASHSCRIPT)

$(OPENOCDDEBUGSCRIPT):
	echo "monitor reset halt" > $(OPENOCDFLASHSCRIPT)
	echo "monitor flash probe 0" >> $(OPENOCDFLASHSCRIPT)
	echo "monitor flash write_image erase ${OBJDIR}/$(PROGNAME).bin 0" >> $(OPENOCDFLASHSCRIPT)
	echo "monitor reset run" >> $(OPENOCDFLASHSCRIPT)

#
# Open a terminal windows
#
terminal: term
term:
	if [ -c $(TTYTERM) ] ; then echo "Starting terminal"; \
	$(TERMAPP) -e "sh -c \" $(TTYPROG) $(TTYPARMS) \" " ; \
	else echo "Could not open $(TTYTERM)"; fi

#
# Generate documentation using doxygen
#
docs: doxygen
doxygen:
	$(DOXYGEN) *.c *.h

#
# These labels are not files !!!
#
.PHONY: default all clean flash burn deploy disassembly dump size nm edit debug terminal
.PHONY: term doxygen docs

#
# Dependencies
#
-include $(OBJFILES:%.o=%.d)
