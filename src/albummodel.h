/*
 * Copyright 2015-2016 Matthieu Gallien <matthieu_gallien@yahoo.fr>
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

#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QVector>
#include <QtCore/QHash>
#include <QtCore/QString>

#include "musicalbum.h"
#include "musicaudiotrack.h"

class DatabaseInterface;
class AlbumModelPrivate;
class MusicStatistics;
class QMutex;

class AlbumModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(DatabaseInterface* databaseInterface
               READ databaseInterface
               WRITE setDatabaseInterface
               NOTIFY databaseInterfaceChanged)

    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)

    Q_PROPERTY(QString author
               READ author
               WRITE setAuthor
               NOTIFY authorChanged)

public:

    enum ItemClass {
        Container = 0,
        Album = 1,
        Artist = 2,
        AudioTrack = 3,
    };

    enum ColumnsRoles {
        TitleRole = Qt::UserRole + 1,
        DurationRole = TitleRole + 1,
        MilliSecondsDurationRole = DurationRole + 1,
        CreatorRole = MilliSecondsDurationRole + 1,
        ArtistRole = CreatorRole + 1,
        AlbumRole = ArtistRole + 1,
        TrackNumberRole = AlbumRole + 1,
        RatingRole = TrackNumberRole + 1,
        ImageRole = RatingRole + 1,
        ResourceRole = ImageRole + 1,
        ItemClassRole = ResourceRole + 1,
        CountRole = ItemClassRole + 1,
        IdRole = CountRole + 1,
        IsPlayingRole = IdRole + 1,
        DatabaseIdRole = IsPlayingRole + 1,
    };

    explicit AlbumModel(QObject *parent = 0);

    virtual ~AlbumModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    DatabaseInterface* databaseInterface() const;

    QString title() const;

    QString author() const;

Q_SIGNALS:

    void databaseInterfaceChanged();

    void newAlbum(const MusicAlbum &album);

    void newAudioTrack(const MusicAudioTrack &audioTrack);

    void refreshContent();

    void titleChanged();

    void authorChanged();

public Q_SLOTS:

    void setDatabaseInterface(DatabaseInterface* databaseInterface);

    void albumsList(const QVector<MusicAlbum> &allAlbums);

    void tracksList(QHash<QString, QVector<MusicAudioTrack> > tracks, QHash<QString, QUrl> covers);

    void setTitle(QString title);

    void setAuthor(QString author);

private Q_SLOTS:

    void beginTrackAdded(QVector<qlonglong> newTracks);

    void endTrackAdded(QVector<qlonglong> newTracks);

private:

    QVariant internalDataTrack(const MusicAudioTrack &track, int role) const;

    AlbumModelPrivate *d;

};

Q_DECLARE_METATYPE(AlbumModel::ItemClass)

#endif // ALBUMMODEL_H
