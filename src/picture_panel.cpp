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
#include <exception>
#include "widgets.h"
#include "solver.h"
#include "formula_processor.h"

using namespace std;
using namespace QtCharts;

QChart *make_new_chart()
{
	auto *new_chart = new QChart();
	new_chart->legend()->hide();

	auto x_axis = new QValueAxis();
	x_axis->setLinePen(Qt::PenStyle::SolidLine);

	auto y_axis = new QValueAxis;
	y_axis->setLinePen(Qt::PenStyle::SolidLine);

	return new_chart;
}

QChart *add_series(QChart *chart, const vector<vector<double>> &vec, int i,
                   int j, QColor color, double step)
{
	auto series = new QSplineSeries();

	auto pen = series->pen();
	pen.setWidth(2);
	pen.setColor(color);
	series->setPen(pen);

	auto get_value = [&vec, step](int comp, int k) {
		return (comp == -1) ? k * step : vec[k][comp];
	};
	for (auto k = 0u; k < vec.size(); k++)
		series->append({get_value(i, k), get_value(j, k)});

	chart->addSeries(series);

	return chart;
}

PicturePanel::PicturePanel(MainWindow *parent, const SeriesVec &base,
                           const FormulasVec &formulas,
                           GraphChoicePanel *c_panel, bool _in_grid)
  : mw(parent), choice_panel(c_panel), baseline(base), my_elements(formulas),
    in_grid(_in_grid)
{
	process_new_equations();
	setRubberBand(QChartView::RubberBand::NoRubberBand);
	setRenderHint(QPainter::Antialiasing);
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

	chart_dialog = new ChartDialog(formulas, this);
}

void PicturePanel::paintEvent(QPaintEvent *e)
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

	if (mouse_pressed && zoom_mode) {
		painter.setPen(Qt::PenStyle::DashLine);
		painter.drawRect(QRect{zoom_start, zoom_end});
	}
}

void PicturePanel::mousePressEvent(QMouseEvent *e)
{
	if (in_grid)
		return;

	if (e->button() != Qt::LeftButton)
		return;

	mouse_pressed = true;

	if (zoom_mode) {
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

void PicturePanel::find_text(QPoint pos)
{
	text_idx = -1;
	for (auto i = 0; i < texts.size(); i++) {
		auto coords = chart2widget({texts[i].coords});
		auto text_rect = QRect(coords, texts[i].pm.size());
		if (text_rect.contains(pos)) {
			text_idx = i;
			mouse_text_offset = pos - text_rect.topLeft();
			return;
		}
	}
}

void PicturePanel::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (in_grid)
		return;

	if (zoom_mode)
		return;

	find_text(e->pos());
	if (text_idx == -1)
		return;

	if (!input_latex()) {
		return;
	}
	texts[text_idx].pm = process_latex();
	texts[text_idx].coords = widget2chart(e->pos() - mouse_text_offset);
	viewport()->update();
}

QPixmap PicturePanel::process_latex()
{
	input.latex = texts[text_idx].text;
	input.fontsize = texts[text_idx].font;
	auto out = KLFBackend::getLatexFormula(input, settings);
	auto pm = QPixmap::fromImage(out.result);
	pm.setMask(pm.createMaskFromColor("white"));
	return pm;
}

void PicturePanel::graph_dialog()
{
	auto elem = chart_dialog->getElements();
	if (elem.has_value()) {
		my_elements = elem.value();
		process_new_equations();
	}
}

void PicturePanel::process_new_equations()
{
	auto new_chart = make_new_chart();
	auto old_chart = this->chart();

	for (auto &s : baseline) {
		old_chart->removeSeries(s);
		new_chart->addSeries(s);
	}

	for (auto &e : my_elements) {
		auto lst = e.init_cond.split(",");
		std::vector<double> init_cond;
		for (auto x : lst)
			init_cond.push_back(x.toDouble());

		VectorProcessor vp(e.equations.toStdString());
		EulerSolver solver(e.step, e.step_num, init_cond, vp);
		auto solution = solver.solve();
		add_series(new_chart, solution, e.x_component, e.y_component, e.color,
		           e.step);
	}
	new_chart->createDefaultAxes();
	for (auto &axis : new_chart->axes()) {
		((QValueAxis *)axis)->applyNiceNumbers();
	}
	setChart(new_chart);
	delete old_chart;
}

bool PicturePanel::input_latex()
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
		return true;
	}
	return false;
}

void PicturePanel::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton)
		return;

	if (in_grid) {
		choice_panel->from_grid(this);
		return;
	}

	mouse_pressed = false;

	if (zoom_mode) {
		chart()->zoomIn(QRectF{zoom_start, zoom_end}.normalized());
	}
	else if (text_idx == -1) {
		auto ok = input_latex();
		if (ok) {
			texts[text_idx].coords = widget2chart(e->pos());
			texts[text_idx].pm = process_latex();
		}
	}
	else {
		return;
	}
	viewport()->update();
}

void PicturePanel::mouseMoveEvent(QMouseEvent *e)
{
	if (in_grid)
		return;
	auto coords = widget2chart(e->pos());
	mw->control_panel->coords_text->setText(QString::number(coords.x(), 'g', 4) +
	                                        ';' +
	                                        QString::number(coords.y(), 'g', 4));

	if (!mouse_pressed)
		return;

	if (zoom_mode)
		zoom_end = e->pos();
	else if (text_idx != -1)
		texts[text_idx].coords = widget2chart(e->pos() - mouse_text_offset);

	viewport()->update();
}

QPointF PicturePanel::widget2chart(QPoint coord) const
{
	return chart()->mapToValue(coord);
}

QPoint PicturePanel::chart2widget(QPointF coord) const
{
	auto float_point = chart()->mapToPosition(coord);
	return QPoint(float_point.x(), float_point.y());
}
