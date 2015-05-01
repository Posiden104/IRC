
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
	
	initialize();
	
	while ( 1 ) {
		
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

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

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

bool
compare(const char *a, const char*b) 
{
	return strcmp(a, b) < 0;
}

void 
IRCServer::newUser(const char *uname, const char *pass) {
	if(strcmp(uname, "") == 0 ) return;
	if(uname == NULL) return;
	printf("User added\r\n");
	User q = (User*)calloc(1, sizeof(User));
	
	q->username = strdup(uname);
	q->password = strdup(pass);
		
	_users.push_front(*q);
		
}

void
IRCServer::initialize()
{
	userWord = (char*)calloc(100, sizeof(char));
	passWord = (char*)calloc(100, sizeof(char));
	int c;
	char *u = userWord;
	char *p = passWord;

	// Open password file <- saves username/password for each user (persistant)
	FILE *fd;
	fd = fopen(PASSWORD_FILE, "a+");
	
	int flag = 0;
	while((c = fgetc(fd)) != EOF) {
		if(c == '\n' || c == '\r') {
			newUser(userWord, passWord);
			userWord = (char*)calloc(100, sizeof(char));
			passWord = (char*)calloc(100, sizeof(char));
			u = userWord;
			p = passWord;
			flag = 0;
		} else if(c == ' ') {
			flag = 1;
			continue;
		} else if(flag) { // writing password
			*p = c;
			p++;
		} else if(!flag) { // writing username
			*u = c;
			u++;
		}
	}

	fclose(fd);
	
	free(userWord);
	free(passWord);
	addUser(-1, "admin", "123", "");
	createRoom(-1, "admin", "123", "room1");
}

bool
IRCServer::checkPassword(int fd, const char * username, const char * password) {
	// Here check the password
	void *data;
	User *u;
	//if(_users.find(username, &data)) {
	//	u = (user*)data;
//		if(strcmp(&(*u->password), password) == 0) {
//			return true;
//		}
//	}
	return false;
}  

bool
IRCServer::findUser(const char *username)
{

}

void
IRCServer::addUser(int fd, const char * username, const char * password, const char * args)
{
	// Here add a new user. For now always return OK.
	
	void * blank;
	const char *msg;
	char *msg2 = (char*)calloc(100, sizeof(char));

//	if(_users.find(username, &blank)) {
		msg = strdup("ERROR (user already exists)\r\n");
//	} else {
		msg = strdup("OK\r\n");
		newUser(username, password);
		
		FILE *psswrd;
		psswrd = fopen(PASSWORD_FILE, "a");
		strcat(msg2, username);
		strcat(msg2, " ");
		strcat(msg2, password);
		strcat(msg2, "\r\n");
		fprintf(psswrd, "%s",  msg2);
		fclose(psswrd);
//	}
	if(fd != -1) write(fd, msg, strlen(msg));
	return;		
}

void
IRCServer::createRoom(int fd, const char * username, const char * password, const char * args) 
{
	const char *msg = (char*)calloc(100, sizeof(char));
	void *blank;
/*	if(_rooms.find(args, &blank)) {
		msg = strdup("DENIED (Room already exists)\r\n");
		write(fd, msg, strlen(msg));
		return;
	} else {
		roomptr = (Room*)malloc(sizeof(Room));
		roomptr->name = strdup(args);
		roomptr->messages = (new std::list<Message>());
		mess.clear();
		roomptr->users.head = NULL;
		_rooms.insertItem(args, (void*)roomptr);
	}

	msg = strdup("OK\r\n");
	if(fd != -1) write(fd, msg, strlen(msg));
*/	return;
}

void
IRCServer::listRoom(int fd, const char * username, const char * password, const char * args)
{
/*	Room *roomptr;
	HashTableVoidIterator HTI(&_rooms);
	const char *key;
	roomptr = (Room*)calloc(1, sizeof(Room));
	void *data;
	char *msg = (char*)calloc(1000, sizeof(char));
	while(HTI.next(key, data)) {
		roomptr = (Room*)data;
		if(roomptr->name == '\0') {
			continue;
		}
		msg = (char*)realloc(msg, (strlen(msg)+strlen(roomptr->name)+2)*sizeof(char));
		strcat(msg, (roomptr->name));
		strcat(msg, "\r\n");
	}
	write(fd, msg, strlen(msg));
*/	return;

}

void
IRCServer::enterRoom(int fd, const char * username, const char * password, const char * args)
{
/*	const char *msg;
	void *data;
	Room *r;
	user *u;
	if(_rooms.find(args, &data)) {
		r = (Room*)data;
		if(_users.find(username, &data)) {
			u = (user*)data;	
			if(!sllist_contains(&(r->users), &(*u->username))) {
				sllist_add_end(&(r->users), (u->username));
			}
			msg = strdup("OK\r\n");			
		} else {
			msg = strdup("ERROR (user not found)\r\n");
		}
	} else {
		msg = strdup("ERROR (No room)\r\n");
	}

	write(fd, msg, strlen(msg));
*/	return;
}

void
IRCServer::leaveRoom(int fd, const char * username, const char * password, const char * args)
{
/*	const char *msg;
	void *data;
	Room r;
	user *u;
	if(_rooms.find(args, &data)){
		r = *((Room*)data);
		if(_users.find(username, &data)){
			u = (user*)data;
			if(sllist_contains(&(r.users), &(*u->username))) {
				sllist_remove(&(r.users), &(*u->username));
				msg = strdup("OK\r\n");
			} else {
				msg = strdup("ERROR (No user in room)\r\n");
			}
		} else {
			msg = strdup("ERROR (user not found)\r\n");
		}
	} else {
		msg = strdup("ERROR (No room)\r\n");
	}

	write(fd, msg, strlen(msg));
*/ return;
}

void
IRCServer::sendMessage(int fd, const char * username, const char * password, const char * args)
{
/*	char *msg;
	char *arguements = strdup(args);
	char *p = arguements;
	if(*p == '\0') {
		char *ret = strdup("ERROR (no room name arguement)\r\n");
		write(fd, ret, strlen(ret));
		return;
	}
	while(*p != ' ' && *p != '\0') p++;
	*p = '\0';
	char *roomName = arguements;
	p++;
	if(*p == '\0') {
		char *ret = strdup("ERROR (user not in room)\r\n"); //GOOD
		write(fd, ret, strlen(ret));
		return;
	}
	char *message = p;
	void *data;
	Room *r;
	if(_rooms.find(roomName, &data)) {
		r = (Room*) data;
		user *u;
		if(_users.find(username, &data)){
			u = (user*)data;
		} else {
			msg = strdup("ERROR (No user)\r\n");
			write(fd, msg, strlen(msg));
			return;
		}
		if(!sllist_contains(&(r->users), &(*u->username))) {
			msg = strdup("ERROR (user not in room)\r\n");
			write(fd, msg, strlen(msg));
			return;
		}
		int numOfMessages = r->messages->back().number;
		int firstMessage = r->messages->front().number;
		while(numOfMessages - firstMessage >= 100) {
			r->messages->pop_front();
			firstMessage = r->messages->front().number;
		}
		Message *m = (Message*)calloc(1, sizeof(Message));
		m->content = message;
		m->username = strdup(username);
		if(r->messages->empty()) {
			m->number = 0;
		} else {
			m->number = (r->messages->back().number) + 1;
		}
		r->messages->push_back(*m);
		
		msg = strdup("OK\r\n");
	} else {
		msg = strdup("ERROR (No room)\r\n");
	}

	write(fd, msg, strlen(msg));
*/	return;
}
void
IRCServer::getMessages(int fd, const char * username, const char * password, const char * args)
{
/*	int msgNum;
	char *arg = strdup(args);
	char *p = arg;
	if(*p == '\0') {
		char *ret = strdup("ERROR (no message number arguement)\r\n");
		write(fd, ret, strlen(ret));
		return;
	}
	while(*p != ' ' && *p != '\0') p++;
	*p = '\0';
	msgNum = atof(arg);
	p++;
	if(*p == '\0') {
		char *ret = strdup("ERROR (no room name)\r\n");
		write(fd, ret, strlen(ret));
		return;
	}
	char *roomName = p;
	void *data;
	Room *r;
	if(_rooms.find(roomName, &data)) {
		r = (Room*) data;
		user *u;
		if(_users.find(username, &data)){
			u = (user*)data;
		} else {
			char *msg = strdup("ERROR (No user)\r\n");
			write(fd, msg, strlen(msg));
			return;
		}
		if(!sllist_contains(&(r->users), &(*u->username))) {
			char *msg = strdup("ERROR (User not in room)\r\n");
			write(fd, msg, strlen(msg));
			return;
		}
		if(r->messages->back().number < msgNum || r->messages->empty()) {
			char *msg = strdup("NO-NEW-MESSAGES\r\n");
			write(fd, msg, strlen(msg));
			return;
		}
		
		int i = r->messages->back().number - 100;
		char *response = (char*)calloc(100, sizeof(char));
		for(std::list<Message>::iterator it = r->messages->begin(); it != r->messages->end(); ++it) {
			Message g = *it;
			if(g.number < msgNum) continue;
			else if(i > 0 && msgNum < i){
				continue;
			}
			else {
				char *num = (char*)calloc(10, sizeof(char));
				sprintf(num, "%d", g.number);
				int length = strlen(num) + strlen(g.username) + strlen(g.content) + strlen("  \r\n");
				char *ret = (char*)calloc(length, sizeof(char));
				strcat(ret, num);
				strcat(ret, " ");
				strcat(ret, g.username);
				strcat(ret, " ");
				strcat(ret, g.content);
				strcat(ret, "\r\n");
				response = (char*)realloc(response, strlen(ret)*sizeof(char));
				strcat(response, ret);
			}
		}
		write(fd, response, strlen(response));
	} else {
		char* ret = strdup("ERROR (No room)\r\n");
		write(fd, ret, strlen(ret));
	}
	write(fd, "\r\n", strlen("\r\n"));
*/	return;
}

void
IRCServer::getUsersInRoom(int fd, const char * username, const char * password, const char * args)
{
/*	char *msg;
	void *data;
	user *u;
	Room *roomptr;
	if(_rooms.find(args, &data)) {
		roomptr = (Room*)data;
		msg = strdup(sllist_returnNames(&(roomptr->users)));
	} else {
		msg = strdup("ERROR (No Room)\r\n");
		write(fd, msg, strlen(msg));
		return;
	}
	char *p = msg;
	char *uName = (char*)calloc(100, sizeof(char));
	char *q = uName;
	std::vector<char*> userVector;
	userVector.clear();
	while(*p != '\0') {
		if(*p == '\n') {
			*q = *p;
			p++;
			userVector.push_back(uName);
			uName = (char*)calloc(100, sizeof(char));
			q = uName;
		} else {
			*q = *p;
			q++;
			p++;
		}
	}
	std::sort(userVector.begin(), userVector.end(), compare);
	char *response = (char*)calloc(100, sizeof(char));
	for(vector<char*>::iterator it = userVector.begin(); it != userVector.end(); ++it) {
		char* ret = *it;
		if(ret == '\0') continue;
		response = (char*)realloc(response, (strlen(response)+strlen(ret)+2)*sizeof(char));
		strcat(response, ret);
		strcat(response, "\r\n");
	}
	response = (char*)realloc(response, (strlen(response)+2)*sizeof(char));
	strcat(response, "\r\n");
	write(fd, response, strlen(response+2));
*/	return;
}

void
IRCServer::getAllUsers(int fd, const char * username, const char * password,const  char * args)
{
/*	HashTableVoidIterator HTI(&_users);
	const char *key;
	user *q = (user*)calloc(1, sizeof(user));
	user u;
	void *data;
	char *msg = (char*)calloc(1000, sizeof(char));
	vector<char*> userVector;
	userVector.clear();
	while(HTI.next(key, data)) {
		q = (user*)data;
		u = *q;
		if(u.username == '\0') {
			continue;
		}
		userVector.push_back(u.username);
	}
	std::sort(userVector.begin(), userVector.end(), compare);
	char *response = (char*)calloc(100, sizeof(char));
	for(vector<char*>::iterator it = userVector.begin(); it != userVector.end(); ++it) {
		char* msg2 = *it;
		if(msg2 == NULL) continue;
		response = (char*)realloc(response, (strlen(response)+strlen(msg2)+2)*sizeof(char));
		strcat(response,  msg2);
		strcat(response, "\r\n");
	}
	response = (char*)realloc(response, (strlen(response)+2)*sizeof(char));
	strcat(response, "\r\n");
	write(fd, response, strlen(response) + 2);
*/	return;
}

