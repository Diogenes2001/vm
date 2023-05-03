#include "command.h"
#include "editor.h"
#include <cctype>
#include "file.h"
// implementation of command
using std::unique_ptr;
Command::Command(Editor *editor, int multiplicity) : editor{ editor }, multiplicity{ multiplicity } {
	
};

void Command::setMultiplicity(int mult) { multiplicity = mult; }

// implement modify and other things that are required globally

void Command::enter(std::vector<std::string> &lines, std::string &curLine, int curChar, int curLineNum) {

        std::string s = curLine;

        if (curChar < static_cast<int>(curLine.size())) {
                curLine.erase(curChar);
        }

        lines.insert(lines.begin() + curLineNum + 1, s.substr(curChar));

        editor->setCurrentLine(curLineNum + 1);
        editor->setCurrentChar(0);
	editor->setMaxCurrentChar(0);

}

int Command::firstNonWS(const std::string &s) { // returns size of string if only whitespace

	for (size_t i = 0; i < s.size(); ++i) {
		if (!isspace(s[i])) return i;
	}

	return s.size();

}

void Command::del(int multiplicity) {
	int curLine = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        std::string &line = editor->getFile().getLines()[curLine];
	std::string delLine;

	if (line.size()) {
	       	delLine.assign(line, curChar, multiplicity);
        	line.erase(curChar, multiplicity);
	}
	if (curChar == static_cast<int>(line.size()) && curChar != 0) {
		editor->setCurrentChar(curChar - 1);
		editor->setMaxCurrentChar(curChar - 1);
	}

	editor->setCopiedChars(delLine);
}

void Command::delPrev(int multiplicity) {
	int curLine = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        std::string &line = editor->getFile().getLines()[curLine];
	std::string delLine;

        int start;
        if (curChar - multiplicity > 0) {
                start = curChar - multiplicity;
		delLine.assign(line, start, multiplicity);
                line.erase(start, multiplicity);
                editor->setCurrentChar(start);
		editor->setMaxCurrentChar(start);
        } else {
		delLine = line.substr(0, curChar);
                line = line.substr(curChar);
                editor->setCurrentChar(0);
		editor->setMaxCurrentChar(0);
        }
	editor->setCopiedChars(delLine);
}

void Command::adjustCurChar() {

        int lineLength = editor->getCurrentLineLength();
        int curChar = editor->getCurrentChar();
        int maxCurChar = editor->getMaxCurrentChar();

	if (lineLength < 0) lineLength = 0;

        if (curChar > lineLength) editor->setCurrentChar(lineLength);
        else if (maxCurChar > curChar) {
                if (maxCurChar > lineLength) editor->setCurrentChar(lineLength);
                else editor->setCurrentChar(maxCurChar);
        }

}

void Command::reset() {

	std::vector<std::string> &lines = editor->getFile().getLines();
	lines.clear();
        lines.push_back("");
        editor->setCurrentLine(0);
       	editor->setCurrentChar(0);
        editor->setMaxCurrentChar(0);

}

void Command::insertLine(bool append) {
	if (!editor->getFile().empty()) {
		std::vector<std::string> &lines = editor->getFile().getLines();
                if (append) 
			lines.push_back("");
		else
			lines.insert(lines.begin() + editor->getCurrentLine(), "");
                editor->setCurrentChar(0);
                editor->setMaxCurrentChar(0);
	}
}

void Command::delLines(int multiplicity) {

	int curLine = editor->getCurrentLine();
        std::vector<std::string> &lines = editor->getFile().getLines();
	std::vector<std::string> delLines;

        if (curLine + multiplicity < static_cast<int>(lines.size())) {
		delLines.assign(lines.begin() + curLine, lines.begin() + curLine + multiplicity);
                lines.erase(lines.begin() + curLine, lines.begin() + curLine + multiplicity);
        } else if (curLine != 0) {
                delLines.assign(lines.begin() + curLine, lines.end());
		lines.erase(lines.begin() + curLine, lines.end());
                editor->setCurrentLine(curLine - 1);
        } else {
		delLines.assign(lines.begin(), lines.end());
                reset();
        }

	editor->setCopiedLines(delLines);
	adjustCurChar();

}

//counts occurrences of a string in a line
int countPatterns(std::string str, std::string pattern){
        size_t x = str.find(pattern);
        if(x!=std::string::npos) return countPatterns(str.substr(x+1), pattern)+1;
        return 0;
}

