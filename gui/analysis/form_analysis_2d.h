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
 *      FormAnalysis2D - 
 *
 ******************************************************************************/


#ifndef FORM_ANALYSIS_2D_H
#define FORM_ANALYSIS_2D_H

#include <QWidget>
#include "qp_2d.h"
#include "project.h"
#include "form_multi_gates.h"
#include "form_integration2d.h"

namespace Ui {
class FormAnalysis2D;
}

class FormAnalysis2D : public QWidget
{
  Q_OBJECT

public:
  explicit FormAnalysis2D(QWidget *parent = 0);
  ~FormAnalysis2D();

  void setSpectrum(Qpx::ProjectPtr newset, int64_t idx);
  void clear();
  void reset();

signals:
  void spectraChanged();
  void detectorsChanged();

public slots:
  void update_spectrum();

private slots:
  void take_boxes();
  void update_peaks();
  void detectorsUpdated() {emit detectorsChanged();}
  void initialize();
  void matrix_selection();
//  void update_range(Bounds2D);

  void on_tabs_currentChanged(int index);

  void clearSelection();
  void clickedPlot2d(double x, double y);

protected:
  void closeEvent(QCloseEvent*);
  void showEvent(QShowEvent* event);

private:
  Ui::FormAnalysis2D *ui;

  FormMultiGates*      my_gates_;
  FormIntegration2D*   form_integration_;

  Qpx::ProjectPtr project_;
  int64_t current_spectrum_;

  bool initialized;

  void loadSettings();
  void saveSettings();
};

#endif // FORM_ANALYSIS2D_H
