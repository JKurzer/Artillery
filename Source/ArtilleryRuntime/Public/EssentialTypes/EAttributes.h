#pragma once
#include "CoreMinimal.h"
#include "ConservedAttribute.h"
#include <array>
#include "EAttributes.generated.h"

UENUM(BlueprintType, Blueprintable)
enum class E_AttribKey : uint8
{
	Speed,
	Health,
	MaxHealth,
	HealthRechargePerTick,
	Shields,
	MaxShields,
	ShieldsRechargePerTick,
	Ammo,
	Mana,
	MaxMana,
	ManaRechargePerTick,
	TicksTilJumpAvailable,
	JumpHeight,
	Damage,
	CooldownTicker
};
namespace Arty
{
	//gonna need to ditch the stupid attributes from gameplay or rework them
	//all because of the bloody Base value meaning that we might have 2 doubles per.
	//if you want a base, add a base.
	//Attributes is a UE namespace, so we gotta call this attributeslist. sigh.

	namespace AttributesList{

	}
	//MANA should always be granted in multiples of 10 since 10m/t is our standard recharge.
	using AttribKey = E_AttribKey;
	using Attr = AttribKey;
	constexpr AttribKey HEALTH = Arty::AttribKey::Health;
	constexpr AttribKey MAX_HEALTH = Arty::AttribKey::MaxHealth;
	constexpr AttribKey MANA = Arty::AttribKey::Mana;
	constexpr AttribKey DASH_CURRENCY = MANA;
	constexpr AttribKey MAX_MANA = Arty::AttribKey::MaxMana;
	typedef TSharedPtr<FConservedAttributeData> AttrPtr;
	typedef TMap<AttribKey, AttrPtr> AttributeMap;
	typedef TSharedPtr<AttributeMap> AttrMapPtr;


}
