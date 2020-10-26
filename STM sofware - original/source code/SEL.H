/* These defines determine the meaning of the fFlags variable.  The low byte
 * is used for the various types of "boxes" to draw.  The high byte is
 * available for special commands.
 */

#define SEL_BOX    1             /* Draw a solid border around the rectangle  */
#define SEL_BLOCK  2             /* Draw a solid rectangle                    */

#define SEL_EXTEND 256           /* Extend the current pattern                */

#define SEL_TYPE    0x00FF       /* Mask out everything but the type flags    */
#define SEL_SPECIAL 0xFF00       /* Mask out everything but the special flags */

#define SEL_INVERTED 0x1
#define SEL_SECONDSEL 0x2
 
typedef struct SEL_POINTDEF
{
        int x;
        int y;
        float z;
        float t;
//        int image;
} SEL_POINT;

typedef struct SEL_REGIONDEF
{
        SEL_POINT *pt1;
        SEL_POINT *pt2;
        int flags;
        int image;
        int type;
} SEL_REGION;

void repaint_sel(HWND,SEL_REGION *);
void paint_line(HWND,SEL_REGION *);
void paint_rect(HWND,SEL_REGION *);
void repaint_sel_p1(HWND);
void repaint_sel_p2(HWND);
void repaint_sel_points(HWND);
int find_coords(SEL_POINT *,LPARAM);
void paint_cross(HWND,SEL_REGION *,HBITMAP);
void square_coords(SEL_POINT,SEL_POINT *,int);
void circle_coords(SEL_POINT,SEL_POINT *,int);
int pt_in_region(int,int,SEL_REGION *);
float avg_z(SEL_REGION *);
void fit_plane(int,SEL_REGION *);

