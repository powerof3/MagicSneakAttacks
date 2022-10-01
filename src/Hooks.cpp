#include "Hooks.h"
#include "Settings.h"

namespace MagicSneakAttacks
{
	struct detail
	{
		static RE::Setting* get_gmst(const char* a_setting)
		{
			return RE::GameSettingCollection::GetSingleton()->GetSetting(a_setting);
		}

		static float get_sneak_attack_mult(RE::Actor* a_attacker, RE::Actor* a_target, RE::MagicItem* a_spell, const RE::BGSProjectile* a_projectile)
		{
			if (a_attacker->IsPlayerRef() && !a_target->IsDead() && a_attacker != a_target) {
				if (a_attacker->IsSneaking() && a_target->RequestDetectionLevel(a_attacker) <= 0) {
					const auto settings = Settings::GetSingleton();

					if (settings->GetSpellValid(a_spell)) {
						float sneakAttackMult = Settings::GetSingleton()->GetSneakAttackMult(a_projectile);
						RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSneakAttackMult, a_attacker, a_spell, a_target, std::addressof(sneakAttackMult));
						return sneakAttackMult;
					}
				}
			}

			return 0.0f;
		}
	};

	struct AdjustActiveEffect
	{
		static void thunk(RE::ActiveEffect* a_this, float a_power, bool a_arg3)
		{
			func(a_this, a_power, a_arg3);

			const auto attacker = a_this->GetCasterActor();
			const auto target = a_this->GetTargetActor();
			const auto effect = a_this->GetBaseObject();
			const auto spell = a_this->spell;

			if (attacker && target && spell && effect && effect->IsHostile()) {
				if (const auto projectile = effect->data.projectileBase; projectile) {
					if (const auto sneakAttackMult = detail::get_sneak_attack_mult(attacker.get(), target, spell, projectile); sneakAttackMult > 1.0f) {
						a_this->magnitude *= sneakAttackMult;
					}
				}
			}
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct SendHitEvent
	{
		static void thunk(RE::BSTEventSource<RE::TESHitEvent>& a_source, RE::TESHitEvent& a_event)
		{
			using HitFlag = RE::TESHitEvent::Flag;

			const auto aggressor = a_event.cause.get();
			const auto target = a_event.target.get();
			const auto source = RE::TESForm::LookupByID(a_event.source);
			const auto projectile = RE::TESForm::LookupByID<RE::BGSProjectile>(a_event.projectile);

			if (aggressor && target && source && projectile) {
				const auto aggressorActor = aggressor->As<RE::Actor>();
				const auto targetActor = target->As<RE::Actor>();

				auto spell = source->As<RE::MagicItem>();
				if (!spell) {
					const auto weapon = source->As<RE::TESObjectWEAP>();
					if (weapon && weapon->IsStaff()) {
						spell = weapon->formEnchanting;
					}
				}

				if (aggressorActor && targetActor && spell) {
					if (const auto effectItem = spell->GetCostliestEffectItem(); effectItem && effectItem->IsHostile()) {
						if (const auto sneakAttackMult = detail::get_sneak_attack_mult(aggressorActor, targetActor, spell, projectile); sneakAttackMult > 1.0f) {
							const auto settings = Settings::GetSingleton();

							a_event.flags.set(HitFlag::kSneakAttack);

							aggressorActor->UseSkill(RE::ActorValue::kSneak, settings->GetSkillXP(), spell);

							if (settings->ShowNotification()) {
								const auto sneakMessage = fmt::format("{}{:.1f}{}",
									detail::get_gmst("sSuccessfulSneakAttackMain")->GetString(),
									sneakAttackMult,
									detail::get_gmst("sSuccessfulSneakAttackEnd")->GetString());

								RE::DebugNotification(sneakMessage.c_str(), settings->PlayNotificationSound() ? "UISneakAttack" : nullptr);
							}

							const RE::CriticalHit::Event event{ aggressorActor, nullptr, true };
							RE::CriticalHit::GetEventSource()->SendEvent(&event);
						}
					}
				}
			}

			func(a_source, a_event);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void Install()
	{
		Settings::GetSingleton()->LoadSettings();

		REL::Relocation<std::uintptr_t> check_add_effect{ RELOCATION_ID(33763, 34547), OFFSET_3(0x4A3, 0x656, 0x427) };  // MagicTarget::CheckAddEffect
		stl::write_thunk_call<AdjustActiveEffect>(check_add_effect.address());

		logger::info("Hooked Active Effect Adjust"sv);

		REL::Relocation<std::uintptr_t> add_target{ RELOCATION_ID(37832, 38786), OFFSET(0x1C3, 0x29B) };  // MagicTarget::AddTarget
		stl::write_thunk_call<SendHitEvent>(add_target.address());

		logger::info("Hooked Magic Hit"sv);
	}
}
