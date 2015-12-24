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
 *      Gain matching interface
 *
 ******************************************************************************/

#include "form_gain_match.h"
#include "ui_form_gain_match.h"
#include "spectrum1D.h"
#include "custom_logger.h"
#include "fityk.h"


FormGainMatch::FormGainMatch(ThreadRunner& thread, QSettings& settings, XMLableDB<Gamma::Detector>& detectors, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FormGainMatch),
  minTimer(nullptr),
  gm_runner_thread_(thread),
  settings_(settings),
  detectors_(detectors),
  gm_spectra_(),
  gm_interruptor_(false),
  gm_plot_thread_(gm_spectra_),
  my_run_(false)
{
  ui->setupUi(this);

  loadSettings();
  connect(&gm_runner_thread_, SIGNAL(runComplete()), this, SLOT(run_completed()));

  if (Qpx::Spectrum::Template *temp = Qpx::Spectrum::Factory::getInstance().create_template("1D")) {
    reference_   = *temp;
    optimizing_  = *temp;
    delete temp;
  }

  current_pass = 0;

  reference_.name_ = "Reference";
  reference_.visible = true;
  reference_.appearance = QColor(Qt::blue).rgba();

  optimizing_.name_ = "Optimizing";
  optimizing_.visible = true;
  optimizing_.appearance = QColor(Qt::red).rgba();

  moving_.appearance.themes["light"] = QPen(Qt::darkYellow, 2);
  moving_.appearance.themes["dark"] = QPen(Qt::yellow, 2);

  a.appearance.themes["light"] = QPen(Qt::darkYellow, 1);
  a.appearance.themes["dark"] = QPen(Qt::yellow, 1);

  QColor translu(Qt::darkYellow);
  translu.setAlpha(32);
  b.appearance.themes["light"] = QPen(translu, 1);
  translu.setAlpha(64);
  b.appearance.themes["dark"] = QPen(translu, 1);

  ap_reference_.themes["light"] = QPen(Qt::darkCyan, 0);
  ap_reference_.themes["dark"] = QPen(Qt::cyan, 0);

  ap_optimized_.themes["light"] = QPen(Qt::darkMagenta, 0);
  ap_optimized_.themes["dark"] = QPen(Qt::magenta, 0);

  marker_ref_.appearance = ap_reference_;
  marker_ref_.visible = true;

  marker_opt_.appearance = ap_optimized_;
  marker_opt_.visible = true;


  style_fit.default_pen = QPen(Qt::blue, 0);
  style_pts.default_pen = QPen(Qt::darkBlue, 7);
  style_pts.themes["selected"] = QPen(Qt::red, 7);

  ui->PlotCalib->setLabels("channel", "energy");



  connect(&gm_plot_thread_, SIGNAL(plot_ready()), this, SLOT(update_plots()));

  connect(ui->plot, SIGNAL(clickedLeft(double)), this, SLOT(addMovingMarker(double)));
  connect(ui->plot, SIGNAL(clickedRight(double)), this, SLOT(removeMovingMarker(double)));

  update_settings();

  gm_plot_thread_.start();
}

FormGainMatch::~FormGainMatch()
{
  delete ui;
}

void FormGainMatch::update_settings() {
  ui->comboReference->clear();
  ui->comboTarget->clear();

  //should come from other thread?
  std::vector<Gamma::Detector> chans = Qpx::Engine::getInstance().get_detectors();

  for (int i=0; i < chans.size(); ++i) {
    QString text = "[" + QString::number(i) + "] "
        + QString::fromStdString(chans[i].name_);
    ui->comboReference->addItem(text, QVariant::fromValue(i));
    ui->comboTarget->addItem(text, QVariant::fromValue(i));
  }
}

void FormGainMatch::loadSettings() {
  settings_.beginGroup("GainMatching");
  ui->spinBits->setValue(settings_.value("bits", 14).toInt());
//  ui->spinRefChan->setValue(settings_.value("reference_channel", 0).toInt());
//  ui->spinOptChan->setValue(settings_.value("optimized_channel", 1).toInt());
  ui->timeDuration->set_total_seconds(settings_.value("mca_secs", 0).toULongLong());
  ui->spinMaxPasses->setValue(settings_.value("maximum_passes", 30).toInt());
  ui->spinPeakWindow->setValue(settings_.value("peak_window", 180).toInt());
  ui->doubleSpinDeltaV->setValue(settings_.value("delta_V", 0.000300).toDouble());
  settings_.endGroup();
}

