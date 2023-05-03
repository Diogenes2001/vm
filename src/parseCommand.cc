#include <string>
#include "state.h"
#include "parseCommand.h"

ParseCommand::ParseCommand() : Model{} {

	// initialize command maps
	needMode['i'] = true;
	needMode['I'] = true;
	needMode['R'] = true;
	needMode['a'] = true;
	needMode['A'] = true;
	needMode['o'] = true;
	needMode['O'] = true;
	needMode['s'] = true;
	needMode['S'] = true;
	needMode['c'] = true;

	needFeed[':'] = true;
	needFeed['/'] = true;
	needFeed['?'] = true;
	
	isLong['r'] = true;
	isLong['f'] = true;
	isLong['F'] = true;

	needSecond['c'] = true;
	needSecond['d'] = true;
	needSecond['y'] = true;

	zeroMultiplicity[21] = true;
	zeroMultiplicity[4] = true;
	zeroMultiplicity['%'] = true;
	zeroMultiplicity['.'] = true;
	zeroMultiplicity[';'] = true;
	zeroMultiplicity['n'] = true;
	zeroMultiplicity['N'] = true;

}

void ParseCommand::doMacro(char c) {
	for(auto &t : macros[c]) t->execute();
}

void ParseCommand::inMode(ModeCommand *c) {

	if (!c) { 
		return; 
	}

	if (!c->start()) return;
	
	updateViews();

	int i = getAction();
	
	while(i != 27) {

		if (i > 31 || i == 13 || i == 9) { // don't pass any control buttons to modes
			c->edit(i);
			updateViews();
		}
		
		i = getAction();

	}

	c->finish();

}

std::string ParseCommand::feed(char c) {

	std::string s;
	s.push_back(c);
	editor.setStatusBar(s);
	int x = 0;

	updateViews();

	//27 is escape, 127 is delete, 13 is enter. may have to change this for Macs
	while(x != 27 && x!=13){
		x = getAction();
		if(x == 27){
			editor.resetStatusBar();
			return "";
		}else if(x==127){
			if (s.size()) {
				editor.popStatusBar();
				s.pop_back();
			}
			if (s.empty()) return "";
		}else if(x == 13){
			return s;
		}else{
			s.push_back(x);
			editor.appendStatusBar(x);
		}
		updateViews();
	}
	return "";
}

Command *ParseCommand::parseLongOrCharCommand(char c, int multiplicity, char prev) {

	if (isLong.count(c)) {
		std::string s;
		s.push_back(c);
		c = getAction();
		if (c == 27) return nullptr;
		s.push_back(c);
		
		LongCommand *command = new LongCommand{&editor, s, multiplicity};
                if (isRecording) macros[currentRegister].emplace_back(new LongCommand{*command});
		return command;
	}

	if (needSecond.count(c) && c != prev) {
		Command *comm = parseNext(c);
		if (comm) {
			MoveActionCommand *command = new MoveActionCommand{&editor, multiplicity, c, comm};
			if (needMode.count(command->getType())) inMode(command);
			if (isRecording) macros[currentRegister].emplace_back(new MoveActionCommand{*command});
			return command;
		}

		if (multiplicity == 0) multiplicity = 1;

		if (needMode.count(c)) {
			ModeCommand *command = new ModeCommand{&editor, c, multiplicity};
			inMode(dynamic_cast<ModeCommand*>(command));
	                if (isRecording) macros[currentRegister].emplace_back(new ModeCommand{*command});
        	        return command;

		}
		std::string s;
		s.push_back(c);
		s.push_back(c);
		
		LongCommand *command = new LongCommand{&editor, s, multiplicity};
                if (isRecording) macros[currentRegister].emplace_back(new LongCommand{*command});
		return command;
	}

	if (needSecond.count(c) && c == prev) {
		return nullptr;
	}

	CharCommand *command = new CharCommand(&editor, c, multiplicity);
        if (isRecording) macros[currentRegister].emplace_back(new CharCommand{*command});

	return command;

}

Command *ParseCommand::parseStatusBarCommand(char c, int multiplicity, char prev) {

	if (needFeed.count(c)) {
		
		std::string s = feed(c);	

		StatusBarCommand *command = new StatusBarCommand{&editor, s, multiplicity};
                
		if (isRecording) macros[currentRegister].emplace_back(new StatusBarCommand{*command});
        	return command;
	}

	return parseLongOrCharCommand(c, multiplicity, prev);
}

Command *ParseCommand::parseModeCommand(char c, int multiplicity, char prev) {
	
	if (needMode.count(c) && !needSecond.count(c)) {
		ModeCommand *command = new ModeCommand{&editor, c, multiplicity};
               	inMode(command); 
		if (isRecording) macros[currentRegister].emplace_back(new ModeCommand{*command});

		return command;
        }

	return parseStatusBarCommand(c, multiplicity, prev);

}

Command *ParseCommand::parseCtrlCommand(int i, char prev) {

	char c = i;

	int multiplicity = 0;
        while (std::isdigit(c)) {
                if(c=='0' && multiplicity == 0) break;
                multiplicity *= 10;
                multiplicity += c - 48;
                i = getAction();
                c = i;
        }

        if (multiplicity == 0 && !zeroMultiplicity.count(c)) multiplicity = 1;

	if(i < 32) { //checks for ctrl commands
		CtrlCommand *command = new CtrlCommand{&editor, i, multiplicity};
                
		if (isRecording) macros[currentRegister].emplace_back(new CtrlCommand{*command});    
                return command;
        }

	return parseModeCommand(i, multiplicity, prev);

}

Command *ParseCommand::parseNext(char prev) {
	
	int i = getAction();
        char c = i; 

	if (c == 'q') {
                if (isRecording) isRecording = false;
                else {
                        isRecording = true;
                        currentRegister = getAction();
                        macros[currentRegister].clear();
                }
                return nullptr;
        }

        if (c == '@') {
                doMacro(getAction());
                return nullptr;
        }

	return parseCtrlCommand(i, prev);

}

void ParseCommand::listen() {
	
	std::unique_ptr<Command> c;

	while(!(editor.getShouldClose())) {
		c.reset(parseNext());
		if (c) {
			char ch = c->getType();
			if (!needMode.count(ch))
				c->execute();
		}
		updateViews();
	}
	
}

void ParseCommand::open(std::string filename) {
	editor.open(filename);
	updateViews();
	listen();
}

State ParseCommand::getState() {

	std::string message = editor.getStatusMessage();

	if (isRecording && (message.length() == 0 || message == "-- INSERT --" || message == "-- REPLACE --")) {
		message += "recording @";
		message.push_back(currentRegister);
	}

	return State{editor.getFile(), editor.getCurrentLine(), editor.getCurrentChar(), message, editor.getColor()};
}
