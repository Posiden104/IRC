
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

using namespace std;

/* Compare function to aid in the alphabetical sorting of users */
bool
compareUsers(User a, User b) 
{
	return strcmp(a.username, b.username) < 0;
}

/* Compare function to aid in the alphebetical sorting of rooms */
bool
compareRooms(Room a, Room b)
{
	return strcmp(a.name, b.name) < 0;
}

void 
IRCServer::newUser(const char *uname, const char *pass) {
	if(strcmp(uname, "") == 0 ) return;
	if(uname == NULL) return;
	printf("User added\r\n");
	User *q = (User*)calloc(1, sizeof(User));
	
	q->username = strdup(uname);
	q->password = strdup(pass);
		
	_users.push_front(*q);
		
}

/* Preforms the initial server startup functions (loading persistant users and passwords) */
void
IRCServer::initialize()
{
	userWord = (char*)calloc(100, sizeof(char));
	passWord = (char*)calloc(100, sizeof(char));
	int c;
	char *u = userWord;
	char *p = passWord;

	// Loads the saved users and their passwords upon a server startup
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
	
	// Create server generated users (ADMIN)
	addUser(-1, "ADMIN", "123", "");

	// Create server generated Rooms (r1)
	createRoom(-1, "ADMIN", "123", "r1");
}

/* Authenticates the user's password */
bool
IRCServer::checkPassword(int fd, const char * username, const char * password) 
{
	printf("check user password\n");
	User *u;
	if(!findUser(username, u)) return false;
	if(u == NULL) return false;	// Extra protection
	if(!strcmp(u->username, username) && !strcmp(u->password, password)) {
		return true;
	}
	return false;
}  

/* Sets ret to the user with a name matching "username", otherwise ret = NULL */
bool
IRCServer::findUser(const char *username, User *ret)
{
	for(std::list<User>::iterator it = _users.begin(); it != _users.end(); ++it) {
		ret = &(*it);				// Assigns the pointer of current user to ret

		if(!strcmp(ret->username, username)) {	// Compares the current user's name to "username"
			return true;
		}
	}
	ret = NULL;
	return false;
}

/* Sets ret to the room with a name matching "room", otherwise ret = NULL */
bool
IRCServer::findRoom(const char *room, Room *ret) 
{
	for(std::list<Room>::iterator it = _rooms.begin(); it != _rooms.end(); ++it) {
		ret = &(*it);				// Assigns the pointer of the current room to ret
		if(!strcmp(ret->name, room)) {		// Compares the current room's name to "room"
			return true;
		}
	}
	ret = NULL;
	return false;
}

/* Screens for existing users attempting to "create" instead of logging in */
void
IRCServer::addUser(int fd, const char * username, const char * password, const char * args)
{
	// Here add a new user.
	User *u;
	char *msg;
	
	// Find if the user exists already
	if(!findUser(username, u)){ 
		
		// Add new user
		newUser(username, password);
		FILE *psswrd;

		// Record the username and password in PASSWORD_FILE
		psswrd = fopen(PASSWORD_FILE, "a");
		fprintf(psswrd, "%s %s\r\n", username, password);
		fclose(psswrd);
		
		msg = strdup("OK\r\n");
	} else {
		msg = strdup("ERROR (user already exists)\r\n");
	}
	
	// Check to see if user is server generated (ADMIN)
	if(fd != -1) write(fd, msg, strlen(msg));
	free(msg);
	return;		
}

/* Construct a new room for users to join and chat */
void
IRCServer::createRoom(int fd, const char * username, const char * password, const char * args) 
{
	char *msg;
	Room *room;
	if(findRoom(args, room)) {
		msg = strdup("DENIED (Room already exists)\r\n");
		write(fd, msg, strlen(msg));
		free(msg);
		return;
	} else {
		room = (Room*)malloc(sizeof(Room));
		room->name = strdup(args);
		room->messages = new std::list<Message>();
		room->messages->clear();
		room->users = new std::list<User>();
		room->users->clear();
		_rooms.push_front(*room);

	}

	msg = strdup("OK\r\n");
	// Check to see if room is server generated (r1)
	if(fd != -1) write(fd, msg, strlen(msg));
	free(msg);
	return;
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

/* Writes all users that have been created since the last username/password wipe to the socket in 1 string */
// IDEA: add reset server/clean users & rooms command?
void
IRCServer::getAllUsers(int fd, const char * username, const char * password,const  char * args)
{
	char *msg = (char*)calloc(1000, sizeof(char));
	int len = 0;
	int max = 1000;
	User *u;
	
	// Sort the user list
	_users.sort(compareUsers);

	for(std::list<User>::iterator it = _users.begin(); it != _users.end(); ++it) {
		u = &(*it);
		len += strlen(u->username) + 2;
		
		// Adjust size of msg
		if(len >= max) {
			msg = (char*)realloc(msg, (2 * len) * sizeof(char));
			if(msg == NULL) {
				write(fd, "Out of memory\r\n", 15);
				exit(1);
			}
			max = 2 * len;
		}

		// Add the current user's name to the msg string
		strcat(msg, u->username);
		strcat(msg, "\r\n");
	}

	write(fd, msg, strlen(msg));
	free(msg);
	return;
} 

