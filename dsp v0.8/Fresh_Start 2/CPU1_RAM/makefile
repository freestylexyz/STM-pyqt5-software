################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./F2837xD_CodeStartBranch.obj" \
"./F2837xD_DefaultISR.obj" \
"./F2837xD_GlobalVariableDefs.obj" \
"./F2837xD_Gpio.obj" \
"./F2837xD_Ipc.obj" \
"./F2837xD_PieCtrl.obj" \
"./F2837xD_PieVect.obj" \
"./F2837xD_SysCtrl.obj" \
"./F2837xD_usDelay.obj" \
"./main.obj" \
"./Functions/Analog In.obj" \
"./Functions/Analog Out.obj" \
"./Functions/Assorted Macros.obj" \
"./Functions/AutoPlusMinus.obj" \
"./Functions/BabySteps.obj" \
"./Functions/DAC.obj" \
"./Functions/DebugGet.obj" \
"./Functions/DebugSet.obj" \
"./Functions/Deposition.obj" \
"./Functions/GiantStep.obj" \
"./Functions/HighResOut.obj" \
"./Functions/Hop.obj" \
"./Functions/Parabola.obj" \
"./Functions/PinInit.obj" \
"./Functions/Ramp.obj" \
"./Functions/RampDiag.obj" \
"./Functions/ReadSeq.obj" \
"./Functions/ScanLine.obj" \
"./Functions/Serial In.obj" \
"./Functions/Serial Out.obj" \
"./Functions/Spec.obj" \
"./Functions/Square.obj" \
"./Functions/TipApproach.obj" \
"./Functions/Track.obj" \
"./Functions/UserDelay.obj" \
"./Functions/UserDelayLong.obj" \
"./Functions/Version.obj" \
$(GEN_CMDS__FLAG) \
-lrts2800_fpu32.lib \
-l2837xD_RAM_lnk_cpu1.cmd \
-lF2837xD_Headers_nonBIOS_cpu1.cmd \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Functions/subdir_vars.mk
-include subdir_rules.mk
-include Functions/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Fresh_Start\ 2.out \

EXE_OUTPUTS__QUOTED += \
"Fresh_Start 2.out" \

BIN_OUTPUTS += \
Fresh_Start\ 2.hex \

BIN_OUTPUTS__QUOTED += \
"Fresh_Start 2.hex" \


# All Target
all: Fresh_Start\ 2.out

# Tool invocations
Fresh_Start\ 2.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C2000 Linker'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi -z -m"Fresh_Start 2.map" --stack_size=0x100 --warn_sections -i"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/lib" -i"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" -i"C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/cmd" -i"C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/cmd" --reread_libs --display_error_number --xml_link_info="Fresh_Start 2_linkInfo.xml" --entry_point=code_start --rom_model -o "Fresh_Start 2.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

Fresh_Start\ 2.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: C2000 Hex Utility'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/hex2000"  -o "Fresh_Start 2.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "F2837xD_CodeStartBranch.obj" "F2837xD_DefaultISR.obj" "F2837xD_GlobalVariableDefs.obj" "F2837xD_Gpio.obj" "F2837xD_Ipc.obj" "F2837xD_PieCtrl.obj" "F2837xD_PieVect.obj" "F2837xD_SysCtrl.obj" "F2837xD_usDelay.obj" "main.obj" "Functions\Analog In.obj" "Functions\Analog Out.obj" "Functions\Assorted Macros.obj" "Functions\AutoPlusMinus.obj" "Functions\BabySteps.obj" "Functions\DAC.obj" "Functions\DebugGet.obj" "Functions\DebugSet.obj" "Functions\Deposition.obj" "Functions\GiantStep.obj" "Functions\HighResOut.obj" "Functions\Hop.obj" "Functions\Parabola.obj" "Functions\PinInit.obj" "Functions\Ramp.obj" "Functions\RampDiag.obj" "Functions\ReadSeq.obj" "Functions\ScanLine.obj" "Functions\Serial In.obj" "Functions\Serial Out.obj" "Functions\Spec.obj" "Functions\Square.obj" "Functions\TipApproach.obj" "Functions\Track.obj" "Functions\UserDelay.obj" "Functions\UserDelayLong.obj" "Functions\Version.obj" 
	-$(RM) "F2837xD_DefaultISR.d" "F2837xD_GlobalVariableDefs.d" "F2837xD_Gpio.d" "F2837xD_Ipc.d" "F2837xD_PieCtrl.d" "F2837xD_PieVect.d" "F2837xD_SysCtrl.d" "main.d" "Functions\Analog In.d" "Functions\Analog Out.d" "Functions\Assorted Macros.d" "Functions\AutoPlusMinus.d" "Functions\BabySteps.d" "Functions\DAC.d" "Functions\DebugGet.d" "Functions\DebugSet.d" "Functions\Deposition.d" "Functions\GiantStep.d" "Functions\HighResOut.d" "Functions\Hop.d" "Functions\Parabola.d" "Functions\PinInit.d" "Functions\Ramp.d" "Functions\RampDiag.d" "Functions\ReadSeq.d" "Functions\ScanLine.d" "Functions\Serial In.d" "Functions\Serial Out.d" "Functions\Spec.d" "Functions\Square.d" "Functions\TipApproach.d" "Functions\Track.d" "Functions\UserDelay.d" "Functions\UserDelayLong.d" "Functions\Version.d" 
	-$(RM) "F2837xD_CodeStartBranch.d" "F2837xD_usDelay.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

