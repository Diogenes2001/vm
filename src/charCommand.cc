#include "command.h"
#include "editor.h"
#include<deque>
#include "parser.h"
#include<memory>
#include<cctype>
#include "file.h"

CharCommand::CharCommand(Editor *editor, char c, int multiplicity) : Command{ editor, multiplicity }, command{ c } {

	// add to fmap here
	fmap['h'] = &CharCommand::moveLeft;
	fmap['l'] = &CharCommand::moveRight;
	fmap['j'] = &CharCommand::moveDown;
	fmap['k'] = &CharCommand::moveUp;
	fmap['x'] = &CharCommand::del;
	fmap['X'] = &CharCommand::delPrev;
	fmap['J'] = &CharCommand::joinLines;
	fmap['.'] = &CharCommand::repeat;
	fmap['u'] = &CharCommand::undo;
	fmap['w'] = &CharCommand::countWordForward;
	fmap['b'] = &CharCommand::countWordBack;
	fmap['$'] = &CharCommand::goToBack;
	fmap['0'] = &CharCommand::goToFront;
	fmap['p'] = &CharCommand::pasteAfter;
	fmap['P'] = &CharCommand::pasteBefore;
	fmap[';'] = &CharCommand::repeatFindChar;
	fmap['^'] = &CharCommand::firstNonBlank;
	fmap['n'] = &CharCommand::repeatSearch;
	fmap['N'] = &CharCommand::switchRepeatSearch;
	fmap['%'] = &CharCommand::matchBracket;
}

void CharCommand::execute() {

	if (fmap.count(command)) {

		FPtr f = fmap[command];
		return (this->*f)();

	}

}

char CharCommand::getType() { return command; }

void CharCommand::modify() {

	editor->modify();
	editor->setMaxCurrentChar(editor->getCurrentChar());

}

void CharCommand::moveLeft() { 

        int c = editor->getCurrentChar();
        int newC;
        if (c > multiplicity) newC = c - multiplicity;
        else newC = 0;
        editor->setCurrentChar(newC);
        editor->setMaxCurrentChar(newC);

}

void CharCommand::moveRight() { 
	int lineLength = editor->getCurrentLineLength();
        int c = editor->getCurrentChar();

        if (c == lineLength) return;

        int newC;
        if (lineLength - c > multiplicity) newC = c + multiplicity;
        else newC = lineLength;
        editor->setCurrentChar(newC);
        editor->setMaxCurrentChar(newC);
}

void CharCommand::moveUp() { 
        int c = editor->getCurrentLine();
        int newC;
        if (c > multiplicity) newC = c - multiplicity;
        else newC = 0;
        editor->setCurrentLine(newC);

        adjustCurChar();

}

void CharCommand::moveDown() { 
        int lineCount = editor->getFile().getLines().size() - 1;
        int c = editor->getCurrentLine();
        int newC;
        if (lineCount - c > multiplicity) newC = c + multiplicity;
        else newC = lineCount;
        editor->setCurrentLine(newC);

        adjustCurChar();
}

void CharCommand::goToPercent() {

        if (multiplicity <= 100) {
                int size = editor->getFile().getLines().size();
                int lineNum = (multiplicity * size + 99)/100;
                if (lineNum < size) editor->setCurrentLine(lineNum);
                else editor->setCurrentLine(size - 1);
                firstNonBlank();
        }

}

