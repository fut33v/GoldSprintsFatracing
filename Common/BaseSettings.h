#ifndef SUBPRJ_COMMON_BASE_BASESETTINGS_H_
#define SUBPRJ_COMMON_BASE_BASESETTINGS_H_

#include <map>
#include <mutex>
#include <string>
#include <iostream>
#include <cstring>

#include "Logger.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>


namespace Fatracing {

//! Базовый класс настроек
template <class T>
class BaseSettings {
	//! Имя файла для хранения настроек
	const char* mFileName;
	// Имя корневого элемента XML 
	const char* mSettingsRoot;

protected:
	//! Мьютекс для синхронизации доступа к настройккам
	std::recursive_mutex mSettingsMutex;
	//! Структура с настройками
	T mSettings;

	static constexpr const char* VALUE_TRUE = "true";
	static constexpr const char* VALUE_FALSE = "false";

public:
	//! Конструктор
	//! @param aFileName имя файла с настройками
	//! @param aSettingsRoot имя корневого элемента XML
	BaseSettings(const char* aFileName, const char* aSettingsRoot);
	//! Деструктор
	virtual ~BaseSettings();

	//! Установить имя файла
	void SetFileName(const char* aFileName);

protected:
	//! Получение настроек из XML 
	//! @param aSettings корневой элемент XML
	virtual T Parse(QXmlStreamReader& aSettings) = 0;
	//! Сохранение настроек в файл
	//! @param aRoot корневой элемент XML
	//! @param aSettings структура с настройками
	virtual void Save(QXmlStreamWriter& aRoot, const T& aSettings) = 0;

public:
	//! Загрузить настройки из файла
	virtual bool LoadSettings();
	//! Сохранить настройки в файл
	virtual bool SaveSettings();
	//! Сохранить настройки в строку
	//		virtual std::string	SettingsToString();

	//! Получить текущую копию настроек
	virtual T GetSettings();
	//! Обновить настройки
	//! @param aInfo структура с новыми настройками
	virtual bool UpdateSettings(const T& aInfo);
};

template <class T>
inline BaseSettings<T>::BaseSettings(const char* aFileName, const char* aSettingsRoot) :
	mFileName(aFileName),
	mSettingsRoot(aSettingsRoot) {
	mSettings = {};
}

template <class T>
inline BaseSettings<T>::~BaseSettings() {
}

template <class T>
void BaseSettings<T>::SetFileName(const char* aFileName) {
	mFileName = aFileName;
}

template <class T>
inline bool
BaseSettings<T>::LoadSettings() {
	QXmlStreamReader xml;
	auto fileContent = Utils::GetFileContents(mFileName);
	if (fileContent.empty()) {
		return false;
	}
	xml.addData(fileContent.c_str());

	while (!xml.atEnd()) {
		xml.readNextStartElement();
		if (xml.name() == QString::fromStdString(mSettingsRoot)) {
			xml.readNextStartElement();
			T settings = Parse(xml);
			std::lock_guard<std::recursive_mutex> lock(mSettingsMutex);
			mSettings = settings;
			return true;
		}
	}

	if (xml.hasError()) {
		// TODO: error handling
	}

	return false;
}

template <class T>
inline bool
BaseSettings<T>::SaveSettings() {
	T settings;
	{
		std::lock_guard<std::recursive_mutex> lock(mSettingsMutex);
		settings = mSettings;
	}

	QFile file(mFileName);
	file.open(QIODevice::WriteOnly);

	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);

	xml.writeStartDocument();
	xml.writeStartElement(QString::fromStdString(mSettingsRoot));

	Save(xml, settings);

	xml.writeEndElement();
	xml.writeEndDocument();

	return true;
}

template <class T>
inline bool
BaseSettings<T>::UpdateSettings(const T& aInfo) {
	std::unique_lock<std::recursive_mutex> lock(mSettingsMutex);
	mSettings = aInfo;
	return SaveSettings();
}

template <class T>
inline T
BaseSettings<T>::GetSettings() {
	std::lock_guard<std::recursive_mutex> lock(mSettingsMutex);
	return mSettings;
}

//template <class T>
//inline std::string
//BaseSettings<T>::SettingsToString()
//{
//	T settings;
//	{
//		std::lock_guard<std::recursive_mutex> lock(mSettingsMutex);
//		settings = mSettings;
//	}

//	pugi::xml_document doc;
//	auto root = doc.append_child(mSettingsRoot);
//	Save(&root, settings);
//	xml_string_writer writer;
//	root.print(writer);

//	return writer.result;
//}

} // namespace Fatracing

#endif // SUBPRJ_COMMON_BASE_BASESETTINGS_H_
