#include <QHBoxLayout>
#include <QFileDialog>
#include <QPdfWriter>
#include <QBitmap>
#include <QShortcut>
#include <iostream>
#include <string>
#include "widgets.h"
#include "picture_panel.h"
using namespace std;

ControlPanel::ControlPanel(MainWindow *parent): QWidget(parent), mw(parent)
{
	QString images_prefix = IMAGES_PATH;
	auto open_button =
	  new QPushButton(QIcon(images_prefix + "/images/folder.png"), "", this);
	open_button->setIconSize({48, 48});
	auto save_button =
	  new QPushButton(QIcon(images_prefix + "/images/filesave.png"), "", this);
	save_button->setIconSize({48, 48});
	zoom_button =
	  new QPushButton(QIcon(images_prefix + "/images/zoom_in.png"), "", this);
	zoom_button->setIconSize({48, 48});
	zoom_button->setCheckable(true);
	auto unzoom_button =
	  new QPushButton(QIcon(images_prefix + "/images/zoom_out.png"), "", this);
	unzoom_button->setIconSize({48, 48});
	auto graph_button =
	  new QPushButton(QIcon(images_prefix + "/images/graph.png"), "", this);
	graph_button->setIconSize({48, 48});
	coords_text = new QLabel("");

	connect(open_button, &QPushButton::released, this, &ControlPanel::on_open);
	connect(save_button, &QPushButton::released, this, &ControlPanel::on_save);
	connect(zoom_button, &QPushButton::released, this, [this]() {
		auto zoom = mw->picture_panel->switch_zoom();
		zoom_button->setChecked(zoom);
	});
	connect(unzoom_button, &QPushButton::released, this,
	        [this]() { mw->picture_panel->chart()->zoomReset(); });
	connect(graph_button, &QPushButton::released, this,
	        [this]() { mw->picture_panel->graph_dialog(); });

	auto save_as = new QShortcut(QKeySequence("Ctrl+Shift+S"), this);
	connect(save_as, &QShortcut::activated, this, &ControlPanel::on_save);
	auto save = new QShortcut(QKeySequence("Ctrl+S"), this);
	connect(save, &QShortcut::activated, this, [this]() {
		if (save_file.size())
			this->save(save_file);
	});

	auto layout = new QHBoxLayout(this);
	layout->addWidget(open_button);
	layout->addWidget(save_button);
	layout->addWidget(zoom_button);
	layout->addWidget(unzoom_button);
	layout->addWidget(graph_button);
	layout->addStretch(1);
	layout->addWidget(coords_text);
	setLayout(layout);
}

void ControlPanel::on_save()
{
	auto fileName = QFileDialog::getSaveFileName(this, "Save Project", "",
	                                             "Save options (*.png *.json)");

	if (!fileName.size())
		return;

	save(fileName);
}

void ControlPanel::save(const QString &fileName)
{
	if (fileName.endsWith("png")) {
		mw->picture_panel->grab().save(fileName);
	}
	else {
		save_file = fileName;
		QFileInfo fi(save_file);
		mw->setWindowTitle(fi.fileName());
		mw->picture_panel->save_project(fileName);
	}
	if (mw->windowTitle().endsWith("[+]"))
		mw->setWindowTitle(mw->windowTitle().chopped(3));
}

void ControlPanel::on_open()
{
	auto filename =
	  QFileDialog::getOpenFileName(this, "Open project", "", "JSON (*.json)");
	if (!filename.size())
		return;

	mw->picture_panel->open_project(filename);
	save_file = filename;
	QFileInfo fi(save_file);
	mw->setWindowTitle(fi.fileName());
}
