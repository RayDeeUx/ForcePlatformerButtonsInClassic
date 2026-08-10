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

$on_mod(Loaded) {
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	listenForSettingChanges<bool>("enabled", [](const bool enabledNew) {
		enabled = enabledNew;
		GJBaseGameLayer* gjbgl = GJBaseGameLayer::get();
		if (!gjbgl || gjbgl->m_isPlatformer || !gjbgl->m_uiLayer) return;
		UILayer* ui = gjbgl->m_uiLayer;
		ui->m_inPlatformer = enabled;
		ui->refreshDpad();
		ui->updateUINodeVisibility(enabled);
	});
	ADD_TOGGLE("Force Platformer Buttons in Classic Mode", "enabled", "If playing a Classic level, forcibly toggle Platformer controls.")
}

class $modify(MyUILayer, UILayer) {
	void togglePlatformerMode(bool inPlatformer) {
		if (enabled && m_gameLayer && !m_gameLayer->m_isPlatformer) {
			inPlatformer = true;
			this->m_inPlatformer = true;
			UILayer::refreshDpad();
			UILayer::updateUINodeVisibility(inPlatformer);
		} else {
			UILayer::togglePlatformerMode(inPlatformer);
		}
	}
};