#include "screen.h"
#include <ncurses.h>
#include <vector>
#include <string>
#include <algorithm>
#include <deque>
#include <memory>
#include <cctype>
#include "parser.h"


#define KEYWORD 1
#define NUMBER 2
#define STRING 3
#define TYPE 4
#define COMMENT 5
#define DIRECTIVE 6
#define IDENTIFIER 7
#define WARNING 8

using namespace Parser;
using std::vector;
using std::string;
//ctor
screen::screen(int nlines,int ncols,int y0,int x0): win{newwin(nlines, ncols, y0, x0)}, x{x0}, y{y0}, maxx{ncols}, maxy{nlines}, firstLineDisplayed{0} {
        scrollok(win, TRUE);
        keypad(win, TRUE);
	box(win, 0, 0);
        wmove(win, y, x);
        wrefresh(win);
	color = true;
	if(has_colors()){
		stringOn = false;
		commentOn = false;
		directiveOn = false;
		multi = false;
		identifier = false;
		//KEYWORDS and TYPES
		cmap["alignas"] = 1;
                cmap["alignof"] = 1;
                cmap["and"] = 1;
                cmap["and_eq"] = 1;
		cmap["asm"] = 1;
		cmap["auto"] = 4;
		cmap["bitand"] = 1;
		cmap["bitor"] = 1;
		cmap["bool"] = 4;
		cmap["break"] = 1;
                cmap["case"] = 1;
                cmap["catch"] = 1;
		cmap["char"] = 4;
                cmap["class"] = 4;
		cmap["compl"] = 1;
                cmap["const"] = 4;
		cmap["const_cast"] = 1;
                cmap["continue"] = 1;
		cmap["decltype"] = 4;
                cmap["default"] = 1;
                cmap["delete"] = 1;
                cmap["do"] = 1;
                cmap["double"] = 4;
                cmap["dynamic_cast"] = 1;
                cmap["else"] = 1;
                cmap["enum"] = 4;
		cmap["explicit"] = 4;
                cmap["export"] = 4;
                cmap["extern"] = 4;
                cmap["false"] = 2;
                cmap["float"] = 4;
                cmap["for"] = 1;
                cmap["friend"] = 1;
                cmap["goto"] = 1;
                cmap["if"] = 1;
                cmap["inline"] = 4;
                cmap["int"] = 4;
                cmap["long"] = 4;
                cmap["mutable"] = 4;
                cmap["namespace"] = 4;
                cmap["new"] = 1;
		cmap["noexcept"] = 4;
                cmap["not"] = 1;
                cmap["not_eq"] = 1;
                cmap["operator"] = 1;
		cmap["or"] = 1;
                cmap["or_eq"] = 1;
                cmap["private"] = 1;
                cmap["protected"] = 1;
		cmap["public"] = 1;
                cmap["register"] = 4;
                cmap["reinterpret_cast"] = 1;
                cmap["return"] = 1;
                cmap["short"] = 4;
                cmap["sizeof"] = 1;
                cmap["static"] = 4;
		cmap["static_assert"] = 1;
                cmap["static_cast"] = 1;
                cmap["struct"] = 4;
                cmap["switch"] = 1;
                cmap["template"] = 4;
                cmap["this"] = 1;
		cmap["thread_local"] = 1;
                cmap["throw"] = 1;
                cmap["true"] = 3;
                cmap["try"] = 1;
                cmap["typedef"] = 4;
                cmap["typeid"] = 1;
                cmap["typename"] = 4;
                cmap["union"] = 4;
                cmap["unsigned"] = 4;
                cmap["using"] = 1;
                cmap["virtual"] = 4;
                cmap["void"] = 4;
                cmap["volatile"] = 4;
                cmap["wchar_t"] = 4;
                cmap["while"] = 1;
		cmap["xor"] = 1;
                cmap["xor_eq"] = 1;

		
		//some additional TYPES
                cmap["nullptr"] = 3;
                cmap["nullptr_t"] = 4;
                cmap["char16_t"] = 4;
                cmap["char32_t"] = 4;
                cmap["size_t"] = 4;
                cmap["ptrdiff_t"] = 4;

		//preprocessor DIRECTIVES
		cmap["#include"] = 6;
                cmap["#pragma"] = 6;
                cmap["#define"] = 6;
                cmap["#error"] = 6;
                cmap["#warning"] = 6;
                cmap["#undef"] = 6;
		cmap["#if"] = 6;
                cmap["#else"] = 6;
                cmap["#elif"] = 6;
                cmap["#endif"] = 6;
                cmap["#ifdef"] = 6;
                cmap["#ifndef"] = 6;
		cmap["#line"] = 6;


	}
    }
//dtor
screen::~screen(){
        delwin(win);
    }

void screen::setColor(bool b){ color = b; }
bool isNum(std::string s){
	if(!isdigit(s[0])) return false;
	for(auto x:s){
		if(!(isdigit(x) || x == '.')) return false;
	}
	return true;
}

