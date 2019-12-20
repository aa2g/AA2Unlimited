--@INFO Start Climax/Regular pose for current

local _M = {}

local initialized = { false, false } -- 1 - hetero, 2 - homo
local initializedCfg = false
local debugMode = false
local verifyRegister = false
local hInfo = false
local maxPoses = 2000
local maxPosesInCat = 200
local hButtonToClimax = {}	-- [1 - hetero|2 - homo][1..maxPoses] == { ... }
local hButtonToNormal = {} -- [1 - hetero|2 - homo][1..maxPoses] == { ... }
local hBtnPosToIndex = {} -- [1 - hetero|2 - homo][category_id (1..9)][1..n button_pos_in_category] == pose_id
local hCategoriesScenario = { {}, {} } -- [1 - hetero|2 - homo] == <list of Scenario commands>

local NUMPAD_1 = 97
local NUMPAD_3 = 99

local InitH

local function InitGenderPoses(gender_id)
	local pose_id = 0
	local button_pos_in_cat = 1
	--local button_i = 0

	for cat_i = 1, 9 do
		button_pos_in_cat = 1
		local list = hInfo:m_hPosButtons(cat_i-1)
		local buttons_count = list:GetButtonCount()
		
		for button_i = 1, buttons_count do
			pose_id = pose_id + 1
			
			local btn = list:GetButton(button_i-1)
			local data = hInfo:GetHPosData(hInfo:GetHPosition(cat_i-1, button_i-1))
			local for_debug = hButtonToClimax[gender_id][pose_id].anywayAdd == true and 1 or 0
					
			if gender_id == 2 or btn.m_posTop ~= 0 or btn.m_posLeft ~= 0 -- for HETERO only visible
				or hButtonToClimax[gender_id][pose_id].anywayAdd then 
				
				if (data.m_yuriAllowance == 0 and gender_id == 1) 			-- GENDERALLOW_HETERO_ONLY and Hetero init
					or ((data.m_yuriAllowance == -1 or data.m_yuriAllowance > 1) -- GENDERALLOW_HOMO_ONLY and Homo init
					and gender_id == 2) 
					or data.m_yuriAllowance == 1 							-- GENDERALLOW_BOTH
					or hButtonToClimax[gender_id][pose_id].anywayAdd then
					
					-- Register button
					hButtonToClimax[gender_id][pose_id].categoryNode = cat_i
					hButtonToClimax[gender_id][pose_id].buttonNode = button_i
					hButtonToNormal[gender_id][pose_id].categoryNode = cat_i
					hButtonToNormal[gender_id][pose_id].buttonNode = button_i
					
					hBtnPosToIndex[gender_id][cat_i][button_pos_in_cat] = pose_id
					-- Normal pose version for normal pose will be same
					if cat_i < 6 then
						hButtonToNormal[gender_id][pose_id].normalPoses[1] = pose_id
						hButtonToNormal[gender_id][pose_id].normalCount = 1
					else
						-- For Climax categories Climax pose will be same
						hButtonToClimax[gender_id][pose_id].climaxPoses[1] = pose_id
						hButtonToClimax[gender_id][pose_id].climaxCount = 1
					end
					
					button_pos_in_cat = button_pos_in_cat + 1
				end
			end
		end
		
		if debugMode then
			log.info("[ClimaxBtn] Registered " .. (button_pos_in_cat-1) .. " poses in " 
				.. (gender_id == 1 and "Hetero" or "Homo") .. " Category id: " .. cat_i)
		end
	end
end

