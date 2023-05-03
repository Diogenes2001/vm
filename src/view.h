#ifndef VIEW_H
#define VIEW_H

struct State;

class View{
	
	public:
		virtual void update() = 0;
		virtual ~View() {};

};

#endif
