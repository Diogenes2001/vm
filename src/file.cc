#include "file.h"
#include<fstream>
#include <vector>
#include <memory>
// constructors

File::File() {

	lines.push_back("");

}

File::File(std::string filename) {

	std::ifstream f = std::ifstream{filename};
	std::string line;
	while (getline(f,line)) lines.push_back(line);
	
	if (lines.empty()) lines.push_back("");

}

File::File(std::vector<std::string> lines, std::shared_ptr<File> next) : lines{lines}, previousVersion{ next } {}

// accessors

std::vector<std::string> &File::getLines() {
	return lines;
}

const std::vector<std::string> &File::getLines() const {
	return lines;
}

bool File::empty() const {

	if (lines.size() == 1 && lines[0] == "") return true;
	return false;

}

std::shared_ptr<File> File::getPrevious() { 
	return previousVersion; 
}

int File::getCurrentLine() { return currentLine; }

void File::setCurrentLine(int curLine) { currentLine = curLine; }

int File::getCurrentChar() { return currentChar; }

void File::setCurrentChar(int curChar) { currentChar = curChar; }
