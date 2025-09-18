#include <ninxout.prntscrn/include/api.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/utils/web.hpp>

#define PREFERRED_HOOK_PRIO (-3999)

using namespace geode::prelude;

const std::filesystem::path &configDirPath = Mod::get()->getConfigDir();
const std::string &configDir = configDirPath.string();

class $modify(SharingEndLevelLayer, EndLevelLayer) {
	struct Fields {
		std::string filePath = "";
	};
	static void onModify(auto& self) {
		(void) self.setHookPriority("EndLevelLayer::customSetup", PREFERRED_HOOK_PRIO);
	}
	static bool getBool(const std::string_view key) {
		return Mod::get()->getSettingValue<bool>(key);
	}
	static bool isDisabled(const std::string_view key) {
		return !getBool(key) || !getBool("enabled");
	}
	static std::string getString(const std::string_view key) {
		return Mod::get()->getSettingValue<std::string>(key);
	}
	static std::filesystem::path getPath(const std::string_view key) {
		return Mod::get()->getSettingValue<std::filesystem::path>(key);
	}
	void shareCompletionTo(std::string_view mode) {
		GJGameLevel* level = this->m_playLayer->m_level;
		bool isOwnLevel = level->m_levelID.value() == 0;
		std::string levelName = isOwnLevel ? "me" : level->m_levelName;
		std::string creatorName = level->m_creatorName;
		int attempts = level->m_attempts.value();
		std::string levelID = isOwnLevel ? "not available right now, but will be soon" : std::to_string(level->m_levelID.value());
		std::string completedOrVerified = isOwnLevel ? "verified" : "completed";
		std::string pluralOrNot = attempts != 1 ? "s" : "";
		for (int i = 0; i < levelName.length(); i++) {
			if (levelName[i] == ' ' && mode != "web") {
				if (mode == "twitter" || mode == "bluesky") levelName.replace(i, 1, "%20");
				else levelName.replace(i, 1, "+");
			}
		}
		if (mode == "twitter") {
			geode::utils::web::openLinkInBrowser(fmt::format("https://twitter.com/intent/tweet?text=I%20just%20{}%20{}%20by%20{}%20in%20{}%20attempt{}%21%20If%20you%20want%20to%20try%20it,%20the%20ID%20is%20{}.",
				completedOrVerified,
				levelName,
				creatorName,
				attempts,
				pluralOrNot,
				levelID
			));
		} else if (mode == "bluesky") {
			geode::utils::web::openLinkInBrowser(fmt::format("https://bsky.app/intent/compose?text=I%20just%20{}%20{}%20by%20{}%20in%20{}%20attempt{}%21%20If%20you%20want%20to%20try%20it,%20the%20ID%20is%20{}.",
				completedOrVerified,
				levelName,
				creatorName,
				attempts,
				pluralOrNot,
				levelID
			));
		} else if (mode == "mastodon") {
			geode::utils::web::openLinkInBrowser(fmt::format("https://{}/share?text=I+just+{}+{}+by+{}+in+{}+attempt{}%21+If+you+want+to+try+it,+the+ID+is+{}.",
				getString("mastodonInstance"),
				completedOrVerified,
				levelName,
				creatorName,
				attempts,
				pluralOrNot,
				levelID
			));
		} else if (mode == "reddit") {
			if (getBool("useSHReddit")) {
				geode::utils::web::openLinkInBrowser(fmt::format("https://sh.reddit.com/r/geometrydash/submit/?title=I+just+{}+{}+in+{}+attempt{}!+ID:+{}&type=IMAGE",
					completedOrVerified,
					levelName,
					attempts,
					pluralOrNot,
					levelID
				));
			} else {
				geode::utils::web::openLinkInBrowser(fmt::format("https://new.reddit.com/r/geometrydash/submit?title=I+just+{}+{}+in+{}+attempt{}!+ID:+{}&selftext=true&text=Hey+there!%0AYou+should+click+on+the+%22Images+%26+Video%22+tab+to+attach+your+video+recording+of+your+level+completion+so+your+post+follows+r%2Fgeometrydash+rules.%0APosting+this+text+alone+will+get+your+post+auto-removed.+Thanks!%0A--RayDeeUx%2C+in+cooperation+with+r%2Fgeometrydash+staff",
					completedOrVerified,
					levelName,
					attempts,
					pluralOrNot,
					levelID
				));
			}
		}
	}
	static void showScreenshotFailurePopup() {
		return FLAlertLayer::create("WarbledCompletions Error!", "There was an error while taking a screenshot.", "Oof...")->show();
	}
	static void showDiscordFailurePopup() {
		geode::utils::web::openLinkInBrowser("https://discord.com/app");
		return FLAlertLayer::create("WarbledCompletions Error!", "You either failed to provide a valid app to open <cb>Discord</c>, or your <cb>Discord</c> app was updated recently.\n\n<cy>WarbledCompletions is opening Discord in your web browser instead.</c>", "Oof...")->show();
	}
	static void openDiscordHopefully() {
		#ifdef GEODE_IS_MOBILE
		return;
		#else
		if (!getBool("enabled")) return;
		if (getPath("discordApp").string().empty() || !std::filesystem::exists(getPath("discordApp"))) return showDiscordFailurePopup();
		std::filesystem::path discordPath = getPath("discordApp");
		std::string discordPathFixed, command;
		#endif
		#ifdef GEODE_IS_WINDOWS
		discordPathFixed = geode::utils::string::wideToUtf8(discordPath);
		if (!utils::string::endsWith(discordPathFixed, ".exe")) return showDiscordFailurePopup();
		command = fmt::format("start {}", discordPathFixed);
		#elif defined(GEODE_IS_MACOS)
		discordPathFixed = discordPath.string();
		if (!utils::string::endsWith(discordPathFixed, ".app")) return showDiscordFailurePopup();
		command = fmt::format("open {}", discordPathFixed);
		#endif
		#ifdef GEODE_IS_DESKTOP
		if (!utils::string::contains(discordPathFixed, "Discord.") && !utils::string::contains(discordPathFixed, "DiscordPTB.") && !utils::string::contains(discordPathFixed, "DiscordCanary.") && !utils::string::contains(discordPathFixed, "Vesktop")) return showDiscordFailurePopup();
		system(command.c_str());
		#endif
	}
	void addScreenshot(CCMenu *menu) {
		if (isDisabled("screenshot")) return;
		const auto screenshotButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("screenshot.png"_spr), this, menu_selector(SharingEndLevelLayer::onScreenshot));
		screenshotButton->setID("screenshot-button"_spr);
		menu->addChild(screenshotButton);
		menu->updateLayout();
	}
	void addWeb(CCMenu *menu) {
		if (getString("customURL").empty() || !getBool("enabled")) return;
		const auto webButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("web.png"_spr), this, menu_selector(SharingEndLevelLayer::onWeb));
		webButton->setID("web-button"_spr);
		menu->addChild(webButton);
		menu->updateLayout();
	}
	void addDiscord(CCMenu *menu) {
		#ifdef GEODE_IS_MOBILE
		return;
		#endif
		if (isDisabled("discord") || getPath("discordApp").empty()) return;
		const auto discordButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onOpenTheDiscordAppOrSomething));
		discordButton->setID("discord-button"_spr);
		menu->addChild(discordButton);
		menu->updateLayout();
	}
	void addMastodon(CCMenu *menu) {
		if (isDisabled("mastodon")) return;
		const auto mastodonButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("mastodon.png"_spr), this, menu_selector(SharingEndLevelLayer::onMastodon));
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
		const auto blueskyButton = CCMenuItemSpriteExtra::create(blueskySprite, this, menu_selector(SharingEndLevelLayer::onBluesky));
		blueskyButton->setID("bluesky-button"_spr);
		menu->addChild(blueskyButton);
		menu->updateLayout();
	}
	void addRedditIfNotRobTopLevel(CCMenu *menu) {
		if (isDisabled("reddit") || m_playLayer->m_level->m_levelType == GJLevelType::Local) return;
		const auto redditButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("gj_rdIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onReddit));
		redditButton->setID("reddit-button"_spr);
		menu->addChild(redditButton);
		menu->updateLayout();
	}
	void addTwitter(CCMenu *menu) {
		if (isDisabled("twitter")) return;
		const auto tweetButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("gj_twIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onTweet));
		tweetButton->setID("tweet-button"_spr);
		menu->addChild(tweetButton);
		menu->updateLayout();
	}
	void onTweet(CCObject*) {
		if (isDisabled("twitter")) return;
		if (getBool("skipConfirmation")) return shareCompletionTo("twitter");
		geode::createQuickPopup("WarbledCompletions", "Would you like to <cj>Tweet</c> this completion?", "No", "Yes", [this](auto, bool tweet) {
			if (tweet) shareCompletionTo("twitter");
		});
	}
	void onBluesky(CCObject*) {
		if (isDisabled("bluesky")) return;
		if (getBool("skipConfirmation")) return shareCompletionTo("bluesky");
		geode::createQuickPopup("WarbledCompletions", "Would you like to post this completion to <cl>Bluesky</c>?", "No", "Yes", [this](auto, bool bluesky) {
			if (bluesky) shareCompletionTo("bluesky");
		});
	}
	void onMastodon(CCObject*) {
		if (isDisabled("mastodon")) return;
		if (getBool("skipConfirmation")) return shareCompletionTo("mastodon");
		geode::createQuickPopup("WarbledCompletions", fmt::format("Would you like to post this completion in <ca>{}</c>, which is hopefully a <ca>Mastodon</c> instance?", getString("mastodonInstance")), "No", "Yes", [this](auto, bool mastodon) {
			if (mastodon) shareCompletionTo("mastodon");
		});
	}
	void onReddit(CCObject*) {
		if (isDisabled("reddit") || m_playLayer->m_level->m_levelType == GJLevelType::Local) return;
		if (getBool("skipConfirmation")) return shareCompletionTo("reddit");
		geode::createQuickPopup("WarbledCompletions", "Would you like to post this completion in <co>r/geometrydash</c>?\n\n<cy>Remember to include video/screenshot evidence of your completion!</c>", "No", "Yes", [this](auto, bool reddit) {
			if (reddit) shareCompletionTo("reddit");
		});
	}
	void onOpenTheDiscordAppOrSomething(CCObject*) {
		#ifdef GEODE_IS_MOBILE
		return;
		#endif
		if (isDisabled("discord")) return;
		if (getBool("skipConfirmation")) return openDiscordHopefully();
		geode::createQuickPopup("WarbledCompletions", "Would you like to open <cb>Discord</c> to share your completion?\n\n<cy>WarbledCompletions is not responsible for any damages (tangible or otherwise) if Discord's \"Streamer Mode\" is not active.</c>", "No", "Yes", [this](auto, bool discord) {
			if (!discord) return;
			return openDiscordHopefully();
		});
	}
	void onWeb(CCObject*) {
		if (getString("customURL").empty() || !getBool("enabled")) return;
		if (getBool("skipConfirmation")) return geode::utils::web::openLinkInBrowser(fmt::format("https://{}", getString("customURL")));
		geode::createQuickPopup("WarbledCompletions", fmt::format("Would you like to share your completion <cb>elsewhere</c>?\n\n<cy>If you choose this option, you are responsible for the contents of the web page you chose:</c>\n\n<cl>{}</c>", getString("customURL")), "No", "Yes", [this](auto, bool web) {
			if (!web) return;
			geode::utils::web::openLinkInBrowser(fmt::format("https://{}", getString("customURL")));
		});
	}
	void onScreenshot(CCObject*) {
		if (isDisabled("screenshot")) return;
		if (!m_playLayer || !m_playLayer->m_level) return showScreenshotFailurePopup();
		auto screenshotResult = PRNTSCRN::screenshotNodeAdvanced(CCScene::get(), {this->getChildByIDRecursive("look-i-did-it-menu"_spr)}, {});
		if (screenshotResult.err()) showScreenshotFailurePopup();
		screenshotResult = PRNTSCRN::screenshotNodeAdvanced(m_playLayer, {this->getChildByIDRecursive("look-i-did-it-menu"_spr)}, {});
		if (screenshotResult.err()) showScreenshotFailurePopup();
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
		addMastodon(menu);
		addDiscord(menu);
		addWeb(menu);
		addScreenshot(menu);
		if (menu->getChildrenCount() < 1) menu->removeMeAndCleanup();
	}
};