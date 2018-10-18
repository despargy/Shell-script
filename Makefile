
CC = gcc
CNAME = argiShell.c
EXECNAME = argiShell

argiShell:
	$(CC) -o $(EXECNAME) $(CNAME)

clean:
	rm $(EXECNAME)
