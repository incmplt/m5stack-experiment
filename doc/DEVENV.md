# M5Stack + 独自 ESP32 基板 共通開発手順（macOS 版）

> **対象環境**：macOS（Apple Silicon / Intel 共通）  
> M5Stack と独自 ESP32 基板の共通開発を前提にした手順です。  
> ビルド環境には VS Code + PlatformIO を使用し、M5Stack 向けには M5Unified ライブラリを採用します。

---

## 目次

1. [推奨構成](#1-推奨構成)
2. [開発環境構築手順](#2-開発環境構築手順)
3. [新規プロジェクト作成](#3-新規プロジェクト作成)
4. [platformio.ini の設定](#4-platformioini-の設定)
5. [サンプルプロジェクトの作成](#5-サンプルプロジェクトの作成)
6. [ビルド・書き込み・シリアル確認](#6-ビルド書き込みシリアル確認)
7. [推奨開発ステップ（発展）](#7-推奨開発ステップ発展)
8. [実務上の注意点](#8-実務上の注意点)

---

## 1. 推奨構成

| 項目 | 内容 |
|------|------|
| OS | macOS 12 Monterey 以降（Apple Silicon / Intel 共通） |
| エディタ | Visual Studio Code |
| ビルド環境 | PlatformIO IDE（VS Code 拡張） |
| Framework | Arduino framework |
| 言語 | C/C++ |
| M5Stack ライブラリ | M5Unified |
| 描画ライブラリ | M5GFX（M5Unified 経由で利用） |

### M5Unified とは

M5Unified は M5Stack 社が提供する統合ライブラリです。従来は機種ごとに `M5Stack.h`・`M5Core2.h`・`M5CoreS3.h` と個別のヘッダを使い分ける必要がありましたが、M5Unified では `M5Unified.h` 一つをインクルードするだけで、LCD・ボタン・タッチ・スピーカー・電源管理・IMU・RTC などを統一 API で操作できます。

内部で使用している描画ライブラリが M5GFX であり、M5Unified 経由で自動的に利用されます。直接 M5GFX を操作することも可能ですが、通常は M5Unified の `M5.Display` 経由で十分です。

---

## 2. 開発環境構築手順

### 2-1. Homebrew のインストール

macOS のパッケージマネージャである Homebrew を使うと、後続のツールを一元管理できます。すでにインストール済みの場合はスキップしてください。

ターミナル（`/Applications/Utilities/Terminal.app`）を開き、以下を実行します。

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

インストール完了後、Apple Silicon Mac の場合はパスを通す必要があります。インストーラの指示に従い、以下を `~/.zprofile` に追記します。

```bash
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"
```

Intel Mac の場合は `/usr/local/bin` にインストールされるため、追加のパス設定は不要です。

インストール確認：

```bash
brew --version
# 例: Homebrew 4.x.x
```

### 2-2. Visual Studio Code のインストール

[VS Code 公式サイト](https://code.visualstudio.com/) から、macOS 向けの `.zip` をダウンロードします。

Apple Silicon Mac の場合は **"Mac (Apple Silicon)"** を、Intel Mac の場合は **"Mac (Intel)"** を選択してください。Universal 版でも動作しますが、ネイティブ版の方が動作が軽快です。

ダウンロードした `.zip` を展開し、`Visual Studio Code.app` を `/Applications` フォルダにドラッグして移動します。

初回起動時は「インターネットからダウンロードされたアプリです」という警告が出ることがあります。**「開く」** をクリックして続行します。

インストール後、コマンドラインから `code` コマンドを使えるようにしておくと便利です。VS Code を起動し、`Cmd+Shift+P` でコマンドパレットを開き、`Shell Command: Install 'code' command in PATH` を実行します。

```bash
# 設定後の確認
code --version
# 例: 1.xx.x
```

### 2-3. PlatformIO IDE のインストール

PlatformIO は VS Code の拡張機能として提供されています。

1. VS Code を起動します
2. 左サイドバーの **Extensions（ブロックアイコン）** をクリック、または `Cmd+Shift+X` を押します
3. 検索ボックスに `PlatformIO IDE` と入力します
4. **PlatformIO IDE**（発行元: PlatformIO）を選択し、**Install** をクリックします
5. インストール完了後、VS Code の再読み込みを求めるダイアログが表示されるので **Reload Window** をクリックします

インストールが完了すると、左サイドバーにアリのアイコン（PlatformIO のロゴ）が追加されます。初回起動時はツールチェーンのダウンロードが自動的に行われます（数分かかることがあります）。

> **注意**：PlatformIO は Python を内部で使用しています。macOS には Python が標準搭載されていますが、PlatformIO は自前の仮想環境を `~/.platformio` 以下に構築するため、ユーザーが Python をインストールする必要はありません。

### 2-4. USB ドライバの確認とインストール

M5Stack の USB-Serial 変換チップは機種によって異なります。macOS では機種に応じてドライバが必要になることがあります。

#### チップの確認

| 機種 | USB-Serial チップ |
|------|-------------------|
| M5Stack Basic / Gray / Fire | CP2104（Silicon Labs） |
| M5Stack Core2 | CP2104（Silicon Labs） |
| M5Stack CoreS3 | USB CDC（ドライバ不要） |
| M5StickC / M5StickC Plus | CH9102（WCH） |
| M5StickC Plus2 | USB CDC（ドライバ不要） |

#### CP210x ドライバ（M5Stack Core2 など）

macOS 11 以降では CP210x は OS に標準搭載されているため、追加ドライバは不要なことがほとんどです。認識されない場合は Silicon Labs の公式ドライバをインストールします。

[Silicon Labs CP210x ドライバダウンロード](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

#### CH9102 ドライバ（M5StickC など）

WCH の公式サイトからドライバをダウンロードしてインストールします。

[WCH CH34x ドライバダウンロード](http://www.wch-ic.com/downloads/CH34XSER_MAC_ZIP.html)

インストール後、**システム設定 → プライバシーとセキュリティ** でドライバの実行を許可する必要があります。

#### 接続確認

M5Stack を USB ケーブルで Mac に接続し、ターミナルで以下を実行します。

```bash
ls /dev/cu.*
```

正常に認識されている場合の出力例：

```
/dev/cu.usbserial-0001
/dev/cu.SLAB_USBtoUART
```

CoreS3 など USB CDC 対応機種の場合：

```
/dev/cu.usbmodem1234561
```

何も表示されない場合は、以下を順に確認します。

- USB ケーブルがデータ通信対応であること（充電専用ケーブルは不可）
- ケーブルを抜き差ししてから再度確認する
- M5Stack の電源が入っていること
- ドライバが正しくインストールされていること

---

## 3. 新規プロジェクト作成

### 3-1. PlatformIO Home を開く

VS Code 左サイドバーのアリアイコン（PlatformIO）をクリックすると、PlatformIO のホーム画面が表示されます。または VS Code 下部のステータスバーにある家のアイコンをクリックします。

### 3-2. New Project

PlatformIO Home の **"New Project"** ボタンをクリックします。

Project Wizard が表示されるので、以下のように設定します。

| 項目 | 値（例） |
|------|----------|
| Name | `m5stack_sample` |
| Board | `M5Stack Core2` |
| Framework | `Arduino` |
| Location | チェックを外して任意のディレクトリを指定 |

**Board** の入力欄はインクリメンタルサーチができます。`m5stack core2` と入力すると候補が絞られます。

**Finish** をクリックすると、PlatformIO がプロジェクトのひな形を生成し、必要なツールチェーンとライブラリを自動ダウンロードします。初回は数分かかります。

### 3-3. 生成されるプロジェクト構造

```
m5stack_sample/
├── .gitignore
├── .vscode/
│   ├── c_cpp_properties.json   # IntelliSense 設定（自動生成）
│   ├── extensions.json
│   └── launch.json
├── include/                    # ヘッダファイル置き場（任意利用）
├── lib/                        # プロジェクト固有ライブラリ置き場
├── platformio.ini              # ビルド設定ファイル
├── src/
│   └── main.cpp                # メインのソースファイル
└── test/                       # ユニットテスト置き場
```

各ディレクトリの役割：

- **`src/`**：アプリケーションのソースコード。`main.cpp` はここに置きます
- **`include/`**：プロジェクト全体で共有するヘッダファイル
- **`lib/`**：プロジェクト専用のライブラリ（Arduino の `libraries` フォルダ相当）
- **`test/`**：PlatformIO のユニットテストフレームワーク用

### 3-4. ボード名の対応表

| 機種 | PlatformIO board 名 |
|------|---------------------|
| M5Stack Basic / Gray | `m5stack-core-esp32` |
| M5Stack Core2 | `m5stack-core2` |
| M5Stack CoreS3 | `m5stack-cores3` |
| M5StickC Plus | `m5stick-c` |
| M5StickC Plus2 | `m5stick-c-plus2` |
| 独自 ESP32 基板（ESP32） | `esp32dev` |
| 独自 ESP32 基板（ESP32-S3） | `esp32-s3-devkitc-1` |

---

## 4. platformio.ini の設定

`platformio.ini` はプロジェクトのビルド設定を管理する中心的なファイルです。ここで対象ボード・フレームワーク・ライブラリ依存・シリアル速度などを指定します。

### 4-1. 単一ボード構成（M5Stack Core2 のみ）

プロジェクト作成直後の `platformio.ini` を以下のように編集します。

```ini
[env:m5stack_core2]
platform = espressif32
board = m5stack-core2
framework = arduino
monitor_speed = 115200
lib_deps =
    m5stack/M5Unified
build_flags =
    -D BOARD_M5STACK
```

各設定の意味：

| キー | 意味 |
|------|------|
| `platform` | 対象マイコンのプラットフォーム。ESP32 系は `espressif32` |
| `board` | 対象ボードの識別子 |
| `framework` | 使用するフレームワーク。`arduino` を指定 |
| `monitor_speed` | シリアルモニタのボーレート |
| `lib_deps` | 依存ライブラリ。PlatformIO が自動でダウンロード・管理する |
| `build_flags` | コンパイラフラグ。`-D XXXX` でプリプロセッサマクロを定義できる |

### 4-2. 複数ボード構成（M5Stack + 独自 ESP32 の共存）

同一プロジェクトで複数のボードをビルドターゲットとして管理できます。共通設定を `[env]` セクションに書き、各ボード固有の設定を `[env:xxx]` に書きます。`[env]` の設定は全 `[env:xxx]` に自動継承されます。

```ini
[platformio]
; デフォルトでビルドする環境（pio run だけで使われる環境）
default_envs = m5stack_core2

[env]
; 全環境共通の設定
platform = espressif32
framework = arduino
monitor_speed = 115200

[env:m5stack_core2]
board = m5stack-core2
lib_deps =
    m5stack/M5Unified
build_flags =
    -D BOARD_M5STACK

[env:custom_esp32]
board = esp32dev
build_flags =
    -D BOARD_CUSTOM
```

特定の環境だけをビルド・書き込みする場合は `-e` オプションで指定します。

```bash
# m5stack_core2 環境でビルド
pio run -e m5stack_core2

# custom_esp32 環境で書き込み
pio run -e custom_esp32 -t upload
```

### 4-3. ライブラリのバージョン固定

開発が進んだら、ライブラリのバージョンを固定することで、将来的なライブラリ更新による動作変化を防げます。

```ini
lib_deps =
    m5stack/M5Unified @ ^0.1.16
```

`^` は「メジャーバージョンが同じ範囲での最新版」を意味します（semver）。完全に固定する場合は `@` の後にバージョン番号のみを記載します。

```ini
lib_deps =
    m5stack/M5Unified @ 0.1.16
```

### 4-4. USB ポートの明示

複数の USB デバイスが接続されている場合や、自動検出が不安定な場合は、`upload_port` と `monitor_port` を明示します。

```ini
[env:m5stack_core2]
board = m5stack-core2
framework = arduino
upload_port = /dev/cu.usbserial-0001
monitor_port = /dev/cu.usbserial-0001
monitor_speed = 115200
lib_deps =
    m5stack/M5Unified
```

ポート名は `ls /dev/cu.*` で確認したものを使います。M5Stack を抜き差しする前後で `ls /dev/cu.*` を実行し、差分として現れたデバイス名がそのポートです。

---

## 5. サンプルプロジェクトの作成

### 5-1. サンプルの概要

M5Stack の画面に文字を表示し、ボタン A を押すたびにカウンターが増加する最小限のサンプルです。M5Unified の基本的な API（画面描画・ボタン入力・シリアルログ）を確認できます。

### 5-2. src/main.cpp の編集

`src/main.cpp` を開き、デフォルトの内容を以下に書き換えます。

```cpp
#include <M5Unified.h>

// カウンターの値をグローバル変数で保持する
int counter = 0;

void setup() {
    // M5Unified の初期化
    // M5.config() でデフォルト設定を取得し、M5.begin() に渡す
    // これにより LCD・ボタン・電源管理などが一括で初期化される
    auto cfg = M5.config();
    M5.begin(cfg);

    // テキストサイズの設定（1〜7 の整数。2 は標準的な大きさ）
    M5.Display.setTextSize(2);

    // 画面上部にタイトルを表示
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack Sample");

    // 操作説明を表示
    M5.Display.setCursor(10, 50);
    M5.Display.println("Button A: Count Up");

    // シリアルにも出力しておく（デバッグ用）
    Serial.println("Setup complete.");
}

void loop() {
    // ボタン状態などを更新する（毎ループ必須）
    M5.update();

    // ボタン A が「押された瞬間」かどうかを判定
    // wasPressed() は押された瞬間の1フレームだけ true を返す
    if (M5.BtnA.wasPressed()) {
        counter++;

        // 表示エリアを黒で塗りつぶして前の値を消去する
        // fillRect(x, y, width, height, color)
        // BLACK は M5GFX で定義された色定数（0x0000）
        M5.Display.fillRect(0, 100, 320, 60, BLACK);

        // カーソルを移動してカウンター値を描画
        M5.Display.setCursor(10, 100);
        M5.Display.printf("Counter: %d", counter);

        // シリアルにも出力
        Serial.printf("Counter: %d\n", counter);
    }

    // 短い待機を入れてループの実行速度を制限する
    // CPU の過負荷を防ぎ、ボタンのチャタリングも軽減する
    delay(10);
}
```

### 5-3. コードの解説

#### M5.config() と M5.begin()

```cpp
auto cfg = M5.config();
M5.begin(cfg);
```

`M5.config()` は `M5Unified::config_t` 型の設定構造体を返します。この構造体のメンバーを変更することで、起動時の動作をカスタマイズできます。たとえば以下のように書くと、起動時の画面クリアや外部ポートの設定を変更できます。

```cpp
auto cfg = M5.config();
cfg.serial_baudrate = 115200;   // シリアルのボーレートを指定
cfg.clear_display   = true;     // 起動時に画面をクリア
M5.begin(cfg);
```

シンプルなサンプルでは `M5.config()` の戻り値をそのまま渡すだけで問題ありません。

#### M5.Display の座標系

M5Stack Core2 の画面解像度は 320×240 ピクセルです。左上が原点 (0, 0) で、右方向が X 軸、下方向が Y 軸です。

```
(0,0) ──────────────── (320,0)
  │                          │
  │   320 x 240 px           │
  │                          │
(0,240) ──────────── (320,240)
```

#### wasPressed() と isPressed() の違い

| メソッド | 動作 |
|----------|------|
| `isPressed()` | ボタンが現在押されている間ずっと `true` |
| `wasPressed()` | ボタンが押された瞬間の1フレームだけ `true` |
| `wasReleased()` | ボタンが離された瞬間の1フレームだけ `true` |

カウントアップには `wasPressed()` を使うのが適切です。`isPressed()` を使うと `delay(10)` ごとにカウントが増え続けてしまいます。

#### fillRect による消去

テキストを更新する際、前の描画を消去してから再描画する必要があります。最もシンプルな方法は `fillRect()` で矩形を塗りつぶすことです。

```cpp
// fillRect(x座標, y座標, 幅, 高さ, 色)
M5.Display.fillRect(0, 100, 320, 60, BLACK);
```

`BLACK` は M5GFX で定義された色定数（値は `0x0000`）です。同様に `WHITE`・`RED`・`GREEN`・`BLUE`・`YELLOW` なども使用できます。

---

## 6. ビルド・書き込み・シリアル確認

### 6-1. ビルド（コンパイル）

VS Code の下部ステータスバーには PlatformIO のショートカットアイコンが並んでいます。チェックマーク（✓）がビルドです。クリックするとビルドが開始されます。

ターミナルから実行する場合：

```bash
# プロジェクトディレクトリに移動してから実行
cd ~/path/to/m5stack_sample
pio run
```

特定の環境だけビルドする場合：

```bash
pio run -e m5stack_core2
```

ビルドが成功すると、以下のような出力が表示されます。

```
Linking .pio/build/m5stack_core2/firmware.elf
Building .pio/build/m5stack_core2/firmware.bin
RAM:   [=         ]  11.7% (used 38332 bytes from 327680 bytes)
Flash: [===       ]  32.1% (used 421013 bytes from 1310720 bytes)
========================= [SUCCESS] Took 12.34 seconds =========================
```

ROM（Flash）と RAM の使用量が確認できます。

#### よくあるビルドエラーと対処

| エラー内容 | 原因と対処 |
|-----------|-----------|
| `'M5Unified.h' file not found` | `lib_deps` に `m5stack/M5Unified` が書かれているか確認。書かれていれば `pio run` の初回実行時に自動ダウンロードされる |
| `error: 'BLACK' was not declared` | M5Unified が正しくインクルードされていないか、古いバージョンの可能性。`#include <M5Unified.h>` を確認 |
| `[FAILED] espressif32 platform is not installed` | PlatformIO が espressif32 プラットフォームをダウンロードできていない。ネットワーク接続を確認 |
| IntelliSense がエラーを表示する | ビルド後に `.vscode/c_cpp_properties.json` が更新される。一度 `pio run` を実行してから VS Code を再読み込みする |

### 6-2. 書き込み（フラッシュ）

M5Stack が USB 接続されていることを確認してから、矢印アイコン（→）をクリック、またはターミナルで実行します。

```bash
pio run -t upload
```

書き込みが開始されると以下のような出力が続きます。

```
Connecting........_____....._
Chip is ESP32-D0WDQ6-V3 (revision v3.0)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
...
Writing at 0x00010000... (10 %)
Writing at 0x000140e4... (20 %)
...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
========================= [SUCCESS] Took 8.56 seconds =========================
```

書き込み完了後、M5Stack は自動的にリセットされ、書き込んだファームウェアが起動します。

#### 書き込みが失敗する場合

書き込み前に M5Stack をブートローダーモードにする必要がある場合があります。

- **M5Stack Core2**：通常は RTS/DTR ラインにより自動でブートローダーに入ります
- 失敗する場合は M5Stack の電源を入れ直した直後に書き込みコマンドを実行します
- `upload_port` を `platformio.ini` に明示することで、誤ったポートへの書き込みを防げます

```ini
upload_port = /dev/cu.usbserial-0001
```

### 6-3. シリアルモニタ

書き込み後、シリアルモニタで `Serial.println()` の出力を確認できます。プラグアイコンをクリック、またはターミナルで実行します。

```bash
pio device monitor
```

サンプルコードではセットアップ完了時に `"Setup complete."` が、ボタン A を押すたびに `"Counter: N"` が出力されます。

```
--- Terminal on /dev/cu.usbserial-0001 | 115200 8-N-1
--- Available filters and text transformations: colorize, debug, default, ...
Setup complete.
Counter: 1
Counter: 2
Counter: 3
```

シリアルモニタを終了するには `Ctrl+C` を押します。

#### 書き込みとシリアルモニタを続けて実行する

以下のコマンドで書き込み後にそのままシリアルモニタを起動できます。

```bash
pio run -t upload && pio device monitor
```

### 6-4. 動作確認のチェックリスト

書き込み後、以下を順に確認します。

- [ ] M5Stack の画面に `"M5Stack Sample"` と `"Button A: Count Up"` が表示される
- [ ] ボタン A（画面左下のボタン）を押すと `"Counter: 1"` のように数値が増えて表示される
- [ ] シリアルモニタに `"Setup complete."` と `"Counter: N"` が出力される
- [ ] ボタンを長押ししても1回だけカウントアップされる（`wasPressed()` の動作確認）

---

## 7. 推奨開発ステップ（発展）

サンプルが動作したら、以下のステップで独自 ESP32 基板との共通開発環境へ発展させます。

### Step 1: Wi-Fi 接続を追加して動作確認

基本動作を確認したら、Wi-Fi 接続とシリアルログも確認しておきます。

```cpp
#include <M5Unified.h>
#include <WiFi.h>

const char* SSID     = "your-ssid";
const char* PASSWORD = "your-password";

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Connecting WiFi...");

    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("WiFi Connected!");
    M5.Display.setCursor(10, 50);
    M5.Display.println(WiFi.localIP().toString().c_str());

    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

void loop() {
    M5.update();
    delay(10);
}
```

### Step 2: 共通ロジックをディレクトリ分離

M5Stack 固有の処理とアプリのビジネスロジックを分けて管理します。

```
m5stack_sample/
├── platformio.ini
├── src/
│   └── main.cpp             # エントリポイント
└── lib/
    ├── app/
    │   ├── app_logic.cpp    # アプリ本体（ボード非依存のロジック）
    │   └── app_logic.h
    └── board/
        ├── board_m5stack.cpp  # M5Stack 固有の処理
        ├── board_m5stack.h
        ├── board_custom.cpp   # 独自基板固有の処理
        └── board_custom.h
```

`board_m5stack.h` の例：

```cpp
#pragma once

void board_init();
void board_display_text(int x, int y, const char* text);
void board_clear_area(int x, int y, int w, int h);
bool board_button_a_pressed();
```

`board_m5stack.cpp` の例：

```cpp
#include "board_m5stack.h"
#include <M5Unified.h>

void board_init() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
}

void board_display_text(int x, int y, const char* text) {
    M5.Display.setCursor(x, y);
    M5.Display.println(text);
}

void board_clear_area(int x, int y, int w, int h) {
    M5.Display.fillRect(x, y, w, h, BLACK);
}

bool board_button_a_pressed() {
    M5.update();
    return M5.BtnA.wasPressed();
}
```

### Step 3: `#ifdef` でボード依存処理を分岐

`platformio.ini` の `build_flags` で定義したマクロを `#ifdef` で使い、ボード依存処理を切り替えます。

```cpp
#include "app/app_logic.h"

#ifdef BOARD_M5STACK
    #include "board/board_m5stack.h"
    #define board_init        board_m5stack_init
    #define board_btn_pressed board_m5stack_button_a_pressed
#elif defined(BOARD_CUSTOM)
    #include "board/board_custom.h"
    #define board_init        board_custom_init
    #define board_btn_pressed board_custom_button_pressed
#endif

void setup() {
    board_init();
    app_init();
}

void loop() {
    if (board_btn_pressed()) {
        app_on_button();
    }
    delay(10);
}
```

### Step 4: 量産前に ESP-IDF 移行可否を検討

以下の要件が生じた場合は ESP-IDF（Espressif IoT Development Framework）への移行を検討します。Arduino フレームワークは ESP-IDF 上に構築されており、必要に応じて ESP-IDF の API を Arduino と併用することも可能です。

| 要件 | Arduino | ESP-IDF |
|------|---------|---------|
| Secure Boot / Flash Encryption | △ 設定困難 | ◎ 公式サポート |
| OTA 更新（堅牢な実装） | △ ライブラリ依存 | ◎ 組み込み機能 |
| 低消費電力制御（Deep Sleep 等） | ○ 一部対応 | ◎ 細かく制御可能 |
| FreeRTOS の細かい制御 | △ 隠蔽されている | ◎ 直接利用可能 |
| 開発のしやすさ | ◎ 簡単 | △ 学習コストが高い |
| M5Unified の利用 | ◎ | × 非対応 |

---

## 8. 実務上の注意点

| 注意点 | 内容 |
|--------|------|
| 古い M5Stack ライブラリ | `M5Stack.h`・`M5Core2.h` より **`M5Unified.h` を優先**する。ネット上の古い記事は旧ライブラリを使っていることがあるため注意 |
| ボード違いによる API 差異 | Core2・CoreS3・Basic で画面サイズ・ボタン配置・電源管理の仕様が異なる。M5Unified を使えば差異は吸収されるが、解像度などハードウェア固有の値は別途管理する |
| USB ケーブル | 充電専用ケーブルではデータ通信ができず書き込み不可。必ずデータ通信対応ケーブルを使用する |
| Apple Silicon Mac でのドライバ | 一部ドライバが Apple Silicon 未対応の場合がある。ドライバのリリースノートで対応アーキテクチャを確認する |
| ポート認識の安定化 | `upload_port`・`monitor_port` を `platformio.ini` に明示すると書き込みが安定する |
| `M5.update()` の呼び忘れ | `loop()` 内で `M5.update()` を呼ばないとボタン状態が更新されない。必ず毎ループの先頭で呼ぶ |
| `delay()` と FreeRTOS | Arduino フレームワークでも内部で FreeRTOS が動いている。`delay()` は他タスクに CPU を譲る。ブロッキング処理を長くしすぎないよう注意 |
| ライブラリのバージョン固定 | 開発が安定したら `lib_deps` にバージョンを明記して固定する。`pio pkg update` で意図せずライブラリが更新されるのを防ぐ |
| 独自基板 | GPIO・I2C・SPI・電源制御の設定はボード抽象化レイヤーにまとめる。`platformio.ini` の `board_build.f_cpu`・`board_build.flash_mode` などで細かいチューニングも可能 |

---

## 付録：よく使う PlatformIO CLI コマンド

```bash
# ビルド（default_envs を対象）
pio run

# 特定の環境だけビルド
pio run -e m5stack_core2

# 書き込み
pio run -t upload

# 特定の環境に書き込み
pio run -e m5stack_core2 -t upload

# シリアルモニタ
pio device monitor

# 接続されているシリアルデバイスを一覧表示
pio device list

# ライブラリの更新
pio pkg update

# プロジェクトのクリーン（ビルドキャッシュ削除）
pio run -t clean

# PlatformIO 自体のアップデート
pio upgrade
```

---

## 付録：最小構成の完成形

プロジェクトの最小構成は以下の2ファイルだけです。

```
m5stack_sample/
├── platformio.ini
└── src/
    └── main.cpp
```

`platformio.ini`:

```ini
[env:m5stack_core2]
platform = espressif32
board = m5stack-core2
framework = arduino
monitor_speed = 115200
lib_deps =
    m5stack/M5Unified
```

`src/main.cpp`:

```cpp
#include <M5Unified.h>

int counter = 0;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack Sample");
    M5.Display.setCursor(10, 50);
    M5.Display.println("Button A: Count Up");
}

void loop() {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        counter++;
        M5.Display.fillRect(0, 100, 320, 60, BLACK);
        M5.Display.setCursor(10, 100);
        M5.Display.printf("Counter: %d", counter);
    }
    delay(10);
}
```

この最小構成で M5Stack 上に画面表示とボタン入力が動作することを確認できれば、独自 ESP32 基板との共通開発へ進められます。
