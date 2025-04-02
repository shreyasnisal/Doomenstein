#include "Game/Gold/Dragon.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

Dragon::Dragon(Map* map, SpawnInfo const& spawnInfo, ActorUID uid)
	: Actor(map, spawnInfo, uid)
{

}

void Dragon::Render() const
{
}
