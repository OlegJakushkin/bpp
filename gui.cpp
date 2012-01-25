#include "gui.h"

#define APP_VERSION QString("v1.0.1")
#define APP_NAME QString("physics")
#define APP_NAME_FULL tr("Bullet Physics Playground")

Gui::Gui(bool savePNG, bool savePOV, QWidget *parent) : QMainWindow(parent) {
  ui.setupUi(this);

  // setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("%1 %2").arg(APP_NAME_FULL).arg(APP_VERSION));
  //  setWindowIcon(QIcon(":images/icon.png"));

  setAcceptDrops(true);

  settings = new QSettings();

  createDock();
  createActions();
  createMenus();
  setStatusBar( new QStatusBar(this) );

  createToolBar();

  loadLastFile();

  connect(editor, SIGNAL(textChanged()), this, SLOT(parseEditor()));

  //  ui.cmdline->setFocus();

  //  connect(ui.cmdline, SIGNAL(execute(QString)), this, SLOT(command(QString)));

  loadSettings();
}

void Gui::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void Gui::dropEvent(QDropEvent *event) {
  QList<QUrl> urls = event->mimeData()->urls();

  if (urls.isEmpty()) return;

  QString filePath = urls.first().toLocalFile();

  if (filePath.isEmpty()) return;

  loadFile(filePath);

  event->acceptProposedAction();
}

void Gui::loadLastFile() {
  QString lastFile;

  settings->beginGroup( "mainwindow" );
  lastFile = settings->value( "lastFile", "").toString();
  settings->endGroup();

  if (lastFile != "") {
    loadFile(lastFile);
  } else {
    loadFile(":demo/00-simple.lua");
  }
}

void Gui::loadFile(const QString &path) {
  QFile file(path);

  settings->beginGroup( "mainwindow" );
  settings->setValue( "lastFile", path);
  settings->endGroup();

  if (file.open(QFile::ReadOnly | QFile::Text)) {
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	//    drillFile->setFileName(file.fileName());

    editor->replaceText(file.readAll());
    setCurrentFile(path);
    setWindowTitle(tr("%1 - %2").arg(APP_NAME_FULL).arg(file.fileName()));

	//    parseEditor();

#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
	statusBar()->showMessage(tr("File loaded"), 2000);
  } else {
    setWindowTitle(tr("%1 %2").arg(APP_NAME_FULL).arg(APP_VERSION));
	//    drillView->zoomFit();
    statusBar()->showMessage(tr("Error loading File %1").arg(path), 5000);
  }
}

void Gui::createToolBar() {
  myToolBar = new QToolBar(this);
  myToolBar->setWindowTitle("Toolbar Main");
  myToolBar->setObjectName("ToolBarMain");

  myToolBar->addAction( newAction );
  myToolBar->addAction( openAction );
  myToolBar->addAction( saveAction );
  myToolBar->addAction( saveAsAction );
  myToolBar->addSeparator();
  myToolBar->addAction( playAction );
  myToolBar->addAction( stopAction );

  addToolBar( myToolBar );
}

