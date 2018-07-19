/*
 *  Copyright (c) 2012 - 2013 EMlyDinEsH(OSXLatitude). All rights reserved.
 *  Copyright (c) 2018 hieplpvip
 *
 *  Asus Fn keys Driver v1.0.0 by hieplpvip for macOS 10.13
 *
 *  Credits: EMlyDinEsH(OSXLatitude) for initial driver
 *
 *  AsusFnKeys.cpp
 *  
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <IOKit/hidsystem/ev_keymap.h>

#include "AsusFnKeys.h"

#define DEBUG_START 0

#if DEBUG_START
#define DEBUG_LOG(fmt, args...) IOLog(fmt, ## args)
#else
#define DEBUG_LOG(fmt, args...)
#endif

#ifdef DEBUG
/*
 * GUID parsing functions
 */
/**
 * wmi_parse_hexbyte - Convert a ASCII hex number to a byte
 * @src:  Pointer to at least 2 characters to convert.
 *
 * Convert a two character ASCII hex string to a number.
 *
 * Return:  0-255  Success, the byte was parsed correctly
 *          -1     Error, an invalid character was supplied
 */
int AsusFnKeys::wmi_parse_hexbyte(const UInt8 *src)
{
	unsigned int x; /* For correct wrapping */
	int h;
	
	/* high part */
	x = src[0];
	if (x - '0' <= '9' - '0') {
		h = x - '0';
	} else if (x - 'a' <= 'f' - 'a') {
		h = x - 'a' + 10;
	} else if (x - 'A' <= 'F' - 'A') {
		h = x - 'A' + 10;
	} else {
		return -1;
	}
	h <<= 4;
	
	/* low part */
	x = src[1];
	if (x - '0' <= '9' - '0')
		return h | (x - '0');
	if (x - 'a' <= 'f' - 'a')
		return h | (x - 'a' + 10);
	if (x - 'A' <= 'F' - 'A')
		return h | (x - 'A' + 10);
	return -1;
}

/**
 * wmi_swap_bytes - Rearrange GUID bytes to match GUID binary
 * @src:   Memory block holding binary GUID (16 bytes)
 * @dest:  Memory block to hold byte swapped binary GUID (16 bytes)
 *
 * Byte swap a binary GUID to match it's real GUID value
 */
void AsusFnKeys::wmi_swap_bytes(UInt8 *src, UInt8 *dest)
{
	int i;
	
	for (i = 0; i <= 3; i++)
		memcpy(dest + i, src + (3 - i), 1);
	
	for (i = 0; i <= 1; i++)
		memcpy(dest + 4 + i, src + (5 - i), 1);
	
	for (i = 0; i <= 1; i++)
		memcpy(dest + 6 + i, src + (7 - i), 1);
	
	memcpy(dest + 8, src + 8, 8);
}

/**
 * wmi_parse_guid - Convert GUID from ASCII to binary
 * @src:   36 char string of the form fa50ff2b-f2e8-45de-83fa-65417f2f49ba
 * @dest:  Memory block to hold binary GUID (16 bytes)
 *
 * N.B. The GUID need not be NULL terminated.
 *
 * Return:  'true'   @dest contains binary GUID
 *          'false'  @dest contents are undefined
 */
bool AsusFnKeys::wmi_parse_guid(const UInt8 *src, UInt8 *dest)
{
	static const int size[] = { 4, 2, 2, 2, 6 };
	int i, j, v;
	
	if (src[8]  != '-' || src[13] != '-' ||
		src[18] != '-' || src[23] != '-')
		return false;
	
	for (j = 0; j < 5; j++, src++) {
		for (i = 0; i < size[j]; i++, src += 2, *dest++ = v) {
			v = wmi_parse_hexbyte(src);
			if (v < 0)
				return false;
		}
	}
	
	return true;
}

/**
 * wmi_dump_wdg - dumps tables to dmesg
 * @src: guid_block *
 */
void AsusFnKeys::wmi_dump_wdg(struct guid_block *g)
{
	char guid_string[37];
	
	wmi_data2Str(g->guid, guid_string);
	DEBUG_LOG("%s:\n", guid_string);
	if (g->flags & ACPI_WMI_EVENT)
		DEBUG_LOG("\tnotify_value: %02X\n", g->notify_id);
	else
		DEBUG_LOG("\tobject_id: %c%c\n",g->object_id[0], g->object_id[1]);
	DEBUG_LOG("\tinstance_count: %d\n", g->instance_count);
	DEBUG_LOG("\tflags: %#x", g->flags);
	if (g->flags) {
		DEBUG_LOG(" ");
		if (g->flags & ACPI_WMI_EXPENSIVE)
			DEBUG_LOG("ACPI_WMI_EXPENSIVE ");
		if (g->flags & ACPI_WMI_METHOD)
			DEBUG_LOG("ACPI_WMI_METHOD ");
		if (g->flags & ACPI_WMI_STRING)
			DEBUG_LOG("ACPI_WMI_STRING ");
		if (g->flags & ACPI_WMI_EVENT)
			DEBUG_LOG("ACPI_WMI_EVENT ");
	}
}
#endif

