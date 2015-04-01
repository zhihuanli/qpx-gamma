/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Description:
 *      DialogDetector - dialog for editing the definition of detector
 *      TableDetectors - table model for detector set
 *      WidgetDetectors - dialog for managing detector set
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

#include "widget_detectors.h"
#include "ui_widget_detectors.h"
#include "ui_dialog_detector.h"
#include <QFileDialog>
#include <QMessageBox>
#include "spectra_set.h"
#include <boost/algorithm/string.hpp>

DialogDetector::DialogDetector(Pixie::Detector mydet, QDir rd, QString formats, bool editName, QWidget *parent) :
  root_dir_(rd),
  mca_formats_(formats),
  my_detector_(mydet),
  table_model_(my_detector_.energy_calibrations_),
  selection_model_(&table_model_),
  QDialog(parent),
  ui(new Ui::DialogDetector)
{
  ui->setupUi(this);

  ui->comboType->insertItem(0, QString::fromStdString("HPGe"));
  ui->comboType->insertItem(1, QString::fromStdString("NaI"));

  ui->lineName->setEnabled(editName);
  my_detector_ = mydet;

  ui->tableCalibrations->setModel(&table_model_);
  ui->tableCalibrations->setSelectionModel(&selection_model_);
  ui->tableCalibrations->verticalHeader()->hide();
  ui->tableCalibrations->horizontalHeader()->setStretchLastSection(true);
  ui->tableCalibrations->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableCalibrations->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableCalibrations->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableCalibrations->show();

  connect(&selection_model_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selection_changed(QItemSelection,QItemSelection)));

  updateDisplay();
}

DialogDetector::~DialogDetector()
{
  delete ui;
}

void DialogDetector::updateDisplay() {
  ui->lineName->setText(QString::fromStdString(my_detector_.name_));
  ui->lineName->setCursorPosition(0);
  ui->comboType->setCurrentText(QString::fromStdString(my_detector_.type_));

  if (my_detector_.setting_names_.empty())
    ui->labelOpti->setText(tr("WITHOUT CHANNEL SETTINGS"));
  else
    ui->labelOpti->setText(tr("WITH CHANNEL SETTINGS"));
  table_model_.update();
}

void DialogDetector::on_lineName_editingFinished()
{
  my_detector_.name_ = boost::algorithm::trim_copy(ui->lineName->text().toStdString());
}

void DialogDetector::on_comboType_currentIndexChanged(const QString &arg1)
{
  my_detector_.type_ = arg1.toStdString();
}

void DialogDetector::on_buttonBox_accepted()
{
  if (my_detector_.name_ == Pixie::Detector().name_) {
    QMessageBox msgBox;
    msgBox.setText("Please give it a proper name");
    msgBox.exec();
  } else
    emit newDetReady(my_detector_);
}


void DialogDetector::on_pushReadOpti_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", root_dir_.absolutePath(),
                                                  "Qpx project file (*.qpx)");
  if (!validateFile(this, fileName, false))
    return;

  Pixie::SpectraSet optiSource;
  optiSource.read_xml(fileName.toStdString(), false);
  optiSource.runInfo().p4_state.save_optimization();
  for (auto &q : optiSource.runInfo().p4_state.get_detectors())
    if (q.name_ == my_detector_.name_)
      my_detector_ = q;
  updateDisplay();
}

void DialogDetector::on_pushRead1D_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Load spectrum", root_dir_.absolutePath(), mca_formats_);

  if (!validateFile(this, fileName, false))
    return;

  this->setCursor(Qt::WaitCursor);

  Pixie::Spectrum::Spectrum* newSpectrum = Pixie::Spectrum::Factory::getInstance().create_from_file(fileName.toStdString());
  if (newSpectrum != nullptr) {
    std::vector<Pixie::Detector> dets = newSpectrum->get_detectors();
    for (auto &q : dets)
      if (q != Pixie::Detector()) {
        PL_INFO << "Looking at calibrations from detector " << q.name_;
        for (auto &p : q.energy_calibrations_.my_data_) {
          if (p.bits_) {
            my_detector_.energy_calibrations_.add(p);
            my_detector_.energy_calibrations_.replace(p); //ask user
          }
        }
      }
    delete newSpectrum;
    updateDisplay();
  } else {
    PL_INFO << "Spectrum construction did not succeed. Aborting";
    delete newSpectrum;
  }

  this->setCursor(Qt::ArrowCursor);
}

void DialogDetector::selection_changed(QItemSelection, QItemSelection) {
  if (selection_model_.selectedIndexes().empty())
    ui->pushRemove->setEnabled(false);
  else
    ui->pushRemove->setEnabled(true);
}

void DialogDetector::on_pushRemove_clicked()
{
  QModelIndexList ixl = ui->tableCalibrations->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  int i = ixl.front().row();
  my_detector_.energy_calibrations_.remove(i);
  table_model_.update();
}




int TableDetectors::rowCount(const QModelIndex & /*parent*/) const
{    return myDB->size(); }

int TableDetectors::columnCount(const QModelIndex & /*parent*/) const
{    return 4; }

QVariant TableDetectors::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();
  std::stringstream dss;

  if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
  {
    switch (col) {
    case 0:
      return QString::fromStdString(myDB->get(row).name_);
    case 1:
      return QString::fromStdString(myDB->get(row).type_);
    case 2:
      if (myDB->get(row).setting_names_.empty())
        return "no";
      else
        return "yes";
    case 3:
      dss.str(std::string());
      for (auto &q : myDB->get(row).energy_calibrations_.my_data_) {
        dss << q.bits_ << " ";
      }
      return QString::fromStdString(dss.str());
    }
  }
  return QVariant();
}

