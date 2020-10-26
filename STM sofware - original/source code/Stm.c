#include <windows.h>
#include "mmsystem.h"
#pragma comment(lib,"winmm.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <i86.h>
#include <time.h>
#include <dos.h>
#include "stm.h"
#include "common.h"
#include "dio.h"
#include "data.h"
#include "file.h"
#include "scan.h"
#include "pal.h"
#include "clock.h"
#include "exportdr.h"
#include "hop.h"
#include "vibmap.h"
#include "latman.h"
#include "serial.h"

HANDLE          hInst;
HWND            InputWnd = NULL;
BOOL            InputOn = FALSE;
HBITMAP         hCross;
HBITMAP         hXcross;
LOGPALETTE     *pLogPal;
HPALETTE        hPal;
float          *smooth_cus_s,
               *smooth_cus_l,
               *smooth_s,
               *smooth_l;
char           *pal_image;
BITMAPINFO     *bitmapinfo;
char caption;

int feedback,x_range,y_range,z_range,x_offset_range,y_offset_range,
    z_offset_range,sample_bias_range,i_set_range;
int datapro = 0;
char firmware_vers_str[VERSION_SIZE + 1];	// + 1 for string termination

extern HWND PalWnd;
extern HWND SelWnd;
extern HWND TrackWnd;
extern HWND VscaleWnd;
extern HWND FourierWnd;
extern HWND GridWnd;
//extern HWND Prescan2Wnd;
extern HWND CountWnd;
extern HWND Wnd;
extern HWND AnlWnd;
extern HWND TraceWnd;
extern HWND LogpalWnd;

extern int image_caller;
extern unsigned char *scan_dacbox;
extern int z_offset_ch;
extern int scan_max_data;
extern int scan_current_data;
extern unsigned int dac_data[];
extern datadef *scan_defaults_data[];
extern datadef *manip_data;
extern unsigned int tip_gain;
extern unsigned int i_setpoint;
extern double i_set;
extern char *current_dir_stm,*current_dir_pal,*current_dir_mas,
    *current_dir_gif,*current_file,*current_dir_spc,*current_dir_dep,
    *current_dir_fig,*current_dir_cut,*current_dir_cnt,*current_dir_exp,
    *current_dir_stp, *current_dir_ps,*current_dir_hop,*current_dir_map;
extern char *initial_dir_stm,*initial_dir_pal,*initial_dir_mas,
    *initial_dir_gif,*initial_dir_spc,*initial_dir_dep,*initial_dir_grd,
    *initial_dir_fig,*initial_dir_cut,*initial_dir_cnt,*initial_dir_exp,
    *initial_dir_trc,*initial_dir_stp,*initial_dir_ps,*initial_dir_hop,
    *initial_dir_map;
extern char *current_file_stm,*current_file_pal,*current_file_mas,
    *current_file_gif,*current_file_spc,*current_file_dep,
    *current_file_fig,*current_file_cut,*current_file_exp,*current_file_cnt,
    *current_file_stp,*current_file_ps,*current_file_hop, *current_file_map;
extern char *current_file_grd,*current_dir_grd,*current_dir_cnt;
extern int file_stm_count,file_spc_count,file_mas_count,
    file_dep_count,file_cut_count;
extern char string[];
extern char exp_src_dir[CURRENT_DIR_MAX];
extern char exp_dest_dir[CURRENT_DIR_MAX];
extern unsigned int out1;
extern int      clock_used;
extern unsigned int input_ch;
extern datadef *data,
               **glob_data, *all_data[],
               *gendata[], *hop_data;
extern COUNT_DATA vm_count_data;
extern float one_input_time,dio_out_time;

extern BOOL FAR PASCAL DepDlg(HWND, unsigned, WPARAM, LPARAM);
//extern BOOL FAR PASCAL NewDepDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL InputDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL OutputDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL TipDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL CoarseDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL FineDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL ScanDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL PrescanDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL IOCtrlDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL EtestDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL OscillDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL MassDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL HeaterDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL ImageDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL ExportDirDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL SpecDlg(HWND, unsigned, WPARAM, LPARAM);

long FAR PASCAL WndProc(HWND, unsigned, WPARAM, LPARAM);
void set_smooth_vals();
void repaint_vers_info(HDC,char *);
void enable_io_menu(HWND,unsigned int);
void enable_datapro_menu(HWND,unsigned int);

void calibrate(HWND);

int PASCAL WinMain(HANDLE hInstance,HANDLE hPrevInstance,
                   LPSTR lpszCmdParam,int nCmdShow)
{
    int             i,
                    j;
    unsigned short int *tchar;
    HWND            hWnd;
    MSG             Message;
    WNDCLASS        WndClass;
//    HANDLE        hAccel;

    
	hInst = hInstance;
    
    // directory and file initialization
    current_dir_stm = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_ps = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_stp = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_pal = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_mas = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_spc = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_gif = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_dep = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_cut = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_grd = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_cnt = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_exp = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_fig = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_hop = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_dir_map = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_stm = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_ps = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_stp = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_dep = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_cut = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_pal = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_mas = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_spc = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_gif = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_grd = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_cnt = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_exp = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_fig = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_trc = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_hop = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    initial_dir_map = (char *)malloc(sizeof(char) * CURRENT_DIR_MAX);
    current_file_stm = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_ps = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_stp = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_pal = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_mas = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_spc = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_gif = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_dep = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_cut = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_grd = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_cnt = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_exp = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_fig = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_hop = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file_map = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    current_file = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);
    init_dirs();
    init_file(current_dir_stm,current_file_stm,"stm",&file_stm_count);
    strcpy(current_file_pal,"greys.pal");
    strcpy(current_file_ps,"figure.ps");
    init_file(current_dir_mas,current_file_mas,"mas",&file_mas_count);
    init_file(current_dir_spc,current_file_spc,"spc",&file_spc_count);
    init_file(current_dir_dep,current_file_dep,"dep",&file_dep_count);
    init_file(current_dir_cut,current_file_cut,"cut",&file_cut_count);
    strcpy(current_file_map,"*.vmp");
    strcpy(current_file_stp,"*.stp");
    strcpy(current_file_hop, "data.hop");
    current_file_fig[0] = '\0';
    current_file[0] = '\0';
    strcpy(exp_src_dir,EXP_SRCDIR_INI);
    strcpy(exp_dest_dir,EXP_DESTDIR_INI);

	// initialize main window
    if (!hPrevInstance)
    {
        WndClass.cbClsExtra = 0;
        WndClass.cbWndExtra = 0;
        WndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
        WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        WndClass.hIcon = LoadIcon(NULL, "END");
        WndClass.hInstance = hInstance;
        WndClass.lpfnWndProc = (LPVOID) WndProc;
        WndClass.lpszMenuName = "STMMenu";
        WndClass.lpszClassName = "STMClass";
        WndClass.style = CS_HREDRAW | CS_VREDRAW;

        RegisterClass(&WndClass);
    }

	hWnd = CreateWindow(
                        "STMClass", /* class name */
                        "STM",  /* caption */
                        WS_OVERLAPPEDWINDOW,    /* style */
                        12,     /* init. x pos */
                        50,     /* init. y pos */
                        600,   /* init. x size */
                        350,    /* init. y size */
                        NULL,   /* parent window */
                        NULL,   /* menu handle */
                        hInstance,  /* program handle */
                        NULL    /* create parms */
        );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
