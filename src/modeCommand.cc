#include "command.h"
#include "editor.h"
#include <vector>
#include <string>
#include "file.h"
ModeCommand::ModeCommand(Editor *editor, char type, int multiplicity) 
	: Command{ editor, multiplicity }, type{ type } {

	// add to fmap here
	fmap['i'] = &ModeCommand::insert;
	fmap['I'] = &ModeCommand::insert;
	fmap['R'] = &ModeCommand::replace;
	fmap['a'] = &ModeCommand::insert;
	fmap['o'] = &ModeCommand::insert;
	fmap['O'] = &ModeCommand::insert;
	fmap['A'] = &ModeCommand::insert;
	fmap['s'] = &ModeCommand::insert;
	fmap['S'] = &ModeCommand::insert;
	fmap['c'] = &ModeCommand::insert;
	
	// add to smap here
	imap['i'] = &ModeCommand::insertInit;
	imap['I'] = &ModeCommand::insWSInit;
	imap['R'] = &ModeCommand::replaceInit;
	imap['a'] = &ModeCommand::appendInit;
	imap['A'] = &ModeCommand::appendEndInit;
	imap['o'] = &ModeCommand::newLineInit;
	imap['O'] = &ModeCommand::newLineAboveInit;
	imap['s'] = &ModeCommand::delInit;
	imap['S'] = &ModeCommand::subInit;
	imap['c'] = &ModeCommand::subInit;

	// add to repeatInit here
	repeatInit['o'] = true;
	repeatInit['O'] = true;

	// add to noRepeatEdit here
	noRepeatEdit['s'] = true;
	noRepeatEdit['S'] = true;
	noRepeatEdit['c'] = true;
}

bool ModeCommand::insertInit() { editor->setStatusBar("-- INSERT --"); return true; }

bool ModeCommand::insWSInit() { 
	std::string &s = editor->getFile().getLines()[editor->getCurrentLine()];
	editor->setCurrentChar(firstNonWS(s));
	insertInit();
	return true;
}

bool ModeCommand::replaceInit() { editor->setStatusBar("-- REPLACE --"); return true; }

bool ModeCommand::appendInit() {
        int curChar = editor->getCurrentChar();
	if (curChar <= editor->getCurrentLineLength()) 
		editor->setCurrentChar(editor->getCurrentChar() + 1);
	insertInit();
	return true;
}

bool ModeCommand::appendEndInit() {
	editor->setCurrentChar(editor->getCurrentLineLength() + 1);
	insertInit();
	return true;
}

bool ModeCommand::newLineInit() {
	std::vector<std::string> &lines = editor->getFile().getLines();
	int curLineNum = editor->getCurrentLine();
	std::string s;
	enter(lines, s, 0, curLineNum);
	insertInit();
	return true;
}

bool ModeCommand::newLineAboveInit() {
	std::vector<std::string> &lines = editor->getFile().getLines();
	int curLineNum = editor->getCurrentLine();
	std::string &curLine = lines[curLineNum];
	enter(lines, curLine, 0, curLineNum);
	editor->setCurrentLine(editor->getCurrentLine() - 1);
	insertInit();
	return true;
}

bool ModeCommand::delInit() { 
	Command::del(multiplicity);
	int curChar = editor->getCurrentChar();
	if (curChar == editor->getCurrentLineLength()) editor->setCurrentChar(curChar + 1);
	insertInit();
	return true;
}

bool ModeCommand::subInit() {
	Command::delLines(multiplicity);

	if (editor->getFile().empty()) {
		insertInit();
		return true;
	}

	int curLine = editor->getCurrentLine();
	bool append = false;
	if (curLine == editor->getFile().getLines().size() - 1) append = true;
	insertLine(append);
	if (append) editor->setCurrentLine(curLine + 1);

	insertInit();
	return true;
}

void ModeCommand::repeat() {

	if (repeatInit.count(type)) start();

	for (auto c : edits) {
                if (fmap.count(type)) {
                        FPtr f = fmap[type];
                        (this->*f)(c);
                }
        }
}

void ModeCommand::execute() {
	start();

	for (auto c : edits) {
                if (fmap.count(type)) {
                        FPtr f = fmap[type];
                        (this->*f)(c);
                }
        }

	if (!noRepeatEdit.count(type))
		for (int i = multiplicity; i > 1; --i) repeat();

	editor->resetStatusBar();
	int curChar = editor->getCurrentChar();
	if(curChar > 0){
		editor->setCurrentChar(curChar - 1);
		editor->setMaxCurrentChar(curChar - 1);
	}

}

char ModeCommand::getType() { return type; }

void ModeCommand::edit(int c) {

	edits.push_back(c);

	if (fmap.count(type)) {

		FPtr f = fmap[type];
		(this->*f)(c);

	}

}

bool ModeCommand::start() {

	editor->modify();
	
	if (imap.count(type)) {
		
		InitF f = imap[type];
		return (this->*f)();

	}
	
	return true;
}

void ModeCommand::finish() {

	if (!noRepeatEdit.count(type)) {
		for (int i = multiplicity; i > 1; --i) {
			repeat();
		}
	}

	editor->resetStatusBar();
	
	if (edits.size()) {
		editor->setLastModification(new ModeCommand(*this));
	}

	int curChar = editor->getCurrentChar();
	if(curChar > 0){
		editor->setCurrentChar(curChar - 1);
		editor->setMaxCurrentChar(curChar - 1);
	}

}

void ModeCommand::backspace(std::vector<std::string> &lines, std::string &curLine, int curChar, int curLineNum) {
	
	if (curChar == 0 && curLineNum != 0) { // remove beginning of line

        	std::string &prevLine = lines[curLineNum - 1];

                editor->setCurrentChar(prevLine.size());
                editor->setCurrentLine(curLineNum - 1);

                prevLine += curLine;
               	lines.erase(lines.begin() + curLineNum);

        } else if (curChar != 0) {

                curLine.erase(curLine.begin() + curChar - 1);
                editor->setCurrentChar(curChar - 1);

       	}

}

void ModeCommand::insert(int i) {
	
	std::vector<std::string> &lines = editor->getFile().getLines();
	int curLineNum = editor->getCurrentLine();
	std::string &curLine = lines[curLineNum];
	int curChar = editor->getCurrentChar();
	
	if (i == 127) {
		//backspace
		backspace(lines, curLine, curChar, curLineNum);
	
	} else if (i == 13) {
		//enter
		enter(lines, curLine, curChar, curLineNum);

	} else {
		char c = i;
		curLine.insert(curLine.begin() + curChar, c);
		editor->setCurrentChar(curChar + 1);
	}

}

void ModeCommand::replace(int i) {

	std::vector<std::string> &lines = editor->getFile().getLines();
	int curLineNum = editor->getCurrentLine();
        std::string &curLine = lines[curLineNum];
        int curChar = editor->getCurrentChar();

        if (i == 127) {
                //backspace
                if (curChar > 0) editor->setCurrentChar(curChar - 1);
		else if (curLineNum > 0) {
			editor->setCurrentLine(curLineNum - 1);
			int newChar = editor->getCurrentLineLength();
			if (newChar == -1) newChar = 0;
			editor->setCurrentChar(newChar);
		}

        } else if (i == 13) {
                //enter
                enter(lines, curLine, curChar, curLineNum);

        } else {
                if (curChar < curLine.size()) curLine[curChar] = i;
		else curLine.push_back(i);
		editor->setCurrentChar(curChar + 1);
        }

}
