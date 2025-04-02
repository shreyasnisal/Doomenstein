#pragma once

struct ActorUID
{
public:
	~ActorUID() = default;
	ActorUID() = default;
	ActorUID(unsigned int salt, unsigned int index);

	bool IsValid() const;
	unsigned int GetIndex() const;
	bool operator==(ActorUID const& otherUID) const;
	bool operator!=(ActorUID const& otherUID) const;

	static const ActorUID INVALID;

private:
	unsigned int m_data = 0xFFFFFFFF;
};

inline const ActorUID ActorUID::INVALID = ActorUID(0x0000FFFF, 0x0000FFFF);
