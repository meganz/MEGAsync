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

	explicit ImportListWidgetItem(QString link, int id, QWidget *parent = 0);
	~ImportListWidgetItem();

	void setNode(Node *node);
	void setData(QString fileName, linkstatus status, long long size=0);
	void updateGui();
	bool isSelected();
	QString getLink();

private slots:
	void on_cSelected_stateChanged(int state);

signals:
	void stateChanged(int id, int state);

private:
	Ui::ImportListWidgetItem *ui;
	Node *node;
	QString fileName;
	linkstatus status;
	long long fileSize;
	QString link;
	int id;
};

#endif // IMPORTLISTWIDGETITEM_H
