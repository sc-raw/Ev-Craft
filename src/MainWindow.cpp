#include "MainWindow.h"
#include "TimelineWidget.h"

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QStackedLayout>
#include <QSignalBlocker>
#include <QUrl>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <limits>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("EV — Editor de vídeo"));
    resize(1440, 900);
    setMinimumSize(1000, 650);
    setDockNestingEnabled(true);
    createMenus();
    createToolbars();
    createDocks();
    statusBar()->showMessage(QStringLiteral("Listo · Qt Multimedia"));
}

void MainWindow::createMenus()
{
    QMenu *file = menuBar()->addMenu(QStringLiteral("Archivo"));
    file->addAction(QStringLiteral("Importar medios…"), QKeySequence::Open, this, &MainWindow::importMedia);
    file->addSeparator();
    file->addAction(QStringLiteral("Salir"), QKeySequence::Quit, this, &QWidget::close);
    QMenu *edit = menuBar()->addMenu(QStringLiteral("Editar"));
    edit->addAction(QStringLiteral("Añadir selección a la línea de tiempo"), this, &MainWindow::addSelectedToTimeline);
    menuBar()->addMenu(QStringLiteral("Ver"));
    menuBar()->addMenu(QStringLiteral("Insertar"));
    menuBar()->addMenu(QStringLiteral("Herramientas"));
    QMenu *help = menuBar()->addMenu(QStringLiteral("Ayuda"));
    help->addAction(QStringLiteral("Acerca de EV"), this, [this] {
        QMessageBox::about(this, QStringLiteral("Acerca de EV"),
                           QStringLiteral("EV 0.1.1\nReproducción multimedia básica con Qt Multimedia."));
    });
}

void MainWindow::createToolbars()
{
    QToolBar *tools = addToolBar(QStringLiteral("Herramientas"));
    tools->setMovable(false);
    QAction *selectAction = tools->addAction(QStringLiteral("Seleccionar"));
    selectAction->setCheckable(true);
    selectAction->setChecked(true);
    selectAction->setToolTip(QStringLiteral("Seleccionar clips en la línea de tiempo"));
    QAction *cutAction = tools->addAction(QStringLiteral("Cortar"));
    cutAction->setEnabled(false);
    cutAction->setToolTip(QStringLiteral("Edición de corte aún no implementada"));
    QAction *splitAction = tools->addAction(QStringLiteral("Dividir"));
    splitAction->setEnabled(false);
    splitAction->setToolTip(QStringLiteral("División de clips aún no implementada"));
    tools->addSeparator();
    tools->addWidget(new QLabel(QStringLiteral(" Zoom línea de tiempo: "), this));
    m_zoom = new QSpinBox(this);
    m_zoom->setRange(50, 200);
    m_zoom->setValue(100);
    m_zoom->setSuffix(QStringLiteral("%"));
    tools->addWidget(m_zoom);
}

