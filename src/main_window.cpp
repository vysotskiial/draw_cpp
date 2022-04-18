#include <QGridLayout>
#include <QApplication>
#include "widgets.h"
using namespace QtCharts;

void MainWindow::fill_grid()
{
	auto i = 0;
	for (auto picture : picture_panels) {
		if (i % 2 == 2)
			pics_layout->setRowStretch(i / 2, 1);

		pics_layout->addWidget(picture, i / 2, i % 2, 1, 1);
		picture->show();
		i++;
	}
}

void MainWindow::to_grid()
{
	if (picture_idx == -1)
		return;

	picture_panels[picture_idx]->in_grid = true;
	picture_panels[picture_idx]->hide();
	for (auto picture : picture_panels)
		picture->show();
	picture_idx = -1;
	layout()->removeWidget(control_panel);
	layout()->removeWidget(picture_panels[picture_idx]);
	fill_grid();

	layout()->addItem(pics_layout);
	layout()->addWidget(control_panel);
}

void MainWindow::from_grid(PicturePanel *choice)
{
	layout()->removeItem(layout()->itemAt(0));
	for (auto i = pics_layout->count(); i > 0; i--)
		pics_layout->itemAt(i - 1)->widget()->hide();
	layout()->removeWidget(control_panel);
	layout()->addWidget(choice);
	choice->in_grid = false;
	choice->show();
	picture_idx = picture_panels.indexOf(choice);
	layout()->addWidget(control_panel);
}

MainWindow::MainWindow(QWidget *parent, QVector<QChart *> c)
{
	control_panel = new ControlPanel(this);
	setWindowTitle(tr("Drawing"));
	auto layout = new QVBoxLayout(this);
	pics_layout = new QGridLayout();
	if (c.size() > 1) {
		pics_layout->setColumnStretch(0, 1);
		pics_layout->setColumnStretch(1, 1);
	}
	else {
		picture_idx = 0;
	}
	for (auto i = 0; i < c.size(); i++) {
		auto picture = new PicturePanel((MainWindow *)parent, c[i], c.size() > 1);
		picture_panels.append(picture);
	}
	fill_grid();
	layout->addLayout(pics_layout);
	layout->addWidget(control_panel);
	setLayout(layout);
	resize(700, 700);
}
