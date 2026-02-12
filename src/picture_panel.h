#pragma once
#include <QString>
#include <QPixmap>
#include <QChartView>
#include <chart_dialog.h>
#include <klfbackend.h>

class MainWindow;
class PictureTab;
constexpr double default_font = 7;

struct Text {
	QPointF coords;
	QPixmap pm;
	QString text;
	double font{default_font};

	operator nlohmann::json() const
	{
		nlohmann::json data;
		data["x"] = coords.x();
		data["y"] = coords.y();
		data["text"] = text.toStdString();
		data["font"] = font;

		return data;
	}
	Text(const nlohmann::json &j)
	  : coords(j["x"], j["y"]), text(j["text"].get<std::string>().c_str()),
	    font(j["font"])
	{
	}
	Text() = default;
};

class PicturePanel : public QWidget {
	MainWindow *mw;
	QTabWidget *tabs;
	QMap<int, PictureTab *> tab2chart;

	bool zoom_mode{false};

	bool draw_grid; // TODO maybe checkmark in dialog

	ChartDialog *chart_dialog;

	void draw_new_equations();

	void mark_unsaved();
	void add_chart(QString label,
	               const QVector<QtCharts::QAbstractSeries *> &series);

public:
	PicturePanel(MainWindow *);
	bool switch_zoom() { return zoom_mode = !zoom_mode; }
	void graph_dialog();
	void open_project(QString filename);
	void save_project(QString filename);
	void zoomReset();
	friend class PictureTab;
};

class PictureTab : public QtCharts::QChartView {
private:
	PicturePanel *owner;

	bool making_cache{false};

	// For latex processing
	KLFBackend::klfSettings settings;
	KLFBackend::klfInput input;
	QPixmap process_latex();
	bool input_latex(QPointF location);
	QVector<Text> texts;
	int text_idx{-1};         // index of text under mouse cursor
	QPoint mouse_text_offset; // Difference between mouse position when clicked
	                          // on text and text pixmap top left corner

	bool mouse_pressed{false};
	QPoint zoom_start;
	QPoint zoom_end;

	QPixmap cached_graph;

	QPoint chart2widget(QPointF coord);
	QPointF widget2chart(QPoint coord);
	void find_text(QPoint pos); // check if there's latex text under mouse

protected:
	void mouseMoveEvent(QMouseEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;
	void paintEvent(QPaintEvent *) override;

public:
	PictureTab(PicturePanel *o);
};