void MainWindow::createDocks()
{
    auto *browser = new QDockWidget(QStringLiteral("Explorador de medios"), this);
    browser->setObjectName(QStringLiteral("MediaBrowserDock"));
    browser->setMinimumWidth(220);
    auto *browserWidget = new QWidget(browser);
    auto *browserLayout = new QVBoxLayout(browserWidget);
    auto *import = new QPushButton(QStringLiteral("Importar archivos…"), browserWidget);
    m_mediaList = new QListWidget(browserWidget);
    m_mediaList->setSelectionMode(QAbstractItemView::SingleSelection);
    auto *add = new QPushButton(QStringLiteral("Añadir selección a pista"), browserWidget);
    browserLayout->addWidget(import);
    browserLayout->addWidget(m_mediaList, 1);
    browserLayout->addWidget(add);
    browser->setWidget(browserWidget);
    addDockWidget(Qt::LeftDockWidgetArea, browser);
    connect(import, &QPushButton::clicked, this, &MainWindow::importMedia);
    connect(add, &QPushButton::clicked, this, &MainWindow::addSelectedToTimeline);
    connect(m_mediaList, &QListWidget::currentRowChanged, this, &MainWindow::updateMediaProperties);

    auto *properties = new QDockWidget(QStringLiteral("Propiedades"), this);
    properties->setObjectName(QStringLiteral("PropertiesDock"));
    properties->setMinimumWidth(220);
    auto *props = new QWidget(properties);
    auto *propsLayout = new QVBoxLayout(props);
    m_propertyName = new QLabel(QStringLiteral("Sin selección"), props);
    m_propertyName->setWordWrap(true);
    m_propertyType = new QLabel(QStringLiteral("Selecciona un medio o clip"), props);
    propsLayout->addWidget(new QLabel(QStringLiteral("Elemento"), props));
    propsLayout->addWidget(m_propertyName);
    propsLayout->addSpacing(8);
    propsLayout->addWidget(new QLabel(QStringLiteral("Tipo"), props));
    propsLayout->addWidget(m_propertyType);
    propsLayout->addStretch();
    props->setLayout(propsLayout);
    properties->setWidget(props);
    addDockWidget(Qt::RightDockWidgetArea, properties);

    auto *preview = new QWidget(this);
    preview->setObjectName(QStringLiteral("PreviewPanel"));
    auto *previewLayout = new QVBoxLayout(preview);
    previewLayout->setContentsMargins(8, 8, 8, 8);
    auto *canvas = new QWidget(preview);
    canvas->setMinimumSize(480, 270);
    auto *canvasLayout = new QStackedLayout(canvas);
    canvasLayout->setStackingMode(QStackedLayout::StackAll);
    m_videoWidget = new QVideoWidget(canvas);
    m_videoWidget->setStyleSheet(QStringLiteral("background:#08090b; border:1px solid #343840;"));
    canvasLayout->addWidget(m_videoWidget);
    m_previewStatus = new QLabel(QStringLiteral("Selecciona un archivo multimedia para reproducirlo."), canvas);
    m_previewStatus->setAlignment(Qt::AlignCenter);
    m_previewStatus->setWordWrap(true);
    m_previewStatus->setStyleSheet(QStringLiteral("color:#aeb4bf; background:transparent; border:0; padding:20px;"));
    canvasLayout->addWidget(m_previewStatus);
    previewLayout->addWidget(canvas, 1);

    m_player = new QMediaPlayer(this);
    auto *audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(0.8);
    m_player->setAudioOutput(audioOutput);
    m_player->setVideoOutput(m_videoWidget);

    auto *transport = new QHBoxLayout;
    auto *begin = new QPushButton(QStringLiteral("⏮"), preview);
    begin->setToolTip(QStringLiteral("Volver al inicio del archivo"));
    m_playPauseButton = new QPushButton(QStringLiteral("▶ / ⏸"), preview);
    m_playPauseButton->setToolTip(QStringLiteral("Reproducir o pausar"));
    auto *stop = new QPushButton(QStringLiteral("⏹"), preview);
    stop->setToolTip(QStringLiteral("Detener reproducción"));
    begin->setEnabled(false);
    m_playPauseButton->setEnabled(false);
    stop->setEnabled(false);
    transport->addStretch();
    transport->addWidget(begin);
    transport->addWidget(m_playPauseButton);
    transport->addWidget(stop);
    m_positionLabel = new QLabel(QStringLiteral("00:00 / 00:00"), preview);
    transport->addWidget(m_positionLabel);
    transport->addStretch();
    m_positionSlider = new QSlider(Qt::Horizontal, preview);
    m_positionSlider->setRange(0, 0);
    m_positionSlider->setEnabled(false);
    previewLayout->addLayout(transport);
    previewLayout->addWidget(m_positionSlider);
    setCentralWidget(preview);

    connect(begin, &QPushButton::clicked, m_player, [this] { m_player->setPosition(0); });
    connect(m_playPauseButton, &QPushButton::clicked, m_player, [this] {
        if (m_player->playbackState() == QMediaPlayer::PlayingState)
            m_player->pause();
        else
            m_player->play();
    });
    connect(stop, &QPushButton::clicked, m_player, &QMediaPlayer::stop);
    connect(m_positionSlider, &QSlider::sliderMoved, m_player, [this](int value) { m_player->setPosition(value); });
    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::updatePlaybackPosition);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::updateMediaDuration);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::updatePlaybackState);
    connect(m_player, &QMediaPlayer::hasVideoChanged, this, [this](bool hasVideo) {
        if (hasVideo)
            m_previewStatus->hide();
        else if (m_player->source().isValid()) {
            m_previewStatus->setText(QStringLiteral("Reproducción de audio · sin vídeo"));
            m_previewStatus->show();
        }
    });
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if ((status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia)
            && !m_player->hasVideo()) {
            m_previewStatus->setText(QStringLiteral("Archivo de audio cargado · sin vídeo"));
            m_previewStatus->show();
        }
    });
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &) {
        showPlaybackError();
    });

    auto *timelineDock = new QDockWidget(QStringLiteral("Línea de tiempo"), this);
    timelineDock->setObjectName(QStringLiteral("TimelineDock"));
    timelineDock->setMinimumHeight(250);
    m_timeline = new TimelineWidget(timelineDock);
    timelineDock->setWidget(m_timeline);
    addDockWidget(Qt::BottomDockWidgetArea, timelineDock);
    connect(m_zoom, qOverload<int>(&QSpinBox::valueChanged), m_timeline, &TimelineWidget::setZoom);
    connect(m_timeline, &TimelineWidget::selectionChanged, this, &MainWindow::setClipProperties);
}

void MainWindow::importMedia()
{
    const QStringList files = QFileDialog::getOpenFileNames(this, QStringLiteral("Importar medios"), QString(),
        QStringLiteral("Medios compatibles (*.*)"));
    for (const QString &path : files) {
        const QFileInfo info(path);
        auto *item = new QListWidgetItem(info.fileName(), m_mediaList);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
    }
    if (!files.isEmpty())
        statusBar()->showMessage(QStringLiteral("%1 archivo(s) añadido(s) al explorador").arg(files.size()), 4000);
}

