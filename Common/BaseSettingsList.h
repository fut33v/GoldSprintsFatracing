#ifndef SUBPRJ_COMMON_BASE_BASESETTINGSLIST_H_
#define SUBPRJ_COMMON_BASE_BASESETTINGSLIST_H_

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include <QXmlStreamReader>
#include <QFile>
#include "Utils.h"


namespace VMM {

template <typename T>
class BaseSettingsList {
protected:
    std::string mFileName;
    std::string mSettingsRoot;
    std::string mElementName;

    std::mutex mSettingsFileMutex;
    std::mutex mSettingsMutex;
    std::map<unsigned int, T> mSettings;

protected:
    BaseSettingsList() = default;
    BaseSettingsList(const std::string& aFileName, const std::string& aSettingsRoot, const std::string& aElementName);

    virtual T ParseElement(QXmlStreamReader& xml) = 0;
    virtual void Save(QXmlStreamWriter& xml, const T& aElement) = 0;
    virtual bool SaveSettings(const std::map<unsigned int, T>& aSettings);

public:
    virtual void Init(const std::string& aFileName, const std::string& aSettingsRoot, const std::string& aElementName);

    virtual bool LoadSettings();
    virtual bool SaveSettings();

    virtual bool AddInfo(unsigned int aId, const T& aInfo);
    virtual bool RemoveInfo(unsigned int aId);

    virtual bool GetInfo(unsigned int aId, T& aInfo);
    virtual bool UpdateInfo(unsigned int aId, const T& aInfo);

    virtual const std::map<unsigned int, T> GetSettings();
};

template <typename T>
BaseSettingsList<T>::BaseSettingsList(const std::string& aFileName, const std::string& aSettingsRoot, const std::string& aElementName) :
    mFileName(aFileName),
    mSettingsRoot(aSettingsRoot),
    mElementName(aElementName)
{
}

template <typename T>
inline void
BaseSettingsList<T>::Init(const std::string& aFileName, const std::string& aSettingsRoot, const std::string& aElementName) {
    mFileName = aFileName;
    mSettingsRoot = aSettingsRoot;
    mElementName = aElementName;
}

template <typename T>
inline bool
BaseSettingsList<T>::LoadSettings()
{
    bool settingsRead = false;
    std::map<unsigned int, T> settings;
    {      
        std::lock_guard<std::mutex> lock(mSettingsFileMutex);

        QXmlStreamReader xml;
        auto fileContent = Utils::GetFileContents(mFileName.c_str());
        xml.addData(fileContent.c_str());
       
        while (!xml.atEnd()) {
            xml.readNextStartElement();
            if (xml.name() == QString::fromStdString(mSettingsRoot)) {
			    xml.readNextStartElement();
                if (xml.name() == QString::fromStdString(mElementName)) {
                    auto attributes =xml.attributes();
                    if (attributes.hasAttribute("id")) {
                        unsigned int id = attributes.value("id").toUInt();
                        settings[id] = ParseElement(xml);
                    }
                }
            }
        }
        
        
        if (xml.hasError()) {
            // TODO: error handling
        }

        settingsRead = true;
    }

    if (settingsRead) {
        std::lock_guard<std::mutex> lock(mSettingsMutex);
        mSettings = settings;
        return true;
    }
    return false;
}

template <class T>
inline bool
BaseSettingsList<T>::SaveSettings(const std::map<unsigned int, T>& aSettings) {
    std::lock_guard<std::mutex> lock(mSettingsFileMutex);

    QFile file(mFileName.c_str());
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement(QString::fromStdString(mSettingsRoot));

    for (auto node : aSettings) {
        xml.writeStartElement(QString::fromStdString(mElementName));
        xml.writeAttribute("id", QString::number(node.first));
        Save(xml, node.second);
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return false;
}

template <class T>
inline bool
BaseSettingsList<T>::SaveSettings() {
    std::map<unsigned int, T> settings;
    {
        std::lock_guard<std::mutex> lock(mSettingsMutex);
        settings = mSettings;
    }
    return SaveSettings(settings);
}

template <class T>
inline bool
BaseSettingsList<T>::GetInfo(unsigned int aId, T& aInfo) {
    std::lock_guard<std::mutex> lock(mSettingsMutex);
    auto itr = mSettings.find(aId);
    if (itr != mSettings.end()) {
        aInfo = itr->second;
        return true;
    }
    return false;
}

template <class T>
inline bool
BaseSettingsList<T>::UpdateInfo(unsigned int aId, const T& aInfo) {
    std::lock_guard<std::mutex> lock(mSettingsMutex);
    auto itr = mSettings.find(aId);
    if (itr != mSettings.end()) {
        itr->second = aInfo;
        return SaveSettings(mSettings);
    }
    return false;
}

template <class T>
inline bool
BaseSettingsList<T>::AddInfo(unsigned int aId, const T& aInfo) {
    std::lock_guard<std::mutex> lock(mSettingsMutex);
    mSettings.emplace(aId, aInfo);
    return SaveSettings(mSettings);
}

template <class T>
inline bool
BaseSettingsList<T>::RemoveInfo(unsigned int aId) {
    std::lock_guard<std::mutex> lock(mSettingsMutex);
    if (mSettings.erase(aId) != 0) {
        return SaveSettings(mSettings);
    }
    return false;
}

template <class T>
inline const std::map<unsigned int, T>
BaseSettingsList<T>::GetSettings() {
    std::lock_guard<std::mutex> lock(mSettingsMutex);
    return mSettings;
}
}
#endif // SUBPRJ_COMMON_BASE_BASESETTINGSLIST_H_
