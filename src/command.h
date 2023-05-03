#ifndef COMMAND_H
#define COMMAND_H

#include<string>
#include<vector>
#include<map>
#include<memory>
class Editor;

class Command {

	public:
		Command(Editor *editor, int multiplicity);
		virtual ~Command() {};
		virtual void execute() = 0;
		virtual char getType() = 0;

		//get methods
		void setMultiplicity(int);

	protected:

		Editor *editor;
		int multiplicity;

		//helper methods
		void enter(std::vector<std::string> &lines, std::string &curLine, int curChar, int curLineNum);
		int firstNonWS(const std::string &s);
		
		void del(int);
		void delPrev(int);
		void adjustCurChar();
		void reset();
		void delLines(int);
		void insertLine(bool append = false);

		void searchFromTop(std::string, int);
		void searchFromBottom(std::string, int);
		void searchForward(std::string, int);
		void searchBack(std::string, int);

		void yank(int);
		void yankLines(int);
		void yankPrev(int);
};

class CharCommand : public Command {

	typedef void (CharCommand::*FPtr)();

	char command;
	// map of char commands to functions
	std::map<char,FPtr> fmap;

	public:
		CharCommand(Editor *editor, char c, int multiplicity);
		~CharCommand() {};
		void execute() override;
		char getType() override;

	private:

		// helper methods
		void modify();
		void paste(bool after);

		// add methods for each command here
		void moveLeft();
		void moveRight();
		void moveUp();
		void moveDown();
		void append();
		void del();
		void delPrev();
		void joinLines();
		void repeat();
		void undo();
		void countWordForward();
		void countWordBack();
		void goToFront();
		void goToBack();
		void pasteAfter();
		void pasteBefore();
		void repeatFindChar();
		void firstNonBlank();
		void repeatSearch();
		void switchRepeatSearch();
		void matchBracket();
                void goToPercent();
                void findOpenComment();
                void findCloseComment();
                void findOpen(char c, char f);
                void findClose(char c, char f);
		bool match(const std::string &, int);
};

class LongCommand : public Command {

	typedef void (LongCommand::*FPtr)();

	std::string command;
	// map of string commands to methods
	std::map<char,FPtr> fmap;

	//add functions for each command
	void replace();
	void delLines();
	void subsLines();
	void yankLines();
	void countCharForward();
        void countCharBack();

	public:
		LongCommand(Editor *editor, std::string c, int multiplicity);
		~LongCommand() {};
		void execute() override;
		char getType() override;

};

class StatusBarCommand : public Command {

	typedef void (StatusBarCommand::*FPtr)();

	std::string command;
	// map of type to methods
	std::map<std::string,FPtr> fmap;
	
	void colonCommand();
	void save();
	void quit();
	void saveQuit();
	void forceQuit();
	void goToLine(const std::string &str);
	
	int searchForwardInLine(std::string line, int pos);
	void searchForward();
	int searchBackInLine(const std::string &line, int pos);
	void searchBack();
	void insertFile();

	public:
		StatusBarCommand(Editor *editor, std::string command, int mult);
		~StatusBarCommand() {};
		void execute() override;
		char getType() override;

		void setType(char);

};

class ModeCommand : public Command {

	typedef void (ModeCommand::*FPtr)(int);
	typedef bool (ModeCommand::*InitF)();

	// map to type of methods
	std::map<char,FPtr> fmap;
	// map to status bar string
	std::map<char,InitF> imap;
	// should init be repeated when executed multiple times?
	std::map<char,bool> repeatInit;
	
	//helper functions
	void backspace(std::vector<std::string> &lines, std::string &curLine, int curChar, int curLineNum);

	//methods for each command
	bool insWSInit();
	void replace(int c);
	bool replaceInit();
	bool appendInit();
	bool appendEndInit();
	bool newLineInit();
	bool newLineAboveInit();
	bool delInit();
	bool subInit();

	protected:
		char type;
		std::vector<int> edits;
		// should the mode part be executed multiple times?
		std::map<char,bool> noRepeatEdit;

		bool insertInit();
		void insert(int c);
		void repeat();

	public:
		ModeCommand(Editor *editor, char type, int multiplicity);
		~ModeCommand() {};
		void execute() override;
		char getType() override;
		void edit(int c);
		virtual void finish();
		virtual bool start();
};

class CtrlCommand : public Command {
	typedef void (CtrlCommand::*FPtr)();
	int type;
	//ctrl commands are all captured as ints from 0-26
	std::map<int,FPtr> fmap;
	void pageUp();
	void pageDown();
	void scrollUp();
	void scrollDown();
	
	void printInfo();
	public:
	CtrlCommand(Editor *editor, int type, int mult);
	virtual void execute() override;
	char getType() override;
};

class MoveActionCommand : public ModeCommand {

	typedef bool (MoveActionCommand::*FPtr)();

	std::shared_ptr<Command> command;

	std::map<char,bool> isMove;
	std::map<char,bool> isCharWise;
	std::map<char,FPtr> isMode;

	//command functions
	bool moveInterface();
	
	void del(int mult);
	void delPrev(int);
	void delLines(int);
	void delMerge(int, int);

	void yank(int);
	void yankPrev(int);
	void yankLines(int);
	void yankMerge(int,int);

	bool subsInit();

	public: 
		MoveActionCommand(Editor *editor, int multiplicity, char type, Command *command);
		void execute() override;
		char getType() override;
		bool start() override;
		void finish() override;
};

#endif
