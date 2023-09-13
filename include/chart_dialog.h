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
#include <nlohmann/json.hpp>
#include "formula_processor.h"

using SeriesVec = QVector<QtCharts::QAbstractSeries *>;

class AuxVarItem : public QWidget {
	QLineEdit *name_edit;
	QLineEdit *formula_edit;

public:
	AuxVarItem(QVBoxLayout *owner, const QString &n = "", const QString &f = "");
	QString name() const { return name_edit->text(); }
	QString formula() const { return formula_edit->text(); }
};

class AuxVarEdit : public QWidget {
	QVBoxLayout *layout;

public:
	std::map<QString, QString> get() const;
	void add(QString name, QString formula);
	AuxVarEdit();
};

class ChartDialogTab;

class EquationsEdit : public QWidget {
	ChartDialogTab *parent;
	QVBoxLayout *layout;
	QVector<QLineEdit *> edits;

public:
	void add(const QString &eq = "");
	QVector<QString> get() const;
	int size() const { return edits.size(); }
	EquationsEdit(ChartDialogTab *parent);
};

class InitEdit : public QDialog {
	QFormLayout *layout;
	QVector<QLineEdit *> edits;

public:
	InitEdit(QWidget *parent);
	std::vector<double> get() const;
	friend class ChartDialogTab;
};

class ChartDialogTab : public QWidget {
	QPushButton *init_button;
	AuxVarEdit *aux_edit;
	EquationsEdit *equations_edit;
	QDoubleSpinBox *step_edit;
	QSpinBox *steps_num_edit;
	QComboBox *x_comp_edit;
	QComboBox *y_comp_edit;
	InitEdit *init_edit;
	QPushButton *color_button;

	std::vector<double> init_value;
	VectorProcessor vp;

	QColor color;

public:
	bool check();
	void comp_added(const QString &num);
	void comp_removed();
	QtCharts::QAbstractSeries *get();
	nlohmann::json to_json() const;
	void from_json(nlohmann::json &j);

	ChartDialogTab(QWidget *p);
};

class ChartDialog : public QDialog {
private:
	bool just_imported{false};
	QTabWidget *tab_widget;
	QVector<ChartDialogTab *> tabs;
	void add_tab();
	void rm_tab();

public:
	std::optional<SeriesVec> getElements();
	void save(const std::string &filename) const;
	void import(const std::string &filename);

	ChartDialog(QWidget *p);
};
