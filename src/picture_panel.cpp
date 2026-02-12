#include <QPainter>
#include <QLayout>
#include <QFormLayout>
#include <QMouseEvent>
#include <QChart>
#include <QBitmap>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QSplineSeries>
#include <QValueAxis>
#include <klfbackend.h>
#include <QApplication>
#include <QColorDialog>
#include <QScreen>
#include <QMessageBox>
#include <exception>
#include <fstream>
#include <print>
#include "widgets.h"
#include "picture_panel.h"

using namespace std;
using namespace QtCharts;

void PicturePanel::add_chart(QString label,
                             const QVector<QAbstractSeries *> &series)
{
	auto chart_view = new PictureTab(this);
	chart_view->setRubberBand(QChartView::RubberBand::NoRubberBand);
	chart_view->setRenderHint(QPainter::Antialiasing);
	auto chart = chart_view->chart();
	chart->legend()->setVisible(true);
	auto x_axis = new QValueAxis();
	x_axis->setLinePen(Qt::PenStyle::SolidLine);

	auto y_axis = new QValueAxis;
	y_axis->setLinePen(Qt::PenStyle::SolidLine);
	for (auto &s : series)
		chart->addSeries(s);

	auto idx = tabs->addTab(chart_view, label);
	tab2chart[idx] = chart_view;
	chart->createDefaultAxes();
	for (auto &axis : chart->axes()) {
		if (draw_grid)
			((QValueAxis *)axis)->applyNiceNumbers();
		else
			((QValueAxis *)axis)->setTickCount(2);
	}
}

PictureTab::PictureTab(PicturePanel *o): QtCharts::QChartView(o), owner(o)
{
	setMouseTracking(true);
	setMinimumSize(500, 500);
	bool ok = KLFBackend::detectSettings(&settings);
	if (!ok) {
		// vital program not found
		throw(std::runtime_error("error in your system: are latex,dvips and gs "
		                         "installed?"));
	}

	input.mathmode = "\\begin{equation*} ... \\end{equation*}";
	input.preamble = "\\usepackage{amsmath}\n";
	input.dpi = 300;
}

PicturePanel::PicturePanel(MainWindow *parent): mw(parent), draw_grid{false}
{
	tabs = new QTabWidget(this);
	auto layout = new QHBoxLayout(this);
	layout->addWidget(tabs);
	add_chart("", {});

	chart_dialog = new ChartDialog(this);
}

void PictureTab::paintEvent(QPaintEvent *e)
{
	if (making_cache || !mouse_pressed) {
		QChartView::paintEvent(e);
		if (making_cache) {
			making_cache = false;
			return;
		}
	}

	QPainter painter(viewport());
	if (mouse_pressed) {
		painter.drawPixmap(0, 0, cached_graph);
	}

	for (auto &text : texts) {
		auto coords = chart2widget({text.coords});
		painter.drawPixmap(coords.x(), coords.y(), text.pm);
	}

	if (mouse_pressed && owner->zoom_mode) {
		painter.setPen(Qt::PenStyle::DashLine);
		painter.drawRect(QRect{zoom_start, zoom_end});
	}
}

void PictureTab::mousePressEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton)
		return;

	mouse_pressed = true;

	if (owner->zoom_mode) {
		auto screen = QApplication::primaryScreen();
		cached_graph = screen->grabWindow(winId());
		zoom_start = e->pos();
	}
	else {
		find_text(e->pos());
		if (text_idx != -1) {
			making_cache = true;
			cached_graph = grab();
		}
	}
}

void PictureTab::find_text(QPoint pos)
{
	for (auto i = 0; i < texts.size(); i++) {
		auto coords = chart2widget({texts[i].coords});
		auto text_rect = QRect(coords, texts[i].pm.size());
		if (text_rect.contains(pos)) {
			text_idx = i;
			mouse_text_offset = pos - text_rect.topLeft();
			return;
		}
	}
	text_idx = -1;
}

void PictureTab::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (owner->zoom_mode)
		return;

	find_text(e->pos());
	if (text_idx == -1)
		return;

	if (input_latex(widget2chart(e->pos() - mouse_text_offset)))
		viewport()->update();
}

QPixmap PictureTab::process_latex()
{
	input.latex = texts[text_idx].text;
	input.fontsize = texts[text_idx].font;
	auto out = KLFBackend::getLatexFormula(input, settings);
	auto pm = QPixmap::fromImage(out.result);
	pm.setMask(pm.createMaskFromColor("white"));
	return pm;
}

