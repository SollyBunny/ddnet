#include "hover_notification.h"
#include <game/client/gameclient.h>

CHoverNotification::CHoverNotification()
{
	m_NumActiveNotifications = 0;
	for(int i = 0; i < MAX_NOTIFICATIONS; i++)
	{
		m_aNotifications[i].m_Active = false;
		m_aNotifications[i].m_StartTime = 0;
		m_aNotifications[i].m_Duration = 0;
		m_aNotifications[i].m_aText[0] = '\0';
		m_aNotifications[i].m_Position = vec2(0, 0);
	}
}

void CHoverNotification::OnInit()
{
	//NYON :3
}

void CHoverNotification::OnReset()
{
	Stop();
}

void CHoverNotification::OnWindowResize()
{
	UpdatePositions();
}

void CHoverNotification::OnShutdown()
{
	Stop();
}

void CHoverNotification::UpdatePositions()
{
	float CurrentY = 10.0f; // Start from top
	float Spacing = 35.0f; // Space between notifications

	for(int i = 0; i < MAX_NOTIFICATIONS; i++)
	{
		if(m_aNotifications[i].m_Active)
		{
			float TextWidth = TextRender()->TextWidth(14.0f, m_aNotifications[i].m_aText, -1);
			m_aNotifications[i].m_Position = vec2(Graphics()->ScreenWidth() - TextWidth - 20.0f, CurrentY);
			CurrentY += Spacing;
		}
	}
}

void CHoverNotification::Start(const char *pText, float Duration)
{
	int SlotIndex = -1;
	int64_t OldestTime = time_get();

	for(int i = 0; i < MAX_NOTIFICATIONS; i++)
	{
		if(!m_aNotifications[i].m_Active)
		{
			SlotIndex = i;
			break;
		}
		if(m_aNotifications[i].m_StartTime < OldestTime)
		{
			OldestTime = m_aNotifications[i].m_StartTime;
			SlotIndex = i;
		}
	}

	if(SlotIndex >= 0)
	{
		str_copy(m_aNotifications[SlotIndex].m_aText, pText, sizeof(m_aNotifications[SlotIndex].m_aText));
		m_aNotifications[SlotIndex].m_Duration = Duration;
		m_aNotifications[SlotIndex].m_StartTime = time_get();
		m_aNotifications[SlotIndex].m_Active = true;
		m_NumActiveNotifications = std::min(m_NumActiveNotifications + 1, MAX_NOTIFICATIONS);

		UpdatePositions();
	}
}

void CHoverNotification::Stop()
{
	for(int i = 0; i < MAX_NOTIFICATIONS; i++)
	{
		m_aNotifications[i].m_Active = false;
	}
	m_NumActiveNotifications = 0;
}

void CHoverNotification::OnRender()
{
	if(m_NumActiveNotifications == 0)
		return;

	Graphics()->MapScreen(0, 0, Graphics()->ScreenWidth(), Graphics()->ScreenHeight());

	for(int i = 0; i < MAX_NOTIFICATIONS; i++)
	{
		if(!m_aNotifications[i].m_Active)
			continue;

		float TimePassed = (time_get() - m_aNotifications[i].m_StartTime) / (float)time_freq();
		if(TimePassed > m_aNotifications[i].m_Duration)
		{
			m_aNotifications[i].m_Active = false;
			m_NumActiveNotifications--;
			UpdatePositions();
			continue;
		}

		// Calculate animation progress
		float Progress = 1.0f;
		if(TimePassed < 0.3f) // Fade in
			Progress = TimePassed / 0.3f;
		else if(TimePassed > m_aNotifications[i].m_Duration - 0.3f) // Fade out
			Progress = (m_aNotifications[i].m_Duration - TimePassed) / 0.3f;

		// Calculate text width for proper sizing
		float TextWidth = TextRender()->TextWidth(14.0f, m_aNotifications[i].m_aText, -1);
		float BoxWidth = TextWidth + 30.0f; // Slightly wider padding
		float BoxHeight = 30.0f;

		// Background with slight gradient
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();

		// Main background
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.7f * Progress);
		IGraphics::CQuadItem QuadItem(m_aNotifications[i].m_Position.x - 15.0f,
			m_aNotifications[i].m_Position.y - 5.0f,
			BoxWidth, BoxHeight);
		Graphics()->QuadsDrawTL(&QuadItem, 1);

		// Left border accent
		Graphics()->SetColor(0.8f, 0.0f, 0.0f, 0.3f * Progress);
		IGraphics::CQuadItem BorderQuad(m_aNotifications[i].m_Position.x - 15.0f,
			m_aNotifications[i].m_Position.y - 5.0f,
			2.0f, BoxHeight);
		Graphics()->QuadsDrawTL(&BorderQuad, 1);
		Graphics()->QuadsEnd();

		// Text with slight shadow effect
		TextRender()->TextColor(0.0f, 0.0f, 0.0f, 0.3f * Progress); // Shadow
		TextRender()->Text(m_aNotifications[i].m_Position.x + 1.0f,
			m_aNotifications[i].m_Position.y + 1.0f,
			14.0f, m_aNotifications[i].m_aText, -1);

		TextRender()->TextColor(1.0f, 1.0f, 1.0f, Progress); // Main text
		TextRender()->Text(m_aNotifications[i].m_Position.x,
			m_aNotifications[i].m_Position.y,
			14.0f, m_aNotifications[i].m_aText, -1);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
	}
}