local function AttachGenderPoses(gender_id) -- Attaching Normal poses to their Climax version (with cfg fixes)
	local btnClimaxVerCategory = 6 		-- 1 & 3 => 6, 2 => 7, 4 => 8, 5 => 9
	local btnClimaxVerButton = 1
	local btnClimaxTouchesSavedPos = 1 	-- Saving Last button pos of 'Climax touches' for category 3 (masturbate)
	
	for cat_i = 1, 5 do		-- For each Category of normal poses
		for button_i = 1, maxPosesInCat do
			local pose_id = hBtnPosToIndex[gender_id][cat_i][button_i]
			if pose_id == -1 then break end
			-- For each Button of normal poses
			
			-- If need apply fix from config
			if hCategoriesScenario[gender_id][1] ~= nil 
				and hCategoriesScenario[gender_id][1].isShift ~= true
				and hCategoriesScenario[gender_id][1].catId == cat_i
				and hCategoriesScenario[gender_id][1].buttonPos == button_i then

				hButtonToClimax[gender_id][pose_id].climaxCount = hCategoriesScenario[gender_id][1].climaxCount
				
				for climax_i = 1, 3 do
					local climax_pose_cat = hCategoriesScenario[gender_id][1].climaxPoses[climax_i][1]
					local climax_pose_btn_pos = hCategoriesScenario[gender_id][1].climaxPoses[climax_i][2]
					if climax_pose_btn_pos ~= -1 then
						local climax_pos_id = hBtnPosToIndex[gender_id][climax_pose_cat][climax_pose_btn_pos]
						hButtonToClimax[gender_id][pose_id].climaxPoses[climax_i] = climax_pos_id
						-- Normal version for this climax pose
						if hButtonToNormal[gender_id][climax_pos_id].normalCount < 3 then
							hButtonToNormal[gender_id][climax_pos_id].normalCount = hButtonToNormal[gender_id][climax_pos_id].normalCount + 1
							hButtonToNormal[gender_id][climax_pos_id].normalPoses[hButtonToNormal[gender_id][climax_pos_id].normalCount] = pose_id
						end
					end
				end
				
				table.remove(hCategoriesScenario[gender_id], 1) -- Remove current scenario command
				
				-- If need shift `climax button_i` also
				if hCategoriesScenario[gender_id][1] ~= nil  
					and hCategoriesScenario[gender_id][1].isShift == true then
					btnClimaxVerButton = btnClimaxVerButton + hCategoriesScenario[gender_id][1].shiftVal
					if btnClimaxVerButton < -1 then btnClimaxVerButton = -1 end
					table.remove(hCategoriesScenario[gender_id], 1) -- Remove this scenario command
				end
			else
				-- Climax 1st version for normal pose (by default)
				local climax_pos_id = hBtnPosToIndex[gender_id][btnClimaxVerCategory][btnClimaxVerButton]
				hButtonToClimax[gender_id][pose_id].climaxPoses[1] = climax_pos_id
				hButtonToClimax[gender_id][pose_id].climaxCount = 1
				-- Normal version for this climax pose
				if hButtonToNormal[gender_id][climax_pos_id].normalCount < 3 then
					hButtonToNormal[gender_id][climax_pos_id].normalCount = hButtonToNormal[gender_id][climax_pos_id].normalCount + 1
					hButtonToNormal[gender_id][climax_pos_id].normalPoses[hButtonToNormal[gender_id][climax_pos_id].normalCount] = pose_id
				end
			end

			btnClimaxVerButton = btnClimaxVerButton + 1
		end
		-- If next Category need for Climax Version of normal button
		if cat_i == 1 then
			btnClimaxTouchesSavedPos = btnClimaxVerButton -- Save for future category 2
		end
		
		if cat_i == 2 then	-- When need restore button pos for category 3
			btnClimaxVerButton = btnClimaxTouchesSavedPos
			btnClimaxVerCategory = 6
		else
			if cat_i == 3 then
				btnClimaxVerCategory = 8
			else 
				btnClimaxVerCategory = btnClimaxVerCategory + 1
			end
			btnClimaxVerButton = 1
		end
	end
end

