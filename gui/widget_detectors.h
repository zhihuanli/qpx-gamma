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
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      DialogDetector - dialog for editing the definition of detector
 *      TableDetectors - table model for detector set
 *      WidgetDetectors - dialog for managing detector set
 *
 ******************************************************************************/

#ifndef WIDGET_DETECTORS_H_
#define WIDGET_DETECTORS_H_

#include <QDir>
#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include "detector.h"
#include "qt_util.h"
#include "special_delegate.h"

namespace Ui {
class WidgetDetectors;
class DialogDetector;
}

class TableCalibrations : public QAbstractTableModel
{
  Q_OBJECT
private:
  XMLableDB<Pixie::Calibration> &myDB;
public:
  TableCalibrations(XMLableDB<Pixie::Calibration>& db, QObject *parent = 0): myDB(db), QAbstractTableModel(parent) {}
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;

  void update();
};

class DialogDetector : public QDialog
{
  Q_OBJECT
public:
  explicit DialogDetector(Pixie::Detector, QDir, QString, bool, QWidget *parent = 0);
  ~DialogDetector();

private slots:
  void on_lineName_editingFinished();
  void on_comboType_currentIndexChanged(const QString &arg1);
  void on_buttonBox_accepted();
  void on_pushReadOpti_clicked();
  void on_pushRead1D_clicked();

  void on_pushRemove_clicked();
  void selection_changed(QItemSelection,QItemSelection);

signals:
  void newDetReady(Pixie::Detector);

private:
  void updateDisplay();

  Ui::DialogDetector *ui;
  Pixie::Detector my_detector_;
  QDir root_dir_;
  QString mca_formats_;

  TableCalibrations table_model_;
  QItemSelectionModel selection_model_;
};


class TableDetectors : public QAbstractTableModel
{
  Q_OBJECT
private:
  XMLableDB<Pixie::Detector> *myDB;
public:
  TableDetectors(QObject *parent = 0): QAbstractTableModel(parent) {}
  void setDB(XMLableDB<Pixie::Detector>& db) {myDB = &db;}
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;

  void update();
};


class WidgetDetectors : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetDetectors(QWidget *parent = 0);
  ~WidgetDetectors();

  void setData(XMLableDB<Pixie::Detector> &newdb, QString outdir, QString formats);

signals:
  void detectorsUpdated();

private:
  Ui::WidgetDetectors *ui;

  XMLableDB<Pixie::Detector> *detectors_;
  TableDetectors table_model_;
  QItemSelectionModel selection_model_;

  QString root_dir_, mca_formats_;

private slots:
  void on_pushNew_clicked();
  void on_pushEdit_clicked();
  void on_pushDelete_clicked();
  void on_pushImport_clicked();
  void on_pushExport_clicked();

  void addNewDet(Pixie::Detector);
  void on_pushSetDefault_clicked();
  void on_pushGetDefault_clicked();
  void selection_changed(QItemSelection,QItemSelection);
  void toggle_push();
};

#endif // WidgetDetectors_H