//    hAccel = LoadAccelerators(hInstance,"Accel");

    // initialization of graphics (palettes, bitmaps, etc)
    hCross = LoadBitmap(hInstance,"CROSS");
    hXcross = LoadBitmap(hInstance,"XCROSS");
    pLogPal = (NPLOGPALETTE)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * PALETTESIZE);
    pLogPal->palVersion = 0x300;
    scan_dacbox = (unsigned char *)malloc(SIZEOFMPAL);
    for (i = 0;i < 256;i++)
    {
        pLogPal->palPalEntry[i].peRed = i;
        pLogPal->palPalEntry[i].peGreen = 0;
        pLogPal->palPalEntry[i].peBlue = 0;
        pLogPal->palPalEntry[i].peFlags = PC_EXPLICIT;
    }
    for (i = 0; i < PALETTESIZE; i++)
    {
        pLogPal->palPalEntry[i].peRed = (BYTE) i;
        pLogPal->palPalEntry[i].peGreen = (BYTE) i;
        pLogPal->palPalEntry[i].peBlue = (BYTE) i;
        pLogPal->palPalEntry[i].peFlags = NULL;
    }
    set_Plasma_palette(scan_dacbox);
    pLogPal->palNumEntries = PALETTESIZE;
    hPal = CreatePalette(pLogPal);
