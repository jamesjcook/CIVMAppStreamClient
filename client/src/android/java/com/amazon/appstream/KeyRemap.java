package com.amazon.appstream;

import java.util.Locale;

import android.util.Log;
import android.view.KeyEvent;

public class KeyRemap {
    public static final int VK_SHIFT = 0x10;
    private static final String TAG = "KeyRemap";

    public static boolean handleAndroidKey(KeyEvent msg, boolean down) {
        int keyCode = msg.getKeyCode();
        switch (keyCode) {
        case KeyEvent.KEYCODE_FORWARD_DEL:
            AppStreamInterface.keyPress(0x2E, down); break;
        case KeyEvent.KEYCODE_DEL:
            AppStreamInterface.keyPress(0x08, down); break;
        case KeyEvent.KEYCODE_META_LEFT :
            AppStreamInterface.keyPress(0x5B, down); break; // VK_LWIN
        case KeyEvent.KEYCODE_META_RIGHT :
            AppStreamInterface.keyPress(0x5C, down); break; // VK_RWIN
        case KeyEvent.KEYCODE_NUM :
        case KeyEvent.KEYCODE_ALT_LEFT:
            AppStreamInterface.keyPress(0x12, down);
            AppStreamInterface.keyPress(0xA4, down); break;
        case KeyEvent.KEYCODE_ALT_RIGHT:
            AppStreamInterface.keyPress(0x12, down);
            AppStreamInterface.keyPress(0xA5, down); break;
        case KeyEvent.KEYCODE_CTRL_LEFT:
            AppStreamInterface.keyPress(0x11, down);
            AppStreamInterface.keyPress(0xA2, down); break;
        case KeyEvent.KEYCODE_CTRL_RIGHT:
            AppStreamInterface.keyPress(0x11, down);
            AppStreamInterface.keyPress(0xA3, down); break;
        case KeyEvent.KEYCODE_SHIFT_LEFT:
            AppStreamInterface.keyPress(VK_SHIFT, down);
            AppStreamInterface.keyPress(0xA0, down); break;
        case KeyEvent.KEYCODE_SHIFT_RIGHT:
            AppStreamInterface.keyPress(VK_SHIFT, down);
            AppStreamInterface.keyPress(0xA1, down); break;
        case KeyEvent.KEYCODE_ENTER:
            AppStreamInterface.keyPress(0x0D, down); break;
        case KeyEvent.KEYCODE_VOLUME_UP:
        	return false; // We'll adjust volume on Android
            //AppStreamInterface.keyPress(0xAF, down); break; // VK_VOLUME_UP
        case KeyEvent.KEYCODE_VOLUME_DOWN:
        	return false; // We'll adjust volume on Android
            //AppStreamInterface.keyPress(0xAE, down); break; // VK_VOLUME_DOWN
        case KeyEvent.KEYCODE_MEDIA_NEXT:
            AppStreamInterface.keyPress(0xB0, down); break; // VK_MEDIA_NEXT_TRACK
        case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
            AppStreamInterface.keyPress(0xB1, down); break; // VK_MEDIA_PREV_TRACK
        case KeyEvent.KEYCODE_MEDIA_STOP:
            AppStreamInterface.keyPress(0xB2, down); break; // VK_MEDIA_STOP

        case KeyEvent.KEYCODE_MEDIA_PLAY:
        case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
        case KeyEvent.KEYCODE_MEDIA_PAUSE :
            AppStreamInterface.keyPress(0xB3, down); break; // VK_MEDIA_PLAY_PAUSE

        case KeyEvent.KEYCODE_ESCAPE:
            AppStreamInterface.keyPress(0x1b, down); break;

        case KeyEvent.KEYCODE_BACK:
            return false;  // Let the system handle "Back" so we can exit.

        case KeyEvent.KEYCODE_FORWARD :
            AppStreamInterface.keyPress(0xA7, down); break; // VK_BROWSER_FORWARD

        case KeyEvent.KEYCODE_MINUS:
            AppStreamInterface.keyPress(0xBD, down); break; // VK_OEM_MINUS
        case KeyEvent.KEYCODE_PERIOD:
            AppStreamInterface.keyPress(0xBE, down); break;
        case KeyEvent.KEYCODE_PLUS:
            AppStreamInterface.keyPress(0x6B, down); break; // VK_ADD
        case KeyEvent.KEYCODE_SLASH :
            AppStreamInterface.keyPress(0xBF, down); break; // VK_OEM_2 on US keyboards
        case KeyEvent.KEYCODE_SEMICOLON :
            AppStreamInterface.keyPress(0xBA, down); break; // VK_OEM_1 on US keyboards
        case KeyEvent.KEYCODE_APOSTROPHE :
            AppStreamInterface.keyPress(0xDE, down); break; // VK_OEM_7 on US keyboards
        case KeyEvent.KEYCODE_AT :
            AppStreamInterface.keyPress(VK_SHIFT, down);
            AppStreamInterface.keyPress('2', down);
            break;
        case KeyEvent.KEYCODE_BACKSLASH :
            AppStreamInterface.keyPress(0xDC, down); break; // VK_OEM_5

            // No Windows equivalent
        case KeyEvent.KEYCODE_SYSRQ : // SysRq doesn't seem to fire a WM_KEYDOWN on Windows
        case KeyEvent.KEYCODE_BOOKMARK :
        case KeyEvent.KEYCODE_AVR_INPUT :
        case KeyEvent.KEYCODE_AVR_POWER :
        case KeyEvent.KEYCODE_DVR :
        case KeyEvent.KEYCODE_CALENDAR :
        case KeyEvent.KEYCODE_CAMERA :
        case KeyEvent.KEYCODE_CALL :
        case KeyEvent.KEYCODE_CAPTIONS :
        case KeyEvent.KEYCODE_CHANNEL_DOWN :
        case KeyEvent.KEYCODE_CHANNEL_UP :
        case KeyEvent.KEYCODE_CONTACTS :
        case KeyEvent.KEYCODE_DPAD_CENTER :
        case KeyEvent.KEYCODE_EISU :
        case KeyEvent.KEYCODE_ENDCALL :
        case KeyEvent.KEYCODE_FUNCTION :
        case KeyEvent.KEYCODE_EXPLORER :
        case KeyEvent.KEYCODE_FOCUS :
        case KeyEvent.KEYCODE_GUIDE :
        case KeyEvent.KEYCODE_HEADSETHOOK :
        case KeyEvent.KEYCODE_HENKAN :
        case KeyEvent.KEYCODE_INFO :
        case KeyEvent.KEYCODE_KANA :
        case KeyEvent.KEYCODE_KATAKANA_HIRAGANA :
        case KeyEvent.KEYCODE_LANGUAGE_SWITCH :
        case KeyEvent.KEYCODE_MANNER_MODE :
        case KeyEvent.KEYCODE_MEDIA_CLOSE :
        case KeyEvent.KEYCODE_MEDIA_EJECT :
        case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD :
        case KeyEvent.KEYCODE_MEDIA_RECORD :
        case KeyEvent.KEYCODE_MEDIA_REWIND :
        case KeyEvent.KEYCODE_MUHENKAN :
        case KeyEvent.KEYCODE_MUSIC :
        case KeyEvent.KEYCODE_PROG_BLUE :
        case KeyEvent.KEYCODE_PROG_GREEN :
        case KeyEvent.KEYCODE_PROG_RED :
        case KeyEvent.KEYCODE_PROG_YELLOW :
        case KeyEvent.KEYCODE_POWER :
        case KeyEvent.KEYCODE_PICTSYMBOLS :
        case KeyEvent.KEYCODE_RO :
        case KeyEvent.KEYCODE_SETTINGS :
        case KeyEvent.KEYCODE_NOTIFICATION :
        case KeyEvent.KEYCODE_SOFT_LEFT :
        case KeyEvent.KEYCODE_SOFT_RIGHT :
        case KeyEvent.KEYCODE_STB_INPUT :
        case KeyEvent.KEYCODE_STB_POWER :
        case KeyEvent.KEYCODE_SWITCH_CHARSET :
        case KeyEvent.KEYCODE_TV :
        case KeyEvent.KEYCODE_TV_INPUT :
        case KeyEvent.KEYCODE_TV_POWER :
        case KeyEvent.KEYCODE_UNKNOWN :
        case KeyEvent.KEYCODE_ZOOM_IN :
        case KeyEvent.KEYCODE_ZOOM_OUT :
        case KeyEvent.KEYCODE_ZENKAKU_HANKAKU :
        case KeyEvent.KEYCODE_YEN :
        case KeyEvent.KEYCODE_SYM :
        case KeyEvent.KEYCODE_WINDOW :

        case KeyEvent.KEYCODE_CALCULATOR : // Calculator and search CAN work on Windows if you
        case KeyEvent.KEYCODE_SEARCH :     // set the extended bit, but the key will need to be given
                                           // to the Windows system instead of the app.

        case KeyEvent.KEYCODE_BUTTON_1 :
        case KeyEvent.KEYCODE_BUTTON_10 :
        case KeyEvent.KEYCODE_BUTTON_11 : // Joypad buttons would be handled
        case KeyEvent.KEYCODE_BUTTON_12 : // through another interface
        case KeyEvent.KEYCODE_BUTTON_13 :
        case KeyEvent.KEYCODE_BUTTON_14 :
        case KeyEvent.KEYCODE_BUTTON_15 :
        case KeyEvent.KEYCODE_BUTTON_16 :
        case KeyEvent.KEYCODE_BUTTON_2 :
        case KeyEvent.KEYCODE_BUTTON_3 :
        case KeyEvent.KEYCODE_BUTTON_4 :
        case KeyEvent.KEYCODE_BUTTON_5 :
        case KeyEvent.KEYCODE_BUTTON_6 :
        case KeyEvent.KEYCODE_BUTTON_7 :
        case KeyEvent.KEYCODE_BUTTON_8 :
        case KeyEvent.KEYCODE_BUTTON_9 :
        case KeyEvent.KEYCODE_BUTTON_L2 :
        case KeyEvent.KEYCODE_BUTTON_R2 :
        case KeyEvent.KEYCODE_BUTTON_C :
        case KeyEvent.KEYCODE_BUTTON_MODE :
        case KeyEvent.KEYCODE_BUTTON_SELECT :
        case KeyEvent.KEYCODE_BUTTON_Z :
        case KeyEvent.KEYCODE_BUTTON_START :
        case KeyEvent.KEYCODE_BUTTON_L1 :
        case KeyEvent.KEYCODE_BUTTON_THUMBL :
        case KeyEvent.KEYCODE_BUTTON_R1 :
        case KeyEvent.KEYCODE_BUTTON_THUMBR :
        case KeyEvent.KEYCODE_BUTTON_A :
        case KeyEvent.KEYCODE_BUTTON_B :
        case KeyEvent.KEYCODE_BUTTON_X :
        case KeyEvent.KEYCODE_BUTTON_Y :
        case KeyEvent.KEYCODE_DPAD_RIGHT :
        case KeyEvent.KEYCODE_DPAD_LEFT :
        case KeyEvent.KEYCODE_DPAD_UP :
        case KeyEvent.KEYCODE_DPAD_DOWN :
            Log.v(TAG, "Unknown Key: " + keyCode);
            return false;


        case KeyEvent.KEYCODE_BREAK :
            AppStreamInterface.keyPress(0x13, down); break; // VK_PAUSE (Pause/Break key)
        case KeyEvent.KEYCODE_CAPS_LOCK :
            AppStreamInterface.keyPress(0x14, down); break; // VK_CAPITAL
        case KeyEvent.KEYCODE_CLEAR :
            AppStreamInterface.keyPress(0xFE, down); break; // VK_OEM_CLEAR
        case KeyEvent.KEYCODE_COMMA :
            AppStreamInterface.keyPress(0xBC, down); break; // VK_OEM_COMMA
        case KeyEvent.KEYCODE_ENVELOPE :
            AppStreamInterface.keyPress(0xB4, down); break; // VK_LAUNCH_MAIL

        case KeyEvent.KEYCODE_NUMPAD_EQUALS :
        case KeyEvent.KEYCODE_EQUALS :
            AppStreamInterface.keyPress(0xBB, down); break; // VK_OEM_PLUS, which is '=' / '+'

        case KeyEvent.KEYCODE_GRAVE :
            AppStreamInterface.keyPress(0xC0, down); break; // VK_OEM_3
        case KeyEvent.KEYCODE_MOVE_HOME :
            AppStreamInterface.keyPress(0x24, down); break; // VK_HOME
        case KeyEvent.KEYCODE_MOVE_END :
            AppStreamInterface.keyPress(0x23, down); break; // VK_END
        case KeyEvent.KEYCODE_INSERT :
            AppStreamInterface.keyPress(0x2D, down); break; // VK_INSERT
        case KeyEvent.KEYCODE_LEFT_BRACKET :
            AppStreamInterface.keyPress(0xDB, down); break; // VK_OEM_4 on US keyboards
        case KeyEvent.KEYCODE_RIGHT_BRACKET :
            AppStreamInterface.keyPress(0xDD, down); break; // VK_OEM_6 on US keyboards
        case KeyEvent.KEYCODE_HOME :
        case KeyEvent.KEYCODE_MENU :
            AppStreamInterface.keyPress(0x5D, down); break; // VK_APPS

        case KeyEvent.KEYCODE_NUMPAD_0 :
        case KeyEvent.KEYCODE_NUMPAD_1 :
        case KeyEvent.KEYCODE_NUMPAD_2 :
        case KeyEvent.KEYCODE_NUMPAD_3 :
        case KeyEvent.KEYCODE_NUMPAD_4 :
        case KeyEvent.KEYCODE_NUMPAD_5 :
        case KeyEvent.KEYCODE_NUMPAD_6 :
        case KeyEvent.KEYCODE_NUMPAD_7 :
        case KeyEvent.KEYCODE_NUMPAD_8 :
        case KeyEvent.KEYCODE_NUMPAD_9 :
            AppStreamInterface.keyPress(keyCode - KeyEvent.KEYCODE_NUMPAD_0 + 0x60, down); break;

        case KeyEvent.KEYCODE_NUMPAD_ADD :
            AppStreamInterface.keyPress(0x6B, down); break; // VK_ADD

        case KeyEvent.KEYCODE_MUTE :
            AppStreamInterface.keyPress(0xAD, down); break; // VK_VOLUME_MUTE

        case KeyEvent.KEYCODE_NUMPAD_COMMA :
            AppStreamInterface.keyPress(0xBC, down); break; // VK_OEM_COMMA
        case KeyEvent.KEYCODE_NUMPAD_DIVIDE :
            AppStreamInterface.keyPress(0x6F, down); break; // VK_DIVIDE
        case KeyEvent.KEYCODE_NUMPAD_DOT :
            AppStreamInterface.keyPress(0x6E, down); break; // VK_DECIMAL
        case KeyEvent.KEYCODE_NUMPAD_ENTER :
            AppStreamInterface.keyPress(0x0D, down); break; // VK_RETURN
        case KeyEvent.KEYCODE_STAR :
        case KeyEvent.KEYCODE_NUMPAD_MULTIPLY :
            AppStreamInterface.keyPress(0x6A, down); break; // VK_MULTIPLY
        case KeyEvent.KEYCODE_NUMPAD_SUBTRACT :
            AppStreamInterface.keyPress(0x6D, down); break; // VK_SUBTRACT

        case KeyEvent.KEYCODE_NUM_LOCK :
            AppStreamInterface.keyPress(0x90, down); break; // VK_NUMLOCK

        case KeyEvent.KEYCODE_POUND :
            AppStreamInterface.keyPress(VK_SHIFT, down);
            AppStreamInterface.keyPress('3', down); break; // US keyboard

        case KeyEvent.KEYCODE_SCROLL_LOCK :
            AppStreamInterface.keyPress(0x91, down); break; // VK_SCROLL

            // I have a Windows keyboard that has a left and right paren
            // on the keypad, and it doesn't produce a WM_KEYDOWN message
            // at all. (!!)
        case KeyEvent.KEYCODE_NUMPAD_LEFT_PAREN :
            AppStreamInterface.keyPress(VK_SHIFT, down);
            AppStreamInterface.keyPress('9', down); break;

        case KeyEvent.KEYCODE_NUMPAD_RIGHT_PAREN :
            AppStreamInterface.keyPress(VK_SHIFT, down);
            AppStreamInterface.keyPress('0', down);  break;

        case KeyEvent.KEYCODE_PAGE_DOWN :
            AppStreamInterface.keyPress(0x22, down); break; // VK_NEXT
        case KeyEvent.KEYCODE_PAGE_UP :
            AppStreamInterface.keyPress(0x21, down); break; // VK_PRIOR

        case KeyEvent.KEYCODE_SPACE :
            AppStreamInterface.keyPress(0x20, down); break; // VK_SPACE
        case KeyEvent.KEYCODE_TAB :
            AppStreamInterface.keyPress(0x09, down); break; // VK_TAB

        case KeyEvent.KEYCODE_F1 :
        case KeyEvent.KEYCODE_F2 :
        case KeyEvent.KEYCODE_F3 :
        case KeyEvent.KEYCODE_F4 :
        case KeyEvent.KEYCODE_F5 :
        case KeyEvent.KEYCODE_F6 :
        case KeyEvent.KEYCODE_F7 :
        case KeyEvent.KEYCODE_F8 :
        case KeyEvent.KEYCODE_F9 :
        case KeyEvent.KEYCODE_F10 :
        case KeyEvent.KEYCODE_F11 :
        case KeyEvent.KEYCODE_F12 :
            AppStreamInterface.keyPress(keyCode - KeyEvent.KEYCODE_F1 + 0x70, down); break;

        default:
            {
                String key = new String(Character.toString((char)msg.getUnicodeChar()));

                keyCode = key.toUpperCase(Locale.US).codePointAt(0);
                AppStreamInterface.keyPress(keyCode, down);
            }
        }
        return true;
    }

}
