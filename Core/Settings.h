#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string>

#include "BaseSettings.h"
#include "Logger.h"
#include "Singleton.h"


namespace Fatracing {

struct SettingsStruct {
    std::string PortName;
    int RaceTimeSeconds;
};

class Settings : public BaseSettings<SettingsStruct> {
    friend class SingletonHolder<Settings>;

    Logger& mLogger = Logger::Instance();

private:
    Settings();
    ~Settings();

protected:
    SettingsStruct Parse(QXmlStreamReader& xml) override;
    void Save(QXmlStreamWriter& xml, const SettingsStruct& aSettings) override;

public:
    Settings(const Settings&) = delete;
    void operator=(Settings const&) = delete;

};

typedef SingletonHolder<Settings> SettingsSingleton;

} // namespace Fatracing

#endif // SETTINGS_H_
