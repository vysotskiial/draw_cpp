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
	save_button = new QPushButton(QIcon("../images/filesave.png"), "", this);
	save_button->setIconSize({48, 48});
	zoom_button = new QPushButton(QIcon("../images/zoom_in.png"), "", this);
	zoom_button->setIconSize({48, 48});
	zoom_button->setCheckable(true);
	unzoom_button = new QPushButton(QIcon("../images/zoom_out.png"), "", this);
	unzoom_button->setIconSize({48, 48});
	grid_button = new QPushButton(QIcon("../images/home.png"), "", this);
	grid_button->setIconSize({48, 48});
	graph_button = new QPushButton(QIcon("../images/graph.png"), "", this);
	graph_button->setIconSize({48, 48});
	coords_text = new QLabel("");

	connect(save_button, &QPushButton::released, this, &ControlPanel::on_save);
	connect(zoom_button, &QPushButton::released, this, &ControlPanel::on_zoom);
	connect(unzoom_button, &QPushButton::released, this,
	        &ControlPanel::on_unzoom);
	connect(grid_button, &QPushButton::released, this, &ControlPanel::on_grid);
	connect(graph_button, &QPushButton::released, this, &ControlPanel::on_graph);

	auto layout = new QHBoxLayout(this);
	layout->addWidget(grid_button);
	layout->addWidget(save_button);
	layout->addWidget(zoom_button);
	layout->addWidget(unzoom_button);
	layout->addWidget(graph_button);
	layout->addStretch(1);
	layout->addWidget(coords_text);
	setLayout(layout);
}

void ControlPanel::on_grid()
{
	auto mw = (MainWindow *)parent();
	mw->graph_panel->to_grid();
	coords_text->setText("");
}

void ControlPanel::on_save()
{
	auto fileName =
	  QFileDialog::getSaveFileName(this, "Save Picture", "", "PNG file (*.png)");

	if (!fileName.size())
		return;

	auto main_window = (MainWindow *)parent();
	main_window->graph_panel->save_widget(fileName);
}

void ControlPanel::on_zoom()
{
	auto mw = (MainWindow *)parent();
	auto zoom = mw->graph_panel->zoom_text_switch();
	zoom_button->setChecked(zoom);
}

void ControlPanel::on_unzoom()
{
	auto main_window = (MainWindow *)parent();
	main_window->graph_panel->reset_zoom();
}

void ControlPanel::on_graph()
{
	auto main_window = (MainWindow *)parent();
	main_window->graph_panel->change_graph();
}
