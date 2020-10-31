--@INFO Supplemental triggers code

local _M = {}
local opts = {}
local trigger = {}

-- lookup function
function on.dispatch_trigger(name, args)
	if (trigger[name] ~= nil) then
		trigger[name](args);
	end
end

--------------------------------------------------------------------------------------------------------------------------
-- Event handlers -- These should only call other functions and not contain any logic directly ---------------------------
-- These react to actual ingame event and are not called from triggers ---------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

local TOGETHER_FOREVER = 81
local MURDER = 82

local prevActions = {}

function on.move(params, user)
	trackPrevAction(params, user);
end

function on.card_expelled(actor0, actor1, murder_action)
	detectiveStartTheCase(actor0, actor1);
end

function on.period()
	detectiveCheckIfMurderFailed();
end

--------------------------------------------------------------------------------------------------------------------------
-- Helper stuffs ---------------------------------------------------------------------------------------------------------
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
		"Boys' changing room",
		"Girls' changing room",
		"Boys' shower",
		"Girls' shower",
		"Lockers",
		"Outside teachers lounge",
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
		"Boys' Toilets",
		"Girls Toilets",
		"Rooftop",
		"Outside counsel",
		"Outside cafeteria",
		"Courtyard",
		"2nd floor hallway",
		"3rd floor passage",
		"Swimming pool",
		"Track",
		"Baseball field",
		"Dojo",
		"Gymnasium",
		"Arts room",
		"Multipurpose room",
		"Japanese room",
		"Behind Dojo",
		"Outside Dojo",
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
	if (inst == nil) then
		return nil;
	end
	return inst.m_char.m_seat .. " " .. inst.m_char.m_charData.m_forename .. " " .. inst.m_char.m_charData.m_surname;	
end

function getCardStorage(card, key)
	local cardKey = getCardStorageKey(card);
	if (cardKey == nil) then
		return nil;
	end
	return get_class_key(cardKey)[key];
end

function setCardStorage(card, key, value)
	local record = get_class_key(getCardStorageKey(card));
	record[key] = value;
	setClassStorage(getCardStorageKey(card), record);
end

function getSeatFromStorageKey(key)
	local text = require("pl.text");
	local result = text.split(key, " ")[1];
	return result;
end

function splitArgs(input)
	local text = require("pl.text");
	return text.split(input, "\n");
end

function trackPrevAction(params, user)
	if params.movementType == 3 then
		prevActions[user.m_seat + 1] = params.conversationId;
	end
end

function createRelationshipPointsDump(seat, towards)
	local dump = {};
	local thisInst = GetCharInstData(seat);
	local towardsInst = GetCharInstData(towards);
	if (thisInst ~= nil and towardsInst ~= nil) then
		dump["LOVE"]	= thisInst:GetLoveTowards(towardsInst);
		dump["LIKE"]	= thisInst:GetLikeTowards(towardsInst);
		dump["DISLIKE"]	= thisInst:GetDislikeTowards(towardsInst);
		dump["HATE"]	= thisInst:GetHateTowards(towardsInst);
		dump["SPARE"]	= 900 - dump["LOVE"] - dump["LIKE"] - dump["DISLIKE"] - dump["HATE"];
	end
	return dump;
end

function restoreRelationshipPointsFromDump(seat, towards, dump)
	local thisInst = GetCharInstData(seat);
	local towardsInst = GetCharInstData(towards);
	if (thisInst == nil or towardsInst == nil or dump == {} or dump == nil) then
		return;
	end
	thisInst:SetPointsTowards(towardsInst, dump["LOVE"] or 0, dump["LIKE"] or 0, dump["DISLIKE"] or 0, dump["HATE"] or 0, dump["SPARE"] or 0);
end

--------------------------------------------------------------------------------------------------------------------------
-- Trigger procedure calls -----------------------------------------------------------------------------------------------
-- Procedures that can be called from triggers require the <trigger.> prefix ---------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

function trigger.storeRelationshipPoints(params)
	local args = splitArgs(params);
	local key = args[1];
	local storageCard = tonumber(args[2]);
	local seat = tonumber(args[3]);
	local towards = tonumber(args[4]);

	local storage = getCardStorage(storageCard, key) or {};
	local storageKey = getCardStorageKey(towards);
	if (storageKey ~= nil and seat ~= towards) then
		storage[storageKey] = createRelationshipPointsDump(seat, towards);
	end
	setCardStorage(storageCard, key, storage);
