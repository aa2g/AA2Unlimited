-- Used by music.lua.
-- Each ID corresponds to a song in data/CustomBGM.
-- Default is 9 (AA2Bgm09.wav)

local room_id = ...

local SONGS = {
   [0]	= 9,	-- School gates
   [1]	= 9,	-- Back street
   [2]	= 9,	-- Outside gymnasium
   [3]	= 9,	-- School route
   [4]	= 9,	-- Mens changing room
   [5]	= 9,	-- Girls changing room
   [6]	= 9,	-- Mens shower
   [7]	= 9,	-- Girls shower
   [8]	= 9,	-- Lockers
   [9]	= 9,	-- Outside lounge
   [10]	= 9,	-- Outside toilets
   [11]	= 9,	-- Outside classroom
   [12]	= 9,	-- Rooftop access
   [13]	= 9,	-- Old building 1st floor
   [14]	= 9,	-- Old building 2nd floor
   [15]	= 9,	-- Old building 3rd floor
   [16]	= 9,	-- Teachers lounge
   [17]	= 9,	-- Infirmary
   [18]	= 9,	-- Library
   [19]	= 9,	-- Classroom
   [20]	= 9,	-- Mens Toilets
   [21]	= 9,	-- Girls Toilets
   [22]	= 9,	-- Rooftop
   [23]	= 9,	-- Outside counsel
   [24]	= 9,	-- Outside cafeteria
   [25]	= 9,	-- Courtyard
   [26]	= 9,	-- 2nd floor hallway
   [27]	= 9,	-- 3rd floor passage
   [28]	= 9,	-- Swimming pool
   [29]	= 9,	-- Track
   [30]	= 9,	-- Sports facility
   [31]	= 9,	-- Dojo
   [32]	= 9,	-- Gymnasium
   [33]	= 9,	-- Arts room
   [34]	= 9,	-- Multipurpose room
   [35]	= 9,	-- Japanese-style room
   [36]	= 9,	-- Behind Dojo
   [37]	= 9,	-- Outside dojo
   [38]	= 9,	-- Cafeteria
   [39]	= 9,	-- Outside station
   [40]	= 9,	-- Karaoke
   [41]	= 9,	-- Boys' night room
   [42]	= 9,	-- Girls' night room
   [43]	= 9,	-- Boys' room
   [44]	= 9,	-- Girls' room
   [45]	= 9,	-- Boys's Shower Stall
   [46]	= 9,	-- Girl's Shower Stall
   [47]	= 9,	-- Boys' Toilet Stall
   [48]	= 9,	-- Girls' Toilet Stall
   [49]	= 9,	-- Counseling Room
   [50]	= 9,	-- Gym Storeroom
   [51]	= 9,	-- Love Hotel
   [52]	= 9,	-- Machine Room
   [53]	= 9,	-- ???
   [54]	= 9,	-- ???
   [55]	= 9,	-- ???
   [56]	= 9,	-- ???
}

return SONGS[room_id]