/**
 * wmi_data2Str - converts binary guid to ascii guid
 *
 */
int AsusFnKeys::wmi_data2Str(const char *in, char *out)
{
	int i;
	
	for (i = 3; i >= 0; i--)
		out += snprintf(out, 3, "%02X", in[i] & 0xFF);
	
	out += snprintf(out, 2, "-");
	out += snprintf(out, 3, "%02X", in[5] & 0xFF);
	out += snprintf(out, 3, "%02X", in[4] & 0xFF);
	out += snprintf(out, 2, "-");
	out += snprintf(out, 3, "%02X", in[7] & 0xFF);
	out += snprintf(out, 3, "%02X", in[6] & 0xFF);
	out += snprintf(out, 2, "-");
	out += snprintf(out, 3, "%02X", in[8] & 0xFF);
	out += snprintf(out, 3, "%02X", in[9] & 0xFF);
	out += snprintf(out, 2, "-");
	
	for (i = 10; i <= 15; i++)
		out += snprintf(out, 3, "%02X", in[i] & 0xFF);
	
	*out = '\0';
	return 0;
}

/**
 * flagsToStr - converts binary flag to ascii flag
 *
 */
OSString * AsusFnKeys::flagsToStr(UInt8 flags)
{
	char str[80];
	char *pos = str;
	if (flags != 0)
	{
		if (flags & ACPI_WMI_EXPENSIVE)
		{
			strncpy(pos, "ACPI_WMI_EXPENSIVE ", 20);
			pos += strlen(pos);
		}
		if (flags & ACPI_WMI_METHOD)
		{
			strncpy(pos, "ACPI_WMI_METHOD ", 20);
			pos += strlen(pos);
            DEBUG_LOG("%s: WMI METHOD\n", this->getName());
		}
		if (flags & ACPI_WMI_STRING)
		{
			strncpy(pos, "ACPI_WMI_STRING ", 20);
			pos += strlen(pos);
		}
		if (flags & ACPI_WMI_EVENT)
		{
			strncpy(pos, "ACPI_WMI_EVENT ", 20);
			pos += strlen(pos);
            DEBUG_LOG("%s: WMI EVENT\n", this->getName());
		}
		//suppress the last trailing space
		str[strlen(str)] = 0;
	}
	else
	{
		str[0] = 0;
	}
	return (OSString::withCString(str));
}

/**
 * wmi_wdg2reg - adds the WDG structure to a dictionary
 *
 */
void AsusFnKeys::wmi_wdg2reg(struct guid_block *g, OSArray *array, OSArray *dataArray)
{
	char guid_string[37];
	char object_id_string[3];
	OSDictionary *dict = OSDictionary::withCapacity(6);
	
	wmi_data2Str(g->guid, guid_string);
    
	dict->setObject("UUID", OSString::withCString(guid_string));
	if (g->flags & ACPI_WMI_EVENT)
		dict->setObject("notify_value", OSNumber::withNumber(g->notify_id, 8));
	else
	{
		snprintf(object_id_string, 3, "%c%c", g->object_id[0], g->object_id[1]);
		dict->setObject("object_id", OSString::withCString(object_id_string));
	}
	dict->setObject("instance_count", OSNumber::withNumber(g->instance_count, 8));
	dict->setObject("flags", OSNumber::withNumber(g->flags, 8));
#if DEBUG
	dict->setObject("flags Str", flagsToStr(g->flags));
#endif
	if (g->flags == 0)
		dataArray->setObject(readDataBlock(object_id_string));
    
	
	array->setObject(dict);
}

OSDictionary * AsusFnKeys::readDataBlock(char *str)
{
	OSObject	*wqxx;
	OSData		*data = NULL;
	OSDictionary *dict;
	char name[5];
	
	snprintf(name, 5, "WQ%s", str);
	dict = OSDictionary::withCapacity(1);
	
	do
	{
		if (WMIDevice->evaluateObject(name, &wqxx) != kIOReturnSuccess)
		{
			IOLog("%s: No object of method %s\n", this->getName(), name);
			continue;
		}
		
		data = OSDynamicCast(OSData , wqxx);
		if (data == NULL){
			IOLog("%s: Cast error %s\n", this->getName(), name);
			continue;
		}
		dict->setObject(name, data);
	}
	while (false);
	return dict;
}

