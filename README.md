# AA2Unlimited

AAFace was supposed to make the Maker do things that it allready could, but wouldnt want to. 
It basically gave a UI to these features.

AAUnlimited, on the other hand, tries to make the game (and maker) do new things.

As a Part of this, it puts additional Information into the card images in form of a PNG chunk right before the first IEND chunk.
The Game treats these as part of the image, which means cards produced with these additional data will still work normally 
if used without AAUnlimited. However, it also means that saving the card with a normal Maker will remove this data, and i asume 
so will AASnowflake.

### Features:
- H-Ai: When forced by an NPC, saying no means yes. In H-Mode, the Buttons will be greyed out, and the H-Ai will take over.
- Shadowing: When opening a File, the Game/Maker first searches for the file in a Folder with the name of the archive file 
  (minues the .pp ending). If found, he will use this one instead.
- Mesh Texture Override: On a Card-Basis, rules that replace certain Textures that are read from a 
  xx Model file with other textures located in AAEdit/data/texture/override/. The rule will be applied while the High-Poly
  Model of a Character is being loaded in the Game. Can be used to, for example, replace the Hair-Highlight with a different one.
- Archive File Override: On a Card-Basis, rules that replace any file loaded from an Archive with any other file
  located in AAPlay/data/.
  
- [technically supports 255 tan slots. However, it seems to crash when you close the game, which is really annoying.]

### Test Builds (more up-to-date but card compatibility is not guaranteed; back up your stuff)
[MEGA](https://mega.nz/#F!IZs3VY5b!axfi9mlecCwLDdvpvIIbzw)