bool CharCommand::match(const std::string &line, int findCase) {

        std::deque<std::string> words = *Parser::parse(line);

        std::map<std::string,bool> start;
        start["if"] = true;
        start["ifdef"] = true;
        start["ifndef"] = true;
        start["else"] = true;
        start["elif"] = true;

        std::map<std::string,bool> cont;
        cont["else"] = true;
        cont["elif"] = true;
        cont["endif"] = true;

        std::map<std::string,bool> begin;
        begin["if"] = true;
        begin["ifndef"] = true;
        begin["ifdef"] = true;

        int index = 0;
        std::string word = words[0];
        while (isspace(word[0]) && index < words.size()) {
                ++index;
                word = words[index];
        }
        if (word != "#") return false;

        ++index;
        word = words[index];
        while (isspace(word[0]) && index < words.size()) {
                ++index;
                word = words[index];
        }
        if (findCase == 0 && start.count(word)) return true;
        if (findCase == 1 && cont.count(word)) return true;
        if (findCase == 2 && begin.count(word)) return true;
        if (findCase == 3 && word == "endif") return true;
        return false;

}

void CharCommand::findOpenComment() {
        int initLineNum = editor->getCurrentLine();
        int initChar = editor->getCurrentChar();

	if (initChar > 0) editor->setCurrentChar(initChar - 1);

	searchBack("*/", 1);
 
        int nextLineNum = editor->getCurrentLine();
        int nextChar = editor->getCurrentChar();

	editor->setCurrentLine(initLineNum);

        searchBack("/*", 1);

        int newLineNum = editor->getCurrentLine();
        int newChar = editor->getCurrentChar();

	if (nextLineNum > initLineNum || (nextLineNum == initLineNum && nextChar >= initChar)) {
		nextLineNum = newLineNum;
		nextChar = newChar;
	}

        if (newLineNum > initLineNum || (initLineNum == newLineNum && initChar < newChar) ||
			newLineNum < nextLineNum || (newLineNum == nextLineNum && nextChar > newChar)) {
                editor->setCurrentLine(initLineNum);
                editor->setCurrentChar(initChar);
                editor->setMaxCurrentChar(initChar);
        }
        editor->resetStatusBar();
}

void CharCommand::findCloseComment() {
        int initLineNum = editor->getCurrentLine();
        int initChar = editor->getCurrentChar();

	searchForward("/*", 1);
 
        int nextLineNum = editor->getCurrentLine();
        int nextChar = editor->getCurrentChar();

	editor->setCurrentLine(initLineNum);
	editor->setCurrentChar(initChar);

        searchForward("*/", 1);

        int newLineNum = editor->getCurrentLine();
        int newChar = editor->getCurrentChar();

	if (nextLineNum < initLineNum || (nextLineNum == initLineNum && nextChar <= initChar + 1)) {
		nextLineNum = newLineNum;
		nextChar = newChar;
	}

        if (newLineNum < initLineNum || (initLineNum == newLineNum && initChar > newChar) ||
			newLineNum > nextLineNum || (newLineNum == nextLineNum && nextChar < newChar)) {
                editor->setCurrentLine(initLineNum);
                editor->setCurrentChar(initChar);
                editor->setMaxCurrentChar(initChar);
        } else {
                editor->setCurrentChar(newChar + 1);
                editor->setMaxCurrentChar(newChar + 1);
        }
        editor->resetStatusBar();
}

void CharCommand::findClose(char c, char f) {

        const std::vector<std::string> &lines = editor->getFile().getLines();
        int curLineNum = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        std::string line = lines[curLineNum];

        int openCount = 0;

        for (int i = curLineNum; i < lines.size(); ++i) {

                line = lines[i];

                for (int j = 0; j < line.size(); ++j) {
                        if (i == curLineNum && j == 0) j = curChar + 1;

                        if (line[j] == c) ++openCount;
                        else if (line[j] == f && openCount) --openCount;
                        else if (line[j] == f) {
                                editor->setCurrentLine(i);
                                editor->setCurrentChar(j);
                                editor->setMaxCurrentChar(j);
                                return;
                        }

                }

        }

}

