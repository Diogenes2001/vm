#include "command.h"
#include <memory>
#include "editor.h"
#include "file.h"

MoveActionCommand::MoveActionCommand(Editor *editor, int multiplicity, char type, Command *command) 
	: ModeCommand{ editor, type, multiplicity}, command{ std::shared_ptr<Command>{command} } {
	isMove['h'] = true;
        isMove['j'] = true; 
        isMove['k'] = true; 
        isMove['l'] = true;
        isMove['f'] = true;
        isMove['F'] = true;
        isMove['w'] = true;
        isMove['b'] = true;
        isMove['0'] = true;
        isMove['$'] = true; 
        isMove['%'] = true; 
        isMove['^'] = true; 
        isMove['/'] = true;
        isMove['?'] = true;
	isMove[':'] = true;

	isCharWise['h'] = true;
	isCharWise['l'] = true;
	isCharWise['w'] = true;
	isCharWise['b'] = true;
	isCharWise['0'] = true;
	isCharWise['$'] = true;
	isCharWise['f'] = true;
	isCharWise['F'] = true;
	isCharWise['^'] = true;	
	isCharWise['/'] = true;
	isCharWise['?'] = true;
	isCharWise[':'] = true;

	isMode['c'] = &MoveActionCommand::subsInit;

}

bool MoveActionCommand::start() {
	if (isMode.count(type)) {
		FPtr f = isMode[type];
		return (this->*f)();
	}
	return false;
}

bool MoveActionCommand::subsInit() {
	if (isMove.count(command->getType())) {
		bool success = moveInterface();
                if (success) insertInit();
		return success;
        }
	return false;
}

char MoveActionCommand::getType() { return type; }

void MoveActionCommand::execute() {

	if (isMode.count(type)) {
		start();
		repeat();
        	if (!noRepeatEdit.count(type))
                	for (int i = multiplicity; i > 1; --i) repeat();

        	editor->resetStatusBar();
        	int curChar = editor->getCurrentChar();
        	if(curChar > 0){
                	editor->setCurrentChar(curChar - 1);
                	editor->setMaxCurrentChar(curChar - 1);
        	}
	} else {
		moveInterface();
	}
        editor->setLastModification(new MoveActionCommand(*this));

}

void MoveActionCommand::finish() {
	
	if (!noRepeatEdit.count(type)) {
                for (int i = multiplicity; i > 1; --i) {
                        repeat();
                }
        }

        editor->resetStatusBar();

        if (edits.size()) {
                editor->setLastModification(new MoveActionCommand(*this));
        }

        int curChar = editor->getCurrentChar();
        if(curChar > 0){
                editor->setCurrentChar(curChar - 1);
                editor->setMaxCurrentChar(curChar - 1);
        }

}

