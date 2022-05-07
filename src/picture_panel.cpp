#include <QPainter>
#include <QLayout>
#include <QFormLayout>
#include <QMouseEvent>
#include <QChart>
#include <QBitmap>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QTextEdit>
#include <QSplineSeries>
#include <QValueAxis>
#include <klfbackend.h>
#include <QApplication>
#include <QColorDialog>
#include <QScreen>
#include "widgets.h"
#include "solver.h"
#include "formula_processor.h"

using namespace std;
using namespace QtCharts;

QChart *make_chart(const vector<vector<double>> &vec, int i, int j,
                   QColor color, double step)
{
	auto chart = new QtCharts::QChart();
	chart->legend()->hide();

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

	auto x_axis = new QValueAxis();
	x_axis->setLinePen(Qt::PenStyle::SolidLine);

	auto y_axis = new QValueAxis;
	y_axis->setLinePen(Qt::PenStyle::SolidLine);

	chart->addAxis(x_axis, Qt::AlignBottom);
	chart->addAxis(y_axis, Qt::AlignLeft);

	series->attachAxis(x_axis);
	series->attachAxis(y_axis);
	x_axis->applyNiceNumbers();
	y_axis->applyNiceNumbers();

	return chart;
}

PicturePanel::PicturePanel(MainWindow *parent, ChartElement c,
                           GraphChoicePanel *c_panel, bool _in_grid)
  : QChartView(c.chart, c_panel), mw(parent), choice_panel(c_panel),
    my_element(c), in_grid(_in_grid)
{
	setRubberBand(QChartView::RubberBand::NoRubberBand);
	setRenderHint(QPainter::Antialiasing);
	setMouseTracking(true);
	setMinimumSize(500, 500);

	bool ok = KLFBackend::detectSettings(&settings);
	if (!ok) {
		// vital program not found
		throw("error in your system: are latex,dvips and gs installed?");
	}

	input.mathmode = "\\begin{equation*} ... \\end{equation*}";
	input.preamble = "\\usepackage{amsmath}\n";
	input.dpi = 300;
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
	QDialog dialog(this);
	QFormLayout form(&dialog);

	// Add some text above the fields
	form.addRow(new QLabel("Input chart"));

	auto text_edit = new QTextEdit(&dialog);
	text_edit->setPlainText(my_element.equations);
	text_edit->setLineWrapMode(QTextEdit::NoWrap);
	form.addRow("Equations", text_edit);

	auto line_edit = new QLineEdit(&dialog);
	line_edit->setPlaceholderText("Comma separated, e.g. 1,2,3");
	form.addRow("Initial conditions", line_edit);

	auto step_edit = new QDoubleSpinBox(&dialog);
	step_edit->setValue(0.001);
	step_edit->setDecimals(5);
	auto steps_num_edit = new QSpinBox(&dialog);
	steps_num_edit->setMaximum(10e8);
	steps_num_edit->setValue(10000);
	auto steps_layout = new QHBoxLayout();
	steps_layout->addWidget(step_edit);
	steps_layout->addWidget(steps_num_edit);
	form.addRow("Step and steps number", steps_layout);

	auto x_comp_edit = new QSpinBox(&dialog);
	x_comp_edit->setMinimum(-1);
	auto y_comp_edit = new QSpinBox(&dialog);
	y_comp_edit->setMinimum(-1);
	auto comps_layout = new QHBoxLayout();
	comps_layout->addWidget(x_comp_edit);
	comps_layout->addWidget(y_comp_edit);
	form.addRow("State vector components", comps_layout);

	auto color_button = new QPushButton("Choose color", &dialog);
	QColor color = Qt::black;
	auto choose_color = [&color, &dialog]() {
		color = QColorDialog::getColor(Qt::black, &dialog, "Select color");
	};
	connect(color_button, &QPushButton::released, &dialog, choose_color);
	form.addRow(color_button);

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
	                           Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);
	connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	if (dialog.exec() == QDialog::Accepted) { // Solve and draw
		my_element.equations = text_edit->toPlainText();
		my_element.x_component = x_comp_edit->value();
		my_element.y_component = y_comp_edit->value();
		my_element.color = color;
		process_new_equations(line_edit->text(), step_edit->value(),
		                      steps_num_edit->value());
	}
}

void PicturePanel::process_new_equations(QString init, double step, int num)
{
	auto lst = init.split(",");
	std::vector<double> init_cond;
	for (auto x : lst)
		init_cond.push_back(x.toDouble());

	VectorProcessor vp(my_element.equations.toStdString());
	EulerSolver solver(step, num, init_cond, vp);
	auto solution = solver.solve();
	auto chart = make_chart(solution, my_element.x_component,
	                        my_element.y_component, my_element.color, step);
	delete this->chart();
	setChart(chart);
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