local function StartClimaxPose() -- Starting Climax version of current pose
	
	
	local gender_id = 1 -- hetero
	if hInfo.m_activeParticipant.m_charPtr.m_charData.m_gender 
		== hInfo.m_passiveParticipant.m_charPtr.m_charData.m_gender then
		gender_id = 2 -- homo
	end

	if not initialized[gender_id] then InitH() end
	if not initialized[gender_id] then -- If InitH not success
		log.warn("[ClimaxBtn] Can't initialize ClimaxButton params")
		return
	end
	
	local current_pose_id = hInfo.m_currPosition + 1
	
	if debugMode or verifyRegister then
		log.info("[ClimaxBtn] Current pose id: " .. current_pose_id)
	end

	if current_pose_id < 1 or current_pose_id > maxPoses then return end

	if hButtonToClimax[gender_id][current_pose_id].buttonNode < 1 then
		log.warn("[ClimaxBtn] Current pose is unregistered")
		return
	end

	if verifyRegister then return end -- If need verify only registered status of current pose

	-- Get Climax version
	local climax_pose_node = 1
	if hButtonToClimax[gender_id][current_pose_id].climaxCount > 1 then -- If need random pick climax pose
		climax_pose_node = math.random(1, hButtonToClimax[gender_id][current_pose_id].climaxCount)
	end

	local climax_id = hButtonToClimax[gender_id][current_pose_id].climaxPoses[climax_pose_node];
	if climax_id < 1 then
		log.warn("[ClimaxBtn] Current pose haven't Climax version")
		return
	end

	local climaxBtnCategory = hButtonToClimax[gender_id][climax_id].categoryNode
	local climaxBtnNode = hButtonToClimax[gender_id][climax_id].buttonNode

	if hInfo:m_hPosButtons(climaxBtnCategory-1):GetButton(climaxBtnNode-1) ~= nil then
		-- If button Showed for current H scene
		if hInfo:m_hPosButtons(climaxBtnCategory-1):GetButton(climaxBtnNode-1).m_bInvalid ~= 1 or debugMode then
			hInfo:m_hPosButtons(climaxBtnCategory-1):GetButton(climaxBtnNode-1):Press()
		end
	end
end

local function StartNormalPose() -- Starting Normal version of current pose
	if not hInfo then return end
	
	local gender_id = 1 -- hetero
	if hInfo.m_activeParticipant.m_charPtr.m_charData.m_gender 
		== hInfo.m_passiveParticipant.m_charPtr.m_charData.m_gender then
		gender_id = 2 -- homo
	end
	
	if not initialized[gender_id] then InitH() end
	if not initialized[gender_id] then -- If InitH not success
		log.warn("[ClimaxBtn] Can't initialize ClimaxButton params")
		return
	end
	
	local current_pose_id = hInfo.m_currPosition + 1
	
	if debugMode or verifyRegister then
		log.info("[ClimaxBtn] Current pose id: " .. current_pose_id)
	end
	
	if current_pose_id < 1 or current_pose_id > maxPoses then return end
	
	if hButtonToClimax[gender_id][current_pose_id].buttonNode < 1 then
		log.warn("[ClimaxBtn] Current pose is unregistered")
		return
	end
	
	if verifyRegister then return end -- If need verify only registered status of current pose
	
	-- Get Normal version
	local normal_pose_node = 1
	if hButtonToNormal[gender_id][current_pose_id].normalCount > 1 then -- If need random pick normal pose
		normal_pose_node = math.random(1, hButtonToNormal[gender_id][current_pose_id].normalCount)
	end
	
	local normal_id = hButtonToNormal[gender_id][current_pose_id].normalPoses[normal_pose_node];
	if normal_id < 1 then
		log.warn("[ClimaxBtn] Current pose haven't Normal version")
		return
	end
	
	local normalBtnCategory = hButtonToNormal[gender_id][normal_id].categoryNode
	local normalBtnNode = hButtonToNormal[gender_id][normal_id].buttonNode
	
	if hInfo:m_hPosButtons(normalBtnCategory-1):GetButton(normalBtnNode-1) ~= nil then
		-- If button Showed for current H scene
		if hInfo:m_hPosButtons(normalBtnCategory-1):GetButton(normalBtnNode-1).m_bInvalid ~= 1 or debugMode then
			hInfo:m_hPosButtons(normalBtnCategory-1):GetButton(normalBtnNode-1):Press()
		end
	end
end