void FormGainMatch::saveSettings() {
  settings_.beginGroup("GainMatching");
  settings_.setValue("bits", ui->spinBits->value());
//  settings_.setValue("reference_channel", ui->spinRefChan->value());
//  settings_.setValue("optimized_channel", ui->spinOptChan->value());
  settings_.setValue("mca_secs", QVariant::fromValue(ui->timeDuration->total_seconds()));
  settings_.setValue("maximum_passes", ui->spinMaxPasses->value());
  settings_.setValue("peak_window", ui->spinPeakWindow->value());
  settings_.setValue("delta_V", ui->doubleSpinDeltaV->value());
  settings_.endGroup();
}

void FormGainMatch::closeEvent(QCloseEvent *event) {
  if (my_run_ && gm_runner_thread_.running()) {
    int reply = QMessageBox::warning(this, "Ongoing data acquisition",
                                     "Terminate?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
      gm_runner_thread_.terminate();
      gm_runner_thread_.wait();
    } else {
      event->ignore();
      return;
    }
  } else {
    gm_runner_thread_.terminate();
    gm_runner_thread_.wait();
  }


  if (!gm_spectra_.empty()) {
    int reply = QMessageBox::warning(this, "Gain matching data still open",
                                     "Discard?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
      gm_spectra_.clear();
    } else {
      event->ignore();
      return;
    }
  }

  gm_spectra_.terminate();
  gm_plot_thread_.wait();
  saveSettings();

  event->accept();
}

void FormGainMatch::toggle_push(bool enable, Qpx::DeviceStatus status) {
  bool online = (status & Qpx::DeviceStatus::can_run);
  ui->pushMatchGain->setEnabled(enable && online);


  ui->timeDuration->setEnabled(enable);

  ui->comboReference->setEnabled(enable);
  ui->comboTarget->setEnabled(enable);
  ui->spinBits->setEnabled(enable && online);
  ui->spinMaxPasses->setEnabled(enable && online);
}

void FormGainMatch::do_run()
{
  ui->pushStop->setEnabled(true);
  emit toggleIO(false);
  my_run_ = true;

  bits = ui->spinBits->value();

  gm_spectra_.clear();
  reference_.bits = bits;
  int refchan = ui->comboReference->currentData().toInt();
  reference_.add_pattern.resize(refchan + 1, 0);
  reference_.add_pattern[refchan] = 1;
  reference_.match_pattern.resize(refchan + 1, 0);
  reference_.match_pattern[refchan] = 1;

  optimizing_.bits = bits;
  int optchan = ui->comboTarget->currentData().toInt();
  optimizing_.add_pattern.resize(optchan + 1, 0);
  optimizing_.add_pattern[optchan] = 1;
  optimizing_.match_pattern.resize(optchan + 1, 0);
  optimizing_.match_pattern[optchan] = 1;

  XMLableDB<Qpx::Spectrum::Template> db("SpectrumTemplates");
  db.add(reference_);
  db.add(optimizing_);

  gm_plot_thread_.start();

  gm_spectra_.set_spectra(db);
  ui->plot->clearGraphs();

  if (minTimer != nullptr)
    delete minTimer;

  minTimer = new CustomTimer(true);
  gm_runner_thread_.do_run(gm_spectra_, gm_interruptor_, 0);
}

void FormGainMatch::run_completed() {
  if (my_run_) {
    gm_spectra_.clear();
    gm_spectra_.terminate();
    gm_plot_thread_.wait();
    marker_ref_.visible = false;
    marker_opt_.visible = false;
    gauss_ref_.gaussian_.height_ = 0;
    gauss_opt_.gaussian_.height_ = 0;
    //replot_markers();

    if ((current_pass < max_passes)  && !gm_runner_thread_.terminating()){
      PL_INFO << "[Gain matching] Passes remaining " << max_passes - current_pass;
      current_pass++;
      do_post_processing();
    } else {
      ui->pushStop->setEnabled(false);
      emit toggleIO(true);
      my_run_ = false;
    }
  }
}

