#include "model.h"
#include "view.h"

void Model::updateViews() {

	view->update();

}

void Model::addView(View *view) {

	this->view.reset(view);

}

void Model::addController(Controller *controller) {

	this->controller.reset(controller);
}

int Model::getAction() {
	return controller->getAction();

}
