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

using SeriesInfo = std::map<QString, QVector<QtCharts::QAbstractSeries *>>;

static const QString im_path = IMAGES_PATH;

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

class ComponentChoice : public QWidget {
	QVBoxLayout *layout;
	struct PairEdit {
		QComboBox *x;
		QComboBox *y;
		QPushButton *button;
		PairEdit(ComponentChoice *parent, QComboBox *base);
	};
	QVector<PairEdit> edits;

public:
	struct CompPair {
		int x_comp;
		int y_comp;
	};
	ComponentChoice(QWidget *parent);
	CompPair getComps(int pair_num)
	{
		return {edits[pair_num].x->currentIndex() - 1,
		        edits[pair_num].y->currentIndex() - 1};
	}
	int comps_size() const { return edits.size(); }
	void add_pair();
	void comp_added(const QString &num);
	void comp_removed();
};

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
	InitEdit *init_edit;
	QPushButton *color_button;
	ComponentChoice *comp_choice;

	std::vector<double> init_value;
	VectorProcessor vp;

	QColor color;

public:
	bool check();
	void comp_added(const QString &num);
	void comp_removed();
	void get(SeriesInfo &info);
	operator nlohmann::json() const;
	void from_json(const nlohmann::json &j);

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
	SeriesInfo getElements();
	operator nlohmann::json() const;
	void import(const nlohmann::json &j);

	ChartDialog(QWidget *p);
};
