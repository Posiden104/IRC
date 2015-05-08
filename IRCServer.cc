
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
	User *u;
	if(!findUser(username, &u, &_users)) return false;
	if(u == NULL) return false;	// Extra protection
	if(!strcmp(u->username, username) && !strcmp(u->password, password)) {
		return true;
	}
	return false;
}  

/* Sets ret to the user with a name matching "username", otherwise ret = NULL */
bool
IRCServer::findUser(const char *username, User **ret, std::list<User> *_list)
{
	User u;
	*ret = (User*)calloc(1, sizeof(User));

	if(!_list->empty()){
		for(std::list<User>::iterator it = _list->begin(); it != _list->end(); ++it) {
			u = *it;				// Assigns the pointer of current user to ret
			if(!strcmp(u.username, username)) {	// Compares the current user's name to "username"
				*ret[0] = u;
				return true;
			}
		}
	}
	ret = NULL;
	return false;
}

/* Sets ret to the room with a name matching "room", otherwise ret = NULL */
bool
IRCServer::findRoom(const char *room, Room **ret) 
{
	Room r;
	*ret = (Room*)malloc(1* sizeof(Room));
	for(std::list<Room>::iterator it = _rooms.begin(); it != _rooms.end(); ++it) {
		r = *it;				// Assigns the pointer of the current room to ret
		if(!strcmp(r.name, room)) {		// Compares the current room's name to "room"
			*ret[0] = r;
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
	if(!findUser(username, &u, &_users)){ 
		free(u);
		// Add new user
		newUser(username, password);
		FILE *psswrd;

		// Record the username and password in PASSWORD_FILE
		psswrd = fopen(PASSWORD_FILE, "a");
		fprintf(psswrd, "%s %s\r\n", username, password);
		fclose(psswrd);
		
		msg = strdup("OK\r\n");
	} else {
		free(u);
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
	if(findRoom(args, &room)) {
		msg = strdup("DENIED (Room already exists)\r\n");
		write(fd, msg, strlen(msg));
		free(msg);
		free(room);
		return;
	} else {
		free(room);
		room = (Room*)malloc(sizeof(Room));
		room->name = strdup(args);
		room->messages = new std::list<Message>();
		room->messages->clear();
		room->users = new std::list<User>();
		room->users->clear();
		_rooms.push_front(*room);
		// Sort the rooms alphabetically
		_rooms.sort(compareRooms);
	}

	msg = strdup("OK\r\n");
	// Check to see if room is server generated (r1)
	if(fd != -1) write(fd, msg, strlen(msg));
	free(msg);
	return;
}

/* Puts the names of all rooms into one char * and writes it to the socket */
void
IRCServer::listRoom(int fd, const char * username, const char * password, const char * args)
{
	Room *roomptr;
	char *msg = (char*)calloc(100, sizeof(char));
	int len = 0;
	int max = 100;

	// Sort the rooms alphabetically
	_rooms.sort(compareRooms);
	for(std::list<Room>::iterator it = _rooms.begin(); it != _rooms.end(); ++it) {
		roomptr = &(*it);
		len += strlen(roomptr->name);
		
		// Resize the char * if needed
		if(len >= max) {
			msg = (char*)realloc(msg, (2*max)*sizeof(char));
			max *= 2;
		}
		if(msg == NULL) {
			write(fd, "Out of memory\r\n", 15);
			exit(1);
		}
		strcat(msg, roomptr->name);
		strcat(msg, "\r\n");
	}

	write(fd, msg, strlen(msg));
	free(msg);
	return;

}

/* Puts the user into the room to send and recieve messages */
void
IRCServer::enterRoom(int fd, const char * username, const char * password, const char * args)
{
	char *msg;
	Room *tr;
	User *tu;

	// See if the room exists
	if(findRoom(args, &tr)) {
		// See if the user is already in the room
		if(!findUser(username, &tu, tr->users) && findUser(username, &tu, &_users)) {	
			tr->users->push_front(*tu);
			tr->users->sort(compareUsers);
			msg = strdup("OK\r\n");			
		} else {
			free(tu);
			msg = strdup("ERROR (user already in room)\r\n");
		}
	} else {
		msg = strdup("ERROR (No room)\r\n");
	}
	free(tr);
	write(fd, msg, strlen(msg));
	free(msg);
	return;
}

void
IRCServer::leaveRoom(int fd, const char * username, const char * password, const char * args)
{
	char *msg;
	Room *r;
	User *u;
	
	// See if the room exists
	if(findRoom(args, &r)){
		// See if the user is in the room
		if(findUser(username, &u, r->users)){
			// Create iterator to check each element
			std::list<User>::iterator it = r->users->begin();
			while(it != r->users->end()) {
				u = &(*it);
				if(strcmp(u->username, username) != 0) {
					++it;
				} else {
					r->users->erase(it);
					msg = strdup("OK\r\n");
					break;
				}
				msg = NULL;
			}
			if(msg == NULL) msg = strdup("ERROR (finding user error)\r\n");
		} else {
			msg = strdup("ERROR (No user in room)\r\n");
		}
	} else {
		msg = strdup("ERROR (No room)\r\n");
	}

	write(fd, msg, strlen(msg));
	free(r);
	free(msg);
 	return;
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
	char *msg = (char*)calloc(1000, sizeof(char));
	int len = 0;
	int max = 1000;
	User *u;
	Room *r;

	if(!findRoom(args, &r)) {
		free(r);
		msg = strdup("Room not found\r\n");
		return;
	}
	// Sort the user list
	r->users->sort(compareUsers);

	for(std::list<User>::iterator it = r->users->begin(); it != r->users->end(); ++it) {
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

