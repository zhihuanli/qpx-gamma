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
 *      TableListData - table to display acquired list mode data
 *
 ******************************************************************************/

#ifndef FORM_LIST_DAQ_H
#define FORM_LIST_DAQ_H

#include <QWidget>
#include <QSettings>
#include "daq_types.h"
#include "thread_runner.h"
#include "special_delegate.h"

#include <QItemSelectionModel>
#include <QAbstractTableModel>


class TableListData : public QAbstractTableModel
{
    Q_OBJECT
private:
    Pixie::ListData* mystuff;
    double time_factor_;
    std::vector<Pixie::Calibration> calibrations_;

public:
    TableListData(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;

    void eat_list(Pixie::ListData*);
    void update();
};

namespace Ui {
class FormListDaq;
}

class FormListDaq : public QWidget
{
  Q_OBJECT

public:
  explicit FormListDaq(ThreadRunner&, QSettings&, QWidget *parent = 0);
  ~FormListDaq();

signals:
  void toggleIO(bool);
  void statusText(QString);

private slots:
  void on_pushListStart_clicked();
  void on_pushListStop_clicked();
  void on_boxChan0_clicked();
  void on_boxChan1_clicked();
  void on_boxChan2_clicked();
  void on_boxChan3_clicked();
  void list_completed(Pixie::ListData*);
  void list_selection_changed(QItemSelection,QItemSelection);
  void toggle_push(bool online, Pixie::LiveStatus);

protected:
  void closeEvent(QCloseEvent*);

private:
  Ui::FormListDaq     *ui;
  ThreadRunner        &runner_thread_;
  QSettings           &settings_;

  boost::atomic<bool> interruptor_;

  Pixie::ListData     *list_data_;

  //list mode data
  TableListData       list_data_model_;
  QItemSelectionModel list_selection_model_;
  QpxSpecialDelegate  list_delegate_;

  void displayTraces();

  void loadSettings();
  void saveSettings();
};

#endif // FORM_LIST_DAQ_H