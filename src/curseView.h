#ifndef CURSEVIEW_H
#define CURSEVIEW_H

#include "view.h"
#include <memory>
using std::unique_ptr;
class ParseCommand;
class screen;
struct State;

class CurseView : public View {

	ParseCommand *model;
	unique_ptr<screen> main;
        unique_ptr<screen> status;
	unique_ptr<screen> info;

	public:
		CurseView(ParseCommand *model);
		void update() override;

};

#endif
