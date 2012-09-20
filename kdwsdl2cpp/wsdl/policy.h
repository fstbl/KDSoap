/*
    This file is part of KD Soap.

    Copyright (c) 2012 Front-safe A/S <fstbl@front-safe.dk>

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

#ifndef KWSDL_POLICY_H
#define KWSDL_POLICY_H

#include <qdom.h>

#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Policy : public Element
{
  public:
    typedef QList<Policy> List;
  
    Policy();
    Policy( const QString &nameSpace );
    ~Policy();

    void setUsernameToken( const bool &b );
    bool usernameToken() const;

    QStringList tokens() const;

    bool loadXML( ParserContext *context, const QDomElement &element );
    void saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const;

  private:
    bool mUsernameToken;
    QStringList mTokens;
};

}

#endif // KWSDL_TYPE_H

