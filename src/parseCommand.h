#ifndef PARSECOMMAND_H
#define PARSECOMMAND_H

#include "model.h"
#include "editor.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
struct State;
class ModeCommand;
class Command;

class ParseCommand : public Model {

	Editor editor;
	bool isRecording = false;
	char currentRegister;
	std::map<char,std::vector<std::unique_ptr<Command>>> macros;
	
	std::map<char,bool> needMode;
	std::map<char,bool> needFeed;
	std::map<char,bool> isLong;
	std::map<char,bool> needSecond;
	std::map<char,bool> zeroMultiplicity;

	//private methods
	
	//parsing
	void listen();
	Command *parseNext(char prev = 0);
	Command *parseCtrlCommand(int i, char prev);
	Command *parseModeCommand(char c, int multiplicity, char prev);
	Command *parseStatusBarCommand(char c, int multiplicity, char prev);
	Command *parseLongOrCharCommand(char c, int multiplicity, char prev);

	//execution
	void inMode(ModeCommand *c);
	std::string feed(char c);
	void doMacro(char c);

	//helper
	std::string getCharOrLongCommand(int &multiplicity, char c, char first = 0);

	public:
		ParseCommand();
		void open(std::string filename);
		State getState();

};

#endif
