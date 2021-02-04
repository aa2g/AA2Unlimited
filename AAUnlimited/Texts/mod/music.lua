--@INFO Change music anywhere, set custom H music

-- This mod allows you to:
--   - Change the background music anywhere. (F5/F6)
--   - Prevent the music from being automatically changed based on mood. (F7)
--   - Automatically change the music in H scenes based on the room loaded.
--
-- To set up H music preferences, edit "mod/musicconfig.lua".
--
-- You can have any arbitrary amount of custom songs beyond the 13
-- provided by the game. If a song doesn't exist it is ignored.

local _M = {}

local opts = {
	{"override_h", 1, "Override H-scene music: %b{Override H-scene music with the ones set in musicconfig.lua.}"},
}

-- base game state struct
local base_offset = 0x367F48

local state_offset = base_offset + 0xE284

function peek_pointer(base, ...)
   local it = g_peek_dword(base)
   for _, off in ipairs({...}) do
      it = peek_dword(it + off)
   end
   return it
end

function poke_pointer(base, ...)
   local it = g_peek_dword(base)
   local offs = {...}
   for i, off in ipairs(offs) do
      if i == #offs-1 then
         poke_dword(it + off, offs[i+1])
      else
         it = peek_dword(it + off)
      end
   end
end

local is_h = false

local function readfile(path)
    local file, err = io.open(path, "rb")
    if not file then return nil, err end
    local data = file:read "*a"
    local size = file:seek()
    file:close()
    return data, size
end

-- Function which returns music ID
local musicconfig = {}

local function load_music_prefs()
   local path = aau_path("mod/musicconfig.lua")
   local chunk, err = loadfile(path)
   if chunk == nil then
      log.warn("Failed to read music preferences: %s", err)
      return
   end

   musicconfig = chunk

   log.spam("Loaded music preferences.")
end

local function is_music_changing()
   return peek_pointer(state_offset, 0x28, 0x14, 0x18, 0x5C0) == 1
end

local function get_song()
   return peek_pointer(state_offset, 0x28, 0x14, 0x18, 0x5D8)
end

local function get_song_h()
   return g_peek_dword(0x167822)
end

local function set_song(id)
   if get_song() == id then return end

   log.info("Set song: %s", id)

   poke_pointer(state_offset, 0x28, 0x14, 0x18, 0x5D8, math.max(id, 0))

   if not is_h then
      poke_pointer(state_offset, 0x28, 0x14, 0x18, 0x5D0, 0)
      poke_pointer(state_offset, 0x28, 0x14, 0x18, 0x5C0, 1)
   end
end

local function set_song_h(id)
   if get_song_h() == id then return end

   log.info("Set song (H): %s", id)

   -- H music constant: AA2Play.exe + 0x167821
   g_poke_dword(0x167822, math.max(id, 0))

   if is_h then
      poke_pointer(state_offset, 0x28, 0x14, 0x18, 0x5D0, 0)
      poke_pointer(state_offset, 0x28, 0x14, 0x18, 0x5C0, 1)
   end
end

local DEFAULT_MUSIC = 7
local DEFAULT_H_MUSIC = 9
local DEFAULT_ROOM_MUSIC = 11
local DEFAULT_SUNDAY_MUSIC = 13

local function set_song_room(id)
   log.spam("Set room: %s", id)
   g_poke(0x487A3, string.pack("B", id))
end

local function set_song_sunday(id)
   log.spam("Set sunday: %s", id)
   g_poke(0x487E8, string.pack("B", id))
   g_poke(0x48790, string.pack("B", id))

end

local function is_auto_music_enabled()
   return g_peek(0x167413, 1) == "\x89"
end

local function get_current_room_id()
   local player = GetPlayerCharacter()
   local inst = GetCharInstData(player.m_seat)
   return inst:GetCurrentRoom()
end

