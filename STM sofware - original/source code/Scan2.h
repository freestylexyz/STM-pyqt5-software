#define STOP_NOW_ABORTED 10

#define SET_DATA(DATA)    DATA->valid = 1; \
    strcpy(DATA->filename,"Unsaved"); \
    strcpy(DATA->full_filename,"Unsaved"); \
    DATA->x = scan_x; \
    DATA->y = scan_y; \
    DATA->z = scan_z; \
    DATA->x_range = x_range; \
    DATA->y_range = y_range; \
    DATA->min_z = min_z; \
    DATA->max_z = max_z; \
    DATA->clip_min = min_z; \
    DATA->clip_max = max_z; \
    DATA->step = scan_step; \
    DATA->size = scan_size; \
    DATA->x_gain = scan_x_gain; \
    DATA->y_gain = scan_y_gain; \
    DATA->z_gain=calc_z_gain(scan_scale); \
    DATA->z_freq=scan_freq; \
    scan_time = time(NULL); \
    DATA->x_offset_range=x_offset_range; \
    DATA->y_offset_range=y_offset_range; \
    DATA->bias=sample_bias; \
    DATA->bias_range=sample_bias_range; \
    if (scan_dir==BACKWARD_DIR) \
        DATA->scan_dir=BACKWARD_DIR; \
    else DATA->scan_dir=FORWARD_DIR; \
    if (x_first) DATA->scan_dir+=SCAN_X_FIRST; \
    DATA->i_setpoint=i_setpoint; \
    DATA->i_set_range=i_set_range; \
    DATA->amp_gain=tip_gain; \
    DATA->step_delay=step_delay; \
    DATA->inter_step_delay=inter_step_delay; \
    DATA->digital_feedback=digital_feedback; \
    DATA->tip_spacing=tip_spacing; \
    DATA->inter_line_delay=inter_line_delay; \
    DATA->scan_num=scan_num; \
    DATA->scan_feedback=scan_feedback; \
    DATA->read_ch=read_ch; \
    DATA->crash_protection=crash_protection; \
    DATA->overshoot=0; \
    if (overshoot_fast) DATA->overshoot|=OVERSHOOT_FAST; \
    if (overshoot_slow) DATA->overshoot|=OVERSHOOT_SLOW; \
    DATA->overshoot_percent=overshoot_percent; \
    DATA->overshoot_wait1=overshoot_wait1; \
    DATA->overshoot_wait2=overshoot_wait2; \
    DATA->read_feedback=read_feedback; \
    DATA->scan_dither0=scan_dither0; \
    DATA->scan_dither1=scan_dither1; \
    DATA->dither_wait=dither_wait; \
    DATA->dither0=dither0; \
    DATA->dither1=dither1; \
    DATA->version=STM_DATA_VERSION;

#define SCAN_FEEDBACK_WAIT      100

