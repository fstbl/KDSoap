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

#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include <schema/parser.h>

#include <QDebug>

#include "policy.h"

using namespace KWSDL;

Policy::Policy()
{
}

Policy::Policy( const QString &nameSpace )
  : Element( nameSpace )
{
}

Policy::~Policy()
{
}

void Policy::setUsernameToken(const bool &b)
{
  mUsernameToken = b;
}

bool Policy::usernameToken() const
{
  return mUsernameToken;
}

QStringList Policy::tokens() const
{
  return mTokens;
}

bool Policy::loadXML( ParserContext *context, const QDomElement &element )
{
  QStringList elementNamespace = element.nodeName().split(":");
  QString nameSpace = elementNamespace.length() == 2? elementNamespace[0]+":":"";
  QDomNodeList policyElements = element.elementsByTagName( nameSpace+QLatin1String("Policy") );

  for(int i=0; i<policyElements.length(); i++) {
    for(int j=0; j<policyElements.item(i).childNodes().length(); j++) {
      if(policyElements.item(i).childNodes().item(j).nodeName() == "#text") {
        continue;
      }
      QString fqTokenName = policyElements.item(i).childNodes().item(j).nodeName();
      QStringList namespaceTokenName = fqTokenName.split(":");
      mTokens.append(namespaceTokenName.length()>1? namespaceTokenName[1]:namespaceTokenName[0]);
    }
  }

  mUsernameToken = false;
  Q_FOREACH(QString token, mTokens) {
    if(token == "UsernameToken") {
      mUsernameToken = true;
    }
  }
  return true;
}

void Policy::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  Q_UNUSED( context );
  Q_UNUSED( document );
  Q_UNUSED( parent );
}