/*
 * Parse the _WDG method for the GUID data blocks
 */
int AsusFnKeys::parse_wdg(OSDictionary *dict)
{
	UInt32 i, total;
	OSObject	*wdg;
	OSData		*data;
	OSArray		*array, *dataArray;
    
	do
	{
		if (WMIDevice->evaluateObject("_WDG", &wdg) != kIOReturnSuccess)
		{
			IOLog("%s: No object of method _WDG\n", this->getName());
			continue;
		}
		
		data = OSDynamicCast(OSData , wdg);
		if (data == NULL){
			IOLog("%s: Cast error _WDG\n", this->getName());
			continue;
		}
		total = data->getLength() / sizeof(struct guid_block);
		array = OSArray::withCapacity(total);
		dataArray = OSArray::withCapacity(1);
		
		for (i = 0; i < total; i++) {
			wmi_wdg2reg((struct guid_block *) data->getBytesNoCopy(i * sizeof(struct guid_block), sizeof(struct guid_block)), array, dataArray);
		}
		setProperty("WDG", array);
		setProperty("DataBlocks", dataArray);
		data->release();
	}
	while (false);
	
	return 0;
}

#pragma mark -
#pragma mark IOService overloading
#pragma mark -

#define super IOService

OSDefineMetaClassAndStructors(AsusFnKeys, IOService)

const FnKeysKeyMap AsusFnKeys::keyMap[] = {
    {0x30, NX_KEYTYPE_SOUND_UP, "NX_KEYTYPE_SOUND_UP"},
    {0x31, NX_KEYTYPE_SOUND_DOWN, "NX_KEYTYPE_SOUND_DOWN"},
    {0x32, NX_KEYTYPE_MUTE, "NX_KEYTYPE_MUTE"},
    {0x61, NX_KEYTYPE_VIDMIRROR, "NX_KEYTYPE_VIDMIRROR"},
    {0x10, NX_KEYTYPE_BRIGHTNESS_UP, "NX_KEYTYPE_BRIGHTNESS_UP"},
    {0x20, NX_KEYTYPE_BRIGHTNESS_DOWN, "NX_KEYTYPE_BRIGHTNESS_DOWN"},
    //Media buttons bound to Asus events keys Down, Left and Right Arrows in full keyboard
    {0x40, NX_KEYTYPE_PREVIOUS, "NX_KEYTYPE_PREVIOUS"},
    {0x41, NX_KEYTYPE_NEXT, "NX_KEYTYPE_NEXT"},
    {0x45, NX_KEYTYPE_PLAY, "NX_KEYTYPE_PLAY"},
    //Media button bound to Asus events keys C, V and Space keys in compact keyboard
    {0x8A, NX_KEYTYPE_PREVIOUS, "NX_KEYTYPE_PREVIOUS"},
    {0x82, NX_KEYTYPE_NEXT, "NX_KEYTYPE_NEXT"},
    {0x5C, NX_KEYTYPE_PLAY, "NX_KEYTYPE_PLAY"},
    {0,0xFF,NULL}
};

bool AsusFnKeys::init(OSDictionary *dict)
{
    keybrdBLightLvl = 0; //Stating with Zero Level
    panelBrighntessLevel = 16; //Mac starts with level 16
    res = 0;
    
    tochpadEnabled = true; //touch enabled by default on startup
    alsMode = false;
    isALSenabled  = true;
    isPanelBackLightOn = true;
    
    hasKeybrdBLight = false;
    hasMediaButtons = true;
    
	bool result = super::init(dict);
	properties = dict;
    DEBUG_LOG("%s:\n", this->getName());
	return result;
}

void AsusFnKeys::free(void)
{
	DEBUG_LOG("%s: Free\n", this->getName());
	super::free();
}

