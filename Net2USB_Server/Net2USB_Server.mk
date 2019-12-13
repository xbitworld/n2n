##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Net2USB_Server
ConfigurationName      :=Debug
WorkspacePath          :=D:/Net2USB/Codelite
ProjectPath            :=D:/Net2USB/Net2USB_Server
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=dutao
Date                   :=06/08/2016
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :=link.exe  /nologo
SharedObjectLinkerName :=link.exe /DLL  /nologo
ObjectSuffix           :=.obj
DependSuffix           :=
PreprocessSuffix       :=
DebugSwitch            :=/Zi 
IncludeSwitch          :=/I
LibrarySwitch          := 
OutputSwitch           :=/OUT:
LibraryPathSwitch      :=/LIBPATH:
PreprocessorSwitch     :=/D
SourceSwitch           :=
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=/Fo
ArchiveOutputSwitch    :=/OUT:
PreprocessOnlySwitch   :=
ObjectsFileList        :="Net2USB_Server.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            := -lboost_system -pthread 
IncludePath            := $(IncludeSwitch)""C:/Program Files/Microsoft Visual Studio 8/VC/include"" $(IncludeSwitch).  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                :=$(LibraryPathSwitch)""C:/Program Files/Microsoft Visual Studio 8/VC/lib""  $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := lib.exe  /nologo
CXX      := cl.exe /nologo /c
CC       := cl.exe /nologo /c
CXXFLAGS := -std=c++11 -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := NASM


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
Objects0=$(IntermediateDirectory)/N2U_Server.cpp$(ObjectSuffix) $(IntermediateDirectory)/Executor.cpp$(ObjectSuffix) $(IntermediateDirectory)/SerialPort.cpp$(ObjectSuffix) $(IntermediateDirectory)/SerialRW.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "./Debug"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/N2U_Server.cpp$(ObjectSuffix): N2U_Server.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Net2USB/Net2USB_Server/N2U_Server.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/N2U_Server.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Executor.cpp$(ObjectSuffix): ../ASIOLib/Executor.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Net2USB/ASIOLib/Executor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Executor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/SerialPort.cpp$(ObjectSuffix): ../ASIOLib/SerialPort.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Net2USB/ASIOLib/SerialPort.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SerialPort.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/SerialRW.cpp$(ObjectSuffix): ../ASIOLib/SerialRW.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Net2USB/ASIOLib/SerialRW.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SerialRW.cpp$(ObjectSuffix) $(IncludePath)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


