#ifndef TRINITY_SDK_ABI_V1_H
#define TRINITY_SDK_ABI_V1_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define TRINITY_ABI_V1 1u
#define TRINITY_CAPABILITY_OBJECT_V1 "trinity.object.v1"
#define TRINITY_CAPABILITY_BUFFER_V1 "trinity.buffer.v1"
#define TRINITY_ABI_LAYOUT_REVISION 1u
#define TRINITY_ABI_V1 1u
#define TRINITY_CAPABILITY_CONFIG_V1 "trinity.config.v1"
#define TRINITY_CAPABILITY_COMMAND_V1 "trinity.command.v1"
 typedef struct trinity_host trinity_host;
 typedef struct trinity_module trinity_module;
 typedef struct trinity_command_context trinity_command_context;
 typedef uint64_t trinity_handle;
typedef struct trinity_object_guid { uint64_t low; uint64_t high; } trinity_object_guid;
/* Buffers are host-owned: callbacks may change size only through the host's
 * buffer API, and data is valid until callback return. Bytes are not NUL
 * terminated unless size includes the terminator explicitly. */
typedef struct trinity_buffer { uint8_t* data; size_t size; size_t capacity; uint32_t flags; } trinity_buffer;
#define TRINITY_BUFFER_READONLY 0x0001u
#define TRINITY_BUFFER_UTF8 0x0002u
#define TRINITY_BUFFER_NUL_TERMINATED 0x0004u
typedef uint32_t trinity_family_id;
#define TRINITY_FAMILY_SPELL_LOADER 1u
#define TRINITY_FAMILY_SERVER 2u
#define TRINITY_FAMILY_WORLD 3u
#define TRINITY_FAMILY_FORMULA 4u
#define TRINITY_FAMILY_MAP 5u
#define TRINITY_FAMILY_WORLD_MAP 6u
#define TRINITY_FAMILY_INSTANCE_MAP 7u
#define TRINITY_FAMILY_BATTLEGROUND_MAP 8u
#define TRINITY_FAMILY_ITEM 9u
#define TRINITY_FAMILY_UNIT 10u
#define TRINITY_FAMILY_CREATURE 11u
#define TRINITY_FAMILY_GAME_OBJECT 12u
#define TRINITY_FAMILY_AREA_TRIGGER 13u
#define TRINITY_FAMILY_ONLY_ONCE_AREA_TRIGGER 14u
#define TRINITY_FAMILY_BATTLEFIELD 15u
#define TRINITY_FAMILY_OUTDOOR_PVP 16u
#define TRINITY_FAMILY_COMMAND 17u
#define TRINITY_FAMILY_WEATHER 18u
#define TRINITY_FAMILY_AUCTION_HOUSE 19u
#define TRINITY_FAMILY_CONDITION 20u
#define TRINITY_FAMILY_VEHICLE 21u
#define TRINITY_FAMILY_DYNAMIC_OBJECT 22u
#define TRINITY_FAMILY_TRANSPORT 23u
#define TRINITY_FAMILY_ACHIEVEMENT 24u
#define TRINITY_FAMILY_ACHIEVEMENT_CRITERIA 25u
#define TRINITY_FAMILY_PLAYER 26u
#define TRINITY_FAMILY_ACCOUNT 27u
#define TRINITY_FAMILY_GUILD 28u
#define TRINITY_FAMILY_GROUP 29u
#define TRINITY_FAMILY_AREA_TRIGGER_ENTITY 30u
#define TRINITY_FAMILY_CONVERSATION 31u
#define TRINITY_FAMILY_SCENE 32u
#define TRINITY_FAMILY_QUEST 33u
#define TRINITY_FAMILY_WORLD_STATE 34u
#define TRINITY_FAMILY_EVENT 35u
#define TRINITY_FAMILY_PLAYER_CHOICE 36u
typedef struct trinity_callback_scope { uint64_t token; uint32_t family; uint32_t reserved; } trinity_callback_scope;
typedef enum trinity_object_kind { TRINITY_OBJECT_UNKNOWN = 0u, TRINITY_OBJECT_UNIT = 1u, TRINITY_OBJECT_PLAYER = 2u, TRINITY_OBJECT_CREATURE = 3u, TRINITY_OBJECT_GAME_OBJECT = 4u, TRINITY_OBJECT_MAP = 5u, TRINITY_OBJECT_SPELL = 6u, TRINITY_OBJECT_AURA = 7u, TRINITY_OBJECT_INSTANCE = 8u, TRINITY_OBJECT_BATTLEGROUND = 9u, TRINITY_OBJECT_BATTLEFIELD = 10u, TRINITY_OBJECT_OUTDOOR_PVP = 11u } trinity_object_kind;
#define TRINITY_OBJECT_FIELD_GUID 1u
#define TRINITY_OBJECT_FIELD_ENTRY 2u
#define TRINITY_OBJECT_FIELD_NAME 3u
typedef struct trinity_object_api_v1 {
  uint32_t size;
  uint32_t abi_version;
  int32_t (*begin_scope)(trinity_host const*, uint32_t, trinity_callback_scope*);
  void (*end_scope)(trinity_host const*, trinity_callback_scope*);
  int32_t (*query)(trinity_host const*, trinity_callback_scope const*, trinity_handle, trinity_object_kind, uint32_t, void*, size_t*);
  int32_t (*mutate)(trinity_host const*, trinity_callback_scope const*, trinity_handle, trinity_object_kind, uint32_t, void const*, size_t);
  int32_t (*buffer_resize)(trinity_host const*, trinity_buffer*, size_t);
} trinity_object_api_v1;
typedef struct trinity_spell_hook_result {
  uint32_t size;
  uint32_t flags;
  int32_t status;
  int64_t value;
  float multiplier;
  uint32_t effect_index;
  trinity_handle target;
} trinity_spell_hook_result;
typedef int32_t (*trinity_spell_hook_callback)(void*, trinity_host const*, trinity_callback_scope const*, trinity_handle, trinity_spell_hook_result*);
typedef int32_t (*trinity_spell_hook_dispatch)(void*, trinity_host const*, uint32_t, trinity_callback_scope const*, trinity_handle, trinity_spell_hook_result*);
typedef struct trinity_spell_callbacks {
  uint32_t size;
  uint32_t abi_version;
  void* context;
  trinity_host const* host;
  int32_t (*register_hook)(void*, trinity_host const*, uint32_t, trinity_spell_hook_callback, void*);
  void (*clear_hooks)(void*, trinity_host const*);
  int32_t (*dispatch_hook)(void*, trinity_host const*, uint32_t, trinity_callback_scope const*, trinity_handle, trinity_spell_hook_result*);
} trinity_spell_callbacks;
typedef trinity_spell_callbacks trinity_aura_callbacks;
typedef struct trinity_spell_loader_callbacks {
  uint32_t size;
  uint32_t abi_version;
  trinity_spell_callbacks const* spell;
  trinity_aura_callbacks const* aura;
} trinity_spell_loader_callbacks;
typedef struct trinity_service_registration trinity_service_registration;
typedef struct trinity_command_registration trinity_command_registration;
typedef struct trinity_host_query { uint32_t size; uint32_t abi_version; void* context; void* (*query)(trinity_host*, uint32_t); int32_t (*register_service)(trinity_host*, const trinity_service_registration*); void* (*get_service)(trinity_host*, uint32_t); int32_t (*register_command)(trinity_host*, const trinity_command_registration*); int32_t (*register_family)(trinity_host*, trinity_family_id, const void*); void* (*get_capability)(trinity_host*, const char*); } trinity_host_query;
typedef struct trinity_log_api { uint32_t size; void (*write)(trinity_host*, uint32_t, const char*, size_t); } trinity_log_api;
typedef struct trinity_config_api_v1 { uint32_t size; uint32_t abi_version; int32_t (*get_string)(trinity_host*, const char*, const char*, char*, size_t); } trinity_config_api_v1;
typedef struct trinity_command_api_v1 { uint32_t size; uint32_t abi_version; void (*send_sys_message)(trinity_command_context*, const char*, size_t); uint8_t (*is_console)(trinity_command_context*); } trinity_command_api_v1;
typedef struct trinity_host_api_v1 { uint32_t size; uint32_t abi_version; trinity_host_query query; trinity_log_api log; } trinity_host_api_v1;
static inline trinity_host_api_v1 const* TrinityHostApiV1(trinity_host const* host) { return host ? (trinity_host_api_v1 const*)host : (trinity_host_api_v1 const*)0; }
struct trinity_service_registration { uint32_t size; uint32_t service_id; const char* name; void* service; };
struct trinity_command_registration { uint32_t size; const char* name; const char* help; int32_t (*execute)(void*, trinity_command_context*, const char*, size_t); void* context; uint32_t permission; uint8_t allow_console; };
typedef struct trinity_module_api_v1 { uint32_t size; uint32_t abi_version; void* module_context; int32_t (*on_load)(void*, const trinity_host*); void (*on_unload)(void*, const trinity_host*); void (*on_config_reload)(void*, const trinity_host*); } trinity_module_api_v1;
typedef trinity_module_api_v1 trinity_module_api;
typedef int32_t (*trinity_module_get_api_v1)(uint32_t requested_abi, trinity_module_api* out_api);
int32_t TrinityModuleGetApi(uint32_t requested_abi, trinity_module_api* out_api);
typedef struct trinity_server_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnNetworkStart)(void* context, trinity_host const* host);
  void (*OnNetworkStop)(void* context, trinity_host const* host);
  void (*OnSocketOpen)(void* context, trinity_host const* host, trinity_handle socket);
  void (*OnSocketClose)(void* context, trinity_host const* host, trinity_handle socket);
  void (*OnPacketSend)(void* context, trinity_host const* host, trinity_handle session, trinity_handle packet);
  void (*OnPacketReceive)(void* context, trinity_host const* host, trinity_handle session, trinity_handle packet);
} trinity_server_callbacks;
typedef struct trinity_world_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnOpenStateChange)(void* context, trinity_host const* host, uint8_t open);
  void (*OnConfigLoad)(void* context, trinity_host const* host, uint8_t reload);
  void (*OnMotdChange)(void* context, trinity_host const* host, trinity_handle newMotd);
  void (*OnShutdownInitiate)(void* context, trinity_host const* host, int32_t code, int32_t mask);
  void (*OnShutdownCancel)(void* context, trinity_host const* host);
  void (*OnUpdate)(void* context, trinity_host const* host, uint32_t diff);
  void (*OnStartup)(void* context, trinity_host const* host);
  void (*OnShutdown)(void* context, trinity_host const* host);
} trinity_world_callbacks;
typedef struct trinity_formula_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnHonorCalculation)(void* context, trinity_host const* host, float* honor, uint8_t level, float multiplier);
  void (*OnGrayLevelCalculation)(void* context, trinity_host const* host, uint8_t* grayLevel, uint8_t playerLevel);
  void (*OnColorCodeCalculation)(void* context, trinity_host const* host, trinity_handle color, uint8_t playerLevel, uint8_t mobLevel);
  void (*OnZeroDifferenceCalculation)(void* context, trinity_host const* host, uint8_t* diff, uint8_t playerLevel);
  void (*OnBaseGainCalculation)(void* context, trinity_host const* host, uint32_t* gain, uint8_t playerLevel, uint8_t mobLevel);
  void (*OnGainCalculation)(void* context, trinity_host const* host, uint32_t* gain, trinity_handle player, trinity_handle unit);
  void (*OnGroupRateCalculation)(void* context, trinity_host const* host, float* rate, uint32_t count, uint8_t isRaid);
} trinity_formula_callbacks;
typedef struct trinity_map_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnCreate)(void* context, trinity_host const* host, trinity_handle map);
  void (*OnDestroy)(void* context, trinity_host const* host, trinity_handle map);
  void (*OnPlayerEnter)(void* context, trinity_host const* host, trinity_handle map, trinity_handle player);
  void (*OnPlayerLeave)(void* context, trinity_host const* host, trinity_handle map, trinity_handle player);
  void (*OnUpdate)(void* context, trinity_host const* host, trinity_handle map, uint32_t diff);
} trinity_map_callbacks;
typedef struct trinity_world_map_callbacks { uint32_t size; void* context; trinity_host const* host;
} trinity_world_map_callbacks;
typedef struct trinity_instance_map_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetInstanceScript)(void* context, trinity_host const* host, trinity_handle map);
} trinity_instance_map_callbacks;
typedef struct trinity_battleground_map_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetBattlegroundScript)(void* context, trinity_host const* host, trinity_handle map);
} trinity_battleground_map_callbacks;
typedef struct trinity_item_callbacks { uint32_t size; void* context; trinity_host const* host;
  uint8_t (*OnQuestAccept)(void* context, trinity_host const* host, trinity_handle player, trinity_handle item, trinity_handle quest);
  uint8_t (*OnUse)(void* context, trinity_host const* host, trinity_handle player, trinity_handle item, trinity_handle targets, int32_t castId);
  uint8_t (*OnExpire)(void* context, trinity_host const* host, trinity_handle player, trinity_handle proto);
  uint8_t (*OnRemove)(void* context, trinity_host const* host, trinity_handle player, trinity_handle item);
  uint8_t (*OnCastItemCombatSpell)(void* context, trinity_host const* host, trinity_handle player, trinity_handle victim, trinity_handle spellInfo, trinity_handle item);
} trinity_item_callbacks;
typedef struct trinity_unit_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnHeal)(void* context, trinity_host const* host, trinity_handle healer, trinity_handle reciever, uint32_t* gain);
  void (*OnDamage)(void* context, trinity_host const* host, trinity_handle attacker, trinity_handle victim, uint32_t* damage);
  void (*ModifyPeriodicDamageAurasTick)(void* context, trinity_host const* host, trinity_handle target, trinity_handle attacker, uint32_t* damage);
  void (*ModifyMeleeDamage)(void* context, trinity_host const* host, trinity_handle target, trinity_handle attacker, uint32_t* damage);
  void (*ModifySpellDamageTaken)(void* context, trinity_host const* host, trinity_handle target, trinity_handle attacker, int32_t* damage, trinity_handle spellInfo);
} trinity_unit_callbacks;
typedef struct trinity_creature_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetAI)(void* context, trinity_host const* host, trinity_handle creature);
} trinity_creature_callbacks;
typedef struct trinity_game_object_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetAI)(void* context, trinity_host const* host, trinity_handle go);
} trinity_game_object_callbacks;
typedef struct trinity_area_trigger_callbacks { uint32_t size; void* context; trinity_host const* host;
  uint8_t (*OnTrigger)(void* context, trinity_host const* host, trinity_handle player, trinity_handle trigger);
  uint8_t (*OnExit)(void* context, trinity_host const* host, trinity_handle player, trinity_handle trigger);
} trinity_area_trigger_callbacks;
typedef struct trinity_only_once_area_trigger_callbacks { uint32_t size; void* context; trinity_host const* host;
  uint8_t (*OnTrigger)(void* context, trinity_host const* host, trinity_handle player, trinity_handle trigger);
  uint8_t (*OnExit)(void* context, trinity_host const* host, trinity_handle player, trinity_handle trigger);
  uint8_t (*TryHandleOnce)(void* context, trinity_host const* host, trinity_handle player, trinity_handle trigger);
} trinity_only_once_area_trigger_callbacks;
typedef struct trinity_battlefield_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetBattlefield)(void* context, trinity_host const* host, trinity_handle map);
} trinity_battlefield_callbacks;
typedef struct trinity_outdoor_pvp_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetOutdoorPvP)(void* context, trinity_host const* host, trinity_handle map);
} trinity_outdoor_pvp_callbacks;
typedef struct trinity_command_callbacks { uint32_t size; void* context; trinity_host const* host;
  int32_t (*GetCommands)(void* context, trinity_host const* host);
} trinity_command_callbacks;
typedef struct trinity_weather_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnChange)(void* context, trinity_host const* host, trinity_handle weather, int32_t state, float grade);
  void (*OnUpdate)(void* context, trinity_host const* host, trinity_handle weather, uint32_t diff);
} trinity_weather_callbacks;
typedef struct trinity_auction_house_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnAuctionAdd)(void* context, trinity_host const* host, trinity_handle ah, trinity_handle auction);
  void (*OnAuctionRemove)(void* context, trinity_host const* host, trinity_handle ah, trinity_handle auction);
  void (*OnAuctionSuccessful)(void* context, trinity_host const* host, trinity_handle ah, trinity_handle auction);
  void (*OnAuctionExpire)(void* context, trinity_host const* host, trinity_handle ah, trinity_handle auction);
} trinity_auction_house_callbacks;
typedef struct trinity_condition_callbacks { uint32_t size; void* context; trinity_host const* host;
  uint8_t (*OnConditionCheck)(void* context, trinity_host const* host, trinity_handle condition, trinity_handle sourceInfo);
} trinity_condition_callbacks;
typedef struct trinity_vehicle_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnInstall)(void* context, trinity_host const* host, trinity_handle veh);
  void (*OnUninstall)(void* context, trinity_host const* host, trinity_handle veh);
  void (*OnReset)(void* context, trinity_host const* host, trinity_handle veh);
  void (*OnInstallAccessory)(void* context, trinity_host const* host, trinity_handle veh, trinity_handle accessory);
  void (*OnAddPassenger)(void* context, trinity_host const* host, trinity_handle veh, trinity_handle passenger, int8_t seatId);
  void (*OnRemovePassenger)(void* context, trinity_host const* host, trinity_handle veh, trinity_handle passenger);
} trinity_vehicle_callbacks;
typedef struct trinity_dynamic_object_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnUpdate)(void* context, trinity_host const* host, trinity_handle obj, uint32_t diff);
} trinity_dynamic_object_callbacks;
typedef struct trinity_transport_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnAddPassenger)(void* context, trinity_host const* host, trinity_handle transport, trinity_handle player);
  void (*OnAddCreaturePassenger)(void* context, trinity_host const* host, trinity_handle transport, trinity_handle creature);
  void (*OnRemovePassenger)(void* context, trinity_host const* host, trinity_handle transport, trinity_handle player);
  void (*OnRelocate)(void* context, trinity_host const* host, trinity_handle transport, uint32_t mapId, float x, float y, float z);
  void (*OnUpdate)(void* context, trinity_host const* host, trinity_handle transport, uint32_t diff);
} trinity_transport_callbacks;
typedef struct trinity_achievement_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnCompleted)(void* context, trinity_host const* host, trinity_handle player, trinity_handle achievement);
} trinity_achievement_callbacks;
typedef struct trinity_achievement_criteria_callbacks { uint32_t size; void* context; trinity_host const* host;
  uint8_t (*OnCheck)(void* context, trinity_host const* host, trinity_handle source, trinity_handle target);
} trinity_achievement_criteria_callbacks;
typedef struct trinity_player_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnPVPKill)(void* context, trinity_host const* host, trinity_handle killer, trinity_handle killed);
  void (*OnCreatureKill)(void* context, trinity_host const* host, trinity_handle killer, trinity_handle killed);
  void (*OnPlayerKilledByCreature)(void* context, trinity_host const* host, trinity_handle killer, trinity_handle killed);
  void (*OnLevelChanged)(void* context, trinity_host const* host, trinity_handle player, uint8_t oldLevel);
  void (*OnFreeTalentPointsChanged)(void* context, trinity_host const* host, trinity_handle player, uint32_t points);
  void (*OnTalentsReset)(void* context, trinity_host const* host, trinity_handle player, uint8_t involuntarily);
  void (*OnMoneyChanged)(void* context, trinity_host const* host, trinity_handle player, int64_t* amount);
  void (*OnMoneyLimit)(void* context, trinity_host const* host, trinity_handle player, int64_t amount);
  void (*OnGiveXP)(void* context, trinity_host const* host, trinity_handle player, uint32_t* amount, trinity_handle victim);
  void (*OnReputationChange)(void* context, trinity_host const* host, trinity_handle player, uint32_t factionId, int32_t* standing, uint8_t incremental);
  void (*OnDuelRequest)(void* context, trinity_host const* host, trinity_handle target, trinity_handle challenger);
  void (*OnDuelStart)(void* context, trinity_host const* host, trinity_handle player1, trinity_handle player2);
  void (*OnDuelEnd)(void* context, trinity_host const* host, trinity_handle winner, trinity_handle loser, int32_t type);
  void (*OnChat)(void* context, trinity_host const* host, trinity_handle player, uint32_t type, uint32_t lang, trinity_handle msg);
  void (*OnChat_2)(void* context, trinity_host const* host, trinity_handle player, uint32_t type, uint32_t lang, trinity_handle msg, trinity_handle receiver);
  void (*OnChat_3)(void* context, trinity_host const* host, trinity_handle player, uint32_t type, uint32_t lang, trinity_handle msg, trinity_handle group);
  void (*OnChat_4)(void* context, trinity_host const* host, trinity_handle player, uint32_t type, uint32_t lang, trinity_handle msg, trinity_handle guild);
  void (*OnChat_5)(void* context, trinity_host const* host, trinity_handle player, uint32_t type, uint32_t lang, trinity_handle msg, trinity_handle channel);
  void (*OnTextEmote)(void* context, trinity_host const* host, trinity_handle player, uint32_t textEmote, uint32_t emoteNum, int32_t guid);
  void (*OnSpellCast)(void* context, trinity_host const* host, trinity_handle player, trinity_handle spell, uint8_t skipCheck);
  void (*OnLogin)(void* context, trinity_host const* host, trinity_handle player, uint8_t firstLogin);
  void (*OnLogout)(void* context, trinity_host const* host, trinity_handle player);
  void (*OnCreate)(void* context, trinity_host const* host, trinity_handle player);
  void (*OnDelete)(void* context, trinity_host const* host, int32_t guid, uint32_t accountId);
  void (*OnFailedDelete)(void* context, trinity_host const* host, int32_t guid, uint32_t accountId);
  void (*OnSave)(void* context, trinity_host const* host, trinity_handle player);
  void (*OnBindToInstance)(void* context, trinity_host const* host, trinity_handle player, int32_t difficulty, uint32_t mapId, uint8_t permanent, uint8_t extendState);
  void (*OnUpdateZone)(void* context, trinity_host const* host, trinity_handle player, uint32_t newZone, uint32_t newArea);
  void (*OnMapChanged)(void* context, trinity_host const* host, trinity_handle player);
  void (*OnQuestStatusChange)(void* context, trinity_host const* host, trinity_handle player, uint32_t questId);
  void (*OnPlayerRepop)(void* context, trinity_host const* host, trinity_handle player);
  void (*OnMovieComplete)(void* context, trinity_host const* host, trinity_handle player, uint32_t movieId);
} trinity_player_callbacks;
typedef struct trinity_account_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnAccountLogin)(void* context, trinity_host const* host, uint32_t accountId);
  void (*OnFailedAccountLogin)(void* context, trinity_host const* host, uint32_t accountId);
  void (*OnEmailChange)(void* context, trinity_host const* host, uint32_t accountId);
  void (*OnFailedEmailChange)(void* context, trinity_host const* host, uint32_t accountId);
  void (*OnPasswordChange)(void* context, trinity_host const* host, uint32_t accountId);
  void (*OnFailedPasswordChange)(void* context, trinity_host const* host, uint32_t accountId);
} trinity_account_callbacks;
typedef struct trinity_guild_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnAddMember)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle player, uint8_t plRank);
  void (*OnRemoveMember)(void* context, trinity_host const* host, trinity_handle guild, int32_t guid, uint8_t isDisbanding, uint8_t isKicked);
  void (*OnMOTDChanged)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle newMotd);
  void (*OnInfoChanged)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle newInfo);
  void (*OnCreate)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle leader, trinity_handle name);
  void (*OnDisband)(void* context, trinity_host const* host, trinity_handle guild);
  void (*OnMemberWitdrawMoney)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle player, uint64_t* amount, uint8_t isRepair);
  void (*OnMemberDepositMoney)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle player, uint64_t* amount);
  void (*OnItemMove)(void* context, trinity_host const* host, trinity_handle guild, trinity_handle player, trinity_handle pItem, uint8_t isSrcBank, uint8_t srcContainer, uint8_t srcSlotId, uint8_t isDestBank, uint8_t destContainer, uint8_t destSlotId);
  void (*OnEvent)(void* context, trinity_host const* host, trinity_handle guild, uint8_t eventType, int32_t playerGuid1, int32_t playerGuid2, uint8_t newRank);
  void (*OnBankEvent)(void* context, trinity_host const* host, trinity_handle guild, uint8_t eventType, uint8_t tabId, int32_t playerGuid, uint64_t itemOrMoney, uint16_t itemStackCount, uint8_t destTabId);
} trinity_guild_callbacks;
typedef struct trinity_group_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnAddMember)(void* context, trinity_host const* host, trinity_handle group, int32_t guid);
  void (*OnInviteMember)(void* context, trinity_host const* host, trinity_handle group, int32_t guid);
  void (*OnRemoveMember)(void* context, trinity_host const* host, trinity_handle group, int32_t guid, int32_t method, int32_t kicker, int8_t const* reason);
  void (*OnChangeLeader)(void* context, trinity_host const* host, trinity_handle group, int32_t newLeaderGuid, int32_t oldLeaderGuid);
  void (*OnDisband)(void* context, trinity_host const* host, trinity_handle group);
} trinity_group_callbacks;
typedef struct trinity_area_trigger_entity_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetAI)(void* context, trinity_host const* host, trinity_handle at);
} trinity_area_trigger_entity_callbacks;
typedef struct trinity_conversation_callbacks { uint32_t size; void* context; trinity_host const* host;
  trinity_handle (*GetAI)(void* context, trinity_host const* host, trinity_handle conversation);
} trinity_conversation_callbacks;
typedef struct trinity_scene_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnSceneStart)(void* context, trinity_host const* host, trinity_handle player, uint32_t sceneInstanceID, trinity_handle sceneTemplate);
  void (*OnSceneTriggerEvent)(void* context, trinity_host const* host, trinity_handle player, uint32_t sceneInstanceID, trinity_handle sceneTemplate, trinity_handle triggerName);
  void (*OnSceneCancel)(void* context, trinity_host const* host, trinity_handle player, uint32_t sceneInstanceID, trinity_handle sceneTemplate);
  void (*OnSceneComplete)(void* context, trinity_host const* host, trinity_handle player, uint32_t sceneInstanceID, trinity_handle sceneTemplate);
} trinity_scene_callbacks;
typedef struct trinity_quest_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnQuestStatusChange)(void* context, trinity_host const* host, trinity_handle player, trinity_handle quest, int32_t oldStatus, int32_t newStatus);
  void (*OnAcknowledgeAutoAccept)(void* context, trinity_host const* host, trinity_handle player, trinity_handle quest);
  void (*OnQuestObjectiveChange)(void* context, trinity_host const* host, trinity_handle player, trinity_handle quest, trinity_handle objective, int32_t oldAmount, int32_t newAmount);
} trinity_quest_callbacks;
typedef struct trinity_world_state_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnValueChange)(void* context, trinity_host const* host, int32_t worldStateId, int32_t oldValue, int32_t newValue, trinity_handle map);
} trinity_world_state_callbacks;
typedef struct trinity_event_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnTrigger)(void* context, trinity_host const* host, trinity_handle object, trinity_handle invoker, uint32_t eventId);
} trinity_event_callbacks;
typedef struct trinity_player_choice_callbacks { uint32_t size; void* context; trinity_host const* host;
  void (*OnResponse)(void* context, trinity_host const* host, trinity_handle object, trinity_handle player, trinity_handle choice, trinity_handle response, uint16_t clientIdentifier);
} trinity_player_choice_callbacks;
#ifdef __cplusplus
}
#endif
#endif
