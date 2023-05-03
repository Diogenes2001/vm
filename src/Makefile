CXX = g++
CXXFLAGS = -std=c++14 -Wall -MMD -g -w
EXEC = vm
OBJECTS = main.o curseKeyboard.o curseView.o editor.o file.o model.o parseCommand.o command.o charCommand.o longCommand.o statusBarCommand.o modeCommand.o screen.o ctrlCommand.o parser.o moveActionCommand.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${OBJECTS} -o ${EXEC} -lncurses
	TERM=xterm-color

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS} ${EXEC}
