#include "settings.h"

#include "converter.h"
#include <QDebug>

using namespace KWSDL;

QString upperlize( const QString &str )
{
  return str[ 0 ].toUpper() + str.mid( 1 );
}

QString lowerlize( const QString &str )
{
  return str[ 0 ].toLower() + str.mid( 1 );
}


Converter::Converter()
{
  mQObject = KODE::Class( QLatin1String("QObject") );
  mKDSoapServerObjectInterface = KODE::Class( QLatin1String("KDSoapServerObjectInterface") );
}

void Converter::setWSDL( const WSDL &wsdl )
{
  mWSDL = wsdl;

  // merge namespaces from wsdl and schema
  QStringList namespaces = wsdl.definitions().type().types().namespaces();
  namespaces.append( QLatin1String("http://schemas.xmlsoap.org/soap/encoding/") );
  const QStringList wsdlNamespaces = wsdl.namespaceManager().uris();
  for ( int i = 0; i < wsdlNamespaces.count(); ++i ) {
    if ( !namespaces.contains( wsdlNamespaces[ i ] ) )
      namespaces.append( wsdlNamespaces[ i ] );
  }

  // Keep the prefixes from the wsdl parsing, they are more meaningful than ns1 :)
  mNSManager = wsdl.namespaceManager();

  // overwrite some default prefixes
  mNSManager.setPrefix( QLatin1String("soapenc"), QLatin1String("http://schemas.xmlsoap.org/soap/encoding/") );
  mNSManager.setPrefix( QLatin1String("http"), QLatin1String("http://schemas.xmlsoap.org/wsdl/http/") );
  mNSManager.setPrefix( QLatin1String("soap"), QLatin1String("http://schemas.xmlsoap.org/wsdl/soap/") );
  mNSManager.setPrefix( QLatin1String("xsd"), QLatin1String("http://www.w3.org/2001/XMLSchema") );
  mNSManager.setPrefix( QLatin1String("xsi"), QLatin1String("http://www.w3.org/2001/XMLSchema-instance") );

  // overwrite with prefixes from settings
  Settings::NSMapping mapping = Settings::self()->namespaceMapping();
  Settings::NSMapping::Iterator it;
  for ( it = mapping.begin(); it != mapping.end(); ++it )
    mNSManager.setPrefix( it.value(), it.key() );

  if (qgetenv("KDSOAP_TYPE_DEBUG").toInt())
      mNSManager.dump();

  mTypeMap.setNSManager( &mNSManager );

  cleanupUnusedTypes();

  // set the xsd types
  mTypeMap.addSchemaTypes( mWSDL.definitions().type().types(), Settings::self()->nameSpace() );

  if (qgetenv("KDSOAP_TYPE_DEBUG").toInt())
    mTypeMap.dump();
}

class TypeCollector
{
public:
    TypeCollector(const QSet<QName>& usedTypes) : m_allUsedTypes(usedTypes) {}

    void collectDependentTypes(const TypeMap& typeMap, const XSD::Types& types)
    {
        QSet<QName> typesToProcess = m_allUsedTypes;
        do {
            m_alsoUsedTypes.clear();
            Q_FOREACH(const QName& typeName, typesToProcess.toList() /*slow!*/) {
                if (typeName.isEmpty())
                    continue;
                if (typeMap.isBuiltinType(typeName))
                    continue;
                //qDebug() << "used type:" << typeName;
                XSD::ComplexType complexType = types.complexType(typeName);
                if (!complexType.name().isEmpty()) { // found it as a complex type
                    usedComplexTypes.append(complexType);

                    addDependency(complexType.baseTypeName());
                    Q_FOREACH(const XSD::Element& element, complexType.elements()) {
                        addDependency(element.type());
                    }
                    Q_FOREACH(const XSD::Attribute& attribute, complexType.attributes()) {
                        addDependency(attribute.type());
                    }
                    addDependency(complexType.arrayType());

                } else {
                    XSD::SimpleType simpleType = types.simpleType(typeName);
                    if (!simpleType.name().isEmpty()) {
                        usedSimpleTypes.append(simpleType);
                        addDependency(simpleType.baseTypeName());
                        if (simpleType.subType() == XSD::SimpleType::TypeList) {
                            addDependency(simpleType.listTypeName());
                        }
                    } // we rely on the warning in simpleType if not found.
                }
            }
            typesToProcess = m_alsoUsedTypes;
        } while (!typesToProcess.isEmpty());
    }

    XSD::ComplexType::List usedComplexTypes;
    XSD::SimpleType::List usedSimpleTypes;
private:
    void addDependency(const QName& type) {
        if (!type.isEmpty() && !m_allUsedTypes.contains(type) && !m_alsoUsedTypes.contains(type)) {
            m_alsoUsedTypes.insert(type);
            m_allUsedTypes.insert(type);
        }
    }

    QSet<QName> m_allUsedTypes; // All already seen types
    QSet<QName> m_alsoUsedTypes; // The list of types to process in the next iteration
};

