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
    void paintEvent(QPaintEvent *event) override;
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

    void setUploads(long long  completed, long long total);
    void setDownloads(long long  completed, long long total);

    void initialize();
    void reset();

    void showAnimated();
    void setPaused(bool value);
    void setPauseEnabled(bool value);

    void setPercentUploads(long long completedBytes, long long totalBytes);
    void setPercentDownloads(long long completedBytes, long long totalBytes);

    bool alwaysAnimateOnShow = false;
    bool neverPainted = true;
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

    void resetDownloads();
    void resetUploads();

    void drawEllipse(int x, int y,  int diam, int width, QPainter *painter);

    Ui::TransfersSummaryWidget *ui;
    QElapsedTimer qe;
    QPen pengrey; //outer border
    QPen pentext;
    QBrush brushspeedUp;
    QBrush brushspeedDown;
    QBrush brushwhitebackground;

    int lastwidth = 0;
    int lastheigth = 0;

    int wpen = 0;
    int diamoutside = 0;
    int diaminside = 0;
    int spacing = 0;
    int marginoutside = 0;
    int margininside = 0;
    int residualin = 0; //related to the width of the pen (0 for FlatCap)

    int firstellipseX = 0;
    int ellipsesMargin = 0;
    int afterEllipsesMargin = 0;

    int fontMarginXLeft = 0;
    int fontMarginXRight = 0;
    int fontY = 0;
    int fontHeight = 0;
    int pixmapArrowMarginX = 0;
    int pixmapArrowY = 0;
    int pixmapWidth = 0;
    QPixmap upArrowPixmapOrig;
    QPixmap dlArrowPixmapOrig;
    QPixmap upArrowPixmap;
    QPixmap dlArrowPixmap;


    int dlEllipseWidth = 0;
    int dlEllipseWidthMin = 0;
    int dlEllipseWidthMax = 0;
    int upEllipseWidth = 0;
    int upEllipseWidthMin = 0;
    int upEllipseWidthMax = 0;


    bool paused = false;
    Status status;
    int initialwidth = 0;
    int goalwidth = 0;

    int originalwidth = 0;
    int originalheight = 0;
    int minwidth = 28;
    int upMaxWidthText = 0;
    int dlMaxWidthText = 0;
    int maxFontSize = 0;

    const int trailingChars = 1;
    int upPosDotsPartial = 0;
    int upPosDotsTotal = 0;
    int dlPosDotsPartial = 0;
    int dlPosDotsTotal = 0;

    qreal acceleration = 0;
    qreal animationTimeMS = 0;

    qreal speed = 0;

    long long totalUploads;
    long long currentUpload;
    long long totalDownloads;
    long long currentDownload;

    QFont fontUploads;
    QFont fontDownloads;
    QString uploadsText;
    QString downloadsText;
    QString uploadsTextToRender;
    QString downloadsTextToRender;

    void calculateSpeed(int initWidth = -1, int endWidth = -1);

    bool isWithinPseudoEllipse(QPoint pos, int x, int margin, int w, int diam);

    qreal computeAnimationStep() const;
    void updateAnimation(const int previousWidth, const int newWidth, const Status newStatus);

    void showEvent(QShowEvent *event) override;
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};


#endif // TRANSFERSSUMMARYWIDGET_H
