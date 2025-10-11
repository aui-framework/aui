#ifndef DBUSDAEMONPROXY_H
#define DBUSDAEMONPROXY_H

#include <AUI/Platform/linux/dbus-cxx/dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
namespace DBus {
class DBusDaemonProxy
    : public DBus::ObjectProxy {
protected:
    DBusDaemonProxy( std::shared_ptr<DBus::Connection> conn, std::string dest = "org.freedesktop.DBus", std::string path = "/org/freedesktop/DBus", DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread );
public:
    static std::shared_ptr<DBusDaemonProxy> create( std::shared_ptr<DBus::Connection> conn, std::string dest = "org.freedesktop.DBus", std::string path = "/org/freedesktop/DBus", DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread );
    std::string Hello( );
    uint32_t RequestName( std::string arg0, uint32_t arg1 );
    uint32_t ReleaseName( std::string arg0 );
    uint32_t StartServiceByName( std::string arg0, uint32_t arg1 );
    void UpdateActivationEnvironment( std::map<std::string, std::string> arg0 );
    bool NameHasOwner( std::string arg0 );
    std::vector<std::string> ListNames( );
    std::vector<std::string> ListActivatableNames( );
    void AddMatch( std::string arg0 );
    void RemoveMatch( std::string arg0 );
    std::string GetNameOwner( std::string arg0 );
    std::vector<std::string> ListQueuedOwners( std::string arg0 );
    uint32_t GetConnectionUnixUser( std::string arg0 );
    uint32_t GetConnectionUnixProcessID( std::string arg0 );
    std::vector<uint8_t> GetAdtAuditSessionData( std::string arg0 );
    std::vector<uint8_t> GetConnectionSELinuxSecurityContext( std::string arg0 );
    void ReloadConfig( );
    std::string GetId( );
    std::map<std::string, DBus::Variant> GetConnectionCredentials( std::string arg0 );
    std::shared_ptr<DBus::SignalProxy<void(std::string, std::string, std::string)>> signal_NameOwnerChanged( );
    std::shared_ptr<DBus::SignalProxy<void(std::string)>> signal_NameLost( );
    std::shared_ptr<DBus::SignalProxy<void(std::string)>> signal_NameAcquired( );
    DBus::Variant Get( std::string arg0, std::string arg1 );
    std::map<std::string, DBus::Variant> GetAll( std::string arg0 );
    void Set( std::string arg0, std::string arg1, DBus::Variant arg2 );
    std::shared_ptr<DBus::SignalProxy<void(std::string, std::map<std::string, DBus::Variant>, std::vector<std::string>)>> signal_PropertiesChanged( );
    std::string Introspect( );
    void BecomeMonitor( std::vector<std::string> arg0, uint32_t arg1 );
    std::map<std::string, DBus::Variant> GetStats( );
    std::map<std::string, DBus::Variant> GetConnectionStats( std::string arg0 );
    std::map<std::string, std::vector<std::string>> GetAllMatchRules( );
    std::string GetMachineId( );
    void Ping( );
protected:
    std::shared_ptr<DBus::MethodProxy<std::string()>>  m_method_Hello;
    std::shared_ptr<DBus::MethodProxy<uint32_t( std::string, uint32_t )>>  m_method_RequestName;
    std::shared_ptr<DBus::MethodProxy<uint32_t( std::string )>>  m_method_ReleaseName;
    std::shared_ptr<DBus::MethodProxy<uint32_t( std::string, uint32_t )>>  m_method_StartServiceByName;
    std::shared_ptr<DBus::MethodProxy<void( std::map<std::string, std::string> )>>  m_method_UpdateActivationEnvironment;
    std::shared_ptr<DBus::MethodProxy<bool( std::string )>>  m_method_NameHasOwner;
    std::shared_ptr<DBus::MethodProxy<std::vector<std::string>()>>  m_method_ListNames;
    std::shared_ptr<DBus::MethodProxy<std::vector<std::string>()>>  m_method_ListActivatableNames;
    std::shared_ptr<DBus::MethodProxy<void( std::string )>>  m_method_AddMatch;
    std::shared_ptr<DBus::MethodProxy<void( std::string )>>  m_method_RemoveMatch;
    std::shared_ptr<DBus::MethodProxy<std::string( std::string )>>  m_method_GetNameOwner;
    std::shared_ptr<DBus::MethodProxy<std::vector<std::string>( std::string )>>  m_method_ListQueuedOwners;
    std::shared_ptr<DBus::MethodProxy<uint32_t( std::string )>>  m_method_GetConnectionUnixUser;
    std::shared_ptr<DBus::MethodProxy<uint32_t( std::string )>>  m_method_GetConnectionUnixProcessID;
    std::shared_ptr<DBus::MethodProxy<std::vector<uint8_t>( std::string )>>  m_method_GetAdtAuditSessionData;
    std::shared_ptr<DBus::MethodProxy<std::vector<uint8_t>( std::string )>>  m_method_GetConnectionSELinuxSecurityContext;
    std::shared_ptr<DBus::MethodProxy<void()>>  m_method_ReloadConfig;
    std::shared_ptr<DBus::MethodProxy<std::string()>>  m_method_GetId;
    std::shared_ptr<DBus::MethodProxy<std::map<std::string, DBus::Variant>( std::string )>>  m_method_GetConnectionCredentials;
    std::shared_ptr<DBus::SignalProxy<void(std::string, std::string, std::string)>> m_signalproxy_NameOwnerChanged;
    std::shared_ptr<DBus::SignalProxy<void(std::string)>> m_signalproxy_NameLost;
    std::shared_ptr<DBus::SignalProxy<void(std::string)>> m_signalproxy_NameAcquired;
    std::shared_ptr<DBus::MethodProxy<DBus::Variant( std::string, std::string )>>  m_method_Get;
    std::shared_ptr<DBus::MethodProxy<std::map<std::string, DBus::Variant>( std::string )>>  m_method_GetAll;
    std::shared_ptr<DBus::MethodProxy<void( std::string, std::string, DBus::Variant )>>  m_method_Set;
    std::shared_ptr<DBus::SignalProxy<void(std::string, std::map<std::string, DBus::Variant>, std::vector<std::string>)>> m_signalproxy_PropertiesChanged;
    std::shared_ptr<DBus::MethodProxy<std::string()>>  m_method_Introspect;
    std::shared_ptr<DBus::MethodProxy<void( std::vector<std::string>, uint32_t )>>  m_method_BecomeMonitor;
    std::shared_ptr<DBus::MethodProxy<std::map<std::string, DBus::Variant>()>>  m_method_GetStats;
    std::shared_ptr<DBus::MethodProxy<std::map<std::string, DBus::Variant>( std::string )>>  m_method_GetConnectionStats;
    std::shared_ptr<DBus::MethodProxy<std::map<std::string, std::vector<std::string>>()>>  m_method_GetAllMatchRules;
    std::shared_ptr<DBus::MethodProxy<std::string()>>  m_method_GetMachineId;
    std::shared_ptr<DBus::MethodProxy<void()>>  m_method_Ping;
};
} /* namespace DBus */
#endif /* DBUSDAEMONPROXY_H */
