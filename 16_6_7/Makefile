
.PHONY:all
all:pollServer pollClient
pollServer:pollServer.c
	gcc -o $@ $^ -g
pollClient:pollClient.c
	gcc -o $@ $^ -g

.PHONY:clean
clean:
	rm -rf pollServer
	rm -rf pollClient
