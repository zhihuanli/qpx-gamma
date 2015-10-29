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
 *      FormSymmetrize2D -
 *
 ******************************************************************************/


#ifndef FORM_SYMMETRIZE_2D_H
#define FORM_SYMMETRIZE_2D_H

#include <QWidget>
#include <QSettings>
#include "spectrum1D.h"
#include "spectra_set.h"
#include "form_gain_calibration.h"

namespace Ui {
class FormSymmetrize2D;
}

class FormSymmetrize2D : public QWidget
{
  Q_OBJECT

public:
  explicit FormSymmetrize2D(QSettings &settings, XMLableDB<Gamma::Detector>& newDetDB, QWidget *parent = 0);
  ~FormSymmetrize2D();

  void setSpectrum(Qpx::SpectraSet *newset, QString spectrum);
  void clear();
  void reset();

signals:
  void spectraChanged();
  void detectorsChanged();

public slots:
  void apply_gain_calibration();

private slots:
  void update_gates(Marker, Marker);
  void update_peaks(bool);
  void detectorsUpdated() {emit detectorsChanged();}
  void on_pushAddGatedSpectra_clicked();
  void initialize();

  void symmetrize();

protected:
  void closeEvent(QCloseEvent*);
  void showEvent(QShowEvent* event);

private:
  Ui::FormSymmetrize2D *ui;
  QSettings &settings_;

  Qpx::Spectrum::Template *tempx, *tempy;

  Qpx::Spectrum::Spectrum *gate_x;
  Qpx::Spectrum::Spectrum *gate_y;

  Gamma::Fitter fit_data_, fit_data_2_;
  int res;

  double live_seconds;

  Gamma::Detector detector1_;
  Gamma::Detector detector2_;

  Gamma::Calibration nrg_calibration1_;
  Gamma::Calibration nrg_calibration2_;

  Gamma::Calibration gain_match_cali_;

  AppearanceProfile style_pts, style_fit;

  FormGainCalibration* my_gain_calibration_;


  //from parent
  QString data_directory_;
  Qpx::SpectraSet *spectra_;
  QString current_spectrum_;

  XMLableDB<Gamma::Detector> &detectors_;

  bool initialized;

  void configure_UI();
  void loadSettings();
  void saveSettings();
  void make_gated_spectra();
};

#endif // FORM_ANALYSIS2D_H