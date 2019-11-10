--@INFO Supplemental triggers code

local _M = {}
local opts = {}

--------------------------------------------------------------------------------------------------------------------------
-- Event handlers --------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

local TOGETHER_FOREVER = 81
local MURDER = 82

local prevActions = {}

-- keep note of the previous action
function on.move(params, user)
	if params.movementType == 3 then
		prevActions[user.m_seat + 1] = params.conversationId;
	end
end

-- identify the murderer
function on.murder(actor0, actor1, murder_action)
	detectiveStartTheCase(actor0, actor1);
end

--------------------------------------------------------------------------------------------------------------------------
-- Detective module supplemental functions -------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

function detectiveStartTheCase(actor0, actor1)
	detectiveDetermineTheMurderer(actor0, actor1);
	detectiveTakeClassSnapshot(getCardStorageKey(getClassStorage("Latest murder case seat")) .. "\'s murder case data");
end

function detectiveDetermineTheMurderer(actor0, actor1)
	local victim = 25;
	local murderer = 25;

	if (prevActions[actor0 + 1] == MURDER or prevActions[actor0 + 1] == TOGETHER_FOREVER) then
		victim = GetCharInstData(actor1);
		murderer = GetCharInstData(actor0);
	end
	if (prevActions[actor1 + 1] == MURDER or prevActions[actor1 + 1] == TOGETHER_FOREVER) then
		victim = GetCharInstData(actor0);
		murderer = GetCharInstData(actor1);
	end
	
	if (victim ~= 25 and murderer ~= 25) then
		setClassStorage("Latest murder case seat", victim.m_char.m_seat);
		local murderKey = getCardStorageKey(victim.m_char.m_seat) .. "'s murderer";
		setClassStorage(murderKey, murderer.m_char.m_seat);
	end	
end

function detectiveTakeClassSnapshot(snapshotKey)
	local snapshotStorage = {};
	for i=0,24 do
		local character = GetCharInstData(i);
		snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s current action"] = character.m_char.m_characterStatus.m_npcStatus.m_currConversationId;
		snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s movement state"] = character.m_char.m_characterStatus.m_npcStatus.m_status;
		if (character ~= 0) then
			for j=0,24 do
				if j == i then goto jloopcontinue end
				local towards = GetCharInstData(j);
				if (towards ~= 0) then	
					if (towards.m_char.m_npcData == character.m_char.m_npcData.m_target) then
						snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s target"] = getCardStorageKey(j);
					end
					snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. " is lovers with " .. getCardStorageKey(j)] = character.m_char:m_lovers(j);
					-- LLDH
					snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s LOVE towards " .. getCardStorageKey(j)] = character:GetLoveTowards(towards);
					snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s LIKE towards " .. getCardStorageKey(j)] = character:GetLikeTowards(towards);
					snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s DISLIKE towards " .. getCardStorageKey(j)] = character:GetDislikeTowards(towards);
					snapshotStorage[getCardStorageKey(character.m_char.m_seat) .. "\'s HATE towards " .. getCardStorageKey(j)] = character:GetHateTowards(towards);
				end
				::jloopcontinue::
			end
		end
	end
	local json = require "json";
	log.info(json.encode(snapshotStorage));
	setClassStorage(snapshotKey, snapshotStorage);
end

--------------------------------------------------------------------------------------------------------------------------

function setClassStorage(key, value)
	set_class_key(key, value);
end

function getClassStorage(key)
	return get_class_key(key);
end

function getCardStorageKey(card)
	local inst = GetCharInstData(card);
	return inst.m_char.m_seat .. " " .. inst.m_char.m_charData.m_forename .. " " .. inst.m_char.m_charData.m_surname;	
end

function getCardStorage(card, key)
	return get_class_key(getCardStorageKey(card))[key];
end

function setCardStorage(card, key, value)
	local record = get_class_key(getCardStorageKey(card));
	record[key] = value;
	setClassStorage(getCardStorageKey(card), record);
end

--------------------------------------------------------------------------------------------------------------------------

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

return _M