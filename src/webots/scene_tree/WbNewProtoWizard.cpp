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

#include "WbApplicationInfo.hpp"
#include "WbNode.hpp"
#include "WbProject.hpp"
#include "WbVersion.hpp"
#include "WbVrmlWriter.hpp"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWizardPage>

WbNewProtoWizard::WbNewProtoWizard(WbNode *node, QWidget *parent) : QWizard(parent) {
  mNode = node;
  mNeedsEdit = false;
  setPage(INTRO, createIntroPage());
  setPage(NAME, createNamePage());
  setPage(HEADER, createHeaderPage());
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
    const QFileInfo fileInfo(file.fileName());
    WbVrmlWriter writer(&file, filename);
    writer << "#VRML_SIM " << WbApplicationInfo::version().toString(false) << " utf8\n";
    if (!mLicenseEdit->text().isEmpty())
      writer << "# license: " << mLicenseEdit->text() << "\n";
    if (!mLicenseUrlEdit->text().isEmpty())
      writer << "# license url: " << mLicenseUrlEdit->text() << "\n";
    if (!mDocumentationUrlEdit->text().isEmpty())
      writer << "# documentation url: " << mDocumentationUrlEdit->text() << "\n";
    foreach (const QString line, mDescriptionTextEdit->toPlainText().split('\n'))
      writer << "# " << line << "\n";
    writer << "\nPROTO " << fileInfo.fileName().split(".", QString::SkipEmptyParts).at(0) << " [\n";
    writer << "]\n";
    writer << "{\n  ";
    writer.increaseIndent();
    writer.writeHeader(filename);
    mNode->write(writer);
    writer.writeFooter();
    writer.decreaseIndent();
    writer << "\n}\n";
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

  mFilenameEdit = new QLineEdit(page);
  QPushButton *chooseButton = new QPushButton(tr("Choose"), page);
  connect(chooseButton, &QPushButton::pressed, this, &WbNewProtoWizard::chooseFilename);

  mFileNameWarning = new QLabel(page);

  QVBoxLayout *vLayout = new QVBoxLayout(page);
  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->addWidget(mFilenameEdit);
  hLayout->addWidget(chooseButton);
  vLayout->addLayout(hLayout);
  vLayout->addWidget(mFileNameWarning);

  return page;
}

QWizardPage *WbNewProtoWizard::createHeaderPage() {
  QWizardPage *page = new QWizardPage(this);
  page->setTitle(tr("PROTO Header"));
  page->setSubTitle(tr("Define the header information of the PROTO."));

  QLabel *licenseLabel = new QLabel(tr("License"), page);
  mLicenseEdit = new QLineEdit("Apache License 2.0", page);
  QLabel *licenseUrlLabel = new QLabel(tr("License url"), page);
  mLicenseUrlEdit = new QLineEdit("http://www.apache.org/licenses/LICENSE-2.0", page);
  QLabel *documentationUrlLabel = new QLabel(tr("Documentation url:"), page);
  mDocumentationUrlEdit = new QLineEdit(page);

  QLabel *descriptionLabel = new QLabel(tr("PROTO description:"), page);
  mDescriptionTextEdit = new QPlainTextEdit(tr("Simulation model of ..."));
  mDescriptionTextEdit->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);

  QGridLayout *layout = new QGridLayout(page);
  layout->addWidget(licenseLabel, 0, 0);
  layout->addWidget(mLicenseEdit, 0, 1);
  layout->addWidget(licenseUrlLabel, 1, 0);
  layout->addWidget(mLicenseUrlEdit, 1, 1);
  layout->addWidget(documentationUrlLabel, 2, 0);
  layout->addWidget(mDocumentationUrlEdit, 2, 1);
  layout->addWidget(descriptionLabel, 3, 0);
  layout->addWidget(mDescriptionTextEdit, 4, 0, 1, 2);

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