bool MoveActionCommand::moveInterface() {

	if (!command.get()) return false;

        typedef void (MoveActionCommand::*F)(int);
        typedef void (MoveActionCommand::*F2)(int,int);
        std::map<char,F> linewise;
        std::map<char,F> charwise;
        std::map<char,F> charback;
        std::map<char,F2> linemerge;
	std::map<char,bool> doesModify;

        linewise['d'] = &MoveActionCommand::delLines;
        linewise['c'] = &MoveActionCommand::delLines;
        linewise['y'] = &MoveActionCommand::yankLines;

        charwise['d'] = &MoveActionCommand::del;
        charwise['c'] = &MoveActionCommand::del;
        charwise['y'] = &MoveActionCommand::yank;

        charback['d'] = &MoveActionCommand::delPrev;
        charback['c'] = &MoveActionCommand::delPrev;
        charback['y'] = &MoveActionCommand::yankPrev;

        linemerge['d'] = &MoveActionCommand::delMerge;
        linemerge['c'] = &MoveActionCommand::delMerge;
        linemerge['y'] = &MoveActionCommand::yankMerge;

	doesModify['d'] = true;
	doesModify['c'] = true;

        char move = command->getType();
	bool ret = false;
	bool multLines = false;

        if (isMove.count(move)) {

                if (doesModify.count(type)) editor->modify();

        	int initChar = editor->getCurrentChar();
        	int initLine = editor->getCurrentLine();

		for (int i = multiplicity; i > 0; --i)
			command->execute();

                int newChar = editor->getCurrentChar();
               	int newLine = editor->getCurrentLine();

                if (move == '%') {
			if ((newLine > initLine || (newLine == initLine && newChar > initChar)) && 
					newChar < editor->getCurrentLineLength() + 1) {
                       		++newChar;
                       		editor->setCurrentChar(newChar);
                       		editor->setMaxCurrentChar(newChar);
			} else if ((newLine < initLine || (newLine == initLine && newChar < initChar)) &&
					newChar > 0) {
				++initChar;
			}
                } 
                if (newLine != initLine && move != '%'
                               	&& move != '?' && move != '/' && move != ':' && multiplicity != 0) {
                       	if (newLine < initLine) {
                               	if (linewise.count(type)) (this->*linewise[type])(initLine - newLine + 1);
                       	} else {
                               	editor->setCurrentLine(initLine);
                               	if (linewise.count(type)) (this->*linewise[type])(newLine - initLine + 1);
                       	}
			multLines = true;
			ret = true;
               	} else if (newLine != initLine) {
                       	if (newLine < initLine) {
                               	if (linemerge.count(type)) (this->*linemerge[type])(initLine - newLine + 1, initChar);
                       	} else {
                               	editor->setCurrentLine(initLine);
                               	editor->setCurrentChar(initChar);
                               	editor->setMaxCurrentChar(initChar);
                               	if (linemerge.count(type)) (this->*linemerge[type])(newLine - initLine + 1, newChar);
                       	}
			ret = true;
                } else if(isCharWise.count(move)) {
                    	if (newChar == initChar && move != '%' && (newChar != 0 || move == 'l')) {
                               	if (charwise.count(type)) (this->*charwise[type])(1);
                        } else if (newChar < initChar) {
                               	if (charwise.count(type)) (this->*charwise[type])(initChar - newChar);
                        } else {
                               	if (charback.count(type)) (this->*charback[type])(newChar - initChar);
                        }
			ret = true;
                } else if(!isCharWise.count(move))
			if (linewise.count(move)) (this->*linewise[type])(1);

		if (type == 'c' && multLines) {
			if (!editor->getFile().empty()) {
				int curLine = editor->getCurrentLine();
				bool append = false;
				if (curLine == editor->getFile().getLines().size() - 1) append = true;
		       		insertLine(append);
				if (append) editor->setCurrentLine(curLine + 1);
			}
		}
		
		if (type == 'y' && move != '%' && move != '?') {
			editor->setCurrentLine(initLine);
			editor->setCurrentChar(initChar);
			editor->setMaxCurrentChar(initChar);
		}

        }
	return ret;

}

void MoveActionCommand::del(int mult) {
	Command::del(mult); 
}

void MoveActionCommand::delPrev(int mult) { 
	Command::delPrev(mult); 
}

void MoveActionCommand::delLines(int mult) {
	Command::delLines(mult); 
}

void MoveActionCommand::yankLines(int mult) { 
	Command::yankLines(mult); 
}

void MoveActionCommand::delMerge(int multiplicity, int newChar) {

        int curLine = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        std::vector<std::string> &lines = editor->getFile().getLines();
        std::vector<std::string> delChars;

        std::string &line = lines[curLine];
        std::string l = line;
        delChars.push_back(l.substr(curChar));
        line = line.substr(0, curChar);

        if (multiplicity > 2) {
                delChars.insert(delChars.end(), lines.begin() + curLine + 1, lines.begin() + curLine + multiplicity - 1);
                lines.erase(lines.begin() + curLine + 1, lines.begin() + curLine + multiplicity - 1);
        }

        std::string &newLine = lines[curLine + 1];
        l = newLine;
        delChars.push_back(l.substr(0, newChar));

        line += newLine.substr(newChar);
        lines.erase(lines.begin() + curLine + 1);

        editor->setCopiedLinesCharWise(delChars);

}

void MoveActionCommand::yankPrev(int multiplicity) {

        editor->setCurrentChar(editor->getCurrentChar() - multiplicity);
        editor->setMaxCurrentChar(editor->getCurrentChar() - multiplicity);
        yank(multiplicity * 1);

}

void MoveActionCommand::yankMerge(int multiplicity, int newChar) {

        int curLine = editor->getCurrentLine();
        int curChar = editor->getCurrentChar();
        const std::vector<std::string> &lines = editor->getFile().getLines();
        std::vector<std::string> delChars;

        const std::string &line = lines[curLine];
        delChars.push_back(line.substr(curChar));

        if (multiplicity > 2) {
                delChars.insert(delChars.end(), lines.begin() + curLine + 1, lines.begin() + curLine + multiplicity - 1);
        }

        const std::string &newLine = lines[curLine + multiplicity - 1];
        delChars.push_back(newLine.substr(0, newChar));

        editor->setCopiedLinesCharWise(delChars);

}

void MoveActionCommand::yank(int multiplicity) {
        std::string line = editor->getFile().getLines()[editor->getCurrentLine()];
	if (line.size()) {
        	line = line.substr(editor->getCurrentChar(), multiplicity);
        	editor->setCopiedChars(line);
	}
}
