# M5Stack Basic Check

M5Stack Basic Core の基本動作を確認する PlatformIO プロジェクトです。

## 確認できること

- LCD 表示
- A/B/C ボタン入力
- スピーカー出力
- シリアルモニター出力
- `millis()` による稼働時間更新

## 使い方

```sh
cd basic
pio run
pio run --target upload
pio device monitor
```

## ボタン操作

- A: カウンターを増やしてビープ音を鳴らす
- B: 背景色を切り替える
- C: カウンターをリセットする
