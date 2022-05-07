#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsItem>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <QGridLayout>
#include <QDialog>
#include <optional>
#include <klfbackend.h>

class MainWindow;
class PicturePanel;

constexpr double default_font = 7;

QtCharts::QChart *make_chart(const std::vector<std::vector<double>> &vec, int i,
                             int j, double step = 0.);

struct Text {
	QPointF coords;
	QPixmap pm;
	QString text;
	double font{default_font};
};

struct ChartElement {
	QtCharts::QChart *chart;
	QString equations{};
	int x_component{0}; // Our equatoins can be more than 2d
	int y_component{1}; // so we specify which compoents to plot
	                    // -1 is to plot against time
};

class GraphChoicePanel : public QWidget {
	Q_OBJECT
	MainWindow *mw;
	QVector<PicturePanel *> pictures;
	std::optional<int> picture_idx; // Index of chosen panel
	QGridLayout *grid_layout;

	void fill_grid();

public:
	GraphChoicePanel(QWidget *parent, MainWindow *mw,
	                 QVector<ChartElement> charts);
	void save_widget(QString filename);
	void from_grid(PicturePanel *panel);
	bool zoom_text_switch();
	void reset_zoom();
	void to_grid();
	void change_graph();
};

class PicturePanel : public QtCharts::QChartView {
	Q_OBJECT

	MainWindow *mw;
	GraphChoicePanel *choice_panel;
	bool making_cache{false};
	QVector<Text> texts;
	ChartElement my_element;

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

	QPoint chart2widget(QPointF coord) const;
	QPointF widget2chart(QPoint coord) const;

	void find_text(QPoint pos); // check if there's latex text under mouse

	QPixmap process_latex();
	bool input_latex();
	void process_new_equations(QString init, double step, int num);

public:
	bool in_grid;
	PicturePanel(MainWindow *, ChartElement, GraphChoicePanel *,
	             bool _in_grid = false);
	bool switch_zoom() { return zoom_mode = !zoom_mode; }
	void graph_dialog();

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
	QPushButton *grid_button;
	QPushButton *zoom_button;
	QPushButton *unzoom_button;
	QPushButton *graph_button;

private slots:
	void on_save();
	void on_zoom();
	void on_unzoom();
	void on_grid();
	void on_graph();

public:
	ControlPanel(QWidget *parent = nullptr);
	QLabel *coords_text;
};

class MainWindow : public QWidget {
	Q_OBJECT

public:
	ControlPanel *control_panel;
	GraphChoicePanel *graph_panel;
	MainWindow(QWidget *parent, QVector<ChartElement> c);
};
