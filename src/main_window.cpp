#include <QGridLayout>
#include <QApplication>
#include "widgets.h"
#include "picture_panel.h"

MainWindow::MainWindow(QWidget *parent, const SeriesVec &base): QWidget(parent)
{
	control_panel = new ControlPanel(this);
	picture_panel = new PicturePanel(this, base);
	setWindowTitle(tr("Drawing"));
	auto layout = new QVBoxLayout(this);
	layout->addWidget(picture_panel);
	layout->addWidget(control_panel);
	setLayout(layout);
	resize(600, 700);
}
