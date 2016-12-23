#pragma once

#include <Windows.h>
/*
found this on hongfire, seems to be accurate to what AA2 does.

The .xx file format. 
Open this text file in notepad or the formatting looks like crap.
-Rawmonk

--------------------------------------------------------------------------------------------------------------
All values in litle endian first (Intel x86)

CHAR	 1 Byte ASCII
WORD	16 bit unsigned
DWORD 	32 bit unsigned	(LongWord)
FLOAT	32 bit float	(Single)
b	byte


Pseudo Code:
--------------------------------------------------------------------------------------------------------------
Read_Object(NumberOfchildren)
	for each NumberOfchildren
		FileRead Object
		if Object.MeshCount > 0
			FileRead Meshes
			FileRead BoneCount
			if BoneCount > 0 
				FileRead Bones
		if Object.NumberOfChildren > 0
			Read_Object(Object.NumberOfChildren)



HEADER
--------------------------------------------------------------------------------------------------------------

    ID     ?              ?
 |------|-----|------------------------|
    5b   DWORD           17b


OBJECT SECTION
--------------------------------------------------------------------------------------------------------------

OBJECT NAME
  Length    Object Name   00 							--
 |------|----------------|--|	(For each CHAR xor 0xFF)			 |
  DWORD                   1b 							 |
	|-- Length x CHAR --|							 |
										 |
										 |
Transformation									 |
  Child Count    		         Matrix			 		 |
 |------------||-------------------------------------------------------|	 |
  DWORD         			16 x FLOAT				 |--- If "Mesh Count" = 0 just
               |-------------------------- 64b ------------------------|	 |    repeat this section
										 |
										 |
MESH COUNT AND MESH MAXIMUM SIZE?						 |
    ?    Mesh Count MIN SIZE?   MAX SIZE?     ?					 |
 |------|----------|-----------|-----------|------|				 |
   16b   DWORD      3 x FLOAT   3 x FLOAT    16b				 |
 |----------------------- 60b --------------------|				 |
										--


MESH SECTION
--------------------------------------------------------------------------------------------------------------

  Mesh Type 	( Value 0 - standard mesh or 1 - User interface graphic
 |---------|	  Mesh type 1 always have 1 mesh with 4 vertices and 2 faces )
     1b		


MESH HEADER									--
         ?        Material ID							 |
 |---------------|-----------|							 |
        16b          DWORD							 |
 |------------ 20b ----------|							 |
										 |
										 |
FACES (Number of faces = Face count / 3)					 |
  Face count FIRST  SECOND THIRD						 |
 |----------|------|------|------|--..						 |
    DWORD    WORD   WORD   WORD							 |--- Repeat "Mesh Count" times
            |------ count x 2b -------|						 |
										 |
										 |
VERTICES NORMALS UVS ETC							 |
  Vertex count Index  V-Coords   Weight     Bone   Normal     UV		 |
 |------------|------|----------|----------|------|----------|----------|--..	 |
     DWORD     DWORD  3 x FLOAT  3 x FLOAT  4 x 1b 3 x FLOAT  2 x FLOAT		 |
              |---------------------- Count x 52b --------------------------|	 |
										--					


MESH SCALE INFO						--
  Top Left   Top Right   Bottom Left  Bottom Right	 |
 |----------|----------|-------------|------------|	 |--- Read if 
  2 x FLOAT 2 x FLOAT     2 x Float    2 x FLOAT	 |    "Mesh Type" = 1
 |--------------------- 32b ----------------------|	 |
							--

Duplicated vertex list
  Vertex count       ?        Index  V-Coords   Weight     Bone   Normal     UV
 |------------|--------------|------|----------|----------|------|----------|----------|--..
      WORD           8b
                             |---------------------- Count x 52b --------------------------|


BONE SECTION
--------------------------------------------------------------------------------------------------------------

  Bone count 
 |----------|
    DWORD    


BONE NAME									--
  Length    Bone Name     00 							 |
 |------|----------------|--|	(For each CHAR xor 0xFF)			 |
  DWORD                   1b 							 |
	|-- Length x CHAR --|							 |
									 	 |
										 |--- Repeat "Bone Count" times
 BONE INDEX    			        Matrix			 		 |
|------------||-------------------------------------------------------|		 |
 DWORD				       16 x FLOAT				 |
              |-------------------------- 64b ------------------------|		 |
										--



MATERIALS (After all objects have been read)
--------------------------------------------------------------------------------------------------------------

    ?   Material Count
 |-----|--------------|
  DWORD     DWORD


  Length   Material Name  00 							--
 |------|----------------|--|	(For each CHAR xor 0xFF)			 |
  DWORD                   1b							 |
	|-- Length x CHAR --|							 |
										 |
										 |
  Diffuse    Ambient    Specular   Emmisive   SpecPower				 |
 |----------|----------|----------|----------|----------|			 |
  4 x FLOAT  4 x FLOAT  4 x FLOAT  4 x FLOAT  FLOAT				 |
 |----------------------- 68b --------------------------|			 |
										 |
										 |
  Length   Texture Name   00					--		 |
 |------|----------------|--|	(For each CHAR xor 0xFF)	 |		 |---- Repeat "Material Count" times
  DWORD                   1b					 |		 |
	|-- Length x CHAR --|					 |		 |
								 |		 |
								 |-- x 4	 |
  Texture Type         ?					 |		 |
 |------------|------------------|				 |		 |
     DWORD            12b					 |		 |
								--		 |
										 |
              ?									 |
 |--------------------------|							 |
             88b								 |
										--


TEXTURES
--------------------------------------------------------------------------------------------------------------

  Texture Count
 |-------------|
     DWORD


  Length   Texture Name   00							--
 |------|----------------|--|	(For each CHAR xor 0xFF)			 |
  DWORD                   1b							 |
	|-- Length x CHAR --|							 |
										 |
										 |
										 |
  0x00  Width  Height   0x01   0x01  Format  0x03  Img Type Checksum File Size	 |
 |-----|------|-------|------|------|------|------|--------|--------|---------|	 |---- Repeat "Texture Count" times
  DWORD DWORD  DWORD   DWORD  DWORD  DWORD  DWORD  DWORD    BYTE     DWORD	 |
										 |
										 |
              Texture File							 |
 |------------------------------------|						 |
              File Size x b							 |
										--

Format = bmp 0x14, tga 0x15
Checksum = read every 32 byte of the file from address 0x00 to the end
	   add them together in a DWORD then do DWORD & 0x000000FF = Checksum
Img Type = bmp 0x00, tga 0x02 might be imagetype for tga 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

--------------------------------------------------------------------------------------------------------------
END oF FILE
*/
namespace FileFormats {

struct XXFile {
	struct Object {
		DWORD m_nameLength;
		char* m_name;		//once-complemented in file
		DWORD m_childBoneCount;
		float m_transMatrix; //diff compared to parent object

		BYTE m_unknown1[16];
		//TODO finish
	};

	//file is a pointer to the start of the objects section in the xx file.
	//size will be returned so that file+retVal is a pointer to the next object
	static DWORD ReadObjectLength(BYTE* file);
};


}