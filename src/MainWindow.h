#pragma once

#include <QMainWindow>

class QListWidget;
class QLabel;
class QSlider;
class QSpinBox;
class QMediaPlayer;
class QPushButton;
class QSlider;
class QVideoWidget;
class TimelineWidget;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void importMedia();
    void addSelectedToTimeline();
    void updateMediaProperties();
    void setClipProperties(const QString &name, const QString &sourcePath, const QString &trackType);
    void loadMedia(const QString &sourcePath, const QString &displayName);
    void updatePlaybackPosition(qint64 position);
    void updateMediaDuration(qint64 duration);
    void updatePlaybackState();
    void showPlaybackError();

private:
    void createMenus();
    void createToolbars();
    void createDocks();

    QListWidget *m_mediaList = nullptr;
    QLabel *m_propertyName = nullptr;
    QLabel *m_propertyType = nullptr;
    QLabel *m_previewStatus = nullptr;
    TimelineWidget *m_timeline = nullptr;
    QSpinBox *m_zoom = nullptr;
    QMediaPlayer *m_player = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QPushButton *m_playPauseButton = nullptr;
    QSlider *m_positionSlider = nullptr;
    QLabel *m_positionLabel = nullptr;
    QString m_currentMediaName;
};
