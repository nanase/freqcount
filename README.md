# freqcount

Raspberry Pi Pico (RP2040) 上の Arduino フレームワークで GPIO ピンに入力した信号の周波数を測定します。

ここでの Arduino フレームワークは [arduino-pico](https://github.com/earlephilhower/arduino-pico) を想定しています。

## 特徴

- 複数のピンを同時に測定可能
- 特性の異なる 2 つの方法で測定が可能
  - IRQ（外部割り込み）
    - コア負荷は高いが精度が高い
    - 最大 30 ピンを同時測定可能
  - PIO
    - コア負荷が低い
    - IRQ による方法よりも高い周波数を測定可能
    - 最大 8 ピンを同時測定可能

## 使用方法

ライブラリをインストール後、 `freqcount.h` をインクルードしてください。

```cpp
#include "freqcount.h"
```

その後、 `FreqCountIRQ` または `FreqCountPIO` を宣言します。

```cpp
// using IRQ（外部割り込み）
FreqCountIRQ freq_count;

// or using PIO
FreqCountPIO freq_count;
```

測定を開始するには、`begin(pin)` を使って測定対象のピンを指定してください。このメソッドを呼び出すと、測定が開始されます。

```cpp
freq_count.begin(PIN_INPUT);
```

測定結果の更新を行うには `update()` を呼び出してください。測定が成功すると、 `true` が返されます。`get_observated_frequency()` を呼び出すと、測定された周波数を取得できます。

```cpp
if (freq_count.update()) {
  Serial.println(freq_count.get_observated_frequency(), 3);
}
```

## ライセンス

MIT License
