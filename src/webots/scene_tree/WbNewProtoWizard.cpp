// Copyright 1996-2020 Cyberbotics Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "WbNewProtoWizard.hpp"

#include "WbLineEdit.hpp"
#include "WbNode.hpp"
#include "WbProject.hpp"
#include "WbVrmlWriter.hpp"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWizardPage>

WbNewProtoWizard::WbNewProtoWizard(WbNode *node, QWidget *parent) : QWizard(parent) {
  mNode = node;
  mNeedsEdit = false;
  setPage(INTRO, createIntroPage());
  setPage(NAME, createNamePage());
  setPage(CONCLUSION, createConclusionPage());
  setOption(QWizard::NoCancelButton, false);
  setOption(QWizard::CancelButtonOnLeft, true);
  setWindowTitle(tr("Create a new PROTO file from a node."));
}

void WbNewProtoWizard::updateUI() {
  mEditCheckBox->setText(tr("Open '%1' in Text Editor.").arg(mFilenameEdit->text()));
}

bool WbNewProtoWizard::validateCurrentPage() {
  updateUI();
  if (currentId() == NAME)
    return validateFilename();
  return true;
}

void WbNewProtoWizard::accept() {
  mNeedsEdit = mEditCheckBox->isChecked();

  const QString filename = mFilenameEdit->text();
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly)) {
    WbVrmlWriter writer(&file, filename);
    writer.writeHeader(filename);
    mNode->write(writer);
    writer.writeFooter();
    file.close();
  }

  QDialog::accept();
}

QWizardPage *WbNewProtoWizard::createIntroPage() {
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("New PROTO creation"));
  QLabel *label = new QLabel(tr("This wizard will help you creating a PROTO file from the selected node."), page);

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(label);

  return page;
}

bool WbNewProtoWizard::validateFilename() {
  const QString filename = mFilenameEdit->text();
  mFileNameWarning->clear();

  if (filename.isEmpty())
    return false;

  if (!filename.endsWith(".proto")) {
    mFileNameWarning->setText("WARNING: filename must end with '.proto'.");
    return false;
  }

  return true;
}

void WbNewProtoWizard::chooseFilename() {
  mFilenameEdit->setText(QFileDialog::getSaveFileName(this, tr("Choose a name for your PROTO file."),
                                                      WbProject::current()->protosPath(), tr("PROTO (*.proto)")));
  validateFilename();
}

QWizardPage *WbNewProtoWizard::createNamePage() {
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("File selection"));
  page->setSubTitle(tr("Please choose a filename for your PROTO:"));

  mFilenameEdit = new WbLineEdit(page);
  QPushButton *chooseButton = new QPushButton(tr("Choose"), page);
  connect(chooseButton, &QPushButton::pressed, this, &WbNewProtoWizard::chooseFilename);

  mFileNameWarning = new QLabel(page);

  QVBoxLayout *vLayout = new QVBoxLayout(page);
  QHBoxLayout *hLayout = new QHBoxLayout(page);
  hLayout->addWidget(mFilenameEdit);
  hLayout->addWidget(chooseButton);
  vLayout->addLayout(hLayout);
  vLayout->addWidget(mFileNameWarning);

  return page;
}

QWizardPage *WbNewProtoWizard::createConclusionPage() {
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("Conclusion"));
  page->setSubTitle(tr("The following file will be created:"));

  QLabel *label = new QLabel(mFilenameEdit->text(), page);
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->addWidget(label);
  // add space
  layout->addSpacing(30);
  // add check box
  mEditCheckBox = new QCheckBox(page);
  mEditCheckBox->setChecked(true);
  layout->addWidget(mEditCheckBox);

  return page;
}
