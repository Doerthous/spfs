OBJ = spfs_devices.o spfs_bitmap.o spfs.o spfs_shell.o main.o

run: $(OBJ)
ifeq ($(shell uname), Linux)
	gcc -lm $? -o spfs
else
	gcc $? -o spfs.exe
endif

%.o: %.c
	gcc -std=c99 -c $? -o $@

clean:
	ls | grep -v -e '.*\.[ch]' -e 'Makefile' -e '.*\.md'  | xargs rm -f
