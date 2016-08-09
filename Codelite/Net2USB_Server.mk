##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Net2USB_Server
ConfigurationName      :=Debug
WorkspacePath          := "/home/dutao/Net2USB/Codelite"
ProjectPath            := "/home/dutao/Net2USB/Codelite"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=
Date                   :=09/08/16
CodeLitePath           :="/home/dutao/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Net2USB_Server.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := -lboost_system -lboost_thread -pthread 
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS := -std=c++11 -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/ASIOLib_Executor.cpp$(ObjectSuffix) $(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(ObjectSuffix) $(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(ObjectSuffix) $(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/ASIOLib_Executor.cpp$(ObjectSuffix): ../ASIOLib/Executor.cpp $(IntermediateDirectory)/ASIOLib_Executor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dutao/Net2USB/ASIOLib/Executor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ASIOLib_Executor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ASIOLib_Executor.cpp$(DependSuffix): ../ASIOLib/Executor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ASIOLib_Executor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ASIOLib_Executor.cpp$(DependSuffix) -MM "../ASIOLib/Executor.cpp"

$(IntermediateDirectory)/ASIOLib_Executor.cpp$(PreprocessSuffix): ../ASIOLib/Executor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ASIOLib_Executor.cpp$(PreprocessSuffix) "../ASIOLib/Executor.cpp"

$(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(ObjectSuffix): ../ASIOLib/SerialPort.cpp $(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dutao/Net2USB/ASIOLib/SerialPort.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(DependSuffix): ../ASIOLib/SerialPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(DependSuffix) -MM "../ASIOLib/SerialPort.cpp"

$(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(PreprocessSuffix): ../ASIOLib/SerialPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ASIOLib_SerialPort.cpp$(PreprocessSuffix) "../ASIOLib/SerialPort.cpp"

$(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(ObjectSuffix): ../ASIOLib/SerialRW.cpp $(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dutao/Net2USB/ASIOLib/SerialRW.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(DependSuffix): ../ASIOLib/SerialRW.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(DependSuffix) -MM "../ASIOLib/SerialRW.cpp"

$(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(PreprocessSuffix): ../ASIOLib/SerialRW.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ASIOLib_SerialRW.cpp$(PreprocessSuffix) "../ASIOLib/SerialRW.cpp"

$(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(ObjectSuffix): ../Net2USB_Server/N2U_Server.cpp $(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dutao/Net2USB/Net2USB_Server/N2U_Server.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(DependSuffix): ../Net2USB_Server/N2U_Server.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(DependSuffix) -MM "../Net2USB_Server/N2U_Server.cpp"

$(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(PreprocessSuffix): ../Net2USB_Server/N2U_Server.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Net2USB_Server_N2U_Server.cpp$(PreprocessSuffix) "../Net2USB_Server/N2U_Server.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