//returns index to the nth occurence of a string in a line
int findnth(std::string str, std::string pattern, int n){
        size_t x = str.find(pattern);
        if (x==std::string::npos) return -1;
        if (n==1) return x;
        return x + 1 + findnth(str.substr(x+1), pattern, n-1);
}


//searches from the top for a pattern
void Command::searchFromTop(std::string command, int mult){
	const int oldmult = mult; 
        int temp = mult;
        std::vector<std::string> vec = editor->getFile().getLines();
        int curLine = 0;
        int pos = 0;
        for(size_t i = curLine; i < vec.size(); ++i){
                std::string str = vec[i];
                temp = mult;
                mult -= countPatterns(str, command);
                if(mult <= 0){
                        int nth = findnth(str, command, temp);
                        editor->setCurrentChar(pos + nth);
                        editor->setMaxCurrentChar(pos + nth);
                        editor->setCurrentLine(i);
                        return;
                }
                pos = 0;
        }
        editor->setStatusBar("search hit BOTTOM, continuing at TOP");
        if(mult < oldmult) {
                return searchFromTop(command, mult);
        }
        editor->setStatusBar("Pattern not found: " + command);
        return;
}


//searches forward for a pattern
void Command::searchForward(std::string command, int mult){
        int temp = mult;
        std::vector<std::string> vec = editor->getFile().getLines();
        int curLine = editor->getCurrentLine();
        int pos = editor->getCurrentChar();
        for(size_t i = curLine; i < vec.size(); ++i){
                std::string str = vec[i];
                if (i==curLine && str.length() > 0) {
                        str = str.substr(pos+1);
			if(pos < static_cast<int>(str.length()) - 1) ++pos;
                }
                temp = mult;
                mult -= countPatterns(str, command);
                if(mult <= 0){
			int nth = findnth(str, command, temp);
                        editor->setCurrentChar(pos + nth);
                        editor->setMaxCurrentChar(pos + nth);
                        editor->setCurrentLine(i);
                        return;
                }
                pos = 0;
        }
        editor->setStatusBar("search hit BOTTOM, continuing at TOP");
        return searchFromTop(command, mult);
}
//searches from the bottom for a pattern
void Command::searchFromBottom(std::string command, int mult){
        const int oldmult = mult;
        int temp = mult;
        std::vector<std::string> vec = editor->getFile().getLines();
        int curLine = vec.size() - 1;
        int pos = vec[curLine].length()-1;
        for(int i = curLine; i >= 0; --i){
                std::string str = vec[i];
                if (i==curLine) {
                        str = str.substr(0, pos+command.length()-1);
                }
                temp = mult;
                int x = countPatterns(str, command);
                mult -= x;
                if(mult <= 0){
                        int nth = findnth(str, command, x-temp+1);
                        editor->setCurrentChar(nth);
                        editor->setMaxCurrentChar(nth);
                        editor->setCurrentLine(i);
                        return;
                }
                pos = 0;
        }
        if(mult < oldmult) {
                return searchFromBottom(command, mult);
        }
        editor->setStatusBar("Pattern not found: " + command);
        return;
}


//search backwards for a string
void Command::searchBack(std::string command, int mult){
        int temp = mult;
        std::vector<std::string> vec = editor->getFile().getLines();
        int curLine = editor->getCurrentLine();
        int pos = editor->getCurrentChar();
        for(int i = curLine; i >= 0; --i){
                std::string str = vec[i];
                if (i==curLine) {
                        str = str.substr(0, pos+command.length()-1);
                }
                temp = mult;
                int x = countPatterns(str, command);
                mult -= x;
                if(mult <= 0){
			int nth = findnth(str, command, x-temp+1);
                        editor->setCurrentChar(nth);
                        editor->setMaxCurrentChar(nth);
                        editor->setCurrentLine(i);
                        return;
                }
                pos = 0;
        }
        editor->setStatusBar("search hit TOP, continuing at BOTTOM");
        return searchFromBottom(command, mult);
}

void Command::yankLines(int multiplicity) {

        std::vector<std::string> lines = editor->getFile().getLines();
        int curLineNum = editor->getCurrentLine();
	int end = curLineNum + multiplicity;
	if (curLineNum + multiplicity > lines.size()) end = lines.size();
        lines.assign(lines.begin() + curLineNum, lines.begin() + end);
        editor->setCopiedLines(lines);

}
