#include <QGridLayout>
#include <QApplication>
#include "widgets.h"
#include "picture_panel.h"

MainWindow::MainWindow(QWidget *parent, const SeriesVec &base): QWidget(parent)
{
	QString images_prefix = IMAGES_PATH;
	setWindowIcon(QIcon(images_prefix + "/images/graph.png"));
	control_panel = new ControlPanel(this);
	picture_panel = new PicturePanel(this, base);
	setWindowTitle("New Project");
	auto layout = new QVBoxLayout(this);
	layout->addWidget(picture_panel);
	layout->addWidget(control_panel);
	setLayout(layout);
	resize(600, 700);
}
