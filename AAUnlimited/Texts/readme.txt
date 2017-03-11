How to install:
1: Move the AAUnlimited folder into your AAPlay main folder (the one with your play exe)
2: Move the exe anywhere

How to use:
1: start the game or editor
2: doubleclick the exe

/////////////////////////////////////////
General Guide

The Program currently supports the following features:

Gameplay changes:
- H-Ai: AI that does H for you. Activates when the player is forced or at evil no prompt h.
	- No means yes when forced by an npc
- Position Buttons during H are aligned in rows if there are too many buttons per category, allowing to
  add more buttons before they go out of the screen
- POV in H-Mode: press Q to switch between POVs of participants. Press W do restore to normal view.
- simple poser: in clothing menu, frames can be rotated. sliders are defined in poserframes.txt. poses can be saved and loaded.

General:
- Shadowing: Instead of only loading files from a .pp file, the game checks for a folder of the same name (without the pp ending)
  first. If this folder exists and contains a file of the required name, this file is loaded instead.
   - Shadowing Sets: a folder called "sets" next to the pp files can define shadowing sets.
					 each subfolder, if its name starts with a !, can contain folders with the name of a pp file to shadow
					 in the same way as normal shadowing works. example:
				jg2e01_00_00.pp
				jg2e01_00_00
					A00_10_00_00.xx			| will replace face 10 from original pp
				sets
					!myActiveSet
						jg2e01_00_00
							A00_20_00_00.xx			| will replace face 20 from original pp
					myPassiveSet
						jg2e01_00_00
							A00_10_00_00.xx			| will do nothing cause set is disabled
						

Card Data: adds additional card data to cards:
- Tan: Uses a tan from AAEdit\data\override\tan. The Folder is expected to have the same format used
  for Tantoo (just copy its tan folder over)
  Note: Archive Override (jg2e03_00_00.pp,A00_00_00_*_*)->(file)
	Hight pixel density skin is highly recommended: https://mega.nz/#F!IVFyWIrb!meEOFx8i7nHDYMHOAQL2Kg
- Hair Highlight: Use a hair Highlight from AAEdit\data\override\hair_highlight.
  Note: Mesh Texture (Asp00_20_00_00_00*)->(file), so this will not work if you have custom highlights without this name prefix
These Features are all simply special, wild-carded rules of one of the three types below. While the rules below do NOT
support wild cards naturally, all of these things could be done for specific slots or files using the rules directly.
Whenever a High-Poly model is loaded (or all the time in AAEdit), these rules from the corresponding card are applied.
The following rule types exist:
- Mesh Texture Overrides: A set of rules of the form (TextureName)->(TextureFileOnHarddrive)
  Whenever a xx file is loaded that contains a texture of a given name, a different texture
  from the file system is loaded instead
- Archive Overrides: A set of rules of the form (archive.pp|file)->(fileOnHarddrive)
  Whenever a given file is read from a given archive, a different file from the harddrive will be loaded instead.
- Archive Redirects: A set of rules of the form (archive.pp|file)->(archive.pp|file)
  Whenever a given file is read from a given archive, a different file from a different archive is loaded instead.
- Object Overrides:  A set of rules of the form (ObjectName)->(file)
  Whenever a xx file is loaded that contains the given Object, it is replaced by the object in the given file.
  The format of the given file must be exactly how it is stored inside the xx file.
  There is a different exe in this project that can extract those.
  WARNING: by the looks of it, the file is radically different depending on the xx file version. These overrides probably
  only work for xx files version 8. 
Additional card based features:
- All files target by these overrides, as well as eye highlights and eye textures, can be saved inside the card;
  if such a card is opened in the editor, the files will be extracted automatically
- Eye Textures: a seperate texture can be used for the right eye
- Body Sliders: additional sliders for certain body parts
- Outline Color: can change outline color for this character (every part of the body)
- Tan Color: can change the tan color from its brown shade
- Additional Hairs: more hairs than the original 4 can be added. note that these hairs do not have low poly modles (yet)
- Body Modification: can put an SRT matrix in front of frames or modify the SRT matrix of bones. only use if you are aware of how AA2 handles bodies
					 note: due to how AA2 handles frames, this is not the same as changing a frame in SB3U. rotations and scales will affect the original
					 matrix of this frame. adjust your matrix accordingly to make up for the incorrect translations.

/////////////////////////////////////////
FAQ

Q: When i try to start AAUnlimitedExe.exe, it complains about missing MSVCRxx or VCRUNTIMEyyy (probably 140) missing
A: you need the vc++2015 redistributable packages. they are the c++ standard library from microsoft
	and are needed for every c++ program compiled with visual studio. Usually these would be installed
	with one of your other games, but 2015 is pretty new still, and game development takes time.
	you can get them here: https://www.microsoft.com/en-us/download/details.aspx?id=48145&751be11f-ede8-5a0c-058c-2ee190a24fa6=True
	note: if you are on a 64-bit system, for some reason, you need both the 32-bit and the 64-bit version. no idea why, shouldnt be that way.

Q: When i try to start it, it says that LoadLibrary failed.
A: This is almost certainly because you did not move the AAUnlimitedDll.dll to the appropriate place. Make sure
	you moved the entire AAUnlimited folder into your AAPlay folder

Q: Is this the new version? Where can i get the latest version?
A: There is a github for this project: https://github.com/aa2g/AA2Unlimited/releases

Q: I found a Bug. What do?
A: You can make an issue on github and describe it, in which case i will see it sooner or later. make sure that you describe
	a way to reproduce it as consistently as possible. if you dont want to make a GitHub account for that, try posting it 
	on /aa2g/ on 4chan (search on www.4chan.org/vg/catalog). I'm looking in there pretty frequently (right now at least).
	If you are from the (probably near) future where /aa2g/ is dead, then RIP /aa2g/

