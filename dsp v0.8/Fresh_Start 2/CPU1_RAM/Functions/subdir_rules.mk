################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Functions/Analog\ In.obj: ../Functions/Analog\ In.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Functions" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Headers" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Functions/Analog In.d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Functions/Analog\ Out.obj: ../Functions/Analog\ Out.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Functions" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Headers" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Functions/Analog Out.d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Functions/Assorted\ Macros.obj: ../Functions/Assorted\ Macros.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Functions" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Headers" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Functions/Assorted Macros.d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Functions/%.obj: ../Functions/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Functions" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Headers" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Functions/$(basename $(<F)).d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Functions/Serial\ In.obj: ../Functions/Serial\ In.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Functions" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Headers" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Functions/Serial In.d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Functions/Serial\ Out.obj: ../Functions/Serial\ Out.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Functions" --include_path="C:/Users/yaoji/OneDrive - personalmicrosoftsoftware.uci.edu/DSP/v0.8/Fresh_Start 2/Headers" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_2_00_00_03_Software/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Functions/Serial Out.d_raw" --obj_directory="Functions" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


