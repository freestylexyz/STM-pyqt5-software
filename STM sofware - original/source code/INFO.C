#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include <string.h>
#include "file.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "pre_scan.h"
#include "scan.h"
#include "image.h"
#include "common.h"
#include "summary.h"

extern datadef *gendata[],**glob_data;
extern int current_image;
extern HANDLE hInst;
extern char string[];
extern double a_per_v,z_a_per_v;

BOOL FAR PASCAL InfoDlg(HWND,unsigned,WPARAM,LPARAM);
// Info dialog displays prominent data fields of scanned image datadef.
// Not sure that this will work with really old scans (say, version < 4).

BOOL FAR PASCAL InfoDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    unsigned int temp_z_scale = 1; // 0 <-> gain=.1,1<-> gain=1,2<-> gain=10
    datadef *ldat;
    static FARPROC lpfnDlgProc;

    switch(Message) {

	  case WM_INITDIALOG:
        ldat = gendata[current_image];

        if(ldat != NULL)
		{
		
			SetDlgItemText(hDlg,INFO_FILENAME,ldat->filename);
			sprintf(string,"%d",ldat->version);
			SetDlgItemText(hDlg,INFO_VERSION,string);

			// Tunneling Conditions
            sprintf(string,"%0.2lf",calc_i_set(ldat->i_setpoint,
                    ldat->i_set_range,ldat->amp_gain));
            SetDlgItemText(hDlg,INFO_CURRENT,string);
            sprintf(string,"%0.3f",dtov(ldat->bias,ldat->bias_range));
            SetDlgItemText(hDlg,INFO_BIAS,string);
            sprintf(string,"%0.2f",ldat->temperature);
            SetDlgItemText(hDlg,INFO_TEMPERATURE,string);
			
			// Scan
			sprintf(string,"%d",ldat->read_ch);
            SetDlgItemText(hDlg,INFO_INPUT_CH,string);
            sprintf(string,"%d",ldat->current_read_seq);
            SetDlgItemText(hDlg,INFO_SEQUENCE,string);
			if(ldat->scan_dither0)
				sprintf(string,"On.");
            else sprintf(string,"Off.");
            SetDlgItemText(hDlg,INFO_DITHER_0,string);
            if(ldat->scan_dither1)
				sprintf(string,"On.");
            else sprintf(string,"Off.");
            SetDlgItemText(hDlg,INFO_DITHER_1,string);
            if(ldat->scan_feedback)
				sprintf(string,"On.");
            else sprintf(string,"Off.");
            SetDlgItemText(hDlg,INFO_FEEDBACK,string);
            sprintf(string,"%0.1f",ldat->z_gain);
            SetDlgItemText(hDlg,INFO_Z_GAIN,string);
            if(ldat->z_gain == 0.1) temp_z_scale = 0;
            else if(ldat->z_gain == 1.0) temp_z_scale = 1;
            else if(ldat->z_gain == 10.0) temp_z_scale = 2;
            scan_freq_str(string,temp_z_scale,ldat->z_freq);
            SetDlgItemText(hDlg,INFO_Z_FREQ,string);
            if(ldat->x_first)
            {
				strcpy(string,"X First");
            }
            else 
            {
                strcpy(string,"Y First");
            }
            if(ldat->scan_dir == FORWARD_DIR)
				strcat(string,",Read Forward.");
            else if (ldat->scan_dir == BOTH_DIR1)
				strcat(string,",Read both directions.");
            else strcat(string,",Read Backward.");
            SetDlgItemText(hDlg,INFO_SCAN_ORDER,string);
            if(ldat->digital_feedback)
            {
				sprintf(string,"Yes. With tip spacing %0.2f",ldat->tip_spacing);
				SetDlgItemText(hDlg,INFO_DIGITAL_FEEDBACK,string);
            } else 
            {
				SetDlgItemText(hDlg,INFO_DIGITAL_FEEDBACK,"No");
            }
            switch(ldat->crash_protection)
            {
                case CRASH_DO_NOTHING:
                    strcpy(string,"Do Nothing.");
                    break;
                case CRASH_STOP:
                    strcpy(string,"Stop Scanning.");
                    break;
                case CRASH_AUTO_Z:
                    strcpy(string,"Auto Z.");
                    break;
                case CRASH_MINMAX_Z:
                    strcpy(string,"Min/Max Z.");
                    break;
            }
            SetDlgItemText(hDlg,INFO_Z_CRASH_PROT,string);
            if(ldat->overshoot)
            {
                if(ldat->overshoot == OVERSHOOT_FAST)
                {
					sprintf(string,"%0.1f %% in fast dir. ",ldat->overshoot_percent);
                }
                if(ldat->overshoot == OVERSHOOT_SLOW)
                {
					sprintf(string,"%0.1f %% slow dir. ",ldat->overshoot_percent);
                }
				SetDlgItemText(hDlg,INFO_OVERSHOT_BY,string);
            }
            else
            {
				SetDlgItemText(hDlg,INFO_OVERSHOT_BY,"None");
            }                                          

			// Delays
            sprintf(string,"%d",ldat->step_delay);
            SetDlgItemText(hDlg,INFO_STEP_DELAY,string);
            sprintf(string,"%d",ldat->inter_step_delay);
            SetDlgItemText(hDlg,INFO_INTER_STEP_DELAY,string);
            sprintf(string,"%d",ldat->inter_line_delay);
            SetDlgItemText(hDlg,INFO_INTER_LINE_DELAY,string);
            SetDlgItemText(hDlg,INFO_DOSED_WITH,ldat->dosed_type.ptr);

			// Scan size and direction
			SetDlgItemInt(hDlg,INFO_WIDTH_PIXELS,ldat->size,FALSE);
			SetDlgItemInt(hDlg,INFO_STEP_SIZE,ldat->step,FALSE);
			sprintf(string,"%0.2f",dtov_len(ldat->step * ldat->size,ldat->x_range) * 
						ldat->x_gain);
			SetDlgItemText(hDlg,INFO_WIDTH_VOLTS,string);
			sprintf(string,"%0.2f",dtov_len(ldat->step * ldat->size,ldat->x_range) *
						ldat->x_gain * A_PER_V);
			SetDlgItemText(hDlg,INFO_WIDTH_ANGS,string);
			sprintf(string,"%0.1f",ldat->x_gain);
			SetDlgItemText(hDlg,INFO_X_GAIN,string);
			sprintf(string,"%0.1f",ldat->y_gain);
			SetDlgItemText(hDlg,INFO_Y_GAIN,string);
			sprintf(string,"%0.2f",10 * dtov(ldat->x,ldat->x_offset_range));
			SetDlgItemText(hDlg,INFO_X_OFFSET,string);
			sprintf(string,"%0.2f",10 * dtov(ldat->y,ldat->y_offset_range));
			SetDlgItemText(hDlg,INFO_Y_OFFSET,string);
			sprintf(string,"%0.2f",dtov(ldat->z,2));
			SetDlgItemText(hDlg,INFO_Z_OFFSET,string);
			
			// Sample and Dosed, comment
			SetDlgItemText(hDlg,INFO_SAMPLE_TYPE,ldat->sample_type.ptr);
			SetDlgItemText(hDlg,INFO_DOSED_WITH,ldat->dosed_type.ptr);
			SetDlgItemText(hDlg,INFO_COMMENT,ldat->comment.ptr);

			// cleanup for older versions
			if(ldat->version < 3)
			{
				strcpy(string,"NA");
				SetDlgItemText(hDlg,INFO_Z_GAIN,string);
				SetDlgItemText(hDlg,INFO_Z_FREQ,string);
				SetDlgItemText(hDlg,INFO_BIAS,string);
				SetDlgItemText(hDlg,INFO_SCAN_ORDER,string);
				SetDlgItemText(hDlg,INFO_CURRENT,string);
				SetDlgItemText(hDlg,INFO_AMP_GAIN,string);
				SetDlgItemText(hDlg,INFO_DOSED_LANG,string);
				SetDlgItemText(hDlg,INFO_STEP_DELAY,string);
				SetDlgItemText(hDlg,INFO_INTER_STEP_DELAY,string);
				SetDlgItemText(hDlg,INFO_INTER_LINE_DELAY,string);
				SetDlgItemText(hDlg,INFO_DIGITAL_FEEDBACK,string);
				SetDlgItemText(hDlg,INFO_TIP_SPACING,string);
				SetDlgItemText(hDlg,INFO_SAMPLES,string);
				SetDlgItemText(hDlg,INFO_SCAN_MODE,string);
				SetDlgItemText(hDlg,INFO_Z_CRASH_PROT,string);
				SetDlgItemText(hDlg,INFO_OVERSHOT_BY,string);
				SetDlgItemText(hDlg,INFO_OVERSHOT_WAITS,string);
			}
			if(ldat->version < 4)
			{
				strcpy(string,"NA");
				SetDlgItemText(hDlg,INFO_TEMPERATURE,string);
			}
		}
        break;

      case WM_COMMAND:
        switch(LOWORD(wParam)) {
            case INFO_SEQ_SUMMARY:
                lpfnDlgProc = MakeProcInstance(SummaryDlg,hInst);
                glob_data = &(gendata[current_image]);
                DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);                  
                break;
            case INFO_EXIT:
                EndDialog(hDlg,TRUE);
                return(TRUE);
                break;
		}
        break;
	}
	return(FALSE);
}

