/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SKINS_H
#define GAME_CLIENT_COMPONENTS_SKINS_H

#include <base/lock.h>

#include <engine/shared/config.h>
#include <engine/shared/jobs.h>

#include <game/client/component.h>
#include <game/client/skin.h>

#include <chrono>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

class CHttpRequest;

class CSkins : public CComponent
{
private:
	/**
	 * Maximum length of normalized skin names. Normalization may increase the length.
	 */
	static constexpr size_t NORMALIZED_SKIN_NAME_LENGTH = 2 * MAX_SKIN_LENGTH;

	/**
	 * The data of a skin that can be loaded in a separate thread.
	 */
	class CSkinLoadData
	{
	public:
		CImageInfo m_Info;
		CImageInfo m_InfoGrayscale;
		CSkin::CSkinMetrics m_Metrics;
		ColorRGBA m_BloodColor;
	};

	/**
	 * An abstract job to load a skin from a source determined by the subclass.
	 */
	class CAbstractSkinLoadJob : public IJob
	{
	public:
		CAbstractSkinLoadJob(CSkins *pSkins, const char *pName);
		virtual ~CAbstractSkinLoadJob();

		CSkinLoadData m_Data;
		bool m_NotFound = false;

	protected:
		CSkins *m_pSkins;
		char m_aName[MAX_SKIN_LENGTH];
	};

public:
	/**
	 * Container for a skin its loading state and job.
	 */
	class CSkinContainer
	{
		friend class CSkins;

	public:
		enum class EType
		{
			/**
			 * Skin should be loaded locally (from skins folder).
			 */
			LOCAL,
			/**
			 * Skin should be downloaded (or loaded from downloadedskins).
			 */
			DOWNLOAD,
		};

		enum class EState
		{
			/**
			 * Skin is unloaded and loading is not desired.
			 */
			UNLOADED,
			/**
			 * Skin is unloaded and should be loaded when a slot is free. Skin will enter @link LOADING @endlink
			 * state when maximum number of loaded skins is not exceeded.
			 */
			PENDING,
			/**
			 * Skin is currently loading, iff @link m_pLoadJob @endlink is set.
			 */
			LOADING,
			/**
			 * Skin is loaded, iff @link m_pSkin @endlink is set.
			 */
			LOADED,
			/**
			 * Skin failed to be loaded due to an unexpected error.
			 */
			ERROR,
			/**
			 * Skin failed to be downloaded because it could not be found.
			 */
			NOT_FOUND,
		};

		CSkinContainer(CSkinContainer &&Other) = default;
		CSkinContainer(const char *pName, EType Type, int StorageType);
		~CSkinContainer();

		bool operator<(const CSkinContainer &Other) const;
		CSkinContainer &operator=(CSkinContainer &&Other) = default;

		const char *Name() const { return m_aName; }
		const char *NormalizedName() const { return m_aNormalizedName; }
		EType Type() const { return m_Type; }
		int StorageType() const { return m_StorageType; }
		bool IsVanilla() const { return m_Vanilla; }
		bool IsSpecial() const { return m_Special; }
		EState State() const { return m_State; }
		const std::unique_ptr<CSkin> &Skin() const { return m_pSkin; }

		/**
		 * Request that this skin should be loaded and should stay loaded.
		 */
		void RequestLoad();

	private:
		char m_aName[MAX_SKIN_LENGTH];
		char m_aNormalizedName[NORMALIZED_SKIN_NAME_LENGTH];
		EType m_Type;
		int m_StorageType;
		bool m_Vanilla;
		bool m_Special;

		EState m_State;
		std::unique_ptr<CSkin> m_pSkin = nullptr;
		std::shared_ptr<CAbstractSkinLoadJob> m_pLoadJob = nullptr;

		/**
		 * When loading of this skin was first requested.
		 */
		std::optional<std::chrono::nanoseconds> m_FirstLoadRequest;
		/**
		 * When loading of this skin was most recently requested.
		 */
		std::optional<std::chrono::nanoseconds> m_LastLoadRequest;
		/**
		 * How many times this skin has been set to loading state.
		 */
		uint32_t m_LoadCount = 0;
	};

	/**
	 * Represents a skin being displayed in a list in the UI.
	 */
	class CSkinListEntry
	{
	public:
		CSkinListEntry() :
			m_pSkinContainer(nullptr),
			m_Favorite(false) {}
		CSkinListEntry(CSkinContainer *pSkinContainer, bool Favorite) :
			m_pSkinContainer(pSkinContainer),
			m_Favorite(Favorite) {}

