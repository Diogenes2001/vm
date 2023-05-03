#include "command.h"
#include "editor.h"
#include <string>
#include <ncurses.h>
#include <algorithm>
#include <sstream>
#include "file.h"
CtrlCommand::CtrlCommand(Editor *editor, int type, int mult) : Command{ editor, mult }, type{ type } {
        // add to fmap here
        fmap[2] = &CtrlCommand::pageUp;
        fmap[4] = &CtrlCommand::scrollDown;
        fmap[6] = &CtrlCommand::pageDown;
        fmap[7] = &CtrlCommand::printInfo;
        fmap[21] = &CtrlCommand::scrollUp;

}

void CtrlCommand::execute() {
        if (fmap.count(type)) {
                return (this->*fmap[type])();

        }
}

char CtrlCommand::getType() { return type; }

void CtrlCommand::printInfo(){
	std::string toDisplay = "";

	toDisplay.append("\"");
       	toDisplay.append(editor->getFilename());
	toDisplay.append("\" ");

	std::string isSaved = editor->getIsSaved() ? "" : "[Modified] ";
	toDisplay.append(isSaved);

	int lines = editor->getFile().getLines().size();
	std::ostringstream ostr1;
	ostr1 << lines;
	toDisplay.append(ostr1.str());
	
	toDisplay.append(" lines --");	
        int curline = editor->getCurrentLine();
	int frac = (curline*100)/lines;
	std::ostringstream ostr2;
	ostr2 << frac;
        toDisplay.append(ostr2.str());
	toDisplay.append("%%--");


	editor->setStatusBar(toDisplay);
}

void CtrlCommand::pageUp(){
	do{
		editor->setCurrentLine(std::max(editor->getCurrentLine() - LINES, 0));
                --multiplicity;
	}while(multiplicity > 0);
	adjustCurChar();
}

void CtrlCommand::pageDown(){
	do{
		editor->setCurrentLine(std::min(editor->getCurrentLine() + LINES, static_cast<int>(editor->getFile().getLines().size())- 1));
                --multiplicity;
	}while(multiplicity > 0);
	adjustCurChar();
}

void CtrlCommand::scrollUp(){
	if(multiplicity > 0) editor->getScrollDownDefault() = multiplicity;
        editor->setCurrentLine(std::max(editor->getCurrentLine() - editor->getScrollUpDefault(), 0));
	adjustCurChar();
}
void CtrlCommand::scrollDown(){
        if(multiplicity > 0) editor->getScrollUpDefault() = multiplicity;
        editor->setCurrentLine(std::min(editor->getCurrentLine() + editor->getScrollUpDefault(), static_cast<int>(editor->getFile().getLines().size())-1));
	adjustCurChar();
}

