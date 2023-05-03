#include "command.h"
#include "editor.h"
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <algorithm>
#include "file.h"
StatusBarCommand::StatusBarCommand(Editor *editor, std::string command, int mult) : Command{ editor, mult }, command{ command } {

	// add to fmap here
	fmap[":"] = &StatusBarCommand::colonCommand;	
	fmap["w"] = &StatusBarCommand::save;
	fmap["q"] = &StatusBarCommand::quit;
	fmap["wq"] = &StatusBarCommand::saveQuit;
	fmap["q!"] = &StatusBarCommand::forceQuit;
	fmap["/"] = &StatusBarCommand::searchForward;
	fmap["?"] = &StatusBarCommand::searchBack;
	fmap["r"] = &StatusBarCommand::insertFile;

}

char StatusBarCommand::getType() { 
	if (command.size()) return command[0];
	else return 0;
}

void StatusBarCommand::setType(char c) { command[0] = c; }

void StatusBarCommand::execute() {

	if (command.empty()) return;

	editor->resetStatusBar();
	if (fmap.count(command.substr(0,1))) {
		return (this->*fmap[command.substr(0,1)])();

	}

}

bool isNum(const std::string &str){
	for(auto x:str){
		if(!(std::isdigit(x))) return false;
	}
	return true;
}

void StatusBarCommand::colonCommand(){
	command = command.substr(1);
	if(command == "color"){
		editor->setStatusBar("Toggling color mode");
		editor->toggleColor();
		return;
	}
	int x = command.find(' ');
	std::string str;
	if(x >= 0){
		str = command.substr(0, x);
		command = command.substr(x+1);
	}else{
		str = command;
		command = "";
	}
	if(isNum(str) || str=="$") return goToLine(str);
	if(fmap.count(str)){
                return (this->*fmap[str])();
	}
	editor->setStatusBar("Not an editor command: " + str);
}
//goes to a line
void StatusBarCommand::goToLine(const std::string &str){
	if(command.length() > 0){
                editor->setStatusBar("Trailing characters");
		return;
        }
	int lastline = editor->getFile().getLines().size() -1;
	if(str == "$"){
	       	editor->setCurrentLine(lastline);
	}else{
		int i;
		try{
			i = std::stol(str)-1;
		}catch(std::out_of_range){
			i = lastline;
		}
		if (i==-1) i=0;
		editor->setCurrentLine(std::min(i, lastline));
	}
	editor->setCurrentChar(0);
        editor->setMaxCurrentChar(0);

	return;

}


//searches forward for a pattern
void StatusBarCommand::searchForward(){ 
	if (command.size() > 1) 
		Command::searchForward(command.substr(1), multiplicity); 
	editor->setLastSearch(new StatusBarCommand(*this));
}

//searches back for a pattern
void StatusBarCommand::searchBack(){ 
	if (command.size() > 1)
		Command::searchBack(command.substr(1), multiplicity); 
	editor->setLastSearch(new StatusBarCommand(*this));
}

//saves
void StatusBarCommand::save(){
	if(command.length() > 0) editor->getFilename() = command;
	std::ofstream out{(editor->getFilename()).c_str()};
	std::vector<std::string> vec = editor->getFile().getLines();
	for(auto x:vec){
		out << x << '\n';
	}
	editor->getIsSaved() = true;
}

//quits
void StatusBarCommand::quit(){
	if(command.length() > 0){
		editor->setStatusBar("Trailing characters");
	}else if(editor->getIsSaved()){
		editor->getShouldClose() = true;
	}else{
		editor->setStatusBar("No write since last change (add ! to override)");
	}
}

//saves and quits
void StatusBarCommand::saveQuit(){
	if(command.length() > 0){
                editor->setStatusBar("Trailing characters");
        }else{
		save();
		quit();
	}
}

//quits without saving
void StatusBarCommand::forceQuit(){
        if(command.length() > 0){
                editor->setStatusBar("Trailing characters");
        }else{
                editor->getShouldClose() = true;
        }
}

void StatusBarCommand::insertFile() {
	std::string filename;
	if (command.length() > 0) filename = command;
	else filename = editor->getFilename();

	std::ifstream file{ filename };

	if (!file.fail()) {

		editor->modify();
		
		int lineNum = editor->getCurrentLine() + 1;
		std::vector<std::string> &lines = editor->getFile().getLines();

		std::string line;

		while (getline(file, line)) {
			lines.insert(lines.begin() + lineNum, line);
			++lineNum;
		}
		
	} else {
		editor->setStatusBar("Can't open file " + filename);
	}

}

