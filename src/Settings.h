#pragma once

class Settings : public REX::TSingleton<Settings>
{
public:
	bool LoadSettings();

	[[nodiscard]] bool ShowNotification() const { return !disableNotification; }
	[[nodiscard]] bool PlayNotificationSound() const { return !disableSound; }

	[[nodiscard]] float GetSkillXP() const { return fSneakAttackSkillUsageMagic.GetValue(); }

	[[nodiscard]] float GetSneakAttackMult(const RE::BGSProjectile* a_projectile) const;
	[[nodiscard]] bool  GetSpellValid(RE::MagicItem* a_spell) const;

private:
	bool IsSkillValid(RE::MagicItem* a_spell) const;

	// members
	static constexpr auto path = R"(Data\SKSE\Plugins\po3_MagicSneakAttacks.ini)"sv;

	REX::TIniSetting<bool>  disableNotification{ "Settings", "bHideNotifications", false };
	REX::TIniSetting<bool>  disableSound{ "Settings", "bHideNotificationSound", false };
	REX::TIniSetting<float> fSneakAttackSkillUsageMagic{ "Settings", "fSneakAttackSkillUsageMagic", 2.5f };

	REX::TIniSetting<float> fCombatSneakMissileMult{ "SneakAttackBaseMultipliers", "fCombatSneakMissileMult", 2.0f };
	REX::TIniSetting<float> fCombatSneakGrenadeMult{ "SneakAttackBaseMultipliers", "fCombatSneakLobberMult", 2.0f };
	REX::TIniSetting<float> fCombatSneakBeamMult{ "SneakAttackBaseMultipliers", "fCombatSneakBeamMult", 2.0f };
	REX::TIniSetting<float> fCombatSneakFlamesMult{ "SneakAttackBaseMultipliers", "fCombatSneakFlamesMult", 2.0f };
	REX::TIniSetting<float> fCombatSneakConeMult{ "SneakAttackBaseMultipliers", "fCombatSneakConeMult", 2.0f };
	REX::TIniSetting<float> fCombatSneakBarrierMult{ "SneakAttackBaseMultipliers", "fCombatSneakBarrierMult", 2.0f };

	REX::TIniSetting<bool> alteration{ "SpellSchools", "bAlteration", true };
	REX::TIniSetting<bool> conjuration{ "SpellSchools", "bConjuration", true };
	REX::TIniSetting<bool> destruction{ "SpellSchools", "bDestruction", true };
	REX::TIniSetting<bool> illusion{ "SpellSchools", "bIllusion", true };
	REX::TIniSetting<bool> restoration{ "SpellSchools", "bRestoration", true };
	REX::TIniSetting<bool> other{ "SpellSchools", "bOther", false };

	REX::TIniSetting<bool> staves{ "SpellTypes", "bStaves", true };
	REX::TIniSetting<bool> scrolls{ "SpellTypes", "bScrolls", true };
};
