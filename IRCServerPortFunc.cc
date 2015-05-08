
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <list> // for std::list<Message>
#include <vector> // for std::vector<char*>
#include <algorithm> // for std::sort
#include "IRCServer.h"

int QueueLength = 5;
using namespace std;

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);
	
	running = true;

	initialize();

	while ( running ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	for(int i = 0; i < MaxCommandLine +1; i++) {
		commandLine[i] = '\0';
	}
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);
	
	char *p = &commandLine[0];
	char *commandLinePtr = &commandLine[0];
	int lenLeft = commandLineLength;

	while(*p != ' '){
		if(lenLeft <= 0) break;
		p++;
		lenLeft --;
	}
	*p = '\0';
	const char * command = commandLinePtr;
	p++;
	lenLeft--;
	commandLinePtr = p;

	while(*p != ' '){
		if(lenLeft <= 0) break;
		p++;
	}
	*p = '\0';
	const char * user = commandLinePtr;
	p++;
	lenLeft--;
	commandLinePtr = p;

	while(*p != ' ' && *p != '\0'){ 
		if(lenLeft <= 0) break;
		p++;
		lenLeft--;
	}
	*p = '\0';
	const char * password = commandLinePtr;
	p++;
	lenLeft--;
	commandLinePtr = p;
	const char * args = (char*)calloc(1,sizeof(char));
	if(lenLeft > 0){
		args = strdup(commandLinePtr);
	}
	
	if((*args >= 'A' && *args <= 'Z') || (*args >= 'a' && *args <= 'z') || (*args >= '0' && *args <= '9')) {
	} else {
		args = "";
	}

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	const char* msg;

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if(!checkPassword(fd, user, password)) {
		msg = strdup("Wrong Password\r\n");
		write(fd, msg, strlen(msg));
	} else {
		if (!strcmp(command, "CREATE-ROOM")) {
				createRoom(fd, user, password, args);
		}
		else if (!strcmp(command, "LIST-ROOMS")) {
				listRoom(fd, user, password, args);
		}else if (!strcmp(command, "ENTER-ROOM")) {
				enterRoom(fd, user, password, args);
		}
		else if (!strcmp(command, "LEAVE-ROOM")) {
				leaveRoom(fd, user, password, args);
		}
		else if (!strcmp(command, "SEND-MESSAGE")) {
				sendMessage(fd, user, password, args);
		}
		else if (!strcmp(command, "GET-MESSAGES")) {	
				getMessages(fd, user, password, args);
		}
		else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
				getUsersInRoom(fd, user, password, args);
		}
		else if (!strcmp(command, "GET-ALL-USERS")) {
				getAllUsers(fd, user, password, args);
		}
		else if(!strcmp(command, "LOGIN")) {
			msg = strdup("Login successful\r\n");
			write(fd, msg, strlen(msg));
		}
		else if(!strcmp(command, "SHUTDOWN")) {
			msg = strdup("Shutting down\r\n");
			running = false;
			write(fd, msg, strlen(msg));
		}
		else {
			msg =  "UNKNOWN COMMAND\r\n";
			write(fd, msg, strlen(msg));
		}
	}
	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));
	close(fd);	
}

