#include <stdio.h>
#include <taihen.h>
#include <vitasdk.h>

#include "draw.h"


#define TIMER_SECOND 	1000000 // 1 second


int colors[] = {WHITE, GREEN, RED, BLUE, CYAN, MAGENTA, YELLOW, ORANGE, PURPLE};

int menuIndex = 0, posIndex = 0, colorIndex = 0;
uint64_t tick = 0, t_tick = 0;
int frames = 0, fps_data = 0;

static uint32_t old_buttons, pressed_buttons;

static SceUID tai_uid;
static tai_hook_ref_t hook;


void checkButtons() {
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
		
	if (menuIndex) {
		pressed_buttons = pad.buttons & ~old_buttons;

		if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_DOWN))
			menuIndex = 0;
		else if ((pad.buttons & SCE_CTRL_START) && (pressed_buttons & SCE_CTRL_LTRIGGER))
			posIndex = (posIndex + 1) % 4;
		else if ((pad.buttons & SCE_CTRL_START) && (pressed_buttons & SCE_CTRL_RTRIGGER))
			colorIndex = (colorIndex + 1) % 9;

		old_buttons = pad.buttons;
	}
	else {
		if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_UP))
			menuIndex = 1;
		else if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_RIGHT))
			menuIndex = 2;
		else if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_LEFT))
			menuIndex = 3;
		else if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_TRIANGLE))
			menuIndex = 4;
	}
}

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	
	checkButtons();
	
	updateFrameBuf(pParam);
	setColor(colors[colorIndex], BLACK);
	
	switch (menuIndex) {
		case 1: {
			int percent = scePowerGetBatteryLifePercent();
			
			drawStringF((1 - posIndex / 2) * (896 - 16 * (percent / 100)), (posIndex % 2) * 528, "%d %%", percent);
			break;
		}
		case 2: {
			int batteryLifeTime = scePowerGetBatteryLifeTime();
			
			if (batteryLifeTime >= 0)
				drawStringF((1 - posIndex / 2) * 848, (posIndex % 2) * 528, "%02ih %02im", batteryLifeTime / 60, batteryLifeTime - (batteryLifeTime / 60 * 60));
			break;
		}
		case 3: {
			int temp = scePowerGetBatteryTemp();
			
			if (temp > 0)
				drawStringF((1 - posIndex / 2) * 800, (posIndex % 2) * 528, "Temp: %02i C", temp / 100);
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
				drawStringF((1 - posIndex / 2) * 848, (posIndex % 2) * 528, "FPS: %2d", fps_data);
			}
			
			frames++;
			break;
	}
	
	return TAI_CONTINUE(int, hook, pParam, sync);
}   

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {

	tai_uid = taiHookFunctionImport(&hook, TAI_MAIN_MODULE, 0x4FAACD11, 0x7A410B64, sceDisplaySetFrameBuf_patched);
										
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	
	// free hooks that didn't fail	
	if (tai_uid >= 0) taiHookRelease(tai_uid, hook);
	
	return SCE_KERNEL_STOP_SUCCESS;
}