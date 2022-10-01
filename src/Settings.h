#pragma once

class Settings
{
public:
    [[nodiscard]] static Settings* GetSingleton()
	{
		static Settings singleton;
		return std::addressof(singleton);
	}

	bool LoadSettings();

    [[nodiscard]] bool ShowNotification() const;
	[[nodiscard]] bool PlayNotificationSound() const;

	[[nodiscard]] float GetSkillXP() const;

    [[nodiscard]] float GetSneakAttackMult(const RE::BGSProjectile* a_projectile) const;
	[[nodiscard]] bool GetSpellValid(const RE::MagicItem* a_spell) const;

private:
	bool disableNotification{ false };
	bool disableSound{ false };

	float fSneakAttackSkillUsageMagic{ 2.5f };

	float fCombatSneakMissileMult{ 2.0f };
	float fCombatSneakGrenadeMult{ 2.0f };
	float fCombatSneakBeamMult{ 2.0f };
	float fCombatSneakFlamesMult{ 2.0f };
	float fCombatSneakConeMult{ 2.0f };
	float fCombatSneakBarrierMult{ 2.0f };

	bool alteration{ true };
	bool conjuration{ true };
	bool destruction{ true };
	bool illusion{ true };
	bool restoration{ true };
	bool enchanting{ true };
	bool other{ false };
};
