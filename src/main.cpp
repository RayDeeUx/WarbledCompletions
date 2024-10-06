#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/utils/web.hpp>
#include "../clip/clip.h"

#define PREFERRED_HOOK_PRIO (-2123456789)

using namespace geode::prelude;

class $modify(SharingEndLevelLayer, EndLevelLayer) {
	static void onModify(auto& self) {
		(void) self.setHookPriority("EndLevelLayer::customSetup", PREFERRED_HOOK_PRIO);
	}
	bool getBool(const std::string_view key) {
		return Mod::get()->getSettingValue<bool>(key);
	}
	void shareCompletionTo(std::string_view mode) {
		GJGameLevel* level = this->m_playLayer->m_level;
		std::string pluralOrNot = "";
		std::string levelName = level->m_levelName;
		std::string creatorName = level->m_creatorName;
		int attempts = level->m_attempts.value();
		if (attempts != 1) pluralOrNot = "s";
		int levelID = level->m_levelID.value();
		for (int i = 0; i < levelName.length(); i++) {
			if (levelName[i] == ' ') {
				if (mode == "twitter") levelName.replace(i, 1, "%20");
				else if (mode == "reddit") levelName.replace(i, 1, "+");
			}
		}
		if (mode == "twitter") {
			web::openLinkInBrowser(fmt::format("https://twitter.com/intent/tweet?text=I%20just%20completed%20{}%20by%20{}%20in%20{}%20attempt{}%21%20If%20you%20want%20to%20try%20it,%20the%20ID%20is%20{}.",
				levelName,
				creatorName,
				attempts,
				pluralOrNot,
				levelID
			));
		} else if (mode == "reddit") {
			if (!getBool("useSHReddit")) {
				web::openLinkInBrowser(fmt::format("https://new.reddit.com/r/geometrydash/submit?title=I+just+completed+{}+in+{}+attempt{}!+ID:+{}&selftext=true&text=Hey+there!%0AYou+should+click+on+the+%22Images+%26+Video%22+tab+to+attach+your+video+recording+of+your+level+completion+so+your+post+follows+r%2Fgeometrydash+rules.%0APosting+this+text+alone+will+get+your+post+auto-removed.+Thanks!%0A--RayDeeUx%2C+in+cooperation+with+r%2Fgeometrydash+staff",
					levelName,
					attempts,
					pluralOrNot,
					levelID
				));
			} else {
				web::openLinkInBrowser(fmt::format("https://sh.reddit.com/r/geometrydash/submit/?title=I+just+completed+{}+in+{}+attempt{}!+ID:+{}&type=IMAGE",
					levelName,
					attempts,
					pluralOrNot,
					levelID
				));
			}
		}
	}
	void customSetup() {
		EndLevelLayer::customSetup();
		if (!getBool("enabled")) return;
		if (!m_playLayer || !m_playLayer->m_level) return;
		if (!m_mainLayer) return;
		auto background = m_mainLayer->getChildByIDRecursive("background");
		if (!background) return;
		auto topBorder = background->getChildByIDRecursive("top-border");
		if (!topBorder) return;
		if (m_playLayer->m_isPracticeMode || m_playLayer->m_isTestMode) return;
		CCMenu* menu = CCMenu::create();
		RowLayout* layout = RowLayout::create();
		menu->setID("look-i-did-it-menu"_spr);
		menu->setContentHeight(32);
		menu->setContentWidth(background->getContentWidth());
		layout->setAutoScale(true)->setAxisAlignment(AxisAlignment::Center)->setAxis(Axis::Row);
		menu->setLayout(layout);
		m_mainLayer->addChild(menu);
		menu->setPositionY(convertToWorldSpace(topBorder->getPosition()).y + 43);
		if (getBool("twitter")) {
			auto tweetButton = CCMenuItemSpriteExtra::create(
			   CCSprite::createWithSpriteFrameName("gj_twIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onTweet)
			);
			tweetButton->setID("tweet-button"_spr);
			menu->addChild(tweetButton);
			menu->updateLayout();
		}
		if (getBool("reddit") && m_playLayer->m_level->m_levelType != GJLevelType::Local) {
			auto redditButton = CCMenuItemSpriteExtra::create(
			   CCSprite::createWithSpriteFrameName("gj_rdIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onReddit)
			);
			redditButton->setID("reddit-button"_spr);
			menu->addChild(redditButton);
			menu->updateLayout();
		}
		if (menu->getChildrenCount() < 2) {
			menu->removeMeAndCleanup();
		}
	}
	void onTweet(CCObject*) {
		geode::createQuickPopup("TweetYourCompletions", "Would you like to Tweet this completion?", "No", "Yes", [=](auto, bool tweet) {
			if (tweet) {
				shareCompletionTo("twitter");
			}
		});
	}
	void onReddit(CCObject*) {
		geode::createQuickPopup("TweetYourCompletions", "Would you like to post this completion in <co>r/geometrydash</c>?\n\n<cy>Remember to include video/screenshot evidence of your completion!</c>", "No", "Yes", [=](auto, bool reddit) {
			if (reddit) {
				shareCompletionTo("reddit");
			}
		});
	}
};