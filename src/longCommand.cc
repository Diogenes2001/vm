#include "command.h"
#include "editor.h"
#include "file.h"
LongCommand::LongCommand(Editor *editor, std::string c, int multiplicity) : Command{ editor, multiplicity }, command{ c } {

	// add to fmap here
	fmap['r'] = &LongCommand::replace;
	fmap['f'] = &LongCommand::countCharForward;
	fmap['F'] = &LongCommand::countCharBack;
	fmap['d'] = &LongCommand::delLines;
	fmap['y'] = &LongCommand::yankLines;
}

void LongCommand::execute() {

	if (fmap.count(command[0])) {

		FPtr f = fmap[command[0]];
		(this->*f)();

	}

}

char LongCommand::getType() { return command[0]; }

void LongCommand::delLines() { 
	editor->modify();
	Command::delLines(multiplicity); 
	editor->setLastModification(new LongCommand(*this));
}

void LongCommand::yankLines() { Command::yankLines(multiplicity); }

void LongCommand::replace() {

	editor->modify();

	std::vector<std::string> &lines = editor->getFile().getLines();
	int curLineNum = editor->getCurrentLine();
	std::string &curLine = lines[curLineNum];
	
	if (curLine.empty()) return;

	int curChar = editor->getCurrentChar();

	if (command[1] == '\r') {
	       	curLine.erase(curLine.begin() + curChar);
		enter(lines, curLine, curChar, curLineNum);
	} else if (command[1] > 31 && command[1] != 127) curLine[curChar] = command[1];

	editor->setLastModification(new LongCommand(*this));

}

void LongCommand::countCharForward(){
	int multiplicity = this->multiplicity;
	char c = command[1];
        std::string line = editor->getFile().getLines()[editor->getCurrentLine()];
	if(line.length() == 0) return;
        int pos = editor->getCurrentChar();
        line = line.substr(editor->getCurrentChar()+1);
        for(auto x:line){
                ++pos;
                if(x == c) --multiplicity;
                if(multiplicity == 0){
                        editor->setCurrentChar(pos);
                        editor->setMaxCurrentChar(pos);
			editor->setLastFindChar(new LongCommand(*this));
                        return;
                }
        }
	editor->setLastFindChar(new LongCommand(*this));
}

void LongCommand::countCharBack(){ 
	int multiplicity = this->multiplicity;
	char c = command[1];
        std::string line = editor->getFile().getLines()[editor->getCurrentLine()];
        int pos = editor->getCurrentChar();
        line = line.substr(0, editor->getCurrentChar());
        for(auto x = line.rbegin(); x != line.rend(); ++x){
                --pos;
                if(*x == c) --multiplicity;
                if(multiplicity == 0){
                        editor->setCurrentChar(pos);
                        editor->setMaxCurrentChar(pos);
			editor->setLastFindChar(new LongCommand(*this));
                        return;
                }
        }
	editor->setLastFindChar(new LongCommand(*this));
}

