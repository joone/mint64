/**
 *  file    Main.c
 *  date    2009/01/09
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   키보드 디바이스 드라이버에 관련된 소스 파일
 */

#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"

////////////////////////////////////////////////////////////////////////////////
//
// 키보드 컨트롤러 및 키보드 제어에 관련된 함수들
//
////////////////////////////////////////////////////////////////////////////////
/**
 *  출력 버퍼(포트 0x60)에 수신된 데이터가 있는지 여부를 반환
 */
BOOL kIsOutputBufferFull( void )
{
    // 상태 레지스터(포트 0x64)에서 읽은 값에 출력 버퍼 상태 비트(비트 0)가
    // 1로 설정되어 있으면 출력 버퍼에 키보드가 전송한 데이터가 존재함
    if( kInPortByte( 0x64 ) & 0x01 )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * 입력 버퍼(포트 0x60)에 프로세서가 쓴 데이터가 남아있는지 여부를 반환
 */
BOOL kIsInputBufferFull( void )
{
    // 상태 레지스터(포트 0x64)에서 읽은 값에 입력 버퍼 상태 비트(비트 1)가
    // 1로 설정되어 있으면 아직 키보드가 데이터를 가져가지 않았음
    if( kInPortByte( 0x64 ) & 0x02 )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  키보드를 활성화
 */
BOOL kActivateKeyboard( void )
{
    int i;
    int j;

    // 컨트롤 레지스터(포트 0x64)에 키보드 활성화 커맨드(0xAE)를 전달하여
    // 키보드 디바이스 활성화
    kOutPortByte( 0x64, 0xAE );

    // 입력 버퍼(포트 0x60)가 빌 때까지 기다렸다가 키보드에 활성화 커맨드를 전송
    // 0xFFFF만큼 루프를 수행할 시간이면 충분히 커맨드가 전송될 수 있음
    // 0xFFFF 루프를 수행한 이후에도 입력 버퍼(포트 0x60)가 비지 않으면 무시하고 전송
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 입력 버퍼(포트 0x60)가 비어있으면 키보드 커맨드 전송 가능
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }
    // 입력 버퍼(포트 0x60)로 키보드 활성화(0xF4) 커맨드를 전달하여 키보드로 전송
    kOutPortByte( 0x60, 0xF4 );

    // ACK가 올 때까지 대기함
    // ACK가 오기 전에 키보드 출력 버퍼(포트 0x60)에 키 데이터가 저장될 수 있으므로
    // 키보드에서 전달된 데이터를 최대 100개까지 수신하여 ACK를 확인
    for( j = 0 ; j < 100 ; j++ )
    {
        // 0xFFFF만큼 루프를 수행할 시간이면 충분히 커맨드의 응답이 올 수 있음
        // 0xFFFF 루프를 수행한 이후에도 출력 버퍼(포트 0x60)가 차 있지 않으면 무시하고 읽음
        for( i = 0 ; i < 0xFFFF ; i++ )
        {
            // 출력 버퍼(포트 0x60)가 차있으면 데이터를 읽을 수 있음
            if( kIsOutputBufferFull() == TRUE )
            {
                break;
            }
        }

        // 출력 버퍼(포트 0x60)에서 읽은 데이터가 ACK(0xFA)이면 성공
        if( kInPortByte( 0x60 ) == 0xFA )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 *  출력 버퍼(포트 0x60)에서 키를 읽음
 */
BYTE kGetKeyboardScanCode( void )
{
    // 출력 버퍼(포트 0x60)에 데이터가 있을 때까지 대기
    while( kIsOutputBufferFull() == FALSE )
    {
        ;
    }
    return kInPortByte( 0x60 );
}

/**
 *  키보드 LED의 ON/OFF를 변경
 */
BOOL kChangeKeyboardLED( BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn )
{
    int i, j;

    // 키보드에 LED 변경 커맨드 전송하고 커맨드가 처리될 때까지 대기
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 출력 버퍼(포트 0x60)가 비었으면 커맨드 전송 가능
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }

    // 출력 버퍼(포트 0x60)로 LED 상태 변경 커맨드(0xED) 전송
    kOutPortByte( 0x60, 0xED );
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 입력 버퍼(포트 0x60)가 비어있으면 키보드가 커맨드를 가져간 것임
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }

    // 키보드가 LED 상태 변경 커맨드를 가져갔으므로 ACK가 올때까지 대기
    for( j = 0 ; j < 100 ; j++ )
    {
        for( i = 0 ; i < 0xFFFF ; i++ )
        {
            // 출력 버퍼(포트 0x60)가 차있으면 데이터를 읽을 수 있음
            if( kIsOutputBufferFull() == TRUE )
            {
                break;
            }
        }

        // 출력 버퍼(포트 0x60)에서 읽은 데이터가 ACK(0xFA)이면 성공
        if( kInPortByte( 0x60 ) == 0xFA )
        {
            break;
        }
    }
    if( j >= 100 )
    {
        return FALSE;
    }

    // LED 변경 값을 키보드로 전송하고 데이터가 처리가 완료될 때까지 대기
    kOutPortByte( 0x60, ( bCapsLockOn << 2 ) | ( bNumLockOn << 1 ) | bScrollLockOn );
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 입력 버퍼(포트 0x60)가 비어있으면 키보드가 LED 데이터를 가져간 것임
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }

    // 키보드가 LED 데이터를 가져갔으므로 ACK가 올 때까지 대기함
    for( j = 0 ; j < 100 ; j++ )
    {
        for( i = 0 ; i < 0xFFFF ; i++ )
        {
            // 출력 버퍼(포트 0x60)가 차있으면 데이터를 읽을 수 있음
            if( kIsOutputBufferFull() == TRUE )
            {
                break;
            }
        }

        // 출력 버퍼(포트 0x60)에서 읽은 데이터가 ACK(0xFA)이면 성공
        if( kInPortByte( 0x60 ) == 0xFA )
        {
            break;
        }
    }
    if( j >= 100 )
    {
        return FALSE;
    }

    return TRUE;
}

