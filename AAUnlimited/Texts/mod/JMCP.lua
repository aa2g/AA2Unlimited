--@INFO Repositions the H position buttons

local _M = {}
local opts = {	
	{ "MAX_BUTTONS_PER_ROW", 18, "Rows: %i" },	-- max number of allowed rows
	{ "X_SPACING", 5, "Horizontal spacing, px: %i" },	-- horizontal spacing beetween buttons for normal buttons
	{ "Y_SPACING", 5, "Vertical spacing, px: %i" },	-- vertical spacing beetween buttons
	{ "X_INTER_SPACING", 15, "Female/Male cumming button thickness, px: %i" },	-- how far do the male/female cum buttons stick out from under the both cum button
	{ "FIRST_X", 1060, "Anchor button's X coordinate, px: %i" },	-- first button coordinates and size. Everything else is positioned relative to it
	{ "FIRST_Y", 99, "Anchor button's Y coordinate, px: %i" },
	{ "FIRST_XS", 136, "Buttons Width, px: %i" },
	{ "FIRST_YS", 24, "Buttons Height, px: %i" },
	{ "FLIP_M_F", 0, "Flip Female/Male buttons: %b"}
	
}

local screenScaleX = 0;
local screenScaleY = 0;

-- local FIRST_X = 1584;
-- local FIRST_Y = 150;
-- local FIRST_XS = 204;
-- local FIRST_YS = 36;

function on.move_h_button(bButtonMoved, category, buttonIdx, btn)
	-- log.info("button " .. category .. "-" .. buttonIdx);
	if (screenScaleX == 0) then
		screenScaleX = Config.renderWidth / 1280;
		screenScaleY = Config.renderHeight / 720;
	end
	if (category >= 0 and category <= 6) then	-- normal positions and solo cumming positions don't need to be grouped in triplets
		moveNormalButton(buttonIdx, btn);
		normalizeButton(btn);
	elseif (category >= 7 and category <= 8) then	-- cumming vaginal and cumming anal positions have to be grouped in triplets
		if (buttonIdx % 3 == 0) then
			if (opts.FLIP_M_F == 1) then
				moveCumFemaleButton(buttonIdx, btn);		
			else
				moveCumMaleButton(buttonIdx, btn);		
			end
			normalizeButton(btn);
		elseif (buttonIdx % 3 == 1) then
			if (opts.FLIP_M_F == 1) then
				moveCumMaleButton(buttonIdx, btn);
			else
				moveCumFemaleButton(buttonIdx, btn);
			end
			normalizeButton(btn);
		elseif (buttonIdx % 3 == 2) then
			moveCumBothButton(buttonIdx, btn);	
			normalizeButton(btn);
		end
	end
	
	bButtonMoved = true;
	return bButtonMoved;
end

function normalizeButton(btn)
	btn.m_renderX = (btn.m_renderX * screenScaleX) // 1;
	btn.m_renderY = (btn.m_renderY * screenScaleY) // 1;
	
	btn.m_posLeft = (btn.m_posLeft * screenScaleX) // 1;	
	btn.m_posTop = (btn.m_posTop * screenScaleY) // 1;	
	btn.m_posRight = (btn.m_posRight * screenScaleX) // 1;	
	btn.m_posBottom = (btn.m_posBottom * screenScaleY) // 1;	
end

function moveNormalButton(idx, btn)
	local column = idx // opts.MAX_BUTTONS_PER_ROW;
	local row = idx % opts.MAX_BUTTONS_PER_ROW;
	local xPos = opts.FIRST_X - (opts.FIRST_XS + opts.X_SPACING) * column;
	local yPos = opts.FIRST_Y + (opts.FIRST_YS + opts.Y_SPACING) * row;
	
	-- top left corner of the button texture
	btn.m_renderX = xPos;
	btn.m_renderY = yPos;
	
	-- actual clickable area
	btn.m_posLeft = btn.m_renderX;
	btn.m_posTop = btn.m_renderY;
	btn.m_posRight = opts.FIRST_XS + btn.m_renderX;
	btn.m_posBottom = opts.FIRST_YS + btn.m_renderY;
end

function moveCumFemaleButton(idx, btn)
	local X_WIDE_SPACING = 2 * opts.X_INTER_SPACING + opts.X_SPACING;		-- horizontal spacing between the both cum buttons. Usually (2*X_INTER_SPACING+X_SPACING)
	local column = idx // (opts.MAX_BUTTONS_PER_ROW * 3);
	local row = (idx % (opts.MAX_BUTTONS_PER_ROW * 3)) // 3;
	local xPos = opts.FIRST_X - (opts.FIRST_XS + X_WIDE_SPACING) * column;
	local yPos = opts.FIRST_Y + (opts.FIRST_YS + opts.Y_SPACING) * row;
	
	-- top left corner of the button texture
	btn.m_renderX = xPos;
	btn.m_renderY = yPos;
	
	-- actual clickable area
	btn.m_posLeft = opts.FIRST_XS + xPos - opts.X_INTER_SPACING;
	btn.m_posTop = btn.m_renderY;
	btn.m_posRight = opts.FIRST_XS + btn.m_renderX;
	btn.m_posBottom = opts.FIRST_YS + btn.m_renderY;
end

function moveCumMaleButton(idx, btn)
	local X_WIDE_SPACING = 2 * opts.X_INTER_SPACING + opts.X_SPACING;		-- horizontal spacing between the both cum buttons. Usually (2*X_INTER_SPACING+X_SPACING)
	local column = idx // (opts.MAX_BUTTONS_PER_ROW * 3);
	local row = (idx % (opts.MAX_BUTTONS_PER_ROW * 3)) // 3;
	local xPos = opts.FIRST_X - (opts.FIRST_XS + X_WIDE_SPACING) * column;
	local yPos = opts.FIRST_Y + (opts.FIRST_YS + opts.Y_SPACING) * row;
	
	-- top left corner of the button texture
	btn.m_renderX = xPos - 2 * opts.X_INTER_SPACING;
	btn.m_renderY = yPos;
	
	-- actual clickable area
	btn.m_posLeft = btn.m_renderX;
	btn.m_posTop = btn.m_renderY;
	btn.m_posRight = btn.m_renderX + opts.X_INTER_SPACING;
	btn.m_posBottom = btn.m_renderY + opts.FIRST_YS;
end

function moveCumBothButton(idx, btn)
	local X_WIDE_SPACING = 2 * opts.X_INTER_SPACING + opts.X_SPACING;		-- horizontal spacing between the both cum buttons. Usually (2*X_INTER_SPACING+X_SPACING)
	local column = idx // (opts.MAX_BUTTONS_PER_ROW * 3);
	local row = (idx % (opts.MAX_BUTTONS_PER_ROW * 3)) // 3;
	local xPos = opts.FIRST_X - (opts.FIRST_XS + X_WIDE_SPACING) * column;
	local yPos = opts.FIRST_Y + (opts.FIRST_YS + opts.Y_SPACING) * row;
	
	-- top left corner of the button texture
	btn.m_renderX = xPos - opts.X_INTER_SPACING;
	btn.m_renderY = yPos;
	
	-- actual clickable area
	btn.m_posLeft = btn.m_renderX;
	btn.m_posTop = btn.m_renderY;
	btn.m_posRight = btn.m_renderX + opts.FIRST_XS;
	btn.m_posBottom = btn.m_renderY + opts.FIRST_YS;
end

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

function on.launch()
end

function _M:config()
	mod_edit_config(self, opts, "JMCP settings")
end

return _M

