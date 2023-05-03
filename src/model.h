#ifndef MODEL_H
#define MODEL_H

#include<memory>
#include "controller.h"
#include "view.h"

class Model {

	std::unique_ptr<View> view;
	std::unique_ptr<Controller> controller;

	//methods
	public:
		virtual ~Model(){};
		void updateViews();
		void addView(View *view);
                void addController(Controller *controller);

	protected:
		int getAction();
		
};

#endif