IOService * AsusFnKeys::probe(IOService *provider, SInt32 *score)
{
    IOService * ret = NULL;
    OSObject * obj;
    OSString * name;
    IOACPIPlatformDevice *dev;
    do
    {
        if (!super::probe(provider, score))
            continue;
        
        dev = OSDynamicCast(IOACPIPlatformDevice, provider);
        if (NULL == dev)
            continue;
        
        dev->evaluateObject("_UID", &obj);
        
        name = OSDynamicCast(OSString, obj);
        if (NULL == name)
            continue;
        
        if (name->isEqualTo("ATK"))
        {
            *score +=20;
            ret = this;
        }
        name->release();
    }
    while (false);
    
    //Reading the prefereces from the plist file
    OSDictionary *Configuration;
    Configuration = OSDynamicCast(OSDictionary, getProperty("Preferences"));
    if (Configuration){
        OSString *tmpString = 0;
        OSNumber *tmpNumber = 0;
        OSData   *tmpData = 0;
        OSBoolean *tmpBoolean = FALSE;
        OSData   *tmpObj = 0;
        bool tmpBool = false;
        UInt64 tmpUI64 = 0;
        
        OSIterator *iter = 0;
        const OSSymbol *dictKey = 0;
        
        iter = OSCollectionIterator::withCollection(Configuration);
        if (iter) {
            while ((dictKey = (const OSSymbol *)iter->getNextObject())) {
                tmpObj = 0;
                
                tmpString = OSDynamicCast(OSString, Configuration->getObject(dictKey));
                if (tmpString) {
                    tmpObj = OSData::withBytes(tmpString->getCStringNoCopy(), tmpString->getLength()+1);
                }
                
                tmpNumber = OSDynamicCast(OSNumber, Configuration->getObject(dictKey));
                if (tmpNumber) {
                    tmpUI64 = tmpNumber->unsigned64BitValue();
                    tmpObj = OSData::withBytes(&tmpUI64, sizeof(UInt32));
                }
                
                tmpBoolean = OSDynamicCast(OSBoolean, Configuration->getObject(dictKey));
                if (tmpBoolean) {
                    tmpBool = (bool)tmpBoolean->getValue();
                    tmpObj = OSData::withBytes(&tmpBool, sizeof(bool));
                    
                }
                
                tmpData = OSDynamicCast(OSData, Configuration->getObject(dictKey));
                if (tmpData) {
                    tmpObj = tmpData;
                }
                if (tmpObj) {
                    //provider->setProperty(dictKey, tmpObj);
                    /*if(tmpUI64>0)
                     setProperty(dictKey->getCStringNoCopy(), tmpUI64 ,64);
                     else
                     setProperty(dictKey->getCStringNoCopy(), tmpBool?1:0 ,32);*/
                    
                    const char *tmpStr = dictKey->getCStringNoCopy();
                    
                    
                    if(!strncmp(dictKey->getCStringNoCopy(),"KeyboardBLightLevelAtBoot", strlen(tmpStr)))
                    {
                        keybrdBLightLvl = (UInt32)tmpUI64;
                        tmpUI64 = 0;
                    }
                    else if(!strncmp(dictKey->getCStringNoCopy(),"HasKeyboardBLight",strlen(tmpStr)))
                        hasKeybrdBLight = tmpBool;
                    
                    else if(!strncmp(dictKey->getCStringNoCopy(),"HasMediaButtons",strlen(tmpStr)))
                        hasMediaButtons = tmpBool;
                    
                    else if(!strncmp(dictKey->getCStringNoCopy(),"HasALSensor",strlen(tmpStr)))
                        hasALSensor = tmpBool;
                    
                    else if(!strncmp(dictKey->getCStringNoCopy(),"ALS Turned on at boot",strlen(tmpStr)))
                        alsAtBoot = tmpBool;
                    
                }
            }
        }
    }
    
    return (ret);
}

void AsusFnKeys::stop(IOService *provider)
{
	DEBUG_LOG("%s: Stop\n", this->getName());
	
	disableEvent();
	PMstop();
	
	super::stop(provider);
	return;
}

static IOPMPowerState powerStateArray[ kPowerStateCount ] =
{
	{ 1,0,0,0,0,0,0,0,0,0,0,0 },
	{ 1,IOPMDeviceUsable,IOPMPowerOn,IOPMPowerOn,0,0,0,0,0,0,0,0 }
};

bool AsusFnKeys::start(IOService *provider)
{
	if(!provider || !super::start( provider ))
	{
		IOLog("%s: Error loading kext\n", this->getName());
		return false;
	}
	
	WMIDevice = (IOACPIPlatformDevice *) provider;		// ACPI device
	
	IOLog("%s: Found WMI Device %s\n", this->getName(), WMIDevice->getName());
	
	_keyboardDevice = NULL;
	
	parse_wdg(properties);
	
	enableEvent();
	
	PMinit();
    registerPowerDriver(this, powerStateArray, 2);
	provider->joinPMtree(this);
	
	this->registerService(0);
    
    	
	return true;
}

/*
 * Computer power state hook
 * Nothing to do for the moment
 *
 */
