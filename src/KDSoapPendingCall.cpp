#include "KDSoapPendingCall.h"
#include "KDSoapPendingCall_p.h"
#include "KDSoapMessage_p.h"
#include <QNetworkReply>
#include <QDebug>

KDSoapPendingCall::Private::~Private()
{
    delete reply.data();
    delete buffer;
}


KDSoapPendingCall::KDSoapPendingCall(QNetworkReply* reply, QBuffer* buffer)
    : d(new Private(reply, buffer))
{
}

KDSoapPendingCall::KDSoapPendingCall(const KDSoapPendingCall &other)
    : d(other.d)
{
}

KDSoapPendingCall::~KDSoapPendingCall()
{
}

KDSoapPendingCall &KDSoapPendingCall::operator=(const KDSoapPendingCall &other)
{
    d = other.d;
    return *this;
}

bool KDSoapPendingCall::isFinished() const
{
    return d->reply.data()->isFinished();
}

KDSoapMessage KDSoapPendingCall::returnMessage() const
{
    d->parseReply();
    return d->replyMessage;
}

QVariant KDSoapPendingCall::returnValue() const
{
    d->parseReply();
    if (!d->replyMessage.d->args.isEmpty())
        return d->replyMessage.d->args.first().value();
    return QVariant();
}

QVariant KDSoapPendingCall::Private::parseReplyElement(QXmlStreamReader& reader)
{
    //qDebug() << "parsing" << reader.name();
    KDSoapValueList lst;
    QString text;
    while (reader.readNext()) {
        if (reader.isEndElement())
            break;
        if (reader.isCharacters()) {
            text = reader.text().toString();
            //qDebug() << "text=" << text;
        } else if (reader.isStartElement()) {
            const QVariant subVal = parseReplyElement(reader);
            const QString name = reader.name().toString();
            lst.append(KDSoapValue(name, subVal));
        }
    }
    if (!lst.isEmpty())
        return QVariant::fromValue(lst);
    return text;
}

void KDSoapPendingCall::Private::parseReply()
{
    if (parsed)
        return;
    parsed = true;
    const bool doDebug = qgetenv("KDSOAP_DEBUG").toInt();
    QNetworkReply* reply = this->reply.data();
    if (!reply->isFinished()) {
        qWarning("KDSoap: Parsing reply before it finished!");
    }
    if (reply->error()) {
        replyMessage.setFault(true);
        replyMessage.addArgument(QString::fromLatin1("faultcode"), QString::number(reply->error()));
        replyMessage.addArgument(QString::fromLatin1("faultstring"), reply->errorString());
        if (doDebug)
            qDebug() << reply->errorString();
    } else {
        const QByteArray data = reply->readAll();
        if (doDebug)
            qDebug() << data;
        QXmlStreamReader reader(data);
        const QString soapNS = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
        //const QString xmlSchemaNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
        //const QString xmlSchemaInstanceNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
        if (reader.readNextStartElement() && reader.name() == "Envelope" && reader.namespaceUri() == soapNS) {
            if (reader.readNextStartElement() && reader.name() == "Body" && reader.namespaceUri() == soapNS) {

                if (reader.readNextStartElement()) { // the method: Response or Fault
                    //qDebug() << "toplevel element:" << reader.name();
                    if (reader.name() == "Fault")
                        replyMessage.setFault(true);

                    while (reader.readNextStartElement()) { // Result
                        const QString name = reader.name().toString();
                        const QVariant val = parseReplyElement(reader);
                        replyMessage.addArgument(name, val);
                        if (doDebug)
                            qDebug() << "got item" << name << "val=" << val;
                    }
                }

            } else {
                reader.raiseError(QObject::tr("Invalid SOAP Response, Body expected"));
            }
        } else {
            reader.raiseError(QObject::tr("Invalid SOAP Response, Envelope expected"));
        }
    }
}
