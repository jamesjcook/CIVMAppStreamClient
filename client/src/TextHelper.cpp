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


#include "TextHelper.h"

#undef LOG_TAG
#define LOG_TAG "TextHelper"
#include "log.h"


bool getVirtualKeyUsingChar (uint16_t &keyChar, uint16_t &commandKeysBitmask, uint16_t &commandKeysIgnored)
{
    //Default to not needing any commandKeysPressed
    commandKeysBitmask = 0;
    //Default to ignoring all the commandKey states except shift
    commandKeysIgnored = ~STX_SHIFT_KEY_MASK;
    
    //Lower-case a-z needs to be converted to upper-case
    // it can then pass through directly
    if (keyChar >= 'a' && keyChar <= 'z') {
        keyChar = toupper(keyChar);
    } else if (keyChar >= 'A' && keyChar <= 'Z')
    {
        //Make sure we pass the shift as well
        commandKeysBitmask |= STX_SHIFT_KEY_MASK;
    } else if (keyChar >= '0' && keyChar <= '9')
    {
        //Numbers can pass straight through
    } else
    {
        switch (keyChar) {
            case '<':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case ',':
                keyChar = VK_OEM_COMMA;
                break;
                
            case '>':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '.':
                keyChar = VK_OEM_PERIOD;
                break;
                
            case '_':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '-':
                keyChar = VK_OEM_MINUS;
                break;
                
            case '+':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '=':
                keyChar = VK_OEM_PLUS;
                break;
                
            case ':':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case ';':
                keyChar = VK_OEM_1;
                break;
                
            case '?':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '/':
                keyChar = VK_OEM_2;
                break;
                
            case '~':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '`':
                keyChar = VK_OEM_3;
                break;
                
            case '{':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '[':
                keyChar = VK_OEM_4;
                break;
                
            case '|':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '\\':
                keyChar = VK_OEM_5;
                break;
                
            case '}':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case ']':
                keyChar = VK_OEM_6;
                break;
                
            case '"':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
            case '\'':
                keyChar = VK_OEM_7;
                break;
                
                
            case '!':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_1;
                break;
                
            case '@':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_2;
                break;
                
            case '#':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_3;
                break;
                
            case '$':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_4;
                break;
                
            case '%':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_5;
                break;
                
            case '^':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_6;
                break;
                
            case '&':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_7;
                break;
                
            case '*':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_8;
                break;
                
            case '(':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_9;
                break;
                
            case ')':
                commandKeysBitmask |= STX_SHIFT_KEY_MASK;
                keyChar = kKEY_0;
                break;
                
            case ' ':
                commandKeysIgnored |= STX_SHIFT_KEY_MASK;
                keyChar = VK_SPACE;
                break;
                
            default:
                LOGV("Unmapped Key: %hu", keyChar);
                return false;
                break;
        }
    }
    
    return true;
}