//    smooth_cus_s = (float *) malloc(sizeof(float) * 3 * 3);
//    smooth_cus_l = (float *) malloc(sizeof(float) * 3 * 3);
    smooth_s = (float *)malloc(sizeof(float) * 4 * 3 * 3);
    smooth_l = (float *)malloc(sizeof(float) * 4 * 3 * 3);
    set_smooth_vals();
    bitmapinfo = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(PALETTEENTRY));
    bitmapinfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);
    bitmapinfo->bmiHeader.biWidth = 1;
    bitmapinfo->bmiHeader.biHeight = 1;
    bitmapinfo->bmiHeader.biPlanes = 1;
    bitmapinfo->bmiHeader.biBitCount = 8;
    bitmapinfo->bmiHeader.biCompression = BI_RGB;
    bitmapinfo->bmiHeader.biSizeImage = 0;
    bitmapinfo->bmiHeader.biXPelsPerMeter = 0;
    bitmapinfo->bmiHeader.biYPelsPerMeter = 0;
    bitmapinfo->bmiHeader.biClrUsed = 0;
    bitmapinfo->bmiHeader.biClrImportant = 0;
    tchar = (unsigned short int *)bitmapinfo->bmiColors;
    for (i = 0;i < PALETTESIZE;i++)
    {
        *(tchar + i) = i;
    }
    pal_image = (char *)malloc(PAL_IM_WIDTH * PAL_IM_HEIGHT * sizeof(char));
    for(i = 0;i < PAL_IM_HEIGHT;i++)
        for(j = 0;j < PAL_IM_WIDTH;j++)
            *(pal_image + j + i * PAL_IM_WIDTH) = (j * LOGPALUSED) / PAL_IM_WIDTH;

    calibrate(hWnd);

	// Take care of messages for modeless dialogs
	SetMessageQueue(64);
    while (GetMessage(&Message,NULL,NULL,NULL))
    {
        if (InputWnd == NULL || !IsDialogMessage(InputWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (PalWnd == NULL || !IsDialogMessage(PalWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }       
        else if (SelWnd == NULL || !IsDialogMessage(SelWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (TrackWnd == NULL || !IsDialogMessage(TrackWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (VscaleWnd == NULL || !IsDialogMessage(VscaleWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (FourierWnd == NULL || !IsDialogMessage(FourierWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
/*
        else if (Prescan2Wnd == NULL || !IsDialogMessage(Prescan2Wnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
*/
        else if (GridWnd == NULL || !IsDialogMessage(GridWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (CountWnd == NULL || !IsDialogMessage(CountWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (AnlWnd == NULL || !IsDialogMessage(AnlWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (TraceWnd == NULL || !IsDialogMessage(TraceWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else if (LogpalWnd == NULL || !IsDialogMessage(LogpalWnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
/*
        else if (Wnd == NULL || !IsDialogMessage(Wnd,&Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
*/
    }
    return (Message.wParam);
}

long FAR PASCAL WndProc(HWND hWnd, unsigned Message,
                                        WPARAM wParam, LPARAM lParam)
{
    static HANDLE   hInstance;
    static FARPROC  lpfnDlgProc;
    static FARPROC  InputDlgProc;
	HDC this_DC;
	PAINTSTRUCT ps;
    char filename[256];
    FILE *fp;
    int i;
    int crashed = 0;
	int serial_established = 0;

    switch (Message)
    {
    case WM_CREATE:
        
        alloc_data(&(all_data[0]),DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
        data = all_data[0];
        scan_max_data = 1;
        scan_current_data = 0;
        if(data->ptr == NULL) MessageBox(hWnd,"Test","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION); 
        for(i=0;i<SCAN_NUM_DEFAULTS;i++)             
        {
            scan_defaults_data[i] = NULL;
            alloc_data(&(scan_defaults_data[i]),DATATYPE_3D_PARAMETERS,0,GEN2D_NONE,GEN2D_NONE,1);
            strcpy(filename,SCAN_INI);
            sprintf(filename + strlen(filename),"%d.ini",i);
            glob_data = &scan_defaults_data[i];
            if(!load_image(filename))
            {
                set_scan_defaults(scan_defaults_data[i]);
            
            }
        }

		// Look for temporary scan settings file. If it exists, we crashed.
        if ((fp=fopen(SCAN_SETTINGS,"rb"))!=NULL)
        {
            fclose(fp);
            if (MessageBox(hWnd,"Previous program crashed! Restore its settings?",
                "Warning!",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
            {
                load_init(SCAN_SETTINGS);
                crashed = 1;
                //dio_set_registers();
                //retract(1);
				//retract_serial(TRUE);

			    // Look for temporary scan image file
                if ((fp=fopen(SCAN_IMAGE,"rb"))!=NULL)
                {
                    fclose(fp);
                    if (MessageBox(hWnd,"Restore Crashed Image?",
                        "Warning!",MB_YESNO | MB_ICONEXCLAMATION)==IDYES)
                    {
                        glob_data=&data;
                        load_image(SCAN_IMAGE);
                    }
                }
            }
            else
            {
                load_init(STM_INI);
            }
        }
        else load_init(STM_INI);
		i_set_range = get_range(i_setpoint_ch);
		i_set = calc_i_set(i_setpoint,i_set_range,tip_gain);

		// open serial port and try to contact dsp
		if(open_serial())
		{
			serial_settings();
			if(read_dsp_version(firmware_vers_str))
			{
				// We found the dsp! Enable i/o menu items & Reinit
				enable_io_menu(hWnd,MF_ENABLED);
				//firmware_vers_str[VERSION_SIZE + 1] = '\0'; //removed because it was crashing the dsp connection trying to write into a variable which was already been written by the com port
				if(!crashed) dio_init_serial();
				//dio_init();
				serial_established = 1;
			}
			else
			{
				MessageBox(hWnd,"Unable to find DSP device at COM1","Comm Error!",MB_OK);
				strcpy(firmware_vers_str,"Inaccessible");
				MessageBeep(MB_OK);
				sprintf(string,"Continue as Datapro?");
                //Mod. Shaowei
				if(MessageBox(hWnd,string, 
                    "Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
                enable_datapro_menu(hWnd,MF_ENABLED);
			}
		}
		else
		{
			MessageBox(hWnd,"Unable to establish serial connection on COM1",
						"Comm Error!",MB_OK);
			strcpy(firmware_vers_str,"Inaccessible");
			sprintf(string,"Continue as Datapro?");
			if(MessageBox(hWnd,string, 
                    "Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
                enable_datapro_menu(hWnd,MF_ENABLED);
		}


        if(crashed && serial_established)
        {
		  retract_serial(TRUE);
		  dio_init_serial();
		  // retract_serial(FALSE); Mod. Shaowei Tip will stay retracted if program crashed
        }
        feedback = ((dac_data[feedback_ch] & feedback_bit) && 1);
        x_range = get_range(x_ch);
        y_range = get_range(y_ch);
        z_range = get_range(zo_ch);
        x_offset_range = get_range(x_offset_ch);
        y_offset_range = get_range(y_offset_ch);
        z_offset_range = get_range(z_offset_ch);
        sample_bias_range = get_range(sample_bias_ch);
        i_set_range = get_range(i_setpoint_ch);
        hInstance = ((CREATESTRUCT far *) MK_FP32((void *) lParam))->hInstance;
        InputDlgProc = MakeProcInstance(InputDlg, hInstance);
        break;

	case WM_PAINT:
		this_DC = BeginPaint(hWnd, &ps);
		repaint_vers_info(this_DC,firmware_vers_str);
		EndPaint(hWnd, &ps);
		break;
    
	case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case TEST_SIX:
			//dep_clock_read_time();
            {
				float ratio;
				unsigned int initial_long = 2000;
				unsigned int final_long = 3000;
				unsigned int initial_short = 2000;
				unsigned int final_short = 2732;
				unsigned int current_long;
				unsigned int current_short;
				unsigned int delta_long;
				unsigned int delta_short;
				unsigned int bump;

				current_long = initial_long;
				current_short = initial_short;

				delta_long = abs(final_long - initial_long);
				delta_short = abs(final_short - initial_short);

				ratio = (float)delta_long/(float)delta_short;

				for(current_long = initial_long;current_long <= final_long;current_long++)
				{
				  bump = (unsigned int)((current_short - initial_short + 1) * ratio);
				  if((current_long - initial_long) == bump )
					current_short++;
				}

				sprintf(string,"long %u, short %u",current_long,current_short);
				mprintf(string);

			}
			break;
		case TEST_FIVE:
			//ad_test()
/*
			{

			  unsigned int long_1 = 0x800;
			  unsigned int long_2 = 0x810;
			  unsigned int short_1 = 0x800;
			  unsigned int short_2 = 0;
			  int read_every = 1;
			  float the_ratio = 16.0/3.0;
			  int move_delay = 1;
			  int read_delay = 2;
			  int l_ch = 2;
			  int s_ch = 3;
			  int we_are_reading = 1;
			  int long_upwards = 1;
			  int short_upwards = 1;
			  float my_buffer[1000];
			  int dummy;
			  int i;
			  datadef	*test_data;

			  for(i = 0;i < 1000;i++) my_buffer[i] = -1.0;
			  
			  alloc_data(&(test_data),DATATYPE_GEN2D,MAX,GEN2D_NONE,GEN2D_NONE,0);

			  test_data->sequence[0].type = READ_TYPE;
			  test_data->sequence[0].in_ch = 0;
			  test_data->sequence[0].num_reads = 1;
			  strcpy(test_data->sequence[0].step_type_string,"Read");
			  test_data->total_steps = 1;

			  readseq_setup(test_data);
 
			  latman_setup(long_1,long_2,short_1,read_every,the_ratio,
				                move_delay,read_delay);
			  latman(l_ch,s_ch,we_are_reading,long_upwards,short_upwards,
			         &long_2,&short_2,&dummy,my_buffer);

			  long_2 = long_2;
			  short_2 = short_2;
			  my_buffer[0] = my_buffer[0];
			  my_buffer[1] = my_buffer[1];
			  my_buffer[2] = my_buffer[2];
			  my_buffer[3] = my_buffer[3];

			}
*/
			break;
		case TEST_FOUR:
			{

			ramp_bias16_serial(30000,40000,1000);

			}
			break;
        case TEST_THREE:
			//input_switch_test();
			{
			  int i;
			  unsigned int last_output;
			  float test_data[256];
			  float blah = 0.0;
			  for(i = 0;i < 256;i++) test_data[i] = blah;

			  //scan_setup(CRASH_DO_NOTHING,5,100,100,128);

			  //scan_line(x_ch,SCANLINE_READOFF,SCANLINE_FORWARD,SCANLINE_FIRSTLINE,
				//		test_data,&last_output);

			  //scan_line(x_ch,SCANLINE_READOFF,SCANLINE_BACKWARD,SCANLINE_FIRSTLINE,
				//		test_data,&last_output);

			  scan_line(x_ch,SCANLINE_READON,SCANLINE_FORWARD,SCANLINE_FIRSTLINE,
						test_data,&last_output);

			  //scan_line(x_ch,SCANLINE_READON,SCANLINE_BACKWARD,SCANLINE_FIRSTLINE,
					//	test_data,&last_output);

			  i = i;
			  blah = test_data[0];
			  blah = test_data[3];
			  blah = test_data[255];
			  blah = test_data[40];
			  blah = test_data[39];
			  blah = test_data[10];

			}
			break;
        case TEST_TWO:
			//time_test();
			{
			/*	if((!dep_pulse_measure_i) & (!dep_pulse_measure_z)) flags+=0x01; //measure none
				else if(dep_pulse_measure_i) flags+=0x02; 
				if(dep_pulse_feedback) flags+=0x04;
				if(dep_pulse_bias_step) flags+=0x08;
				if(dep_pulse_duration_mode!=DEP_PULSE_DURATION_FIXED) flags+=0x10;
				if(bit16) flags += 0x20;
				if(dep_pulse_use_move_bias) flags+=0x40;
			*/

//			  dep_setup(move_bias,write_bias,move_i_setpoint,write_i_setpoint,dep_pulse_z_offset,
//		        dep_pulse_measure_i_wait,pts_number,move_time,dep_pulse_avg_stop_pts,(int)(dep_pulse_ignore_initial/ADC_CONV_TIME),
//		        (int)(dep_pulse_change_time/ADC_CONV_TIME),(int)(dep_pulse_after_time/ADC_CONV_TIME),dep_pulse_percent_change/100,dep_pulse_avg_pts,flags);

			  int i;
		      int data;
			  			  
			  dep_setup(0x800,0x800,0x800,0x800,0x000,
		        10,10,1000,10,10,
		        10,10,.1,10,0x5F);
			  dep_start();
			  
			  
			  for(i=0;i<10;i++) 
				  data = read_2_serial();	
               

/*				
			  unsigned int my_data = 0xf0f0;

			  highres_bias_serial(my_data);
			  my_data = 0xffff;
			  highres_bias_serial(my_data);
			  my_data = 0x0101;
			  highres_bias_serial(my_data);
			  my_data = 0x1111;
			  highres_bias_serial(my_data);
*/
			}

#ifdef OLD
			{
				unsigned int first = 0;
				unsigned int last = 100;
				unsigned int actual = 0;
				int delay_time = 1000;
				unsigned int data_peek = 0;

//				scan_setup(CRASH_MINMAX_Z,5 /* % */);
//				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				last = 200;
//				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				last = 500;
//				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				first = 500;
				last = 0;
//				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				scan_setup(CRASH_STOP,5 /* % */);
				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				data_peek = dac_data[z_offset_ch];
				data_peek = dac_data[3];
				scan_setup(CRASH_DO_NOTHING,5 /* % */);
				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				data_peek = dac_data[z_offset_ch];
				data_peek = dac_data[3];
				scan_setup(CRASH_AUTO_Z,5 /* % */);
				ramp_serial(3,first,last,5 /* delay */,1 /* do protection */);
				data_peek = dac_data[z_offset_ch];

				data_peek = dac_data[3];
				//actual = move_to_protect_serial(3,first,last,delay_time,CRASH_MINMAX_Z,2);
/*

				sprintf(string,"first %u last %u actual %u offset %u",
								first,last,dac_data[3],dac_data[z_offset_ch]);
				mprintf(string);
*/
			}
#endif
			break;
        case TEST_ONE:
			//moh_test();
			{
			  char		other_buffer[3] = {0xFF,0xFF,0xFF};
			  int		count = 0;
			  char		test_buffer[10000];
			  int		index = 0;
			  datadef	*test_data;
			  int		i,unread = 1;
			  int		failed = 0;
			  int		still_to_read = 88; 


			  for(i = 0;i < 10000;i++) test_buffer[i] = 0xFF;
			  alloc_data(&(test_data),DATATYPE_GEN2D,MAX,GEN2D_NONE,GEN2D_NONE,0);

			  test_data->sequence[1].type = READ_TYPE;
			  test_data->sequence[1].in_ch = 0;
			  test_data->sequence[1].num_reads = 2;
			  strcpy(test_data->sequence[1].step_type_string,"Read");

			  test_data->sequence[2].type = READ_TYPE;
			  test_data->sequence[2].in_ch = 1;
			  test_data->sequence[2].num_reads = 2;
			  strcpy(test_data->sequence[2].step_type_string,"Read");

			  test_data->sequence[3].type = READ_TYPE;
			  test_data->sequence[3].in_ch = 2;
			  test_data->sequence[3].num_reads = 2;
			  strcpy(test_data->sequence[3].step_type_string,"Read");

			  test_data->sequence[0].type = SERIAL_OUT_TYPE;
			  test_data->sequence[0].serial_out_char = DSPCMD_UPDATE;
			  strcpy(test_data->sequence[0].step_type_string,"Serial Out");
			  test_data->total_steps = 4;

			  readseq_setup(test_data);
			  spec_setup((unsigned int)0x1001,(unsigned int)0x1011,1,1,1);
			  //spec_setup(sp_scan_min,sp_scan_max,sp_bits_per_step,sp_move_delay,sp_speed);
			  spec_start(1/*fwd*/,0/*not bkd*/,10/*out ch*/);
			  
			  while(unread != 0)
              {
				  read_serial(&(test_buffer[index]),still_to_read,&unread);
				  index += (still_to_read - unread);
				  still_to_read = unread;
              }

			  /*
			  for(i = 0;i < 513;i++)
			  {
			    if(spec_continue()) count++;
			    else failed = 1;
			  }

              if(!failed) spec_finish(test_buffer,0);
			  */
			  test_buffer[0] = test_buffer[0];
			  other_buffer[0] = other_buffer[0];
			  test_buffer[1] = test_buffer[1];
			  test_buffer[2] = test_buffer[2];
			  test_buffer[12] = test_buffer[12];
				
/*
			  float scan_buffer[MAX];
              int scan_buffer_index = 0;
			  unsigned int pos = 0;
			  short m = 0;
			  for(m = 0; m < MAX; m++)
				  scan_buffer[m] = -1.0;

              readseq_setup(scan_defaults_data[2]);
			  scan_setup(scan_defaults_data[2]->crash_protection,
				  scan_defaults_data[2]->z_limit_percent,
				  scan_defaults_data[2]->inter_step_delay,
				  scan_defaults_data[2]->step_delay);
			  scan_line(x_ch,SCANLINE_READON,SCANLINE_BACKWARD,0,
				        &(scan_buffer[scan_buffer_index]),&pos);
			  pos = pos;
			  scan_buffer[1] = scan_buffer[1];
			  scan_buffer[1] = scan_buffer[1];
			  scan_buffer[1] = scan_buffer[1];
			  scan_buffer[1] = scan_buffer[1];
			  scan_buffer[1] = scan_buffer[1];
*/

/*
				char dummy_version[VERSION_SIZE + 1];

				for(i=0;i<10000;i++)
				{
					//serial_soft_delay(1);
					read_dsp_version(dummy_version);
				}
				MessageBox(hWnd,"20 SECS","Done!",MB_OK);
*/

/*
				int done=0;
				MSG Message2;

				while(!done)
				{
					ramp_serial(7,ZERO,MAX,100,0);
					ramp_serial(7,MAX,ZERO,100,0);
					PeekMessage(&Message2, hWnd, NULL, NULL, PM_REMOVE);
					if(LOWORD(Message2.wParam) == VK_ESCAPE)
					{
						sprintf(string, "Sqaure Wave terminated.");
						MessageBox(hWnd, string, "Warning", MB_ICONEXCLAMATION);
						done = 1;
					}
				}
*/
			}
            break;
        case FILE_LOAD:
            glob_data=&data;
            if (!data->saved)
            {
                if (MessageBox(hWnd, "Image not saved!\nLoad will overwrite image data...", "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
                    if (file_open(hWnd, hInstance,FTYPE_STM,0,current_file_stm))
                        data->saved = TRUE;
            } else
                file_open(hWnd, hInstance,FTYPE_STM,0,current_file_stm);
            break;
        case FILE_SAVE_AS:
            glob_data=&data;
            if (file_save_as(hWnd, hInstance,FTYPE_STM))
                inc_file(current_file_stm,&file_stm_count);
            break;
	case FILE_EXPORT:
	    {
	  		// export all *.spc files from one directory
			char    temp[CURRENT_DIR_MAX];
			char	fname[CURRENT_FILENAME_MAX];
//			unsigned int attr;

			datadef	*exp_data;
			int	count = 0;
			int	m = 0;
			int	result = 0;
			HANDLE current_file = NULL;
			WIN32_FIND_DATA lpffd;

//			struct find_t  spec_file;	OLD_COMPILER


			// dialog for user to choose src and dest directories
			lpfnDlgProc = MakeProcInstance( ExportDirDlg, hInstance);
			result = DialogBox(hInstance, "EXPORTDLG", hWnd, lpfnDlgProc);
			FreeProcInstance(lpfnDlgProc);
			if( result == EXPORT_CANCELLATION) break;

			// look for *.spc files in the source directory
			strcpy( temp, exp_src_dir);
			strcat( temp, "\\*.spc");

//			if(!_dos_findfirst( temp, _A_NORMAL, &spec_file))	OLD_COMPILER

			current_file = FindFirstFile(temp, &lpffd);
			if(current_file != 0xFFFFFFFF)
			{

				// prepare datadef to store spc data
				alloc_data( &exp_data, DATATYPE_GEN2D, 1, GEN2D_NONE, GEN2D_NONE, 0);
				glob_data = &exp_data;

				result = TRUE;
				while(result == TRUE)
				{
//					strcpy( fname, spec_file.name);	OLD_COMPILER
					strcpy(fname, lpffd.cFileName);

					// load spc data from *.spc file
					strcpy( temp, exp_src_dir);
					strcat( temp, "\\");
					strcat( temp, fname);
					if( !load_mas( temp))
					{
						MessageBox( hWnd, fname, "Error opening file.", MB_OK);
//						result = _dos_findnext( &spec_file);	OLD_COMPILER
						result = FindNextFile(current_file, &lpffd);
						break;
	 				}

					// now, export spc data to *.exp file
					strcpy( temp, exp_dest_dir);
					strcat( temp, "\\");
					m = 0;
					while( fname[m++] != '.');
					strcpy( &(fname[m]), "exp");
					strcat( temp, fname);
					// check that file doesn't alreay exist
//					if(!_dos_getfileattr(temp, &attr))	OLD_COMPILER
					if(GetFileAttributes(temp) != 0xFFFFFFFF)
					{
						MessageBox(hWnd, temp, "File already exists", MB_OK);
//						result = _dos_findnext(&spec_file);	OLD_COMPILER
						result = FindNextFile(current_file, &lpffd);
						continue;
					}
					save_exp(temp);
					count++;

					// look for the next *.spc file
//					result = _dos_findnext( &spec_file);	OLD_COMPILER
					result = FindNextFile(current_file, &lpffd);

				} // end of while(result == TRUE)
		  
				sprintf(temp, "%d files exported", count);
				MessageBox(hWnd, temp, "Export complete", MB_OK);
				free_data(&exp_data);

				FindClose(current_file);
			}

			else
			{
				sprintf(temp, "No spc files found in:\n %s", exp_src_dir);
				MessageBox(hWnd, temp, "Alas", MB_OK);
			}
	    }
	    break;

        case FILE_EXIT:
            remove(SCAN_IMAGE);
            remove(SCAN_SETTINGS);
            if (data->valid && !data->saved)
            {
                if (MessageBox(hWnd, "Image not saved!\nExit without saving?", "Warning", MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) == IDYES)
                    DestroyWindow(hWnd);
            } else
                DestroyWindow(hWnd);
            break;
        case IO_REINIT:
			dio_init_serial();
//            dio_init();
            break;
        case STM_ETEST:
            lpfnDlgProc = MakeProcInstance(EtestDlg, hInstance);
            DialogBox(hInstance, "ETESTDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;

        case IOCTRL:
            lpfnDlgProc = MakeProcInstance(IOCtrlDlg, hInstance);
            DialogBox(hInstance, "IODLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;

/*
        case OSCILLOSCOPE:
            lpfnDlgProc = MakeProcInstance(OscillDlg, hInstance);
            DialogBox(hInstance, "OSCILLDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
*/

/*
        case MASSPEC:
            lpfnDlgProc = MakeProcInstance(MassDlg, hInstance);
            DialogBox(hInstance, "MASSDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
*/

/*
        case HEATER:
            lpfnDlgProc = MakeProcInstance(HeaterDlg, hInstance);
            DialogBox(hInstance, "HEATERDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
*/
                     
        case INPUT:
            if (!InputOn)
            {
                InputWnd = CreateDialog(hInstance, "INPUTDLG", hWnd, InputDlgProc);
                InputOn = TRUE;
            }
            break;
        case OUTPUT:
            lpfnDlgProc = MakeProcInstance(OutputDlg, hInstance);
            DialogBox(hInstance, "OUTPUTDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
        case TIP:
			if(MessageBox(NULL,"Tip approach?","Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) ==
				IDCANCEL)
		  {
	        break;
		  }

            lpfnDlgProc = MakeProcInstance(TipDlg, hInstance);
            DialogBox(hInstance, "TIPDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;

/* 
        case COARSE:
            lpfnDlgProc = MakeProcInstance(CoarseDlg, hInstance);
            DialogBox(hInstance, "COARSEDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
*/

/*
        case FINE:
            lpfnDlgProc = MakeProcInstance(FineDlg, hInstance);
            DialogBox(hInstance, "FINEDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
*/

        case SCAN:
            lpfnDlgProc = MakeProcInstance(ScanDlg, hInstance);
            DialogBox(hInstance, "SCANDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
/*
        case MENU_SCANNING_DEPOSITION:
            lpfnDlgProc = MakeProcInstance(DepDlg, hInstance);
            DialogBox(hInst, "DEPDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
*/
		case MENU_DATA_MANIPULATION:
            image_caller=IMAGE_CALLER_STM;
            lpfnDlgProc = MakeProcInstance(ImageDlg, hInst);
            DialogBox(hInst, "IMAGEDLG", hWnd, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;

		case MENU_SPEC:
            lpfnDlgProc = MakeProcInstance(SpecDlg,hInst);
            DialogBox(hInst,"SPECDLG",hWnd,lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);                    
            break;
		case MENU_MODE_STM:
			datapro=0;
            enable_io_menu(hWnd,MF_ENABLED);
			SetWindowText(hWnd,"STM");
			break;
		case MENU_MODE_DATAPRO:
			enable_io_menu(hWnd,MF_DISABLED);
			enable_datapro_menu(hWnd,MF_ENABLED);
			break;
            /*
        case PRESCAN:
            lpfnDlgProc = MakeProcInstance(PrescanDlg,hInstance);
            DialogBox(hInstance,"PRESCANDLG",hWnd,lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
*/
        }
        break;

    case WM_DESTROY:
//        for(i=0;i<SCAN_NUM_DEFAULTS;i++) free_data(&scan_defaults_data[i]);
        remove(SCAN_IMAGE);
        remove(SCAN_SETTINGS);
        save_init(STM_INI);
        free_data(&data);
		free_data(&manip_data);
        DeleteObject(hPal);
        free(pLogPal);
//        free(smooth_cus_s);
//        free(smooth_cus_l);
        free(smooth_s);
        free(smooth_l);
        free(bitmapinfo);
        for (i = 0; i < 4; i++)
            if (gendata[i] != NULL)
                free_data(&gendata[i]);
	free_data( &hop_data);
        free(pal_image);
		close_serial();
        FreeProcInstance(InputDlgProc);
        PostQuitMessage(0);
        break;
    default:
        return (DefWindowProc(hWnd, Message, wParam, lParam));
    }

    return (0L);
}

void set_smooth_vals() {
    *(smooth_s + 3 * 3 * 0 + 3 * 0 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 0 + 1) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 0 + 2) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 1 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 1 + 1) = (float)0.6;
    *(smooth_s + 3 * 3 * 0 + 3 * 1 + 2) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 2 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 2 + 1) = (float)0.05;
    *(smooth_s + 3 * 3 * 0 + 3 * 2 + 2) = (float)0.05;
    *(smooth_l + 3 * 3 * 0 + 3 * 0 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 0 + 3 * 0 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 0 + 3 * 0 + 2) = (float)0.68;
    *(smooth_l + 3 * 3 * 0 + 3 * 1 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 0 + 3 * 1 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 0 + 3 * 1 + 2) = (float)0.02;
    *(smooth_l + 3 * 3 * 0 + 3 * 2 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 0 + 3 * 2 + 1) = (float)0.01;
    *(smooth_l + 3 * 3 * 0 + 3 * 2 + 2) = (float)0.01;
    *(smooth_s + 3 * 3 * 1 + 3 * 0 + 0) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 0 + 1) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 0 + 2) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 1 + 0) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 1 + 1) = (float)0.76;
    *(smooth_s + 3 * 3 * 1 + 3 * 1 + 2) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 2 + 0) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 2 + 1) = (float)0.03;
    *(smooth_s + 3 * 3 * 1 + 3 * 2 + 2) = (float)0.03;
    *(smooth_l + 3 * 3 * 1 + 3 * 0 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 1 + 3 * 0 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 1 + 3 * 0 + 2) = (float)0.68;
    *(smooth_l + 3 * 3 * 1 + 3 * 1 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 1 + 3 * 1 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 1 + 3 * 1 + 2) = (float)0.02;
    *(smooth_l + 3 * 3 * 1 + 3 * 2 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 1 + 3 * 2 + 1) = (float)0.01;
    *(smooth_l + 3 * 3 * 1 + 3 * 2 + 2) = (float)0.01;
    *(smooth_s + 3 * 3 * 2 + 3 * 0 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 0 + 1) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 0 + 2) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 1 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 1 + 1) = (float)0.6;
    *(smooth_s + 3 * 3 * 2 + 3 * 1 + 2) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 2 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 2 + 1) = (float)0.05;
    *(smooth_s + 3 * 3 * 2 + 3 * 2 + 2) = (float)0.05;
    *(smooth_l + 3 * 3 * 2 + 3 * 0 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 2 + 3 * 0 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 2 + 3 * 0 + 2) = (float)0.68;
    *(smooth_l + 3 * 3 * 2 + 3 * 1 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 2 + 3 * 1 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 2 + 3 * 1 + 2) = (float)0.02;
    *(smooth_l + 3 * 3 * 2 + 3 * 2 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 2 + 3 * 2 + 1) = (float)0.01;
    *(smooth_l + 3 * 3 * 2 + 3 * 2 + 2) = (float)0.01;
    *(smooth_s + 3 * 3 * 3 + 3 * 0 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 0 + 1) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 0 + 2) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 1 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 1 + 1) = (float)0.6;
    *(smooth_s + 3 * 3 * 3 + 3 * 1 + 2) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 2 + 0) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 2 + 1) = (float)0.05;
    *(smooth_s + 3 * 3 * 3 + 3 * 2 + 2) = (float)0.05;
    *(smooth_l + 3 * 3 * 3 + 3 * 0 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 3 + 3 * 0 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 3 + 3 * 0 + 2) = (float)0.68;
    *(smooth_l + 3 * 3 * 3 + 3 * 1 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 3 + 3 * 1 + 1) = (float)0.02;
    *(smooth_l + 3 * 3 * 3 + 3 * 1 + 2) = (float)0.02;
    *(smooth_l + 3 * 3 * 3 + 3 * 2 + 0) = (float)0.01;
    *(smooth_l + 3 * 3 * 3 + 3 * 2 + 1) = (float)0.01;
    *(smooth_l + 3 * 3 * 3 + 3 * 2 + 2) = (float)0.01;

}
    
void repaint_vers_info(HDC hDC,char *dsp_vers)
{
    if(!datapro)
	{sprintf(string, "Ho group STM Software");}
	else
	{sprintf(string, "Ho group Data Manipulation Software");}
    TextOut(hDC, 200, 20, string, strlen(string));
    sprintf(string, "STM2 Version 3.14 - Shaowei Li");
    TextOut(hDC, 200, 40, string, strlen(string));
    sprintf(string, "Last Modified 03/25/19");
    TextOut(hDC, 200, 60, string, strlen(string));
	   
	sprintf(string, "DSP Module Firmware");
	TextOut(hDC, 200, 100, string, strlen(string));
	sprintf(string, "Version: ");
	strcat(string, dsp_vers);
	TextOut(hDC, 200, 120, string, strlen(string));
}

void enable_io_menu(HWND hWnd,unsigned int state)
{
// state should be MF_GRAYED, MF_ENABLED, or MF_DISABLED

	HMENU hMenu = GetMenu(hWnd);

	EnableMenuItem(hMenu,OUTPUT,state);
	EnableMenuItem(hMenu,INPUT,state);
	EnableMenuItem(hMenu,IO_REINIT,state);
	EnableMenuItem(hMenu,STM_ETEST,state);
	EnableMenuItem(hMenu,IOCTRL,state);
	EnableMenuItem(hMenu,TIP,state);
	EnableMenuItem(hMenu,SCAN,state);
	EnableMenuItem(hMenu,MENU_SPEC,state);
}

void enable_datapro_menu(HWND hWnd,unsigned int state)
{
HMENU hMenu = GetMenu(hWnd);
    datapro = 1;
	EnableMenuItem(hMenu,SCAN,state);
	EnableMenuItem(hMenu,MENU_SPEC,state);
	SetWindowText(hWnd,"Datapro");
}

void calibrate(HWND hWnd)
{    
  one_input_time = 24.5; // both values approximate. in microsecs.
  dio_out_time = 2.0;	 
}


// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

#define READ_DEP_CLOCK()       {\
        outp(cntrcmd,0x84); /*latch cntr3 */ \
        lobyte=inp(cntr3); \
        hibyte=inp(cntr3)*256+lobyte; \
        if (cntr2value<hibyte) \
        { \
            dep_oscill_time_offset+=65535.0/2; \
        } \
        cntr2value=hibyte; \
        dep_oscill_time=((double)(65535-hibyte))*0.5+ \
            dep_oscill_time_offset; \
        }

void calibrate_old(HWND hWnd)
{
// calibrate(HWND) used with N.I. dio card. No longer used. 12/13/01 -mw
    float dep_oscill_time_offset,dep_oscill_time,start_time;
    unsigned int lobyte,hibyte,cntr2value,i;
#ifdef OLD
    char cal_string[500];
    int repeat=10000;
    unsigned int temp;
    HDC hDC;
#endif
    int win_time1,win_time2;
    outpw(cfg3,0x2400);
    outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
    outp(cntr3,0xFF);
    outp(cntr3,0xFF);
    cntr2value=65535;
    dep_oscill_time_offset=0;
    dep_oscill_time=0;
    dio_start_clock(1000000);
    dio_wait_clock(); /* paranoia */    

    win_time1=clock();
    READ_DEP_CLOCK();
    start_time=dep_oscill_time;
    for(i=0;i<1000;i++)
    {
        READ_DEP_CLOCK();
    }
    win_time2=clock();
    outpw(cfg3,0x2400);


    dep_read_clock_time=(dep_oscill_time-start_time)/1000.0;    
/*
    sprintf(cal_string,"start: %f after 1000: %f avg:%f windows time %d",start_time,
        dep_oscill_time,(dep_oscill_time-start_time)/1000.0,win_time2-win_time1);
    MessageBox(hWnd,cal_string,"Dep Clock Read Time",MB_OKCANCEL);
*/
    
#ifdef OLD
    repeat=100;
    dio_in_ch(0);
    _disable();
    outpw(cfg3,0x2400);
    outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
    outp(cntr3,0xFF);
    outp(cntr3,0xFF);
    cntr2value=65535;
    dep_oscill_time_offset=0;
    dep_oscill_time=0;
    READ_DEP_CLOCK();
    start_time=dep_oscill_time;
    for(i=0;i<repeat;i++)
        dio_in_data(&temp);
    READ_DEP_CLOCK();
    outpw(cfg3,0x2400);
    _enable();
    one_input_time=(dep_oscill_time-start_time)/(float)repeat;    
#endif
    
    one_input_time=24.5;
/*
     sprintf(cal_string,"One input time: %f",one_input_time);
    MessageBox(hWnd,cal_string,"One Input Time",MB_OKCANCEL);
*/
#ifdef OLD
    repeat=1000;
    _disable();
    outpw(cfg3,0x2400);
    outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
    outp(cntr3,0xFF);
    outp(cntr3,0xFF);
    cntr2value=65535;
    dep_oscill_time_offset=0;
    dep_oscill_time=0;
    READ_DEP_CLOCK();
    start_time=dep_oscill_time;
    for(i=0;i<repeat;i++)
    {   
        dio_in_ch(i%2);
    }
    READ_DEP_CLOCK();
    outpw(cfg3,0x2400);
    _enable();
    input_switch_time=(dep_oscill_time-start_time)/(float)repeat;    
#endif

    input_switch_time=2.7;

#ifdef OLD
    sprintf(cal_string,"Dep clock read time %f\nOne input time: %f\nInput switch time: %f\n",
        dep_read_clock_time,one_input_time,input_switch_time);
    hDC=GetDC(hWnd);
    TextOut(hDC,0,0,cal_string,strlen(cal_string));
   
    ReleaseDC(hWnd,hDC);
#endif
//    MessageBox(hWnd,string,"Auto Calibration",MB_OKCANCEL);
}

void dep_clock_read_time()
{
	float dep_oscill_time_offset,dep_oscill_time,start_time;
	unsigned int lobyte,hibyte,cntr2value,i;
	char dep_string[500];
        
	outpw(cfg3,0x2400);
	outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
	outp(cntr3,0xFF);
	outp(cntr3,0xFF);
	cntr2value=65535;
	dep_oscill_time_offset=0;
	dep_oscill_time=0;
	READ_DEP_CLOCK();
	start_time=dep_oscill_time;
	for(i=0;i<1000;i++)
	{
		READ_DEP_CLOCK();
	}
	outpw(cfg3,0x2400);

	sprintf(dep_string,"start: %f after 1000: %f avg:%f",start_time,
		dep_oscill_time,(dep_oscill_time-start_time)/1000.0);
	MessageBox(hWnd,dep_string,"Dep Clock Read Time",MB_OKCANCEL);
}

void ad_test()
{
	int time=20;
	int in_channel=3;
	int out_channel=0;
	int data;
            
	_disable();
	dio_out(out_channel,vtod(-10,get_range(out_channel)));
	dio_in_ch(in_channel);
	dio_start_clock(1000);
	dio_wait_clock();
	dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
	dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
	adc_delay();              /* Wait for conversion (approx. 15 microsec) */
	data = inpw(portc);          /* Read input */
	sprintf(string,"read %d, adc_delay",data);
	MessageBox(hWnd,string,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
	dio_out(out_channel,MAX);
	dio_in_ch(in_channel);
	dio_start_clock(1000);
	dio_wait_clock();
	dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
	dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
	adc_delay();              /* Wait for conversion (approx. 15 microsec) */
	data = inpw(portc);          /* Read input */
	sprintf(string,"read %d, adc_delay",data);
	MessageBox(hWnd,string,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
	while(time)
	{
		dio_out(out_channel,vtod(-10,get_range(out_channel)));
		dio_in_ch(in_channel);
		dio_start_clock(1000);
		dio_wait_clock();
		dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
		dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
		dio_start_clock(time);
		dio_wait_clock();
		data = inpw(portc);          /* Read input */
		sprintf(string,"read %d, %d delay",data,time);
		MessageBox(hWnd,string,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
		dio_out(out_channel,MAX);
		dio_in_ch(in_channel);
		dio_start_clock(1000);
		dio_wait_clock();
		dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
		dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
		dio_start_clock(time);
		dio_wait_clock();
		data = inpw(portc);          /* Read input */
		sprintf(string,"read %d, %d delay",data,time);
		MessageBox(hWnd,string,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
		time--;
		}
		_enable();
}

void one_input_test()
{
	int repeat=10000;
	int channel=0;
	int min,max,i;
	unsigned int temp;
            
	min=vtod(-1,get_range(channel));
	max=vtod(1,get_range(channel));
	MessageBox(hWnd,"Output 0 is pulses while input 0 is measured 10000 times","Test",MB_OK);
	dio_in_ch(0);
	_disable();
	dio_out(channel,min);
	for(i=0;i<repeat;i++)
		dio_in_data(&temp);
	dio_out(channel,max);
	_enable();
	dio_out(channel,dac_data[channel]);
}

void input_switch_test()
{
/*            int repeat=10000;
            int channel=0;
            int min,max,i;
            
            min=vtod(-1,get_range(channel));
            max=vtod(1,get_range(channel));
            MessageBox(hWnd,"Output 0 is pulses input is switched 10000 times","Test",MB_OK);
            _disable();
            dio_out(channel,min);
            for(i=0;i<repeat;i++)
            {   
                dio_in_ch(i%2);
            }
            dio_out(channel,max);
            _enable();
            dio_out(channel,dac_data[channel]);
*/
	int my_int = (IN_MAX * IN_MAX / 2) - 1;
	unsigned int my_uint = IN_MAX * IN_MAX / 2;
	if(my_int >= my_uint)
	{
		sprintf(string,"%d is greater or equal to %u",my_int,my_uint);
		MessageBox(hWnd,string,"Test",MB_OK);
	}
	else
	{
		sprintf(string,"%d is less than %u",my_int,my_uint);
		MessageBox(hWnd,string,"Test",MB_OK);
	}
		
}

void time_test()
{
	int time=15;
	int repeat=100;
	int channel=0;
	int pulses=10000;
	int min,max,i;
            
	min=vtod(-1,get_range(channel));
	max=vtod(1,get_range(channel));
	_disable();
	while(pulses)
	{
		dio_out(channel,min);
		for(i=0;i<repeat;i++)
		{
			dio_start_clock(time);
			dio_wait_clock();
		}
		dio_out(channel,max);
		for(i=0;i<repeat;i++)
		{
			dio_start_clock(time);
			dio_wait_clock();
		}
		pulses--;

	}
	dio_out(channel,dac_data[channel]);
	_enable();
}

void moh_test()
{
	int i,j;
	float min_z,max_z;
/* fill the data with 9 different "colors" */
	data->size=256;
	for(i=0;i<256;i++)
	for(j=0;j<256;j++) *(data->ptr+i*256+j)=IN_ZERO;
	for(i=0;i<86;i++)
	for(j=0;j<256;j++)
	{
		if (j<86) *(data->ptr+i*256+j)=IN_ZERO+1;
		else if (j<86*2) *(data->ptr+i*256+j)=IN_ZERO+2;
		else *(data->ptr+i*256+j)=IN_ZERO+3;
	}
	for(i=86;i<86*2;i++)
	for(j=0;j<256;j++)
	{
		if (j<86) *(data->ptr+i*256+j)=IN_ZERO+4;
		else if (j<86*2) *(data->ptr+i*256+j)=IN_ZERO+5;
		else *(data->ptr+i*256+j)=IN_ZERO+6;
	}
	for(i=172;i<256;i++)
	for(j=0;j<256;j++)
	{
		if (j<86) *(data->ptr+i*256+j)=IN_ZERO+7;
		else if (j<86*2) *(data->ptr+i*256+j)=IN_ZERO+8;
		else *(data->ptr+i*256+j)=IN_ZERO+9;
	}
	find_min_max(data,&min_z,&max_z);
	data->valid=1;
/* end of color fill */


#ifdef BLAH
	j=mdelay(1);
	sprintf(string,"%d",j);
	MessageBox(hWnd,string,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
                          
	j=mdelay(10);
	sprintf(string,"%d",j);
	MessageBox(hWnd,string,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
#endif                
                
#ifdef BLAH
	data->size=64;
	for(i=0;i<64;i++) for(j=0;j<64;j++) *(data->ptr+i*64+j)=i*j;
	data->valid=1;
	glob_data=&data;
	file_save_as(hWnd, hInstance,FTYPE_STM);
	file_open(hWnd, hInstance,FTYPE_STM,0,NULL);
	for(i=0;i<64;i++) for(j=0;j<64;j++) if (*(data->ptr+i*64+j)!=i*j)
		MessageBox(hWnd,"Bad Bad","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
      
#endif

#ifdef BLAH
	int             i,j;
	int             maxi,maxj,time;
	char teststr[256];

	sprintf(teststr,"%d",sizeof(float));        

	MessageBox(hWnd,teststr,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
 
	time = 20;
	maxj = 5000 / time;
	maxi = 1000;

	_disable();

	for (i = 0; i < maxi; i++)
	{
		dio_out(test_ch, 1024);
		for (j = 0; j < maxj; j++)
		{
			dio_start_clock(time);
			dio_wait_clock();
		}
		dio_out(test_ch, 3072);
		for (j = 0; j < maxj; j++)
		{
			dio_start_clock(time);
			dio_wait_clock();
		}
	}

	_enable();

	char           *image;
	int             i,j,iLoop;
	HDC             hDC;
	BITMAPINFO     *bitmapinfo2;
	unsigned short int *tchar;

//	      MessageBox(hWnd,"Test","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
	image = (char *) malloc(sizeof(char) * DATA_MAX * DATA_MAX);
	bitmapinfo2 = (BITMAPINFO *) malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(PALETTEENTRY));
	bitmapinfo2->bmiHeader.biSize = (DWORD) sizeof(BITMAPINFOHEADER);
	bitmapinfo2->bmiHeader.biWidth = DATA_MAX;
	bitmapinfo2->bmiHeader.biHeight = DATA_MAX;
	bitmapinfo2->bmiHeader.biPlanes = 1;
	bitmapinfo2->bmiHeader.biBitCount = 8;
	bitmapinfo2->bmiHeader.biCompression = BI_RGB;
	bitmapinfo2->bmiHeader.biSizeImage = DATA_MAX * DATA_MAX;
	bitmapinfo2->bmiHeader.biXPelsPerMeter = 0;
	bitmapinfo2->bmiHeader.biYPelsPerMeter = 0;
	bitmapinfo2->bmiHeader.biClrUsed = 0;
	bitmapinfo2->bmiHeader.biClrImportant = 0;

	tchar = (unsigned short int *) bitmapinfo2->bmiColors;
	for (i = 0; i < PALETTESIZE; i++)
	{
		*(tchar++) = i;
	}
	for (i = 0; i < DATA_MAX; i++)
		for (j = 0; j < DATA_MAX; j++)
			*(image + i * DATA_MAX + j) = j / 2 % LOGPALUSED;

	set_Plasma_palette();
	hDC = GetDC(GetFocus());
	SetMapMode(hDC, MM_TEXT);
	SelectPalette(hDC, hPal, 0);
	RealizePalette(hDC);
	SetDIBitsToDevice(hDC, 512, 0, 512, 512, 0, 0, 0, 512, image, bitmapinfo2, DIB_PAL_COLORS);
//            set_Plasma_palette();
	spindac(0, 1);
	SetDIBitsToDevice(hDC, 0, 0, 512, 512, 0, 0, 0, 512, image, bitmapinfo2, DIB_PAL_COLORS);

//            UpdateColors(hDC);
	ReleaseDC(hWnd, hDC);
	for (i = 0; i < 1000; i++)
		spindac(1, 1);
	free(image);
	free(bitmapinfo2);

#endif
}

#endif

void mfcoutput(HWND hWnd)
{
	FARPROC  my_lpfnDlgProc;
	my_lpfnDlgProc = MakeProcInstance(OutputDlg, hInst);
    DialogBox(hInst, "OUTPUTDLG", hWnd, my_lpfnDlgProc);
    FreeProcInstance(my_lpfnDlgProc);
}