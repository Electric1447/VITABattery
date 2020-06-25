#include <stdio.h>
#include <taihen.h>
#include <vitasdk.h>

#include "draw.h"


int colors[] = {WHITE, GREEN, RED, BLUE, CYAN, MAGENTA, YELLOW, ORANGE, PURPLE};

static SceUID g_hooks[5];
int showMenu = 0;
int posIndex = 0, colorIndex = 0;

static uint32_t old_buttons, pressed_buttons;

static tai_hook_ref_t ref_hook0, ref_hook1, ref_hook2, ref_hook3, ref_hook4;


int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	
	drawSetFrameBuf(pParam);
	
	if (showMenu == 1) {
		int percent = scePowerGetBatteryLifePercent();
		drawSetColour(colors[colorIndex], BLACK);

		drawStringf(((posIndex <= 1) ? ((percent == 100) ? 880 : 896) : 0), ((posIndex % 2 == 0) ? 0 : 528), "%d %%", percent);
	}
	else if (showMenu == 2) {
		int batteryLifeTime = scePowerGetBatteryLifeTime();
		drawSetColour(colors[colorIndex], BLACK);
	
		if (batteryLifeTime >= 0)
			drawStringf(((posIndex <= 1) ? 848 : 0), ((posIndex % 2 == 0) ? 0 : 528), "%02ih %02im", batteryLifeTime / 60, batteryLifeTime - (batteryLifeTime / 60 * 60));
	}
	else if (showMenu == 3) {
		int temp = scePowerGetBatteryTemp();
		drawSetColour(colors[colorIndex], BLACK);
		
		if (scePowerGetBatteryTemp() > 0)
			drawStringf(((posIndex <= 1) ? 800 : 0), ((posIndex % 2 == 0) ? 0 : 528), "Temp: %02i C", temp / 100);
	}
	
	return TAI_CONTINUE(int, ref_hook0, pParam, sync);
}   

int checkButtons(int port, tai_hook_ref_t ref_hook, SceCtrlData *ctrl, int count) {
	int ret;

	if (ref_hook == 0) {
		ret = 1;
	} else {
		ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count);

		if (showMenu) {
			pressed_buttons = ctrl->buttons & ~old_buttons;

			if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_DOWN))
				showMenu = 0;
			else if ((ctrl->buttons & SCE_CTRL_START) && (pressed_buttons & SCE_CTRL_LTRIGGER))
				posIndex = (posIndex + 1) % 4;
			else if ((ctrl->buttons & SCE_CTRL_START) && (pressed_buttons & SCE_CTRL_RTRIGGER))
				colorIndex = (colorIndex + 1) % 9;

			old_buttons = ctrl->buttons;
		}
		else {
			if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_UP))
				showMenu = 1;
			else if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_RIGHT))
				showMenu = 2;
			else if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_LEFT))
				showMenu = 3;
		}
	}
  
	return ret;
}

static int keys_patched1(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, ref_hook1, ctrl, count);
}   

static int keys_patched2(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, ref_hook2, ctrl, count);
}   

static int keys_patched3(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, ref_hook3, ctrl, count);
}   

static int keys_patched4(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, ref_hook4, ctrl, count);
}   

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

	g_hooks[0] = taiHookFunctionImport(&ref_hook0, 
										TAI_MAIN_MODULE,
										TAI_ANY_LIBRARY,
										0x7A410B64, // sceDisplaySetFrameBuf
										sceDisplaySetFrameBuf_patched);

	g_hooks[1] = taiHookFunctionImport(&ref_hook1, 
										TAI_MAIN_MODULE,
										TAI_ANY_LIBRARY,
										0xA9C3CED6, // sceCtrlPeekBufferPositive
										keys_patched1);

	g_hooks[2] = taiHookFunctionImport(&ref_hook2, 
										TAI_MAIN_MODULE,
										TAI_ANY_LIBRARY,
										0x15F81E8C, // sceCtrlPeekBufferPositive2
										keys_patched2);

	g_hooks[3] = taiHookFunctionImport(&ref_hook3, 
										TAI_MAIN_MODULE,
										TAI_ANY_LIBRARY,
										0x67E7AB83, // sceCtrlReadBufferPositive
										keys_patched3);

	g_hooks[4] = taiHookFunctionImport(&ref_hook4, 
										TAI_MAIN_MODULE,
										TAI_ANY_LIBRARY,
										0xC4226A3E, // sceCtrlReadBufferPositive2
										keys_patched4);
										
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	
	// free hooks that didn't fail
	if (g_hooks[0] >= 0) taiHookRelease(g_hooks[0], ref_hook0);
	if (g_hooks[1] >= 0) taiHookRelease(g_hooks[1], ref_hook1);
	if (g_hooks[2] >= 0) taiHookRelease(g_hooks[2], ref_hook2);
	if (g_hooks[3] >= 0) taiHookRelease(g_hooks[3], ref_hook3);
	if (g_hooks[4] >= 0) taiHookRelease(g_hooks[4], ref_hook4);
	
	return SCE_KERNEL_STOP_SUCCESS;
}