bool screen::isArgList(std::string s){
	std::unique_ptr<std::deque<std::string>> d = parse(s);
	while(d->size() > 0){
		while(d->size() > 0 && isspace(d->front()[0])) d->pop_front();
		if(d->size() == 0) return false;
		if(cmap[d->front()] == TYPE || cmap[d->front()] == IDENTIFIER){
			while(d->size() > 0 && d->front() != ","){
				d->pop_front();	
			}
			if (d->size() == 0) return true;
			d->pop_front();
		}else return false;
	}
	return false;	
}

bool screen::isClass(std::unique_ptr<std::deque<std::string>> &d){
	if(d->size() < 5) return false;
	d->pop_front();
	if(!isspace(d->front()[0])) return 0;
	d->pop_front();
	if(!isalnum(d->front()[0])) return 0;
	d->pop_front();
        if(!isspace(d->front()[0])) return 0;
	d->pop_front();
	if(d->front() == "final") return true;
	return false;
}

//checks if a line is a function or class/struct declaration for final/override
bool screen::isHeader(std::string s){
        std::unique_ptr<std::deque<std::string>> d = parse(s);
	while(isspace(d->front()[0])) d->pop_front();
	if(d->size() == 0) return false;
	if(d->front() == "struct" || d->front() == "class") return isClass(d);
	if(cmap[d->front()] == TYPE || cmap[d->front()] == IDENTIFIER){
		while(cmap[d->front()] == TYPE || cmap[d->front()] == IDENTIFIER || isspace(d->front()[0])) d->pop_front();
		while(d->front()[d->front().length() -1] != '(' && d->front() != "()") d->pop_front();
		if(d->front() == "()") return true;
		if(d->size() == 0) return false;
		std::string list = "";
		d->pop_front();
		while(d->front()[0] != ')'){
			list = list.append(d->front());
			d->pop_front();
		}
		if(d->size() == 0) return false;
		return isArgList(list);
	}else return false;
}


void screen::toggleString(){
	if(stringOn){
		wattroff(win, COLOR_PAIR(STRING));
	}else{
		wattron(win, COLOR_PAIR(STRING));
	}
	stringOn = !stringOn;
}

void screen::toggleComment(){
        if(commentOn){
                wattroff(win, COLOR_PAIR(COMMENT));
        }else{
                wattron(win, COLOR_PAIR(COMMENT));
        }
        commentOn = !commentOn;
}


void screen::prettyPrint(string str){
	if(commentOn) wattron(win, COLOR_PAIR(COMMENT));
	else if(directiveOn) wattron(win, COLOR_PAIR(DIRECTIVE));	
	
	std::unique_ptr<std::deque<std::string>> d = parse(str);
	bool hash = false;
	std::string last="";
	while(d->size() > 0){
		bool isNewIdentifier = false;
		std::string word = d->front();
		d->pop_front();
		if(hash){
			std::string s = "#";
			word = s.append(word);
			hash = false;
		}

		if(word == "#"){
			hash = true;
		}else if(word == "/**/"){
			wattron(win, COLOR_PAIR(COMMENT));
			wprintw(win, word.c_str());
			wattroff(win, COLOR_PAIR(COMMENT));
		}else if(word.find("*/") != std::string::npos){
			std::string temp = word.substr(0, word.find("*/"));
                        word = word.substr(word.find("*/")+2);
                        wprintw(win, temp.c_str());
			if(!commentOn){
				wattron(win, COLOR_PAIR(WARNING));
				wprintw(win, "*/");
				wattroff(win, COLOR_PAIR(WARNING));	
			}else{
				wprintw(win, "*/");
			}
                        wattroff(win, COLOR_PAIR(COMMENT));
                        wprintw(win, word.c_str());
                        commentOn = false;
                        multi = false;
		}else if(commentOn){
			wprintw(win, word.c_str());
		}else if(word == "\"\""){
			wattron(win, COLOR_PAIR(STRING));
			wprintw(win, word.c_str());
			wattroff(win, COLOR_PAIR(STRING));
		}else if(word.find('"')!=std::string::npos){
			while(word.find('"')!=std::string::npos){
				std::string temp = word.substr(0, word.find('"'));
				word = word.substr(word.find('"') +1);
                        	wprintw(win, temp.c_str());
                        	wattron(win, COLOR_PAIR(STRING));
                        	waddch(win, '"');
                        	toggleString();
			}
			wprintw(win, word.c_str());
			
		}else if(stringOn){
			wprintw(win, word.c_str());
		}else if(word.find("//") != std::string::npos){
			std::string temp = word.substr(0, word.find("//"));
			word = word.substr(word.find("//"));
			wprintw(win, temp.c_str());
			wattron(win, COLOR_PAIR(COMMENT));
			wprintw(win, word.c_str());
			commentOn = true;
		}else if(word.find("/*") != std::string::npos){
			std::string temp = word.substr(0, word.find("/*"));
                        word = word.substr(word.find("/*"));
			wprintw(win, temp.c_str());
                        wattron(win, COLOR_PAIR(COMMENT));
			wprintw(win, word.c_str());
                        commentOn = true;
			multi = true;
		}else if(directiveOn){
			wprintw(win, word.c_str());
		}else if(isNum(word)){
			wattron(win, COLOR_PAIR(NUMBER));
			wprintw(win, word.c_str());
			wattroff(win, COLOR_PAIR(NUMBER));
		}else if(cmap.count(word)){
			int x = cmap[word];
			if (x==4) {
				identifier = true;
				isNewIdentifier = true;
			}else if(x==6){
				directiveOn = true;
			}
			wattron(win, COLOR_PAIR(x));
                        wprintw(win, word.c_str());
                        wattroff(win, COLOR_PAIR(x));
		}else{
			if(word == "final" || word == "override"){
				if(isHeader(str)){
					wattron(win, COLOR_PAIR(TYPE));
	        	                wprintw(win, word.c_str());
        	        	        wattroff(win, COLOR_PAIR(TYPE));
				}else{
					wprintw(win, word.c_str());
				}
			}else if(identifier && word != "std" && isalpha(word[0])){
				cmap[word] = 7;
				wattron(win, COLOR_PAIR(IDENTIFIER));
				wprintw(win, word.c_str());
				wattroff(win, COLOR_PAIR(IDENTIFIER));
				
			}else{
				wprintw(win, word.c_str());
			}
			
		}
		if(!isNewIdentifier && isalnum(word[0])){
			identifier = false;
		}
	}
	if (hash) wprintw(win, "#");

}

