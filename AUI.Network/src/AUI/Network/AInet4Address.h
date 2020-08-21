#pragma once

#include <cstdint>
#include <AUI/Common/AString.h>
#include <AUI/Network.h>

struct sockaddr_in;

class API_AUI_NETWORK AInet4Address
{
private:
	uint32_t mAddr;
	uint16_t mPort;

public:
	uint64_t toLong() const;
	uint32_t toLongAddressOnly() const;
	AInet4Address();
	AInet4Address(const sockaddr_in& other);
	AInet4Address(const AInet4Address& other);
	AInet4Address(uint8_t ip[4], uint16_t port);
	AInet4Address(uint32_t ip, uint16_t port);
	AInet4Address(const AString& addr, uint16_t port);
	
	sockaddr_in addr() const;
	bool operator>(const AInet4Address& r) const;
	bool operator<(const AInet4Address& r) const;
	bool operator==(const AInet4Address& o) const;
	
	AString toString() const;
};
