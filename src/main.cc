#include <memory>
#include "curseView.h"
#include "curseKeyboard.h"
#include "model.h"
#include <iostream>
#include <fstream>
#include <ncurses.h>
#include <string>
#include "parseCommand.h"
int main(int argc, char *argv[]) {

	if (argc == 1) {
		std::cout << "vm: CS246E F2019 final project\nMade by Ethan Zhang and Sophia Pietsch" << std::endl;
		return 0;
	}
	initscr();
	refresh();
	nonl();
	noecho();
	std::string str = argv[1];
	/*
	 * To use custom themes:
	 * Pass a .color file with 6 lines of space-seperated rgb values. The lines are:
	 * 1. KEYWORDS
	 * 2. STRING AND NUMERIC LITERALS
	 * 3. TYPES
	 * 4. COMMENTS
	 * 5. PREPROCESSOR DIRECTIVES
	 * 6. IDENTIFIERS
	 * 7. BACKGROUND
	 */
	if(has_colors() && (!str.compare(str.length()-3, 3, ".cc") || !str.compare(str.length()-2, 2, ".h"))) {
		start_color();
		if(argc == 3){
			std::string colorfile = argv[2];
			if(!colorfile.compare(colorfile.length()-6, 6, ".color")){
				if(!can_change_color()){
					endwin();
					std::cerr << "Your terminal does not support custom colours, please use default" << std::endl;
	                                throw 1;
				}
				std::ifstream f{colorfile};
				int x, y, z;
				int arr[7] = {COLOR_YELLOW, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_BLACK};
				
				for(int i = 0; i < 7; ++i){
					f >> x;	
					f >> y;
					f >> z;
					init_color(arr[i], x, y, z);
				}
			}else{
				endwin();
				std::cerr << "Please select a valid color file (or use default)" << std::endl;
		                throw 1;
			}
		}
		init_pair(1, COLOR_YELLOW, COLOR_BLACK);
               	init_pair(2, COLOR_RED, COLOR_BLACK);
               	init_pair(3, COLOR_RED, COLOR_BLACK);
               	init_pair(4, COLOR_GREEN, COLOR_BLACK);
               	init_pair(5, COLOR_BLUE, COLOR_BLACK);
               	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(7, COLOR_CYAN, COLOR_BLACK);
		init_pair(8, COLOR_WHITE, COLOR_RED);
	}
	std::unique_ptr<ParseCommand> model{new ParseCommand()};

	View *view = new CurseView(model.get());
	Controller *controller = new CurseKeyboard();

	model->addView(view);
	model->addController(controller);

	model->open(argv[1]);
	endwin();

}
