/*
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include "fileprovider.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QTemporaryFile>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

FileProvider::FileProvider()
  : QObject( 0 )
{
}

void FileProvider::cleanUp()
{
  if (!mFileName.isEmpty()) {
    QFile::remove( mFileName );
    mFileName = QString();
  }
}

bool FileProvider::get( const QUrl &url, QString &target, bool ignoreSslErrors )
{
  if ( !mFileName.isEmpty() ) {
    cleanUp();
  }

  if (url.scheme() == QLatin1String("file")) {
      target = url.toLocalFile();
      return true;
  }
  if (url.scheme() == QLatin1String("qrc")) {
      target = QLatin1String(":") + url.path();
      return true;
  }

  if ( target.isEmpty() ) {
    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    tmpFile.open();
    target = tmpFile.fileName();
    mFileName = target;
  }

  mIgnoreSslErrors = ignoreSslErrors;

  qDebug("Downloading '%s'", url.toEncoded().constData());

  QNetworkAccessManager manager;
  QNetworkRequest request(url);
  job = manager.get(request);

  QEventLoop loop;
  connect(job, SIGNAL(finished()),
          &loop, SLOT(quit()));
  connect(job, SIGNAL(sslErrors(QList<QSslError>)),
          this, SLOT(sslErrors(QList<QSslError>)));
  loop.exec();

  if (job->error()) {
      qWarning("Error downloading '%s': %s", url.toEncoded().constData(), qPrintable(job->errorString()));
      return false;
  }

  const QByteArray data = job->readAll();
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly ) ) {
      qDebug( "Unable to create temporary file" );
      return false;
  }

  qDebug( "Download successful" );
  file.write( data );
  file.close();

  return true;
}

void FileProvider::sslErrors( QList<QSslError> sslErrors )
{
  if( mIgnoreSslErrors ) {
    job->ignoreSslErrors();
  }
}

#include "moc_fileprovider.cpp"
