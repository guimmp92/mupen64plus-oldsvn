/**
 * Mupen64 - plugin.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include <dlfcn.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "winlnxdefs.h"
#include "plugin.h"
#include "main.h"
#include "util.h"
#include "rom.h"
#include "../memory/memory.h"
#include "../r4300/interupt.h"
#include "../r4300/r4300.h"

CONTROL Controls[4];

static GFX_INFO gfx_info;
static AUDIO_INFO audio_info;
static CONTROL_INFO control_info;
static RSP_INFO rsp_info;

void (*getDllInfo)(PLUGIN_INFO *PluginInfo);
void (*dllConfig)(HWND hParent);
void (*dllTest)(HWND hParent);
void (*dllAbout)(HWND hParent);

/* dummy functions to prevent mupen from crashing if a plugin is missing */
static void dummy_void() {}
static BOOL dummy_initiateGFX(GFX_INFO Gfx_Info) { return TRUE; }
static BOOL dummy_initiateAudio(AUDIO_INFO Audio_Info) { return TRUE; }
static void dummy_initiateControllers(CONTROL_INFO Control_Info) {}
static void dummy_aiDacrateChanged(int SystemType) {}
static DWORD dummy_aiReadLength() { return 0; }
//static void dummy_aiUpdate(BOOL Wait) {}
static void dummy_controllerCommand(int Control, BYTE * Command) {}
static void dummy_getKeys(int Control, BUTTONS *Keys) {}
static void dummy_readController(int Control, BYTE *Command) {}
static void dummy_keyDown(WPARAM wParam, LPARAM lParam) {}
static void dummy_keyUp(WPARAM wParam, LPARAM lParam) {}
static void dummy_setConfigDir(char *configDir) {}
static unsigned int dummy;
static DWORD dummy_doRspCycles(DWORD Cycles) { return Cycles; };
static void dummy_initiateRSP(RSP_INFO Rsp_Info, DWORD * CycleCount) {};
static void dummy_fBRead(DWORD addr) {};
static void dummy_fBWrite(DWORD addr, DWORD size) {};
static void dummy_fBGetFrameBufferInfo(void *p) {};

void (*changeWindow)() = dummy_void;
void (*closeDLL_gfx)() = dummy_void;
BOOL (*initiateGFX)(GFX_INFO Gfx_Info) = dummy_initiateGFX;
void (*processDList)() = dummy_void;
void (*processRDPList)() = dummy_void;
void (*romClosed_gfx)() = dummy_void;
void (*romOpen_gfx)() = dummy_void;
void (*showCFB)() = dummy_void;
void (*updateScreen)() = dummy_void;
void (*viStatusChanged)() = dummy_void;
void (*viWidthChanged)() = dummy_void;
void (*readScreen)(void **dest, int *width, int *height) = 0;
void (*captureScreen)(char *dirpath) = 0;

void (*aiDacrateChanged)(int SystemType) = dummy_aiDacrateChanged;
void (*aiLenChanged)() = dummy_void;
DWORD (*aiReadLength)() = dummy_aiReadLength;
//void (*aiUpdate)(BOOL Wait) = dummy_aiUpdate;
void (*closeDLL_audio)() = dummy_void;
BOOL (*initiateAudio)(AUDIO_INFO Audio_Info) = dummy_initiateAudio;
void (*processAList)() = dummy_void;
void (*romClosed_audio)() = dummy_void;
void (*romOpen_audio)() = dummy_void;

void (*closeDLL_input)() = dummy_void;
void (*controllerCommand)(int Control, BYTE * Command) = dummy_controllerCommand;
void (*getKeys)(int Control, BUTTONS *Keys) = dummy_getKeys;
void (*initiateControllers)(CONTROL_INFO ControlInfo) = dummy_initiateControllers;
void (*readController)(int Control, BYTE *Command) = dummy_readController;
void (*romClosed_input)() = dummy_void;
void (*romOpen_input)() = dummy_void;
void (*keyDown)(WPARAM wParam, LPARAM lParam) = dummy_keyDown;
void (*keyUp)(WPARAM wParam, LPARAM lParam) = dummy_keyUp;
void (*setConfigDir)(char *configDir) = dummy_setConfigDir;