end	

function trigger.loadRelationshipPoints(params)
	local args = splitArgs(params);
	local key = args[1];
	local storageCard = tonumber(args[2]);
	local seat = tonumber(args[3]);
	local towards = tonumber(args[4]);

	local storage = getCardStorage(storageCard, key);	
	if (storage ~= nil) then
		local storageKey = getCardStorageKey(towards);
		if (storageKey ~= nil and seat ~= towards) then
			local dump = storage[storageKey];
			restoreRelationshipPointsFromDump(seat, towards, dump);
		end
	end
	storage[storageKey] = {};
	setCardStorage(storageCard, key, storage);
end

function trigger.storeAllRelationshipPoints(params)
	local args = splitArgs(params);
	local key = args[1];
	local storageCard = tonumber(args[2]);
	local seat = tonumber(args[3]);

	local storage = {};
	for towards=0,24 do
		local storageKey = getCardStorageKey(towards);
		if (storageKey ~= nil and seat ~= towards) then
			storage[storageKey] = createRelationshipPointsDump(seat, towards);
		end
	end
	setCardStorage(storageCard, key, storage);
end

function trigger.loadAllRelationshipPoints(params)
	local args = splitArgs(params);
	local key = args[1];
	local storageCard = tonumber(args[2]);
	local seat = tonumber(args[3]);

	local storage = getCardStorage(storageCard, key);
	if (storage ~= nil) then
		for towards=0,24 do
			local storageKey = getCardStorageKey(towards);
			if (storageKey ~= nil and seat ~= towards) then
				local dump = storage[storageKey];
				restoreRelationshipPointsFromDump(seat, towards, dump);
			end
		end
	end
	setCardStorage(storageCard, key, {});
end

--------------------------------------------------------------------------------------------------------------------------
-- Detective specific code -----------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

function detectiveStartTheCase(actor0, actor1)
	if (detectiveIsThereIsADetectiveInClass() == false) then	-- if there's no detective we just don't track the murder stuffs
		log.info("No detectives in class, no snapshot taken");
		return;
	end
	detectiveDetermineTheMurderer(actor0, actor1);
	local case = getClassStorage("Latest murder case");
	detectiveTakeClassSnapshot(case);
	-- detectivePrintAllReports(case);
end

function detectiveIsThereIsADetectiveInClass()
	for seat=0,24 do
		if (getCardStorage(seat, "Detective") == true) then
			return true;
		end
	end
	return false;
end

function detectiveCloseCase(case, detective)
	if (detective ~= nil) then
		setCardStorage(detective, "Murder Case", "CLOSED");
	end
	
	setClassStorage(case, {});
	setClassStorage("Latest murder case", "CLOSED");
end

function detectiveDetermineTheMurderer(actor0, actor1)
	local victim = 25;
	local murderer = 25;
	local storage = {};

	-- if (prevActions[actor0 + 1] == MURDER or prevActions[actor0 + 1] == TOGETHER_FOREVER) then
	-- 	victim = GetCharInstData(actor1);
	-- 	murderer = GetCharInstData(actor0);
	-- end
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
			storage.victim = getCardStorageKey(victim.m_char.m_seat);
			storage.murderer = getCardStorageKey(murderer.m_char.m_seat);
			setClassStorage(case, storage);
		else
			setClassStorage("Latest murder case", "CLOSED");	-- there's no case if the killer was caught or victim survived
			detectiveCloseCase(case);
			log.info("Victim survived, case closed");
		end
	else
		setClassStorage("Latest murder case", "CLOSED");
	end
end

function detectiveCheckIfMurderFailed()
	local case = getClassStorage("Latest murder case");
	if (case ~= nil and case ~= "CLOSED") then
		local murderCase = getClassStorage(case);
		local victimSeat = getClassStorage("Latest murdered seat");
		local victim = GetCharInstData(victimSeat);
		if (victim ~= nil) then	-- victim's seat is occupied
			if (murderCase.victim == getCardStorageKey(victimSeat)) then	-- victim is alive, close the case
				detectiveCloseCase(case);
			end
		end
		if (murderCase.murderer ~= getCardStorageKey(getSeatFromStorageKey(murderCase.murderer))) then
			detectiveCloseCase(case);
		end
	end
