/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>      *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/
#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include "qkeychain_export.h"

#include <QtCore/QObject>
#include <QtCore/QString>

class QSettings;

#define QTKEYCHAIN_VERSION 0x000100

namespace QKeychain {

/**
 * Error codes
 */
enum Error {
    NoError=0, /**< No error occurred, operation was successful */
    EntryNotFound, /**< For the given key no data was found */
    CouldNotDeleteEntry, /**< Could not delete existing secret data */
    AccessDeniedByUser, /**< User denied access to keychain */
    AccessDenied, /**< Access denied for other reasons */
    NoBackendAvailable, /**< No platform-specific keychain service available */
    NotImplemented, /**< Not implemented on platform */
    OtherError /**< Something else went wrong (errorString() might provide details) */
};

class JobExecutor;
class JobPrivate;

/**
 * @brief Abstract base class for all QKeychain jobs.
 */
class QKEYCHAIN_EXPORT Job : public QObject {
    Q_OBJECT
public:    
    ~Job();

    QSettings* settings() const;
    void setSettings( QSettings* settings );

    /**
     * Call this method to start the job.
     * Tipically you want to connect some slot to the finished() signal first.
     * You can run the job either synchronously or asynchronously.
     *
     * In the first case you tipically use an inner event loop:
     *
     * \code
     * SomeClass::startJob()
     * {
     *     QEventLoop eventLoop;
     *     connect(job, &Job::finished, &eventLoop, &QEventLoop::quit);
     *     job->start();
     *     eventLoop.exec();
     *
     *     if (job->error() {
     *         // handle error
     *     } else {
     *         // do job-specific stuff
     *     }
     * }
     * \endcode
     *
     * In the asynchronous case you just connect some slot to the finished() signal
     * and you will handle the job's completion there:
     *
     * \code
     * SomeClass::startJob()
     * {
     *     connect(job, &Job::finished, this, &SomeClass::slotJobFinished);
     *     job->start();
     * }
     *
     * SomeClass::slotJobFinished(Job *job)
     * {
     *     if (job->error() {
     *         // handle error
     *     } else {
     *         // do job-specific stuff
     *     }
     * }
     * \endcode
     *
     * @see finished()
     */
    void start();

    QString service() const;

    /**
     * @note Call this method only after finished() has been emitted.
     * @return The error code of the job (0 if no error).
     */
    Error error() const;

    /**
     * @return An error message that might provide details if error() returns OtherError.
     */
    QString errorString() const;

    /**
     * @return Whether this job autodeletes itself once finished() has been emitted. Default is true.
     * @see setAutoDelete()
     */
    bool autoDelete() const;

    /**
     * Set whether this job should autodelete itself once finished() has been emitted.
     * @see autoDelete()
     */
    void setAutoDelete( bool autoDelete );

    /**
     * @return Whether this job will use plaintext storage on unsupported platforms. Default is false.
     * @see setInsecureFallback()
     */
    bool insecureFallback() const;

    /**
     * Set whether this job should use plaintext storage on unsupported platforms.
     * @see insecureFallback()
     */
    void setInsecureFallback( bool insecureFallback );

    /**
     * @return The string used as key by this job.
     * @see setKey()
     */
    QString key() const;

    /**
     * Set the @p key that this job will use to read or write data from/to the keychain.
     * The key can be an empty string.
     * @see key()
     */
    void setKey( const QString& key );

Q_SIGNALS:
    /**
     * Emitted when this job is finished.
     * You can connect to this signal to be notified about the job's completion.
     * @see start()
     */
    void finished( QKeychain::Job* );

protected:
    explicit Job( JobPrivate *q, QObject* parent=0 );
    Q_INVOKABLE void doStart();

private:
    void setError( Error error );
    void setErrorString( const QString& errorString );
    void emitFinished();
    void emitFinishedWithError(Error, const QString& errorString);

    void scheduledStart();

protected:
    JobPrivate* const d;

friend class JobExecutor;
friend class JobPrivate;
friend class ReadPasswordJobPrivate;
friend class WritePasswordJobPrivate;
friend class DeletePasswordJobPrivate;
};

class ReadPasswordJobPrivate;

class QKEYCHAIN_EXPORT ReadPasswordJob : public Job {
    Q_OBJECT
public:
    explicit ReadPasswordJob( const QString& service, QObject* parent=0 );
    ~ReadPasswordJob();

    QByteArray binaryData() const;
    QString textData() const;

private:
    friend class QKeychain::ReadPasswordJobPrivate;
};

class WritePasswordJobPrivate;

class QKEYCHAIN_EXPORT WritePasswordJob : public Job {
    Q_OBJECT
public:
    explicit WritePasswordJob( const QString& service, QObject* parent=0 );
    ~WritePasswordJob();

    void setBinaryData( const QByteArray& data );
    void setTextData( const QString& data );

private:

    friend class QKeychain::WritePasswordJobPrivate;
};

class DeletePasswordJobPrivate;

class QKEYCHAIN_EXPORT DeletePasswordJob : public Job {
    Q_OBJECT
public:
    explicit DeletePasswordJob( const QString& service, QObject* parent=0 );
    ~DeletePasswordJob();

private:
    friend class QKeychain::DeletePasswordJobPrivate;
};

} // namespace QtKeychain

#endif
