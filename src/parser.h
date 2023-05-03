#ifndef PARSER_H
#define PARSER_H

#include <deque>
#include <string>
#include <memory>

namespace Parser{
	int getType(char c);
	std::unique_ptr<std::deque<std::string>> parse(std::string str);
}

#endif
