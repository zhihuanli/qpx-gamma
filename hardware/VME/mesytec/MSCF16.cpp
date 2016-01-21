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
 *      MSCF16
 *
 ******************************************************************************/

#include "MSCF16.h"

namespace Qpx {

static DeviceRegistrar<MSCF16> registrar("VME/MesytecRC/MSCF16");

MSCF16::MSCF16() {
  controller_ = nullptr;
  modnum_ = 0;
  status_ = DeviceStatus::loaded | DeviceStatus::can_boot;
  module_ID_code_ = -1;
}


MSCF16::~MSCF16() {
  die();
}

}
