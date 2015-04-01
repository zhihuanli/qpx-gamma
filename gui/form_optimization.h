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
 *     Optimization UI
 *
 ******************************************************************************/

#ifndef FORM_OPTIMIZATION_H
#define FORM_OPTIMIZATION_H

#include <QWidget>
#include <QSettings>
#include "spectra_set.h"
#include "thread_plot_signal.h"
#include "thread_runner.h"
#include "widget_plot1d.h"

namespace Ui {
class FormOptimization;
}

class FormOptimization : public QWidget
{
  Q_OBJECT

public:
  explicit FormOptimization(ThreadRunner&, QSettings&, XMLableDB<Pixie::Detector>&, QWidget *parent = 0);
  ~FormOptimization();

signals:
  void toggleIO(bool);
  void restart_run();
  void post_proc();
  void optimization_approved();

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void on_pushTakeOne_clicked();
  void update_plots();
  void run_completed();

  void on_pushMatchGain_clicked();
  void addMovingMarker(double);
  void removeMovingMarker(double);
  void replot_markers();

  bool find_peaks();

  void on_pushStop_clicked();
  void do_post_processing();
  void do_run();

  void toggle_push(bool, Pixie::LiveStatus);

  void on_pushSaveOpti_clicked();

private:
  Ui::FormOptimization *ui;

  Pixie::Wrapper&      pixie_;
  Pixie::SpectraSet    spectra_;
  Pixie::Spectrum::Template reference_, optimizing_;

  ThreadRunner         &runner_thread_;
  XMLableDB<Pixie::Detector> &detectors_;
  QSettings &settings_;

  ThreadPlotSignal     plot_thread_;
  boost::atomic<bool>  interruptor_;

  std::vector<double> x, y_ref, y_opt;
  double max_x;

  bool running;

  Marker moving, refm, optm, a, b;
  int bits, current_pass;
  int ref_peak, opt_peak, refpk, optpk;

  std::vector<int> markers_ref, markers_opt; //channel

};

#endif // FORM_OPTIMIZATION_H