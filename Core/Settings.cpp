#include "./Settings.h"


namespace Fatracing {
Settings::Settings() :
    BaseSettings("GoldSprintsSettings.xml", "GoldSprintsSettings") {
}

Settings::~Settings() {
}

SettingsStruct Settings::Parse(QXmlStreamReader& xml) {
    SettingsStruct params;

    while (!xml.atEnd()) {
        QStringRef name = xml.name();
        QString value = xml.readElementText();
        if (name == QString::fromStdString("RaceTimeSeconds")) {
            params.RaceTimeSeconds = value.toUInt();
        }
        else if (name == QString::fromStdString("PortName")) {
            params.PortName = value.toStdString();
        }

        xml.readNextStartElement();
    }

    return params;
}

void Settings::Save(QXmlStreamWriter& xml, const SettingsStruct& aSettings) {
    auto writeElement = [&](std::string name, QString value) {
        xml.writeStartElement(QString::fromStdString(name));
        xml.writeCharacters(value);
        xml.writeEndElement();
    };

    writeElement("RaceTimeSeconds", QString::number(aSettings.RaceTimeSeconds));
    writeElement("PortName", QString::fromStdString(aSettings.PortName));
}
} // namespace Fatracing
