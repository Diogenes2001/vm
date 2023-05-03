#ifndef CURSEKEYBOARD_H
#define CURSEKEYBOARD_H
#include <memory>
#include "controller.h"
class screen;
class CurseKeyboard : public Controller {
	public:
		int getAction() override;

};

#endif
