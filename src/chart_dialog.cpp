#include <QColorDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>
#include <QSplineSeries>
#include "chart_dialog.h"
#include "solver.h"

using namespace std;
using namespace QtCharts;

AuxVarItem::AuxVarItem(QVBoxLayout *o)
{
	static QString im_path = IMAGES_PATH;
	auto layout = new QHBoxLayout(this);
	name_edit = new QLineEdit("");
	formula_edit = new QLineEdit("");
	layout->addWidget(name_edit);
	layout->addWidget(new QLabel(" = "));
	layout->addWidget(formula_edit);
	auto rm_button = new QPushButton(QIcon(im_path + "/images/delete.png"), "");
	layout->addWidget(rm_button);
	connect(rm_button, &QPushButton::released, o,
	        [o, this]() { o->removeWidget(this); });
}

AuxVarEdit::AuxVarEdit()
{
	layout = new QVBoxLayout(this);
	auto add_button = new QPushButton("Add auxilliary");
	layout->addWidget(add_button);

	connect(add_button, &QPushButton::released, this,
	        [this]() { layout->addWidget(new AuxVarItem(layout)); });
}

map<QString, QString> AuxVarEdit::get() const
{
	map<QString, QString> result;
	for (auto i = 1; i < layout->count(); i++) {
		auto item = layout->itemAt(i);
		auto variable = dynamic_cast<AuxVarItem *>(item->widget());
		result[variable->name()] = variable->formula();
	}
	return result;
}

EquationsEdit::EquationsEdit()
{
	auto layout = new QFormLayout(this);
	auto add_button = new QPushButton("Add component");
	auto rm_button = new QPushButton("Remove component");
	layout->addRow(add_button, rm_button);
	connect(add_button, &QPushButton::released, this, [layout, this]() {
		auto new_edit = new QLineEdit("");
		edits.append(new_edit);
		layout->addRow("dx" + QString::number(layout->rowCount()), new_edit);
	});
	connect(rm_button, &QPushButton::released, this, [layout, this]() {
		if (layout->rowCount() > 1) {
			layout->removeRow(layout->rowCount() - 1);
			edits.pop_back();
		}
	});
}

QVector<QString> EquationsEdit::get() const
{
	QVector<QString> result;
	transform(begin(edits), end(edits), back_inserter(result),
	          [](auto edit) { return edit->text(); });

	return result;
}

InitEdit::InitEdit(int state_size, QWidget *parent): QDialog(parent)
{
	auto layout = new QFormLayout(this);
	for (auto i = 1; i <= state_size; i++) {
		auto edit = new QLineEdit("");
		edit->setValidator(new QDoubleValidator(-1000, 1000, 3, edit));
		layout->addRow("x" + QString::number(i) + "(0) = ", edit);
		edits.append(edit);
	}

	auto buttonBox =
	  new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
	                       Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout->addRow(buttonBox);
}

vector<double> InitEdit::get() const
{
	vector<double> result;
	transform(begin(edits), end(edits), back_inserter(result),
	          [](auto edit) { return edit->text().toInt(); });
	return result;
}

ChartDialogTab::ChartDialogTab(QWidget *parent): QWidget(parent)
{
	auto form = new QFormLayout(this);
	aux_edit = new AuxVarEdit;
	equations_edit = new EquationsEdit;
	form->addRow(aux_edit);
	form->addRow(equations_edit);

	auto init_button = new QPushButton("Initial conditions");
	connect(init_button, &QPushButton::released, this, [this]() {
		auto init_edit = InitEdit(equations_edit->size(), this);
		if (init_edit.exec() == QDialog::Accepted) {
			init_value = init_edit.get();
		}
	});
	form->addRow(init_button);

	step_edit = new QDoubleSpinBox(this);
	step_edit->setDecimals(5);
	steps_num_edit = new QSpinBox(this);
	steps_num_edit->setMaximum(10e8);
	auto steps_layout = new QHBoxLayout();
	steps_layout->addWidget(step_edit);
	steps_layout->addWidget(steps_num_edit);
	form->addRow("Step and steps number", steps_layout);

	x_comp_edit = new QSpinBox(this);
	y_comp_edit = new QSpinBox(this);
	auto comps_layout = new QHBoxLayout();
	comps_layout->addWidget(x_comp_edit);
	comps_layout->addWidget(y_comp_edit);
	form->addRow("State vector components", comps_layout);

	QPixmap icon_map(100, 100);
	icon_map.fill(color);
	auto color_button = new QPushButton(QIcon(icon_map), "Choose color", this);
	auto choose_color = [color_button, this]() {
		color = QColorDialog::getColor(Qt::black, this, "Select color");
		QPixmap pixmap(100, 100);
		pixmap.fill(color);
		color_button->setIcon(QIcon(pixmap));
	};
	connect(color_button, &QPushButton::released, this, choose_color);
	form->addRow(color_button);
}

bool ChartDialogTab::check()
{
	vp = {};
	try {
		for (auto &eq : equations_edit->get())
			vp.add_comp(eq.toStdString());
		for (auto &[name, eq] : aux_edit->get())
			vp[name.toStdString()] = eq.toStdString();
	}
	catch (exception &e) {
		// TODO location of error
		QMessageBox::warning(this, "Error", "Wrong equation format");
		return false;
	}

	return true;
}

QAbstractSeries *ChartDialogTab::get() const
{
	auto step = step_edit->value();
	auto steps_num = steps_num_edit->value();
	EulerSolver solver(step, steps_num, init_value, vp);
	auto solution = solver.solve();
	auto series = new QSplineSeries();

	auto pen = series->pen();
	pen.setWidth(2);
	pen.setColor(color);
	series->setPen(pen);

	auto x_comp = x_comp_edit->value();
	auto y_comp = y_comp_edit->value();

	auto get_value = [&solution, step](int comp, int k) {
		return (comp == -1) ? k * step : solution[k][comp];
	};
	for (auto k = 0u; k < solution.size(); k++)
		series->append({get_value(x_comp, k), get_value(y_comp, k)});

	return series;
}

std::optional<SeriesVec> ChartDialog::getElements()
{
	if (exec() != QDialog::Accepted) {
		return {};
	}
	for (auto &tab : tabs)
		if (!tab->check())
			return {};

	SeriesVec result;
	transform(begin(tabs), end(tabs), back_inserter(result),
	          [](auto tab) { return tab->get(); });
	return result;
}

ChartDialog::ChartDialog(QWidget *p): QDialog(p)
{
	auto form = new QFormLayout(this);
	auto tab_widget = new QTabWidget(this);
	form->addRow(tab_widget);

	auto add_rm_layout = new QHBoxLayout();
	auto add_button = new QPushButton("Add series", this);
	auto remove_button = new QPushButton("Remove series", this);
	add_rm_layout->addWidget(add_button);
	add_rm_layout->addWidget(remove_button);
	form->addRow(add_rm_layout);

	auto buttonBox =
	  new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
	                       Qt::Horizontal, this);
	form->addRow(buttonBox);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(add_button, &QPushButton::released, this, [tab_widget, this]() {
		tabs.push_back(new ChartDialogTab(this));
		tab_widget->addTab(tabs.back(), "Chart " + QString::number(tabs.size()));
		tab_widget->setCurrentIndex(tabs.size() - 1);
	});
	connect(remove_button, &QPushButton::released, this, [tab_widget, this]() {
		tabs.erase(begin(tabs) + tab_widget->currentIndex());
		tab_widget->removeTab(tab_widget->currentIndex());
	});
}
