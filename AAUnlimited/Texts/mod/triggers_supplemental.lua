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
	local case = getClassStorage("Latest murder case");
	detectiveTakeClassSnapshot(case);
	-- detectivePrintAllReports(case);
end

function detectiveDetermineTheMurderer(actor0, actor1)
	local victim = 25;
	local murderer = 25;
	local storage = {};

	if (prevActions[actor0 + 1] == MURDER or prevActions[actor0 + 1] == TOGETHER_FOREVER) then
		victim = GetCharInstData(actor1);
		murderer = GetCharInstData(actor0);
	end
	if (prevActions[actor1 + 1] == MURDER or prevActions[actor1 + 1] == TOGETHER_FOREVER) then
		victim = GetCharInstData(actor0);
		murderer = GetCharInstData(actor1);
	end
	
	if (victim ~= 25 and murderer ~= 25) then
		local case = getCardStorageKey(victim.m_char.m_seat) .. "\'s murder case";
		local latestMurderCase = getClassStorage("Latest murder case");
		if (latestMurderCase ~= case) then
			setClassStorage("Latest murder case", case);
			setClassStorage("Latest murdered seat", victim.m_char.m_seat);
		else
			setClassStorage("Latest murder case", "CLOSED");	-- there's no case if the killer wac caught or victim survived
		end
		storage["victim"] = getCardStorageKey(victim.m_char.m_seat);
		storage["murderer"] = getCardStorageKey(murderer.m_char.m_seat);
		setClassStorage(case, storage);
	else
		setClassStorage("Latest murder case", "CLOSED");	-- there's no case if the killer was caught or victim survived		
	end
end

function detectiveTakeClassSnapshot(case)
	local snapshotStorage = {};
	snapshotStorage.classMembers = {};	--	list of class members at the time of murder
	for i=0,24 do
		local character = GetCharInstData(i);
		if (character ~= nil) then
			local storageLovers = {};
			local storageLOVE = {};
			local storageLIKE = {};
			local storageDISLIKE = {};
			local storateHATE = {};
			local charKey = getCardStorageKey(character.m_char.m_seat);
			snapshotStorage.classMembers["" .. i] = charKey;

			local currentAction = character.m_char:GetActivity().m_currConversationId;
			if (currentAction == 4294967295) then
				currentAction = -1;
			end
			snapshotStorage[charKey .. "\'s current action"] = currentAction;			
			local currentState = character.m_char.m_characterStatus.m_npcStatus.m_status;
			if (currentState == 4294967295) then
				currentState = -1;
			end
			snapshotStorage[charKey .. "\'s movement state"] = character.m_char.m_characterStatus.m_npcStatus.m_status;
			snapshotStorage[charKey .. "\'s current room"] = character:GetCurrentRoom();
			for j=0,24 do
				if j == i then goto jloopcontinue end
				local towards = GetCharInstData(j);
				if (towards ~= nil) then

					local towardsKey = getCardStorageKey(j);
					if (towards.m_char.m_npcData == character.m_char.m_npcData.m_target) then
						snapshotStorage[charKey .. "\'s target"] = towardsKey;
					end
					if (character.m_char:m_lovers(j) == 1) then
						storageLovers[towardsKey] = true;
					end
					-- LLDH
					storageLOVE[towardsKey] = ter:GetLoveTowards(towards);
					storageLIKE[towardsKey] = ter:GetLikeTowards(towards);
					storageDISLIKE[towardsKey] = ter:GetDislikeTowards(towards);
					storateHATE[towardsKey] = ter:GetHateTowards(towards);
				end
				::jloopcontinue::
			end
			snapshotStorage[charKey .. "\'s lovers"] = storageLovers;
			snapshotStorage[charKey .. "\'s LOVE"] = storageLOVE;
			snapshotStorage[charKey .. "\'s LIKE"] = storageLIKE;
			snapshotStorage[charKey .. "\'s DISLIKE"] = storageDISLIKE;
			snapshotStorage[charKey .. "\'s HATE"] = storateHATE;
		end
	end

	setClassStorage(case, snapshotStorage);
end