void MainWindow::addSelectedToTimeline()
{
    const QListWidgetItem *item = m_mediaList->currentItem();
    if (!item) {
        statusBar()->showMessage(QStringLiteral("Selecciona primero un archivo del explorador"), 3500);
        return;
    }
    const QString path = item->data(Qt::UserRole).toString();
    const QString suffix = QFileInfo(path).suffix().toLower();
    const QStringList audioExtensions{QStringLiteral("wav"), QStringLiteral("mp3"), QStringLiteral("aac"),
                                      QStringLiteral("flac"), QStringLiteral("ogg"), QStringLiteral("m4a")};
    const bool audio = audioExtensions.contains(suffix);
    m_timeline->addClip(item->text(), path, audio);
    statusBar()->showMessage(QStringLiteral("Clip visual añadido a la pista %1").arg(audio ? QStringLiteral("de audio") : QStringLiteral("de vídeo")), 3500);
}

void MainWindow::updateMediaProperties()
{
    const QListWidgetItem *item = m_mediaList->currentItem();
    if (!item) {
        m_propertyName->setText(QStringLiteral("Sin selección"));
        m_propertyType->setText(QStringLiteral("Selecciona un medio o clip"));
        return;
    }
    const QString path = item->data(Qt::UserRole).toString();
    m_propertyName->setText(item->text());
    m_propertyType->setText(QStringLiteral("Archivo · %1").arg(QFileInfo(path).suffix().toUpper()));
    loadMedia(path, item->text());
}

void MainWindow::setClipProperties(const QString &name, const QString &sourcePath, const QString &trackType)
{
    m_propertyName->setText(name);
    m_propertyType->setText(QStringLiteral("Clip de %1 · duración pendiente de análisis").arg(trackType));
    loadMedia(sourcePath, name);
}

void MainWindow::loadMedia(const QString &sourcePath, const QString &displayName)
{
    m_currentMediaName = displayName;
    m_previewStatus->setText(QStringLiteral("Cargando: %1").arg(displayName));
    m_previewStatus->show();
    m_positionSlider->setEnabled(false);
    m_positionSlider->setRange(0, 0);
    m_positionLabel->setText(QStringLiteral("00:00 / 00:00"));
    m_player->setSource(QUrl::fromLocalFile(sourcePath));
    m_playPauseButton->setEnabled(true);
    statusBar()->showMessage(QStringLiteral("Cargando %1…").arg(displayName));
}

void MainWindow::updatePlaybackPosition(qint64 position)
{
    if (!m_positionSlider->isSliderDown()) {
        const QSignalBlocker blocker(m_positionSlider);
        m_positionSlider->setValue(static_cast<int>(qMin(position, qint64(m_positionSlider->maximum()))));
    }
    const qint64 duration = m_player->duration();
    const auto formatTime = [](qint64 milliseconds) {
        const qint64 seconds = milliseconds / 1000;
        return seconds >= 3600
            ? QStringLiteral("%1:%2:%3").arg(seconds / 3600, 2, 10, QLatin1Char('0')).arg((seconds / 60) % 60, 2, 10, QLatin1Char('0')).arg(seconds % 60, 2, 10, QLatin1Char('0'))
            : QStringLiteral("%1:%2").arg(seconds / 60, 2, 10, QLatin1Char('0')).arg(seconds % 60, 2, 10, QLatin1Char('0'));
    };
    m_positionLabel->setText(QStringLiteral("%1 / %2").arg(formatTime(position), formatTime(duration)));
}

void MainWindow::updateMediaDuration(qint64 duration)
{
    m_positionSlider->setRange(0, static_cast<int>(qMin(duration, qint64(std::numeric_limits<int>::max()))));
    m_positionSlider->setEnabled(duration > 0);
    updatePlaybackPosition(m_player->position());
}

void MainWindow::updatePlaybackState()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
        statusBar()->showMessage(QStringLiteral("Reproduciendo: %1").arg(m_currentMediaName));
    else if (m_player->playbackState() == QMediaPlayer::PausedState)
        statusBar()->showMessage(QStringLiteral("En pausa: %1").arg(m_currentMediaName));
    else if (!m_currentMediaName.isEmpty())
        statusBar()->showMessage(QStringLiteral("Detenido: %1").arg(m_currentMediaName));
}

void MainWindow::showPlaybackError()
{
    const QString reason = m_player->errorString().isEmpty()
        ? QStringLiteral("Formato, códec o acceso al archivo no compatible.")
        : m_player->errorString();
    const QString message = QStringLiteral("No se pudo reproducir %1.\n%2").arg(m_currentMediaName, reason);
    m_previewStatus->setText(message);
    m_previewStatus->show();
    statusBar()->showMessage(message, 10000);
}
