#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsItem>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <klfbackend.h>

class MainWindow;
class ControlPanel;

class TextWindow : public QWidget {
	Q_OBJECT
	ControlPanel *parent_panel;
	QTableWidget *table;
	QPushButton *save_button;
	QPushButton *add_button;
	QPushButton *delete_button;

	KLFBackend::klfSettings settings;
	KLFBackend::klfInput input;

private slots:
	void on_save();

public:
	TextWindow(ControlPanel *parent = nullptr);
};

struct Text {
	double x;
	double y;
	QPixmap pm;
	QString text;
	double font;
};

class PicturePanel : public QtCharts::QChartView {
	Q_OBJECT
	bool mouse_pressed{false};
	QPoint zoom_start;
	QPoint zoom_end;

	bool have_cache{false};
	QPixmap cached_graph;

	QPoint chart2widget(QPointF coord);
	QPointF widget2chart(QPoint coord);

public:
	PicturePanel(MainWindow *parent, QtCharts::QChart *c);
	QVector<Text> texts;

protected:
	void mouseMoveEvent(QMouseEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void paintEvent(QPaintEvent *) override;
};

class ControlPanel : public QWidget {
	Q_OBJECT
	QPushButton *save_button;
	QPushButton *text_button;
	QPushButton *unzoom_button;

private slots:
	void on_save();
	void on_text();
	void on_unzoom();

public:
	ControlPanel(QWidget *parent = nullptr);
	QLabel *coords_text;
};

class MainWindow : public QWidget {
	Q_OBJECT

public:
	ControlPanel *control_panel;
	PicturePanel *picture_panel;
	MainWindow(QWidget *parent, QtCharts::QChart *c);
};

