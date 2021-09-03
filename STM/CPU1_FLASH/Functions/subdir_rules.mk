################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Functions/%.obj: ../Functions/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-c2000_20.2.4.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --cla_support=cla1 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/Users/wh740/OneDrive/Desktop/CCS workspace/STM/Functions" --include_path="C:/Users/wh740/OneDrive/Desktop/CCS workspace/STM/Headers" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-c2000_20.2.4.LTS/include" --include_path="C:/ti/C2000Ware_3_04_00_00_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_3_04_00_00_Software/device_support/f2837xd/common/include" -g --define=_LAUNCHXL_F28379D --define=CPU1 --define=_FLASH --display_error_number --diag_suppress=10063 --diag_warning=225 --preproc_with_compile --preproc_dependency="Functions/$(basename $(<F)).d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


