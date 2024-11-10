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
	bool isDisabled(const std::string_view key) {
		return !getBool(key) || !getBool("enabled");
	}
	std::string getString(const std::string_view key) {
		return Mod::get()->getSettingValue<std::string>(key);
	}
	void shareCompletionTo(std::string_view mode) {
		GJGameLevel* level = this->m_playLayer->m_level;
		std::string levelName = level->m_levelName;
		std::string creatorName = level->m_creatorName;
		int attempts = level->m_attempts.value();
		int levelID = level->m_levelID.value();
		std::string pluralOrNot = attempts != 1 ? "s" : "";
		for (int i = 0; i < levelName.length(); i++) {
			if (levelName[i] == ' ') {
				if (mode == "twitter" || mode == "bluesky") levelName.replace(i, 1, "%20");
				else levelName.replace(i, 1, "+");
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
		} else if (mode == "bluesky") {
			web::openLinkInBrowser(fmt::format("https://bsky.app/intent/compose?text=I%20just%20completed%20{}%20by%20{}%20in%20{}%20attempt{}%21%20If%20you%20want%20to%20try%20it,%20the%20ID%20is%20{}.",
				levelName,
				creatorName,
				attempts,
				pluralOrNot,
				levelID
			));
		} else if (mode == "mastodon") {
			web::openLinkInBrowser(fmt::format("https://{}/share?text=I+just+completed+{}+by+{}+in+{}+attempt{}%21+If+you+want+to+try+it,+the+ID+is+{}.",
				getString("mastodonInstance"),
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
	void addWeb(CCMenu *menu) {
		if (getString("customURL").empty() || !getBool("enabled")) return;
		auto webButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("web.png"_spr), this, menu_selector(SharingEndLevelLayer::onWeb));
		webButton->setID("web-button"_spr);
		menu->addChild(webButton);
		menu->updateLayout();
	}
	void addMastodon(CCMenu *menu) {
		if (isDisabled("mastodon")) return;
		auto mastodonButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("mastodon.png"_spr), this, menu_selector(SharingEndLevelLayer::onMastodon));
		mastodonButton->setID("mastodon-button"_spr);
		menu->addChild(mastodonButton);
		menu->updateLayout();
	}
	void addBluesky(CCMenu *menu) {
		if (isDisabled("bluesky")) return;
		CCSprite* blueskySprite = CCSprite::createWithSpriteFrameName("blueskyAlt.png"_spr);
		const std::string &blueskyStyle = getString("blueskyStyle");
		if (blueskyStyle == "Media Kit") blueskySprite = CCSprite::createWithSpriteFrameName("bluesky.png"_spr);
		else if (blueskyStyle == "Alphalaneous") blueskySprite = CCSprite::createWithSpriteFrameName("blueskyAlpha.png"_spr);
		else if (blueskyStyle == "Colon") blueskySprite = CCSprite::createWithSpriteFrameName("blueskyColon.png"_spr);
		auto blueskyButton = CCMenuItemSpriteExtra::create(blueskySprite, this, menu_selector(SharingEndLevelLayer::onBluesky));
		blueskyButton->setID("bluesky-button"_spr);
		menu->addChild(blueskyButton);
		menu->updateLayout();
	}
	void addRedditIfNotRobTopLevel(CCMenu *menu) {
		if (isDisabled("reddit") || m_playLayer->m_level->m_levelType == GJLevelType::Local) return;
		auto redditButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("gj_rdIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onReddit));
		redditButton->setID("reddit-button"_spr);
		menu->addChild(redditButton);
		menu->updateLayout();
	}
	void addTwitter(CCMenu *menu) {
		if (isDisabled("twitter")) return;
		auto tweetButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("gj_twIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onTweet));
		tweetButton->setID("tweet-button"_spr);
		menu->addChild(tweetButton);
		menu->updateLayout();
	}
	void onTweet(CCObject*) {
		if (isDisabled("twitter")) return;
		geode::createQuickPopup("WarbledCompletions", "Would you like to <cj>Tweet</c> this completion?", "No", "Yes", [=](auto, bool tweet) {
			if (tweet) shareCompletionTo("twitter");
		});
	}
	void onBluesky(CCObject*) {
		if (isDisabled("bluesky")) return;
		geode::createQuickPopup("WarbledCompletions", "Would you like to post this completion to <cl>Bluesky</c>?", "No", "Yes", [=](auto, bool bluesky) {
			if (bluesky) shareCompletionTo("bluesky");
		});
	}
	void onMastodon(CCObject*) {
		if (isDisabled("mastodon")) return;
		geode::createQuickPopup("WarbledCompletions", fmt::format("Would you like to post this completion in <ca>{}</c>?", getString("mastodonInstance")), "No", "Yes", [=](auto, bool mastodon) {
			if (mastodon) shareCompletionTo("mastodon");
		});
	}
	void onReddit(CCObject*) {
		if (isDisabled("reddit") || m_playLayer->m_level->m_levelType == GJLevelType::Local) return;
		geode::createQuickPopup("WarbledCompletions", "Would you like to post this completion in <co>r/geometrydash</c>?\n\n<cy>Remember to include video/screenshot evidence of your completion!</c>", "No", "Yes", [=](auto, bool reddit) {
			if (reddit) shareCompletionTo("reddit");
		});
	}
	void onWeb(CCObject*) {
		if (getString("customURL").empty() || !getBool("enabled")) return;
		geode::createQuickPopup("WarbledCompletions", fmt::format("Would you like to share your completion <cb>elsewhere</c>?\n\n<cy>If you choose this option, you are responsible for the contents of the web page you chose:</c>\n\n<cl>{}</c>", getString("customURL")), "No", "Yes", [=](auto, bool web) {
			if (!web) return;
			web::openLinkInBrowser(fmt::format("https://{}", getString("customURL")));
		});
	}
	void customSetup() {
		EndLevelLayer::customSetup();
		if (!getBool("enabled")) return;
		if (!m_playLayer || !m_playLayer->m_level || m_playLayer->m_isPracticeMode || m_playLayer->m_isTestMode || !m_mainLayer) return;
		auto background = m_mainLayer->getChildByIDRecursive("background");
		if (!background) return;
		auto topBorder = background->getChildByIDRecursive("top-border");
		if (!topBorder) return;
		CCMenu* menu = CCMenu::create();
		RowLayout* layout = RowLayout::create();
		menu->setID("look-i-did-it-menu"_spr);
		menu->setContentHeight(32);
		menu->setContentWidth(background->getContentWidth());
		layout->setAutoScale(true)->setAxisAlignment(AxisAlignment::Center)->setAxis(Axis::Row)->setGap(5.f)->setAutoScale(true)->setCrossAxisOverflow(false)->setDefaultScaleLimits(1.f, 1.f);
		menu->setLayout(layout);
		m_mainLayer->addChild(menu);
		menu->setPositionY(convertToWorldSpace(topBorder->getPosition()).y + 43);
		addTwitter(menu);
		addRedditIfNotRobTopLevel(menu);
		addBluesky(menu);
		addWeb(menu);
		if (menu->getChildrenCount() < 2) menu->removeMeAndCleanup();
	}
};