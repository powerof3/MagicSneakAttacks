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
	get_value(other, "Valid Spell Schools", "Other", nullptr);

	get_value(staves, "Valid Spell Types", "Staves", nullptr);
	get_value(scrolls, "Valid Spell Types", "Scrolls", nullptr);

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

bool Settings::GetSpellValid(RE::MagicItem* a_spell) const
{
	using namespace RE::MagicSystem;

	if (a_spell->GetCastingType() == CastingType::kConstantEffect) {
		return false;
	}
	if (a_spell->GetDelivery() == Delivery::kSelf) {
		return false;
	}

	const auto is_skill_valid = [&](RE::ActorValue a_skill) {
		switch (a_skill) {
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
		default:
			return other;
		}
	};

	constexpr auto get_costliest_effect_skill = [](RE::MagicItem* a_spell) {
		const auto effect = a_spell->GetCostliestEffectItem();
		const auto baseEffect = effect ? effect->baseEffect : nullptr;

		return baseEffect ? baseEffect->GetMagickSkill() : RE::ActorValue::kNone;
	};

	bool result = false;

	switch (a_spell->GetSpellType()) {
	case SpellType::kScroll:
		result = scrolls && is_skill_valid(get_costliest_effect_skill(a_spell));
		break;
	case SpellType::kStaffEnchantment:
		result = staves && is_skill_valid(get_costliest_effect_skill(a_spell));
		break;
	default:
		result = is_skill_valid(a_spell->GetAssociatedSkill());
		break;
	}

	return result;
}