local function InitCfg()
	if initializedCfg then return end
	-- Starting data for buttons
	for gender_i = 1, 2 do
		hBtnPosToIndex[gender_i] = {}
		for categ_i = 1, 9 do
			hBtnPosToIndex[gender_i][categ_i] = {}
			for pose_pos_in_cat = 1, maxPosesInCat do
				hBtnPosToIndex[gender_i][categ_i][pose_pos_in_cat] = -1
			end
		end
		
		hButtonToClimax[gender_i] = {}
		hButtonToNormal[gender_i] = {}
		for pose_i = 1, maxPoses do
			hButtonToClimax[gender_i][pose_i] = {
				categoryNode = -1,
				buttonNode = -1,
				anywayAdd = false,
				climaxPoses = { -1, -1 , -1 },
				climaxCount = 0
			}
			hButtonToNormal[gender_i][pose_i]= {
				categoryNode = -1,
				buttonNode = -1,
				normalPoses = { -1, -1 , -1 },
				normalCount = 0
			}
		end
	end
	
	-- Get the config file for current poses pack
	local cfg_path = aau_path("resources", "climax_button_poses.txt")
	local cfg_file = io.open(cfg_path, "r")
	if not cfg_file then
		log.warn("[ClimaxBtn] Can't open the AAUnlimited/resources/climax_button_poses.txt file")
		return 
	end
	
	local matches = {}
	local matchesIn = {}
	for line in cfg_file:lines() do
		if not line:match("^%s*//.*") then -- if not a comment
			
			-- Debug param
			matches[1] = line:match("Debug%s*=%s*([a-zA-Z]+)")
			if matches[1] and matches[1] == "true" then
				debugMode = true
				log.info("[ClimaxBtn] Debug Activated")
			elseif line:match("VerifyRegister%s*=%s*[a-zA-Z]+") then
				matches[1] = line:match("VerifyRegister%s*=%s*([a-zA-Z]+)")
				if matches[1] and matches[1] == "true" then
					verifyRegister = true
					if debugMode then
						log.info("[ClimaxBtn] VerifyRegister Activated")
					end
				end
			-- Fix register poses id
			elseif line:match("%[FIX_[^%]]+%]") then
				matches[1], matches[2] = line:match("%[FIX_([^%]]+)%][^0-9]*(%d+)")
				if matches[1] and matches[2] then
					local gender_id = matches[1] == "HETERO" and 1 or 2
					local pose_id = tonumber(matches[2])
					if pose_id > 0 and pose_id <= maxPoses then
						hButtonToClimax[gender_id][pose_id].anywayAdd = true
						if debugMode then
							log.info("[ClimaxBtn] Detected fix register for " 
								.. matches[1] .. " pose id: " .. pose_id)
						end
					end
				end
			elseif line:match("%[[^%]]+_LINK%]") then
				matches[1], matches[2], matches[3], matches[4], matches[5] 
					= line:match("%[([^%]]+)_LINK%][^{]*{([^{]*)}[^{]*{([^{]*)}[^{]*{([^{]*)}[^{]*{([^{]*)}")
				if matches[1] and matches[2] and matches[3] and matches[4] and matches[5] then
					local gender_id = matches[1] == "HETERO" and 1 or 2
					local climaxScenario = {
						isShift = false,
						shiftVal = 0,
						catId = -1,		-- if !isOffset
						buttonPos = -1,	-- if !isOffset
						climaxPoses = { {-1, -1}, { -1, -1 }, { -1, -1 } },
						climaxCount = 0
					}
					
					matchesIn[1], matchesIn[2] = matches[2]:match("(%d+)[^0-9]+(%d+)")
					if matchesIn[1] and matchesIn[2] then
						climaxScenario.catId = tonumber(matchesIn[1])
						climaxScenario.buttonPos = tonumber(matchesIn[2])
					end
					local climaxCount = 0
					-- 1st climax ver.
					matchesIn[1], matchesIn[2] = matches[3]:match("(%d+)[^0-9]+(%d+)")
					if matchesIn[1] and matchesIn[2] then
						climaxScenario.climaxPoses[1][1] = tonumber(matchesIn[1])
						climaxScenario.climaxPoses[1][2] = tonumber(matchesIn[2])
						climaxCount = climaxCount + 1
					end
					local pos_i = 1
					-- 2nd climax ver.
					matchesIn[1], matchesIn[2] = matches[4]:match("(%d+)[^0-9]+(%d+)")
					if matchesIn[1] and matchesIn[2] then
						pos_i = 1
						if climaxScenario.climaxPoses[1][1] ~= -1 then
							pos_i = pos_i + 1
						end
						climaxScenario.climaxPoses[pos_i][1] = tonumber(matchesIn[1])
						climaxScenario.climaxPoses[pos_i][2] = tonumber(matchesIn[2])
						climaxCount = climaxCount + 1
					end
					-- 3rd climax ver.
					matchesIn[1], matchesIn[2] = matches[5]:match("(%d+)[^0-9]+(%d+)")
					if matchesIn[1] and matchesIn[2] then
						pos_i = 1
						if climaxScenario.climaxPoses[1][1] ~= -1 then
							pos_i = pos_i + 1
						end
						if climaxScenario.climaxPoses[2][1] ~= -1 then
							pos_i = pos_i + 1
						end
						climaxScenario.climaxPoses[pos_i][1] = tonumber(matchesIn[1])
						climaxScenario.climaxPoses[pos_i][2] = tonumber(matchesIn[2])
						climaxCount = climaxCount + 1
					end
					climaxScenario.climaxCount = climaxCount
					-- Add Scenario command to result scenario
					table.insert(hCategoriesScenario[gender_id], climaxScenario)
					if debugMode then
						log.info("[ClimaxBtn] Detected Link " .. matches[1] .. " {".. matches[2] .."} => "
							.. "{".. matches[3] .."} " .. "{".. matches[4] .."} "  .. "{".. matches[5] .."} " )
					end
				end
			elseif line:match("%[[^%]]+_SHIFT%]") then
				matches[1], matches[2], matches[3] = line:match("%[([^%]]+)_SHIFT%][^%+%-]([%+%-])(%d+)")
				if matches[1] and matches[2] and matches[3] then
					local gender_id = matches[1] == "HETERO" and 1 or 2
					local singPlusMinus = matches[2] == "+" and 1 or -1
					local shiftVal = tonumber(matches[3])
					-- Fill the climax scenario command for current gender
					local climaxScenario = {
						isShift = false,
						shiftVal = 0,
						catId = -1,		-- if !isOffset
						buttonPos = -1,	-- if !isOffset
						climaxPoses = { {-1, -1}, { -1, -1 }, { -1, -1 } },
						climaxCount = 0
					}
					climaxScenario.isShift = true
					climaxScenario.shiftVal = shiftVal * singPlusMinus
					-- Add Scenario command to result scenario
					table.insert(hCategoriesScenario[gender_id], climaxScenario)
					if debugMode then
						log.info("[ClimaxBtn] Detected Shift " .. matches[1] .. " ".. matches[2] .. matches[3])
					end
				end
			end
		end
	end
	
	if debugMode then log.info("[ClimaxBtn] Climax InitCfg() OK") end
	
	initializedCfg = true