void Gui::createActions() {
  QIcon newIcon = QIcon::fromTheme("document-new");
  newAction = new QAction(newIcon,tr("&New"), this);
  newAction->setShortcut(tr("Ctrl+N"));
  newAction->setStatusTip(tr("Create a new file"));
  connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

  QIcon openIcon = QIcon::fromTheme("document-open" );
  openAction = new QAction(openIcon, tr("&Open..."), this);
  openAction->setShortcut(tr("Ctrl+O"));
  openAction->setStatusTip(tr("Open an existing file"));
  connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

  QIcon saveIcon = QIcon::fromTheme("document-save" );
  saveAction = new QAction(saveIcon, tr("&Save.."), this);
  saveAction->setShortcut(tr("Ctrl+S"));
  saveAction->setStatusTip(tr("Save file"));
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  QIcon saveAsIcon = QIcon::fromTheme("document-save-as" );
  saveAsAction = new QAction(saveAsIcon, tr("&Save As.."), this);
  saveAsAction->setShortcut(tr("Ctrl+A"));
  saveAsAction->setStatusTip(tr("Save file as.."));
  connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

  cutAction = new QAction(QIcon::fromTheme("document-cut"), tr("Cu&t"), this);
  cutAction->setShortcuts(QKeySequence::Cut);
  cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
  connect(cutAction, SIGNAL(triggered()), editor, SLOT(cut()));

  copyAction = new QAction(QIcon::fromTheme(":document-copy"), tr("&Copy"), this);
  copyAction->setShortcuts(QKeySequence::Copy);
  copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
  connect(copyAction, SIGNAL(triggered()), editor, SLOT(copy()));

  pasteAction = new QAction(QIcon::fromTheme("document-paste"), tr("&Paste"), this);
  pasteAction->setShortcuts(QKeySequence::Paste);
  pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
  connect(pasteAction, SIGNAL(triggered()), editor, SLOT(paste()));
  cutAction->setEnabled(false);
  copyAction->setEnabled(false);
  connect(editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)));
  connect(editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)));

  prefsAction = new QAction(tr("&Preferences.."), this);
  prefsAction->setShortcut(tr("Ctrl+P"));
  prefsAction->setStatusTip(tr("Edit application preferences"));
  connect(prefsAction, SIGNAL(triggered()), this, SLOT(editPrefs()));

  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+X"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

  aboutAction = new QAction(tr("&About"), this);
  aboutAction->setStatusTip(tr("Show the application's About box"));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  QIcon playIcon = QIcon::fromTheme("media-playback-start");
  playAction = new QAction(playIcon,tr("&Run simulation.."), this);
  playAction->setShortcut(tr("Ctrl+P"));
  playAction->setStatusTip(tr("Run Simulation"));
  connect(playAction, SIGNAL(triggered()), this, SLOT( runProgram() ));

  QIcon stopIcon = QIcon::fromTheme("media-playback-stop");
  stopAction = new QAction(stopIcon,tr("&Stop Simulation"), this);
  stopAction->setShortcut(tr("Ctrl+C"));
  stopAction->setStatusTip(tr("Stop Simulation"));
  connect(stopAction, SIGNAL(triggered()), this, SLOT( stopProgram() ));
  for (int i = 0; i < MAX_RECENT_FILES; ++i) {
    recentFileActions[i] = new QAction(this);
    recentFileActions[i]->setVisible(false);
    connect(recentFileActions[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
  }
}

void Gui::createMenus() {
  fileMenu = menuBar()->addMenu( tr("&File") );
  fileMenu->addAction( newAction );
  fileMenu->addAction( openAction );
  fileMenu->addSeparator();
  fileMenu->addAction( saveAction );
  fileMenu->addAction( saveAsAction );
  separatorAction = fileMenu->addSeparator();
  for (int i = 0; i < MAX_RECENT_FILES; ++i)
    fileMenu->addAction(recentFileActions[i]);
  fileMenu->addSeparator();
  fileMenu->addAction( exitAction );

  updateRecentFileActions();

  editMenu = new QMenu(tr("E&dit"));
  editAction = menuBar()->addMenu(editMenu);
  editMenu->addAction(cutAction);
  editMenu->addAction(copyAction);
  editMenu->addAction(pasteAction);
  editMenu->addSeparator();
  editMenu->addAction(prefsAction);

  helpMenu = new QMenu(tr("&Help"));
  helpAction = menuBar()->addMenu(helpMenu);
  helpMenu->addAction(aboutAction);
}

void Gui::updateRecentFileActions() {
  QStringList files = settings->value("recentFileList").toStringList();

  int numRecentFiles = qMin(files.size(), (int)MAX_RECENT_FILES);

  for (int i = 0; i < numRecentFiles; ++i) {
    QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
    recentFileActions[i]->setText(text);
    recentFileActions[i]->setData(files[i]);
    recentFileActions[i]->setVisible(true);
  }
  for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j)
    recentFileActions[j]->setVisible(false);

  separatorAction->setVisible(numRecentFiles > 0);
}

void Gui::setCurrentFile(const QString &fileName) {

  /*
  if (drillFile->getFileName().isEmpty())
    setWindowTitle(tr("Recent Files"));
  else
    setWindowTitle(tr("%1 %2 - %3").arg(APP_NAME_FULL).arg(APP_VERSION).arg(strippedName(drillFile->getFileName())));
  */

  QStringList files = settings->value("recentFileList").toStringList();
  files.removeAll(fileName);
  files.prepend(fileName);
  while (files.size() > MAX_RECENT_FILES)
    files.removeLast();

  settings->setValue("recentFileList", files);

  foreach (QWidget *widget, QApplication::topLevelWidgets()) {
    Gui *mainWin = qobject_cast<Gui *>(widget);
    if (mainWin)
      mainWin->updateRecentFileActions();
  }
}

