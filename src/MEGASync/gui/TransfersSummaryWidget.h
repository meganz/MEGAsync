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

    enum class Status { EXPANDING, EXPANDED, SHRINKING, SHRUNK, RESIZING, RESIZED };

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
    int adjustSizeToText(QFont *font, int maxWidth, int minWidth, int margins, long long partial, long long total, int &posDotsPartial, int &posDotsTotal, QString &text, int fontsize);


    void setTotalUploads(long long  value);
    void setCompletedDownloads(long long  value);
    void setTotalDownloads(long long  value);
    void setCompletedUploads(long long  value);

    void initialize();

    void showAnimated();
    void setPaused(bool value);

signals:
    void pauseResumeClicked();
    void generalAreaClicked();
    void dlAreaClicked();
    void upAreaClicked();

    void pauseResumeHovered(QMouseEvent *event);
    void generalAreaHovered(QMouseEvent *event);
    void dlAreaHovered(QMouseEvent *event);
    void upAreaHovered(QMouseEvent *event);

private slots:
    void resizeAnimation();

public slots:
    void expand(bool noAnimate = false);
    void shrink(bool noAnimate = false);
    void doResize(int futureWidth, bool noAnimate = false);

private:

    void updateUploadsText(bool force = false);
    void updateDownloadsText(bool force = false);
    void updateUploads();
    void updateDownloads();

    void setPercentInnerCircle(const qreal &value);
    void setPercentOuterCircle(const qreal &value);

    void drawEllipse(int x, int y,  int diam, int width, QPainter *painter);

    Ui::TransfersSummaryWidget *ui;
    QElapsedTimer qe;
    QPen pengrey; //outer border
    QPen pentext;
    QBrush brushspeedUp;
    QBrush brushspeedDown;
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

    int firstellipseX;
    int ellipsesMargin;
    int afterEllipsesMargin;

    int fontMarginXLeft;
    int fontMarginXRight;
    int fontY;
    int fontHeight;
    int pixmapArrowMarginX;
    int pixmapArrowY;
    int pixmapWidth;
    QPixmap upArrowPixmapOrig;
    QPixmap dlArrowPixmapOrig;
    QPixmap upArrowPixmap;
    QPixmap dlArrowPixmap;


    int dlEllipseWidth;
    int dlEllipseWidthMin;
    int dlEllipseWidthMax;
    int upEllipseWidth;
    int upEllipseWidthMin;
    int upEllipseWidthMax;


    bool paused = false;
    Status status;
    int initialwidth;
    int goalwidth;

    int originalwidth;
    int originalheight;
    int minwidth;
    int upMaxWidthText;
    int dlMaxWidthText;
    int maxFontSize;

    const int trailingChars = 1;
    int upPosDotsPartial;
    int upPosDotsTotal;
    int dlPosDotsPartial;
    int dlPosDotsTotal;

    qreal acceleration;
    qreal animationTimeMS;

    qreal speed;

    long long totalUploads;
    long long completedUploads;
    long long totalDownloads;
    long long completedDownloads;

    QFont fontUploads;
    QFont fontDownloads;
    QString uploadsText;
    QString downloadsText;
    QString uploadsTextToRender;
    QString downloadsTextToRender;

    void calculateSpeed(int initWidth = -1, int endWidth = -1);

    bool isWithinPseudoEllipse(QPoint pos, int x, int margin, int w, int diam);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
};


#endif // TRANSFERSSUMMARYWIDGET_H
