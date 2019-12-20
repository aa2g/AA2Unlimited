local charamgr = require "poser.charamgr"

local lights = { kougen = true, kage = true, kesi01 = true }
-- local cameralights = { "cam", "rim" }
local xxfiles = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 11 }

local sliderstep = 0.05

local function setlightdirection(x, y, z)
	if x == 0 and y == 0 and z == 0 then return end
	local norma = math.sqrt(x*x + y*y + z*z)
	local x, y, z = x/norma, y/norma, z/norma
	for _,character in pairs(charamgr.characters) do
		if character.ischaracter == true and character.struct.m_xxSkeleton then
			local skeleton = character.struct.m_xxSkeleton
			local lightcount = skeleton.m_lightsCount
			for i = 1, lightcount, 1 do
				local light = skeleton:m_lightsArray(i - 1)
				if lights[light.m_name] then
					light:SetLightDirection(x, y, z, 0)
				end
			end
		end
	end
end

local sliderx, slidery, sliderz
local function slidervaluechanged()
	setlightdirection(sliderx.value, slidery.value, sliderz.value)
end
sliderx = iup.val { orientation = "horizontal", expand = "horizontal", min = -1, max = 1, step = sliderstep, value = 0, inverted = "yes", valuechanged_cb = slidervaluechanged }
slidery = iup.val { orientation = "horizontal", expand = "horizontal", min = -1, max = 1, step = sliderstep, value = 0, inverted = "yes", valuechanged_cb = slidervaluechanged }
sliderz = iup.val { orientation = "horizontal", expand = "horizontal", min = -1, max = 1, step = sliderstep, value = 0, inverted = "yes", valuechanged_cb = slidervaluechanged }

local function getlight(character, name)
	if character.ischaracter == true and character.struct.m_xxSkeleton then
		local skeleton = character.struct.m_xxSkeleton
		local lightcount = skeleton.m_lightsCount
		for i = 1, lightcount, 1 do
			local light = skeleton:m_lightsArray(i - 1)
			if light.m_name == name then
				return light
			end
		end
	end
end

local baser = iup.text {}
local baseg = iup.text {}
local baseb = iup.text {}
local ambientlighteditor = iup.hbox {
		tabtitle = "ambient",
		iup.gridbox {
			numdiv = 4,
			baser, baseg, baseb,
		},
		iup.hbox {
			iup.button { title = "Get", action = function()
				local character = charamgr.current
				if character.ischaracter ~= true or not character.struct.m_xxSkeleton then return end
				local skel = character.struct.m_xxSkeleton
				baser.value = skel.m_ambientLightRed
				baseg.value = skel.m_ambientLightGreen
				baseb.value = skel.m_ambientLightBlue
			end},
			iup.button { title = "Set", action = function()
				local character = charamgr.current
				if character.ischaracter ~= true then return end
				character = character.struct
				for _,xx in ipairs(xxfiles) do
					local xxfile = character:GetXXFile(xx)
					if xxfile then
						xxfile.m_ambientLightRed = tonumber(baser.value)
						xxfile.m_ambientLightGreen = tonumber(baseg.value)
						xxfile.m_ambientLightBlue = tonumber(baseb.value)
					end
				end
			end},
		}
	}


local function lightmaterialeditor(name)
	local mat0r = iup.text {}
	local mat0g = iup.text {}
	local mat0b = iup.text {}
	local mat0a = iup.text {}
	local mat1r = iup.text {}
	local mat1g = iup.text {}
	local mat1b = iup.text {}
	local mat1a = iup.text {}
	local mat2r = iup.text {}
	local mat2g = iup.text {}
	local mat2b = iup.text {}
	local mat2a = iup.text {}
	local layout = iup.hbox {
		tabtitle = name,
		iup.gridbox {
			numdiv = 4,
			mat0r, mat0g, mat0b, mat0a,
			mat1r, mat1g, mat1b, mat1a,
			mat2r, mat2g, mat2b, mat2a,
		},
		iup.hbox {
			iup.button { title = "Get", action = function()
				local light = getlight(charamgr.current, name)
				if not light then return end
				mat0r.value, mat0g.value, mat0b.value, mat0a.value = light:GetLightMaterialColor(0)
				mat1r.value, mat1g.value, mat1b.value, mat1a.value = light:GetLightMaterialColor(1)
				mat2r.value, mat2g.value, mat2b.value, mat2a.value = light:GetLightMaterialColor(2)
			end},
			iup.button { title = "Set", action = function()
				local light = getlight(charamgr.current, name)
				if not light then return end
				light:SetLightMaterialColor(0, tonumber(mat0r.value), tonumber(mat0g.value), tonumber(mat0b.value), tonumber(mat0a.value))
				light:SetLightMaterialColor(1, tonumber(mat1r.value), tonumber(mat1g.value), tonumber(mat1b.value), tonumber(mat1a.value))
				light:SetLightMaterialColor(2, tonumber(mat2r.value), tonumber(mat2g.value), tonumber(mat2b.value), tonumber(mat2a.value))
			end},
		}
	}
	return layout
end


-- ----
-- Form
-- ----

local control = iup.vbox {
	iup.frame {
		title = "Light Direction",
		iup.vbox {
			nmargin = "6x3",
			iup.label { title = "Left <> Right" },
			sliderx,
			iup.label { title = "Bottom <> Top" },
			slidery,
			iup.label { title = "Back <> Front" },
			sliderz,
		},
	},
	iup.hbox {
		iup.tabs {
			ambientlighteditor,
			lightmaterialeditor("kougen"),
			lightmaterialeditor("kage"),
			lightmaterialeditor("kesi01"),
			lightmaterialeditor("cam"),
			lightmaterialeditor("rim"),
		}
	}
}

return control
