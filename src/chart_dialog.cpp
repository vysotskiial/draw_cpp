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
using namespace nlohmann;

AuxVarItem::AuxVarItem(QVBoxLayout *o, const QString &n, const QString &f)
{
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

ComponentChoice::PairEdit::PairEdit(ComponentChoice *parent, QComboBox *base)
  : x(new QComboBox(parent)), y(new QComboBox(parent)),
    button(new QPushButton(QIcon(im_path + "/images/"
                                           "delete.png"),
                           ""))
{
	if (!base) {
		x->addItem("t");
		y->addItem("t");
		return;
	}
	for (auto i = 0; i < base->count(); i++) {
		x->addItem(base->itemText(i));
		y->addItem(base->itemText(i));
	}
}

void ComponentChoice::add_pair()
{
	auto row_layout = new QHBoxLayout();
	edits.push_back({this, edits.size() ? edits.front().x : nullptr});
	row_layout->addWidget(new QLabel("x: "));
	row_layout->addWidget(edits.back().x);
	row_layout->addWidget(new QLabel("y: "));
	row_layout->addWidget(edits.back().y);
	row_layout->addStretch();
	row_layout->addWidget(edits.back().button);
	connect(edits.back().button, &QPushButton::released, [row_layout, this]() {
		if (edits.size() == 1)
			return;
		layout->removeItem(row_layout);
		edits.pop_back();
	});
	layout->addLayout(row_layout);
}

void ComponentChoice::comp_removed()
{
	for (auto &edit : edits) {
		edit.x->removeItem(edit.x->count() - 1);
		edit.y->removeItem(edit.y->count() - 1);
	}
}

void ComponentChoice::comp_added(const QString &num)
{
	for (auto &edit : edits) {
		edit.x->addItem("x" + num);
		edit.y->addItem("x" + num);
	}
}

ComponentChoice::ComponentChoice(QWidget *parent): QWidget(parent)
{
	layout = new QVBoxLayout(this);
	add_pair();
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
	comp_choice->comp_added(num);
	init_button->setStyleSheet("QPushButton {color: red;}");
	init_value.clear();
}

void ChartDialogTab::comp_removed()
{
	comp_choice->comp_removed();
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
	steps_layout->addWidget(new QLabel("Step size:"));
	steps_layout->addWidget(step_edit);
	steps_layout->addWidget(new QLabel("Steps num:"));
	steps_layout->addWidget(steps_num_edit);
	form->addRow(steps_layout);

	comp_choice = new ComponentChoice(this);
	form->addRow("Axis:", comp_choice);
	auto add_axis = new QPushButton("Add Axis", this);
	connect(add_axis, &QPushButton::released,
	        [this]() { comp_choice->add_pair(); });
	form->addRow(add_axis);

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
	auto i = 1z;
	QString var_name;
	try {
		for (auto &eq : equations_edit->get()) {
			var_name = QString("%1%2").arg(default_variable).arg(i);
			vp[i++] = eq.toStdString();
		}
		for (auto &[name, eq] : aux_edit->get()) {
			var_name = name;
			vp[name.toStdString()] = eq.toStdString();
		}
	}
	catch (exception &e) {
		auto err_msg = QString("Wrong equation format for variable %1:\n %2")
		                 .arg(var_name)
		                 .arg(e.what());
		QMessageBox::warning(this, "Error", err_msg);
		return false;
	}

	return true;
}

void ChartDialogTab::get(SeriesInfo &info)
{
	if (!init_value.size()) {
		QMessageBox::warning(this, "Error", "Initial conditions not set");
		return;
	}
	auto step = step_edit->value();
	auto steps_num = steps_num_edit->value();
	EulerSolver solver(step, steps_num, init_value, vp);
	decltype(solver.solve()) solution;
	try {
		solution = solver.solve();
	}
	catch (exception &e) {
		QMessageBox::warning(this, "Error",
		                     "Wrong equation format: " + QString(e.what()));
		return;
	}

	for (auto i = 0; i < comp_choice->comps_size(); i++) {
		auto series = new QSplineSeries();
		auto pen = series->pen();
		pen.setWidth(2);
		pen.setColor(color);
		series->setPen(pen);

		auto comp_pair = comp_choice->getComps(i);
		auto x_comp = comp_pair.x_comp;
		auto y_comp = comp_pair.y_comp;

		auto get_value = [&solution, step](int comp, int k) {
			return (comp == -1) ? k * step : solution[k][comp];
		};
		for (auto k = 0u; k < solution.size(); k++)
			series->append({get_value(x_comp, k), get_value(y_comp, k)});

		auto comp_name = [](int comp) {
			return (comp == -1) ? "t" : "x_" + QString::number(comp);
		};
		info[comp_name(x_comp) + "/" + comp_name(y_comp)].append(series);
	}
}

ChartDialogTab::operator json() const
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

	result["x_comp"] = comp_choice->getComps(0).x_comp;
	result["y_comp"] = comp_choice->getComps(0).y_comp;

	result["color"] = color.name().toStdString();
	return result;
}

void ChartDialogTab::from_json(const json &j)
{
	try {
		for (auto &[var, formula] : j.at("aux_vars").get<json::object_t>())
			aux_edit->add(QString::fromStdString(var),
			              QString::fromStdString(formula));
	}
	catch (std::exception &) { // not haveing aux variables isn't an error
	}

	for (auto &eq : j["equations"])
		equations_edit->add(QString::fromStdString(eq));

	for (auto i = 0; auto &init : j["inits"])
		init_edit->edits[i++]->setText(QString::number(init.get<double>()));

	init_value = init_edit->get();
	init_button->setStyleSheet("QPushButton {color: black;}");

	step_edit->setValue(j["step"]);
	steps_num_edit->setValue(j["steps_num"]);

	color = QColor(j["color"].get<string>().c_str());
	QPixmap pixmap(100, 100);
	pixmap.fill(color);
	color_button->setIcon(QIcon(pixmap));
}

SeriesInfo ChartDialog::getElements()
{
	if (!just_imported && exec() != QDialog::Accepted)
		return {};

	just_imported = false;
	SeriesInfo result;
	for (auto &tab : tabs) {
		if (!tab->check())
			return {};
		tab->get(result);
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

ChartDialog::operator json() const
{
	json result;
	for (auto &tab : tabs)
		result["charts"].push_back(*tab);

	return result;
}

void ChartDialog::import(const json &info)
{
	for (auto i = 0; i < tabs.size(); i++)
		rm_tab();

	for (auto &chart : info["charts"]) {
		add_tab();
		tabs.back()->from_json(chart);
	}

	just_imported = true;
}
