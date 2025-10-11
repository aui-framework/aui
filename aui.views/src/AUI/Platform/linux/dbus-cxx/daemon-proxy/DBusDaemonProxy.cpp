#include "DBusDaemonProxy.h"

using DBus::DBusDaemonProxy;

DBusDaemonProxy::DBusDaemonProxy( std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path, DBus::ThreadForCalling signalCallingThread ) : DBus::ObjectProxy( conn, dest, path ) {
    m_method_Hello = this->create_method<std::string()>( "org.freedesktop.DBus", "Hello" );
    m_method_RequestName = this->create_method<uint32_t( std::string, uint32_t )>( "org.freedesktop.DBus", "RequestName" );
    m_method_ReleaseName = this->create_method<uint32_t( std::string )>( "org.freedesktop.DBus", "ReleaseName" );
    m_method_StartServiceByName = this->create_method<uint32_t( std::string, uint32_t )>( "org.freedesktop.DBus", "StartServiceByName" );
    m_method_UpdateActivationEnvironment = this->create_method<void( std::map<std::string, std::string> )>( "org.freedesktop.DBus", "UpdateActivationEnvironment" );
    m_method_NameHasOwner = this->create_method<bool( std::string )>( "org.freedesktop.DBus", "NameHasOwner" );
    m_method_ListNames = this->create_method<std::vector<std::string>()>( "org.freedesktop.DBus", "ListNames" );
    m_method_ListActivatableNames = this->create_method<std::vector<std::string>()>( "org.freedesktop.DBus", "ListActivatableNames" );
    m_method_AddMatch = this->create_method<void( std::string )>( "org.freedesktop.DBus", "AddMatch" );
    m_method_RemoveMatch = this->create_method<void( std::string )>( "org.freedesktop.DBus", "RemoveMatch" );
    m_method_GetNameOwner = this->create_method<std::string( std::string )>( "org.freedesktop.DBus", "GetNameOwner" );
    m_method_ListQueuedOwners = this->create_method<std::vector<std::string>( std::string )>( "org.freedesktop.DBus", "ListQueuedOwners" );
    m_method_GetConnectionUnixUser = this->create_method<uint32_t( std::string )>( "org.freedesktop.DBus", "GetConnectionUnixUser" );
    m_method_GetConnectionUnixProcessID = this->create_method<uint32_t( std::string )>( "org.freedesktop.DBus", "GetConnectionUnixProcessID" );
    m_method_GetAdtAuditSessionData = this->create_method<std::vector<uint8_t>( std::string )>( "org.freedesktop.DBus", "GetAdtAuditSessionData" );
    m_method_GetConnectionSELinuxSecurityContext = this->create_method<std::vector<uint8_t>( std::string )>( "org.freedesktop.DBus", "GetConnectionSELinuxSecurityContext" );
    m_method_ReloadConfig = this->create_method<void()>( "org.freedesktop.DBus", "ReloadConfig" );
    m_method_GetId = this->create_method<std::string()>( "org.freedesktop.DBus", "GetId" );
    m_method_GetConnectionCredentials = this->create_method<std::map<std::string, DBus::Variant>( std::string )>( "org.freedesktop.DBus", "GetConnectionCredentials" );
    m_signalproxy_NameOwnerChanged = this->create_signal<void(std::string, std::string, std::string)>( "org.freedesktop.DBus", "NameOwnerChanged" );
    m_signalproxy_NameLost = this->create_signal<void(std::string)>( "org.freedesktop.DBus", "NameLost" );
    m_signalproxy_NameAcquired = this->create_signal<void(std::string)>( "org.freedesktop.DBus", "NameAcquired" );
    m_method_Get = this->create_method<DBus::Variant( std::string, std::string )>( "org.freedesktop.DBus.Properties", "Get" );
    m_method_GetAll = this->create_method<std::map<std::string, DBus::Variant>( std::string )>( "org.freedesktop.DBus.Properties", "GetAll" );
    m_method_Set = this->create_method<void( std::string, std::string, DBus::Variant )>( "org.freedesktop.DBus.Properties", "Set" );
    m_signalproxy_PropertiesChanged = this->create_signal<void(std::string, std::map<std::string, DBus::Variant>, std::vector<std::string>)>( "org.freedesktop.DBus.Properties", "PropertiesChanged" );
    m_method_Introspect = this->create_method<std::string()>( "org.freedesktop.DBus.Introspectable", "Introspect" );
    m_method_BecomeMonitor = this->create_method<void( std::vector<std::string>, uint32_t )>( "org.freedesktop.DBus.Monitoring", "BecomeMonitor" );
    m_method_GetStats = this->create_method<std::map<std::string, DBus::Variant>()>( "org.freedesktop.DBus.Debug.Stats", "GetStats" );
    m_method_GetConnectionStats = this->create_method<std::map<std::string, DBus::Variant>( std::string )>( "org.freedesktop.DBus.Debug.Stats", "GetConnectionStats" );
    m_method_GetAllMatchRules = this->create_method<std::map<std::string, std::vector<std::string>>()>( "org.freedesktop.DBus.Debug.Stats", "GetAllMatchRules" );
    m_method_GetMachineId = this->create_method<std::string()>( "org.freedesktop.DBus.Peer", "GetMachineId" );
    m_method_Ping = this->create_method<void()>( "org.freedesktop.DBus.Peer", "Ping" );

}
std::shared_ptr<DBusDaemonProxy> DBusDaemonProxy::create( std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path, DBus::ThreadForCalling signalCallingThread ) {
    return std::shared_ptr<DBusDaemonProxy>( new DBusDaemonProxy( conn, dest, path, signalCallingThread ) );

}
std::string DBusDaemonProxy::Hello( ) {
    return ( *m_method_Hello )();

}
uint32_t DBusDaemonProxy::RequestName( std::string arg0, uint32_t arg1 ) {
    return ( *m_method_RequestName )( arg0, arg1 );

}
uint32_t DBusDaemonProxy::ReleaseName( std::string arg0 ) {
    return ( *m_method_ReleaseName )( arg0 );

}
uint32_t DBusDaemonProxy::StartServiceByName( std::string arg0, uint32_t arg1 ) {
    return ( *m_method_StartServiceByName )( arg0, arg1 );

}
void DBusDaemonProxy::UpdateActivationEnvironment( std::map<std::string, std::string> arg0 ) {
    ( *m_method_UpdateActivationEnvironment )( arg0 );

}
bool DBusDaemonProxy::NameHasOwner( std::string arg0 ) {
    return ( *m_method_NameHasOwner )( arg0 );

}
std::vector<std::string> DBusDaemonProxy::ListNames( ) {
    return ( *m_method_ListNames )();

}
std::vector<std::string> DBusDaemonProxy::ListActivatableNames( ) {
    return ( *m_method_ListActivatableNames )();

}
void DBusDaemonProxy::AddMatch( std::string arg0 ) {
    ( *m_method_AddMatch )( arg0 );

}
void DBusDaemonProxy::RemoveMatch( std::string arg0 ) {
    ( *m_method_RemoveMatch )( arg0 );

}
std::string DBusDaemonProxy::GetNameOwner( std::string arg0 ) {
    return ( *m_method_GetNameOwner )( arg0 );

}
std::vector<std::string> DBusDaemonProxy::ListQueuedOwners( std::string arg0 ) {
    return ( *m_method_ListQueuedOwners )( arg0 );

}
uint32_t DBusDaemonProxy::GetConnectionUnixUser( std::string arg0 ) {
    return ( *m_method_GetConnectionUnixUser )( arg0 );

}
uint32_t DBusDaemonProxy::GetConnectionUnixProcessID( std::string arg0 ) {
    return ( *m_method_GetConnectionUnixProcessID )( arg0 );

}
std::vector<uint8_t> DBusDaemonProxy::GetAdtAuditSessionData( std::string arg0 ) {
    return ( *m_method_GetAdtAuditSessionData )( arg0 );

}
std::vector<uint8_t> DBusDaemonProxy::GetConnectionSELinuxSecurityContext( std::string arg0 ) {
    return ( *m_method_GetConnectionSELinuxSecurityContext )( arg0 );

}
void DBusDaemonProxy::ReloadConfig( ) {
    ( *m_method_ReloadConfig )();

}
std::string DBusDaemonProxy::GetId( ) {
    return ( *m_method_GetId )();

}
std::map<std::string, DBus::Variant> DBusDaemonProxy::GetConnectionCredentials( std::string arg0 ) {
    return ( *m_method_GetConnectionCredentials )( arg0 );

}
std::shared_ptr<DBus::SignalProxy<void(std::string, std::string, std::string)>> DBusDaemonProxy::signal_NameOwnerChanged( ) {
    return m_signalproxy_NameOwnerChanged;

}
std::shared_ptr<DBus::SignalProxy<void(std::string)>> DBusDaemonProxy::signal_NameLost( ) {
    return m_signalproxy_NameLost;

}
std::shared_ptr<DBus::SignalProxy<void(std::string)>> DBusDaemonProxy::signal_NameAcquired( ) {
    return m_signalproxy_NameAcquired;

}
DBus::Variant DBusDaemonProxy::Get( std::string arg0, std::string arg1 ) {
    return ( *m_method_Get )( arg0, arg1 );

}
std::map<std::string, DBus::Variant> DBusDaemonProxy::GetAll( std::string arg0 ) {
    return ( *m_method_GetAll )( arg0 );

}
void DBusDaemonProxy::Set( std::string arg0, std::string arg1, DBus::Variant arg2 ) {
    ( *m_method_Set )( arg0, arg1, arg2 );

}
std::shared_ptr<DBus::SignalProxy<void(std::string, std::map<std::string, DBus::Variant>, std::vector<std::string>)>> DBusDaemonProxy::signal_PropertiesChanged( ) {
    return m_signalproxy_PropertiesChanged;

}
std::string DBusDaemonProxy::Introspect( ) {
    return ( *m_method_Introspect )();

}
void DBusDaemonProxy::BecomeMonitor( std::vector<std::string> arg0, uint32_t arg1 ) {
    ( *m_method_BecomeMonitor )( arg0, arg1 );

}
std::map<std::string, DBus::Variant> DBusDaemonProxy::GetStats( ) {
    return ( *m_method_GetStats )();

}
std::map<std::string, DBus::Variant> DBusDaemonProxy::GetConnectionStats( std::string arg0 ) {
    return ( *m_method_GetConnectionStats )( arg0 );

}
std::map<std::string, std::vector<std::string>> DBusDaemonProxy::GetAllMatchRules( ) {
    return ( *m_method_GetAllMatchRules )();

}
std::string DBusDaemonProxy::GetMachineId( ) {
    return ( *m_method_GetMachineId )();

}
void DBusDaemonProxy::Ping( ) {
    ( *m_method_Ping )();

}
