Set wshShell = CreateObject ( "WScript.Shell" )
Set objFSO = CreateObject("Scripting.FileSystemObject")
objectName = InputBox("Please enter a frame name to extract:","Enter Frame Name")
If objectName = "" Then
	WScript.Quit
End If

aauPath = objFSO.GetParentFolderName(WScript.ScriptFullName)
filePath = WScript.Arguments(0)
filePathFolder = objFSO.GetParentFolderName(filePath)
fileName = objFSO.getBaseName(filePath)
objectFileName = fileName & "-" & objectName & ".xxo"
objectFilePath = filePathFolder & "\" & objectFileName

wshShell.Run chr(34) & aauPath & "\XXObjectExtracter.exe" & chr(34) & " " & chr(34) & filePath & chr(34) & " " & chr(34) & objectName & chr(34) & " " & chr(34) & objectFilePath & chr(34)
