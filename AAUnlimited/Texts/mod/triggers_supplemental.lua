--@INFO Supplemental triggers code

local _M = {}
local opts = {}

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
		local murderKey = getCardStorageKey(victim.m_char.m_seat) .. "'s murderer";
		setClassStorage(murderKey, murderer.m_char.m_seat);
	end
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