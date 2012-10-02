/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#include "KDSoapNamespaceManager.h"

#include "KDSoapClientInterface.h"

KDSoapNamespaceManager::KDSoapNamespaceManager()
{
}

QString KDSoapNamespaceManager::xmlSchema1999()
{
    static QString s = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
    return s;
}

QString KDSoapNamespaceManager::xmlSchema2001()
{
    static QString s = QString::fromLatin1("http://www.w3.org/2001/XMLSchema");
    return s;
}

QString KDSoapNamespaceManager::xmlSchemaInstance1999()
{
    static QString s = QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
    return s;
}

QString KDSoapNamespaceManager::xmlSchemaInstance2001()
{
    static QString s = QString::fromLatin1("http://www.w3.org/2001/XMLSchema-instance");
    return s;
}

QString KDSoapNamespaceManager::soapEnvelope(const SoapVersion soapVersion)
{
    QString s;
    if(soapVersion == SOAP1_2 || SOAP1_2_XOP) {
        s = QString::fromLatin1("http://www.w3.org/2003/05/soap-envelope");
    } else {
        s = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
    }
    return s;
}

QString KDSoapNamespaceManager::soapEncoding(const SoapVersion soapVersion)
{
    QString s;
    if(soapVersion == SOAP1_2 || SOAP1_2_XOP) {
        s = QString::fromLatin1("http://www.w3.org/2003/05/soap-encoding");
    } else {
        s = QString::fromLatin1("http://schemas.xmlsoap.org/soap/encoding/");
    }
    return s;
}
