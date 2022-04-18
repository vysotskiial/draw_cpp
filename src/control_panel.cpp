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
	grid_button = new QPushButton("Grid", this);
	coords_text = new QLabel("100");

	connect(save_button, &QPushButton::released, this, &ControlPanel::on_save);
	connect(text_button, &QPushButton::released, this, &ControlPanel::on_text);
	connect(unzoom_button, &QPushButton::released, this,
	        &ControlPanel::on_unzoom);
	connect(grid_button, &QPushButton::released, this, &ControlPanel::on_grid);

	auto layout = new QHBoxLayout(this);
	layout->addWidget(save_button);
	layout->addWidget(text_button);
	layout->addWidget(unzoom_button);
	layout->addWidget(grid_button);
	layout->insertStretch(4, 1);
	layout->addWidget(coords_text);
	setLayout(layout);
}

void ControlPanel::on_grid()
{
	auto mw = (MainWindow *)parent();
	mw->to_grid();
}

void ControlPanel::on_save()
{
	auto fileName =
	  QFileDialog::getSaveFileName(this, "Save Picture", "", "PNG file (*.png)");

	if (!fileName.size())
		return;

	auto main_window = (MainWindow *)parent();
	main_window->picture_panels[main_window->picture_idx]->grab().save(fileName);
}

void ControlPanel::on_text()
{
	auto mw = (MainWindow *)parent();
	auto zoom = mw->picture_panels[mw->picture_idx]->switch_zoom();
	text_button->setText(zoom ? "Text" : "Zoom");
}

void ControlPanel::on_unzoom()
{
	auto main_window = (MainWindow *)parent();
	main_window->picture_panels[main_window->picture_idx]->chart()->zoomReset();
}