void CharCommand::findOpen(char c, char f) {

        const std::vector<std::string> &lines = editor->getFile().getLines();
        int curLineNum = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        std::string line = lines[curLineNum];

        int closeCount = 0;

        for (int i = curLineNum; i >= 0; --i) {

                line = lines[i];

                for (int j = line.size() - 1; j >= 0; --j) {
                        if (i == curLineNum && j == line.size() - 1) j = curChar - 1;

                        if (line[j] == c) ++closeCount;
                        else if (line[j] == f && closeCount) --closeCount;
                        else if (line[j] == f) {
                                editor->setCurrentLine(i);
                                editor->setCurrentChar(j);
                                editor->setMaxCurrentChar(j);
                                return;
                        }

                }

        }

}

void CharCommand::matchBracket() { 
	if (multiplicity != 0) goToPercent();
        else {
                multiplicity = 1;

                std::vector<std::string> lines = editor->getFile().getLines();
                int curLine = editor->getCurrentLine();
                std::string line = lines[curLine];
                int curChar = editor->getCurrentChar();
                char c = line[curChar];

                std::map<char,char> openParens;
                openParens['('] = ')';
                openParens['['] = ']';
                openParens['{'] = '}';

                std::map<char,char> closeParens;
                closeParens['}'] = '{';
                closeParens[']'] = '[';
                closeParens[')'] = '(';

                if (openParens.count(c)) {
                        findClose(c, openParens[c]);
                } else if (closeParens.count(c)) {
                        findOpen(c, closeParens[c]);
                } else if (line[firstNonWS(line)] == '#') {
                        int index = firstNonWS(line);

                        // #if #ifdef #ifndef #else #elif #endif
                        if (match(line, 0)) {
                                // find first out of #else, #elif, #endif
                                for (int i = curLine + 1; i < lines.size(); ++i) {
                                        index = firstNonWS(lines[i]);
                                        if (match(lines[i], 1)) {
                                                editor->setCurrentLine(i);
                                                editor->setCurrentChar(index);
                                                editor->setMaxCurrentChar(index);
                                                return;
                                        }
                                }
                        } else if (match(line, 3)) {
                                // find previous #if / #ifndef / #ifdef
                                for (int i = curLine - 1; i >= 0; --i) {
                                        index = firstNonWS(lines[i]);
                                        if (match(lines[i], 2)) {
                                                editor->setCurrentLine(i);
                                                editor->setCurrentChar(index);
                                                editor->setMaxCurrentChar(index);
                                                return;
                                        }
                                }
                        }
                }
                else if (c == '/') {
                        // check if beginnning or end of comment
                        if (curChar < line.size() - 1 && line[curChar + 1] == '*') findCloseComment();
                        else if (curChar > 0 && line[curChar - 1] == '*') findOpenComment();
                } else if (c == '*') {
                        // check if beginning or end of comment
                        if (curChar < line.size() - 1 && line[curChar + 1] == '/') findOpenComment();
                        else if (curChar > 0 && line[curChar - 1] == '/') findCloseComment();
                }

        }

}

void CharCommand::del() {

	modify();

	Command::del(multiplicity);
        editor->setLastModification(new CharCommand(*this));
}

void CharCommand::delPrev() {

	modify();

	Command::delPrev(multiplicity);
        editor->setLastModification(new CharCommand(*this));

}

void CharCommand::joinLines() {
	
	modify();
	
	int curLineNum = editor->getCurrentLine();
	int curChar = editor->getCurrentChar();
	std::vector<std::string> &lines = editor->getFile().getLines();
	std::string &curLine = lines[curLineNum];
	for (int i = multiplicity; i > 0; --i) {
		if (curLineNum + 1 < lines.size()) {
			std::string add = lines[curLineNum + 1].substr(firstNonWS(lines[curLineNum + 1]));
			if (curLine.size() && curLine.back() != ' ') {
				add = " " + add;
			}
			if (curLine.size()) curChar = curLine.size() - 1;
			if (add.size()) {
				if (curLine.size()) ++curChar;
				curLine += add;
			}
			lines.erase(lines.begin() + curLineNum + 1);
		} else {
			break;
		}
	}
	editor->setCurrentChar(curChar);
	editor->setMaxCurrentChar(curChar);
        editor->setLastModification(new CharCommand(*this));
}