void (*closeDLL_RSP)() = dummy_void;
DWORD (*doRspCycles)(DWORD Cycles) = dummy_doRspCycles;
void (*initiateRSP)(RSP_INFO Rsp_Info, DWORD * CycleCount) = dummy_initiateRSP;
void (*romClosed_RSP)() = dummy_void;

void (*fBRead)(DWORD addr) = dummy_fBRead;
void (*fBWrite)(DWORD addr, DWORD size) = dummy_fBWrite;
void (*fBGetFrameBufferInfo)(void *p) = dummy_fBGetFrameBufferInfo;

list_t g_PluginList = NULL;

void plugin_delete_list(void)
{
    list_node_t *node;
    plugin *p;

    list_foreach(g_PluginList, node)
    {
        p = (plugin *)node->data;
        free(p->file_name);
        free(p->plugin_name);
        if (p->handle != NULL)
            dlclose(p->handle);
    }

    list_delete(&g_PluginList);
}

/* plugin_scan_file
 *  If given filename is a valid plugin, inserts it into the plugin list and returns TRUE.
 *   file_name - string containing either full path to plugin file or, if just the filename is given, it is assumed that the
 *               plugin is in the plugins/ subfolder of the installdir.
 *   plugin_type - if nonzero, plugin_scan_file will check that the given plugin's type matches plugin_type. If it doesn't, the
 *                 plugin will not be added to the list.
 */
int plugin_scan_file(const char *file_name, WORD plugin_type)
{
    PLUGIN_INFO pluginInfo;
    void *handle;
    plugin *p;
    char *bname = NULL;
    char filepath[PATH_MAX];

    if(strstr(file_name, "/"))
        realpath(file_name, filepath);
    else
        strncpy(filepath, file_name, PATH_MAX);

    // if this is not an absolute path, assume plugin file is in install dir
    if(filepath[0] != '/')
    {
        bname = strdup(filepath);
        basename(bname);
        snprintf(filepath, PATH_MAX, "%splugins/%s", get_installpath(), bname);
    }

    handle = dlopen(filepath, RTLD_NOW);
    if(handle)
    {
        getDllInfo = dlsym(handle, "GetDllInfo");
        if(getDllInfo)
        {
            getDllInfo(&pluginInfo);

            if(plugin_type != 0 &&
               pluginInfo.Type != plugin_type)
            {
                printf("Plugin '%s' is the wrong type!\n", file_name);
                dlclose(handle);
                return FALSE;

            }
            else
                plugin_type = pluginInfo.Type;
        }
        else
        {
            printf("Plugin '%s' is an invalid plugin\n", file_name);
            dlclose(handle);
            return FALSE;
        }
    }
    else
    {
        printf("Couldn't load plugin '%s': %s\n", file_name, dlerror());
        return FALSE;
    }

    p = malloc(sizeof(plugin));
    p->type = plugin_type;
    p->handle = handle;
    if(bname)
        p->file_name = bname;
    else
        p->file_name = strdup(file_name);
    p->plugin_name = strdup(pluginInfo.Name);
    list_append(&g_PluginList, p);

    return TRUE;
}

/* plugin_scan_installdir
 *  Populates plugin list with any valid plugins found in the "plugins" folder
 *  of the install directory
 */
void plugin_scan_installdir(void)
{
    DIR *dir;
    char cwd[PATH_MAX];
    struct dirent *entry;

    strncpy(cwd, get_installpath(), PATH_MAX);
    strncat(cwd, "plugins", PATH_MAX - strlen(cwd));
    dir = opendir(cwd);

    if(dir == NULL)
    {
        perror(cwd);
        return;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name + strlen(entry->d_name) - 3, ".so") != 0)
          continue;
        
        plugin_scan_file(entry->d_name, 0);
    }

    closedir(dir);
}

