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
 *      FormMultiGates -
 *
 ******************************************************************************/


#ifndef FORM_MULTI_GATES_H
#define FORM_MULTI_GATES_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include <QWidget>
#include "project.h"
#include "form_coinc_peaks.h"
#include "qp_2d.h"
#include "peak2d.h"

namespace Ui {
class FormMultiGates;
}

struct Gate
{
  Bounds2D constraints;

  double cps {0};
  bool approved {false};
  Qpx::Fitter fit_data_;
};

class TableGates : public QAbstractTableModel
{
  Q_OBJECT

private:
  std::vector<Gate> gates_;

public:
  void set_data(std::vector<Gate> gates);

  explicit TableGates(QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;

  void update();
};


class FormMultiGates : public QWidget
{
  Q_OBJECT

public:
  explicit FormMultiGates(QWidget *parent = 0);
  ~FormMultiGates();

  void setSpectrum(Qpx::ProjectPtr newset, int64_t idx);

  void make_range(double energy);

  std::list<Bounds2D> get_all_peaks() const;
  std::list<Bounds2D> current_peaks() const;

  void choose_peaks(std::list<Bounds2D>);

  void clear();
  void clearSelection();

  void loadSettings();
  void saveSettings();

signals:
  void gate_selected();
  void boxes_made();

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void selection_changed(QItemSelection,QItemSelection);
  void on_pushApprove_clicked();
  void on_pushRemove_clicked();
  void on_pushDistill_clicked();
  void on_doubleGateOn_editingFinished();

  void update_range_selection(double l, double r);

  void peaks_changed_in_plot();
  void peak_selection_changed(std::set<double> sel);

  void on_pushAddGatedSpectrum_clicked();

  void on_pushAuto_clicked();

  void fitting_finished();

private:
  Ui::FormMultiGates *ui;

  Bounds2D range2d;

  Qpx::ProjectPtr project_;
  int64_t current_spectrum_ {0};
  Qpx::Metadata md_;
  int res_ {0};

  bool auto_ {false};

  std::vector<Gate> gates_;
  TableGates table_model_;
  QSortFilterProxyModel sortModel;

  Qpx::Fitter fit_data_;

  int32_t current_idx() const;
  int32_t index_of(double, bool fuzzy) const;

  Gate current_gate() const;
  Qpx::SinkPtr make_gated_spectrum(Bounds2D &bounds);
  int32_t energy_to_bin(double energy) const;

  void rebuild_table(bool contents_changed);

  void make_gate();
  void update_current_gate();
};

#endif
