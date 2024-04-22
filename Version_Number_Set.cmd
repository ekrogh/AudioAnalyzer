start /w ..\projucer\Projucer.exe .\GuitarFineTune.jucer
call code.cmd .\snap\snapcraft.yaml .\snap\gui\guitarfinetune.desktop
start /w D:\Program_Files\Microsoft_Visual_Studio\2022\Community\Common7\IDE\devenv.exe .\Builds\VisualStudio2022\GuitarFineTune.sln
D:\Program_Files\SmartGit\bin\smartgit.exe .
