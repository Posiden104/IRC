
#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"
#define ROOM_FILE "room.txt"

typedef struct User {
	char *username;
	char *password;
}User;

typedef struct Message {
	char *content;
	char *username;
	int number;
}Message;

typedef struct Room {
	char *name;
	std::list<User> *users;
	std::list<Message> *messages;
}Room;

class IRCServer {
	// Add any variables you need
private:
	int open_server_socket(int port);
	char *userWord;
	char *passWord;
	std::list<User> _users;
	std::list<Room> _rooms;

public:
	void processRequest( int socket );
	void initialize();
	bool checkPassword(int fd, const char * user, const char * password);
	bool findUser(const char *user);
	void addUser(int fd, const char * user, const char * password, const char * args);
	void createRoom(int fd, const char * user, const char * password, const char * args);
	void listRoom(int fd, const char * user, const char * password, const char * args);
	void enterRoom(int fd, const char * user, const char * password, const char * args);
	void leaveRoom(int fd, const char * user, const char * password, const char * args);
	void sendMessage(int fd, const char * user, const char * password, const char * args);
	void getMessages(int fd, const char * user, const char * password, const char * args);
	void getUsersInRoom(int fd, const char * user, const char * password, const char * args);
	void getAllUsers(int fd, const char * user, const char * password, const char * args);
	void runServer(int port);
	void newUser(const char *uname, const char *pass);

};
#endif
