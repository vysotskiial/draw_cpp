#include <QHBoxLayout>
#include <QFileDialog>
#include <QPdfWriter>
#include <QBitmap>
#include <iostream>
#include <string>
#include "widgets.h"
using namespace std;

TextWindow::TextWindow(ControlPanel *parent): parent_panel(parent)
{
	auto mw = (MainWindow *)parent_panel->parent();
	table = new QTableWidget(mw->picture_panel->texts.size(), 4, this);
	table->setHorizontalHeaderLabels({"String", "X", "Y", "Font"});
	auto i = 0;
	for (auto &text : mw->picture_panel->texts) {
		table->setItem(i, 0, new QTableWidgetItem(text.text));
		table->setItem(i, 1, new QTableWidgetItem(QString::number(text.x)));
		table->setItem(i, 2, new QTableWidgetItem(QString::number(text.y)));
		table->setItem(i, 3, new QTableWidgetItem(QString::number(text.font)));
		i++;
	}

	bool ok = KLFBackend::detectSettings(&settings);
	if (!ok) {
		// vital program not found
		throw("error in your system: are latex,dvips and gs installed?");
	}

	input.mathmode = "\\begin{equation*} ... \\end{equation*}";
	input.preamble = "\\usepackage{amsmath}\n";
	input.dpi = 300;

	auto layout = new QVBoxLayout();
	layout->addWidget(table);
	auto button_layout = new QHBoxLayout();
	save_button = new QPushButton("Save");
	add_button = new QPushButton("Add");
	delete_button = new QPushButton("Delete");

	connect(save_button, &QPushButton::released, this, &TextWindow::on_save);
	connect(add_button, &QPushButton::released, this,
	        [this]() { table->setRowCount(table->rowCount() + 1); });
	connect(delete_button, &QPushButton::released, this,
	        [this]() { table->removeRow(table->currentRow()); });

	button_layout->addWidget(add_button);
	button_layout->addWidget(delete_button);

	button_layout->addStretch();
	button_layout->addWidget(save_button);
	layout->addLayout(button_layout);
	setLayout(layout);
	resize(500, 500);
}

void TextWindow::on_save()
{
	MainWindow *mw = (MainWindow *)parent_panel->parent();

	mw->picture_panel->texts.clear();
	for (auto i = 0; i < table->rowCount(); i++) {
		input.latex = table->item(i, 0)->text();
		input.fontsize = table->item(i, 3)->text().toInt();
		auto out = KLFBackend::getLatexFormula(input, settings);
		auto pm = QPixmap::fromImage(out.result);
		pm.setMask(pm.createMaskFromColor("white"));
		double x = table->item(i, 1)->text().toDouble();
		double y = table->item(i, 2)->text().toDouble();
		mw->picture_panel->texts.push_back({x, y, pm, input.latex, input.fontsize});
	}
	mw->picture_panel->update();
	close();
}

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
	auto window = new TextWindow(this);
	window->show();
}

void ControlPanel::on_unzoom()
{
	auto main_window = (MainWindow *)parent();
	main_window->picture_panel->chart()->zoomReset();
}
