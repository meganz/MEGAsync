#ifndef IMPORTLISTWIDGETITEM_H
#define IMPORTLISTWIDGETITEM_H

#include <QListWidgetItem>
#include "sdk/megaapi.h"

namespace Ui {
class ImportListWidgetItem;
}

class ImportListWidgetItem : public QWidget
{
	Q_OBJECT

public:
	enum linkstatus {LOADING, CORRECT, WARNING, FAILED};

	explicit ImportListWidgetItem(QString link, QWidget *parent = 0);
	~ImportListWidgetItem();

	void setNode(Node *node);
	void setData(QString fileName, linkstatus status);
	void updateGui();
	bool isSelected();
	QString getLink();

private:
	Ui::ImportListWidgetItem *ui;
	Node *node;
	QString fileName;
	linkstatus status;
	QString link;
};

#endif // IMPORTLISTWIDGETITEM_H
