#include "Settings.h"

bool Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_MagicSneakAttacks.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	const auto get_value = [&]<class T>(T& a_value, const char* a_section, const char* a_key, const char* a_comment) {
		if constexpr (std::is_same_v<bool, T>) {
			a_value = ini.GetBoolValue(a_section, a_key, a_value);
			ini.SetBoolValue(a_section, a_key, a_value, a_comment);
		} else {
			a_value = static_cast<float>(ini.GetDoubleValue(a_section, a_key, a_value));
			ini.SetDoubleValue(a_section, a_key, a_value, a_comment);
		}
	};

	get_value(disableNotification, "Settings", "Hide Notifications", nullptr);
	get_value(disableSound, "Settings", "Hide Notification Sound", nullptr);
	get_value(fSneakAttackSkillUsageMagic, "Settings", "fSneakAttackSkillUsageMagic", ";Skill XP gained for each magic sneak attack");

	get_value(fCombatSneakMissileMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakMissileMult", ";Missiles");
	get_value(fCombatSneakGrenadeMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakLobberMult", ";Runes");
	get_value(fCombatSneakBeamMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakBeamMult", ";Lightning");
	get_value(fCombatSneakFlamesMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakFlamesMult", ";eg.Flames");
	get_value(fCombatSneakConeMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakConeMult", ";Shouts");
	get_value(fCombatSneakBarrierMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakBarrierMult", ";Wall spells");

	get_value(alteration, "Valid Spell Schools", "Alteration", ";Spells belonging to these schools can trigger sneak attacks");
	get_value(conjuration, "Valid Spell Schools", "Conjuration", nullptr);
	get_value(destruction, "Valid Spell Schools", "Destruction", nullptr);
	get_value(illusion, "Valid Spell Schools", "Illusion", nullptr);
	get_value(restoration, "Valid Spell Schools", "Restoration", nullptr);
	get_value(enchanting, "Valid Spell Schools", "Enchantments", nullptr);
	get_value(other, "Valid Spell Schools", "Other", nullptr);

	(void)ini.SaveFile(path);

	return true;
}

bool Settings::ShowNotification() const
{
	return !disableNotification;
}

bool Settings::PlayNotificationSound() const
{
	return !disableSound;
}

float Settings::GetSkillXP() const
{
	return fSneakAttackSkillUsageMagic;
}

float Settings::GetSneakAttackMult(const RE::BGSProjectile* a_projectile) const
{
	using Type = RE::BGSProjectileData::Type;

	if (a_projectile->data.types.all(Type::kMissile)) {
		return fCombatSneakMissileMult;
	}
	if (a_projectile->data.types.all(Type::kGrenade)) {
		return fCombatSneakGrenadeMult;
	}
	if (a_projectile->data.types.all(Type::kBeam)) {
		return fCombatSneakBeamMult;
	}
	if (a_projectile->data.types.all(Type::kFlamethrower)) {
		return fCombatSneakFlamesMult;
	}
	if (a_projectile->data.types.all(Type::kCone)) {
		return fCombatSneakConeMult;
	}
	if (a_projectile->data.types.all(Type::kBarrier)) {
		return fCombatSneakBarrierMult;
	}

	return 0.0f;
}

bool Settings::GetSpellValid(const RE::MagicItem* a_spell) const
{
	if (a_spell->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect) {
		return false;
	}
	if (a_spell->GetDelivery() == RE::MagicSystem::Delivery::kSelf) {
		return false;
	}

	switch (a_spell->GetAssociatedSkill()) {
	case RE::ActorValue::kAlteration:
		return alteration;
	case RE::ActorValue::kConjuration:
		return conjuration;
	case RE::ActorValue::kDestruction:
		return destruction;
	case RE::ActorValue::kIllusion:
		return illusion;
	case RE::ActorValue::kRestoration:
		return restoration;
	case RE::ActorValue::kEnchanting:
		return enchanting && a_spell->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment;
	default:
		return other;
	}
}
