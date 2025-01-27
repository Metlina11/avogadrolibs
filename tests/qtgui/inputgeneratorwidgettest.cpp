/******************************************************************************
  This source file is part of the Avogadro project.
  This source code is released under the 3-Clause BSD License, (see "LICENSE").
******************************************************************************/

#include <gtest/gtest.h>

#include "qtguitests.h"

#include <avogadro/molequeue/inputgeneratorwidget.h>
#include <avogadro/qtgui/filebrowsewidget.h>
#include <avogadro/qtgui/molecule.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>

#include <QtCore/QFile>
#include <QtCore/QTimer>

using Avogadro::MoleQueue::InputGeneratorWidget;
using Avogadro::QtGui::FileBrowseWidget;
using Avogadro::QtGui::Molecule;

namespace {

void flushEvents()
{
  // Post a quit event at the end of the event queue
  QTimer::singleShot(0, qApp, SLOT(quit()));
  // Process events in the queue
  qApp->exec();
}

} // namespace

TEST(InputGeneratorWidgetTest, exercise)
{
  // Fake a QApplication -- needed to instantiate widgets.
  int argc = 1;
  char argName[] = "FakeApp.exe";
  char* argv[2] = { argName, nullptr };
  QApplication app(argc, argv);
  Q_UNUSED(app);

  // Setup the widget
  InputGeneratorWidget widget;
  QString scriptFilePath(AVOGADRO_DATA
                         "/tests/avogadro/scripts/inputgeneratortest.py");
  widget.setInputGeneratorScript(scriptFilePath);
  Molecule mol;
  mol.addAtom(6).setPosition3d(Avogadro::Vector3(1, 1, 1));
  mol.addAtom(1).setPosition3d(Avogadro::Vector3(2, 3, 4));
  mol.addAtom(8).setPosition3d(Avogadro::Vector3(-2, 3, -4));
  widget.setMolecule(&mol);

  // Check that the generator is configured properly.
  EXPECT_EQ(widget.inputGenerator().displayName().toStdString(),
            std::string("Input Generator Test"));

  // Verify that appropriate widgets are produced for each parameter type:
  EXPECT_TRUE(widget.findChild<QComboBox*>("Test StringList") != nullptr);
  EXPECT_TRUE(widget.findChild<QLineEdit*>("Test String") != nullptr);
  EXPECT_TRUE(widget.findChild<QSpinBox*>("Test Integer") != nullptr);
  EXPECT_TRUE(widget.findChild<QCheckBox*>("Test Boolean") != nullptr);
  EXPECT_TRUE(widget.findChild<FileBrowseWidget*>("Test FilePath") != nullptr);

  // Set a test filepath
  FileBrowseWidget* testFilePathWidget(
    widget.findChild<FileBrowseWidget*>("Test FilePath"));
  QString testFilePath(AVOGADRO_DATA "/data/ethane.cml");
  testFilePathWidget->setFileName(testFilePath);

  // Show the widget so that events are processed
  widget.show();

  // Clear out the event queue so that the text edits are updated:
  flushEvents();

  // Check the contents of the filepath file:
  QTextEdit* filePathEdit = widget.findChild<QTextEdit*>("job.testFilePath");
  QFile testFile(testFilePath);
  EXPECT_TRUE(testFile.open(QFile::ReadOnly | QFile::Text));
  QByteArray refData(testFile.readAll());
  EXPECT_EQ(std::string(refData.constData()),
            filePathEdit->document()->toPlainText().toStdString());

  // Check the coords:
  QTextEdit* coordsEdit = widget.findChild<QTextEdit*>("job.coords");
  QString coords(coordsEdit->document()->toPlainText());
  EXPECT_TRUE(
    coords.contains("C      1.000000 0    1.000000 1    1.000000 1 Carbon"));
  EXPECT_TRUE(
    coords.contains("H      2.000000 0    3.000000 1    4.000000 1 Hydrogen"));
  EXPECT_TRUE(
    coords.contains("O     -2.000000 0    3.000000 1   -4.000000 1 Oxygen"));

  // Test the default reset -- trigger a reset, then verify that testFilePath
  // is cleared (we set it earlier)
  QPushButton* defaultsButton(widget.findChild<QPushButton*>("defaultsButton"));
  defaultsButton->click();
  flushEvents();
  EXPECT_TRUE(testFilePathWidget->fileName().isEmpty());
  EXPECT_EQ(filePathEdit->document()->toPlainText().toStdString(),
            std::string("Reference file '' does not exist."));

  // Test the autogenerated title:
  QLineEdit* titleEdit = widget.findChild<QLineEdit*>("Title");
  EXPECT_EQ(titleEdit->placeholderText().toStdString(),
            std::string("CHO | Equilibrium Geometry | B3LYP/6-31G(d)"));
}
