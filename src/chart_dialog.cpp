#include <QFormLayout>
#include <QColorDialog>
#include <QDialogButtonBox>
#include "widgets.h"

ChartDialogTab::ChartDialogTab(const SeriesElement &e, QWidget *parent)
  : QWidget(parent)
{
	auto form = new QFormLayout(this);

	// Add some text above the fields
	form->addRow(new QLabel("Input chart"));

	equations_edit = new QTextEdit(this);
	equations_edit->setPlainText(e.equations);
	equations_edit->setLineWrapMode(QTextEdit::NoWrap);
	form->addRow("Equations", equations_edit);

	init_edit = new QLineEdit(this);
	init_edit->setPlaceholderText("Comma separated, e.g. 1,2,3");
	init_edit->setText(e.init_cond);
	form->addRow("Initial conditions", init_edit);

	step_edit = new QDoubleSpinBox(this);
	step_edit->setDecimals(5);
	step_edit->setValue(e.step);
	steps_num_edit = new QSpinBox(this);
	steps_num_edit->setMaximum(10e8);
	steps_num_edit->setValue(e.step_num);
	auto steps_layout = new QHBoxLayout();
	steps_layout->addWidget(step_edit);
	steps_layout->addWidget(steps_num_edit);
	form->addRow("Step and steps number", steps_layout);

	x_comp_edit = new QSpinBox(this);
	x_comp_edit->setMinimum(-1);
	x_comp_edit->setValue(e.x_component);
	y_comp_edit = new QSpinBox(this);
	y_comp_edit->setMinimum(-1);
	y_comp_edit->setValue(e.y_component);
	auto comps_layout = new QHBoxLayout();
	comps_layout->addWidget(x_comp_edit);
	comps_layout->addWidget(y_comp_edit);
	form->addRow("State vector components", comps_layout);

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

std::optional<QVector<SeriesElement>> ChartDialog::getElements()
{
	if (exec() != QDialog::Accepted) {
		return {};
	}
	QVector<SeriesElement> elements;
	for (auto i = 0; i < tabs->count(); i++) {
		SeriesElement element;
		auto tab = dynamic_cast<ChartDialogTab *>(tabs->widget(i));
		element.equations = tab->equations_edit->toPlainText();
		element.x_component = tab->x_comp_edit->value();
		element.y_component = tab->y_comp_edit->value();
		element.color = tab->color;
		element.init_cond = tab->init_edit->text();
		element.step = tab->step_edit->value();
		element.step_num = tab->steps_num_edit->value();
		elements.append(element);
	}
	return elements;
}

ChartDialog::ChartDialog(const QVector<SeriesElement> &elements, QWidget *p)
  : QDialog(p)
{
	auto form = new QFormLayout(this);
	tabs = new QTabWidget(this);
	int i = 1;
	for (auto &e : elements) {
		tabs->addTab(new ChartDialogTab(e, this), "Chart " + QString::number(i));
	}
	form->addRow(tabs);

	auto add_rm_layout = new QHBoxLayout();
	add_button = new QPushButton("Add series", this);
	remove_button = new QPushButton("Remove series", this);
	add_rm_layout->addWidget(add_button);
	add_rm_layout->addWidget(remove_button);
	form->addRow(add_rm_layout);

	auto buttonBox =
	  new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
	                       Qt::Horizontal, this);
	form->addRow(buttonBox);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(add_button, &QPushButton::released, this, &ChartDialog::on_add);
	connect(remove_button, &QPushButton::released, this, &ChartDialog::on_remove);
}

void ChartDialog::on_add()
{
	tabs->addTab(new ChartDialogTab({}, this),
	             "Chart " + QString::number(tabs->count() + 1));
	tabs->setCurrentIndex(tabs->count() - 1);
}

void ChartDialog::on_remove()
{
	tabs->removeTab(tabs->currentIndex());
}