/**
 *  A20 게이트를 활성화
 */
void kEnableA20Gate( void )
{
    BYTE bOutputPortData;
    int i;

    // 컨트롤 레지스터(포트 0x64)에 키보드 컨트롤러의 출력 포트 값을 읽는 커맨드(0xD0) 전송
    kOutPortByte( 0x64, 0xD0 );

    // 출력 포트의 데이터를 기다렸다가 읽음
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 출력 버퍼(포트 0x60)가 차있으면 데이터를 읽을 수 있음
        if( kIsOutputBufferFull() == TRUE )
        {
            break;
        }
    }
    // 출력 포트(포트 0x60)에 수신된 키보드 컨트롤러의 출력 포트 값을 읽음
    bOutputPortData = kInPortByte( 0x60 );

    // A20 게이트 비트 설정
    bOutputPortData |= 0x01;

    // 입력 버퍼(포트 0x60)에 데이터가 비어있으면 출력 포트에 값을 쓰는 커맨드와 출력 포트 데이터 전송
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 입력 버퍼(포트 0x60)가 비었으면 커맨드 전송 가능
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }

    // 커맨드 레지스터(0x64)에 출력 포트 설정 커맨드(0xD1)을 전달
    kOutPortByte( 0x64, 0xD1 );

    // 입력 버퍼(0x60)에 A20 게이트 비트가 1로 설정된 값을 전달
    kOutPortByte( 0x60, bOutputPortData );
}

/**
 *  프로세서를 리셋(Reset)
 */
