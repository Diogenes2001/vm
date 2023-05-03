#include "curseView.h"
#include "state.h"
#include "screen.h"
#include <memory>
#include <ncurses.h>
#include <string>
#include <algorithm>
#include "parseCommand.h"

CurseView::CurseView(ParseCommand *model): View{}, model{ model }, main{std::make_unique<screen>(LINES-1, COLS, 0,0)}, status{std::make_unique<screen>(1, COLS-20, LINES-1,0)}, info{std::make_unique<screen>(1, 20, LINES-1,COLS-20)}{};

int getDisplayPos(std::string str, int pos){
	int x = 0;
	for(int i = 0; i < pos; ++i){
		if(str[i] == '\t'){
			++x;
			while(x%8 != 0) ++x;
		}else ++x;
	}
	return x;
} 

void CurseView::update(){

	State s = model->getState();
	status->display(s.statusMessage);
	std::string str = s.file.getLines()[s.currentLine];
	std::string infoStr;
	main->setColor(s.color);
	if(str.length() == 0){
		infoStr = std::to_string(s.currentLine+1) + "," + std::to_string(0);
		infoStr.append("-"+std::to_string(1));
	}else{
		infoStr = std::to_string(s.currentLine+1) + "," + std::to_string(s.currentChar+1);
		int x = getDisplayPos(str, s.currentChar);
		if(x > s.currentChar) infoStr.append("-" + std::to_string(x + 1));

	}
	info->display(infoStr);
	main->display(s.file.getLines());
	main->resetcursor();
	main->setcursor(s.file.getLines(), s.currentChar, s.currentLine);
	 main->setcursor(s.file.getLines(), s.currentChar, s.currentLine);

}

