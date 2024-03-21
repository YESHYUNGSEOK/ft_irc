#ifndef COMMANDHANDLER_HPP
# define COMMANDHANDLER_HPP

# include <string>
# include <iostream>
# include <map>

typedef enum CMD {
	PASS, NICK, USER, OPER, //0-3
	QUIT, JOIN, PART, TOPIC, MODE, //4-8
	NAMES, LIST, INVITE, KICK, //9-12
	PRIVMSG, WHO, WHOIS, WHOWAS, //13-16
	KILL, PING, PONG,	//17-19
	NONE //20
} e_cmd;

class CommandHandler {
	public:
		static void getCommand(e_cmd cmd);
		// every command functions
		static void pass(); // param: serverPS
		static void nick(); // param: user list(nick, user, host, server, real)
		static void user(); // param: user list
		static void oper(); // param: name, password, channel list
		static void quit(); // param: serv, client socket
		static void join(); // param: channel list, user list
		static void part(); // param: channel, user
		static void topic(); // param: channel list
		static void mode(); // param: channel
		static void names(); // param: user list
		static void list(); // param: channel list
		static void invite(); // param: user list, channel list
		static void kick(); // param: user
		static void privmsg(); // param: channel, user list
		static void who();
		static void whois();
		static void whowas();
		static void kill();
		static void ping();
		static void pong();
};

#endif