IOReturn AsusFnKeys::setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker)
{
	if (kPowerStateOff == powerStateOrdinal)
	{
		DEBUG_LOG("%s: setPowerState(kPowerStateOff)\n", this->getName());
		
	}
	else if (kPowerStateOn == powerStateOrdinal)
	{
        DEBUG_LOG("%s: setPowerState(kPowerStateOn)\n", this->getName());
		
	}
	
	return IOPMAckImplied;
}

#pragma mark -
#pragma mark AsusFnKeys Methods
#pragma mark -

IOReturn AsusFnKeys::message( UInt32 type, IOService * provider, void * argument)
{
	if (type == kIOACPIMessageDeviceNotification)
	{
		UInt32 event = *((UInt32 *) argument);
		OSObject * wed;
		
		OSNumber * number = OSNumber::withNumber(event,32);
		WMIDevice->evaluateObject("_WED", &wed, (OSObject**)&number,1);
		number->release();
		number = OSDynamicCast(OSNumber, wed);
		if (NULL == number)
        {
            //try a package
            OSArray * array = OSDynamicCast(OSArray, wed);
            if (NULL == array)
            {
                //try a buffer
                OSData * data = OSDynamicCast(OSData, wed);
                if ( (NULL == data) || (data->getLength() == 0))
                {
                    DEBUG_LOG("%s: Fail to cast _WED returned objet %s\n", this->getName(), wed->getMetaClass()->getClassName());
                    return kIOReturnError;
                }
                const char * bytes = (const char *) data->getBytesNoCopy();
                number = OSNumber::withNumber(bytes[0],32);
            }
            else
            {
                number = OSDynamicCast(OSNumber, array->getObject(0));
                if (NULL == number)
                {
                    DEBUG_LOG("%s: Fail to cast _WED returned 1st objet in array %s\n", this->getName(), array->getObject(0)->getMetaClass()->getClassName());
                    return kIOReturnError;
                }
            }
        }
        
		handleMessage(number->unsigned32BitValue());
	}
	else
	{	// Someone unexpected is sending us messages!
		DEBUG_LOG("%s: Unexpected message, Type %x Provider %s \n", this->getName(), uint(type), provider->getName());
	}
	
	return kIOReturnSuccess;
}

void AsusFnKeys::handleMessage(int code)
{
    loopCount = kLoopCount = res = 0;
    alsMode = false;
    
    //Processing the code
    switch (code) {
        case 0x57: //AC disconnected
        case 0x58: //AC connected
            //ignore silently
            break;
            
            //Backlight
        case 0x33://hardwired On
        case 0x34://hardwired Off
        case 0x35://Soft Event, Fn + F7
            if(isPanelBackLightOn)
            {
                code = NOTIFY_BRIGHTNESS_DOWN_MIN;
                loopCount = 16;
                
                //Read Panel brigthness value to restore later with backlight toggle
                ReadPanelBrightnessValue();
            }
            else
            {
                code = NOTIFY_BRIGHTNESS_UP_MIN;
                loopCount = panelBrighntessLevel;
            }
            
            isPanelBackLightOn = !isPanelBackLightOn;
            break;
            
        case 0x6B: //Fn + F9, Tochpad On/Off
            tochpadEnabled = !tochpadEnabled;
            if(tochpadEnabled)
            {
                setProperty("TouchpadEnabled", true);
                removeProperty("TouchpadDisabled");
                IOLog("AsusFnKeys: Touchpad Enabled\n");
            }
            else
            {
                removeProperty("TouchpadEnabled");
                setProperty("TouchpadDisabled", true);
                IOLog("AsusFnKeys: Touchpad Disabled\n");
            }
            break;
            
        case 0x5C: //Fn + space bar, Processor Speedstepping changes
            
            /*params[0] =OSNumber::withNumber(4, 8);
             
             if(WMIDevice->evaluateInteger("PSTT", &res, params, 1))
             IOLog("AsusFnKeys: Processor speedstep Changed\n");
             else
             IOLog("AsusFnKeys: Processor speedstep change failed %d\n",res);*/
            
            break;
            
        case 0x7A: // Fn + A, ALS Sensor
            isALSenabled = !isALSenabled;
            
            params[0] =OSNumber::withNumber(isALSenabled, 8);
            
            if(WMIDevice->evaluateInteger("ALSC", &res, params, 1))
                IOLog("AsusFnKeys: ALS Enabled %d\n",isALSenabled);
            else
                IOLog("AsusFnKeys: ALS Disabled %d E %d\n",res,isALSenabled);
            break;
            
        case 0xC6: //ALS Notifcations
            if(hasALSensor)
            {
                code = processALS();
                alsMode = true;
            }
            break;
            
        case 0xC7: //ALS Notifcations (Optional)
            if(hasALSensor)
            {
                code = processALS();
                alsMode = true;
            }
            break;
            
        case 0xC5: //Fn + F3,Decrease Keyboard Backlight
            if(hasKeybrdBLight)
            {
                if(keybrdBLightLvl>0)
                    keybrdBLightLvl--;
                else
                    keybrdBLightLvl = 0;
                
                keyboardBackLightEvent(keybrdBLightLvl);
                
                curKeybrdBlvl  = keybrdBLightLvl;
                
                //Updating value in ioregistry
                setProperty("KeyboardBLightLevel", keybrdBLightLvl,32);
            }
            
            break;
            
        case 0xC4: //Fn + F4, Increase Keyboard Backlight
            if(hasKeybrdBLight)
            {
                if(keybrdBLightLvl == 3)
                    keybrdBLightLvl = 3;
                else
                    keybrdBLightLvl++;
                
                keyboardBackLightEvent(keybrdBLightLvl);
                
                curKeybrdBlvl  = keybrdBLightLvl;
                
                //Updating value in ioregistry
                setProperty("KeyboardBLightLevel", keybrdBLightLvl,32);
            }
            
            break;
            
        default:
            //Fn + F5, Panel Brightness Down
            if(code >= NOTIFY_BRIGHTNESS_DOWN_MIN && code<= NOTIFY_BRIGHTNESS_DOWN_MAX)
            {
                code = NOTIFY_BRIGHTNESS_DOWN_MIN;
                
                if(panelBrighntessLevel > 0)
                    panelBrighntessLevel--;
            }
            //Fn + F6, Panel Brightness Up
            else if(code >= NOTIFY_BRIGHTNESS_UP_MIN && code<= NOTIFY_BRIGHTNESS_UP_MAX)
            {
                code = NOTIFY_BRIGHTNESS_UP_MIN;
                
                panelBrighntessLevel++;
                if(panelBrighntessLevel>16)
                    panelBrighntessLevel = 16;
            }
            break;
    }
    
    //IOLog("AsusFnKeys: Received Key %d(0x%x) ALS mode %d\n",code, code, alsMode);
    
    //have media buttons then skip C, V and Space & ALS sensor keys events
    if(hasMediaButtons && (code == 0x8A || code == 0x82 || code == 0x5c || code == 0xc6 || code == 0x5c))
        return;
    
    //Sending the code for the keyboard handler
    processFnKeyEvents(code, alsMode, kLoopCount, loopCount);
    
    //Clearing ALS mode after processing
    if(alsMode)
        alsMode = false;
}

