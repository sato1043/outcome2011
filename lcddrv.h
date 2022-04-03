//-----------------------------------------------------------------------
// キャラクター型デバイスドライバ
// lcddrv.h  for CAT709, CAT760
//
// for linux-2.6 kernel          written by S.Shintani
// mod by T.Sato Jan 07, 2010
//-----------------------------------------------------------------------
#ifndef __lcddrv_h__
#define __lcddrv_h__

#define LCD_CLEAR   (0) /* 画面のクリア                       */
#define LCD_HOME    (1) /* カーソルをホームポジションへ移動   */
#define LCD_DISPLAY (2) /* 表示のON/OFF（バッファは消さない） */
#define LCD_CURSOR  (3) /* カーソル表示のON/OFF               */
#define LCD_BLINK   (4) /* カーソルブリンクのON/OFF           */
#define LCD_LOCATE  (5) /* カーソルポジションの移動 0=left 1=right*/
#define LCD_RESET   (6) /* LCDコントローラのリセット          */
#define LCD_SHIFT   (7) /* LCD表示のシフト 0=left 1=right*/

#define LED_RED     (10) /* 赤色LED(PTC7) 0=OFF 1=ON */

#define LCDPOS(x,y) ((x<<8)|y) /* カーソルポジション計算マクロ */

#endif/*__lcddrv_h__*/
