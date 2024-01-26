##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=neweditor
ConfigurationName      :=Debug
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/media/jmg/New450/src/olcpixelengine/olcPixelGameEngine/isometric/isometric
ProjectPath            :=/media/jmg/New450/src/neweditor/eddy
IntermediateDirectory  :=../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy
OutDir                 :=../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=jose ondina
Date                   :=25/01/24
CodeLitePath           :=/home/jmg/.codelite
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
OutputFile             :=../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/bin/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  
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
CXXFLAGS :=  -g -O0 -Wall -std=c++20 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): ../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/.d $(Objects) 
	@mkdir -p "../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@mkdir -p "../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy"
	@mkdir -p ""../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/bin""

../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/.d:
	@mkdir -p "../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy"

PreBuild:


##
## Objects
##
../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(ObjectSuffix): main.cpp ../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/media/jmg/New450/src/neweditor/eddy/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(ObjectSuffix) -MF../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(DependSuffix) -MM main.cpp

../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy/main.cpp$(PreprocessSuffix) main.cpp


-include ../../olcpixelengine/olcPixelGameEngine/isometric/isometric/build-$(ConfigurationName)/__/__/__/__/neweditor/eddy//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


