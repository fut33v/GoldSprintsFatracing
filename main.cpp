#include <QtWidgets/QApplication>
#include "UI/GoldSprintsFatracing.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	GoldSprintsFatracing w;
	w.show();
	return a.exec();
}
