/*

  Defined functions to create a set of alignment instructions
  and to execute such sets.

  Restrictions:
  
	* horizonal borders can only depend on horizontal borders
	* vertical borders can only depend on vertical borders
	* the size of a window can only depend on aother window's size of it depends on another windows border
	* a window's hcenter cannot depend on anything if the windows left or right border depends on something
	* a window's vcenter cannot depend on anything if the windows top or bottom border depends on something


*/


#ifndef I_ATTACHED_WINDOWS
#define I_ATTACHED_WINDOWS

#include "windows.h"
#include <vector>

const int ATTB_LEFT    = 0x01;
const int ATTB_RIGHT   = 0x02;
const int ATTB_TOP     = 0x04;
const int ATTB_BOTTOM  = 0x08;
const int ATTB_HCENTER = 0x10;
const int ATTB_VCENTER = 0x20;
const int ATTB_FINALIZE= 0x80;

const int ATTB_LEFTRIGHT = ATTB_LEFT | ATTB_RIGHT;
const int ATTB_TOPBOTTOM = ATTB_TOP | ATTB_BOTTOM;
const int ATTB_CENTER = ATTB_HCENTER | ATTB_VCENTER;

const int ATTB_WIDTHRATIO = 0x40;
const int ATTB_HEIGHTRATIO = 0x80;
const int ATTB_WIDTHHEIGHTRATIO = ATTB_WIDTHRATIO | ATTB_HEIGHTRATIO;

typedef struct
{
	int		border;
	HWND	hWndAttachedTo;
	int		target_border;
	int		distance;
} ATTACHED_BORDER;

typedef struct 
{
	HWND	hWnd;
	int		flags;
	float	width_ratio;
	float	height_ratio;
	HWND	hWndWidth;
	HWND	hWndHeight;
	std::vector<ATTACHED_BORDER> attached_borders;
} ATTACHED_WINDOW;

/* vector of alignment instructions */
typedef std::vector<ATTACHED_WINDOW> ATTACHED_WINDOWS;

/* add one alignment instruction to such a vector */
void AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, int target_border, int distance, ATTACHED_WINDOWS& atws);
void AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, int distance, ATTACHED_WINDOWS& atws);
void AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, ATTACHED_WINDOWS& atws);
void AttachWindow(HWND hWnd, HWND hWndAttachTo, int flags, float width_ratio, float height_ratio, ATTACHED_WINDOWS& atws);

/* execute alignment instructions */
void ReorderWindows(ATTACHED_WINDOWS& atws, int& bRedraw, int& redo, HWND hParent);

#endif