#include "Settings.h"

bool Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_MagicSneakAttacks.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	if (ini.GetSection("Valid Spell Types")) {
		ini::get_value(ini, disableNotification, "Settings", "Hide Notifications", nullptr);
		ini::get_value(ini, disableSound, "Settings", "Hide Notification Sound", nullptr);
		ini::get_value(ini, fSneakAttackSkillUsageMagic, "Settings", "fSneakAttackSkillUsageMagic", ";Skill XP gained for each magic sneak attack");

		ini::get_value(ini, fCombatSneakMissileMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakMissileMult", ";Missiles");
		ini::get_value(ini, fCombatSneakGrenadeMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakLobberMult", ";Runes");
		ini::get_value(ini, fCombatSneakBeamMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakBeamMult", ";Lightning");
		ini::get_value(ini, fCombatSneakFlamesMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakFlamesMult", ";eg.Flames");
		ini::get_value(ini, fCombatSneakConeMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakConeMult", ";Shouts");
		ini::get_value(ini, fCombatSneakBarrierMult, "Sneak Attack Base Multipliers (Projectile Type)", "fCombatSneakBarrierMult", ";Wall spells");

		ini::get_value(ini, alteration, "Valid Spell Schools", "Alteration", ";Spells belonging to these schools can trigger sneak attacks");
		ini::get_value(ini, conjuration, "Valid Spell Schools", "Conjuration", nullptr);
		ini::get_value(ini, destruction, "Valid Spell Schools", "Destruction", nullptr);
		ini::get_value(ini, illusion, "Valid Spell Schools", "Illusion", nullptr);
		ini::get_value(ini, restoration, "Valid Spell Schools", "Restoration", nullptr);
		ini::get_value(ini, other, "Valid Spell Schools", "Other", nullptr);

		ini::get_value(ini, staves, "Valid Spell Types", "Staves", nullptr);
		ini::get_value(ini, scrolls, "Valid Spell Types", "Scrolls", nullptr);

		ini.Delete("Settings", nullptr);
		ini.Delete("Sneak Attack Base Multipliers (Projectile Type)", nullptr);
		ini.Delete("Valid Spell Schools", nullptr);
		ini.Delete("Valid Spell Types", nullptr);
	}

	ini::get_value(ini, disableNotification, "Settings", "bHideNotifications", nullptr);
	ini::get_value(ini, disableSound, "Settings", "bHideNotificationSound", nullptr);
	ini::get_value(ini, fSneakAttackSkillUsageMagic, "Settings", "fSneakAttackSkillUsageMagic", ";Skill XP gained for each magic sneak attack");

	ini::get_value(ini, fCombatSneakMissileMult, "SneakAttackBaseMultipliers", "fCombatSneakMissileMult", ";Missiles");
	ini::get_value(ini, fCombatSneakGrenadeMult, "SneakAttackBaseMultipliers", "fCombatSneakLobberMult", ";Runes");
	ini::get_value(ini, fCombatSneakBeamMult, "SneakAttackBaseMultipliers", "fCombatSneakBeamMult", ";Lightning");
	ini::get_value(ini, fCombatSneakFlamesMult, "SneakAttackBaseMultipliers", "fCombatSneakFlamesMult", ";eg.Flames");
	ini::get_value(ini, fCombatSneakConeMult, "SneakAttackBaseMultipliers", "fCombatSneakConeMult", ";Shouts");
	ini::get_value(ini, fCombatSneakBarrierMult, "SneakAttackBaseMultipliers", "fCombatSneakBarrierMult", ";Wall spells");

	ini::get_value(ini, alteration, "SpellSchools", "bAlteration", ";Spells belonging to these schools can trigger sneak attacks");
	ini::get_value(ini, conjuration, "SpellSchools", "bConjuration", nullptr);
	ini::get_value(ini, destruction, "SpellSchools", "bDestruction", nullptr);
	ini::get_value(ini, illusion, "SpellSchools", "bIllusion", nullptr);
	ini::get_value(ini, restoration, "SpellSchools", "bRestoration", nullptr);
	ini::get_value(ini, other, "SpellSchools", "bOther", nullptr);

	ini::get_value(ini, staves, "SpellTypes", "bStaves", nullptr);
	ini::get_value(ini, scrolls, "SpellTypes", "bScrolls", nullptr);

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
	if (a_projectile->IsMissile()) {
		return fCombatSneakMissileMult;
	}
	if (a_projectile->IsGrenade()) {
		return fCombatSneakGrenadeMult;
	}
	if (a_projectile->IsBeam()) {
		return fCombatSneakBeamMult;
	}
	if (a_projectile->IsFlamethrower()) {
		return fCombatSneakFlamesMult;
	}
	if (a_projectile->IsCone()) {
		return fCombatSneakConeMult;
	}
	if (a_projectile->IsBarrier()) {
		return fCombatSneakBarrierMult;
	}

	return 0.0f;
}

bool Settings::GetSpellValid(RE::MagicItem* a_spell) const
{
	using namespace RE::MagicSystem;

	bool result = false;

	if (a_spell->GetCastingType() == CastingType::kConstantEffect || a_spell->HasKeywordString("MagicNoSneakAttack")) {
		return result;
	}

	switch (a_spell->GetSpellType()) {
	case SpellType::kScroll:
		result = scrolls && IsSkillValid(a_spell);
		break;
	case SpellType::kStaffEnchantment:
		result = staves && IsSkillValid(a_spell);
		break;
	default:
		result = IsSkillValid(a_spell);
		break;
	}

	return result;
}

bool Settings::IsSkillValid(RE::MagicItem* a_spell) const
{
	auto skill = a_spell->GetAssociatedSkill();
	if (a_spell->Is(RE::FormType::Scroll, RE::FormType::Enchantment)) {
		if (const auto effect = a_spell->GetCostliestEffectItem(); effect && effect->baseEffect) {
			skill = effect->baseEffect->GetMagickSkill();
		}
	}

	switch (skill) {
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
}
