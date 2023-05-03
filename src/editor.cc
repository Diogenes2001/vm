#include "editor.h"
#include <ncurses.h>
#include <string>
#include "file.h"

Editor::Editor() : isSaved{true}, shouldClose{false}, scrollUpDefault{LINES/2}, scrollDownDefault{LINES/2}, color{true} {};

//get methods

File &Editor::getFile() { return *file; }

int Editor::getCurrentLine() { return currentLine; }

int Editor::getCurrentLineLength() {
	
	std::string line = file->getLines()[currentLine];
	return line.size() - 1;

}

int Editor::getCurrentChar() { return currentChar; }

int Editor::getMaxCurrentChar() { return maxCurrentChar; }

bool &Editor::getShouldClose() { return shouldClose; }

bool &Editor::getIsSaved() { return isSaved; }

std::string &Editor::getStatusMessage() { return statusMessage; }

std::string &Editor::getFilename() { return filename; }

int &Editor::getScrollUpDefault() { return scrollUpDefault; }

int &Editor::getScrollDownDefault() { return scrollDownDefault; }

Command *Editor::getLastModification() { return lastModification.get(); }

LongCommand *Editor::getLastFindChar() { 
	return lastFindChar.get(); 
}

StatusBarCommand *Editor::getLastSearch() { 
	return lastSearch.get(); 
}

//other methods

void Editor::open(std::string filename) {

	this->filename = filename;
	
	file.reset(new File{filename});

}

void Editor::toggleColor(){ color = !color; }
bool Editor::getColor() { return color; }

void Editor::modify() { 
	std::shared_ptr<File> oldFile = file;
	oldFile->setCurrentLine(currentLine);
	oldFile->setCurrentChar(currentChar);
	file.reset(new File{ oldFile->getLines(), oldFile });
	isSaved = false;
}
void Editor::load(std::shared_ptr<File> file) {
	if (!file.get()) return;
	currentLine = file->getCurrentLine();
	currentChar = file->getCurrentChar();
	maxCurrentChar = file->getCurrentChar();
	this->file = file;
}

void Editor::setLastModification(Command *c) {
	lastModification.reset(c);
}

void Editor::setLastFindChar(LongCommand *c) { 
	lastFindChar.reset(c); 
}

void Editor::setLastSearch(StatusBarCommand *c) { lastSearch.reset(c); }

void Editor::appendStatusBar(char c) { statusMessage.push_back(c); }
void Editor::popStatusBar() { statusMessage.pop_back(); }

void Editor::setStatusBar(const std::string &s) { statusMessage.assign(s); }

void Editor::resetStatusBar() { statusMessage.clear(); }

void Editor::setCurrentChar(int c) { 
	currentChar = c;
}

void Editor::setCurrentLine(int c) { currentLine = c; }

void Editor::setMaxCurrentChar(int c) { maxCurrentChar = c; }

void Editor::setCopiedChars(std::string txt) { 
	copiedChars = txt;
	didCopyMultipleLines = false;
}

void Editor::setCopiedLines(std::vector<std::string> lines) { 
	copiedLines = lines;
	didCopyMultipleLines = true;
	isCharWise = false;
}

void Editor::setCopiedLinesCharWise(std::vector<std::string> lines) {
	copiedLines = lines;
	didCopyMultipleLines = true;
	isCharWise = true;
}

bool Editor::getDidCopyMultipleLines() { return didCopyMultipleLines; }
bool Editor::getCharWise() { return isCharWise; }
const std::string &Editor::getCopiedChars() { return copiedChars; }
const std::vector<std::string> &Editor::getCopiedLines() { return copiedLines; }
