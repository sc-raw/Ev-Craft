#pragma once

#include <QMainWindow>

class QListWidget;
class QLabel;
class QSlider;
class QSpinBox;
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
    void setClipProperties(const QString &name, const QString &trackType);

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
};
