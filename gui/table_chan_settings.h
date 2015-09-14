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
 *      TableChanSettings - table for displaying and manipulating Pixie's
 *      channel settings and chosing detectors.
 *
 ******************************************************************************/


#ifndef TABLE_CHAN_SETTINGS_H_
#define TABLE_CHAN_SETTINGS_H_

#include <QAbstractTableModel>
#include <QFont>
#include <QBrush>
#include "wrapper.h"

class TableChanSettings : public QAbstractTableModel
{
    Q_OBJECT
private:
    Pixie::Settings &my_settings_;
    std::vector<Gamma::Setting> chan_meta_;

public:
    TableChanSettings(QObject *parent = 0): my_settings_(Pixie::Wrapper::getInstance().settings()) {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    void update();

signals:
    void detectors_changed();

public slots:

};

#endif