void screen::display(const vector<string> &vec, int line){
        firstLineDisplayed = std::min(line, static_cast<int>(vec.size()-LINES+6));;
        display(vec);
}

string tabsToSpaces(const std::string &s){
	if(s.length() == 0) return "";
	string news = "";
	int a = 0;
	for(size_t x = 0; x < s.length(); ++x){
		if(s[x] == '\t'){
			do{
				++a;
				news.append(1, ' ');
			}while(a % 8 != 0);
		}else{
			++a;
			news.append(1, s[x]);
		}
	}
	return news;
}

//display a file at the current cursor position, starting at line line
void screen::display(const vector<string> &vec){
	int line = firstLineDisplayed;
	displayedWhere.clear();
        wclear(win);
        string str = tabsToSpaces(vec[line]);
	displayedWhere[line] = 0;
        ++line;
        for(int i = 0; i < maxy-1; ++i){
		
	string newstr = str.substr(0, maxx);
	str = str.substr(newstr.length());
            if(color) prettyPrint(newstr);
	    else wprintw(win, newstr.c_str());

            if(str.length() == 0){
		waddch(win, '\n');
		wattroff(win, COLOR_PAIR(DIRECTIVE));
		directiveOn = false;
        	if(!multi){
			wattroff(win, COLOR_PAIR(COMMENT));
        		commentOn = false;
		}
                if(line == vec.size()){
		    wattron(win, COLOR_PAIR(COMMENT));
                    str = "~";
                }else{
		    displayedWhere[line] = i+1;
                    str = tabsToSpaces(vec[line]);
                    ++line;
                }
            }
        }

	wattroff(win, COLOR_PAIR(COMMENT));
	wattroff(win, COLOR_PAIR(DIRECTIVE));
	wattroff(win, COLOR_PAIR(STRING));

	stringOn = false;
        commentOn = false;
        directiveOn = false;
	multi = false;
        wmove(win, x, y);
        wrefresh(win);
    }
//clears window and displays a string, mostly used for status bar
void screen::display(const string &s){
	wclear(win);
	wprintw(win, s.c_str());
	wrefresh(win);
}

//set and move the cursor, a is char, b is line
int screen::setcursor(const vector<string> &vec, int a, int b){
	if(displayedWhere.count(b)){
		y=displayedWhere[b];
		x=0;
		std::string line = vec[b];
		for(int i = 0; i < a; ++i){
			if(line[i] == '\t'){
				++x;
				while(x%8 != 0) ++x;
			}else ++x;
		}
		/*if(line[a] == '\t'){
			while(x%8 != 0) ++x;
		}*/
		y+=x/maxx;
		x%=maxx;
		if(y > maxy-5){
			display(vec, firstLineDisplayed + 1);
			--y;
		}else if(y < 5 && firstLineDisplayed > 0){
			display(vec, firstLineDisplayed - 1);
                	++y;
		}
	}else{
		display(vec, b);
	}
	wmove(win, y, x);
        wrefresh(win);
        return y;
}

int screen::resetcursor(){
	wmove(win, 0, 0);
	return 0;
}