//
//Process Fn key event for ALS
//
void AsusFnKeys::processFnKeyEvents(int code, bool alsMode, int kLoopCount, int bLoopCount)
{
    //Ambient Light Sensor Mode sends either 4 Brightness Up/Down events
    if(alsMode)
    {
        for(int i =0; i < kLoopCount; i++)
            _keyboardDevice->keyPressed(code);
        DEBUG_LOG("AsusFnKeys: Loop Count %d, Dispatch Key %d(0x%x)\n",kLoopCount, code, code);
    }
    else if(bLoopCount>0)
    {
        for (int j = 0; j < bLoopCount; j++)
            _keyboardDevice->keyPressed(code);
        DEBUG_LOG("AsusFnKeys: Loop Count %d, Dispatch Key %d(0x%x)\n",bLoopCount, code,code);
    }
    else
    {
        _keyboardDevice->keyPressed(code);
        DEBUG_LOG("AsusFnKeys: Dispatch Key %d(0x%x)\n", code, code);
    }
}

UInt32 AsusFnKeys::processALS()
{
    UInt32 brightnessLvlcode;
    keybrdBLightLvl = 0;
    
    WMIDevice->evaluateInteger("ALSS", &keybrdBLightLvl, NULL, NULL);
    //IOLog("AsusFnKeys: ALS %d\n",keybrdBLightLvl);
    
    if(keybrdBLightLvl == 1 && curKeybrdBlvl > keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_DOWN_MIN;
        kLoopCount = 6;
    }
    else if(keybrdBLightLvl == 1 && curKeybrdBlvl < keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_UP_MIN;
        kLoopCount = 6;
    }
    else if(keybrdBLightLvl == 2 && curKeybrdBlvl > keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_DOWN_MIN;
        kLoopCount = 3;
    }
    else if(keybrdBLightLvl == 2 && curKeybrdBlvl < keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_UP_MIN;
        kLoopCount = 3;
    }
    else if(keybrdBLightLvl == 3 && curKeybrdBlvl > keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_DOWN_MIN;
        kLoopCount = 3;
    }
    else if(keybrdBLightLvl == 3 && curKeybrdBlvl < keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_UP_MIN;
        kLoopCount = 3;
    }
    else if(keybrdBLightLvl == 4 && curKeybrdBlvl > keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_DOWN_MIN;
        kLoopCount = 6;
    }
    else if(keybrdBLightLvl == 4 && curKeybrdBlvl < keybrdBLightLvl)
    {
        brightnessLvlcode = NOTIFY_BRIGHTNESS_UP_MIN;
        kLoopCount = 6;
    }
    else
    {
        brightnessLvlcode = 0xC6; //ALS event code which does nothing
        kLoopCount = 0;
    }
    
    curKeybrdBlvl = keybrdBLightLvl;
    
    return brightnessLvlcode;
}

