start /w ..\projucer\Projucer.exe .\AudioAnalyzer.jucer
call code.cmd .\snap\snapcraft.yaml .\snap\gui\audioanalyzer.desktop
start /w D:\Program_Files\Microsoft_Visual_Studio\2022\Community\Common7\IDE\devenv.exe .\Builds\VisualStudio2022\AudioAnalyzer.sln
D:\Program_Files\SmartGit\bin\smartgit.exe .
