function PeekD(addr)
	return string.unpack("<I4", Peek(addr, 4))
end

function PokeD(addr, d)
	return Poke(addr, string.pack("<I4", d))
end


function GWrap(f)
	return function(addr, ...)
		return f(addr + GameBase, ...)
	end
end

function XchgD(addr, d)
	local orig = PeekD(addr)
	PokeD(addr, d)
	return orig
end

GPeek = GWrap(Peek)
GPoke = GWrap(Poke)
GPeekD = GWrap(PeekD)
GXchgD = GWrap(XchgD)

