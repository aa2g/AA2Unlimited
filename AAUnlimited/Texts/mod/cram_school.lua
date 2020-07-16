--@INFO cram school high poly lighting controls

local _M = {}
local opts = {}

local lightingPresets = {
	"lighting_night_full",
	"lighting_night_subtle",
}

--------------------------------------------------------------------------------------------------------------------------
-- Event handlers --------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

function on.roomChange(seat, room, action)
	local os = require("os");
	local setsPath = ".\\data\\sets\\!lighting_night\\";
	for i=0,24 do
		local character = GetCharInstData(i);
		if (character ~= nil and character:IsPC()) then
			-- reset all lighting presets
			for i,v in pairs(lightingPresets) do
				if(os.rename(setsPath .. "jg2p01_00_00", setsPath .. v)) then
					break;
				end
			end

			if (isRoomInterior(character:GetCurrentRoom())) then
				os.rename(setsPath .. "lighting_night_subtle", setsPath .. "jg2p01_00_00");
			else
				os.rename(setsPath .. "lighting_night_full", setsPath .. "jg2p01_00_00");
			end	
			return;
		end
	end
end

function on.period()
	local os = require("os");
	local setsPath = ".\\data\\sets\\!lighting_night\\";
	for i=0,24 do
		local character = GetCharInstData(i);
		if (character ~= nil and character:IsPC()) then
			-- reset all lighting presets
			for i,v in pairs(lightingPresets) do
				if(os.rename(setsPath .. "jg2p01_00_00", setsPath .. v)) then
					break;
				end
			end

			if (isRoomInterior(character:GetCurrentRoom())) then
				os.rename(setsPath .. "lighting_night_subtle", setsPath .. "jg2p01_00_00");
			else
				os.rename(setsPath .. "lighting_night_full", setsPath .. "jg2p01_00_00");
			end	
			return;
		end
	end
end
--------------------------------------------------------------------------------------------------------------------------

function getRoomName(idxRoom)
	local rooms = {
		"School gates",
		"Back street",
		"Outside gymnasium",
		"School route",
		"Mens changing room",
		"Girls changing room",
		"Mens shower",
		"Girls shower",
		"Lockers",
		"Outside lounge",
		"Outside toilets",
		"Outside classroom",
		"Rooftop access",
		"Old building 1st floor",
		"Old building 2nd floor",
		"Old building 3rd floor",
		"Teachers lounge",
		"Infirmary",
		"Library",
		"Classroom",
		"Mens Toilets",
		"Girls Toilets",
		"Rooftop",
		"Outside counsel",
		"Outside cafeteria",
		"Courtyard",
		"2nd floor hallway",
		"3rd floor passage",
		"Swimming pool",
		"Track",
		"Sports facility",
		"Dojo",
		"Gymnasium",
		"Arts room",
		"Multipurpose room",
		"Japanese room",
		"Behind Dojo",
		"Outside dojo",
		"Cafeteria",
		"Outside Station",
		"Karaoke",
		"Boys' night room",		--	probably nonexistant according to backgrounds
		"Girls' night room",	--	probably nonexistant according to backgrounds
		"Boys' room",
		"Girls' room",
		"Boys's Shower Stall",
		"Girl's Shower Stall",
		"Boys' Toilet Stall",
		"Girls' Toilet Stall",
		"Counseling Room",
		"Gym Storeroom",
		"Love Hotel",
		"Machine Room",
	};
	return rooms[idxRoom + 1];
end

function isRoomInterior(idxRoom)
	local	rooms = {};
			rooms["School gates"] = false;
			rooms["Back street"] = false;
			rooms["Outside gymnasium"] = false;
			rooms["School route"] = false;
			rooms["Mens changing room"] = true;
			rooms["Girls changing room"] = true;
			rooms["Mens shower"] = true;
			rooms["Girls shower"] = true;
			rooms["Lockers"] = true;
			rooms["Outside lounge"] = true;
			rooms["Outside toilets"] = true;
			rooms["Outside classroom"] = true;
			rooms["Rooftop access"] = true;
			rooms["Old building 1st floor"] = true;
			rooms["Old building 2nd floor"] = true;
			rooms["Old building 3rd floor"] = true;
			rooms["Teachers lounge"] = true;
			rooms["Infirmary"] = true;
			rooms["Library"] = true;
			rooms["Classroom"] = true;
			rooms["Mens Toilets"] = true;
			rooms["Girls Toilets"] = true;
			rooms["Rooftop"] = false;
			rooms["Outside counsel"] = true;
			rooms["Outside cafeteria"] = false;
			rooms["Courtyard"] = false;
			rooms["2nd floor hallway"] = true;
			rooms["3rd floor passage"] = false;
			rooms["Swimming pool"] = false;
			rooms["Track"] = false;
			rooms["Sports facility"] = false;
			rooms["Dojo"] = true;
			rooms["Gymnasium"] = true;
			rooms["Arts room"] = true;
			rooms["Multipurpose room"] = true;
			rooms["Japanese room"] = true;
			rooms["Behind Dojo"] = false;
			rooms["Outside dojo"] = false;
			rooms["Cafeteria"] = true;
			rooms["Outside Station"] = true;
			rooms["Karaoke"] = true;
			rooms["Boys' night room"] = true;		--	probably nonexistant according to backgrounds
			rooms["Girls' night room"] = true;		--	probably nonexistant according to backgrounds
			rooms["Boys' room"] = true;
			rooms["Girls' room"] = true;
			rooms["Boys's Shower Stall"] = true;
			rooms["Girl's Shower Stall"] = true;
			rooms["Boys' Toilet Stall"] = true;
			rooms["Girls' Toilet Stall"] = true;
			rooms["Counseling Room"] = true;
			rooms["Gym Storeroom"] = true;
			rooms["Love Hotel"] = true;
			rooms["Machine Room"] = true;
	local roomName = getRoomName(idxRoom);
	return rooms[roomName];
end

--------------------------------------------------------------------------------------------------------------------------

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "Jizou options")
end

return _M