void FormGainMatch::do_post_processing() {
  PL_DBG << "[Gain matching] Gain post-processing...";

  update_plots();


  int optchan = ui->comboTarget->currentData().toInt();
  Gamma::Setting gain_setting("VGAIN", optchan);

  Qpx::Engine::getInstance().get_all_settings();
  gain_setting = Qpx::Engine::getInstance().pull_settings().get_setting(gain_setting, Gamma::Match::name | Gamma::Match::indices);
  double old_gain = gain_setting.value_dbl;
  QThread::sleep(1);
  double delta = gauss_opt_.gaussian_.center_ - gauss_ref_.gaussian_.center_;

  PL_DBG << "[Gain matching] current gain = " << old_gain << " centroid = " << gauss_opt_.gaussian_.center_ << " with delta = " << delta;

  gains.push_back(old_gain);
  deltas.push_back(gauss_opt_.gaussian_.center_);

  double new_gain = old_gain;
  if (gauss_opt_.gaussian_.center_ < gauss_ref_.gaussian_.center_)
    new_gain += ui->doubleSpinDeltaV->value();
  else if (gauss_opt_.gaussian_.center_ > gauss_ref_.gaussian_.center_)
    new_gain -= ui->doubleSpinDeltaV->value();

  if (gains.size() > 1) {
    Polynomial p = Polynomial(gains, deltas, 2);
    double predict = p.inverse_evaluate(gauss_ref_.gaussian_.center_/*, ui->doubleThreshold->value() / 4.0*/);
    PL_INFO << "[Gain matching] for next pass, predicted gain = " << predict;
    new_gain = predict;

    QVector<double> xx = QVector<double>::fromStdVector(gains);
    QVector<double> yy = QVector<double>::fromStdVector(deltas);

    xx.push_back(predict);
    yy.push_back(gauss_ref_.gaussian_.center_);

    double xmin = std::numeric_limits<double>::max();
    double xmax = - std::numeric_limits<double>::max();

    for (auto &q : gains) {
      if (q < xmin)
        xmin = q;
      if (q > xmax)
        xmax = q;
    }

    double x_margin = (xmax - xmin) / 10;
    xmax += x_margin;
    xmin -= x_margin;

    ui->PlotCalib->clearGraphs();

    PL_DBG << "gain response will plot " << xx.size() << " points";

    ui->PlotCalib->addPoints(xx, yy, style_pts);

    std::set<double> chosen_peaks_chan;
    chosen_peaks_chan.insert(predict);
    ui->PlotCalib->set_selected_pts(chosen_peaks_chan);


    xx.clear();
    yy.clear();

    double step = (xmax-xmin) / 50.0;

    for (double i=xmin; i < xmax; i+=step) {
      xx.push_back(i);
      yy.push_back(p.evaluate(i));
    }
    PL_DBG << "gain response will plot " << xx.size() << " for curve";


    ui->PlotCalib->addFit(xx, yy, style_fit);
    ui->PlotCalib->setFloatingText("E = " + QString::fromStdString(p.to_UTF8(3, true)));

  }

  if (std::abs(delta) < ui->doubleThreshold->value()) {
    PL_INFO << "[Gain matching] gain matching complete";
    ui->pushStop->setEnabled(false);
    emit toggleIO(true);
    return;
  }

  gain_setting.value_dbl = new_gain;
  Qpx::Engine::getInstance().set_setting(gain_setting, Gamma::Match::id | Gamma::Match::indices);
  QThread::sleep(2);
  gain_setting = Qpx::Engine::getInstance().pull_settings().get_setting(gain_setting, Gamma::Match::id | Gamma::Match::indices);
  new_gain = gain_setting.value_dbl;


  PL_INFO << "[Gain matching] gain changed from " << std::fixed << std::setprecision(6)
          << old_gain << " to " << new_gain;

  QThread::sleep(2);
  do_run();
}

