#ifndef SCREEN_H
#define SCREEN_H
#include <ncurses.h>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <memory>
using std::vector;
using std::string;
    class screen{
    	WINDOW * win;
    	int x;
    	int y;
    	int maxx;
    	int maxy;
	int firstLineDisplayed;
	std::map<int, int> displayedWhere;
	std::map<string, int> cmap;

	bool color;
	bool stringOn;
	bool commentOn;
	bool directiveOn;
	bool multi;
	bool identifier;

	void toggleString();
	void toggleComment();
	
	//for contexual keywords
	bool isHeader(std::string s);
	bool isArgList(std::string s);
        bool isClass(std::unique_ptr<std::deque<std::string>> &d);

	public:
        screen(int nlines,int ncols,int y0,int x0);
        ~screen();
	void display (const vector<string> &vec);
        void display (const vector<string> &vec, int line);
	void display (const string &s);
        int setcursor(const vector<string> &vec, int a, int b);
	int resetcursor();

	void prettyPrint(std::string s);
	void setColor(bool b);
    };
#endif
