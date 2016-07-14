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


#include "AppleTextHelper.h"


bool appleGetVirtualKeyUsingChar (uint16_t &keyChar, uint16_t &commandKeysBitmask, uint16_t &commandKeysIgnored)
{
    //Default to not needing any command keys and ignoring the state of all
    // except the shift key
    commandKeysBitmask = 0;
    commandKeysIgnored = ~STX_SHIFT_KEY_MASK;
    
    BOOL mappedKey = false;
    switch (keyChar)
    {
        case NSEnterCharacter:
        case NSFormFeedCharacter:
        case NSNewlineCharacter:
        case NSCarriageReturnCharacter:
        case NSLineSeparatorCharacter:
        case NSParagraphSeparatorCharacter:
            mappedKey = true;
            commandKeysIgnored |= STX_SHIFT_KEY_MASK;
            keyChar = VK_RETURN;
            break;
            
        case NSBackspaceCharacter:
            mappedKey = true;
            keyChar = VK_BACK;
            break;
            
        case NSTabCharacter:
            mappedKey = true;
            keyChar = VK_TAB;
            break;
            
        case NSBackTabCharacter: //SHIFT-TAB
            mappedKey = true;
            commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            keyChar = VK_TAB;
            break;
            
        case NSDeleteCharacter:
            mappedKey = true;
            keyChar = VK_BACK;
            break;
            
        case NSUpArrowFunctionKey:
            mappedKey = true;
            commandKeysIgnored |= STX_SHIFT_KEY_MASK;
            keyChar = VK_UP;
            break;
            
        case NSDownArrowFunctionKey:
            mappedKey = true;
            commandKeysIgnored |= STX_SHIFT_KEY_MASK;
            keyChar = VK_DOWN;
            break;
            
        case NSLeftArrowFunctionKey:
            mappedKey = true;
            commandKeysIgnored |= STX_SHIFT_KEY_MASK;
            keyChar = VK_LEFT;
            break;
            
        case NSRightArrowFunctionKey:
            mappedKey = true;
            commandKeysIgnored |= STX_SHIFT_KEY_MASK;
            keyChar = VK_RIGHT;
            break;
            
        case NSF1FunctionKey:
            mappedKey = true;
            keyChar = VK_F1;
            break;
            
        case NSF2FunctionKey:
            mappedKey = true;
            keyChar = VK_F2;
            break;
            
        case NSF3FunctionKey:
            mappedKey = true;
            keyChar = VK_F3;
            break;
            
        case NSF4FunctionKey:
            mappedKey = true;
            keyChar = VK_F4;
            break;
            
        case NSF5FunctionKey:
            mappedKey = true;
            keyChar = VK_F5;
            break;
            
        case NSF6FunctionKey:
            mappedKey = true;
            keyChar = VK_F6;
            break;
            
        case NSF7FunctionKey:
            mappedKey = true;
            keyChar = VK_F7;
            break;
            
        case NSF8FunctionKey:
            mappedKey = true;
            keyChar = VK_F8;
            break;
            
        case NSF9FunctionKey:
            mappedKey = true;
            keyChar = VK_F9;
            break;
            
        case NSF10FunctionKey:
            mappedKey = true;
            keyChar = VK_F10;
            break;
            
        case NSF11FunctionKey:
            mappedKey = true;
            keyChar = VK_F11;
            break;
            
        case NSF12FunctionKey:
            mappedKey = true;
            keyChar = VK_F12;
            break;
            
        case NSF13FunctionKey:
            mappedKey = true;
            keyChar = VK_F13;
            break;
            
        case NSF14FunctionKey:
            mappedKey = true;
            keyChar = VK_F14;
            break;
            
        case NSF15FunctionKey:
            mappedKey = true;
            keyChar = VK_F15;
            break;
            
        case NSF16FunctionKey:
            mappedKey = true;
            keyChar = VK_F16;
            break;
            
        case NSF17FunctionKey:
            mappedKey = true;
            keyChar = VK_F17;
            break;
            
        case NSF18FunctionKey:
            mappedKey = true;
            keyChar = VK_F18;
            break;
            
        case NSF19FunctionKey:
            mappedKey = true;
            keyChar = VK_F19;
            break;
            
        case NSF20FunctionKey:
            mappedKey = true;
            keyChar = VK_F20;
            break;
            
        case NSF21FunctionKey:
            mappedKey = true;
            keyChar = VK_F21;
            break;
            
        case NSF22FunctionKey:
            mappedKey = true;
            keyChar = VK_F22;
            break;
            
        case NSF23FunctionKey:
            mappedKey = true;
            keyChar = VK_F23;
            break;
            
        case NSF24FunctionKey:
            mappedKey = true;
            keyChar = VK_F24;
            break;
            
            //No keys available to map F25-F35
            //            NSF25FunctionKey            = 0xF71C,
            //            NSF26FunctionKey            = 0xF71D,
            //            NSF27FunctionKey            = 0xF71E,
            //            NSF28FunctionKey            = 0xF71F,
            //            NSF29FunctionKey            = 0xF720,
            //            NSF30FunctionKey            = 0xF721,
            //            NSF31FunctionKey            = 0xF722,
            //            NSF32FunctionKey            = 0xF723,
            //            NSF33FunctionKey            = 0xF724,
            //            NSF34FunctionKey            = 0xF725,
            //            NSF35FunctionKey            = 0xF726,
        case NSInsertFunctionKey:
            mappedKey = true;
            keyChar = VK_INSERT;
            break;
            
        case NSDeleteFunctionKey:
            mappedKey = true;
            keyChar = VK_DELETE;
            break;
            
        case NSHomeFunctionKey:
            mappedKey = true;
            keyChar = VK_HOME;
            break;
            
        case NSEndFunctionKey:
            mappedKey = true;
            keyChar = VK_END;
            break;
            
        case NSPageUpFunctionKey:
            mappedKey = true;
            keyChar = VK_PRIOR;
            break;
            
        case NSPageDownFunctionKey:
            mappedKey = true;
            keyChar = VK_NEXT;
            break;
            
        case NSPrintScreenFunctionKey:
            mappedKey = true;
            keyChar = VK_SNAPSHOT;
            break;
            
        case NSScrollLockFunctionKey:
            mappedKey = true;
            keyChar = VK_SCROLL;
            break;
            
        case NSPauseFunctionKey:
            mappedKey = true;
            keyChar = VK_PAUSE;
            break;
            
        case NSMenuFunctionKey:
            mappedKey = true;
            keyChar = VK_MENU;
            break;
            
        case NSPrintFunctionKey:
            mappedKey = true;
            keyChar = VK_PRINT;
            break;
            
        case NSHelpFunctionKey:
            mappedKey = true;
            keyChar = VK_HELP;
            break;
            
        case NSSysReqFunctionKey:
            mappedKey = true;
            keyChar = VK_PRINT;
            commandKeysBitmask &= ~STX_ALT_KEY_MASK;
            break;
            
        case 0x1B: //Escape Key, does not have an Apple Define
            mappedKey = true;
            keyChar = VK_ESCAPE;
            break;
            
            
            
        case NSBeginFunctionKey:
        case NSBreakFunctionKey:
        case NSResetFunctionKey:
        case NSStopFunctionKey:
        case NSUserFunctionKey:
        case NSSystemFunctionKey:
        case NSClearLineFunctionKey:
        case NSClearDisplayFunctionKey:
        case NSInsertLineFunctionKey:
        case NSDeleteLineFunctionKey:
        case NSInsertCharFunctionKey:
        case NSDeleteCharFunctionKey:
        case NSPrevFunctionKey:
        case NSNextFunctionKey:
        case NSSelectFunctionKey:
        case NSExecuteFunctionKey:
        case NSUndoFunctionKey:
        case NSRedoFunctionKey:
        case NSFindFunctionKey:
        case NSModeSwitchFunctionKey:
#warning No mapping available for these keys
            NSLog(@"Received unmappable key");
            break;
        default:
            break;
    }
    
    return mappedKey;
}
