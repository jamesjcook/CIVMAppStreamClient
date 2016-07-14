/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights
 * Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may
 * not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 * http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, express or implied. See the License for
 * the specific language governing permissions and limitations under
 * the License.
 *
 */


#ifndef __AppStreamSampleClient__TextHelper__
#define __AppStreamSampleClient__TextHelper__


//Bit-wise masks for Windows Modifier Keys
#define STX_SHIFT_KEY_MASK      1
#define STX_CONTROL_KEY_MASK    2
#define STX_ALT_KEY_MASK        4
#define STX_WINDOWS_KEY_MASK    8


//Maps to the Virtual KeyCode when passed in an ASCII char
bool getVirtualKeyUsingChar (uint16_t &keyChar, uint16_t &commandKeysBitmask, uint16_t &commandKeysIgnored);


//Key constants
static const uint32_t VK_BACK                   = 0x08; //BACKSPACE key
static const uint32_t VK_TAB                    = 0x09; //TAB key
static const uint32_t VK_CLEAR                  = 0x0C; //CLEAR key
static const uint32_t VK_RETURN                 = 0x0D; //ENTER key
static const uint32_t VK_SHIFT                  = 0x10; //SHIFT key
static const uint32_t VK_CONTROL                = 0x11; //CTRL key
static const uint32_t VK_MENU                   = 0x12; //ALT key
static const uint32_t VK_PAUSE                  = 0x13; //PAUSE key
static const uint32_t VK_CAPITAL                = 0x14; //CAPS LOCK key
static const uint32_t VK_KANA                   = 0x15; //IME Kana mode
static const uint32_t VK_HANGUEL                = 0x15; //IME Hanguel mode (maintained for compatibility; use VK_HANGUL)
static const uint32_t VK_HANGUL                 = 0x15; //IME Hangul mode
static const uint32_t VK_JUNJA                  = 0x17; //IME Junja mode
static const uint32_t VK_FINAL                  = 0x18; //IME final mode
static const uint32_t VK_HANJA                  = 0x19; //IME Hanja mode
static const uint32_t VK_KANJI                  = 0x19; //IME Kanji mode
static const uint32_t VK_ESCAPE                 = 0x1B; //ESC key
static const uint32_t VK_CONVERT                = 0x1C; //IME convert
static const uint32_t VK_NONCONVERT             = 0x1D; //IME nonconvert
static const uint32_t VK_ACCEPT                 = 0x1E; //IME accept
static const uint32_t VK_MODECHANGE             = 0x1F; //IME mode change request
static const uint32_t VK_SPACE                  = 0x20; //SPACEBAR
static const uint32_t VK_PRIOR                  = 0x21; //PAGE UP key
static const uint32_t VK_NEXT                   = 0x22; //PAGE DOWN key
static const uint32_t VK_END                    = 0x23; //END key
static const uint32_t VK_HOME                   = 0x24; //HOME key
static const uint32_t VK_LEFT                   = 0x25; //LEFT ARROW key
static const uint32_t VK_UP                     = 0x26; //UP ARROW key
static const uint32_t VK_RIGHT                  = 0x27; //RIGHT ARROW key
static const uint32_t VK_DOWN                   = 0x28; //DOWN ARROW key
static const uint32_t VK_SELECT                 = 0x29; //SELECT key
static const uint32_t VK_PRINT                  = 0x2A; //PRINT key
static const uint32_t VK_EXECUTE                = 0x2B; //EXECUTE key
static const uint32_t VK_SNAPSHOT               = 0x2C; //PRINT SCREEN key
static const uint32_t VK_INSERT                 = 0x2D; //INS key
static const uint32_t VK_DELETE                 = 0x2E; //DEL key
static const uint32_t VK_HELP                   = 0x2F; //HELP key
static const uint32_t kKEY_0                    = 0x30; //0 key
static const uint32_t kKEY_1                    = 0x31; //1 key
static const uint32_t kKEY_2                    = 0x32; //2 key
static const uint32_t kKEY_3                    = 0x33; //3 key
static const uint32_t kKEY_4                    = 0x34; //4 key
static const uint32_t kKEY_5                    = 0x35; //5 key
static const uint32_t kKEY_6                    = 0x36; //6 key
static const uint32_t kKEY_7                    = 0x37; //7 key
static const uint32_t kKEY_8                    = 0x38; //8 key
static const uint32_t kKEY_9                    = 0x39; //9 key
static const uint32_t kKEY_A                    = 0x41; //A key
static const uint32_t kKEY_B                    = 0x42; //B key
static const uint32_t kKEY_C                    = 0x43; //C key
static const uint32_t kKEY_D                    = 0x44; //D key
static const uint32_t kKEY_E                    = 0x45; //E key
static const uint32_t kKEY_F                    = 0x46; //F key
static const uint32_t kKEY_G                    = 0x47; //G key
static const uint32_t kKEY_H                    = 0x48; //H key
static const uint32_t kKEY_I                    = 0x49; //I key
static const uint32_t kKEY_J                    = 0x4A; //J key
static const uint32_t kKEY_K                    = 0x4B; //K key
static const uint32_t kKEY_L                    = 0x4C; //L key
static const uint32_t kKEY_M                    = 0x4D; //M key
static const uint32_t kKEY_N                    = 0x4E; //N key
static const uint32_t kKEY_O                    = 0x4F; //O key
static const uint32_t kKEY_P                    = 0x50; //P key
static const uint32_t kKEY_Q                    = 0x51; //Q key
static const uint32_t kKEY_R                    = 0x52; //R key
static const uint32_t kKEY_S                    = 0x53; //S key
static const uint32_t kKEY_T                    = 0x54; //T key
static const uint32_t kKEY_U                    = 0x55; //U key
static const uint32_t kKEY_V                    = 0x56; //V key
static const uint32_t kKEY_W                    = 0x57; //W key
static const uint32_t kKEY_X                    = 0x58; //X key
static const uint32_t kKEY_Y                    = 0x59; //Y key
static const uint32_t kKEY_Z                    = 0x5A; //Z key
static const uint32_t VK_LWIN                   = 0x5B; //Left Windows key (Natural keyboard)
static const uint32_t VK_RWIN                   = 0x5C; //Right Windows key (Natural keyboard)
static const uint32_t VK_APPS                   = 0x5D; //Applications key (Natural keyboard)
static const uint32_t VK_SLEEP                  = 0x5F; //Computer Sleep key
static const uint32_t VK_NUMPAD0                = 0x60; //Numeric keypad 0 key
static const uint32_t VK_NUMPAD1                = 0x61; //Numeric keypad 1 key
static const uint32_t VK_NUMPAD2                = 0x62; //Numeric keypad 2 key
static const uint32_t VK_NUMPAD3                = 0x63; //Numeric keypad 3 key
static const uint32_t VK_NUMPAD4                = 0x64; //Numeric keypad 4 key
static const uint32_t VK_NUMPAD5                = 0x65; //Numeric keypad 5 key
static const uint32_t VK_NUMPAD6                = 0x66; //Numeric keypad 6 key
static const uint32_t VK_NUMPAD7                = 0x67; //Numeric keypad 7 key
static const uint32_t VK_NUMPAD8                = 0x68; //Numeric keypad 8 key
static const uint32_t VK_NUMPAD9                = 0x69; //Numeric keypad 9 key
static const uint32_t VK_MULTIPLY               = 0x6A; //Multiply key
static const uint32_t VK_ADD                    = 0x6B; //Add key
static const uint32_t VK_SEPARATOR              = 0x6C; //Separator key
static const uint32_t VK_SUBTRACT               = 0x6D; //Subtract key
static const uint32_t VK_DECIMAL                = 0x6E; //Decimal key
static const uint32_t VK_DIVIDE                 = 0x6F; //Divide key
static const uint32_t VK_F1                     = 0x70; //F1 key
static const uint32_t VK_F2                     = 0x71; //F2 key
static const uint32_t VK_F3                     = 0x72; //F3 key
static const uint32_t VK_F4                     = 0x73; //F4 key
static const uint32_t VK_F5                     = 0x74; //F5 key
static const uint32_t VK_F6                     = 0x75; //F6 key
static const uint32_t VK_F7                     = 0x76; //F7 key
static const uint32_t VK_F8                     = 0x77; //F8 key
static const uint32_t VK_F9                     = 0x78; //F9 key
static const uint32_t VK_F10                    = 0x79; //F10 key
static const uint32_t VK_F11                    = 0x7A; //F11 key
static const uint32_t VK_F12                    = 0x7B; //F12 key
static const uint32_t VK_F13                    = 0x7C; //F13 key
static const uint32_t VK_F14                    = 0x7D; //F14 key
static const uint32_t VK_F15                    = 0x7E; //F15 key
static const uint32_t VK_F16                    = 0x7F; //F16 key
static const uint32_t VK_F17                    = 0x80; //F17 key
static const uint32_t VK_F18                    = 0x81; //F18 key
static const uint32_t VK_F19                    = 0x82; //F19 key
static const uint32_t VK_F20                    = 0x83; //F20 key
static const uint32_t VK_F21                    = 0x84; //F21 key
static const uint32_t VK_F22                    = 0x85; //F22 key
static const uint32_t VK_F23                    = 0x86; //F23 key
static const uint32_t VK_F24                    = 0x87; //F24 key
static const uint32_t VK_NUMLOCK                = 0x90; //NUM LOCK key
static const uint32_t VK_SCROLL                 = 0x91; //SCROLL LOCK key
static const uint32_t VK_LSHIFT                 = 0xA0; //Left SHIFT key
static const uint32_t VK_RSHIFT                 = 0xA1; //Right SHIFT key
static const uint32_t VK_LCONTROL               = 0xA2; //Left CONTROL key
static const uint32_t VK_RCONTROL               = 0xA3; //Right CONTROL key
static const uint32_t VK_LMENU                  = 0xA4; //Left MENU key
static const uint32_t VK_RMENU                  = 0xA5; //Right MENU key
static const uint32_t VK_BROWSER_BACK           = 0xA6; //Browser Back key
static const uint32_t VK_BROWSER_FORWARD        = 0xA7; //Browser Forward key
static const uint32_t VK_BROWSER_REFRESH        = 0xA8; //Browser Refresh key
static const uint32_t VK_BROWSER_STOP           = 0xA9; //Browser Stop key
static const uint32_t VK_BROWSER_SEARCH         = 0xAA; //Browser Search key
static const uint32_t VK_BROWSER_FAVORITES      = 0xAB; //Browser Favorites key
static const uint32_t VK_BROWSER_HOME           = 0xAC; //Browser Start and Home key
static const uint32_t VK_VOLUME_MUTE            = 0xAD; //Volume Mute key
static const uint32_t VK_VOLUME_DOWN            = 0xAE; //Volume Down key
static const uint32_t VK_VOLUME_UP              = 0xAF; //Volume Up key
static const uint32_t VK_MEDIA_NEXT_TRACK       = 0xB0; //Next Track key
static const uint32_t VK_MEDIA_PREV_TRACK       = 0xB1; //Previous Track key
static const uint32_t VK_MEDIA_STOP             = 0xB2; //Stop Media key
static const uint32_t VK_MEDIA_PLAY_PAUSE       = 0xB3; //Play/Pause Media key
static const uint32_t VK_LAUNCH_MAIL            = 0xB4; //Start Mail key
static const uint32_t VK_LAUNCH_MEDIA_SELECT    = 0xB5; //Select Media key
static const uint32_t VK_LAUNCH_APP1            = 0xB6; //Start Application 1 key
static const uint32_t VK_LAUNCH_APP2            = 0xB7; //Start Application 2 key
static const uint32_t VK_OEM_1                  = 0xBA; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the ';:' key
static const uint32_t VK_OEM_PLUS               = 0xBB; //For any country/region, the '+' key
static const uint32_t VK_OEM_COMMA              = 0xBC; //For any country/region, the ',' key
static const uint32_t VK_OEM_MINUS              = 0xBD; //For any country/region, the '-' key
static const uint32_t VK_OEM_PERIOD             = 0xBE; //For any country/region, the '.' key
static const uint32_t VK_OEM_2                  = 0xBF; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the '/?' key
static const uint32_t VK_OEM_3                  = 0xC0; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the '`~' key
static const uint32_t VK_OEM_4                  = 0xDB; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the '[{' key
static const uint32_t VK_OEM_5                  = 0xDC; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the '\|' key
static const uint32_t VK_OEM_6                  = 0xDD; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the ']}' key
static const uint32_t VK_OEM_7                  = 0xDE; //Used for miscellaneous characters; it can vary by keyboard.
//For the US standard keyboard, the 'single-quote/double-quote' key
static const uint32_t VK_OEM_8                  = 0xDF; //Used for miscellaneous characters; it can vary by keyboard.
static const uint32_t VK_OEM_102                = 0xE2; //Either the angle bracket key or the backslash key on the RT 102-key keyboard
static const uint32_t VK_PROCESSKEY             = 0xE5; //static const uint32_t IME PROCESS key
static const uint32_t VK_PACKET                 = 0xE7; //Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
static const uint32_t VK_ATTN                   = 0xF6; //Attn key
static const uint32_t VK_CRSEL                  = 0xF7; //CrSel key
static const uint32_t VK_EXSEL                  = 0xF8; //ExSel key
static const uint32_t VK_EREOF                  = 0xF9; //Erase EOF key
static const uint32_t VK_PLAY                   = 0xFA; //Play key
static const uint32_t VK_ZOOM                   = 0xFB; //Zoom key
static const uint32_t VK_NONAME                 = 0xFC; //Reserved
static const uint32_t VK_PA1                    = 0xFD; //PA1 key
static const uint32_t VK_OEM_CLEAR              = 0xFE; //Clear key


#endif /* defined(__AppStreamSampleClient__TextHelper__) */
