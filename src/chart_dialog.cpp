#include <QColorDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>
#include <QSplineSeries>
#include <fstream>
#include "chart_dialog.h"
#include "solver.h"

using namespace std;
using namespace QtCharts;
using namespace nlohmann;

AuxVarItem::AuxVarItem(QVBoxLayout *o, const QString &n, const QString &f)
{
	static QString im_path = IMAGES_PATH;
	auto layout = new QHBoxLayout(this);
	name_edit = new QLineEdit(n);
	formula_edit = new QLineEdit(f);
	layout->addWidget(name_edit, 2);
	layout->addWidget(new QLabel("="));
	layout->addWidget(formula_edit, 6);
	auto rm_button = new QPushButton(QIcon(im_path + "/images/delete.png"), "");
	layout->addWidget(rm_button);
	connect(rm_button, &QPushButton::released, o, [o, this]() {
		o->removeWidget(this);
		delete this;
	});
}

AuxVarEdit::AuxVarEdit()
{
	layout = new QVBoxLayout(this);
	auto add_button = new QPushButton("Add auxilliary");
	layout->addWidget(add_button);

	connect(add_button, &QPushButton::released, this,
	        [this]() { layout->addWidget(new AuxVarItem(layout)); });
}

void AuxVarEdit::add(QString name, QString formula)
{
	layout->addWidget(new AuxVarItem(layout, name, formula));
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

EquationsEdit::EquationsEdit(ChartDialogTab *p): parent(p)
{
	layout = new QVBoxLayout(this);
	auto add_button = new QPushButton("Add component");
	auto rm_button = new QPushButton("Remove component");
	auto but_layout = new QHBoxLayout;
	but_layout->addWidget(add_button);
	but_layout->addWidget(rm_button);
	layout->addLayout(but_layout);
	connect(add_button, &QPushButton::released, this, [this]() { add(); });
	connect(rm_button, &QPushButton::released, this, [this]() {
		if (layout->count() > 1) {
			layout->removeItem(layout->itemAt(layout->count() - 1));
			edits.pop_back();
			parent->comp_removed();
		}
	});
}

void EquationsEdit::add(const QString &eq)
{
	auto edit_layout = new QHBoxLayout;
	auto new_edit = new QLineEdit(eq);
	edits.append(new_edit);
	auto idx = QString::number(layout->count());
	edit_layout->addWidget(new QLabel("dx" + idx + " = "), 1);
	edit_layout->addWidget(new_edit, 5);
	layout->addLayout(edit_layout);
	parent->comp_added(idx);
}

QVector<QString> EquationsEdit::get() const
{
	QVector<QString> result;
	transform(begin(edits), end(edits), back_inserter(result),
	          [](auto edit) { return edit->text(); });

	return result;
}

InitEdit::InitEdit(QWidget *parent): QDialog(parent)
{
	layout = new QFormLayout(this);

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
	          [](auto edit) { return edit->text().toDouble(); });
	return result;
}

void ChartDialogTab::comp_added(const QString &num)
{
	auto comp = "x" + num;
	init_edit->edits.push_back(new QLineEdit);
	init_edit->layout->insertRow(init_edit->layout->rowCount() - 1,
	                             comp + "(0) = ", init_edit->edits.back());
	x_comp_edit->addItem(comp);
	y_comp_edit->addItem(comp);
	init_button->setStyleSheet("QPushButton {color: red;}");
	init_value.clear();
}

void ChartDialogTab::comp_removed()
{
	x_comp_edit->removeItem(x_comp_edit->count() - 1);
	y_comp_edit->removeItem(y_comp_edit->count() - 1);
	delete init_edit->edits.back();
	init_edit->edits.pop_back();
	init_edit->layout->removeRow(init_edit->layout->rowCount() - 2);
	init_button->setStyleSheet("QPushButton {color: red;}");
	init_value.clear();
}

