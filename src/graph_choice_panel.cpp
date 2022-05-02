#include <widgets.h>

GraphChoicePanel::GraphChoicePanel(QWidget *parent, MainWindow *main,
                                   QVector<ChartElement> charts)
  : QWidget(parent), mw(main)
{
	grid_layout = new QGridLayout();
	for (auto i = 0; i < charts.size(); i++) {
		auto picture = new PicturePanel(mw, charts[i], this, charts.size() > 1);
		pictures.append(picture);
	}

	if (charts.size() > 1) {
		grid_layout->setColumnStretch(0, 1);
		grid_layout->setColumnStretch(1, 1);
	}
	else {
		picture_idx = 0;
	}
	fill_grid();
	setLayout(grid_layout);
}

void GraphChoicePanel::fill_grid()
{
	auto i = 0;
	for (auto picture : pictures) {
		if (i % 2 == 0)
			grid_layout->setRowStretch(i / 2, 1);

		grid_layout->addWidget(picture, i / 2, i % 2, 1, 1);
		picture->show();
		i++;
	}
}

void GraphChoicePanel::to_grid()
{
	if (!picture_idx.has_value())
		return;

	if (pictures.size() == 1)
		return;

	pictures[picture_idx.value()]->in_grid = true;
	pictures[picture_idx.value()]->hide();
	for (auto picture : pictures)
		picture->show();
	picture_idx.reset();

	// fill_grid();
	mw->resize(1200, 700);
	update();
}

void GraphChoicePanel::from_grid(PicturePanel *choice)
{
	for (auto i = grid_layout->count(); i > 0; i--)
		grid_layout->itemAt(i - 1)->widget()->hide();

	choice->in_grid = false;
	choice->show();
	picture_idx = pictures.indexOf(choice);
	mw->resize(600, 700);
	update();
}

void GraphChoicePanel::save_widget(QString filename)
{
	if (!picture_idx.has_value())
		return;

	pictures[picture_idx.value()]->grab().save(filename);
}

bool GraphChoicePanel::zoom_text_switch()
{
	if (!picture_idx.has_value())
		return true;

	return pictures[picture_idx.value()]->switch_zoom();
}

void GraphChoicePanel::reset_zoom()
{
	if (!picture_idx.has_value())
		return;

	pictures[picture_idx.value()]->chart()->zoomReset();
}

void GraphChoicePanel::change_graph()
{
	if (!picture_idx.has_value())
		return;

	pictures[picture_idx.value()]->graph_dialog();
}