/* plugin_set_configdir
 *  Sets config dir of all plugins that support the SetConfigDir API call to the given dir.
 */
void plugin_set_configdir(char *configdir)
{
    plugin *p = NULL;
    list_node_t *node;

    list_foreach(g_PluginList, node)
    {
        p = (plugin *)node->data;

        if(p->handle)
        {
            // if plugin provides ability to set a config dir, set it.
            setConfigDir = dlsym(p->handle, "SetConfigDir");
            if(setConfigDir)
                setConfigDir(configdir);
        }
    }
}

plugin *plugin_get_by_name(const char *name)
{
    plugin *p = NULL;
    list_node_t *node;

    list_foreach(g_PluginList, node)
    {
        p = (plugin *)node->data;
        if (!strcmp(p->plugin_name, name))
            return p;
    }

    return NULL;
}

char *plugin_filename_by_name(const char *name)
{
    plugin *p = plugin_get_by_name(name);

    if(p) return p->file_name;
    return NULL;
}

char *plugin_name_by_filename(const char *filename)
{
    plugin *p;
    list_node_t *node;
    char real_filename1[PATH_MAX], real_filename2[PATH_MAX];

    if (!realpath(filename, real_filename1))
        strcpy(real_filename1, filename);

    list_foreach(g_PluginList, node)
    {
        p = (plugin *)node->data;

        if (!realpath(p->file_name, real_filename2))
            strcpy(real_filename2, p->file_name);

        if (!strcmp(real_filename1, real_filename2))
            return p->plugin_name;
    }

    return NULL;
}

static void sucre()
{
}

void plugin_exec_config(const char *name)
{
    plugin *p = plugin_get_by_name(name);

    if(p && p->handle)
    {
        dllConfig = dlsym(p->handle, "DllConfig");
        if(dllConfig)
            dllConfig(0);
    }
}

void plugin_exec_test(const char *name)
{
    plugin *p = plugin_get_by_name(name);

    if(p && p->handle)
    {
        dllTest = dlsym(p->handle, "DllTest");
        if(dllTest)
            dllTest(0);
    }
}

void plugin_exec_about(const char *name)
{
    plugin *p = plugin_get_by_name(name);

    if(p && p->handle)
    {
        dllAbout = dlsym(p->handle, "DllAbout");
        if(dllAbout)
            dllAbout(0);
    }
}