function detectivePrintAllReports(case)
	for seat=0,24 do
		local testifier = GetCharInstData(seat);
		if (testifier ~= nil) then
			local json = require "json";
			log.info(getCardStorageKey(seat) .. "'s alibi report: \n" .. json.encode(detectiveCompileAlibiReport(seat, case)));
			log.info(getCardStorageKey(seat) .. "'s intrigue report: \n" .. json.encode(detectiveCompileIntrigueReport(seat, case)));
			log.info(getCardStorageKey(seat) .. "'s trivia report: \n" .. json.encode(detectiveCompileTriviaReport(seat, case)));
		end
	end
end

function detectiveCompileAlibiReport(testifier, case)
	local storage = getClassStorage(case);
	local victim= case.victim;
	local murderer= case.murderer;

	local line = 0;
	local alibiReport = {};	-- where all the alibi report data is gonna be stored

	local myselfInst = GetCharInstData(testifier);
	local myKey = getCardStorageKey(testifier);
	
	-- I was in <room>
	local alibiMyself = "I was in " .. getRoomName(storage[getCardStorageKey(myselfInst.m_char.m_seat) .. "\'s current room"]) .. " at the time\n";
	if (myKey == murderer) then
		-- lie about my involvement
		alibiMyself = "I was in " .. getRoomName(getRandomRoom(storage[getCardStorageKey(myselfInst.m_char.m_seat) .. "\'s current room"])) .. " at the time\n";
		alibiMyself = alibiMyself .. "I was trying to " .. getActionName(-1);
	else
		-- I was doing <action> / talking to <target> about <action>
		local myTarget = storage[myKey .. "\'s target"];
		local myAction = storage[myKey .. "\'s current action"];
		if (myTarget ~= nil) then
			local targetsTarget = storage[myTarget .. "\'s target"];
			if (targetsTarget == myKey) then
				alibiMyself = alibiMyself .. "I was talking to " .. myTarget .. " about " .. getActionName(myAction); 		
			else
				alibiMyself = alibiMyself .. "I was going to talk to " .. myTarget .. " about " .. getActionName(myAction); 	
			end
		else
			alibiMyself = alibiMyself .. "I was trying to " .. getActionName(myAction);
		end
	end
	line = line + 1;
	alibiReport[line] = alibiMyself;
		
	-- <student> was in the same room, doing <action> / talking to <target> about <action>
	-- avoid talking about the victim and the murderer
	for i=0,24 do
		local X = storage.classMembers["" .. i];
		if (X ~= nil and X ~= myKey and X ~= victim and X ~= murderer) then
			if (storage[X .. "\'s current room"] == storage[myKey .. "\'s current room"]) then
				local targetX = storage[X .. "\'s target"];
				local actionX = storage[X .. "\'s current action"];
				local alibiX = X .. " was in the same room,";
				if (targetX ~= nil) then
					local targetXsTarget = storage[targetX .. "\'s target"];
					if (targetXsTarget == X) then
						alibiX = alibiX .. " talking to " .. targetX .. " about " .. getActionName(actionX);
					else
						alibiX = alibiX .. " trying to talk to " .. targetX .. " about " .. getActionName(actionX);
					end
				else
					alibiX = alibiX .. " trying to " .. getActionName(actionX);
				end
				line = line + 1;
				alibiReport[line] = alibiX;
			end
		end
	end
	
	return alibiReport;
end

