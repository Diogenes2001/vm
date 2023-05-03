#ifndef FILE_H
#define FILE_H

#include<vector>
#include<memory>
#include<string>

class File {

	std::vector<std::string> lines;
	std::shared_ptr<File> previousVersion;

	int currentLine = 0;
	int currentChar = 0;

	public:
		File();
		File(std::string filename);
		File(std::vector<std::string> lines, std::shared_ptr<File> next);
		std::vector<std::string> &getLines();
		const std::vector<std::string> &getLines() const;
		bool empty() const;
		std::shared_ptr<File> getPrevious();

		int getCurrentLine();
		void setCurrentLine(int);

		int getCurrentChar();
		void setCurrentChar(int);

};

#endif
