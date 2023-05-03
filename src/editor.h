#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include "command.h"
class File;

class Editor {

	std::shared_ptr<File> file;
	std::string filename;
	std::string statusMessage;
	int currentLine = 0;
	int maxCurrentChar = 0;
	int currentChar = 0;
	bool isSaved;
	bool shouldClose;
	bool didCopyMultipleLines = false;
	bool isCharWise = false;
	std::string copiedChars;
	std::vector<std::string> copiedLines;
	std::unique_ptr<StatusBarCommand> lastSearch;
	std::unique_ptr<LongCommand> lastFindChar;
	int scrollUpDefault;
	int scrollDownDefault;
	std::unique_ptr<Command> lastModification;

	bool color;	
	public:
		Editor();

		File &getFile();

		int getCurrentLine();
		int getCurrentLineLength();
		int getCurrentChar();
		int getMaxCurrentChar();

		void setCurrentLine(int c);
		void setCurrentChar(int c);
		void setMaxCurrentChar(int c);
	
		void toggleColor();
		bool getColor();

		int &getScrollUpDefault();
		int &getScrollDownDefault();

		bool &getShouldClose();
		bool &getIsSaved();
		std::string &getStatusMessage();
		std::string &getFilename();

		void open(std::string filename);

		void modify();
		void load(std::shared_ptr<File> file);
		void setLastModification(Command *c);
		Command *getLastModification();
		void setLastFindChar(LongCommand *c);
		LongCommand *getLastFindChar();
		void setLastSearch(StatusBarCommand *c);
		StatusBarCommand *getLastSearch();

		void appendStatusBar(char c);
		void popStatusBar();
		void setStatusBar(const std::string &s);
		void resetStatusBar();

		void setCopiedChars(std::string txt);
		void setCopiedLines(std::vector<std::string> lines);
		void setCopiedLinesCharWise(std::vector<std::string> lines);

		bool getDidCopyMultipleLines();
		bool getCharWise();
		const std::string &getCopiedChars();
		const std::vector<std::string> &getCopiedLines();

};

#endif
