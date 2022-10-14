#include <QGridLayout>
#include <QApplication>
#include "widgets.h"

MainWindow::MainWindow(QWidget *parent, const QVector<SeriesVec> &base,
                       const QVector<FormulasVec> &c)
  : QWidget(parent)
{
	control_panel = new ControlPanel(this);
	setWindowTitle(tr("Drawing"));
	auto layout = new QVBoxLayout(this);
	auto scroll_area = new QScrollArea(this);
	graph_panel = new GraphChoicePanel(scroll_area, this, base, c);
	scroll_area->setWidget(graph_panel);
	scroll_area->setWidgetResizable(true);
	layout->addWidget(scroll_area);
	layout->addWidget(control_panel);
	setLayout(layout);
	resize(600 * (1 + (c.size() > 1)), 700);
}
