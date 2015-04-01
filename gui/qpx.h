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
 *      qpx - main application window
 *
 ******************************************************************************/

#ifndef qpx_H
#define qpx_H

#include <QMainWindow>
#include <QSettings>

#include "custom_logger.h"
#include "qt_boost_logger.h"

#include "thread_runner.h"

#include "form_start.h"
#include "form_calibration.h"


namespace Ui {
class qpx;
}

class qpx : public QMainWindow
{
  Q_OBJECT

public:
  explicit qpx(QWidget *parent = 0);
  ~qpx();

private:
  Ui::qpx *ui;

  //connect gui with boost logger framework
  std::stringstream qpx_stream_;
  LogEmitter        my_emitter_;
  LogStreamBuffer   text_buffer_;

  QString                       data_directory_;    //data directory
  QSettings                     settings_;
  XMLableDB<Pixie::Detector>    detectors_;
  ThreadRunner                  runner_thread_;

  FormStart* main_tab_;

  //helper functions
  void saveSettings();
  void loadSettings();

signals:
  void toggle_push(bool, Pixie::LiveStatus);

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void toggleIO(bool);
  void updateStatusText(QString);

  void tabCloseRequested(int index);

  //logger receiver
  void add_log_text(QString);

  //application settings
  void on_OutputDirFind_clicked();

  void on_splitter_splitterMoved(int pos, int index);

  void calibrate(FormCalibration*);
  void detectors_updated();

  void on_pushAbout_clicked();

  void on_pushOpenSpectra_clicked();
  void on_pushOpenList_clicked();
  void on_pushOpenOptimize_clicked();
};

#endif // qpx_H