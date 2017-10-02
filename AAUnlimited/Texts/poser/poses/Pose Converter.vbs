Set fso = CreateObject("Scripting.FileSystemObject")

For Each poseFile in Wscript.Arguments
	Set f = fso.OpenTextFile(poseFile)
	c = f.Read(1)
	f.Close
	If c = "{" Then
		fso.MoveFile poseFile, poseFile + ".pose"
	End If
Next
