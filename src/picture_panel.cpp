#include "widgets.h"
#include <QPainter>
#include <QLayout>
#include <QMouseEvent>
#include <QChart>
#include <QBitmap>
#include <QFileDialog>
#include <QObject>
#include <klfbackend.h>
#include <qnamespace.h>

using namespace QtCharts;

PicturePanel::PicturePanel(MainWindow *parent, QChart *c): QChartView(parent)
{
	setMouseTracking(true);

	QPalette pal{};
	pal.setColor(QPalette::Window, Qt::white);
	setAutoFillBackground(true);
	setPalette(pal);

	setRenderHint(QPainter::SmoothPixmapTransform);
	setRenderHint(QPainter::Antialiasing);

	setChart(c);
}

void PicturePanel::paintEvent(QPaintEvent *e)
{
	if (mouse_pressed) {
		auto painter = QPainter(viewport());
		painter.drawPixmap(0, 0, cached_graph);
		painter.setPen(Qt::PenStyle::DashLine);
		painter.drawRect({QPointF(zoom_start), zoom_end});
		return;
	}

	QChartView::paintEvent(e); // has to be before painter, segfaults otherwise
	auto painter = QPainter(viewport());
	for (auto &text : texts) {
		auto coords = chart2widget({text.x, text.y});
		painter.drawPixmap(coords.x(), coords.y(), text.pm);
	}
	have_cache = false;
}

void PicturePanel::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::MouseButton::LeftButton) {
		zoom_start = e->pos();
		cached_graph = grab();
		mouse_pressed = true;
		have_cache = true;
	}
}

void PicturePanel::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::MouseButton::LeftButton) {
		mouse_pressed = false;
		chart()->zoomIn({zoom_start, zoom_end});
		viewport()->update();
	}
}

void PicturePanel::mouseMoveEvent(QMouseEvent *e)
{
	auto coords = widget2chart(e->pos());
	auto main_window = (MainWindow *)this->parent();
	main_window->control_panel->coords_text->setText(
	  QString::number(coords.x(), 'g', 4) + ';' +
	  QString::number(coords.y(), 'g', 4));
	if (mouse_pressed) {
		zoom_end = e->pos();
		viewport()->update();
	}
}

QPointF PicturePanel::widget2chart(QPoint coord)
{
	return chart()->mapToValue(coord);
}

QPoint PicturePanel::chart2widget(QPointF coord)
{
	auto float_point = chart()->mapToPosition(coord);
	return QPoint(float_point.x(), float_point.y());
}

MainWindow::MainWindow(QWidget *parent, QChart *c)
{
	control_panel = new ControlPanel(this);
	picture_panel = new PicturePanel(this, c);
	setWindowTitle(tr("Drawing"));
	auto layout = new QVBoxLayout(this);
	layout->addWidget(picture_panel, 1);
	layout->addWidget(control_panel);
	setLayout(layout);
	resize(700, 700);
}