ChartDialogTab::ChartDialogTab(QWidget *parent): QWidget(parent)
{
	auto form = new QFormLayout(this);
	aux_edit = new AuxVarEdit;
	equations_edit = new EquationsEdit(this);
	form->addRow(aux_edit);
	form->addRow(equations_edit);
	init_edit = new InitEdit(this);

	init_button = new QPushButton("Initial conditions");
	init_button->setStyleSheet("QPushButton {color: red;}");

	connect(init_button, &QPushButton::released, this, [this]() {
		if (init_edit->exec() == QDialog::Accepted) {
			init_value = init_edit->get();
			init_button->setStyleSheet("QPushButton {color: black;}");
		}
	});
	form->addRow(init_button);

	step_edit = new QDoubleSpinBox(this);
	step_edit->setSingleStep(0.001);
	step_edit->setDecimals(5);
	step_edit->setValue(0.001);
	steps_num_edit = new QSpinBox(this);
	steps_num_edit->setSingleStep(1000);
	steps_num_edit->setMaximum(1e9);
	steps_num_edit->setValue(1e4);
	auto steps_layout = new QHBoxLayout();
	steps_layout->addWidget(step_edit);
	steps_layout->addWidget(steps_num_edit);
	form->addRow("Step and step number:", steps_layout);

	x_comp_edit = new QComboBox(this);
	y_comp_edit = new QComboBox(this);
	x_comp_edit->addItem("t");
	y_comp_edit->addItem("t");
	auto comps_layout = new QHBoxLayout();
	comps_layout->addWidget(new QLabel("x"));
	comps_layout->addWidget(x_comp_edit);
	comps_layout->addWidget(new QLabel("y"));
	comps_layout->addWidget(y_comp_edit);
	form->addRow("Axis:", comps_layout);

	QPixmap icon_map(100, 100);
	icon_map.fill(color);
	color_button = new QPushButton(QIcon(icon_map), "Choose color", this);
	auto choose_color = [this]() {
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

QAbstractSeries *ChartDialogTab::get()
{
	if (!init_value.size()) {
		QMessageBox::warning(this, "Error", "Initial conditions not set");
		return nullptr;
	}
	auto step = step_edit->value();
	auto steps_num = steps_num_edit->value();
	EulerSolver solver(step, steps_num, init_value, vp);
	decltype(solver.solve()) solution;
	try {
		solution = solver.solve();
	}
	catch (exception &e) {
		QMessageBox::warning(this, "Error", "Wrong equation format");
		return nullptr;
	}
	auto series = new QSplineSeries();

	auto pen = series->pen();
	pen.setWidth(2);
	pen.setColor(color);
	series->setPen(pen);

	auto x_comp = x_comp_edit->currentIndex() - 1;
	auto y_comp = y_comp_edit->currentIndex() - 1;

	auto get_value = [&solution, step](int comp, int k) {
		return (comp == -1) ? k * step : solution[k][comp];
	};
	for (auto k = 0u; k < solution.size(); k++)
		series->append({get_value(x_comp, k), get_value(y_comp, k)});

	return series;
}

json ChartDialogTab::to_json() const
{
	json result;

	for (auto &[name, formula] : aux_edit->get())
		result["aux_vars"][name.toStdString()] = formula.toStdString();

	for (auto &equation : equations_edit->get())
		result["equations"].push_back(equation.toStdString());

	for (auto init : init_edit->get())
		result["inits"].push_back(init);

	result["step"] = step_edit->value();
	result["steps_num"] = steps_num_edit->value();

	result["x_comp"] = x_comp_edit->currentIndex();
	result["y_comp"] = y_comp_edit->currentIndex();

	result["color"] = color.name().toStdString();
	return result;
}

void ChartDialogTab::from_json(json &j)
{
	for (auto &[var, formula] : j["aux_vars"].items())
		aux_edit->add(QString::fromStdString(var), QString::fromStdString(formula));

	for (auto &eq : j["equations"])
		equations_edit->add(QString::fromStdString(eq));

	for (auto i = 0; auto &init : j["inits"])
		init_edit->edits[i++]->setText(QString::number(init.get<double>()));

	init_value = init_edit->get();
	init_button->setStyleSheet("QPushButton {color: black;}");

	step_edit->setValue(j["step"]);
	steps_num_edit->setValue(j["steps_num"]);

	x_comp_edit->setCurrentIndex(j["x_comp"].get<int>());
	y_comp_edit->setCurrentIndex(j["y_comp"].get<int>());

	color = QColor(j["color"].get<string>().c_str());
	QPixmap pixmap(100, 100);
	pixmap.fill(color);
	color_button->setIcon(QIcon(pixmap));
}

std::optional<SeriesVec> ChartDialog::getElements()
{
	if (!just_imported && exec() != QDialog::Accepted)
		return {};

	just_imported = false;
	SeriesVec result;
	for (auto &tab : tabs) {
		if (!tab->check())
			return {};
		auto series = tab->get();
		if (!series)
			return {};
		result.push_back(series);
	}

	return result;
}

ChartDialog::ChartDialog(QWidget *p): QDialog(p)
{
	auto form = new QFormLayout(this);
	tab_widget = new QTabWidget(this);
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
	connect(add_button, &QPushButton::released, this, &ChartDialog::add_tab);
	connect(remove_button, &QPushButton::released, this, &ChartDialog::rm_tab);
}

void ChartDialog::add_tab()
{
	tabs.push_back(new ChartDialogTab(this));
	tab_widget->addTab(tabs.back(), "Chart " + QString::number(tabs.size()));
	tab_widget->setCurrentIndex(tabs.size() - 1);
}

void ChartDialog::rm_tab()
{
	auto tab = tabs[tab_widget->currentIndex()];
	tabs.erase(begin(tabs) + tab_widget->currentIndex());
	tab_widget->removeTab(tab_widget->currentIndex());
	delete tab;
}

void ChartDialog::save(const string &filename) const
{
	ofstream ostr(filename);
	json result;
	for (auto &tab : tabs)
		result["charts"].push_back(tab->to_json());

	ostr << setw(4) << result;
}

void ChartDialog::import(const string &filename)
{
	ifstream ifs(filename);
	json info;
	ifs >> info;

	for (auto i = 0; i < tabs.size(); i++)
		rm_tab();

	for (auto &chart : info["charts"]) {
		add_tab();
		tabs.back()->from_json(chart);
	}

	just_imported = true;
}