void plugin_load_plugins(const char *gfx_name,
             const char *audio_name,
             const char *input_name,
             const char *RSP_name)
{
   int i;
   plugin *p;
   void *handle_gfx = NULL,
    *handle_audio = NULL,
    *handle_input = NULL,
    *handle_RSP = NULL;

   p = plugin_get_by_name(gfx_name);
   if(p) handle_gfx = p->handle;

   p = plugin_get_by_name(audio_name);
   if(p) handle_audio = p->handle;

   p = plugin_get_by_name(input_name);
   if(p) handle_input = p->handle;

   p = plugin_get_by_name(RSP_name);
   if(p) handle_RSP = p->handle;

   if (handle_gfx)
     {
    changeWindow = dlsym(handle_gfx, "ChangeWindow");
    closeDLL_gfx = dlsym(handle_gfx, "CloseDLL");
    dllAbout = dlsym(handle_gfx, "DllAbout");
    dllConfig = dlsym(handle_gfx, "DllConfig");
    dllTest = dlsym(handle_gfx, "DllTest");
    initiateGFX = dlsym(handle_gfx, "InitiateGFX");
    processDList = dlsym(handle_gfx, "ProcessDList");
    processRDPList = dlsym(handle_gfx, "ProcessRDPList");
    romClosed_gfx = dlsym(handle_gfx, "RomClosed");
    romOpen_gfx = dlsym(handle_gfx, "RomOpen");
    showCFB = dlsym(handle_gfx, "ShowCFB");
    updateScreen = dlsym(handle_gfx, "UpdateScreen");
    viStatusChanged = dlsym(handle_gfx, "ViStatusChanged");
    viWidthChanged = dlsym(handle_gfx, "ViWidthChanged");
    readScreen = dlsym(handle_gfx, "ReadScreen");
    captureScreen = dlsym(handle_gfx, "CaptureScreen");
    
    fBRead = dlsym(handle_gfx, "FBRead");
    fBWrite = dlsym(handle_gfx, "FBWrite");
    fBGetFrameBufferInfo = dlsym(handle_gfx, "FBGetFrameBufferInfo");

    if (changeWindow == NULL) changeWindow = dummy_void;
    if (closeDLL_gfx == NULL) closeDLL_gfx = dummy_void;
    if (initiateGFX == NULL) initiateGFX = dummy_initiateGFX;
    if (processDList == NULL) processDList = dummy_void;
    if (processRDPList == NULL) processRDPList = dummy_void;
    if (romClosed_gfx == NULL) romClosed_gfx = dummy_void;
    if (romOpen_gfx == NULL) romOpen_gfx = dummy_void;
    if (showCFB == NULL) showCFB = dummy_void;
    if (updateScreen == NULL) updateScreen = dummy_void;
    if (viStatusChanged == NULL) viStatusChanged = dummy_void;
    if (viWidthChanged == NULL) viWidthChanged = dummy_void;
    if (captureScreen == NULL) captureScreen = dummy_void;

    gfx_info.MemoryBswaped = TRUE;
    gfx_info.HEADER = rom;
    gfx_info.RDRAM = (BYTE*)rdram;
    gfx_info.DMEM = (BYTE*)SP_DMEM;
    gfx_info.IMEM = (BYTE*)SP_IMEM;
    gfx_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    gfx_info.DPC_START_REG = &(dpc_register.dpc_start);
    gfx_info.DPC_END_REG = &(dpc_register.dpc_end);
    gfx_info.DPC_CURRENT_REG = &(dpc_register.dpc_current);
    gfx_info.DPC_STATUS_REG = &(dpc_register.dpc_status);
    gfx_info.DPC_CLOCK_REG = &(dpc_register.dpc_clock);
    gfx_info.DPC_BUFBUSY_REG = &(dpc_register.dpc_bufbusy);
    gfx_info.DPC_PIPEBUSY_REG = &(dpc_register.dpc_pipebusy);
    gfx_info.DPC_TMEM_REG = &(dpc_register.dpc_tmem);
    gfx_info.VI_STATUS_REG = &(vi_register.vi_status);
    gfx_info.VI_ORIGIN_REG = &(vi_register.vi_origin);
    gfx_info.VI_WIDTH_REG = &(vi_register.vi_width);
    gfx_info.VI_INTR_REG = &(vi_register.vi_v_intr);
    gfx_info.VI_V_CURRENT_LINE_REG = &(vi_register.vi_current);
    gfx_info.VI_TIMING_REG = &(vi_register.vi_burst);
    gfx_info.VI_V_SYNC_REG = &(vi_register.vi_v_sync);
    gfx_info.VI_H_SYNC_REG = &(vi_register.vi_h_sync);
    gfx_info.VI_LEAP_REG = &(vi_register.vi_leap);
    gfx_info.VI_H_START_REG = &(vi_register.vi_h_start);
    gfx_info.VI_V_START_REG = &(vi_register.vi_v_start);
    gfx_info.VI_V_BURST_REG = &(vi_register.vi_v_burst);
    gfx_info.VI_X_SCALE_REG = &(vi_register.vi_x_scale);
    gfx_info.VI_Y_SCALE_REG = &(vi_register.vi_y_scale);
    gfx_info.CheckInterrupts = sucre;
    initiateGFX(gfx_info);
     }
   else
     {
    changeWindow = dummy_void;
    closeDLL_gfx = dummy_void;
    initiateGFX = dummy_initiateGFX;
    processDList = dummy_void;
    processRDPList = dummy_void;
    romClosed_gfx = dummy_void;
    romOpen_gfx = dummy_void;
    showCFB = dummy_void;
    updateScreen = dummy_void;
    viStatusChanged = dummy_void;
    viWidthChanged = dummy_void;
    readScreen = 0;
    captureScreen = dummy_void;
     }

   if (handle_audio)
     {
    closeDLL_audio = dlsym(handle_audio, "CloseDLL");
    aiDacrateChanged = dlsym(handle_audio, "AiDacrateChanged");
    aiLenChanged = dlsym(handle_audio, "AiLenChanged");
    aiReadLength = dlsym(handle_audio, "AiReadLength");
    //aiUpdate = dlsym(handle_audio, "AiUpdate");
    initiateAudio = dlsym(handle_audio, "InitiateAudio");
    processAList = dlsym(handle_audio, "ProcessAList");
    romClosed_audio = dlsym(handle_audio, "RomClosed");
    romOpen_audio = dlsym(handle_audio, "RomOpen");
    
    if (aiDacrateChanged == NULL) aiDacrateChanged = dummy_aiDacrateChanged;
    if (aiLenChanged == NULL) aiLenChanged = dummy_void;
    if (aiReadLength == NULL) aiReadLength = dummy_aiReadLength;
    //if (aiUpdate == NULL) aiUpdate = dummy_aiUpdate;
    if (closeDLL_audio == NULL) closeDLL_audio = dummy_void;
    if (initiateAudio == NULL) initiateAudio = dummy_initiateAudio;
    if (processAList == NULL) processAList = dummy_void;
    if (romClosed_audio == NULL) romClosed_audio = dummy_void;
    if (romOpen_audio == NULL) romOpen_audio = dummy_void;
    
    audio_info.MemoryBswaped = TRUE;
    audio_info.HEADER = rom;
    audio_info.RDRAM = (BYTE*)rdram;
    audio_info.DMEM = (BYTE*)SP_DMEM;
    audio_info.IMEM = (BYTE*)SP_IMEM;
    audio_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    audio_info.AI_DRAM_ADDR_REG = &(ai_register.ai_dram_addr);
    audio_info.AI_LEN_REG = &(ai_register.ai_len);
    audio_info.AI_CONTROL_REG = &(ai_register.ai_control);
    audio_info.AI_STATUS_REG = &dummy;
    audio_info.AI_DACRATE_REG = &(ai_register.ai_dacrate);
    audio_info.AI_BITRATE_REG = &(ai_register.ai_bitrate);
    audio_info.CheckInterrupts = sucre;
    initiateAudio(audio_info);
     }
   else
     {
    aiDacrateChanged = dummy_aiDacrateChanged;
    aiLenChanged = dummy_void;
    aiReadLength = dummy_aiReadLength;
    //aiUpdate = dummy_aiUpdate;
    closeDLL_audio = dummy_void;
    initiateAudio = dummy_initiateAudio;
    processAList = dummy_void;
    romClosed_audio = dummy_void;
    romOpen_audio = dummy_void;
     }
   
   if (handle_input)
     {
    closeDLL_input = dlsym(handle_input, "CloseDLL");
    controllerCommand = dlsym(handle_input, "ControllerCommand");
    getKeys = dlsym(handle_input, "GetKeys");
    initiateControllers = dlsym(handle_input, "InitiateControllers");
    readController = dlsym(handle_input, "ReadController");
    romClosed_input = dlsym(handle_input, "RomClosed");
    romOpen_input = dlsym(handle_input, "RomOpen");
    keyDown = dlsym(handle_input, "WM_KeyDown");
    keyUp = dlsym(handle_input, "WM_KeyUp");
    
    if (closeDLL_input == NULL) closeDLL_input = dummy_void;
    if (controllerCommand == NULL) controllerCommand = dummy_controllerCommand;
    if (getKeys == NULL) getKeys = dummy_getKeys;
    if (initiateControllers == NULL) initiateControllers = dummy_initiateControllers;
    if (readController == NULL) readController = dummy_readController;
    if (romClosed_input == NULL) romClosed_input = dummy_void;
    if (romOpen_input == NULL) romOpen_input = dummy_void;
    if (keyDown == NULL) keyDown = dummy_keyDown;
    if (keyUp == NULL) keyUp = dummy_keyUp;
    
    control_info.MemoryBswaped = TRUE;
    control_info.HEADER = rom;
    control_info.Controls = Controls;
    for (i=0; i<4; i++)
      {
         Controls[i].Present = FALSE;
         Controls[i].RawData = FALSE;
         Controls[i].Plugin = PLUGIN_NONE;
      }
    initiateControllers(control_info);
     }
   else
     {
    closeDLL_input = dummy_void;
    controllerCommand = dummy_controllerCommand;
    getKeys = dummy_getKeys;
    initiateControllers = dummy_initiateControllers;
    readController = dummy_readController;
    romClosed_input = dummy_void;
    romOpen_input = dummy_void;
    keyDown = dummy_keyDown;
    keyUp = dummy_keyUp;
     }
   
   if (handle_RSP)
     {
    closeDLL_RSP = dlsym(handle_RSP, "CloseDLL");
    doRspCycles = dlsym(handle_RSP, "DoRspCycles");
    initiateRSP = dlsym(handle_RSP, "InitiateRSP");
    romClosed_RSP = dlsym(handle_RSP, "RomClosed");
    
    if (closeDLL_RSP == NULL) closeDLL_RSP = dummy_void;
    if (doRspCycles == NULL) doRspCycles = dummy_doRspCycles;
    if (initiateRSP == NULL) initiateRSP = dummy_initiateRSP;
    if (romClosed_RSP == NULL) romClosed_RSP = dummy_void;
    
    rsp_info.MemoryBswaped = TRUE;
    rsp_info.RDRAM = (BYTE*)rdram;
    rsp_info.DMEM = (BYTE*)SP_DMEM;
    rsp_info.IMEM = (BYTE*)SP_IMEM;
    rsp_info.MI_INTR_REG = &MI_register.mi_intr_reg;
    rsp_info.SP_MEM_ADDR_REG = &sp_register.sp_mem_addr_reg;
    rsp_info.SP_DRAM_ADDR_REG = &sp_register.sp_dram_addr_reg;
    rsp_info.SP_RD_LEN_REG = &sp_register.sp_rd_len_reg;
    rsp_info.SP_WR_LEN_REG = &sp_register.sp_wr_len_reg;
    rsp_info.SP_STATUS_REG = &sp_register.sp_status_reg;
    rsp_info.SP_DMA_FULL_REG = &sp_register.sp_dma_full_reg;
    rsp_info.SP_DMA_BUSY_REG = &sp_register.sp_dma_busy_reg;
    rsp_info.SP_PC_REG = &rsp_register.rsp_pc;
    rsp_info.SP_SEMAPHORE_REG = &sp_register.sp_semaphore_reg;
    rsp_info.DPC_START_REG = &dpc_register.dpc_start;
    rsp_info.DPC_END_REG = &dpc_register.dpc_end;
    rsp_info.DPC_CURRENT_REG = &dpc_register.dpc_current;
    rsp_info.DPC_STATUS_REG = &dpc_register.dpc_status;
    rsp_info.DPC_CLOCK_REG = &dpc_register.dpc_clock;
    rsp_info.DPC_BUFBUSY_REG = &dpc_register.dpc_bufbusy;
    rsp_info.DPC_PIPEBUSY_REG = &dpc_register.dpc_pipebusy;
    rsp_info.DPC_TMEM_REG = &dpc_register.dpc_tmem;
    rsp_info.CheckInterrupts = sucre;
    rsp_info.ProcessDlistList = processDList;
    rsp_info.ProcessAlistList = processAList;
    rsp_info.ProcessRdpList = processRDPList;
    rsp_info.ShowCFB = showCFB;
    initiateRSP(rsp_info,(DWORD*)&i);
     }
   else
     {
    closeDLL_RSP = dummy_void;
    doRspCycles = dummy_doRspCycles;
    initiateRSP = dummy_initiateRSP;
    romClosed_RSP = dummy_void;
     }
}