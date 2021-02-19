#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QApplication>
#include "sdk/HCNetSDK.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VideoWidget; }
QT_END_NAMESPACE

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
    bool openVideo();
    void closeVideo();

private slots:
    void on_btn_close_clicked();

private:
    Ui::VideoWidget *ui;

    LONG lUserID;
    LONG lRealPlayHandle;
    NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
    //const String url = "rtsp://admin:Hik12345@192.168.137.100:554/mpeg4/ch1/main/av_stream";
private:
    virtual bool eventFilter(QObject *, QEvent *) override;
};

#endif // VIDEOWIDGET_H
