OBJ = spfs_devices.o spfs_bitmap.o spfs.o spfs_shell.o main.o

spfs.exe: $(OBJ) 
	gcc $? -o $@

%.o: %.c
	gcc -std=c99 -c $? -o $@

clean:
	ls | grep -v -e '.*\.[ch]' -e 'Makefile' -e '.*\.md'  | xargs rm -f
