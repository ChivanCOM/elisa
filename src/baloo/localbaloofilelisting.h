/*
 * Copyright 2016 Matthieu Gallien <matthieu_gallien@yahoo.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef LOCALBALOOFILELISTING_H
#define LOCALBALOOFILELISTING_H

#include "../abstractfile/abstractfilelisting.h"

#include <QObject>
#include <QString>
#include <QUrl>
#include <QHash>
#include <QVector>

#include <memory>

class LocalBalooFileListingPrivate;
class MusicAudioTrack;

class LocalBalooFileListing : public AbstractFileListing
{

    Q_OBJECT

public:

    explicit LocalBalooFileListing(QObject *parent = 0);

    virtual ~LocalBalooFileListing();

Q_SIGNALS:

public Q_SLOTS:

private Q_SLOTS:

private:

    void triggerRefreshOfContent() override;

    MusicAudioTrack scanOneFile(QUrl scanFile) override;

    std::unique_ptr<LocalBalooFileListingPrivate> d;

};

#endif // LOCALBALOOFILELISTING_H
