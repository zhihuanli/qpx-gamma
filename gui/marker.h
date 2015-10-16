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
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      Marker - for marking a channel or energt in a plot
 *
 ******************************************************************************/

#ifndef Q_MARKER_H
#define Q_MARKER_H

#include "detector.h"
#include <QPen>
#include <QMap>

struct AppearanceProfile {
  QMap<QString, QPen> themes;
  QPen default_pen;

  AppearanceProfile() : default_pen(Qt::gray, 1, Qt::SolidLine) {}
  QPen get_pen(QString theme) {
    if (themes.count(theme))
      return themes[theme];
    else
      return default_pen;
  }
};

struct Marker {
  bool operator!= (const Marker& other) const { return (!operator==(other)); }
  bool operator== (const Marker& other) const {
    if (energy != other.energy) return false;
    if (channel != other.channel) return false;
    return true;
  }

  double energy, channel;
  uint16_t bits;

  AppearanceProfile appearance, selected_appearance;

  bool visible;
  bool selected;
  bool chan_valid, energy_valid;

  Marker() : energy(0), channel(0), bits(0), visible(false), chan_valid(false), energy_valid(false) {}

  void shift(uint16_t to_bits) {
    if (!to_bits || !bits)
      return;

    if (bits > to_bits)
      channel = channel / pow(2, bits - to_bits);
    if (bits < to_bits)
      channel = channel * pow(2, to_bits - bits);

    bits = to_bits;
  }

  void calibrate(Gamma::Detector det) {
    if (chan_valid && det.energy_calibrations_.has_a(Gamma::Calibration("Energy", bits))) {
      energy = det.energy_calibrations_.get(Gamma::Calibration("Energy", bits)).transform(channel);
      energy_valid = true;
    } // else highest?
    if (!chan_valid && energy_valid && det.energy_calibrations_.has_a(Gamma::Calibration("Energy", bits))) {
      Gamma::Calibration cal = det.energy_calibrations_.get(Gamma::Calibration("Energy", bits));
      channel = 0;
      while (cal.transform(channel) < energy)
        ++channel;
    } // else highest?
  }

};

struct Range {
  bool visible;
  AppearanceProfile base, top;
  Marker center, l, r;
};

struct MarkerBox2D {
  MarkerBox2D() : visible(false), selected(false) {}
  bool operator== (const MarkerBox2D& other) const {
    if (x1 != other.x1) return false;
    if (x2 != other.x2) return false;
    if (y1 != other.y1) return false;
    if (y2 != other.y2) return false;
    if (x_c != other.x_c) return false;
    if (y_c != other.y_c) return false;
    return true;
  }

  bool visible;
  bool selected;
  Marker x1, x2, y1, y2;
  double x_c, y_c;
};

#endif
