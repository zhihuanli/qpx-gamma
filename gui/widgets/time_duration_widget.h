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
 *      TimeDurationWidget -
 *
 ******************************************************************************/

#ifndef TIME_DURATION_WIDGET_H_
#define TIME_DURATION_WIDGET_H_

#include <QDialog>
#include "qt_util.h"


namespace Ui {
class TimeDurationWidget;
}

class TimeDurationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit TimeDurationWidget(QWidget *parent = 0);
  ~TimeDurationWidget();

  uint64_t total_seconds();
  void set_total_seconds(uint64_t secs);

signals:
  void editingFinished();

private slots:
  void on_spinM_valueChanged(int);
  void on_spinH_valueChanged(int);
  void on_spinS_valueChanged(int);

  void on_spinDays_editingFinished();
  void on_spinH_editingFinished();
  void on_spinM_editingFinished();
  void on_spinS_editingFinished();

private:
  Ui::TimeDurationWidget *ui;

};


#endif // TIME_DURATION_WIDGET_H_
