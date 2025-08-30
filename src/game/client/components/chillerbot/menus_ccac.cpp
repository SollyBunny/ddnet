#include <base/log.h>
#include <base/math.h>
#include <base/system.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>
#include <engine/shared/localization.h>
#include <engine/shared/protocol7.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/updater.h>

#include <game/generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/components/chat.h>
#include <game/client/components/menu_background.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/skin.h>
#include <game/client/ui.h>
#include <game/client/ui_listbox.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

#include "../menus.h"

using namespace FontIcons;
using namespace std::chrono_literals;

void CMenus::RenderSettingsCCAC(CUIRect MainView)
{
	CUIRect Button, Left, Label, LeftRect, RightRect, Bottom;
	char aBuf[128];
	MainView.HSplitTop(20.0f, &Button, &MainView);
	MainView.HSplitMid(&MainView, &Bottom, 20.0f);
	if(DoButton_CheckBox(&g_Config.m_CcacEnableAc, Localize("Enable anticheat features"), g_Config.m_CcacEnableAc, &Button))
	{
		g_Config.m_CcacEnableAc ^= 1;
	}
	if(g_Config.m_CcacEnableAc)
	{
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_CcacSilentCommands, Localize("Toggle silent chat commands"), g_Config.m_CcacSilentCommands, &Button))
		{
			g_Config.m_CcacSilentCommands ^= 1;
		}
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_CcacMarkClean, Localize("Mark clean players"), g_Config.m_CcacMarkClean, &Button))
		{
			g_Config.m_CcacMarkClean ^= 1;
		}
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_CcacEnableReplay, Localize("Automatically save replay when marking bots"), g_Config.m_CcacEnableReplay, &Button))
		{
			g_Config.m_CcacEnableReplay ^= 1;
		}

		MainView.VSplitMid(&Left, nullptr, 20.0f);
		ColorRGBA BotDefault(0.72f, 0.23f, 0.23f, 1.0f);
		ColorRGBA CleanDefault(0.68f, 1.0f, 0.68f, 1.0f);
		ColorRGBA SusDefault(0.81f, 0.68f, 0.68f, 1.0f);
		static CButtonContainer s_BottingPlayerColor, s_CleanPlayerColor, s_SusPlayerColor;
		const float ColorPickerLineSize = 25.0f;
		const float ColorPickerLabelSize = 13.0f;
		const float ColorPickerLineSpacing = 5.0f;
		DoLine_ColorPicker(&s_BottingPlayerColor, ColorPickerLineSize, ColorPickerLabelSize, ColorPickerLineSpacing, &Left, Localize("Botting player color"), &g_Config.m_CcacBottingPlayerColor, BotDefault, false);
		DoLine_ColorPicker(&s_SusPlayerColor, ColorPickerLineSize, ColorPickerLabelSize, ColorPickerLineSpacing, &Left, Localize("Unsure player color"), &g_Config.m_CcacSusPlayerColor, SusDefault, false);
		DoLine_ColorPicker(&s_CleanPlayerColor, ColorPickerLineSize, ColorPickerLabelSize, ColorPickerLineSpacing, &Left, Localize("Clean player color"), &g_Config.m_CcacCleanPlayerColor, CleanDefault, false);

		CUIRect BotLinkCheckbox;
		Bottom.HSplitTop(20.0f, &Button, &Bottom);
		Button.VSplitLeft(110.0f, &Label, &Button);
		Button.VSplitLeft(50.0f, &Button, &BotLinkCheckbox);
		Button.VSplitLeft(700.0f, &Button, 0);
		str_format(aBuf, sizeof(aBuf), "%s:", "Online bot list");
		Ui()->DoLabel(&Label, aBuf, 14.0f, -1);
		static CLineInput s_BotLinkInput;
		s_BotLinkInput.SetBuffer(g_Config.m_CcacBotLink, sizeof(g_Config.m_CcacBotLink));
		s_BotLinkInput.SetEmptyText("https://raw.githubusercontent.com/ccac52158/cdn/refs/heads/main/bots.json");
		Ui()->DoEditBox(&s_BotLinkInput, &Button, 14.0f);
		Bottom.HSplitTop(20.0f, &Button, &Bottom);
		if(DoButton_CheckBox(&g_Config.m_CcacUseLocal, Localize("Use local name list"), g_Config.m_CcacUseLocal, &Button))
		{
			g_Config.m_CcacUseLocal ^= 1;
		}
		if(g_Config.m_CcacUseLocal)
		{
			Bottom.HSplitTop(5.0f, &Button, &Bottom);
			{
				CUIRect Checkbox;
				Bottom.HSplitTop(20.0f, &Button, &Bottom);
				Button.VSplitLeft(110.0f, &Label, &Button);
				Button.VSplitLeft(50.0f, &Button, &Checkbox);
				Button.VSplitLeft(500.0f, &Button, 0);
				str_format(aBuf, sizeof(aBuf), "%s:", "Local list names");
				Ui()->DoLabel(&Label, aBuf, 14.0f, -1);
				static CLineInput s_NameInput;
				s_NameInput.SetBuffer(g_Config.m_CcacLocalNames, sizeof(g_Config.m_CcacLocalNames));
				s_NameInput.SetEmptyText("names.txt");
				Ui()->DoEditBox(&s_NameInput, &Button, 14.0f);
			}
		}
	}
	//=======================================================================

	Bottom.HSplitBottom(20.0f, &Button, &Bottom);
	Bottom.VSplitMid(&LeftRect, &RightRect, 20.0f);
	//MainView.VSplitLeft(20.0f, nullptr, &MainView);

	static CButtonContainer s_ConfigButtonId;
	if(DoButton_Menu(&s_ConfigButtonId, Localize("Replay directory"), 0, &LeftRect))
	{
		Storage()->GetCompletePath(IStorage::TYPE_SAVE, "demos/replays", aBuf, sizeof(aBuf));
		Client()->ViewFile(aBuf);
	}
	GameClient()->m_Tooltips.DoToolTip(&s_ConfigButtonId, &MainView, Localize("Open the directory containing automatically recorded replays"));

	static CButtonContainer s_NameButtonId;
	if(DoButton_Menu(&s_NameButtonId, Localize("Local name list"), 0, &RightRect))
	{
		Storage()->GetCompletePath(IStorage::TYPE_SAVE, "ccac", aBuf, sizeof(aBuf));
		Client()->ViewFile(aBuf);
	}
	GameClient()->m_Tooltips.DoToolTip(&s_NameButtonId, &MainView, Localize("Open the directory containing the name list"));
}
