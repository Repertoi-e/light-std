Function ReplaceInFile(strFileName, strOldText, strNewText)

Const ForReading = 1    
Const ForWriting = 2

Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.OpenTextFile(strFileName, ForReading)
strText = objFile.ReadAll
objFile.Close

strNewText = Replace(strText, strOldText, strNewText)
Set objFile = objFSO.OpenTextFile(strFileName, ForWriting)
objFile.Write strNewText  'WriteLine adds extra CR/LF
objFile.Close

End Function

Set objFSO = CreateObject("Scripting.FileSystemObject")
strScriptPath = WScript.ScriptFullName
strScriptDir = objFSO.GetParentFolderName(strScriptPath)

Dim arg
arg = WScript.Arguments(0)
Call ReplaceInFile(strScriptDir & "..\build\" & target & "lstd.vcxproj", "<ClInclude Include=""..\..\src\lstd\lstd.h"" />", "<ClCompile Include=""..\..\src\lstd\lstd.h"">" & vbCrLf & "      <CompileAs Condition=""'$(Configuration)|$(Platform)'=='Debug|x64'"">CompileAsHeaderUnit</CompileAs>" & vbCrLf & "      <CompileAs Condition=""'$(Configuration)|$(Platform)'=='Release|x64'"">CompileAsHeaderUnit</CompileAs>" & vbCrLf & "      <CompileAs Condition=""'$(Configuration)|$(Platform)'=='DebugOptimized|x64'"">CompileAsHeaderUnit</CompileAs>" & vbCrLf & "    </ClCompile>")
