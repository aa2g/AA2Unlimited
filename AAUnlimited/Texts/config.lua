bUnlimitedOnTop = false

-- What to do with modfiles in cards
-- 0 - Ask
-- 1 - Always extract
-- 2 - Don't extract
savedFileUsage = 0

-- Flat UI (but not in maker)
bUseVisualStyles = true

--load more than 6 tan slots. currently crashes.
bUseAdditionalTanSlots = false		

--apply mesh texture override rules. When a xx (mesh) file is loaded, the 
--textures and texture list will be compared to a set of override rules,
--and if a rule applies, the texture will be replaced.
--this is needed for gui sliders and poser to work, among other things
bUseMeshTextureOverrides = true		

--enables H-AI
bUseHAi = true						

--when shadowing is enabled, every time a file is opened from a pp archive, the game
--checks for a folder of the same name (excluding the pp at the end) containing
--the same file first. If it exists, this file is taken instead.
bUseShadowing = true				

--when enabled, pressing q locks the camera in the face of the participants during h

--reorders the position buttons in multiple columns so they dont go out of the
--window and become unclickable
bEnableHPosButtonReorder = true		

--format to save game screenshots in
--0 - BMP
--1 - JPG
--2 - PNG
screenshotFormat = 2
									
--HAi should be activated at no prompt h done by evil npcs
bHAiOnNoPromptH = true				

sFont = "MS Gothic"

-- Set log level
--0: spam
--1: info
--2: warn
--3: err
--4: crit
logPrio = 2

-- load .ppx files
bUsePPeX = false

-- load .pp2 files
bUsePP2 = false

-- MB cache memory for general data, includes data allocated by game itself
PP2Cache = 500

-- MB cache memory for decompressed audio
PP2AudioCache = 150
PP2Buffers = 300
PP2Profiling = true

-- The modified launcher of MKIII decensor
-- rewrites the string so that AS00_03_00_00_00.bmp - AS00_03_00_04_00.bmp
-- becomes AS00_03_00_04_00.tga. This forces the game to recognize that the file
-- actually is tga format. Sometimes it needs to be disabled for stuff which
-- expects the BMP behavior of vanilla (SVII perhaps)
bUseMKIII = true

-- List file names instead of name in card select (AA2Play)
bListFilenames = false

-- Enable card triggers
bTriggers = true


mods = {
	{ "aaface" },
	{ "poser" },
	{ "nobra" },
	{ "edit" },
	{ "hirestex" },
	{ "unlocks" },
	{ "fixlocale" },
	{ "makertrans" },
	{ "playtrans" },
	{ "subtitles" },
	{ "facecam" }, 
	{ "geass"},
	{ "jizou"},
	{ "timewarp" },
	{ "extsave" },
	{ "borderless" },
	{ "launcher" },
}
