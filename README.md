
# DOWNLOAD THE LATEST RELEASE IN THE [Releases](https://github.com/aa2g/AA2Unlimited/releases) TAB

# AA2Unlimited

AAUnlimited is a mod, and a mod framework for the game Aritifical Academy 2 by Illusion. It adds many new features. In short, it makes your game run smoother, it makes your characters look pretty, and it makes the gameplay interesting.

## Features:

### Gameplay changes:
- **Poser**: You can pose characters, just like in Koikatu. You have complete control of the scene which includes every part of the body, objects, rooms, and lighting. Here are some examples of the poser in action.
- **Modules**: You can create custom traits and make changes to the AI of your characters. List of pre-made traits can be found [here.](https://pastebin.com/R4DW0dxw)
- **H-Ai**: When a player is being raped, the player will not be in control of the sex scene. The NPC's Ai will be picking the positions, and the scene will last until they are satisfied. The player cannot say no to rape if they have the exploitable trait exploitable. H-Ai turns on when you spectate two NPCs who are publicly fucking.
- **Body Sliders** - This adds a lot of new sliders that let you edit everything about the body of your character. Even facial features can be edited: jaw, nose, chin, cheeks, etc. 
- **Extra hair** - You can add additional hair, which means that you aren't limited to picking only 1 hair for front/side/back/extension hair slot. 
- **Unlocks** - We've made the character editor more powerful. We've unlocked all the slider values that Illusion originally limited. We've added the option to set custom shadows, and custom outlines to various body parts. We've made it possible to add custom highlight and custom hair texture. Add to this the new sliders, and you can make some really cute girls!
- **Overrides and redirects** - Every time a file is read from some archive, a different file that the user specifies could be loaded instead of it. So this basically removes all limits like there only being a certain amount of available hair slots, clothes etc. This lets modders run wild by editting anything in the game. A lot of new mods and assets have been made for AA2, and this makes it possible to use all of them together without conflict.
- **Custom tans and tattoos** - Why yes, you can make your girl look like yakuza.
- **Vanilla game fixes** - We fix various drivers issues, as well as make it possible to run AA2 on newer sytstems (Windows 10, Linux). We've also fixed all Japanese locale issues.
- **Mod embedding** - A system of embedding mods into cards themselves and auto extraction for the ease of sharing.



- **POV**: in H-Mode press `A` to switch between POVs of participants. Press `Q` `W` `E` to restore to normal view.


### General:
- **Shadow Files**: Instead of only loading files from a .pp file, the game checks for a folder of the same name (without the pp ending) first. If this folder exists and contains a file of the required name, this file is loaded instead.
   - **Shadowing Sets**: grouped and isolated sets of shadow files. A folder called *sets* next to the pp files(works for both *AA2Play* and *AA2Edit*) can define shadowing sets. Each subfolder, if its name starts with a `!`, can contain folders with the name of a pp file to shadow in the same way as normal shadowing works. Example:
```
data
├ jg2e01_00_00
│ └ A00_10_00_00.xx			| Shadow file. Will replace face 10 from original pp
└ sets
  ├ !myActiveSet
  │ └ jg2e01_00_00
  │   └A00_20_00_00.xx			| Shadow set. Will replace face 20 from original pp
  └ myPassiveSet
    └ jg2e01_00_00
      └ A00_10_00_00.xx			| Shadow set. Will do nothing because set is disabled
```

### Card Data: adds additional card data to cards:
- **Tan**: Uses a tan from *AAEdit\data\override\tan*. The folder is expected to have the same format used for [Tantoo](http://www.hongfire.com/forum/forum/hentai-lair/hf-modding-translation/artificial-academy-2-mods/409828-skin-cum-tan-nipples-lips-manager-pack-aa2tantoo) (just copy its tan folder over). [Hight pixel density skin](https://mega.nz/#F!ZMh1QD5a!lPAANflSsmkjOjywPatYCg) is highly recommended. If it doesn't work and you get blackface get the same thing with Tantoo instead.
- **Hair Highlight**: Use a hair highlight from *AAEdit\data\override\hair_highlight*. *Note*: effectively the same as **Mesh Override** `(Asp00_20_00_00_00\*)->(file)`, so this will not work if you have custom highlights without this name prefix

These Features above are all simply special, wild-carded rules of one of the types below. While the rules below do NOT support wild cards naturally, all of these things could be done for specific slots or files using the rules directly. Whenever a High-Poly model is loaded (or all the time in AAEdit), these rules from the corresponding card are applied. The following generic rule types exist:
- **Mesh Texture Overrides**: A set of rules of the form `(TextureName)->(TextureFileOnHarddrive)`. Whenever a xx file is loaded that contains a texture of a given name, a different texture from the file system is loaded instead
- **Archive Overrides**: A set of rules of the form `(archive.pp|file)->(fileOnHarddrive)`. Whenever a given file is read from a given archive, a different file from the harddrive will be loaded instead.
- **Archive Redirects**: A set of rules of the form `(archive.pp|file)->(archive.pp|file)`. Whenever a given file is read from a given archive, a different file from a different archive is loaded instead.
- **Object Overrides**:  A set of rules of the form `(FrameName)->(file.xxo)`. Whenever a xx file is loaded that contains the given FrameName, it is replaced by the object in the given xxo file. The format of the given file must be exactly how it is stored inside the xx file. There is a different exe in this project that can extract those: `XXObjectExtracter`. *Note*: by the looks of it, the file is radically different depending on the xx file version. These overrides probably only work for xx files version 8.
  
### Additional card based features:
- **Save Override Files**: all files target by these overrides, can be saved inside the card; if such a card is opened or previewed in the editor, the files will be extracted automatically if they're missing.
- **Eye Textures**: a seperate texture can be used for the right eye
- **Body Sliders**: additional sliders for certain body parts
- **Submesh Shadows/Outlines (SMSH/SMOL)**: can change outline or shadow color for this character (you can pick specific submeshes by their material names)
- **Tan Color**: can change the tan color from its brown shade
- **Additional Hairs**: more hairs than the original 4 can be added. Note that these hairs do not have low poly modles (yet)
- **Body Modification**: can put an SRT(*S*cale *R*otation *T*ranslation) matrix in front of frames or modify the SRT matrix of bones. Essentially generic body(or hair, or outfit, or anything) sliders. Only use if you are aware of how AA2 handles bodies.
  - *Note*: due to how AA2 handles frames, this is not the same as changing a frame in SB3U. rotations and scales will affect the original matrix of this frame. Adjust your matrix accordingly to make up for the incorrect translations.

### Tech-Support (in order from most preferable to least preferable)
- Ask questions in [/aa2g/](https://boards.4chan.org/vg/aa2g)
- Try asking questions in [this discord channel](https://discord.gg/5MfdAPT)
- Make an issue on github

### I have NVIDIA crashes or glitches. Help!

Try all combinations of enabling and disabling **Type 2 Renderer**, **wined3d** (must disable win10fix) and **Software Vertex Processing**.