#define READ_Z() \
    for(this_seq=0;this_seq<SD->read_seq_num;this_seq++) \
    { \
        if (feedback_now!=SD->read_seq[this_seq].feedback) \
        { \
            dio_feedback(SD->read_seq[this_seq].feedback); \
            feedback_now=SD->read_seq[this_seq].feedback; \
        } \
        if (SD->read_seq[this_seq].wait1) \
        { \
            dio_start_clock(SD->read_seq[this_seq].wait1); \
            dio_wait_clock(); \
        } \
        if (dither0_now!=SD->read_seq[this_seq].dither0) \
        { \
            dio_dither(0,SD->read_seq[this_seq].dither0); \
            dither0_now=SD->read_seq[this_seq].dither0; \
        } \
        if (SD->read_seq[this_seq].wait2) \
        { \
            dio_start_clock(SD->read_seq[this_seq].wait2); \
            dio_wait_clock(); \
        } \
        if (dither1_now!=SD->read_seq[this_seq].dither1) \
        { \
            dio_dither(1,SD->read_seq[this_seq].dither1); \
            dither1_now=SD->read_seq[this_seq].dither1; \
        } \
        if (SD->read_seq[this_seq].wait3) \
        { \
            dio_start_clock(SD->read_seq[this_seq].wait3); \
            dio_wait_clock(); \
        } \
	    if( SD->read_seq[this_seq].do_ramp_bias) \
		{ \
	        dio_out(SD->read_seq[this_seq].ramp_ch,SD->read_seq[this_seq].ramp_value); \
	        dac_data[sample_bias_ch] = SD->read_seq[this_seq].ramp_value; \
		} \
        if (SD->read_seq[this_seq].wait4) \
        { \
            dio_start_clock(SD->read_seq[this_seq].wait4); \
            dio_wait_clock(); \
        } \
        if (SD->read_seq[this_seq].record) \
        { \
            if (read_ch_now!=SD->read_seq[this_seq].read_ch) \
            { \
                dio_in_ch(SD->read_seq[this_seq].read_ch); \
                read_ch_now=SD->read_seq[this_seq].read_ch; \
            } \
            this_data[data_pos]=dio_read(SD->read_seq[this_seq].num_samples); \
            if (read_ch_now==zi_ch) \
            { \
                z=this_data[data_pos]; \
                this_data[data_pos]+=delta_z; \
                z_scan_num=SD->read_seq[this_seq].num_samples; \
            } \
            if (!data_pos) \
            { \
                if (this_data[data_pos] > max_z) \
                    max_z = this_data[data_pos]; \
                if (this_data[data_pos] < min_z) \
                    min_z = this_data[data_pos]; \
            } \
            data_pos++; \
        } \
    } \
    if (dither0_now!=SD->scan_dither0) \
    { \
        dio_dither(0,SD->scan_dither0); \
        dither0_now=SD->scan_dither0; \
    } \
    if (dither1_now!=SD->scan_dither1) \
    { \
        dio_dither(1,SD->scan_dither1); \
        dither1_now=SD->scan_dither1; \
    } \
    if (feedback_now!=SD->scan_feedback) \
    { \
        dio_feedback(SD->scan_feedback); \
        feedback_now=SD->scan_feedback; \
    }


#define READ_Z_OLD() \
    if (scan_feedback!=read_feedback) \
    { \
        dio_feedback(read_feedback); \
        dio_start_clock(SCAN_FEEDBACK_WAIT); \
        dio_wait_clock(); \
    } \
    if (scan_dither0!=dither0) \
    { \
        dio_dither(0,dither0); \
    } \
    if (scan_dither1!=dither1) \
    { \
        dio_dither(1,dither1); \
    } \
    if (dither1 || dither0) \
    { \
        dio_start_clock(dither_wait); \
        dio_wait_clock(); \
    } \
    z = dio_read(scan_num); \
    if (z+delta_z > max_z) \
        max_z = z+delta_z; \
    if (z+delta_z < min_z) \
        min_z = z+delta_z; \
    if (scan_dither0!=dither0) \
    { \
        dio_dither(0,scan_dither0); \
    } \
    if (scan_dither1!=dither1) \
    { \
        dio_dither(1,scan_dither1); \
    } \
    if (scan_feedback!=read_feedback) \
    { \
        dio_feedback(scan_feedback); \
        dio_start_clock(inter_step_delay); \
        dio_wait_clock(); \
    }
    
                    
