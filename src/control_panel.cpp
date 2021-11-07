#include <QHBoxLayout>
#include <QFileDialog>
#include <QPdfWriter>
#include <QBitmap>
#include <iostream>
#include <string>
#include "widgets.h"
using namespace std;

ControlPanel::ControlPanel(QWidget *parent): QWidget(parent)
{
	save_button = new QPushButton("Save", this);
	text_button = new QPushButton("Text", this);
	unzoom_button = new QPushButton("Home", this);
	coords_text = new QLabel("100");

	connect(save_button, &QPushButton::released, this, &ControlPanel::on_save);
	connect(text_button, &QPushButton::released, this, &ControlPanel::on_text);
	connect(unzoom_button, &QPushButton::released, this,
	        &ControlPanel::on_unzoom);

	auto layout = new QHBoxLayout(this);
	layout->addWidget(save_button);
	layout->addWidget(text_button);
	layout->addWidget(unzoom_button);
	layout->insertStretch(3, 1);
	layout->addWidget(coords_text);
	setLayout(layout);
}

void ControlPanel::on_save()
{
	auto fileName =
	  QFileDialog::getSaveFileName(this, "Save Picture", "", "PNG file (*.png)");

	if (!fileName.size())
		return;

	auto main_window = (MainWindow *)parent();
	main_window->picture_panel->grab().save(fileName);
}

void ControlPanel::on_text()
{
	auto mw = (MainWindow *)parent();
	mw->picture_panel->switch_zoom();
}

void ControlPanel::on_unzoom()
{
	auto main_window = (MainWindow *)parent();
	main_window->picture_panel->chart()->zoomReset();
}
