#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsItem>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <QSplineSeries>
#include <QGridLayout>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <optional>
#include <klfbackend.h>

class MainWindow;
class PicturePanel;
using SeriesVec = QVector<QtCharts::QAbstractSeries *>;

class ControlPanel : public QWidget {
	Q_OBJECT
	MainWindow *mw;
	QPushButton *zoom_button;
	QString save_file{};

	void on_save();
	void save(const QString &filename);
	void on_open();

public:
	ControlPanel(MainWindow *parent = nullptr);
	QLabel *coords_text;
};

class MainWindow : public QWidget {
	Q_OBJECT

public:
	ControlPanel *control_panel;
	PicturePanel *picture_panel;
	MainWindow(QWidget *parent, const SeriesVec &baselines, bool grid = false);
};