//Keyboard Backlight set
void AsusFnKeys::keyboardBackLightEvent(UInt32 level)
{
    OSObject * params[1];
    OSObject * ret = NULL;
    
    params[0] = OSNumber::withNumber(level,8);
    
    //Asus WMI Specific Method Inside the DSDT
    //Calling the Method SLKB from the DSDT For setting Keyboard Backlight control in DSDT
    WMIDevice->evaluateObject("SKBL", &ret, params,1);
}

void AsusFnKeys::ReadPanelBrightnessValue()
{
    //
    //Reading AppleBezel Values from Apple Backlight Panel driver for controlling the bezel levels
    //
    
    IORegistryEntry *displayDeviceEntry = IORegistryEntry::fromPath("IOService:/AppleACPIPlatformExpert/PCI0@0/AppleACPIPCI/IGPU@2/AppleIntelFramebuffer@0/display0/AppleBacklightDisplay");
    
    if (displayDeviceEntry != NULL) {
        
        OSNumber *brightnessValue = 0;
        OSDictionary *ioDisplayParaDict = 0;
        
        ioDisplayParaDict = OSDynamicCast(OSDictionary, displayDeviceEntry->getProperty("IODisplayParameters"));
        
        if(ioDisplayParaDict)
        {
            OSDictionary  *brightnessDict = 0;
            OSIterator *brightnessIter = 0;
            
            brightnessDict = OSDynamicCast(OSDictionary, ioDisplayParaDict->getObject("brightness"));
            
            if(brightnessDict){
                const OSSymbol *dicKey = 0;
                
                brightnessIter = OSCollectionIterator::withCollection(brightnessDict);
                
                if(brightnessIter)
                {
                    while((dicKey = (const OSSymbol *)brightnessIter->getNextObject()))
                    {
                        
                        //IOLog("AsusFnKeys: Brightness %s\n",dicKey->getCStringNoCopy());
                        brightnessValue = OSDynamicCast(OSNumber, brightnessDict->getObject(dicKey));
                        
                        if(brightnessValue)
                        {
                            if(brightnessValue->unsigned32BitValue() != 0)
                                panelBrighntessLevel = brightnessValue->unsigned32BitValue()/64;
                            //IOLog("AsusFnKeys: PB %d BValue %d\n",panelBrighntessLevel,brightnessValue->unsigned32BitValue());
                        }
                    }
                }
            }
        }
    }
}

void AsusFnKeys::getDeviceStatus(const char * guid, UInt32 methodId, UInt32 deviceId, UInt32 *status)
{
	DEBUG_LOG("%s: getDeviceStatus()\n", this->getName());
	
	char method[5];
	OSObject * params[3];
	OSString *str;
	OSDictionary *dict = getDictByUUID(guid);
	if (NULL == dict)
		return;
	
	str = OSDynamicCast(OSString, dict->getObject("object_id"));
	if (NULL == str)
		return;
	
	snprintf(method, 5, "WM%s", str->getCStringNoCopy());
    
	params[0] = OSNumber::withNumber(0x00D,32);
	params[1] = OSNumber::withNumber(methodId,32);
	params[2] = OSNumber::withNumber(deviceId,32);
	
	WMIDevice->evaluateInteger(method, status, params, 3);
	
	params[0]->release();
	params[1]->release();
	params[2]->release();
	
	return;
}

