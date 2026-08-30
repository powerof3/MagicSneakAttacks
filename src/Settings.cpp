#include "Settings.h"

bool Settings::LoadSettings()
{
	const auto store = REX::FIniSettingStore::GetSingleton();
	store->Init(path.data(), "");

	store->Load();
	store->Save();

	return true;
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
