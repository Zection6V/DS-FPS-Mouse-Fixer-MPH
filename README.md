# DS-FPS-Mouse-Fixer-MPH

MPH Optimized version of [DS-FPS-Mouse-Fixer](https://github.com/JDoe212/DS-FPS-Mouse-Fixer)

I made some improvements based on the excellent "DS FPS Mouse Fix" developed by JDoe212. It was only possible to achieve further optimization thanks to the outstanding "DS FPS Mouse Fix" that JDoe212 created. I am sincerely grateful.  
JDoe212が開発した素晴らしい「DS FPS Mouse Fix」をベースに改良を行いました。このツールがあったからこそ、さらなる最適化が可能になりました。心から感謝いたします。

## Changes from DS FPS Mouse Fix

- Supported only **Metroid Prime Hunters**.
- Speeded up right-click zoom.
- Fixed shooting to left click.
  - If you use a PAD, set left click to the button used for shooting.
  - The Shoot setting in `DS_MPH_Controls.txt` is ignored.
- Low latency shooting.
- Fast zoom with the **C key**. It is currently fixed to the C key, so it cannot be used for other keys.

## DS FPS Mouse Fixからの変更点

- 対応ソフトを**メトロイドプライムハンターズ**のみにした。
- 右クリックズームを高速化した。
- 射撃を左クリックに固定。
  - PADで使う場合は射撃で使うボタンに左クリックを設定すること。
  - `DS_MPH_Controls.txt`のShoot設定は無視される。
- 射撃を低遅延化した。
- **Cキー**で高速ズームできる。今のところCキー固定なので他のキーにすることはできない。


以下の英語の説明の後に日本語の説明を記載しているので、スクロールしてご覧ください。

# DS Mouse Input Fix V 1.4

This tool aims to make DS FPS games more enjoyable by adding custom inputs that map to mouse inputs and keyboard macros, so that the player can simply focus on the game.

**IMPORTANT:** IF THE SCRIPT CRASHES AND YOUR MOUSE 1 AND MOUSE 2 INPUTS ARE STILL REVERSED, RUN "Fix Mouse.exe" THIS SHOULD FIX THE ISSUE.

## HOW TO USE

First, we must make sure that your melon DS inputs match the script's requirements. These should look like this:

- **up, down, left, right**: Can be whatever buttons you see fit, as long as they are not listed below.
- **Y, A, B**: Can be whatever buttons you see fit, as long as they are not listed below.
- **Select, Start**: Can be whatever buttons you see fit, as long as they are not listed below.

- **X** = Space key
- **L** = N key
- **R** = M key

You do not need to create any config files ahead of time; in fact, I recommend you don't and let the script write them for you.

**Before getting started, REMEMBER:**
- Kill the program at any time with `BACKSPACE`.
- Pause it at any time with `RIGHT_SHIFT`.

### Steps to Get Started:

1. Run the "DS Mouse Input Fix.exe".
2. Read the information presented in the window.
3. Select the game you wish to play from the drop-down menu.

**REMINDER:** After pressing start, you have 2 seconds before the window screen turns red. Once the screen has turned red, the script is active.

4. Press Start when ready.
5. Once the script is active, **DO NOT CLICK**. Align your mouse loosely with the bottom left corner of your stylus play space and click once. Next, align it with the top right corner and click again.
6. At this point, you should be good to go. Your cursor should automatically center, and your inputs should be recognized. Depending on what game you chose, the script will have generated some input config files, and you can adjust them as you please.

**AGAIN, ALWAYS REMEMBER:**
- `BACKSPACE` TO KILL.
- `RIGHT_SHIFT` TO PAUSE.

---

## CHANGING WAIT TIMES AND OTHER SETTINGS

This script uses wait times within its inputs to ensure the DS emulator catches the desired input. However, sometimes these waits are too short, which can result in ghost inputs and camera stealing. To remedy this, there is a `DS_Config.txt` that this script will generate.

Within that file, there are 4 values that can be changed:

- **mouseResetWait**: Pertains to the time before the mouse moves again. Adjust this if your camera keeps jerking when your cursor is reset to center.
- **buttonWait**: Pertains to the time before the mouse moves after pressing a button. Adjust this if you get ghost inputs (buttons not properly pressed).
- **swapWait**: COD-specific; pertains to the wait time between swapping weapons in the COD games. Adjust accordingly.
- **keyWait**: Pertains to the time between key inputs. This is used for some macros, such as sprinting and crouching in the COD games. Adjust this if those inputs are not caught.

Please note that the times MUST be written in milliseconds. For example, `1000` = 1 second of wait time.

These are the defaults:
- `mouseResetWait` = 33
- `buttonWait` = 33
- `swapWait` = 220
- `keyWait` = 30

**DO NOT** define more than one of these values on the same line.
**DO NOT** include any other text on a line where these values are defined.

Another setting that can be changed in this config file is whether or not you want the program to automatically recenter and drag your mouse across your screen for you. This is done by changing the `autoMouseDrag` value.

- Set this equal to `0` to turn off auto mouse drag, or `1` to turn it on. By default, it is set to `1`.

**IMPORTANT:**
If you choose to play with this setting off, be warned you will have to drag your mouse with `RIGHT CLICK`, unless you bind a key to do it with external software, in which case, you will have to bind it to `RIGHT CLICK`. You should also make sure that the keybinds config file does not have any function bound to `RIGHT CLICK` either if you choose to play with this setting off.

If you mess up this file too much and start getting crashes, just delete this txt file, and the script will fix it for you.

---

## CHANGING INPUTS

Each game that this script covers has unique inputs. Some of them are shared if the games are similar enough (COD series, Dementium 1 and 2).

The script will generate these files if it does not detect them, then you can edit them.

Note the input keys must be denoted by their corresponding hex values as denoted here:
[[Virtual-Key Codes]()](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

**DO NOT** define more than one of these inputs on the same line.
**DO NOT** include any other text on a line where these inputs are defined.

If you would like to customize your keybinds, you can do so in any of the Controls text files.

If you mess up these files too much and start getting crashes, just delete them, and the script will fix it for you.

### Default Controls:

#### MPH:
- **Shoot/attack** = Left Mouse
- **ADS/Zoom** = Right Mouse
- **Manual Mouse Reset** = Middle Mouse
- **Screen Tap Jump (MPH)** = Right Control
- **Jump (MPH)** = Space Bar
- **Ball (MPH)** = Left Control
- **Scan Vision (MPH)** = V key
- **Special Weapon (MPH) 1 - 6** = 1 - 6 keys
- **Swap to Main Weapon (MPH)** = Q key
- **Swap to Missiles (MPH)** = E key
- **Swap to Third Weapon** = R key
- **Select "Yes" in Menu** = Mouse Back Button
- **Select "No" in Menu** = Mouse Forward Button
- **Select Left Arrow in Menu** = Z key
- **Select Right Arrow in Menu** = C key


# DS Mouse Input Fix V 1.4

このツールは、DSのFPSゲームをより楽しむために、マウス入力とキーボードマクロに対応したカスタム入力を追加し、プレイヤーがゲームに集中できるようにします。

**重要：** スクリプトがクラッシュしてマウス1とマウス2の入力が逆になっている場合は、「Fix Mouse.exe」を実行してください。これで問題が解決するはずです。

## 使い方

まず、Melon DSの入力がスクリプトの要件に一致していることを確認します。それらは次のように設定してください：

- **上、下、左、右** = 以下にリストされていない限り、任意のボタンで構いません  
- **Y、A、B** = 以下にリストされていない限り、任意のボタンで構いません  
- **セレクト、スタート** = 以下にリストされていない限り、任意のボタンで構いません  

- **X** = スペースキー
- **L** = Nキー
- **R** = Mキー

事前に設定ファイルを作成する必要はありません。むしろ、スクリプトが自動で作成することをお勧めします。

**始める前に覚えておくこと：**
- バックスペースでプログラムをいつでも終了できます。
- 右シフトで一時停止できます。

### 開始手順：

1. 「DS Mouse Input Fix.exe」を実行
2. ウィンドウに表示される情報を読む
3. ドロップダウンメニューからプレイしたいゲームを選択

**注意：** スタートを押すと、ウィンドウの画面が赤くなるまで2秒かかります。画面が赤くなったら、スクリプトがアクティブになります。

4. 準備ができたらスタートを押す
5. スクリプトがアクティブになったらクリックしないで、スタイラスプレイスペースの左下にマウスを合わせてクリックし、次に右上に合わせて再度クリックします。
6. これで準備完了です。カーソルが自動的に中央に戻り、入力が認識されます。選択したゲームに応じてスクリプトがいくつかの入力設定ファイルを生成するので、必要に応じて調整してください。

**再度覚えておくこと：**
- バックスペースで終了
- 右シフトで一時停止

---

## 設定の変更

このスクリプトは、DSエミュレータが望ましい入力を確実にキャッチするために、入力間に待機時間を使用します。しかし、時にはこれらの待機時間が短すぎて、ゴースト入力やカメラの引き込みが発生することがあります。  
この問題を解決するために、スクリプトが生成する`DS_Config.txt`ファイルで次の4つの値を変更できます：

- **mouseResetWait**：カーソルが中央にリセットされたときにカメラが引っかかる場合はこれを調整
- **buttonWait**：ボタンを押した後にマウスが移動する前の待機時間を調整
- **swapWait**：CODゲームで武器を切り替える間の待機時間を調整
- **keyWait**：CODゲームでスプリントやしゃがむマクロのためのキー入力間の待機時間を調整

待機時間はミリ秒で指定する必要があります。例：`1000` = 1秒

これらのデフォルト値は次のとおりです:
- `mouseResetWait` = 33
- `buttonWait` = 33
- `swapWait` = 220
- `keyWait` = 30

これらの値を同じ行に複数定義しないでください。  
これらの値が定義されている行には他のテキストを含めないでください。

この設定ファイルで変更できるもう一つの設定は、プログラムが自動的にマウスを再中央化し、画面全体をドラッグするかどうかです。  
これは「autoMouseDrag」値を変更することで行えます。これを`0`に設定すると自動マウスドラッグがオフになり、`1`に設定するとオンになります。  
デフォルトは`1`です。

**重要：**  
この設定をオフにすると、マウスを右クリックでドラッグする必要があります。外部ソフトウェアでキーを右クリックにバインドしない限り、この設定をオフにしないことをお勧めします。また、この設定をオフにする場合は、右クリックに機能がバインドされていないことを確認してください。

このファイルを誤って修正しすぎてクラッシュが発生した場合は、このテキストを削除し、スクリプトが自動的に修正します。

---

## 入力の変更

このスクリプトが対応する各ゲームには固有の入力があります。同じゲームであれば、入力が共有されることがあります（CODシリーズ、Dementium 1と2）。

スクリプトがこれらのファイルを検出しない場合、生成します。その後、編集できます。

入力キーは対応する16進数値で指定する必要があります：  
[[Virtual-Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)](https://learn.microsoft.com/ja-jp/windows/win32/inputdev/virtual-key-codes)

これらの入力を同じ行に複数定義しないでください。  
これらの入力が定義されている行には他のテキストを含めないでください。

キー設定をカスタマイズしたい場合は、任意のコントロールテキストファイルで行えます。

これらのファイルを誤って修正しすぎてクラッシュが発生した場合は、削除してスクリプトが自動的に修正します。

### デフォルトのコントロールリスト:

#### MPH:
- **射撃/攻撃** = 左クリック
- **エイム/ズーム** = 右クリック
- **手動マウスリセット** = 中クリック
- **画面タップジャンプ (MPH)** = 右コントロール
- **ジャンプ (MPH)** = スペースバー
- **ボール (MPH)** = 左コントロール
- **スキャンビジョン (MPH)** = Vキー
- **特殊武器 (MPH) 1 - 6** = 1 - 6キー
- **メイン武器に切り替え (MPH)** = Qキー
- **ミサイルに切り替え (MPH)** = Eキー
- **第三武器に切り替え** = Rキー
- **メニューで「はい」を選択** = マウスバックボタン
- **メニューで「いいえ」を選択** = マウスフォワードボタン
- **メニューで左矢印を選択** = Zキー
- **メニューで右矢印を選択** = Cキー
