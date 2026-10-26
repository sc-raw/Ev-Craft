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
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("EV — Editor de vídeo"));
    resize(1440, 900);
    setMinimumSize(1000, 650);
    setDockNestingEnabled(true);
    createMenus();
    createToolbars();
    createDocks();
    statusBar()->showMessage(QStringLiteral("Listo · motor multimedia pendiente de integración"));
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
                           QStringLiteral("EV 0.1.0\nInterfaz inicial. Reproducción y procesamiento multimedia aún no implementados."));
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
    canvas->setStyleSheet(QStringLiteral("background:#08090b; border:1px solid #343840;"));
    auto *canvasLayout = new QVBoxLayout(canvas);
    m_previewStatus = new QLabel(QStringLiteral("Vista previa\n\nEl motor de reproducción de vídeo se incorporará en una etapa posterior."), canvas);
    m_previewStatus->setAlignment(Qt::AlignCenter);
    m_previewStatus->setStyleSheet(QStringLiteral("color:#aeb4bf; border:0;"));
    canvasLayout->addWidget(m_previewStatus);
    previewLayout->addWidget(canvas, 1);
    auto *transport = new QHBoxLayout;
    auto *begin = new QPushButton(QStringLiteral("|◀"), preview);
    auto *play = new QPushButton(QStringLiteral("▶"), preview);
    auto *end = new QPushButton(QStringLiteral("▶|"), preview);
    for (QPushButton *button : {begin, play, end}) {
        button->setEnabled(false);
        button->setToolTip(QStringLiteral("Disponible cuando se integre el motor multimedia"));
    }
    transport->addStretch();
    transport->addWidget(begin);
    transport->addWidget(play);
    transport->addWidget(end);
    transport->addStretch();
    auto *position = new QSlider(Qt::Horizontal, preview);
    position->setRange(0, 0);
    position->setEnabled(false);
    previewLayout->addLayout(transport);
    previewLayout->addWidget(position);
    setCentralWidget(preview);

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
    m_timeline->addClip(item->text(), audio);
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
}

void MainWindow::setClipProperties(const QString &name, const QString &trackType)
{
    m_propertyName->setText(name);
    m_propertyType->setText(QStringLiteral("Clip de %1 · duración pendiente de análisis").arg(trackType));
}
