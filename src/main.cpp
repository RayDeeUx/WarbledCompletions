#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/utils/web.hpp>

#define PREFERRED_HOOK_PRIO (-2123456789)

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
		return FLAlertLayer::create("WarbledCompletions Error!", "You did not provide a valid app to open <cb>Discord</c>.\n\n<cy>WarbledCompletions is opening Discord in your web browser instead.</c>", "Oof...")->show();
	}
	static void openDiscordHopefully() {
		if (!getBool("enabled")) return;
		if (getPath("discordApp").string().empty()) return showDiscordFailurePopup();
		std::filesystem::path discordPath = getPath("discordApp");
		std::string discordPathFixed, command;
		#ifdef GEODE_IS_WINDOWS
		discordPathFixed = geode::utils::string::wideToUtf8(discordPath);
		if (!utils::string::endsWith(discordPathFixed, ".exe")) return showDiscordFailurePopup();
		command = fmt::format("start {}", discordPathFixed);
		#elif defined(GEODE_IS_MACOS)
		discordPathFixed = discordPath.string();
		if (!utils::string::endsWith(discordPathFixed, ".app")) return showDiscordFailurePopup();
		command = fmt::format("open {}", discordPathFixed);
		#endif
		if (!utils::string::contains(discordPathFixed, "Discord.") && !utils::string::contains(discordPathFixed, "DiscordPTB.") && !utils::string::contains(discordPathFixed, "DiscordCanary.") && !utils::string::contains(discordPathFixed, "Vesktop")) return showDiscordFailurePopup();
		system(command.c_str());
	}
	void addScreenshot(CCMenu *menu) {
		if (isDisabled("screenshot")) return;
		auto screenshotButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("screenshot.png"_spr), this, menu_selector(SharingEndLevelLayer::onScreenshot));
		screenshotButton->setID("screenshot-button"_spr);
		menu->addChild(screenshotButton);
		menu->updateLayout();
	}
	void addWeb(CCMenu *menu) {
		if (getString("customURL").empty() || !getBool("enabled")) return;
		auto webButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("web.png"_spr), this, menu_selector(SharingEndLevelLayer::onWeb));
		webButton->setID("web-button"_spr);
		menu->addChild(webButton);
		menu->updateLayout();
	}
	void addDiscord(CCMenu *menu) {
		if (isDisabled("discord") || getPath("discordApp").empty()) return;
		auto discordButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png"), this, menu_selector(SharingEndLevelLayer::onOpenTheDiscordAppOrSomething));
		discordButton->setID("discord-button"_spr);
		menu->addChild(discordButton);
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
	void delayedClipboardScreenshot() {
		// escape chars or else terminal cmd fails
		std::string filePath = m_fields->filePath;
		filePath = utils::string::replace(utils::string::replace(utils::string::replace(utils::string::replace(utils::string::replace(filePath, " ", "\\ "), "(", "\\("), ")", "\\)"), "[", "\\["), "]", "\\]");
		system(fmt::format("screencapture -wxo -tpng {}", filePath).c_str());
		std::string message = !Loader::get()->isModInstalled("ninxout.prntscrn") ? "Pro tip: Hold the SHIFT key while clicking the screenshot shortcut button to send screenshots directly to your clipboard,</c> <co>instead of saving the screenshot to the config folder!</c><cy>" : "Y'know, you could've done that exact same thing with ninXout's PRNTSCRN mod...";
		geode::createQuickPopup("WarbledCompletions", fmt::format("Screenshot complete! Would you like to open the location of your screenshot?\n\n<cy>({})</c>", message), "No", "Yes", [this](auto, bool configDir) {
			if (!configDir) return;
			geode::utils::file::openFolder(configDirPath);
		});
		this->getChildByIDRecursive("look-i-did-it-menu"_spr)->setVisible(true);
	}
	void delayedConfigScreenshot() {
		system("screencapture -wxoc -tpng");
		FLAlertLayer::create("WarbledCompletions", "Screenshot complete! It should be on your clipboard now.", "OK")->show();
		this->getChildByIDRecursive("look-i-did-it-menu"_spr)->setVisible(true);
	}
	// adapted from code by TheSillyDoggo (she/her): https://discord.com/channels/911701438269386882/911702535373475870/1291198134013394946
	// original code: https://raw.githubusercontent.com/TheSillyDoggo/Screenshot-Mod/main/src/main.cpp
	void onScreenshot(CCObject*) {
		if (isDisabled("screenshot")) return;
		if (!m_playLayer || !m_playLayer->m_level) return showScreenshotFailurePopup();
		const auto &level = m_playLayer->m_level;
		std::string levelID = level->m_levelID.value() == 0 ? "Custom level" : std::to_string(level->m_levelID.value());
		m_fields->filePath = fmt::format(
			"{}/{} by {} [{}] ({}).png",
			configDir,
			std::string(level->m_levelName),
			std::string(level->m_creatorName),
			levelID,
			std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
		);
		#ifndef __APPLE__
		CCScene* scene = CCDirector::sharedDirector()->getRunningScene();
		int width = static_cast<int>(scene->getContentWidth());
		int height = static_cast<int>(scene->getContentHeight());
		CCRenderTexture* renderTexture = CCRenderTexture::create(width, height);

		// Send data to renderTexture for captured image
		this->getChildByIDRecursive("look-i-did-it-menu"_spr)->setVisible(false);
		renderTexture->begin();
		scene->visit();
		renderTexture->end();
		this->getChildByIDRecursive("look-i-did-it-menu"_spr)->setVisible(true);

		// Get the captured image
		CCImage* image = renderTexture->newCCImage();

		// Save the image to file
		if (!image) return showScreenshotFailurePopup();
		image->saveToFile(m_fields->filePath.c_str());

		// Release image
		image->release();

		std::string message = !Loader::get()->isModInstalled("ninxout.prntscrn") ? "Copying a screenshot to your clipboard for copy-pasting isn't as easy as it sounds." : "Y'know, you could've done that exact same thing with ninXout's PRNTSCRN mod...";
		geode::createQuickPopup("WarbledCompletions", fmt::format("Screenshot complete! Would you like to open the location of your screenshot?\n\n<cy>({})</c>", message), "No", "Yes", [this](auto, bool configDir) {
			if (!configDir) return;
			geode::utils::file::openFolder(configDirPath);
		});
		#else
		// run macos terminal commands
		/*
		from justin:
		@ery • 埃里曼索斯 i found the function [address on macOS] but there is one problem

		***there is zero functionality for it***
		https://discord.com/channels/911701438269386882/911702535373475870/1305405118354690139
		*/
		if (CCKeyboardDispatcher::get()->getShiftKeyPressed()) {
			this->getChildByIDRecursive("look-i-did-it-menu"_spr)->setVisible(false);
			this->scheduleOnce(schedule_selector(SharingEndLevelLayer::delayedConfigScreenshot), 0.017f); // 1.f / 60.f ==> 0.016666666666666666666666
		} else {
			this->getChildByIDRecursive("look-i-did-it-menu"_spr)->setVisible(false);
			this->scheduleOnce(schedule_selector(SharingEndLevelLayer::delayedClipboardScreenshot), 0.017f);
		}
		#endif
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