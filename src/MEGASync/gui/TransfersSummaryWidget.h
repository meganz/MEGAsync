#ifndef TRANSFERSSUMMARYWIDGET_H
#define TRANSFERSSUMMARYWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPen>
#include <QElapsedTimer>

namespace Ui {
class TransfersSummaryWidget;
}

class TransfersSummaryWidget : public QWidget
{
    Q_OBJECT

public:

    enum class Status { EXPANDING, EXPANDED, SHRINKING, SHRUNK };


    explicit TransfersSummaryWidget(QWidget *parent = 0);
    ~TransfersSummaryWidget();

    void updateSizes();
    void paintEvent(QPaintEvent *event);
    int getDisplacement() const;
    void setDisplacement(int value);

    qreal getAcceleration() const;

    /**
     * @brief Sets the acceleration of the animation. 1 = no acceleration, < 1 it goes faster in the end, > 1 accelerates
     *
     * @param value - Recommended values are between 0.2 and 10. Default: 0.35
     */
    void setAcceleration(const qreal &value);

    qreal getAnimationTimeMS() const;

    /**
     * @brief Set the time that the animation with take. Default: 800 ms
     * @param value
     */
    void setAnimationTimeMS(const qreal &value);

    void adjustFontSizeToText(QFont *font, int maxWidth, QString uploadText, int fontsize = 12.0);

    void setTotalUploads(int value);
    void setCompletedDownloads(int value);
    void setTotalDownloads(int value);
    void setCompletedUploads(int value);

    void initialize();

    void showAnimated();

private slots:
    void resizeAnimation();


public slots:
    void expand(bool noAnimate = false);
    void shrink(bool noAnimate = false);

private:

    void updateUploadsText(bool force = false);
    void updateDownloadsText(bool force = false);
    void updateUploads();
    void updateDownloads();

    void setPercentInnerCircle(const qreal &value);
    void setPercentOuterCircle(const qreal &value);

    Ui::TransfersSummaryWidget *ui;
    QElapsedTimer qe;
    QPen pengrey; //outer border
    QPen pentext;
    QBrush brushgreyspeeds;
    QBrush brushwhitebackground;

    int lastwidth;
    int lastheigth;

    int wpen;
    int diamoutside;
    int diaminside;
    int spacing;
    int marginoutside;
    int margininside;
    int residualin; //related to the width of the pen (0 for FlatCap)
    int displacement;

    Status status;

    int originalwidth;
    int originalheight;
    int minwidth;
    int maxWidthText;
    int maxFontSize;

    qreal acceleration;
    qreal animationTimeMS;

    qreal speed;

    int totalUploads;
    int completedUploads;
    int totalDownloads;
    int completedDownloads;

    QFont fontUploads;
    QFont fontDownloads;
    QString uploadsText;
    QString downloadsText;

    void calculateSpeed();
};


#endif // TRANSFERSSUMMARYWIDGET_H
