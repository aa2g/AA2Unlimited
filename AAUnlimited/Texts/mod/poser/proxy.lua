local _M = {}

local facetoggles = { tears = "", dimeyes = "" }
local facekeys = { eye = "m_eye", eyeopen = "m_eyeOpen", eyebrow = "m_eyebrow", mouth = "m_mouth", mouthopen = "m_mouthOpen", blush = "Blush", blushlines = "BlushLines", eyetracking = "m_eyeTracking" }
local skelkeys = { pose = "m_poseNumber", frame = "m_animFrame", skelname = "m_name" }
local charkeys = { clothstate = "m_clothState" }
local structfuncs = { spawn = "Spawn", despawn = "Despawn", update = "Update", skeleton = "Skeleton", destroy = "Destroy", unload = "Unload" }
local poserfuncs = { getslider = "GetSlider", sliders = "Sliders", override = "Override", props = "Props" }

function _M.wrap(entity)
	log.spam("Poser: Wrapping %s", entity)
	local mt = getmetatable(entity)
	if not entity or not mt then return end
	local ischaracter
	local isprop
	if mt.__name == "ExtClass::CharacterStruct" then
		ischaracter = true
	elseif  mt.__name == "ExtClass::XXFile" then
		isprop = true
	end
	log.spam("Poser: Is character: %s", ischaracter)
	if not (ischaracter or isprop) then
		log.error("Poser: Cannot wrap unknown struct %s", entity)
	end
	
	local struct = entity
	local poser = ischaracter and GetPoserCharacter(struct) or GetPoserProp(struct)
	local name = ischaracter and sjis_to_utf8(string.format("%s %s", struct.m_charData.m_forename, struct.m_charData.m_surname)) or struct.m_name
	local cache = {}
	local wrapper = {}
	
	local charamt
	charamt = {
		__index = function(t, k)
			local cached = cache[k]
			if cached then return cached end
			if ischaracter then
				if facekeys[k] then
					return face[facekeys[k]]
				end
				if skelkeys[k] then
					return struct.m_xxSkeleton[skelkeys[k]]
				end
				if charkeys[k] then
					return struct[charkeys[k]]
				end
			end
			if structfuncs[k] then
				return function (...) return struct[structfuncs[k]](struct, ...) end
			end
			if poserfuncs[k] then
				log.spam("Poser: return poserfunc %s for %s", poserfuncs[k], poser)
				return function (...) return poser[poserfuncs[k]](poser, ...) end
			end
			return charamt[k]
		end,
		
		__newindex = function(t, k, v)
			if ischaracter then
				if facekeys[k] then
					local face = struct:GetXXFileFace()
					rawset(cache, k, v)
					if k == "eyebrow" then
						local eyebrow = face.m_eyebrow
						local offset = eyebrow % 7
						v = eyebrow - offset + v
					end
					face[facekeys[k]] = v
					-- character:update_face()
				elseif facetoggles[k] then
					rawset(cache, k, v)
				elseif skelkeys[k] then
					local skel = struct.m_xxSkeleton
					skel[skelkeys[k]] = v
				elseif charkeys[k] then
					struct[charkeys[k]] = v
				else
					rawset(charamt, k, v)
				end
			end
		end
	}
	
	rawset(wrapper, "ischaracter", ischaracter)
	rawset(wrapper, "isprop", isprop)
	rawset(wrapper, "struct", struct)
	rawset(wrapper, "poser", poser)
	rawset(wrapper, "name", name)
	rawset(wrapper, "cache", cache)
	rawset(wrapper, "boneselection", {})
	setmetatable(wrapper, charamt)

	if ischaracter then
		
		function charamt.update(clothstate)
			struct:Update(clothstate or 0, exe_type == "edit" and 1 or 0)
		end
		
		function charamt.setclip(clip)
			wrapper.pose = clip
			wrapper.frame = 9000
		end

		function charamt.context_name()
			log.spam("chr is %s",chr)
			return name .. '@' .. wrapper.pose .. '@' .. (wrapper.xa or '<DEFAULT>')
		end
	end
	
	if ischaracter then
		wrapper.origskel = wrapper.skelname
	end
	
	return wrapper
end

return _M
