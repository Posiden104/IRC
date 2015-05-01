
CXX = g++ -fPIC

all: IRCServer

IRCServer: IRCServer.cc IRCServerPortFunc.cc 
	g++ -g -o IRCServer IRCServer.cc IRCServerPortFunc.cc
	git add -A  >> .local.git.out
	git commit -a -m "Make IRCServer" >> .local.git.out

clean:
	rm -f *.out
	rm -f *.o IRCServer