		bool operator<(const CSkinListEntry &Other) const;

		const CSkinContainer *SkinContainer() const { return m_pSkinContainer; }
		bool IsFavorite() const { return m_Favorite; }
		const void *ListItemId() const { return &m_ListItemId; }
		const void *FavoriteButtonId() const { return &m_FavoriteButtonId; }
		const void *ErrorTooltipId() const { return &m_ErrorTooltipId; }

		/**
		 * Request that this skin should be loaded and should stay loaded.
		 */
		void RequestLoad();

	private:
		CSkinContainer *m_pSkinContainer;
		bool m_Favorite;
		char m_ListItemId;
		char m_FavoriteButtonId;
		char m_ErrorTooltipId;
	};

	class CSkinLoadingStats
	{
	public:
		size_t m_NumUnloaded = 0;
		size_t m_NumPending = 0;
		size_t m_NumLoading = 0;
		size_t m_NumLoaded = 0;
		size_t m_NumError = 0;
		size_t m_NumNotFound = 0;
	};

	CSkins();

	typedef std::function<void()> TSkinLoadedCallback;

	int Sizeof() const override { return sizeof(*this); }
	void OnConsoleInit() override;
	void OnInit() override;
	void OnShutdown() override;
	void OnUpdate() override;

	void Refresh(TSkinLoadedCallback &&SkinLoadedCallback);
	CSkinLoadingStats LoadingStats() const;

	std::vector<CSkinListEntry> &SkinList();
	void ForceRefreshSkinList();

	const CSkinContainer *FindContainerOrNullptr(const char *pName);
	const CSkin *FindOrNullptr(const char *pName, bool IgnorePrefix = false);
	const CSkin *Find(const char *pName);

	void AddFavorite(const char *pName);
	void RemoveFavorite(const char *pName);
	bool IsFavorite(const char *pName) const;

	void RandomizeSkin(int Dummy);

	static bool IsVanillaSkin(const char *pName);
	static bool IsSpecialSkin(const char *pName);

private:
	constexpr static const char *VANILLA_SKINS[] = {"bluekitty", "bluestripe", "brownbear",
		"cammo", "cammostripes", "coala", "default", "limekitty",
		"pinky", "redbopp", "redstripe", "saddo", "toptri",
		"twinbop", "twintri", "warpaint", "x_ninja", "x_spec"};

	class CSkinLoadJob : public CAbstractSkinLoadJob
	{
	public:
		CSkinLoadJob(CSkins *pSkins, const char *pName, int StorageType);

	protected:
		void Run() override;

	private:
		int m_StorageType;
	};

	class CSkinDownloadJob : public CAbstractSkinLoadJob
	{
	public:
		CSkinDownloadJob(CSkins *pSkins, const char *pName);

		bool Abort() override REQUIRES(!m_Lock);

	protected:
		void Run() override REQUIRES(!m_Lock);

	private:
		CLock m_Lock;
		std::shared_ptr<CHttpRequest> m_pGetRequest GUARDED_BY(m_Lock);
	};

	std::unordered_map<std::string_view, std::unique_ptr<CSkinContainer>> m_Skins;
	std::optional<std::chrono::nanoseconds> m_LastRefreshTime;
	std::optional<std::chrono::nanoseconds> m_ContainerUpdateTime;

	std::vector<CSkinListEntry> m_vSkinList;
	std::optional<std::chrono::nanoseconds> m_SkinListLastRefreshTime;

	std::unordered_set<std::string> m_Favorites;

	CSkin m_PlaceholderSkin;
	char m_aEventSkinPrefix[MAX_SKIN_LENGTH];

	bool LoadSkinData(const char *pName, CSkinLoadData &Data) const;
	void LoadSkinFinish(CSkinContainer *pSkinContainer, const CSkinLoadData &Data);
	void LoadSkinDirect(const char *pName);
	const CSkin *FindImpl(const char *pName);
	static int SkinScan(const char *pName, int IsDir, int StorageType, void *pUser);

	void UpdateUnloadSkins(CSkinLoadingStats &Stats);
	void UpdateStartLoading(CSkinLoadingStats &Stats);
	void UpdateFinishLoading(std::chrono::nanoseconds StartTime, std::chrono::nanoseconds MaxTime);

	static void ConAddFavoriteSkin(IConsole::IResult *pResult, void *pUserData);
	static void ConRemFavoriteSkin(IConsole::IResult *pResult, void *pUserData);
	static void ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData);
	void OnConfigSave(IConfigManager *pConfigManager);
};
#endif