void kReboot( void )
{
    int i;

    // 입력 버퍼(포트 0x60)에 데이터가 비어있으면 출력 포트에 값을 쓰는 커맨드와 출력 포트 데이터 전송
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // 입력 버퍼(포트 0x60)가 비었으면 커맨드 전송 가능
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }

    // 커맨드 레지스터(0x64)에 출력 포트 설정 커맨드(0xD1)을 전달
    kOutPortByte( 0x64, 0xD1 );

    // 입력 버퍼(0x60)에 0을 전달하여 프로세서를 리셋(Reset)함
    kOutPortByte( 0x60, 0x00 );

    while( 1 )
    {
        ;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// 스캔 코드를 ASCII 코드로 변환하는 기능에 관련된 함수들
//
////////////////////////////////////////////////////////////////////////////////
// 키보드 상태를 관리하는 키보드 매니저
static KEYBOARDMANAGER gs_stKeyboardManager = { 0, };

// 스캔 코드를 ASCII 코드로 변환하는 테이블
static KEYMAPPINGENTRY gs_vstKeyMappingTable[ KEY_MAPPINGTABLEMAXCOUNT ] =
{
    /*  0   */  {   KEY_NONE        ,   KEY_NONE        },
    /*  1   */  {   KEY_ESC         ,   KEY_ESC         },
    /*  2   */  {   '1'             ,   '!'             },
    /*  3   */  {   '2'             ,   '@'             },
    /*  4   */  {   '3'             ,   '#'             },
    /*  5   */  {   '4'             ,   '$'             },
    /*  6   */  {   '5'             ,   '%'             },
    /*  7   */  {   '6'             ,   '^'             },
    /*  8   */  {   '7'             ,   '&'             },
    /*  9   */  {   '8'             ,   '*'             },
    /*  10  */  {   '9'             ,   '('             },
    /*  11  */  {   '0'             ,   ')'             },
    /*  12  */  {   '-'             ,   '_'             },
    /*  13  */  {   '='             ,   '+'             },
    /*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
    /*  15  */  {   KEY_TAB         ,   KEY_TAB         },
    /*  16  */  {   'q'             ,   'Q'             },
    /*  17  */  {   'w'             ,   'W'             },
    /*  18  */  {   'e'             ,   'E'             },
    /*  19  */  {   'r'             ,   'R'             },
    /*  20  */  {   't'             ,   'T'             },
    /*  21  */  {   'y'             ,   'Y'             },
    /*  22  */  {   'u'             ,   'U'             },
    /*  23  */  {   'i'             ,   'I'             },
    /*  24  */  {   'o'             ,   'O'             },
    /*  25  */  {   'p'             ,   'P'             },
    /*  26  */  {   '['             ,   '{'             },
    /*  27  */  {   ']'             ,   '}'             },
    /*  28  */  {   '\n'            ,   '\n'            },
    /*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
    /*  30  */  {   'a'             ,   'A'             },
    /*  31  */  {   's'             ,   'S'             },
    /*  32  */  {   'd'             ,   'D'             },
    /*  33  */  {   'f'             ,   'F'             },
    /*  34  */  {   'g'             ,   'G'             },
    /*  35  */  {   'h'             ,   'H'             },
    /*  36  */  {   'j'             ,   'J'             },
    /*  37  */  {   'k'             ,   'K'             },
    /*  38  */  {   'l'             ,   'L'             },
    /*  39  */  {   ';'             ,   ':'             },
    /*  40  */  {   '\''            ,   '\"'            },
    /*  41  */  {   '`'             ,   '~'             },
    /*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
    /*  43  */  {   '\\'            ,   '|'             },
    /*  44  */  {   'z'             ,   'Z'             },
    /*  45  */  {   'x'             ,   'X'             },
    /*  46  */  {   'c'             ,   'C'             },
    /*  47  */  {   'v'             ,   'V'             },
    /*  48  */  {   'b'             ,   'B'             },
    /*  49  */  {   'n'             ,   'N'             },
    /*  50  */  {   'm'             ,   'M'             },
    /*  51  */  {   ','             ,   '<'             },
    /*  52  */  {   '.'             ,   '>'             },
    /*  53  */  {   '/'             ,   '?'             },
    /*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
    /*  55  */  {   '*'             ,   '*'             },
    /*  56  */  {   KEY_LALT        ,   KEY_LALT        },
    /*  57  */  {   ' '             ,   ' '             },
    /*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
    /*  59  */  {   KEY_F1          ,   KEY_F1          },
    /*  60  */  {   KEY_F2          ,   KEY_F2          },
    /*  61  */  {   KEY_F3          ,   KEY_F3          },
    /*  62  */  {   KEY_F4          ,   KEY_F4          },
    /*  63  */  {   KEY_F5          ,   KEY_F5          },
    /*  64  */  {   KEY_F6          ,   KEY_F6          },
    /*  65  */  {   KEY_F7          ,   KEY_F7          },
    /*  66  */  {   KEY_F8          ,   KEY_F8          },
    /*  67  */  {   KEY_F9          ,   KEY_F9          },
    /*  68  */  {   KEY_F10         ,   KEY_F10         },
    /*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
    /*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },

    /*  71  */  {   KEY_HOME        ,   '7'             },
    /*  72  */  {   KEY_UP          ,   '8'             },
    /*  73  */  {   KEY_PAGEUP      ,   '9'             },
    /*  74  */  {   '-'             ,   '-'             },
    /*  75  */  {   KEY_LEFT        ,   '4'             },
    /*  76  */  {   KEY_CENTER      ,   '5'             },
    /*  77  */  {   KEY_RIGHT       ,   '6'             },
    /*  78  */  {   '+'             ,   '+'             },
    /*  79  */  {   KEY_END         ,   '1'             },
    /*  80  */  {   KEY_DOWN        ,   '2'             },
    /*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
    /*  82  */  {   KEY_INS         ,   '0'             },
    /*  83  */  {   KEY_DEL         ,   '.'             },
    /*  84  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  85  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  86  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  87  */  {   KEY_F11         ,   KEY_F11         },
    /*  88  */  {   KEY_F12         ,   KEY_F12         }
};

/**
 *  스캔 코드가 알파벳 범위인지 여부를 반환
 */
BOOL kIsAlphabetScanCode( BYTE bScanCode )
{
    // 변환 테이블을 값을 직접 읽어서 알파벳 범위인지 확인
    if( ( 'a' <= gs_vstKeyMappingTable[ bScanCode ].bNormalCode ) &&
        ( gs_vstKeyMappingTable[ bScanCode ].bNormalCode <= 'z' ) )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  숫자 또는 기호 범위인지 여부를 반환
 */
BOOL kIsNumberOrSymbolScanCode( BYTE bScanCode )
{
    // 숫자 패드나 확장 키 범위를 제외한 범위(스캔 코드 2~53)에서 영문자가 아니면
    // 숫자 또는 기호임
    if( ( 2 <= bScanCode ) && ( bScanCode <= 53 ) &&
        ( kIsAlphabetScanCode( bScanCode ) == FALSE ) )
    {
        return TRUE;
    }

    return FALSE;
}

/**
 * 숫자 패드 범위인지 여부를 반환
 */
BOOL kIsNumberPadScanCode( BYTE bScanCode )
{
    // 숫자 패드는 스캔 코드의 71~83에 있음
    if( ( 71 <= bScanCode ) && ( bScanCode <= 83 ) )
    {
        return TRUE;
    }

    return FALSE;
}

/**
 *  조합된 키 값을 사용해야 하는지 여부를 반환
 */
BOOL kIsUseCombinedCode( BOOL bScanCode )
{
    BYTE bDownScanCode;
    BOOL bUseCombinedKey;

    bDownScanCode = bScanCode & 0x7F;

    // 알파벳 키라면 Shift 키와 Caps Lock의 영향을 받음
    if( kIsAlphabetScanCode( bDownScanCode ) == TRUE )
    {
        // 만약 Shift 키와 Caps Lock 키 중에 하나만 눌러져있으면 조합된 키를 되돌려 줌
        if( gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn )
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    // 숫자와 기호 키라면 Shift 키의 영향을 받음
    else if( kIsNumberOrSymbolScanCode( bDownScanCode ) == TRUE )
    {
        // Shift 키가 눌러져있으면 조합된 키를 되돌려 줌
        if( gs_stKeyboardManager.bShiftDown == TRUE )
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    // 숫자 패드 키라면 Num Lock 키의 영향을 받음
    // 0xE0만 제외하면 확장 키 코드와 숫자 패드의 코드가 겹치므로,
    // 확장 키 코드가 수신되지 않았을 때만처리 조합된 코드 사용
    else if( ( kIsNumberPadScanCode( bDownScanCode ) == TRUE ) &&
             ( gs_stKeyboardManager.bExtendedCodeIn == FALSE ) )
    {
        // Num Lock 키가 눌러져있으면, 조합된 키를 되돌려 줌
        if( gs_stKeyboardManager.bNumLockOn == TRUE )
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }

    return bUseCombinedKey;
}

/**
 *  조합 키의 상태를 갱신하고 LED 상태도 동기화 함
 */
void UpdateCombinationKeyStatusAndLED( BYTE bScanCode )
{
    BOOL bDown;
    BYTE bDownScanCode;
    BOOL bLEDStatusChanged = FALSE;

    // 눌림 또는 떨어짐 상태처리, 최상위 비트(비트 7)가 1이면 키가 떨어졌음을 의미하고
    // 0이면 떨어졌음을 의미함
    if( bScanCode & 0x80 )
    {
        bDown = FALSE;
        bDownScanCode = bScanCode & 0x7F;
    }
    else
    {
        bDown = TRUE;
        bDownScanCode = bScanCode;
    }

    // 조합 키 검색
    // Shift 키의 스캔 코드(42 or 54)이면 Shift 키의 상태 갱신
    if( ( bDownScanCode == 42 ) || ( bDownScanCode == 54 ) )
    {
        gs_stKeyboardManager.bShiftDown = bDown;
    }
    // Caps Lock 키의 스캔 코드(58)이면 Caps Lock의 상태 갱신하고 LED 상태 변경
    else if( ( bDownScanCode == 58 ) && ( bDown == TRUE ) )
    {
        gs_stKeyboardManager.bCapsLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    // Num Lock 키의 스캔 코드(69)이면 Num Lock의 상태를 갱신하고 LED 상태 변경
    else if( ( bDownScanCode == 69 ) && ( bDown == TRUE ) )
    {
        gs_stKeyboardManager.bNumLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    // Scroll Lock 키의 스캔 코드(70)이면 Scroll Lock의 상태를 갱신하고 LED 상태 변경
    else if( ( bDownScanCode == 70 ) && ( bDown == TRUE ) )
    {
        gs_stKeyboardManager.bScrollLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }

    // LED 상태가 변했으면 키보드로 커맨드를 전송하여 LED를 변경
    if( bLEDStatusChanged == TRUE )
    {
        kChangeKeyboardLED( gs_stKeyboardManager.bCapsLockOn,
            gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn );
    }
}

/**
 *  스캔 코드를 ASCII 코드로 변환
 */
BOOL kConvertScanCodeToASCIICode( BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags )
{
    BOOL bUseCombinedKey;

    // 이전에 Pause 키가 수신되었다면, Pause의 남은 스캔 코드를 무시
    if( gs_stKeyboardManager.iSkipCountForPause > 0 )
    {
        gs_stKeyboardManager.iSkipCountForPause--;
        return FALSE;
    }

    // Pause 키는 특별히 처리
    if( bScanCode == 0xE1 )
    {
        *pbASCIICode = KEY_PAUSE;
        *pbFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return TRUE;
    }
    // 확장 키 코드가 들어왔을 때, 실제 키 값은 다음에 들어오므로 플래그 설정만 하고 종료
    else if( bScanCode == 0xE0 )
    {
        gs_stKeyboardManager.bExtendedCodeIn = TRUE;
        return FALSE;
    }

    // 조합된 키를 반환해야 하는가?
    bUseCombinedKey = kIsUseCombinedCode( bScanCode );

    // 키 값 설정
    if( bUseCombinedKey == TRUE )
    {
        *pbASCIICode = gs_vstKeyMappingTable[ bScanCode & 0x7F ].bCombinedCode;
    }
    else
    {
        *pbASCIICode = gs_vstKeyMappingTable[ bScanCode & 0x7F ].bNormalCode;
    }

    // 확장 키 유무 설정
    if( gs_stKeyboardManager.bExtendedCodeIn == TRUE )
    {
        *pbFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = FALSE;
    }
    else
    {
        *pbFlags = 0;
    }

    // 눌러짐 또는 떨어짐 유무 설정
    if( ( bScanCode & 0x80 ) == 0 )
    {
        *pbFlags |= KEY_FLAGS_DOWN;
    }

    // 조합 키 눌림 또는 떨어짐 상태를 갱신
    UpdateCombinationKeyStatusAndLED( bScanCode );
    return TRUE;
}
