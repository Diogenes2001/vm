#ifndef CONTROLLER_H
#define CONTROLLER_H
class screen;
class Controller{

	public:
		virtual ~Controller(){};
		virtual int getAction() = 0;

};

#endif
