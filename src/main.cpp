#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(SharingEndLevelLayer, EndLevelLayer) {
	void customSetup() {
		EndLevelLayer::customSetup();

		auto menu = CCMenu::create();
		menu->setID("look-i-did-it-menu");
		if (auto endText = this->m_mainLayer->getChildByIDRecursive("complete-message")) {
			menu->setPosition({
				endText->getPositionX() + 150.f, 
				endText->getPositionY() - 9.f
			});
		} else if (auto endText = this->m_mainLayer->getChildByIDRecursive("end-text")) {
			menu->setPosition({
				endText->getPositionX() + 150.f, 
				endText->getPositionY() - 2.f
			});
		}
		this->m_mainLayer->addChild(menu);
		
		auto tweetButton = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("gj_twIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onTweetButton)
		);
		menu->addChild(tweetButton);
	}

	void onTweetButton(CCObject*) {
		geode::createQuickPopup("Look I Did It", "Would you like to share this completion on Twitter?", "No", "Yes", [=](auto, bool btn2) {
			if (btn2) {
				auto levelName = this->m_playLayer->m_level->m_levelName;
				auto creatorName = this->m_playLayer->m_level->m_creatorName;
				auto attempts = this->m_playLayer->m_level->m_attempts.value();
				auto levelID = this->m_playLayer->m_level->m_levelID.value();
				for (int i = 0; i < levelName.length(); i++) {
					if (levelName[i] == ' ') {
						levelName.replace(i, 1, "%20");
					}
				}
				web::openLinkInBrowser(fmt::format("https://twitter.com/intent/tweet?text=I%20just%20completed%20{}%20by%20{}%20in%20{}%20attempts%21%20If%20you%20want%20to%20try%2C%20the%20level%20id%20is%20{}", 
				levelName, 
				creatorName, 
				attempts,
				levelID
				));
			}
		});
	}
};