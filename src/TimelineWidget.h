#pragma once

#include <QWidget>

class QPaintEvent;
class QMouseEvent;

class TimelineWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget *parent = nullptr);
    void addClip(const QString &name, bool audio);
    void setZoom(int percent);

signals:
    void selectionChanged(const QString &name, const QString &trackType);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    struct Clip {
        QString name;
        int track = 0;
        int start = 0;
        int width = 180;
    };

    QList<Clip> m_clips;
    int m_selected = -1;
    int m_zoom = 100;
};
