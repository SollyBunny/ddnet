#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_COLORED_PARTS_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_COLORED_PARTS_H

#include <base/color.h>

#include <engine/shared/console.h>

#include <variant>
#include <vector>

using ColoredPart = std::variant<const char *, ColorRGBA>;
class CColoredParts
{
public:
	char *m_pStringStorage = nullptr;
	std::vector<ColoredPart> m_vParts;
	~CColoredParts() { delete[] m_pStringStorage; }
	CColoredParts(const char *pText, bool Parse)
	{
		if(Parse)
		{
			m_pStringStorage = new char[str_length(pText) + 1];
			mem_copy(m_pStringStorage, pText, str_length(pText) + 1);
			char *pFinder = m_pStringStorage;
			char *pWritten = m_pStringStorage;
			while(*pFinder)
			{
				char *pMarkerStart = strstr(pFinder, "[[");
				if(!pMarkerStart)
				{
					// No more markers, add the rest of the string
					m_vParts.emplace_back(pWritten);
					break;
				}
				// From the start of just after the marker found in pMarkerStart, find the end marker
				char *pMarkerEnd = strstr(pMarkerStart + 2, "]]");
				if(!pMarkerEnd)
				{
					// No marker end, add the rest of the string
					m_vParts.emplace_back(pWritten);
					break;
				}
				// Check if the marker is valid
				*pMarkerEnd = '\0';
				const auto Color = CConsole::ColorParse(pMarkerStart + 2, 0.0f);
				if(Color.has_value())
				{
					// Add text before the marker, if any
					if(pMarkerStart != pWritten)
					{
						*pMarkerStart = '\0';
						m_vParts.emplace_back(pWritten);
					}
					// Add the color
					m_vParts.emplace_back(color_cast<ColorRGBA>(*Color));
					// Move written to end
					pWritten = pMarkerEnd + 2;
				}
				else
				{
					// Restore ']'
					*pMarkerEnd = ']';
				}
				// Move finder to end
				pFinder = pMarkerEnd + 2;
			}
			return;
		}
		m_vParts.emplace_back(pText);
	}
};

#endif
