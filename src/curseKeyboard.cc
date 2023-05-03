#include "curseKeyboard.h"
#include <ncurses.h>
int CurseKeyboard::getAction() {
	return getch();
}