void CharCommand::repeat() {

	Command *command = editor->getLastModification();
	if (!command) return;
	if (multiplicity) command->setMultiplicity(multiplicity);
	command->execute();

}

void CharCommand::repeatFindChar() {
	LongCommand *command = editor->getLastFindChar();
	if (!command) return;
	if (multiplicity) command->setMultiplicity(multiplicity);
	command->execute();
}

void CharCommand::repeatSearch() {

	StatusBarCommand *command = editor->getLastSearch();
	if (!command) return;
	if (multiplicity) command->setMultiplicity(multiplicity);
	command->execute();

}

void CharCommand::switchRepeatSearch() {

	StatusBarCommand *command = editor->getLastSearch();
	if (!command) return;
	if (multiplicity) command->setMultiplicity(multiplicity);
	char type = command->getType();
	if (type == '/') command->setType('?');
	else command->setType('/');
	command->execute();

}

void CharCommand::undo() {

	File &file = editor->getFile();
	
	if (file.getPrevious()) {
		
		std::shared_ptr<File> temp = file.getPrevious();
		
		for (int i = multiplicity; i > 1; --i) {
			if (temp->getPrevious().get()) temp = temp->getPrevious();
		}
		editor->load(temp);
	
	} else {
		editor->setStatusBar("Already at oldest change");
	}
}

void CharCommand::countWordForward(){ 
	int mult = multiplicity;
        int pos = editor->getCurrentChar();
        int curLine = editor->getCurrentLine();
        int maxLine = editor->getFile().getLines().size();
        int firstword = true;
        if(curLine == maxLine) return;
        while(mult > 0 && curLine < maxLine){
                std::string str = editor->getFile().getLines()[curLine];
                if(curLine == editor->getCurrentLine()){
                        str = str.substr(pos);
                }else{
                        pos = 0;
                }
		std::unique_ptr<std::deque<std::string>> q = Parser::parse(str);
                std::string word;
                while(q->size() > 0 && mult > 0){
                        word = q->front();
                        q->pop_front();
                        pos += word.length();
                        if(!isspace(word[0]) || firstword) --mult;
                        firstword = false;
                }
                if(mult <= 0 && q->size() &&  isspace(q->front()[0])){
                                pos += q->front().length();
                                q->pop_front();
                }
		++curLine;
	}
        if(!(curLine == maxLine) && pos == editor->getFile().getLines()[curLine-1].length()){
                ++curLine;
                pos = 0;
                std::string line = editor->getFile().getLines()[curLine-1];
                while(isspace(line[0])){
                        ++pos;
                        line = line.substr(1);
                        if(line.length() == 0){
                                ++curLine;
                                line = editor->getFile().getLines()[curLine-1];
                                pos = 0;
                        }
                }
        
	}
	
	if(curLine == maxLine-1 && pos == editor->getFile().getLines()[curLine-1].length()) --pos;
        
	editor->setCurrentLine(curLine-1);
        editor->setCurrentChar(pos);
        editor->setMaxCurrentChar(pos);
}

