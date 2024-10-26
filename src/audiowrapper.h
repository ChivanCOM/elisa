/*
   SPDX-FileCopyrightText: 2017 (c) Matthieu Gallien <matthieu_gallien@yahoo.fr>

   SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef AUDIOWRAPPER_H
#define AUDIOWRAPPER_H

#include "elisaLib_export.h"

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QMediaPlayer>
#include <QString>

#include <memory>
#include <models/renderermodel.h>

class AudioWrapperPrivate;

class ELISALIB_EXPORT AudioWrapper : public QObject
{
    Q_OBJECT

    QML_ELEMENT

    Q_PROPERTY(bool muted
                   READ muted
                       WRITE setMuted
                           NOTIFY mutedChanged)

    Q_PROPERTY(qreal volume
                   READ volume
                       WRITE setVolume
                           NOTIFY volumeChanged)

    Q_PROPERTY(QUrl source
                   READ source
                       WRITE setSource
                           NOTIFY sourceChanged)

    Q_PROPERTY(QMediaPlayer::MediaStatus status
                   READ status
                       NOTIFY statusChanged)

    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState
                   READ playbackState
                       NOTIFY playbackStateChanged)

    Q_PROPERTY(QMediaPlayer::Error error
                   READ error
                       NOTIFY errorChanged)

    Q_PROPERTY(qint64 duration
                   READ duration
                       NOTIFY durationChanged)

    Q_PROPERTY(qint64 position
                   READ position
                       WRITE setPosition
                           NOTIFY positionChanged)

    Q_PROPERTY(bool seekable
                   READ seekable
                       NOTIFY seekableChanged)

    Q_PROPERTY(RendererModel *renderersModel
                   READ renderersModel
                       NOTIFY renderersChanged)

    Q_PROPERTY(bool renderersSupported
                   READ renderersSupported)

public:

    explicit AudioWrapper(QObject *parent = nullptr);

    ~AudioWrapper() override;

    [[nodiscard]] bool muted() const;

    [[nodiscard]] qreal volume() const;

    [[nodiscard]] QUrl source() const;

    [[nodiscard]] QMediaPlayer::MediaStatus status() const;

    [[nodiscard]] QMediaPlayer::PlaybackState playbackState() const;

    [[nodiscard]] QMediaPlayer::Error error() const;

    [[nodiscard]] qint64 duration() const;

    [[nodiscard]] qint64 position() const;

    [[nodiscard]] bool seekable() const;

    [[nodiscard]] bool renderersSupported() const;

Q_SIGNALS:

    void mutedChanged(bool muted);

    void volumeChanged();

    void sourceChanged();

    void statusChanged(QMediaPlayer::MediaStatus status);

    void playbackStateChanged(QMediaPlayer::PlaybackState state);

    void errorChanged(QMediaPlayer::Error error);

    void durationChanged(qint64 duration);

    void positionChanged(qint64 position);

    void currentPlayingForRadiosChanged(const QString &title, const QString &nowPlaying);

    void seekableChanged(bool seekable);

    void playing();

    void paused();

    void stopped();

    void renderersChanged();

public Q_SLOTS:

    void setMuted(bool muted);

    void setVolume(qreal volume);

    void setSource(const QUrl &source);

    void setPosition(qint64 position);

    void saveUndoPosition(qint64 position);

    void restoreUndoPosition();

    void play();

    void pause();

    void stop();

    void seek(qint64 position);

    void setRenderer(const QString name, const QString type);

private Q_SLOTS:

    void mediaStatusChanged();

    void playerStateChanged();

    void playerMutedChanged();

    void playerVolumeChanged();

private:
    void savePosition(qint64 position);

    void playerStateSignalChanges(QMediaPlayer::PlaybackState newState);

    void mediaStatusSignalChanges(QMediaPlayer::MediaStatus newStatus);

    void playerErrorSignalChanges(QMediaPlayer::Error error);

    void playerDurationSignalChanges(qint64 newDuration);

    void playerPositionSignalChanges(qint64 newPosition);

    void playerVolumeSignalChanges();

    void playerMutedSignalChanges(bool isMuted);

    void playerSeekableSignalChanges(bool isSeekable);

    void queueStatusChanged();

    void notifyStatusChanges();

    friend class AudioWrapperPrivate;

    std::unique_ptr<AudioWrapperPrivate> d;

public:
    RendererModel *renderersModel();

    void startRendererDiscovery();

    void renderersChanges();
    
};

#endif // AUDIOWRAPPER_H
