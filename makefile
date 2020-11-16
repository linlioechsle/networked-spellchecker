simpleServer: simpleServer.c open_listenfd.c simpleServer.h
	gcc -Wall -Werror -o simpleServer simpleServer.c open_listenfd.c -lpthread