function detectiveCompileIntrigueReport(testifier, case)
	local math = require "math";

	local storage = getClassStorage(case);
	local victim = case.victim;
	local murderer = case.murderer;

	local line = 0;
	local intrigueReport = {};	-- where all the intrigue report data is gonna be stored

	local myselfInst = GetCharInstData(testifier);
	local myKey = getCardStorageKey(testifier);
			
	for i=0,24 do
		local X = storage.classMembers["" .. i];
		if (X ~= nil and X ~= myKey) then
			local storageLOVE = storage[X .. "\'s LOVE"];
			local storageLIKE = storage[X .. "\'s LIKE"];
			local storageDISLIKE = storage[X .. "\'s DISLIKE"];
			local storateHATE = storage[X .. "\'s HATE"];

			local LoveLike = storageLOVE[myKey] + storageLIKE[myKey];
			local DislikeHate = storageDISLIKE[myKey] + storateHATE[myKey];

			if (LoveLike > DislikeHate) then
				for j=0,24 do
					local Y = storage.classMembers["" .. j];
					if (Y ~= nil and i ~= j and Y ~= myKey) then	--	don't talk about myself
						-- Apparently, <student X> <LOVED/LIKED/DISLIKED/HATED> <student Y>
						local LOVE_X_Y = storageLOVE[Y];
						local LIKE_X_Y = storageLIKE[Y];
						local DISLIKE_X_Y = storageDISLIKE[Y];
						local HATE_X_Y = storateHATE[Y];

						local MAX_X_Y = math.max(LOVE_X_Y, math.max(LIKE_X_Y, math.max(DISLIKE_X_Y, math.max(HATE_X_Y))));
						if (MAX_X_Y == LIKE_X_Y and MAX_X_Y >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " liked " .. Y;
						end
						if (MAX_X_Y == DISLIKE_X_Y and MAX_X_Y >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " disliked " .. Y;
						end
						if (MAX_X_Y == LOVE_X_Y and MAX_X_Y >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " loved " .. Y;
						end
						if (MAX_X_Y == HATE_X_Y and MAX_X_Y >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " hated " .. Y;
						end

						-- Apparently, <student X> felt <LOVED/LIKED/DISLIKED/HATED> by <student Y>						
						local LOVE_Y_X = storage[Y .. "\'s LOVE"][X];
						local LIKE_Y_X = storage[Y .. "\'s LIKE"][X];
						local DISLIKE_Y_X = storage[Y .. "\'s DISLIKE"][X];
						local HATE_Y_X = storage[Y .. "\'s HATE towards"][X];

						local MAX_Y_X = math.max(LOVE_Y_X, math.max(LIKE_Y_X, math.max(DISLIKE_Y_X, math.max(HATE_Y_X))));
						if (MAX_Y_X == LIKE_Y_X and MAX_Y_X >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " felt liked by " .. Y;
						end
						if (MAX_Y_X == DISLIKE_Y_X and MAX_Y_X >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " felt disliked by " .. Y;
						end
						if (MAX_Y_X == LOVE_Y_X and MAX_Y_X >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " felt loved by " .. Y;
						end
						if (MAX_Y_X == HATE_Y_X and MAX_Y_X >= 30) then
							line = line + 1;
							intrigueReport[line] = "Apparently, " .. X .. " felt hated by " .. Y;
						end						
					end
				end				
			end
		end
	end

	return intrigueReport;
end

function detectiveCompileTriviaReport(testifier, case)
	local storage = getClassStorage(case);
	local victim = case.victim;
	local murderer = case.murderer;

	local line = 0;
	local triviaReport = {};	-- where all the alibi report data is gonna be stored

	local myselfInst = GetCharInstData(testifier);
	local myKey = getCardStorageKey(testifier);

	for i=0,24 do
		local X = storage.classMembers["" .. i];
		if (X ~= nil and X ~= myKey) then
			local storageLovers = storage[X .. "\'s lovers"];
			local storageLOVE = storage[X .. "\'s LOVE"];
			local storageLIKE = storage[X .. "\'s LIKE"];
			local storageDISLIKE = storage[X .. "\'s DISLIKE"];
			local storateHATE = storage[X .. "\'s HATE"];
			
			local LoveLike = storageLOVE[myKey] + storageLIKE[myKey];
			local DislikeHate = storageDISLIKE[myKey] + storateHATE[myKey];

			if (LoveLike > DislikeHate) then
				for j=0,24 do
					local Y = storage.classMembers["" .. j];
					if (Y ~= nil and i ~= j) then	--	don't talk about myself
						-- <student X> and <student Y> were dating
						local loversFlag = storageLovers[Y];
						if (loversFlag == true) then
							line = line + 1;
							triviaReport[line] = "Looks like " .. X .. " and " .. Y .. " were dating";
						end
					end
				end				
			end
		end
	end

	return triviaReport;
end

function detectiveCompileReport(case, testifier, reportKey)
	local report = {};
	if (reportKey == getCardStorageKey(testifier) .. "'s alibi report") then
		report = detectiveCompileAlibiReport(testifier, case);
	end
	if (reportKey == getCardStorageKey(testifier) .. "'s intrigue report") then
		report = detectiveCompileIntrigueReport(testifier, case);
	end
	if (reportKey == getCardStorageKey(testifier) .. "'s trivia report") then
		report = detectiveCompileTriviaReport(testifier, case);
	end
	return report;
end

function detectiveGatherReport(case, detective, testifier, reportKey)
	local math = require "math";
	local detectiveStorageKey = case .. " : " .. reportKey;
	local detectiveReport = getCardStorage(detective, detectiveStorageKey);
	local testifierReport = detectiveCompileReport(case, testifier, reportKey);
	local detectiveInst = GetCharInstData(detective);
	local testifierInst = GetCharInstData(testifier);
	local disposition = (2 * testifierInst:GetLoveTowards(detectiveInst) + testifierInst:GetLikeTowards(detectiveInst)) / 900.0;
	local result = {};
	for k,v in ipairs(detectiveReport) do
		result[v] = k;
	end
	for line in pairs(testifierReport) do
		local proc = math.random() < disposition;
		if (proc) then
			result[line] = 0;
		end
	end
	detectiveReport = {};
	local i = 0;
	for k,v in ipairs(result) do
		i = i + 1;
		detectiveReport[i] = k;
	end
	setCardStorage(detective, detectiveStorageKey, detectiveReport);
end

function on.gatherAlibiReport(case, detective, testifier)
	detectiveGatherReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s alibi report");
end

function on.gatherIntrigueReport(case, detective, testifier)
	detectiveGatherReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s intrigue report");
end

function on.gatherTriviaReport(case, detective, testifier)
	detectiveGatherReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s trivia report");
end

function printReport(case, detective, testifier, reportKey)
	local detectiveStorageKey = case .. ":" .. reportKey;
	local report = getCardStorage(detective, detectiveStorageKey);
	local strReport = "";
	for line in pairs(report) do
		strReport = strReport .. "\n" .. line;
	end
	log.info(reportKey .. ":" .. strReport);
end

function on.printAlibiReport(case, detective, testifier)
	printReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s alibi report");
end

function on.printIntrigueReport(case, detective, testifier)
	printReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s intrigue report");
end

function on.printTriviaReport(case, detective, testifier)
	printReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s trivia report");
end

--------------------------------------------------------------------------------------------------------------------------

function getStudentCount()
	local counter = 0;
	for i=0,24 do
		if (GetCharInstData(i) ~= nil) then
			counter = counter + 1;
		end
	end
	return counter;
end

function getRandomRoom(except)
	local math = require "math";
	local min = 0;
	local max = 52;
	local ret = math.random(min, max);
	return ret;
end

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

function getActionName(idxAction)
	local actions = {};
		actions[0] = "ENCOURAGE";
		actions[1] = "CALM";
		actions[2] = "PRAISE";
		actions[3] = "GRUMBLE";
		actions[4] = "APOLOGIZE";
		actions[5] = "ENCOURAGE_STUDY";
		actions[6] = "ENCOURAGE_EXERCISE";
		actions[7] = "ENCOURAGE_CLUB";
		actions[8] = "ENCOURAGE_GET_ALONG";
		actions[9] = "REPRIMAND_LEWD";
		actions[10] = "GOOD_RUMOR";
		actions[11] = "GET_ALONG_WITH";
		actions[12] = "I_WANNA_GET_ALONG_WITH";
		actions[13] = "BAD_RUMOR";
		actions[14] = "DO_YOU_LIKE";
		actions[15] = "TALK_LIFE";
		actions[16] = "TALK_HOBBIES";
		actions[17] = "TALK_FOOD";
		actions[18] = "TALK_LOVE";
		actions[19] = "TALK_LEWD";
		actions[20] = "STUDY_TOGETHER";
		actions[21] = "EXERCISE_TOGETHER";
		actions[22] = "CLUB_TOGETHER";
		actions[23] = "MASSAGE";
		actions[24] = "GOTO_CLASS";
		actions[25] = "LUNCH_TOGETHER";
		actions[26] = "TEA_TOGETHER";
		actions[27] = "GO_HOME_TOGETHER";
		actions[28] = "GO_PLAY_TOGETHER";
		actions[29] = "GO_EAT_TOGETHER";
		actions[30] = "GO_KARAOKE_TOGETHER";
		actions[31] = "STUDY_HOME";
		actions[32] = "STUDY_HOME_H";
		actions[33] = "INSULT";
		actions[34] = "FIGHT";
		actions[35] = "FORCE_IGNORE";
		actions[36] = "FORCE_SHOW_THAT";
		actions[37] = "FORCE_PUT_THIS_ON";
		actions[38] = "FORCE_H";
		actions[39] = "MAKE_JOIN_CLUB";
		actions[40] = "ASK_DATE";
		actions[41] = "CONFESS";
		actions[42] = "ASK_COUPLE";
		actions[43] = "ASK_BREAKUP";
		actions[44] = "HEADPAT";
		actions[45] = "HUG";
		actions[46] = "KISS";
		actions[47] = "TOUCH";
		actions[48] = "NORMAL_H";
		actions[49] = "FOLLOW_ME"; 
		actions[50] = "GO_AWAY";
		actions[51] = "COME_TO"; 
		actions[52] = "NEVERMIND";
		actions[53] = "MINNA_STUDY";
		actions[54] = "MINNA_SPORTS";
		actions[55] = "MINNA_CLUB";
		actions[56] = "MINNA_LUNCH";
		actions[57] = "MINNA_REST";
		actions[58] = "MINNA_EAT";
		actions[60] = "MINNA_KARAOKE";
		actions[61] = "MINNA_BE_FRIENDLY";
		actions[62] = "MINNA_COME";
		actions[63] = "INTERRUPT_COMPETE";
		actions[64] = "INTERRUPT_WHAT_ARE_YOU_DOING";
		actions[65] = "INTERRUPT_STOP_QUARREL";
		actions[66] = "H_END";
		actions[67] = "H_NOTE";
		actions[68] = "TRY_3P";
		actions[69] = "REQUEST_MASSAGE";
		actions[70] = "REQUEST_KISS";
		actions[71] = "REQUEST_HUG";
		actions[72] = "SKIP_CLASS";
		actions[73] = "SKIP_CLASS_H";
		actions[74] = "SKIP_CLASS_SURPRISE_H";
		actions[75] = "DID_YOU_HAVE_H_WITH";
		actions[76] = "SHOW_UNDERWEAR";
		actions[77] = "DID_YOU_HAVE_H";
		actions[78] = "EXCHANGE_ITEMS";
		actions[79] = "LEWD_PROMISE";
		actions[80] = "LEWD_REWARD";
		actions[81] = "TOGETHER_FOREVER";
		actions[82] = "MURDER";
		actions[83] = "SLAP";
		actions[84] = "GOOD_MORNING_KISS";
		actions[85] = "GOOD_BYE_KISS";
		actions[86] = "NO_PROMPT_KISS";
		actions[87] = "FORCE_BREAKUP";
		actions[88] = "REVEAL_PREGNANCY";
		actions[89] = "I_WILL_CHEAT";
		actions[90] = "EXPLOITABLE_LINE";
		actions[91] = "STOP_FOLLOWING";
		actions[92] = "MURDER_NOTICE";
		actions[93] = "SOMEONE_LIKES_YOU";
		actions[94] = "SOMEONE_GOT_CONFESSED_TO";
		actions[95] = "DID_YOU_DATE_SOMEONE";
		actions[96] = "I_SAW_SOMEONE_HAVE_H";
		actions[97] = "DO_NOT_GET_INVOLVED";
		actions[98] = "SHAMELESS";
		actions[99] = "NO_PROMPT_H";
		actions[101] = "AFTER_DATE_H";
		actions[102] = "FOLLOW_ME_H";
		actions[103] = "DATE_GREETING";
		actions[105] = "CHANGE_CLOTHES";
		actions[106] = "STALK";
		actions[107] = "STALK_FROM_AFAR";
		actions[108] = "DO_STUDY";
		actions[109] = "DO_EXERCISE";
		actions[110] = "DO_CLUB";
		actions[112] = "BREAK_CHAT";
		actions[113] = "BREAK_H";
		actions[114] = "GUST_OF_WIND";
		actions[115] = "TEST_3P";
		actions[117] = "MINNA_H";
	if (idxAction < 0) then
		return "DO_NOTHING";
	end
	return actions[idxAction];
end

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