void Converter::cleanupUnusedTypes()
{
    // Keep only the portTypes, messages, and types that are actually used, no point in generating unused classes.

    Definitions definitions = mWSDL.definitions();
    Type type = definitions.type();
    XSD::Types types = type.types();

    const bool printDebug = qgetenv("KDSOAP_TYPE_DEBUG").toInt();
    if (printDebug) {
        qDebug() << "Before cleanup:";
        qDebug() << definitions.messages().count() << "messages";
        qDebug() << types.complexTypes().count() << "complex types";
        qDebug() << types.simpleTypes().count() << "simple types";
        qDebug() << types.elements().count() << "elements";

        //Q_FOREACH(const XSD::Element& elem, types.elements()) {
        //    qDebug() << "element:" << elem.qualifiedName();
        //}
    }

    QSet<QName> usedMessageNames;
    //QSet<QName> portTypeNames;
    Q_FOREACH( const Service& service, definitions.services() ) {
        Q_FOREACH( const Port& port, service.ports() ) {
            Binding binding = mWSDL.findBinding( port.bindingName() );
            //portTypeNames.insert( binding.portTypeName() );
            //qDebug() << "binding" << port.bindingName() << binding.name() << "port type" << binding.portTypeName();
            PortType portType = mWSDL.findPortType( binding.portTypeName() );
            const Operation::List operations = portType.operations();
            //qDebug() << "portType" << portType.name() << operations.count() << "operations";
            Q_FOREACH( const Operation& operation, operations ) {
                //qDebug() << "  operation" << operation.operationType() << operation.name();
                switch(operation.operationType()) {
                case Operation::OneWayOperation:
                    usedMessageNames.insert(operation.input().message());
                    break;
                case Operation::RequestResponseOperation:
                case Operation::SolicitResponseOperation:
                    usedMessageNames.insert(operation.input().message());
                    usedMessageNames.insert(operation.output().message());
                    break;
                case Operation::NotificationOperation:
                    usedMessageNames.insert(operation.output().message());
                    break;
                };
                if ( binding.type() == Binding::SOAPBinding ) {
                    const SoapBinding soapBinding( binding.soapBinding() );
                    const SoapBinding::Operation op = soapBinding.operations().value( operation.name() );
                    Q_FOREACH(const SoapBinding::Header& header, op.inputHeaders()) {
                        usedMessageNames.insert(header.message());
                    }
                    Q_FOREACH(const SoapBinding::Header& header, op.outputHeaders()) {
                        usedMessageNames.insert(header.message());
                    }
                }
            }
        }
    }

    // Keep only the messages in usedMessageNames
    QSet<QName> usedTypes;
    QSet<QString> usedTypesStrings; // for debug
    QSet<QName> usedElementNames;
    Message::List newMessages;
    Q_FOREACH(const QName& messageName, usedMessageNames.toList() /*slow!*/) {
        //qDebug() << "used message:" << messageName;
        Message message = mWSDL.findMessage(messageName);
        newMessages.append(message);
        Q_FOREACH(const Part& part, message.parts()) {
            if (!part.type().isEmpty()) {
                usedTypes.insert(part.type());
                usedTypesStrings.insert(part.type().qname());
            } else {
                const QName elemName = part.element();
                XSD::Element element = mWSDL.findElement(elemName);
                if (element.qualifiedName().isEmpty()) {
                    qDebug() << "in message" << messageName << ": element not found:" << elemName.qname();
                } else if (element.type().isEmpty()) {
                    qDebug() << "in message" << messageName << ": element without type:" << elemName.qname();
                } else {
                    usedElementNames.insert(element.qualifiedName());
                    usedTypes.insert(element.type());
                    usedTypesStrings.insert(element.type().qname());
                }
            }
        }
    }

    //qDebug() << "usedTypes:" << usedTypesStrings.toList();

    // keep only the types used in these messages
    TypeCollector collector(usedTypes);
    collector.collectDependentTypes(mTypeMap, types);

    XSD::Element::List usedElements;
    QSetIterator<QName> elemIt(usedElementNames);
    while (elemIt.hasNext()) {
        const QName name = elemIt.next();
        XSD::Element element = mWSDL.findElement(name);

        if (element.type().isEmpty()) {
          qDebug() << "ERROR: Element without type:" << element.qualifiedName() << element.nameSpace() << element.name();
          Q_ASSERT(!element.type().isEmpty());
        }

        if (element.name().isEmpty())
            qDebug() << "cleanupUnusedTypes: element" << name << "not found";
        else
            usedElements.append(element);
    }
    definitions.setMessages(newMessages);
    types.setComplexTypes(collector.usedComplexTypes);
    types.setSimpleTypes(collector.usedSimpleTypes);
    types.setElements(usedElements);
    type.setTypes(types);
    definitions.setType(type);

    mWSDL.setDefinitions(definitions);

    if (printDebug) {
        qDebug() << "After cleanup:";
        qDebug() << definitions.messages().count() << "messages";
        qDebug() << types.complexTypes().count() << "complex types";
        qDebug() << types.simpleTypes().count() << "simple types";
        qDebug() << types.elements().count() << "elements";
    }
}

KODE::Class::List Converter::classes() const
{
  return mClasses;
}

bool Converter::convert()
{
    convertTypes();
    //  mNSManager.dump();
    if (Settings::self()->generateServerCode()) {
        convertServerService();
    }
    return convertClientService();
}

void Converter::convertTypes()
{
  const XSD::Types types = mWSDL.definitions().type().types();

  XSD::SimpleType::List simpleTypes = types.simpleTypes();
  qDebug() << "Converting" << simpleTypes.count() << "simple types";
  for ( int i = 0; i < simpleTypes.count(); ++i )
    convertSimpleType( &(simpleTypes[ i ]), simpleTypes );

  XSD::ComplexType::List complexTypes = types.complexTypes();
  qDebug() << "Converting" << complexTypes.count() << "complex types";
  for ( int i = 0; i < complexTypes.count(); ++i )
    convertComplexType( &(complexTypes[ i ]) );
}
