OBJ = spdev/dev.o  spfs/spfs.o \
		spsh/spsh.o spsh/spio.o \
		spsh/cat.o spsh/cd.o spsh/ls.o spsh/mkdir.o \
		spsh/mv.o spsh/pwd.o spsh/ist.o spsh/dsk.o main.o 

run: $(OBJ)
ifeq ($(shell uname), Linux)
	gcc -lm $? -o spfs
else
	gcc $? -o spfs.exe
endif

%.o: %.c
	gcc -std=c99 -c $? -o $@

clean:
	rm -f $(OBJ) spfs.exe