void Gui::createDock() {
  QDockWidget* dw1 = new QDockWidget(this);
  dw1->setObjectName("DockDebug");
  dw1->setWindowTitle("Debug");
  dw1->setTitleBarWidget(new QWidget(this));

  debugText = new CodeEditor(this);
  dw1->setWidget(debugText);
  debugText->setReadOnly(true);

  addDockWidget(Qt::BottomDockWidgetArea, dw1);

  QDockWidget *dw2 = new QDockWidget(this);
  dw2->setObjectName("DockLUAScript");
  dw2->setWindowTitle("LUA Script");
  editor = new CodeEditor(this);
  dw2->setWidget(editor);

  addDockWidget(Qt::RightDockWidgetArea, dw2);

  // tabifyDockWidget(dw2, dw3);
}

void Gui::about() {
  QString txt =
    tr("<p><b>%1 (%2)</b></p>").arg(APP_NAME_FULL).arg(APP_VERSION) +
    tr("<p>Build: %1 - %2</p>").arg(BUILDDATE).arg(BUILDTIME) +
    tr("<p>&copy; <a href=\"mailto:jakob.flierl@gmail.com\">") +
    tr("Jakob Flierl &lt;jakob.flierl@gmail.com&gt;</a></p>");

  QMessageBox::about(this, tr("About"), txt);
}

QString Gui::strippedName(const QString &fullFileName) {
  return QFileInfo(fullFileName).fileName();
}

void Gui::parseEditor() {

}

void Gui::animStarted() {
}
void Gui::animProgress(QString fmt, int n) {
}

void Gui::animFinished() {

}

void Gui::debug(QString txt) {
}

void Gui::newFile() {
}

void Gui::openFile(const QString& path) {
}

void Gui::save() {
}

void Gui::saveAs() {
}


void Gui::saveFile(const QString& path) {
}

void Gui::editPrefs() {
}

void Gui::openRecentFile() {
}

void Gui::fontChanged(const QString& family, uint size) {
}

#define pi (3.1415926535f)

static double r2d(double radians) { return radians * 180 / pi; }

void Gui::loadSettings() {
  settings->beginGroup("gui");

  restoreGeometry(settings->value("geometry", saveGeometry() ).toByteArray());
  restoreState(settings->value("state", saveState() ).toByteArray());
  move(settings->value("pos", pos()).toPoint());
  resize(settings->value("size", size()).toSize());

  if ( settings->value("maximized", isMaximized() ).toBool()) {
    showMaximized();
  }

  settings->endGroup();
}

void Gui::saveSettings() {
  settings->beginGroup("gui");

  settings->setValue("geometry", saveGeometry());
  settings->setValue("state", saveState());
  settings->setValue("maximized", isMaximized());

  if ( !isMaximized() ) {
    settings->setValue("pos", pos());
    settings->setValue("size", size());
  }

  settings->endGroup();
}

void Gui::moveEvent(QMoveEvent *) {
}

void Gui::resizeEvent(QResizeEvent *) {
}

void Gui::closeEvent(QCloseEvent *) {
  // qDebug() << "Gui::closeEvent";
  saveSettings();
}

void Gui::command(QString cmd) {
  log("> " + cmd);
  cmd = cmd.toLower();

  if (cmd.startsWith("home")) {
    log("moving into home position.");
    ui.viewer->rm->cmdHome();
  } else if (cmd.startsWith("m")) {
    QStringList s = cmd.split(" ");
    double x = s.at(1).toDouble();
    double y = s.at(2).toDouble();
    double z = s.at(3).toDouble();
    
    log(QString("moving to <%1, %2, %3>.").arg(x).arg(y).arg(z));

    ui.viewer->rm->cmdMoveTo(x, y, z);
  } else if (cmd.startsWith("?") || cmd.startsWith("help")) {
    log("available commands:");
    log(" - home ");
    log(" - move [x][y][z] move end effector to <x,y,z>");
  }
}

void Gui::log(QString text) {
  // ui.cmdlog->appendPlainText(text);
}

void Gui::updateValues() {
}
