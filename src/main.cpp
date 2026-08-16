#include <ninxout.options_api/include/API.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

bool enabled = false;

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
	return gjbgl && !gjbgl->m_isPlatformer;
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
	if (!ui) return;
	togglePlatformerControls(ui, true);
}

void disablePlatformerControls(UILayer* ui) {
	if (!ui) return;
	togglePlatformerControls(ui, false);
}

$on_game(Loaded) {
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	listenForSettingChanges<bool>("enabled", [](const bool enabledNew) {
		enabled = enabledNew;
		if (GJBaseGameLayer* gjbgl = GJBaseGameLayer::get(); shouldBotherWithThis(gjbgl)) {
			togglePlatformerControls(gjbgl->m_uiLayer, enabled);
		}
	});
	ADD_TOGGLE("Force Platformer Buttons", "enabled", "If playing a Classic level, forcibly toggle Platformer controls.")
}

class $modify(MyUILayer, UILayer) {
	void togglePlatformerMode(bool inPlatformer) {
		if (enabled && shouldBotherWithThis(this->m_gameLayer)) {
			enablePlatformerControls(this);
		} else {
			UILayer::togglePlatformerMode(inPlatformer);
		}
	}
};