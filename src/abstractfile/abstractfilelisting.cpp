/*
   SPDX-FileCopyrightText: 2016 (c) Matthieu Gallien <matthieu_gallien@yahoo.fr>

   SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "abstractfilelisting.h"

#include "config-upnp-qt.h"

#include "abstractfile/indexercommon.h"

#include "filescanner.h"
#include "elisa_settings.h"

#include <QThread>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QSet>
#include <QAtomicInt>


#include <algorithm>
#include <utility>

struct FileSystemPath {
    QUrl path;
    bool isFile;
};

inline bool operator==(const FileSystemPath &path1, const FileSystemPath &path2)
{
    return path1.path == path2.path && path1.isFile == path2.isFile;
}

inline size_t qHash(const FileSystemPath &fileSystemPath, size_t seed = 0)
{
    return qHash(fileSystemPath.path, seed);
}

class AbstractFileListingPrivate
{
public:

    QStringList mAllRootPaths;

    QFileSystemWatcher mFileSystemWatcher;

    QHash<QString, QUrl> mAllAlbumCover;

    QHash<QUrl, QSet<FileSystemPath>> mDiscoveredDirectories;

    FileScanner mFileScanner;

    QHash<QUrl, QDateTime> mAllFiles;

    QAtomicInt mStopRequest = 0;

    int mImportedTracksCount = 0;

    int mNewFilesEmitInterval = 1;

    bool mHandleNewFiles = true;

    bool mWaitEndTrackRemoval = false;

    bool mErrorWatchingFileSystemChanges = false;

    bool mIsActive = false;

};

AbstractFileListing::AbstractFileListing(QObject *parent) : QObject(parent), d(std::make_unique<AbstractFileListingPrivate>())
{
    connect(&d->mFileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &AbstractFileListing::directoryChanged);
    connect(&d->mFileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &AbstractFileListing::fileChanged);
}

AbstractFileListing::~AbstractFileListing()
= default;

void AbstractFileListing::init()
{
    qCDebug(orgKdeElisaIndexer()) << "AbstractFileListing::init";
    d->mIsActive = true;

    const bool autoScan = Elisa::ElisaConfiguration::self()->scanAtStartup();
    if (autoScan) {
        Q_EMIT askRestoredTracks();
    }
}

void AbstractFileListing::stop()
{
    d->mIsActive = false;

    triggerStop();
}

void AbstractFileListing::newTrackFile(const DataTypes::TrackDataType &partialTrack)
{
    auto scanFileInfo = QFileInfo(partialTrack.resourceURI().toLocalFile());
    const auto &newTrack = scanOneFile(partialTrack.resourceURI(), scanFileInfo, WatchChangedDirectories | WatchChangedFiles);

    if (newTrack.isValid() && newTrack != partialTrack) {
        Q_EMIT modifyTracksList({newTrack}, d->mAllAlbumCover);
    }
}

void AbstractFileListing::restoredTracks(QHash<QUrl, QDateTime> allFiles)
{
    executeInit(std::move(allFiles));

    refreshContent();
}

void AbstractFileListing::setAllRootPaths(const QStringList &allRootPaths)
{
    d->mAllRootPaths = allRootPaths;
}

void AbstractFileListing::databaseFinishedInsertingTracksList()
{
}

void AbstractFileListing::databaseFinishedRemovingTracksList()
{
    if (waitEndTrackRemoval()) {
        Q_EMIT indexingFinished();
        setWaitEndTrackRemoval(false);
    }
}

void AbstractFileListing::applicationAboutToQuit()
{
    d->mStopRequest = 1;
}

const QStringList &AbstractFileListing::allRootPaths() const
{
    return d->mAllRootPaths;
}

bool AbstractFileListing::canHandleRootPaths() const
{
    return true;
}

void AbstractFileListing::scanDirectory(DataTypes::ListTrackDataType &newFiles, const QUrl &path, FileSystemWatchingModes watchForFileSystemChanges)
{
    if (d->mStopRequest == 1) {
        return;
    }

    QDir rootDirectory(path.toLocalFile());
    rootDirectory.refresh();

    if (rootDirectory.exists()) {
        if (watchForFileSystemChanges & WatchChangedDirectories) {
            watchPath(path.toLocalFile());
        }
    }

    auto currentDirectoryListingFiles = d->mDiscoveredDirectories[path];

    auto currentFilesList = QSet<QUrl>();

    rootDirectory.refresh();
    const auto entryList = rootDirectory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (const auto &oneEntry : entryList) {
        auto newFilePath = QUrl::fromLocalFile(oneEntry.canonicalFilePath());

        if (oneEntry.isDir() || oneEntry.isFile()) {
            currentFilesList.insert(newFilePath);
        }
    }

    auto allRemovedTracks = QList<QUrl>();
    for (const auto &removedFilePath : currentDirectoryListingFiles) {
        if (currentFilesList.contains(removedFilePath.path)) {
            continue;
        }

        if (removedFilePath.isFile) {
            allRemovedTracks.push_back(removedFilePath.path);
        } else {
            removeFile(removedFilePath.path, allRemovedTracks);
        }

        currentDirectoryListingFiles.remove(removedFilePath);
    }

    if (!allRemovedTracks.isEmpty()) {
        Q_EMIT removedTracksList(allRemovedTracks);
    }

    if (!d->mHandleNewFiles) {
        return;
    }

    for (const auto &newFilePath : std::as_const(currentFilesList)) {
        QFileInfo oneEntry(newFilePath.toLocalFile());

        if (oneEntry.isDir()) {
            addFileInDirectory(newFilePath, path, WatchChangedDirectories | WatchChangedFiles);
            scanDirectory(newFiles, newFilePath, WatchChangedDirectories | WatchChangedFiles);

            if (d->mStopRequest == 1) {
                break;
            }

            continue;
        }
        if (!oneEntry.isFile()) {
            continue;
        }

        auto itExistingFile = allFiles().constFind(newFilePath);
        if (itExistingFile != allFiles().cend()) {
            if (*itExistingFile >= oneEntry.metadataChangeTime()) {
                allFiles().erase(itExistingFile);
                qCDebug(orgKdeElisaIndexer()) << "AbstractFileListing::scanDirectory" << newFilePath << "file not modified since last scan";
                continue;
            }
        }

        auto newTrack = scanOneFile(newFilePath, oneEntry, WatchChangedDirectories | WatchChangedFiles);

        if (newTrack.isValid() && d->mStopRequest == 0) {
            addCover(newTrack);

            addFileInDirectory(newTrack.resourceURI(), path, WatchChangedDirectories | WatchChangedFiles);
            newFiles.push_back(newTrack);

            ++d->mImportedTracksCount;

            if (newFiles.size() > d->mNewFilesEmitInterval && d->mStopRequest == 0) {
                d->mNewFilesEmitInterval = std::min(50, 1 + d->mNewFilesEmitInterval * d->mNewFilesEmitInterval);
                emitNewFiles(newFiles);
                newFiles.clear();
            }
        } else {
            qCDebug(orgKdeElisaIndexer()) << "AbstractFileListing::scanDirectory" << newFilePath << "is not a valid track";
        }

        if (d->mStopRequest == 1) {
            break;
        }
    }
}

void AbstractFileListing::directoryChanged(const QString &path)
{
    if (!d->mDiscoveredDirectories.contains(QUrl::fromLocalFile(path))) {
        return;
    }

    Q_EMIT indexingStarted();

    scanDirectoryTree(path);

    Q_EMIT indexingFinished();
}

void AbstractFileListing::fileChanged(const QString &modifiedFileName)
{
    QFileInfo modifiedFileInfo(modifiedFileName);
    auto modifiedFile = QUrl::fromLocalFile(modifiedFileName);

    auto modifiedTrack = scanOneFile(modifiedFile, modifiedFileInfo, WatchChangedDirectories | WatchChangedFiles);

    if (modifiedTrack.isValid()) {
        Q_EMIT modifyTracksList({modifiedTrack}, d->mAllAlbumCover);
    }
}

void AbstractFileListing::executeInit(QHash<QUrl, QDateTime> allFiles)
{
    d->mAllFiles = std::move(allFiles);
}

void AbstractFileListing::triggerStop()
{
}

void AbstractFileListing::triggerRefreshOfContent()
{
    d->mImportedTracksCount = 0;
}

void AbstractFileListing::refreshContent()
{
    triggerRefreshOfContent();
}

DataTypes::TrackDataType AbstractFileListing::scanOneFile(const QUrl &scanFile, const QFileInfo &scanFileInfo, FileSystemWatchingModes watchForFileSystemChanges)
{
    DataTypes::TrackDataType newTrack;

    qCDebug(orgKdeElisaIndexer) << "AbstractFileListing::scanOneFile" << scanFile;

    auto localFileName = scanFile.toLocalFile();

    if (!d->mFileScanner.shouldScanFile(localFileName)) {
        qCDebug(orgKdeElisaIndexer) << "AbstractFileListing::scanOneFile" << "invalid mime type";
        return newTrack;
    }

    if (scanFileInfo.exists()) {
        auto itExistingFile = d->mAllFiles.constFind(scanFile);
        if (itExistingFile != d->mAllFiles.cend()) {
            if (*itExistingFile >= scanFileInfo.metadataChangeTime()) {
                d->mAllFiles.erase(itExistingFile);
                qCDebug(orgKdeElisaIndexer) << "AbstractFileListing::scanOneFile" << "not changed file";
                return newTrack;
            }
        }
    }

    newTrack = d->mFileScanner.scanOneFile(scanFile, scanFileInfo);

    if (newTrack.isValid() && scanFileInfo.exists()) {
        if (watchForFileSystemChanges & WatchChangedFiles) {
            watchPath(scanFile.toLocalFile());
        }
    }

    return newTrack;
}

void AbstractFileListing::watchPath(const QString &pathName)
{
    if (!d->mFileSystemWatcher.addPath(pathName)) {
        qCDebug(orgKdeElisaIndexer) << "AbstractFileListing::watchPath" << "fail for" << pathName;

        if (!d->mErrorWatchingFileSystemChanges) {
            d->mErrorWatchingFileSystemChanges = true;
            Q_EMIT errorWatchingFileSystemChanges();
        }
    }
}

void AbstractFileListing::addFileInDirectory(const QUrl &newFile, const QUrl &directoryName, FileSystemWatchingModes watchForFileSystemChanges)
{
    if (!d->mDiscoveredDirectories.contains(directoryName)) {
        if (watchForFileSystemChanges & WatchChangedDirectories) {
            watchPath(directoryName.toLocalFile());
        }

        QDir currentDirectory(directoryName.toLocalFile());
        if (currentDirectory.cdUp()) {
            const auto parentDirectoryName = currentDirectory.absolutePath();
            const auto parentDirectory = QUrl::fromLocalFile(parentDirectoryName);
            if (!d->mDiscoveredDirectories.contains(parentDirectory)) {
                if (watchForFileSystemChanges & WatchChangedDirectories) {
                    watchPath(parentDirectoryName);
                }
            }

            auto &parentCurrentDirectoryListingFiles = d->mDiscoveredDirectories[parentDirectory];

            parentCurrentDirectoryListingFiles.insert({directoryName, false});
        }
    }
    auto &currentDirectoryListingFiles = d->mDiscoveredDirectories[directoryName];

    QFileInfo isAFile(newFile.toLocalFile());
    currentDirectoryListingFiles.insert({newFile, isAFile.isFile()});
}

void AbstractFileListing::scanDirectoryTree(const QString &path)
{
    auto newFiles = DataTypes::ListTrackDataType();

    qCDebug(orgKdeElisaIndexer()) << "AbstractFileListing::scanDirectoryTree" << path;

    scanDirectory(newFiles, QUrl::fromLocalFile(path), WatchChangedDirectories | WatchChangedFiles);

    if (!newFiles.isEmpty() && d->mStopRequest == 0) {
        emitNewFiles(newFiles);
    }
}

void AbstractFileListing::setHandleNewFiles(bool handleThem)
{
    d->mHandleNewFiles = handleThem;
}

void AbstractFileListing::emitNewFiles(const DataTypes::ListTrackDataType &tracks)
{
    Q_EMIT tracksList(tracks, d->mAllAlbumCover);
}

void AbstractFileListing::addCover(const DataTypes::TrackDataType &newTrack)
{
    if (d->mAllAlbumCover.contains(newTrack.album())) {
        return;
    }

    auto coverUrl = d->mFileScanner.searchForCoverFile(newTrack.resourceURI().toLocalFile());
    if (!coverUrl.isEmpty()) {
        d->mAllAlbumCover[newTrack.resourceURI().toString()] = coverUrl;
    }
}

void AbstractFileListing::removeDirectory(const QUrl &removedDirectory, QList<QUrl> &allRemovedFiles)
{
    const auto itRemovedDirectory = d->mDiscoveredDirectories.constFind(removedDirectory);

    if (itRemovedDirectory == d->mDiscoveredDirectories.cend()) {
        return;
    }

    const auto &currentRemovedDirectory = *itRemovedDirectory;
    for (const auto &itFile : currentRemovedDirectory) {
        if (itFile.path.isValid() && !itFile.path.isEmpty()) {
            removeFile(itFile.path, allRemovedFiles);
            if (itFile.isFile) {
                allRemovedFiles.push_back(itFile.path);
            }
        }
    }

    d->mDiscoveredDirectories.erase(itRemovedDirectory);
}

void AbstractFileListing::removeFile(const QUrl &oneRemovedTrack, QList<QUrl> &allRemovedFiles)
{
    if (d->mDiscoveredDirectories.contains(oneRemovedTrack)) {
        removeDirectory(oneRemovedTrack, allRemovedFiles);
    }
}

QHash<QUrl, QDateTime> &AbstractFileListing::allFiles()
{
    return d->mAllFiles;
}

void AbstractFileListing::checkFilesToRemove()
{
    qCDebug(orgKdeElisaIndexer()) << "AbstractFileListing::checkFilesToRemove" << d->mAllFiles.size();

    if (!d->mAllFiles.isEmpty()) {
        setWaitEndTrackRemoval(true);
        Q_EMIT removedTracksList(d->mAllFiles.keys());
    }
}

FileScanner &AbstractFileListing::fileScanner()
{
    return d->mFileScanner;
}

bool AbstractFileListing::waitEndTrackRemoval() const
{
    return d->mWaitEndTrackRemoval;
}

void AbstractFileListing::setWaitEndTrackRemoval(bool wait)
{
    d->mWaitEndTrackRemoval = wait;
}

bool AbstractFileListing::isActive() const
{
    return d->mIsActive;
}


#include "moc_abstractfilelisting.cpp"