end

function detectiveTakeClassSnapshot(case)
	local snapshotStorage = getClassStorage(case);
	if (snapshotStorage == nil) then
		return;
	end;
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
					storageLOVE[towardsKey] = character:GetLoveTowards(towards);
					storageLIKE[towardsKey] = character:GetLikeTowards(towards);
					storageDISLIKE[towardsKey] = character:GetDislikeTowards(towards);
					storateHATE[towardsKey] = character:GetHateTowards(towards);
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
	local storage = getClassStorage(case) or {};
	local victim = storage.victim;
	local murderer = storage.murderer;

	local line = 0;
	local alibiReport = {};	-- where all the alibi report data is gonna be stored

	local myselfInst = GetCharInstData(testifier);
	local myKey = getCardStorageKey(testifier);
	
	-- I was in <room>
	local alibiMyself = "I was in " .. getRoomName(storage[myKey .. "\'s current room"]) .. " at the time\n";
	if (myKey == murderer) then
		-- lie about my involvement
		alibiMyself = "I was in " .. getRoomName(getRandomRoom(storage[myKey .. "\'s current room"])) .. " at the time\n";
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
	local storage = getClassStorage(case) or {};
	local victim = storage.victim;
	local murderer = storage.murderer;

	local line = 0;
	local intrigueReport = {};	-- where all the intrigue report data is gonna be stored

	local myselfInst = GetCharInstData(testifier);
	local myKey = getCardStorageKey(testifier);

	local X = victim;
	if (X ~= nil and X ~= myKey) then
		local storageLOVE = storage[X .. "\'s LOVE"];
		local storageLIKE = storage[X .. "\'s LIKE"];
		local storageDISLIKE = storage[X .. "\'s DISLIKE"];
		local storateHATE = storage[X .. "\'s HATE"];

		local LoveLike = storageLOVE[myKey] + storageLIKE[myKey];
		local DislikeHate = storageDISLIKE[myKey] + storateHATE[myKey];

		if (LoveLike >= 600) then
			for j=0,24 do
				local Y = storage.classMembers["" .. j];			
				if (Y ~= nil and Y ~= myKey and Y ~= victim) then
					-- Apparently, <student X> <LOVED/LIKED/DISLIKED/HATED> <student Y>

					local LOVE_X_Y = storageLOVE[Y];
					local LIKE_X_Y = storageLIKE[Y];
					local DISLIKE_X_Y = storageDISLIKE[Y];
					local HATE_X_Y = storateHATE[Y];

					if (LIKE_X_Y >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " liked " .. Y;
					end
					if (DISLIKE_X_Y >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " disliked " .. Y;
					end
					if (LOVE_X_Y >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " loved " .. Y;
					end
					if (HATE_X_Y >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " hated " .. Y;
					end

					-- Apparently, <student X> felt <LOVED/LIKED/DISLIKED/HATED> by <student Y>						
					local LOVE_Y_X = storage[Y .. "\'s LOVE"][X];
					local LIKE_Y_X = storage[Y .. "\'s LIKE"][X];
					local DISLIKE_Y_X = storage[Y .. "\'s DISLIKE"][X];
					local HATE_Y_X = storage[Y .. "\'s HATE"][X];

					if (LIKE_Y_X >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " felt liked by " .. Y;
					end
					if (DISLIKE_Y_X >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " felt disliked by " .. Y;
					end
					if (LOVE_Y_X >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " felt loved by " .. Y;
					end
					if (HATE_Y_X >= 600) then
						line = line + 1;
						intrigueReport[line] = "Apparently, " .. X .. " felt hated by " .. Y;
					end						
				end
			end				
		end
	end

	return intrigueReport;
end

function detectiveCompileTriviaReport(testifier, case)
	local storage = getClassStorage(case) or {};
	local victim = storage.victim;
	local murderer = storage.murderer;

	local line = 0;
	local triviaReport = {};	-- where all the trivia report data is gonna be stored

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

			if (LoveLike >= 300) then
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

			local Xinst = GetCharInstData(getSeatFromStorageKey(X));
			if (Xinst ~= nil and DislikeHate >= 300) then
				local XMurderous = getClassStorage(X)["Murderous"] or false;
				if (XMurderous) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Murderous vibe";
				end
				local XCheating = getClassStorage(X)["Cheating"] or false;
				if (XCheating) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Cheating vibe";
				end
				local XSexCrazed= getClassStorage(X)["Sex Crazed"] or false;
				if (XSexCrazed) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Sex Crazed vibe";
				end
				local XBrute= getClassStorage(X)["Brute"] or false;
				if (XBrute) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Brute vibe";
				end
				local XBlackmailer= getClassStorage(X)["Blackmailer"] or false;
				if (XBlackmailer) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Blackmailer vibe";
				end
				local XDelinquent= getClassStorage(X)["Delinquent"] or false;
				if (XDelinquent) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Delinquent vibe";
				end
				local XDominant= getClassStorage(X)["Dominant"] or false;
				if (XDominant) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Dominant vibe";
				end
				local XForceful= getClassStorage(X)["Forceful"] or false;
				if (XForceful) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Forceful vibe";
				end
				local XHomewrecker= getClassStorage(X)["Homewrecker"] or false;
				if (XHomewrecker) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Homewrecker vibe";
				end
				local XHotBody= getClassStorage(X)["Hot Body"] or false;
				if (XHotBody) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Hot Body vibe";
				end
				local XInsecure= getClassStorage(X)["Insecure"] or false;
				if (XInsecure) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Insecure vibe";
				end
				local XKidnapper= getClassStorage(X)["Kidnapper"] or false;
				if (XKidnapper) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Kidnapper vibe";
				end
				local XKiller= getClassStorage(X)["Killer"] or false;
				if (XKiller) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Killer vibe";
				end
				local XMartialArtist= getClassStorage(X)["Martial Artist"] or false;
				if (XMartialArtist) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Martial Artist vibe";
				end
				local XMoody= getClassStorage(X)["Moody"] or false;
				if (XMoody) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Moody vibe";
				end
				local XParanoid= getClassStorage(X)["Paranoid"] or false;
				if (XParanoid) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Paranoid vibe";
				end
				local XPheromones= getClassStorage(X)["Pheromones"] or false;
				if (XPheromones) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Pheromones vibe";
				end
				local XProfane= getClassStorage(X)["Profane"] or false;
				if (XProfane) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Profane vibe";
				end
				local XResentful= getClassStorage(X)["Resentful"] or false;
				if (XResentful) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Resentful vibe";
				end
				local XSadist= getClassStorage(X)["Sadist"] or false;
				if (XSadist) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Sadist vibe";
				end
				local XSaint= getClassStorage(X)["Saint"] or false;
				if (XSaint) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Saint vibe";
				end
				local XSeducer= getClassStorage(X)["Seducer"] or false;
				if (XSeducer) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Seducer vibe";
				end
				local XSexAddict= getClassStorage(X)["Sex Addict"] or false;
				if (XSexAddict) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Sex Addict vibe";
				end
				local XShaming= getClassStorage(X)["Shaming"] or false;
				if (XShaming) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Shaming vibe";
				end
				local XStalker= getClassStorage(X)["Stalker"] or false;
				if (XStalker) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Stalker vibe";
				end
				local XThief= getClassStorage(X)["Thief"] or false;
				if (XThief) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Thief vibe";
				end
				local XWhiteKnight= getClassStorage(X)["White Knight"] or false;
				if (XWhiteKnight) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this White Knight vibe";
				end
				local XEasygoing= Xinst.m_char.m_charData:m_traitBools(0);
				if (XEasygoing == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Easygoing vibe";
				end
				local XCharming= Xinst.m_char.m_charData:m_traitBools(4);
				if (XCharming == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Charming vibe";
				end
				local XJealous= Xinst.m_char.m_charData:m_traitBools(11);
				if (XJealous == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Jealous vibe";
				end
				local XPerverted= Xinst.m_char.m_charData:m_traitBools(13);
				if (XPerverted == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Perverted vibe";
				end
				local XImpulsive= Xinst.m_char.m_charData:m_traitBools(16);
				if (XImpulsive == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Impulsive vibe";
				end
				local XViolent= Xinst.m_char.m_charData:m_traitBools(18);
				if (XViolent == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Violent vibe";
				end
				local XMeddlesome= Xinst.m_char.m_charData:m_traitBools(20);
				if (XMeddlesome == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Meddlesome vibe";
				end
				local XSingleminded= Xinst.m_char.m_charData:m_traitBools(25);
				if (XSingleminded == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Singleminded vibe";
				end
				local XCompetitive= Xinst.m_char.m_charData:m_traitBools(27);
				if (XCompetitive == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Competitive vibe";
				end
				local XScheming = Xinst.m_char.m_charData:m_traitBools(28);
				if (XScheming == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Scheming vibe";
				end
				local XWild= Xinst.m_char.m_charData:m_traitBools(30);
				if (XWild == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Wild vibe";
				end
				local XEvil = Xinst.m_char.m_charData:m_traitBools(33);
				if (XEvil == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Evil vibe";
				end
				local XExploitable = Xinst.m_char.m_charData:m_traitBools(35);
				if (XExploitable == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Exploitable vibe";
				end
				local XLucky= Xinst.m_char.m_charData:m_traitBools(37);
				if (XLucky == 1) then
					line = line + 1;
					triviaReport[line] = X .. " gives off this Lucky vibe";
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
	local detectiveReport = getCardStorage(detective, detectiveStorageKey) or {};
	local testifierReport = detectiveCompileReport(case, testifier, reportKey) or {};
	local detectiveInst = GetCharInstData(detective);
	local testifierInst = GetCharInstData(testifier);
	local disposition = (2 * testifierInst:GetLoveTowards(detectiveInst) + testifierInst:GetLikeTowards(detectiveInst)) / 900.0;
	local result = {};
	for k,v in ipairs(detectiveReport) do
		result[v] = "whatever it doesn't fucking matter";
	end
	for k,v in ipairs(testifierReport) do
		local proc = math.random() < disposition;
		if (proc) then
			result[v] = "whatever it doesn't fucking matter";
		end
	end
	detectiveReport = {};
	local i = 0;
	for k,v in pairs(result) do
		i = i + 1;
		detectiveReport[i] = k;
	end
	local caseStorage = getClassStorage(case);
	setCardStorage(detective, case .. " murderer", caseStorage.murderer);
	setCardStorage(detective, detectiveStorageKey, detectiveReport);
end

function printReport(case, detective, testifier, reportKey)
	local detectiveStorageKey = case .. " : " .. reportKey;
	local report = getCardStorage(detective, detectiveStorageKey) or {};
	local strReport = "";
	for key,line in ipairs(report) do
		strReport = strReport .. "\n" .. line;
	end
	log.info(strReport);
end

-- trigger procedure calls for detective

function trigger.gatherAlibiReport(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];
	local testifier = args[3];
	detectiveGatherReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s alibi report");
end

function trigger.gatherIntrigueReport(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];
	local testifier = args[3];
	detectiveGatherReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s intrigue report");
end

function trigger.gatherTriviaReport(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];
	local testifier = args[3];
	detectiveGatherReport(case, detective, testifier, getCardStorageKey(testifier) .. "'s trivia report");
end

function trigger.printAlibiReport(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];
	local testifier = args[3];
	local reportKey = getCardStorageKey(testifier) .. "'s alibi report";
	log.info(reportKey);
	printReport(case, detective, testifier, reportKey);
end

function trigger.printIntrigueReport(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];
	local testifier = args[3];
	local reportKey = getCardStorageKey(testifier) .. "'s intrigue report";
	log.info(reportKey);
	printReport(case, detective, testifier, reportKey);
end

function trigger.printTriviaReport(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];
	local testifier = args[3];
	local reportKey = getCardStorageKey(testifier) .. "'s trivia report";
	log.info(reportKey);
	printReport(case, detective, testifier, reportKey);
end

function trigger.closeCase(params)
	local args = splitArgs(params);
	local case = args[1];
	local detective = args[2];

	detectiveCloseCase(case, detective);
end

--------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

return _M