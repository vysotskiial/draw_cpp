#pragma once
#include <QString>
#include <QPixmap>
#include <QChartView>
#include <chart_dialog.h>
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
	MainWindow *mw;
	bool making_cache{false};
	bool draw_grid;
	SeriesVec baseline; // Series provided not through
	                    // formulas
	QVector<Text> texts;
	SeriesVec formula_lines;

	bool zoom_mode{false};
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

	ChartDialog *chart_dialog;

	QPoint chart2widget(QPointF coord) const;
	QPointF widget2chart(QPoint coord) const;

	void find_text(QPoint pos); // check if there's latex text under mouse

	QPixmap process_latex();
	bool input_latex();
	void draw_new_equations();

public:
	PicturePanel(MainWindow *, const SeriesVec &, bool grid);
	bool switch_zoom() { return zoom_mode = !zoom_mode; }
	void graph_dialog();
	void open_project(QString filename);
	void save_project(QString filename);

protected:
	void mouseMoveEvent(QMouseEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;
	void paintEvent(QPaintEvent *) override;
};
