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
#ifndef KDSOAPNAMESPACEMANAGER_H
#define KDSOAPNAMESPACEMANAGER_H

#include "KDSoapGlobal.h"
#include "KDSoapClientInterface.h"
#include <QtCore/QString>

class KDSoapClientInterface;

/**
 * Repository of namespaces
 */
class KDSOAP_EXPORT KDSoapNamespaceManager
{
public:
    static QString xmlSchema1999();
    static QString xmlSchema2001();
    static QString xmlSchemaInstance1999();
    static QString xmlSchemaInstance2001();
    static QString soapEnvelope(const SoapVersion soapVersion);
    static QString soapEncoding(const SoapVersion soapVersion);
    static QString soapEnvelope200305();
    static QString soapEncoding200305();

private: // TODO instanciate to handle custom namespaces per clientinterface
    KDSoapNamespaceManager();
};

#endif // KDSOAPNAMESPACEMANAGER_H
