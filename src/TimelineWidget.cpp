#include "TimelineWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QList>
#include <algorithm>

namespace {
constexpr int headerWidth = 128;
constexpr int rulerHeight = 30;
constexpr int trackHeight = 66;
}

TimelineWidget::TimelineWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(520, rulerHeight + trackHeight * 4);
    setMouseTracking(true);
}

void TimelineWidget::addClip(const QString &name, bool audio)
{
    const int track = audio ? 2 : 0;
    int start = 12;
    for (const Clip &clip : m_clips) {
        if (clip.track == track)
            start = std::max(start, clip.start + clip.width + 12);
    }
    m_clips.append({name, track, start, 190});
    update();
}

void TimelineWidget::setZoom(int percent)
{
    m_zoom = std::clamp(percent, 50, 200);
    update();
}

void TimelineWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.fillRect(rect(), QColor("#17191d"));
    p.fillRect(QRect(0, 0, width(), rulerHeight), QColor("#202329"));
    p.setPen(QColor("#3a3e47"));
    p.drawLine(headerWidth, 0, headerWidth, height());

    const int spacing = 80 * m_zoom / 100;
    for (int x = headerWidth, n = 0; x < width(); x += spacing, ++n) {
        p.setPen(QColor("#454a54"));
        p.drawLine(x, rulerHeight - 7, x, height());
        p.setPen(QColor("#aeb4bf"));
        p.drawText(x + 5, 19, QStringLiteral("%1:%2").arg(n / 5).arg((n % 5) * 12, 2, 10, QLatin1Char('0')));
    }

    const QStringList trackNames{QStringLiteral("VIDEO 1"), QStringLiteral("VIDEO 2"),
                                 QStringLiteral("AUDIO 1"), QStringLiteral("AUDIO 2")};
    for (int row = 0; row < trackNames.size(); ++row) {
        const int y = rulerHeight + row * trackHeight;
        p.fillRect(QRect(0, y, headerWidth - 1, trackHeight - 1), QColor(row < 2 ? "#242830" : "#22272a"));
        p.setPen(QColor("#e2e5eb"));
        p.drawText(QRect(12, y, headerWidth - 20, trackHeight), Qt::AlignVCenter | Qt::AlignLeft, trackNames.at(row));
        p.setPen(QColor("#30343c"));
        p.drawLine(0, y + trackHeight - 1, width(), y + trackHeight - 1);
    }

    for (int i = 0; i < m_clips.size(); ++i) {
        const Clip &clip = m_clips.at(i);
        const int x = headerWidth + clip.start * m_zoom / 100;
        const int y = rulerHeight + clip.track * trackHeight + 9;
        const QRect bounds(x, y, clip.width * m_zoom / 100, trackHeight - 18);
        const QColor color = clip.track < 2 ? QColor("#315f83") : QColor("#39705b");
        p.setPen(QPen(i == m_selected ? QColor("#d9edf9") : color.lighter(135), i == m_selected ? 2 : 1));
        p.setBrush(color);
        p.drawRoundedRect(bounds, 4, 4);
        p.setPen(Qt::white);
        p.drawText(bounds.adjusted(8, 4, -6, -4), Qt::AlignLeft | Qt::AlignTop,
                   QFontMetrics(font()).elidedText(clip.name, Qt::ElideRight, bounds.width() - 14));
        p.setPen(QColor("#a9c2d4"));
        p.drawText(bounds.adjusted(8, 22, -6, -3), Qt::AlignLeft | Qt::AlignTop,
                   clip.track < 2 ? QStringLiteral("Vídeo · duración pendiente") : QStringLiteral("Audio · duración pendiente"));
    }
}

void TimelineWidget::mousePressEvent(QMouseEvent *event)
{
    m_selected = -1;
    for (int i = 0; i < m_clips.size(); ++i) {
        const Clip &clip = m_clips.at(i);
        const QRect bounds(headerWidth + clip.start * m_zoom / 100,
                           rulerHeight + clip.track * trackHeight + 9,
                           clip.width * m_zoom / 100, trackHeight - 18);
        if (bounds.contains(event->pos())) {
            m_selected = i;
            emit selectionChanged(clip.name, clip.track < 2 ? QStringLiteral("Vídeo") : QStringLiteral("Audio"));
            break;
        }
    }
    update();
    QWidget::mousePressEvent(event);
}