local function update_h_song_from_room(room_id)
   if is_h then return end
   if not is_auto_music_enabled() then return end
   if not musicconfig then return end

   room_id = room_id or get_current_room_id()

   local ok, song_id = xpcall(musicconfig, debug.traceback, room_id)
   if not ok then
      log.info("Failed to update h song: %s", song_id)
      song_id = DEFAULT_H_MUSIC
   end

   log.info("Update h song: room %s -> %s", room_id, song_id)
   set_song_h(song_id)
end

local function count_charas_in_room()
   local cur_room = get_current_room_id()
   local count = 0
   for i=0,24 do
      local inst = GetCharInstData(i)
      if inst and inst:GetCurrentRoom() == cur_room then
         count = count + 1
      end
   end
   return count
end

local orig_bytes, orig_bytes2
local set_music = true

function on.period(period)
   if period == 1 then
      set_music = true
   end
end

function on.room_change(inst)
   if GetPlayerCharacter() == inst.m_char then
      update_h_song_from_room()
   end

   -- emulate original behavior where music is reset if player becomes
   -- alone, but only one time instead of every frame so music can
   -- still be changed
   local count = count_charas_in_room()

   if count > 1 and not set_music then
      set_music = true
   end

   if count == 1 and set_music and is_auto_music_enabled() then
      set_song(DEFAULT_MUSIC)
      set_music = false
   end
end

function on.move(inst)
   -- log.spam("MOVE: %s", inst)
   if GetPlayerCharacter() == inst.m_char then
      update_h_song_from_room()
   end

   local count = count_charas_in_room()

   if count > 1 and not set_music then
      set_music = true
   end

   if count == 1 and set_music and is_auto_music_enabled() then
      set_song(DEFAULT_MUSIC)
      set_music = false
   end
end

function on.convo()
   update_h_song_from_room()
end

function on.load_class()
   set_song_h(DEFAULT_H_MUSIC)
end

-- allow/prevent music from being switched automatically on mood/scene
-- change
local function toggle_auto_music()
   if exe_type ~= "play" then return end

   -- TODO only stub music flag set, not jump
   if is_auto_music_enabled() then
      log.info("Disabling music auto change.")
      orig_bytes = g_peek(0x167413, 36)
      g_poke(0x167413, string.rep("\x90", 36))
      local cur_song = get_song()
      set_song_h(cur_song)
      set_song_room(cur_song)
      set_song_sunday(cur_song)
      -- prevent music being forced to 7 if player is alone
      orig_bytes2 = g_peek(0x167C24, 1)
      g_poke(0x167C24, "\xEB")
   else
      log.info("Enabling music auto change.")
      g_poke(0x167413, orig_bytes)
      update_h_song_from_room()
      set_song_room(DEFAULT_ROOM_MUSIC)
      set_song_sunday(DEFAULT_SUNDAY_MUSIC)
      g_poke(0x167C24, orig_bytes2)
   end
end

function on.start_h(hi)
   is_h = hi
end

function on.end_h()
   is_h = false
end

local PREV_SONG_KEY = 0x74 -- F5
local NEXT_SONG_KEY = 0x75 -- F6
local TOGGLE_AUTO_KEY = 0x76 -- F7

function on.keydown(k)
   if is_h then
      local current = get_song_h()

      if k == NEXT_SONG_KEY then
         set_song_h(current + 1)
      elseif k == PREV_SONG_KEY then
         set_song_h(current - 1)
      end
   else
      local current = get_song()

      if k == NEXT_SONG_KEY then
         set_song(current + 1)
      elseif k == PREV_SONG_KEY then
         set_song(current - 1)
      end
   end

   if k == TOGGLE_AUTO_KEY then
      toggle_auto_music()
   end
end

local load_room_fn = 0x93BB0

-- AA2Play.exe+167C24 - EB 14                 - jmp AA2Play.exe+167C3A { change this to jmp?!}


function _M:load()
   mod_load_config(self, opts)

   if opts.override_h then
      load_music_prefs()
   else
      musicconfig = nil
   end
end

function _M:unload()
end

function _M:config()
   mod_edit_config(self, opts, "Music options")

   if opts.override_h then
      load_music_prefs()
   else
      musicconfig = nil
   end
end

return _M
