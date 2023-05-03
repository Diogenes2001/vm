#ifndef STATE_H
#define STATE_H

#include "file.h"
#include <string>

struct State {

	const File &file;
	int currentLine;
	int currentChar;
	std::string statusMessage;
	bool color;

};

#endif
