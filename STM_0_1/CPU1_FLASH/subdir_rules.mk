################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/STM_0_1/Headers" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/STM_0_1/Functions" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --advice:performance=all --define=CPU1 --define=_FLASH --define=_LAUNCHXL_F28379D -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/STM_0_1/Headers" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/STM_0_1/Functions" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --advice:performance=all --define=CPU1 --define=_FLASH --define=_LAUNCHXL_F28379D -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


