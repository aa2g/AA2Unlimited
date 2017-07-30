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

--enables poser in cloth select screen
bUseClothesPoser = true

--enables poser in dialogue and sex scenes
bUseDialoguePoser = true


--format to save game screenshots in
--0 - BMP
--1 - JPG
screenshotFormat = 0				
									
--HAi should be activated at no prompt h done by evil npcs
bHAiOnNoPromptH = true				

------------------
-- Card Options --
------------------

--determines how eye files that were saved into the card data are handled.
--unlike eye textures, these can not be loaded from memory, are probably huge,
--and might be malicious, so take caution
--1: on first load, check if all files are there. If not, give a pop up
--   informing about the missing files and extract if wanted
--2: same as 1, but do not ask for permission just extract
--3: do not extract files
savedFileUsage = 1					

--if true, after saved files were successfully extracted, they will be removed
--from the card in an attempt to save hard drive space
bSaveFileAutoRemove = true

--does a backup of a card before saved files are removed
bSaveFileBackup = true				


--determines how old cards using the old paths are treated.
--0: old cards are treated as non-AAU cards and are ignored
--1: old cards will use the old pathing methods.
--2: old cards will be treated like new cards
--paths will be reinterpreted
--3: the game will attempt a conversion
legacyMode = 3

-- Set log level
--0: spam
--1: info
--2: warn
--3: err
--4: crit
logPrio = 0

-- (don't set, these are currently broken)
sPoserHotKeys = ""

-- load .ppx files
bUsePPeX = false

-- load .pp2 files
bUsePP2 = true

-- MB cache memory for general data
PP2Cache = 512

-- MB cache memory for decompressed audio
PP2AudioCache = 256

-- The modified launcher of MKIII decensor
-- rewrites the string so that AS00_03_00_00_00.bmp - AS00_03_00_04_00.bmp
-- becomes AS00_03_00_04_00.tga. This forces the game to recognize that the file
-- actually is tga format. Sometimes it needs to be disabled for stuff which
-- expects the BMP behavior of vanilla (SVII perhaps)
bUseMKIII = true

mods = {
--	{ "fakereg" },
	{ "fixlocale" },
	{ "makertrans" },
	{ "facecam" }, 
	{ "geass"},
	{ "launcher" },
}
