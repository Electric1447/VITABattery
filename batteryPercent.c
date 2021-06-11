#include <stdio.h>
#include <taihen.h>
#include <vitasdk.h>

#include "draw.h"


#define TIMER_SECOND 	1000000 // 1 second


int colors[] = {WHITE, GREEN, RED, BLUE, CYAN, MAGENTA, YELLOW, ORANGE, PURPLE};

int menuIndex = 0, posIndex = 0, colorIndex = 0;
static SceUInt64 tick = 0, t_tick = 0;
static SceInt frames = 0, fps_data = 0;

static uint32_t old_buttons, pressed_buttons;

static SceUID tai_uid[5];
static tai_hook_ref_t hook[5];


int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	
	drawSetFrameBuf(pParam);
	
	drawSetColour(colors[colorIndex], BLACK);
	
	switch (menuIndex) {
		case 1: {
			int percent = scePowerGetBatteryLifePercent();
			
			drawStringf(((posIndex <= 1) ? ((percent == 100) ? 880 : 896) : 0), ((posIndex % 2 == 0) ? 0 : 528), "%d %%", percent);
			break;
		}
		case 2: {
			int batteryLifeTime = scePowerGetBatteryLifeTime();
			
			if (batteryLifeTime >= 0)
				drawStringf(((posIndex <= 1) ? 848 : 0), ((posIndex % 2 == 0) ? 0 : 528), "%02ih %02im", batteryLifeTime / 60, batteryLifeTime - (batteryLifeTime / 60 * 60));
			break;
		}
		case 3: {
			int temp = scePowerGetBatteryTemp();
			
			if (scePowerGetBatteryTemp() > 0)
				drawStringf(((posIndex <= 1) ? 800 : 0), ((posIndex % 2 == 0) ? 0 : 528), "Temp: %02i C", temp / 100);
			break;
		}
		case 4:
			// This function is from Framecounter by Rinnegatamante.
			t_tick = sceKernelGetProcessTimeWide();
				
			if (tick == 0) {
				tick = t_tick;
			} else {
				if ((t_tick - tick) > TIMER_SECOND) {
					fps_data = frames;
					frames = 0;
					tick = t_tick;
				}
				drawStringf(((posIndex <= 1) ? 848 : 0), ((posIndex % 2 == 0) ? 0 : 528), "FPS: %2d", fps_data);
			}
			
			frames++;
			break;
	}
	
	return TAI_CONTINUE(int, hook[0], pParam, sync);
}   

int checkButtons(int port, tai_hook_ref_t ref_hook, SceCtrlData *ctrl, int count) {
	int ret;

	if (ref_hook == 0) {
		ret = 1;
	} else {
		ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count);

		if (menuIndex) {
			pressed_buttons = ctrl->buttons & ~old_buttons;

			if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_DOWN))
				menuIndex = 0;
			else if ((ctrl->buttons & SCE_CTRL_START) && (pressed_buttons & SCE_CTRL_LTRIGGER))
				posIndex = (posIndex + 1) % 4;
			else if ((ctrl->buttons & SCE_CTRL_START) && (pressed_buttons & SCE_CTRL_RTRIGGER))
				colorIndex = (colorIndex + 1) % 9;

			old_buttons = ctrl->buttons;
		}
		else {
			if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_UP))
				menuIndex = 1;
			else if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_RIGHT))
				menuIndex = 2;
			else if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_LEFT))
				menuIndex = 3;
			else if ((ctrl->buttons & SCE_CTRL_START) && (ctrl->buttons & SCE_CTRL_TRIANGLE))
				menuIndex = 4;
		}
	}
  
	return ret;
}

static int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, hook[1], ctrl, count);
}   

static int sceCtrlPeekBufferPositive2_patched(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, hook[2], ctrl, count);
}   

static int sceCtrlReadBufferPositive_patched(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, hook[3], ctrl, count);
}   

static int sceCtrlReadBufferPositive2_patched(int port, SceCtrlData *ctrl, int count) {
	return checkButtons(port, hook[4], ctrl, count);
}   

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {

	tai_uid[0] = taiHookFunctionImport(&hook[0], TAI_MAIN_MODULE, 0x4FAACD11, 0x7A410B64, sceDisplaySetFrameBuf_patched);
	tai_uid[1] = taiHookFunctionImport(&hook[1], TAI_MAIN_MODULE, 0xD197E3C7, 0xA9C3CED6, sceCtrlPeekBufferPositive_patched);
	tai_uid[2] = taiHookFunctionImport(&hook[2], TAI_MAIN_MODULE, 0xD197E3C7, 0x15F81E8C, sceCtrlPeekBufferPositive2_patched);
	tai_uid[3] = taiHookFunctionImport(&hook[3], TAI_MAIN_MODULE, 0xD197E3C7, 0x67E7AB83, sceCtrlReadBufferPositive_patched);
	tai_uid[4] = taiHookFunctionImport(&hook[4], TAI_MAIN_MODULE, 0xD197E3C7, 0xC4226A3E, sceCtrlReadBufferPositive2_patched);
										
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	
	// free hooks that didn't fail
	for (SceInt i = 0; i < 5; i++)
		if (tai_uid[i] >= 0) 
			taiHookRelease(tai_uid[i], hook[i]);
	
	return SCE_KERNEL_STOP_SUCCESS;
}