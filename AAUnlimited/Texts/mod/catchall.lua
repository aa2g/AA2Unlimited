_EVENTS.WinMsg = function(m,w,l)
	print("got win",m,w,l)
end

_EVENTS.Seats.CardInitialize = function(n,rel)
	print('CardInitialize',n,rel)
end

_EVENTS.Seats.CardDestroy = function(n)
	print('CardDestroy',n)
end

_EVENTS.H.PreTick = function(n)
	print('PreTick',n)
end

_EVENTS.H.FocusCameraEvent = function(n)
	print('Focus',n)
end

_EVENTS.H.PostTick = function(n, contScene)
	print('PostTick',n)
end

_EVENTS.NpcActions.ClothesChangedEvent = function(a,b)
	print('Clothes',a,b)
	return b
end

_EVENTS.NpcActions.NpcAnswerEvent = function(a,b,c)
	print('Answer',a,b,c)
	return c
end

_EVENTS.NpcActions.NpcMovingActionEvent = function(u,p)
	print('Moving',u,p)
end

_EVENTS.NpcActions.NpcMovingActionPlanEvent = function(p)
--	print('Plan',p)
end

_EVENTS.PcActions.ClothesPickEvent = function(p)
	print('Pick',p)
	return p
end

_EVENTS.Convo.StartEvent = function()
	print('Start')
end

_EVENTS.Convo.EndEvent  = function()
	print('End')
end

_EVENTS.Convo.NpcPcInteractivePreTick  = function(p,s)
	print('InteractPre',p,s)
end

_EVENTS.Convo.NpcPcInteractivePostTick  = function(p,s)
	print('InteractPost',p,s)
end

_EVENTS.Convo.NpcPcNonInteractivePreTick = function(s)
	print('NpcNon',s)
end

_EVENTS.Convo.NpcPcNonInteractivePostTick = function(s)
	print('NpcPcnonPost',s)
end

_EVENTS.Convo.NpcAnswer = function(s)
	print('NpcAns',s)
end

_EVENTS.Convo.PcAnswer = function(s)
	print('PcAns',s)
end

_EVENTS.Time.PeriodEnds  = function(o,n)
	print('Period',o,n)
end