QVariant TableDetectors::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return "Name";
      case 1:
        return "Type";
      case 2:
        return "Optimization";
      case 3:
        return "Calib. resolutions (bits)";
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}


void TableDetectors::update() {
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex( rowCount() - 1, 3 );
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags TableDetectors::flags(const QModelIndex &index) const
{
  return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
}




int TableCalibrations::rowCount(const QModelIndex & /*parent*/) const
{    return myDB.size(); }

int TableCalibrations::columnCount(const QModelIndex & /*parent*/) const
{    return 4; }

QVariant TableCalibrations::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();
  std::stringstream dss;

  if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
  {
    switch (col) {
    case 0:
      return QString::number(myDB.get(row).bits_);
    case 1:
      return QString::fromStdString(myDB.get(row).units_);
    case 2:
      return QString::fromStdString(boost::gregorian::to_simple_string(myDB.get(row).calib_date_.date()));
    case 3:
      dss.str(std::string());
      for (auto &q : myDB.get(row).coefficients_) {
        dss << q << " ";
      }
      return QString::fromStdString(dss.str());
    }
  }
  return QVariant();
}

QVariant TableCalibrations::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return "Bits";
      case 1:
        return "Units";
      case 2:
        return "Date";
      case 3:
        return "Coefficients";
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}


void TableCalibrations::update() {
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex( rowCount() - 1, 3 );
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags TableCalibrations::flags(const QModelIndex &index) const
{
  return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
}




WidgetDetectors::WidgetDetectors(QWidget *parent) :
  QWidget(parent),
  selection_model_(&table_model_),
  ui(new Ui::WidgetDetectors)
{
  ui->setupUi(this);

  connect(&selection_model_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selection_changed(QItemSelection,QItemSelection)));
}

void WidgetDetectors::setData(XMLableDB<Pixie::Detector> &newdb, QString outdir, QString formats) {
  table_model_.setDB(newdb);
  detectors_ = &newdb;
  root_dir_  = outdir;
  mca_formats_ = formats;

  ui->tableDetectorDB->setModel(&table_model_);
  ui->tableDetectorDB->setSelectionModel(&selection_model_);
  ui->tableDetectorDB->verticalHeader()->hide();
  ui->tableDetectorDB->horizontalHeader()->setStretchLastSection(true);
  ui->tableDetectorDB->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableDetectorDB->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableDetectorDB->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableDetectorDB->show();

}

WidgetDetectors::~WidgetDetectors()
{
  delete ui;
}

void WidgetDetectors::selection_changed(QItemSelection, QItemSelection) {
  toggle_push();
}

void WidgetDetectors::toggle_push() {
  if (selection_model_.selectedIndexes().empty()) {
    ui->pushEdit->setEnabled(false);
    ui->pushDelete->setEnabled(false);
  } else {
    ui->pushEdit->setEnabled(true);
    ui->pushDelete->setEnabled(true);
  }
  emit detectorsUpdated();
}

void WidgetDetectors::on_pushNew_clicked()
{
  DialogDetector* newDet = new DialogDetector(Pixie::Detector(), QDir(root_dir_), mca_formats_, true, this);
  connect(newDet, SIGNAL(newDetReady(Pixie::Detector)), this, SLOT(addNewDet(Pixie::Detector)));
  newDet->exec();
}

void WidgetDetectors::on_pushEdit_clicked()
{
  QModelIndexList ixl = ui->tableDetectorDB->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  int i = ixl.front().row();

  DialogDetector* newDet = new DialogDetector(detectors_->get(i), QDir(root_dir_), mca_formats_, false, this);
  connect(newDet, SIGNAL(newDetReady(Pixie::Detector)), this, SLOT(addNewDet(Pixie::Detector)));
  newDet->exec();
}

void WidgetDetectors::on_pushDelete_clicked()
{
  QItemSelectionModel *selected = ui->tableDetectorDB->selectionModel();
  QModelIndexList rowList = selected->selectedRows();
  if (rowList.empty())
    return;
  detectors_->remove(rowList.front().row());
  selection_model_.reset();
  table_model_.update();
  toggle_push();
}

void WidgetDetectors::on_pushImport_clicked()
{
  // ask delete old or append
  QString fileName = QFileDialog::getOpenFileName(this, "Load detector settings",
                                                  root_dir_, "Detector settings (*.det)");
  if (validateFile(this, fileName, false)) {
    PL_INFO << "Reading detector settings from xml file " << fileName.toStdString();
    detectors_->read_xml(fileName.toStdString());
    selection_model_.reset();
    table_model_.update();
    toggle_push();
  }
}

void WidgetDetectors::on_pushExport_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(this, "Save detector settings",
                                                  root_dir_, "Detector settings (*.det)");
  if (validateFile(this, fileName, true)) {
    QFileInfo file(fileName);
    if (file.suffix() != "det")
      fileName += ".det";
    PL_INFO << "Writing detector settings to xml file " << fileName.toStdString();
    detectors_->write_xml(fileName.toStdString());
  }
}

void WidgetDetectors::addNewDet(Pixie::Detector newDetector) {
  detectors_->add(newDetector);
  detectors_->replace(newDetector);
  selection_model_.reset();
  table_model_.update();
  toggle_push();
}

void WidgetDetectors::on_pushSetDefault_clicked()
{
  //ask sure?
  detectors_->write_xml(root_dir_.toStdString() + "/default_detectors.det");
}

void WidgetDetectors::on_pushGetDefault_clicked()
{
  //ask sure?
  detectors_->clear();
  detectors_->read_xml(root_dir_.toStdString() + "/default_detectors.det");
  selection_model_.reset();
  table_model_.update();
  toggle_push();
}