#define Z_CALC_MINMAX() if (SD->crash_protection==CRASH_MINMAX_Z) \
                { \
                    if (k==0) \
                    { \
                        if (j==0) \
                        { \
                            this_line_min=this_line_max=last_line_min= \
                                last_line_max=z+delta_z; \
                        } \
                        else \
                        { \
                            last_line_max=this_line_max; \
                            last_line_min=this_line_min; \
                            this_line_min=this_line_max=z+delta_z; \
                        } \
                    } \
                    else \
                    { \
                        if (z+delta_z>this_line_max) this_line_max=z+delta_z; \
                        if (z+delta_z<this_line_min) this_line_min=z+delta_z; \
                    } \
                    temp_min=min(this_line_min,last_line_min); \
                    temp_max=max(this_line_max,last_line_max); \
                    temp_range=temp_max-temp_min; \
                    if (temp_range>=(IN_MAX-low_limit)) \
                    { \
                        low_target=DEFAULT_Z_OFFSET_MOVE*low_limit; \
                        high_target=IN_MAX-(unsigned int) ((float)IN_MAX* \
                            SD->z_limit_percent*DEFAULT_Z_OFFSET_MOVE/100); \
                    } \
                    else \
                    { \
                        high_target=low_target=(IN_MAX-temp_range)/2+ \
                            z+delta_z-temp_min; \
                        if (high_target>high_limit || high_target<low_limit) \
                        { \
                            low_target=DEFAULT_Z_OFFSET_MOVE*low_limit; \
                            high_target=IN_MAX-(unsigned int) ((float)IN_MAX* \
                                SD->z_limit_percent*DEFAULT_Z_OFFSET_MOVE/100); \
                        } \
                    } \
                } 
                
#define Z_CRASH_PROTECT()    if (z>high_limit || z<low_limit) \
                { \
                    if (SD->crash_protection==CRASH_STOP) \
                    { \
                        stop_now=Z_CRASHED; \
                        goto STOP_NOW; \
                    } \
                    else  if (SD->crash_protection==CRASH_AUTO_Z || \
                             SD->crash_protection==CRASH_MINMAX_Z) \
                    { \
                        old_scan_z=z; \
                        read_ch_now=zi_ch; \
                        if (z>high_limit) \
                        { \
                            if (!auto_z_below(high_target)) \
                            { \
                                stop_now=Z_CRASHED; \
                                goto STOP_NOW; \
                            } \
                        } \
                        else \
                        { \
                            if (!auto_z_above(low_target)) \
                            { \
                                stop_now=Z_CRASHED; \
                                goto STOP_NOW; \
                            } \
                        } \
                        z=dio_read(z_scan_num); \
                        delta_z+=old_scan_z-z; \
                    } \
                }

#define Z_CRASH_PROTECT_BACK()  if (z>high_limit || z<low_limit) \
                        { \
                            if (SD->crash_protection==CRASH_STOP) \
                            { \
                                stop_now=Z_CRASHED; \
                                goto STOP_NOW; \
                            } \
                            else if (SD->crash_protection==CRASH_AUTO_Z || \
                                     SD->crash_protection==CRASH_MINMAX_Z) \
                            { \
                                dio_start_clock(SD->step_delay); \
                                dio_wait_clock(); \
                                if (read_ch_now!=zi_ch) \
                                { \
                                    read_ch_now=zi_ch; \
                                    dio_in_ch(read_ch_now); \
                                } \
                                old_scan_z=dio_read(z_scan_num); \
                                if (z>high_limit) \
                                { \
                                    if (!auto_z_below(high_target)) \
                                    { \
                                        stop_now=Z_CRASHED; \
                                        goto STOP_NOW; \
                                    } \
                                } \
                                else \
                                { \
                                    if (!auto_z_above(low_target)) \
                                    { \
                                        stop_now=Z_CRASHED; \
                                        goto STOP_NOW; \
                                    } \
                                } \
                                z=dio_read(z_scan_num); \
                                delta_z+=old_scan_z-z; \
                            } \
                        } \

                    
#define DO_DIGITAL_FEEDBACK()                     if (SD->digital_feedback) \
                    { \
                        if (digital_abort) stop_now=!dio_digital_feedback(Imin,Imax,SD->digital_feedback_max,SD->digital_feedback_reread); \
                        else dio_digital_feedback(Imin,Imax,SD->digital_feedback_max,SD->digital_feedback_reread); \
                        read_ch_now=0; \
                    } 

