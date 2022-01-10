#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMPlugin.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>



#define VERBOSE 0

#define PLUGIN_NAME         "XPThrottleFPS"
#define PLUGIN_SIG          "MATATATA.XPThrottleFPS"
#define PLUGIN_DESCRIPTION  "Deliberately throttle X-Plane's performance in order to save power and reduce fan noise on laptops."
#define PLUGIN_VERSION      "0.9"

#define SETTINGS_FILE "Output/preferences/XPThrottleFPS.prf"

#include <unistd.h> // for usleep


void sleep_microseconds(int micro)
{
    usleep(micro);
}

XPLMDataRef frame_rate_period = NULL;

XPLMCommandRef toggle_fps_limit = NULL;
XPLMCommandRef increase_target_fps = NULL;
XPLMCommandRef decrease_target_fps = NULL;

int fps_limit_enabled = 0;
int target_fps = 25;

int pause_microsecs = 0;
int delta_ms = 0;
float fps = 0.0f;
#if VERBOSE
float magenta[] = { 1.0f, 0, 1.0f };
#endif

float last_timer_elaped_time_sec = 0.0;


float loop_cb(float last_call, float last_loop, int count, void *ref);
int command_handler(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref);

int draw_cb(XPLMDrawingPhase phase, int before, void *ref);

void get_settings_filepath(char *path);

void enable();
void disable();

void readSettings();
void writeSettings();


float target_frame_period_microsecs(){
    return 1000.0*1000.0/target_fps;
}

/**
 * X-Plane 11 Plugin Entry Point.
 *
 * Called when a plugin is initially loaded into X-Plane 11. If 0 is returned,
 * the plugin will be unloaded immediately with no further calls to any of
 * its callbacks.
 */
PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
    /* SDK docs state buffers are at least 256 bytes. */
    sprintf(name, "%s (v%s)", PLUGIN_NAME, PLUGIN_VERSION);
    strcpy(sig, PLUGIN_SIG);
    strcpy(desc, PLUGIN_DESCRIPTION);
    
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
   
    frame_rate_period = XPLMFindDataRef("sim/time/framerate_period");
    
    toggle_fps_limit = XPLMCreateCommand("XPTHrottleFPS/Toggle",
                                         "Toggle plugin active/inactive");
    increase_target_fps = XPLMCreateCommand("XPTHrottleFPS/incr_target_fps",
                                         "Increase target fps");
    decrease_target_fps = XPLMCreateCommand("XPTHrottleFPS/decr_target_fps",
                                         "Decrease target fps");
    
    XPLMRegisterCommandHandler(toggle_fps_limit, command_handler,
                               0, NULL);
    XPLMRegisterCommandHandler(increase_target_fps, command_handler,
                               0, NULL);
    XPLMRegisterCommandHandler(decrease_target_fps, command_handler,
                               0, NULL);
    
    
    
    
    
    return 1;
}


/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be unloaded from X-Plane 11.
 */
PLUGIN_API void XPluginStop(void) {
    
    XPLMUnregisterCommandHandler(toggle_fps_limit, command_handler,
                                 0, NULL);
    XPLMUnregisterCommandHandler(increase_target_fps, command_handler,
                                 0, NULL);
    XPLMUnregisterCommandHandler(decrease_target_fps, command_handler,
                                 0, NULL);
    
  
    
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be enabled. Return 1 if the plugin
 * started successfully, otherwise 0.
 */
PLUGIN_API int XPluginEnable(void) {
    readSettings();
    XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
    XPLMRegisterFlightLoopCallback(loop_cb,0.2,NULL);
   
    return 1;
}




/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be disabled.
 */
PLUGIN_API void XPluginDisable(void) {
    XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
    XPLMUnregisterFlightLoopCallback(loop_cb, NULL);
}





/**
 * X-Plane 11 Plugin Callback
 *
 * Called when a message is sent to the plugin by X-Plane 11 or another plugin.
 */
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
    
}



void readSettings() {
    FILE *f = fopen(SETTINGS_FILE,"r");
    if(f){
        fscanf(f, "%i,%i", &target_fps,&fps_limit_enabled);
        if(target_fps < 5)
            target_fps = 5;
        fclose(f);
    }
    else {
        char buf[1024];
        sprintf(buf, "XPThrottlePFS cannot read %s\n",SETTINGS_FILE);
        XPLMDebugString(buf);
    }
}

void writeSettings() {
    FILE * f = fopen (SETTINGS_FILE, "w+");
    if(f){
        fprintf(f, "%i,%i\n",target_fps,fps_limit_enabled);
        fclose(f);
    }
    else {
        char buf[1024];
        sprintf(buf, "XPThrottlePFS cannot write %s\n",SETTINGS_FILE);
        XPLMDebugString(buf);
    }
}


void enable(){
    pause_microsecs = 0;
    fps_limit_enabled = 1;
}


void disable(){
    fps_limit_enabled = 0;
    pause_microsecs = 0;
}


int toggle_fps_limit_cb() {
    readSettings();
    
    if (fps_limit_enabled) {
        disable();
    } else {
        enable();
    }
    return 1;
}

int command_handler(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
   
    if(phase != xplm_CommandBegin)
        return 1;
    
    if(cmd == toggle_fps_limit){
         toggle_fps_limit_cb();
    }
    
    if(cmd == increase_target_fps){
        target_fps += 1;
    }
    else if(cmd == decrease_target_fps){
        target_fps -= 1;
    }
    
    if(target_fps < 5)
        target_fps = 5;
    
    writeSettings();
    
    return 1;
}

void compute_limit(float last_call,float last_loop){
    
    float period = XPLMGetDataf(frame_rate_period);
    
    fps = 1.0f/period;
    
    float delta = fabs(fps - target_fps);
    delta_ms = 1000*delta*last_call;
    
    if(target_fps < fps){
        pause_microsecs += delta_ms;
    }
    else if(target_fps > fps){
        pause_microsecs -= delta_ms;
    }
    
    if(pause_microsecs < 0){
        pause_microsecs = 0;
    }
    
    float target_frame_rate_period = target_frame_period_microsecs();
    if(pause_microsecs >= target_frame_rate_period){
        pause_microsecs = target_frame_rate_period;
    }
    

    
}



void do_limit_fps(){
    if(fps_limit_enabled){
        if(pause_microsecs>0) {
            sleep_microseconds(pause_microsecs);
        }
    }
}


int draw_cb(XPLMDrawingPhase phase, int before, void *ref) {
    if(!fps_limit_enabled)
        return 1;
    
    if(pause_microsecs<=0)
        return 1;
    
    do_limit_fps();
    
    
    
#if VERBOSE
    static char buf[1000];
    sprintf(buf,"target_fps=%i, µsec=%d (%i delta), current fps=%f",target_fps,pause_microsecs,delta_ms,fps);
    XPLMDrawString(magenta, 20, 15, buf,
                   NULL, xplmFont_Proportional);
#endif
    return 1;
}






float loop_cb(float last_call, float last_loop, int count, void *ref) {
    
    if(!fps_limit_enabled)
        return 1.0;
    compute_limit(last_call,last_loop);
    
    return -1.0f;
}



