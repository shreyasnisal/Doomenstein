#include "Game/ActorUID.hpp"

constexpr unsigned int INDEX_BITFIELD = 0x0000FFFF;
constexpr unsigned int SALT_BITFIELD = 0xFFFF0000;

ActorUID::ActorUID(unsigned int salt, unsigned int index)
{
	m_data = ((salt << 16) & SALT_BITFIELD) | (index & INDEX_BITFIELD);
}

bool ActorUID::IsValid() const
{
	return *this != INVALID;
}

unsigned int ActorUID::GetIndex() const
{
	return m_data & INDEX_BITFIELD;
}

bool ActorUID::operator==(ActorUID const& otherUID) const
{
	return m_data == otherUID.m_data;
}

bool ActorUID::operator!=(ActorUID const& otherUID) const
{
	return m_data != otherUID.m_data;
}
