GCC	= gcc
RM	= rm
EXE	= ../bin/exodus-live
BK	= ../bin/exodus-live.bk
CP	= cp
#EXE	= exodus-live

C_FLAGS	= -DSYSV -ggdb -w
L_FLAGS	= -ggdb -lm -w 

#Source Files.
SRC	:= $(wildcard *.c) $(wildcard clans/*.c)

#Object files.
OBJ_DIR	= obj
OBJ	:= $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))

#Make exodus
exodus:	$(OBJ)
#	$(RM) -f $(EXE)
	$(GCC) $(L_FLAGS) -o $(EXE) $(OBJ) -lcrypt

$(OBJ_DIR)/%.o: %.c
	$(GCC) $(C_FLAGS) -c -o $@ $<

clean: 
	$(RM) -f $(OBJ) $(EXE) *~ *.bak *.orig *.rej
	make