bool FormGainMatch::find_peaks() {
  gauss_ref_ = Gamma::Peak();
  fitter_ref_.set_mov_avg(ui->spinMovAvg->value());
  fitter_ref_.overlap_ = ui->doubleOverlapWidth->value();
  fitter_ref_.sum4edge_samples = ui->spinSum4EdgeW->value();
  fitter_ref_.find_peaks(ui->spinMinPeakWidth->value());
  if (fitter_ref_.peaks_.size()) {
    for (auto &q : fitter_ref_.peaks_) {
      double center = q.first;
      if ((q.second.gaussian_.height_ > gauss_ref_.height)
          && ((!moving_.visible)
              || ((center > a.pos.bin(a.pos.bits())) && (center < b.pos.bin(b.pos.bits())))
              )
          )
        gauss_ref_ = q.second;
    }
  }

  gauss_opt_ = Gamma::Peak();
  fitter_opt_.set_mov_avg(ui->spinMovAvg->value());
  fitter_ref_.overlap_ = ui->doubleOverlapWidth->value();
  fitter_ref_.sum4edge_samples = ui->spinSum4EdgeW->value();
  fitter_opt_.find_peaks(ui->spinMinPeakWidth->value());
  if (fitter_opt_.peaks_.size()) {
    for (auto &q : fitter_opt_.peaks_) {
      double center = q.first;
      if ((q.second.gaussian_.height_ > gauss_opt_.height)
          && ((!moving_.visible)
              || ((center > a.pos.bin(a.pos.bits())) && (center < b.pos.bin(b.pos.bits())))
              )
          )
        gauss_opt_ = q.second;
    }
  }

  return (gauss_ref_.gaussian_.height_ && gauss_opt_.gaussian_.height_);
}

void FormGainMatch::plot_one_spectrum(Gamma::Fitter &sp, std::map<double, double> &minima, std::map<double, double> &maxima) {

  for (int i=0; i < sp.y_.size(); ++i) {
    double yy = sp.y_[i];
    if (!minima.count(i) || (minima[i] > yy))
      minima[i] = yy;
    if (!maxima.count(i) || (maxima[i] < yy))
      maxima[i] = yy;
  }

  AppearanceProfile profile;
  profile.default_pen = QPen(QColor::fromRgba(sp.metadata_.appearance), 1);
  ui->plot->addGraph(QVector<double>::fromStdVector(sp.x_),
                     QVector<double>::fromStdVector(sp.y_),
                     profile, sp.metadata_.bits);
}


void FormGainMatch::update_plots() {

  bool have_data = false;
  bool have_peaks = false;

  for (auto &q: gm_spectra_.by_type("1D")) {
    Qpx::Spectrum::Metadata md;
    if (q)
      md = q->metadata();

    if (md.total_count > 0) {
      have_data = true;

      if (md.name == "Reference")
        fitter_ref_.setData(q);
      else
        fitter_opt_.setData(q);
    }
  }

  if (have_data)
    have_peaks = find_peaks();

  if (ui->plot->isVisible()) {
    this->setCursor(Qt::WaitCursor);

    std::map<double, double> minima, maxima;

    ui->plot->clearGraphs();

    plot_one_spectrum(fitter_ref_, minima, maxima);
    plot_one_spectrum(fitter_opt_, minima, maxima);

    ui->plot->setLabels("channel", "count");

    if (gauss_ref_.gaussian_.height_) {
      ui->plot->addGraph(QVector<double>::fromStdVector(gauss_ref_.x_), QVector<double>::fromStdVector(gauss_ref_.y_fullfit_gaussian_), ap_reference_);
      ui->plot->addGraph(QVector<double>::fromStdVector(gauss_ref_.x_), QVector<double>::fromStdVector(gauss_ref_.y_baseline_), ap_reference_);
    }

    if (gauss_opt_.gaussian_.height_) {
      ui->plot->addGraph(QVector<double>::fromStdVector(gauss_opt_.x_), QVector<double>::fromStdVector(gauss_opt_.y_fullfit_gaussian_), ap_optimized_);
      ui->plot->addGraph(QVector<double>::fromStdVector(gauss_opt_.x_), QVector<double>::fromStdVector(gauss_opt_.y_baseline_), ap_optimized_);
    }

    std::string new_label = boost::algorithm::trim_copy(gm_spectra_.status());
    ui->plot->setTitle(QString::fromStdString(new_label));

    ui->plot->setYBounds(minima, maxima);
    ui->plot->replot_markers();
    ui->plot->redraw();
  }


  if (have_peaks && (minTimer != nullptr) && (minTimer->s() > ui->timeDuration->total_seconds())) {
    delete minTimer;
    minTimer = nullptr;
    gm_interruptor_.store(true);
  }

  this->setCursor(Qt::ArrowCursor);
}