void PicturePanel::open_project(QString fileName)
{
	tab2chart.clear();
	try {
		ifstream ifs(fileName.toStdString());
		nlohmann::json info;
		ifs >> info;

		chart_dialog->import(info);
		graph_dialog();
		mw->setWindowTitle(mw->windowTitle().chopped(3));
		// TODO
		// for (auto &latex : info["latex"]) {
		//  content()->texts.push_back(latex);
		//  content()->texts[text_idx++].pm = process_latex();
		//}
	}
	catch (std::exception &e) {
		QMessageBox::warning(this, "Import Error", e.what());
	}
}

void PicturePanel::save_project(QString filename)
{
	// FIXME save all charts
	ofstream ofs(filename.toStdString());
	auto js = nlohmann::json(*chart_dialog);

	// TODO
	// for (auto text : content()->texts)
	// js["latex"].push_back(text);

	ofs << setw(4) << js;
}

void PicturePanel::mark_unsaved()
{
	if (!mw->windowTitle().endsWith("[+]"))
		mw->setWindowTitle(mw->windowTitle() + "[+]");
}

void PicturePanel::graph_dialog()
{
	auto elems = chart_dialog->getElements();
	if (!elems.size())
		return;
	mark_unsaved();
	while (tabs->count()) {
		tabs->widget(0)->deleteLater();
		tabs->removeTab(0);
	}
	for (auto &[label, series] : elems)
		add_chart(label, series);
}

bool PictureTab::input_latex(QPointF location)
{
	QDialog dialog(this);
	QFormLayout form(&dialog);
	// Add some text above the fields
	form.addRow(new QLabel("Input latex"));

	auto text = (text_idx == -1) ? "" : texts[text_idx].text;
	auto font = (text_idx == -1) ? default_font : texts[text_idx].font;

	// Add the lineEdits with their respective labels
	auto lineEdit = new QLineEdit(text, &dialog);
	form.addRow("Text", lineEdit);

	auto doubleEdit = new QDoubleSpinBox(&dialog);
	doubleEdit->setValue(font);
	form.addRow("Font", doubleEdit);

	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
	                           Qt::Horizontal, &dialog);
	if (text_idx != -1) {
		auto delete_button = new QPushButton("Delete");
		buttonBox.addButton(delete_button, QDialogButtonBox::ActionRole);
		connect(delete_button, &QPushButton::released, this, [this, &buttonBox]() {
			texts.removeAt(text_idx);
			buttonBox.rejected();
		});
	}

	form.addRow(&buttonBox);
	connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	// Show the dialog as modal
	if (dialog.exec() == QDialog::Accepted) {
		if (text_idx == -1) {
			texts.append(Text{});
			text_idx = texts.size() - 1;
		}
		texts[text_idx].text = lineEdit->text();
		texts[text_idx].font = doubleEdit->value();
		texts[text_idx].coords = location;
		texts[text_idx].pm = process_latex();
		owner->mark_unsaved();
		return true;
	}
	return false;
}

void PictureTab::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton)
		return;

	mouse_pressed = false;

	if (owner->zoom_mode)
		chart()->zoomIn(QRectF{zoom_start, zoom_end}.normalized());
	else if (text_idx == -1)
		input_latex(widget2chart(e->pos()));
	viewport()->update();
}

void PictureTab::mouseMoveEvent(QMouseEvent *e)
{
	auto coords = widget2chart(e->pos());
	owner->mw->control_panel->coords_text->setText(
	  QString::number(coords.x(), 'g', 4) + ';' +
	  QString::number(coords.y(), 'g', 4));

	if (!mouse_pressed)
		return;

	if (owner->zoom_mode)
		zoom_end = e->pos();
	else if (text_idx != -1)
		texts[text_idx].coords = widget2chart(e->pos() - mouse_text_offset);
	else
		return;

	viewport()->update();
}

void PicturePanel::zoomReset()
{
	((PictureTab *)tabs->currentWidget())->chart()->zoomReset();
}

QPointF PictureTab::widget2chart(QPoint coord)
{
	return chart()->mapToValue(coord);
}

QPoint PictureTab::chart2widget(QPointF coord)
{
	auto float_point = chart()->mapToPosition(coord);
	return QPoint(float_point.x(), float_point.y());
}