void AsusFnKeys::setDeviceStatus(const char * guid, UInt32 methodId, UInt32 deviceId, UInt32 *status)
{
	DEBUG_LOG("%s: setDeviceStatus()\n", this->getName());
	
	char method[5];
	char buffer[8];
	OSObject * params[3];
	OSString *str;
	OSDictionary *dict = getDictByUUID(guid);
	if (NULL == dict)
		return;
	
	str = OSDynamicCast(OSString, dict->getObject("object_id"));
	if (NULL == str)
		return;
	
	snprintf(method, 5, "WM%s", str->getCStringNoCopy());
	
	memcpy(buffer, &deviceId, 4);
	memcpy(buffer+4, status, 4);
	
	params[0] = OSNumber::withNumber(0x00D,32);
	params[1] = OSNumber::withNumber(methodId,32);
	params[2] = OSData::withBytes(buffer, 8);
    
	*status = ~0;
	WMIDevice->evaluateInteger(method, status, params, 3);
	
	DEBUG_LOG("%s: setDeviceStatus Res = %x\n", this->getName(), (unsigned int)*status);
	
	params[0]->release();
	params[1]->release();
	params[2]->release();
    
	return;
}

void AsusFnKeys::setDevice(const char * guid, UInt32 methodId, UInt32 *status)
{
	DEBUG_LOG("%s: setDevice(%d)\n", this->getName(), (int)*status);
	
	char method[5];
	char buffer[4];
	OSObject * params[3];
	OSString *str;
	OSDictionary *dict = getDictByUUID(guid);
	if (NULL == dict)
		return;
	
	str = OSDynamicCast(OSString, dict->getObject("object_id"));
	if (NULL == str)
		return;
	
	snprintf(method, 5, "WM%s", str->getCStringNoCopy());
	
	memcpy(buffer, status, 4);
	
	params[0] = OSNumber::withNumber(0x00D,32);
	params[1] = OSNumber::withNumber(methodId,32);
	params[2] = OSData::withBytes(buffer, 8);
	
	*status = ~0;
	WMIDevice->evaluateInteger(method, status, params, 3);
	
	DEBUG_LOG("%s: setDevice Res = %x\n", this->getName(), (unsigned int)*status);
	
	params[0]->release();
	params[1]->release();
	params[2]->release();
	
	return;
}


OSDictionary* AsusFnKeys::getDictByUUID(const char * guid)
{
	UInt32 i;
	OSDictionary	*dict = NULL;
	OSString		*uuid;
	OSArray *array = OSDynamicCast(OSArray, properties->getObject("WDG"));
	if (NULL == array)
		return NULL;
	for (i=0; i<array->getCount(); i++) {
		dict = OSDynamicCast(OSDictionary, array->getObject(i));
		uuid = OSDynamicCast(OSString, dict->getObject("UUID"));
		if (uuid->isEqualTo(guid)){
            break;
        }
        
	}
	return dict;
}


IOReturn AsusFnKeys::enableFnKeyEvents(const char * guid, UInt32 methodId)
{
   
    //Asus WMI Specific Method Inside the DSDT
    //Calling the Asus Method INIT from the DSDT to enable the Hotkey Events
    WMIDevice->evaluateObject("INIT", NULL, NULL, NULL);
    
	return kIOReturnSuccess;//return always success
}



#pragma mark -
#pragma mark Event handling methods
#pragma mark -

void AsusFnKeys::enableEvent()
{
    
    if (enableFnKeyEvents(ASUS_WMI_MGMT_GUID, ASUS_WMI_METHODID_INIT) != kIOReturnSuccess)
        IOLog("Unable to enable events!!!\n");
    else
    {
        _keyboardDevice = new FnKeysHIKeyboardDevice;
        
        if ( !_keyboardDevice               ||
            !_keyboardDevice->init()       ||
            !_keyboardDevice->attach(this) )  // goto fail;
        {
            _keyboardDevice->release();
            IOLog("%s: Error creating keyboardDevice\n", this->getName());
        }
        else
        {
            _keyboardDevice->setKeyMap(keyMap);
            _keyboardDevice->registerService();
            
            //Setting Touchpad state on startup
            setProperty("TouchpadEnabled", true);
            
            /**** Keyboard brightness level at boot
             ***** Calling the keyboardBacklight Event for Setting the Backlight at boot ***/
            if(hasKeybrdBLight)
                keyboardBackLightEvent(keybrdBLightLvl);
            
            curKeybrdBlvl = keybrdBLightLvl;
            
            /**** ALS Sesnor at boot ***/
            if(alsAtBoot)
            {
                isALSenabled = !isALSenabled;
                params[0] = OSNumber::withNumber(isALSenabled, 8);
                WMIDevice->evaluateInteger("ALSC", &res,params,1);
                
                IOLog("AsusFnKeys: ALS turned on at boot %d\n",res);
            }
            
            IOLog("%s: Asus Fn Hotkey Events Enabled\n", this->getName());
            
        }
    }
    
}

void AsusFnKeys::disableEvent()
{
	if (_keyboardDevice)
	{
		_keyboardDevice->release();
		_keyboardDevice = NULL;
	}
}
