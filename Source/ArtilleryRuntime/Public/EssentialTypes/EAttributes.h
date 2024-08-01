#include "CoreMinimal.h"
#include "ConservedAttribute.h"

namespace Arty
{
	enum AttribKey
	{
		Speed,
		Health,
		MaxHealth,
		Shields,
		MaxShields,
		Ammo,
		Mana,
		MaxMana,
		JumpHeight,
		Damage,
		CooldownTicker
	};

	//MANA should always be granted in multiples of 10 since 10m/t is our standard recharge.
	constexpr AttribKey MANA = Arty::AttribKey::Mana;
	constexpr AttribKey DASH_CURRENCY = MANA;
	constexpr AttribKey MAX_MANA = Arty::AttribKey::MaxMana;
	typedef TSharedPtr<FConservedAttributeData> AttrPtr;
	typedef TMap<AttribKey, AttrPtr> AttributeMap;
	typedef TSharedPtr<AttributeMap> AttrMapPtr;
}