void FormGainMatch::addMovingMarker(double x) {
  moving_.pos.set_bin(x, fitter_ref_.metadata_.bits, fitter_ref_.nrg_cali_);
  moving_.visible = true;
  a.pos.set_bin(x - ui->spinPeakWindow->value() / 2, fitter_ref_.metadata_.bits, fitter_ref_.nrg_cali_);
  a.visible = true;
  b.pos.set_bin(x + ui->spinPeakWindow->value() / 2, fitter_ref_.metadata_.bits, fitter_ref_.nrg_cali_);
  b.visible = true;
  replot_markers();
}

void FormGainMatch::removeMovingMarker(double x) {
  moving_.visible = false;
  a.visible = false;
  b.visible = false;
  replot_markers();
}


void FormGainMatch::replot_markers() {
  std::list<Marker> markers;

  if (gauss_ref_.gaussian_.height_) {
    marker_ref_.pos.set_bin(gauss_ref_.gaussian_.center_, fitter_ref_.metadata_.bits, fitter_ref_.nrg_cali_);
    marker_ref_.visible = true;
    markers.push_back(marker_ref_);
  }

  if (gauss_opt_.gaussian_.height_) {
    marker_opt_.pos.set_bin(gauss_opt_.gaussian_.center_, fitter_opt_.metadata_.bits, fitter_opt_.nrg_cali_);
    marker_opt_.visible = true;
    markers.push_back(marker_opt_);
  }

  ui->plot->set_markers(markers);
  ui->plot->set_block(a,b);
  ui->plot->replot_markers();
  ui->plot->redraw();
}


void FormGainMatch::on_pushMatchGain_clicked()
{
  gains.clear();
  deltas.clear();

  current_setting_ = ui->comboSetting->currentText().toStdString();

  ui->PlotCalib->setFloatingText("");
  ui->PlotCalib->clearGraphs();

  current_pass = 0;
  max_passes = ui->spinMaxPasses->value();
  do_run();
}

void FormGainMatch::on_pushStop_clicked()
{
  PL_INFO << "Acquisition interrupted by user";
  max_passes = 0;
  gm_interruptor_.store(true);
}

void FormGainMatch::on_pushSaveOpti_clicked()
{
  Qpx::Engine::getInstance().save_optimization();
  std::vector<Gamma::Detector> dets = Qpx::Engine::getInstance().get_detectors();
  for (auto &q : dets) {
    if (detectors_.has_a(q)) {
      Gamma::Detector modified = detectors_.get(q);
      modified.settings_ = q.settings_;
      detectors_.replace(modified);
    }
  }
  emit optimization_approved();
}

void FormGainMatch::on_pushFindPeaks_clicked()
{
  bool have_peaks = find_peaks();
  if (have_peaks && (minTimer != nullptr) && (minTimer->s() > ui->timeDuration->total_seconds())) {
    delete minTimer;
    minTimer = nullptr;
    gm_interruptor_.store(true);
  }
}

void FormGainMatch::on_comboTarget_currentIndexChanged(int index)
{
  int optchan = ui->comboTarget->currentData().toInt();

  std::vector<Gamma::Detector> chans = Qpx::Engine::getInstance().get_detectors();

  ui->comboSetting->clear();
  if (optchan < chans.size()) {
    for (auto &q : chans[optchan].settings_.branches.my_data_) {
      if (q.metadata.flags.count("gain") > 0) {
        QString text = QString::fromStdString(q.id_);
        ui->comboSetting->addItem(text);
      }
    }
  }

}
