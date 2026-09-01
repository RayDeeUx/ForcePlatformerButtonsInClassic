#include <ninxout.options_api/include/API.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

bool enabled = false;
bool autoDetectEventTriggersInstead = false;
bool debugLabel = false;

#define ADD_TOGGLE(displayName, settingsID, detailedDesc)\
	OptionsAPI::addPreLevelSetting<bool>(\
		displayName,\
		settingsID""_spr,\
		[](GJGameLevel*) {\
			const bool origValue = Mod::get()->getSettingValue<bool>(settingsID);\
			Mod::get()->setSettingValue<bool>(settingsID, !origValue);\
		},\
		[](GJGameLevel*) {\
			return Mod::get()->getSettingValue<bool>(settingsID);\
		},\
		"<cl>(From ForcePlatformerButtonsInClassic by RayDeeUx)</c>\n" detailedDesc\
	);\
	OptionsAPI::addMidLevelSetting<bool>(\
		displayName,\
		settingsID""_spr,\
		[](GJBaseGameLayer*) {\
			const bool origValue = Mod::get()->getSettingValue<bool>(settingsID);\
			Mod::get()->setSettingValue<bool>(settingsID, !origValue);\
		},\
		[](GJBaseGameLayer*) {\
			return Mod::get()->getSettingValue<bool>(settingsID);\
		},\
		"<cl>(From ForcePlatformerButtonsInClassic by RayDeeUx)</c>\n" detailedDesc\
	);

bool shouldBotherWithThis(GJBaseGameLayer* gjbgl) {
	return gjbgl && !gjbgl->m_isPlatformer && !gjbgl->m_level->isPlatformer();
}

void togglePlatformerControls(UILayer* ui, const bool enabledFromParam) {
	#ifdef GEODE_IS_DESKTOP
	return log::info("ui exists: {}, enabledFromParam: {}", ui != nullptr, enabledFromParam);
	#else
	if (!ui) return;
	ui->m_inPlatformer = enabledFromParam;
	ui->refreshDpad();
	ui->updateUINodeVisibility(enabledFromParam);
	#endif
}

void enablePlatformerControls(UILayer* ui) {
	if (!ui || ui->m_inPlatformer) return;
	togglePlatformerControls(ui, true);
}

void disablePlatformerControls(UILayer* ui) {
	if (!ui || !ui->m_inPlatformer) return;
	togglePlatformerControls(ui, false);
}

void reactToSettingsChanges() {
	if (GJBaseGameLayer* gjbgl = GJBaseGameLayer::get(); gjbgl && shouldBotherWithThis(gjbgl) && gjbgl->m_uiLayer) {
		if (gjbgl->m_uiLayer->getChildByID("relevant-labels"_spr)) {
			gjbgl->m_uiLayer->getChildByID("relevant-labels"_spr)->setVisible(autoDetectEventTriggersInstead && debugLabel);
		}
		if (autoDetectEventTriggersInstead) return;
		enabled ? enablePlatformerControls(gjbgl->m_uiLayer) : disablePlatformerControls(gjbgl->m_uiLayer);
	}
}

$on_game(Loaded) {
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	autoDetectEventTriggersInstead = Mod::get()->getSettingValue<bool>("autoDetectEventTriggersInstead");
	debugLabel = Mod::get()->getSettingValue<bool>("debugLabel");
	listenForSettingChanges<bool>("enabled", [](const bool enabledNew) {
		enabled = enabledNew;
		reactToSettingsChanges();
	});
	listenForSettingChanges<bool>("autoDetectEventTriggersInstead", [](const bool autoDetectEventTriggersInsteadNew) {
		autoDetectEventTriggersInstead = autoDetectEventTriggersInsteadNew;
		reactToSettingsChanges();
	});
	listenForSettingChanges<bool>("debugLabel", [](const bool debugLabelNew) {
		debugLabel = debugLabelNew;
		reactToSettingsChanges();
	});
	ADD_TOGGLE("Force Platformer Buttons", "enabled", "If playing a Classic level, forcibly toggle Platformer controls.")
	ADD_TOGGLE("Auto-Detect Platformer Buttons", "autoDetectEventTriggersInstead", "Auto-detect Event triggers instead of manually toggling Platformer controls.")
	ADD_TOGGLE("Platformer Buttons Auto-Detection Debug label", "debugLabel", "If playing a Classic level with the \"Auto-Detect Platformer Buttons\" setting enabled, view debugging information for a sanity check.")
}

class $modify(MyUILayer, UILayer) {
	void togglePlatformerMode(bool inPlatformer) {
		if (enabled && !autoDetectEventTriggersInstead && shouldBotherWithThis(this->m_gameLayer)) {
			enablePlatformerControls(this);
		} else {
			UILayer::togglePlatformerMode(inPlatformer);
		}
	}
};

class $modify(GJBaseGameLayer) {
	void update(float dt) {
		GJBaseGameLayer::update(dt);
		if (!enabled || !autoDetectEventTriggersInstead || !shouldBotherWithThis(this)) return;

		geode::Label* stupid = nullptr;
		if (m_uiLayer && !m_uiLayer->getChildByID("relevant-labels"_spr)) {
			stupid = geode::Label::create("foobar", "chatFont.fnt");
			stupid->setID("relevant-labels"_spr);
			stupid->setTag(20260831);
			m_uiLayer->addChild(stupid);
			stupid->setAnchorPoint({1.f, 1.f});
			stupid->setLimitLabelSize({CCDirector::get()->getWinSize().width * .7f, CCDirector::get()->getWinSize().height * .25f});
			stupid->setPosition({CCDirector::get()->getWinSize().width * .98f, 80.f});
		} else {
			stupid = static_cast<geode::Label*>(m_uiLayer->getChildByID("relevant-labels"_spr));
		}

		std::string result = "";
		std::array<bool, 4> found{false, false, false, false};
		std::array<bool, 4> active{false, false, false, false};

		for (auto& [pairOfEventsAndUnkInts, triggers] : m_gameState.m_unkMapPairGJGameEventIntVectorEventTriggerInstance) {
			const int gameEvent = static_cast<int>(pairOfEventsAndUnkInts.first);
			if (gameEvent < 71 || gameEvent > 74) continue;

			const int idx = gameEvent - 71;
			found[idx] = true;

			for (auto trigger : triggers) {
				if (static_cast<EventTriggerInstance>(trigger).m_inactive) continue;;
				active[idx] = true;
				break;
			}
		}

		bool shouldNotBeInPlatformer = true;
		for (int i = 0; i < 4; ++i) {
			if (!found[i]) continue;
			if (shouldNotBeInPlatformer && active[i]) shouldNotBeInPlatformer = false;
			result.append(fmt::format("GJGameEvent {} is active: {}\n", 71 + i, active[i]));
		}

		shouldNotBeInPlatformer ? disablePlatformerControls(this->m_uiLayer) : enablePlatformerControls(this->m_uiLayer);
		stupid->setColor(shouldNotBeInPlatformer ? ccColor3B{255, 0, 0} : ccColor3B{0, 255, 0});
		stupid->setText(result);
		stupid->setVisible(debugLabel);
	}
};