end

InitH = function()
	if not initializedCfg then InitCfg() end
	if not initializedCfg then 
		log.warn("[ClimaxBtn] Can't initialize CFG txt file for ClimaxButton")
	end
	
	if not hInfo then return end
	
	local gender_id = 1 -- hetero
	if hInfo.m_activeParticipant.m_charPtr.m_charData.m_gender 
		== hInfo.m_passiveParticipant.m_charPtr.m_charData.m_gender then
		gender_id = 2 -- homo
	end
	
	if initialized[gender_id] then return end
	
	if debugMode then log.info("[ClimaxBtn] Climax Init() START") end
	
	InitGenderPoses(gender_id)
	-- Gotten hButtonToClimax (not all params) and  hBtnPosToIndex
	-- Now we need to attach normal poses to their climax version (using also fixes from hCategoriesScenario)
	AttachGenderPoses(gender_id)

	if debugMode then log.info("[ClimaxBtn] Climax Init() END (success)") end
	
	initialized[gender_id] = true
end


function on.start_h(hi)
	hInfo = hi
	InitH()
end

function on.end_h()
	hInfo = false
end

function on.convo()
	hInfo = false
end

-- Radial Menu events
function on.climax_btn_start_climax()
	StartClimaxPose()
end
function on.climax_btn_start_normal()
	StartNormalPose()
end

function on.keydown(k)
	if (not hInfo) then return k end
		if k == NUMPAD_1 then StartClimaxPose()
		elseif k == NUMPAD_3 then StartNormalPose() end
	return k
end

function on.launch()
	InitCfg()
end

function _M:load()
	--mod_load_config(self, opts)
end

function _M:unload()
	hButtonToClimax = {}
	hButtonToNormal = {}
	hBtnPosToIndex = {}
	hCategoriesScenario = {}
end

--function _M:config()
	--mod_edit_config(self, opts, "Climax button options")
--end

return _M
