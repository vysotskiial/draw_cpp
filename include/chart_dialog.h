#pragma once
#include <QString>
#include <QPixmap>
#include <QChart>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVector>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <optional>
#include "formula_processor.h"

using SeriesVec = QVector<QtCharts::QAbstractSeries *>;

class AuxVarItem : public QWidget {
	QLineEdit *name_edit;
	QLineEdit *formula_edit;

public:
	AuxVarItem(QVBoxLayout *owner);
	QString name() const { return name_edit->text(); }
	QString formula() const { return formula_edit->text(); }
};

class AuxVarEdit : public QWidget {
	QVBoxLayout *layout;

public:
	AuxVarEdit();
	std::map<QString, QString> get() const;
};

class ChartDialogTab;

class EquationsEdit : public QWidget {
	QVector<QLineEdit *> edits;

public:
	EquationsEdit(ChartDialogTab *parent);
	QVector<QString> get() const;
	int size() const { return edits.size(); }
};

class InitEdit : public QDialog {
	QVector<QLineEdit *> edits;

public:
	InitEdit(int state_size, QWidget *parent);
	std::vector<double> get() const;
};

class ChartDialogTab : public QWidget {
	AuxVarEdit *aux_edit;
	EquationsEdit *equations_edit;
	QDoubleSpinBox *step_edit;
	QSpinBox *steps_num_edit;
	QComboBox *x_comp_edit;
	QComboBox *y_comp_edit;

	std::vector<double> init_value;
	VectorProcessor vp;

	QColor color;

public:
	bool check();
	void comp_added(const QString &num);
	void comp_removed();
	QtCharts::QAbstractSeries *get() const;
	ChartDialogTab(QWidget *p);
};

class ChartDialog : public QDialog {
private:
	QVector<ChartDialogTab *> tabs;

public:
	std::optional<SeriesVec> getElements();
	ChartDialog(QWidget *p);
};
