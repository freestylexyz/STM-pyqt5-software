#define GRID_MOVE_DELT 1
#define GRID_MOVE_DDELT 10

#define GRID_INIT_DIST 1
#define GRID_ANGLE_MIN (-90)
#define GRID_ANGLE_MAX 90
#define GRID_ANGLE_DELT 0.01	// Mod. #8, X.Cao, 2004-06-17, Higher grid resolution, was 0.1
#define GRID_ANGLE_DDELT 1
#define GRID_YINT_MIN 0
#define GRID_YINT_DELT 0.01		// Mod. #8, X.Cao, 2004-06-17, Higher grid resolution, was 0.1
#define GRID_YINT_DDELT 1
#define GRID_DIST_MIN 0
#define GRID_DIST_DELT 0.01		// Mod. #8, X.Cao, 2004-06-17, Higher grid resolution, was 0.1
#define GRID_DIST_DDELT 1





int grid_hidden(int);

void copy_grid(GRID *dest,GRID *source);

