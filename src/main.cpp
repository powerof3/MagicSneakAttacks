#include "Settings.h"

namespace Hit::Magic
{
	struct detail
	{
		static RE::Setting* get_gmst(const char* a_setting)
		{
			const auto static gmst = RE::GameSettingCollection::GetSingleton();
			return gmst->GetSetting(a_setting);
		}

		static float get_sneak_attack_mult(RE::Actor* a_attacker, RE::Actor* a_target, RE::MagicItem* a_spell, RE::BGSProjectile* a_projectile)
		{
			if (a_attacker->IsPlayerRef() && !a_target->IsDead() && a_attacker != a_target) {
				if (a_attacker->IsSneaking() && a_target->RequestDetectionLevel(a_attacker) <= 0) {
					const auto settings = Settings::GetSingleton();

				    if (settings->GetSpellValid(a_spell)) {
						float sneakAttackMult = Settings::GetSingleton()->GetSneakAttackMult(a_projectile);
						if (sneakAttackMult > 1.0f) {
							RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSneakAttackMult, a_attacker, a_spell, a_target, &sneakAttackMult);
							return sneakAttackMult;
						}
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

		static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(33763, 34547), OFFSET(0x4A3, 0x656) };
			stl::write_thunk_call<AdjustActiveEffect>(target.address());

			logger::info("Hooked Active Effect Adjust"sv);
		}
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
				const auto spell = source->As<RE::MagicItem>();

				if (aggressorActor && targetActor && spell) {
                    if (const auto effectItem = spell->GetCostliestEffectItem(); effectItem && effectItem->IsHostile()) {
						if (const auto sneakAttackMult = detail::get_sneak_attack_mult(aggressorActor, targetActor, spell, projectile); sneakAttackMult > 1.0f) {
							const auto settings = Settings::GetSingleton();

							a_event.flags.set(HitFlag::kSneakAttack);

							aggressorActor->UseSkill(RE::ActorValue::kSneak, settings->GetSkillXP(), source);

							if (settings->ShowNotification()) {
								const auto sneakMessage = fmt::format("{}{:.1f}{}", detail::get_gmst("sSuccessfulSneakAttackMain")->GetString(), sneakAttackMult, detail::get_gmst("sSuccessfulSneakAttackEnd")->GetString());
								RE::DebugNotification(sneakMessage.c_str(), settings->ShowNotificationSound() ? "UISneakAttack" : nullptr);
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

		static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37832, 38786), OFFSET(0x1C3, 0x29B) };
			stl::write_thunk_call<SendHitEvent>(target.address());

			logger::info("Hooked Magic Hit"sv);
		}
	};

	void Install()
	{
		AdjustActiveEffect::Install();
		SendHitEvent::Install();
	}
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("Sneaky Spell Attacks");
	v.AuthorName("powerofthree");
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Sneaky Spell Attacks";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver
#	ifndef SKYRIMVR
		< SKSE::RUNTIME_1_5_39
#	else
		> SKSE::RUNTIME_VR_1_4_15_1
#	endif
	) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("loaded");

	SKSE::Init(a_skse);

	Settings::GetSingleton()->LoadSettings();

	Hit::Magic::Install();

	return true;
}
