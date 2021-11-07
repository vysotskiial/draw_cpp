#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsItem>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <klfbackend.h>

class MainWindow;
constexpr double default_font = 7;

struct Text {
	QPointF coords;
	QPixmap pm;
	QString text;
	double font{default_font};
};

class PicturePanel : public QtCharts::QChartView {
	Q_OBJECT

	bool making_cache{false};
	QVector<Text> texts;

	bool zoom_mode{true};
	int text_idx{-1};         // index of text under mouse cursor
	QPoint mouse_text_offset; // Difference between mouse position when clicked on
	                          // text and text pixmap top left corner

	bool mouse_pressed{false};
	QPoint zoom_start;
	QPoint zoom_end;

	QPixmap cached_graph;

	// For latex processing
	KLFBackend::klfSettings settings;
	KLFBackend::klfInput input;

	QPoint chart2widget(QPointF coord) const;
	QPointF widget2chart(QPoint coord) const;

	void find_text(QPoint pos); // check if there's latex text under mouse

	QPixmap process_latex();
	bool input_latex();

public:
	PicturePanel(MainWindow *, QtCharts::QChart *);
	void switch_zoom() { zoom_mode = !zoom_mode; }

protected:
	void mouseMoveEvent(QMouseEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;
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