void CharCommand::countWordBack(){ 
	int mult = multiplicity;
        int pos = editor->getCurrentChar();
        int curLine = editor->getCurrentLine();
        while(mult > 0 && curLine >= 0){
                std::string str = editor->getFile().getLines()[curLine];
                if(curLine == editor->getCurrentLine()){
                        str = str.substr(0, pos);
                }else{
                        pos = str.length() -1;
			if(firstNonWS(str) < str.length() || pos == -1) --mult;
                }
			std::unique_ptr<std::deque<std::string>> q = Parser::parse(str);
	                std::string word;
			while(q->size() > 0 && mult > 0){
                        word = q->back();
                        q->pop_back();
                        if(word.length()>0){
                                pos -= word.length();
                                if(!isspace(word[0])) --mult;
                        }
                	}
                --curLine;
        }
        std::string line = editor->getFile().getLines()[curLine+1];
        if(!(curLine == -1) && pos == line.length() - 1){
                
		if(isspace(line[pos])){
		 	while(isspace(line[pos])){
                        	--pos;
                        	if(pos <= 0){
                            	    	--curLine;
                                	line = editor->getFile().getLines()[curLine+1];
                        	        pos = line.length()-1;
                        	}
                	}
		}
		 int type = Parser::getType(line[pos]);
		while(Parser::getType(line[pos]) == type && pos >= 0){
                        --pos;
                }

                ++pos;

        }
        editor->setCurrentLine(std::max(curLine+1, 0));
        editor->setMaxCurrentChar(std::max(pos, 0));
        editor->setCurrentChar(std::max(pos, 0));
}

void CharCommand::goToFront(){ 
	editor->setCurrentChar(0);
        editor->setMaxCurrentChar(0);
}

void CharCommand::goToBack(){ 
	editor->setCurrentLine(std::min(static_cast<int>(editor->getFile().getLines().size())-1, editor->getCurrentLine()+multiplicity-1));
        editor->setCurrentChar(editor->getFile().getLines()[editor->getCurrentLine()].length()-1);
        editor->setMaxCurrentChar(editor->getFile().getLines()[editor->getCurrentLine()].length()-1);
}

void CharCommand::paste(bool after) {

	modify();

        bool didCopyMultipleLines = editor->getDidCopyMultipleLines();
	bool charWise = editor->getCharWise();
        int lineNum = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        std::vector<std::string> &lines = editor->getFile().getLines();
	std::string &line = lines[lineNum];
	if (after && line.size()) {
		++curChar;
		editor->setCurrentChar(curChar);
		editor->setMaxCurrentChar(curChar);
	}

        if (didCopyMultipleLines) {
                const std::vector<std::string> &copiedLines = editor->getCopiedLines();
		if (copiedLines.empty()) return;
		if (after && !charWise) {
			++lineNum;
			editor->setCurrentLine(lineNum);
		}
		for (int i = multiplicity; i > 0; --i) {
			if (!charWise) lines.insert(lines.begin() + lineNum, copiedLines.begin(), copiedLines.end());
			else {
				line = lines[lineNum];
				std::string rest = line;
				rest = rest.substr(curChar);
				std::string first = copiedLines[0];
				line = line.substr(0,curChar);
				line.insert(line.begin() + curChar, first.begin(), first.end());
				lines.insert(lines.begin() + lineNum + 1, copiedLines.begin() + 1, copiedLines.end());
				std::string &last = lines[lineNum + copiedLines.size() - 1];
				last += rest;
			}
		}
		if (!charWise) firstNonBlank();
        } else {
                const std::string &copiedChars = editor->getCopiedChars();
		if (copiedChars.empty()) return;
		for (int i = multiplicity; i > 0; --i)
			line.insert(line.begin() + curChar, copiedChars.begin(), copiedChars.end());
		editor->setCurrentChar(curChar - 1 + multiplicity * copiedChars.size());
		editor->setMaxCurrentChar(curChar - 1 + multiplicity * copiedChars.size());
        }
        editor->setLastModification(new CharCommand(*this));

}

void CharCommand::pasteAfter() { paste(true); }

void CharCommand::pasteBefore() { paste(false); }

void CharCommand::firstNonBlank() {
	std::string &line = editor->getFile().getLines()[editor->getCurrentLine()];
        int first = firstNonWS(line);
        int lineLength = editor->getCurrentLineLength();
        if (first > lineLength) {
                editor->setCurrentChar(lineLength);
                editor->setMaxCurrentChar(lineLength);
        } else {
                editor->setCurrentChar(first);
                editor->setMaxCurrentChar(first);
        }
}
