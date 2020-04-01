# Cannylsを用いた他ファイルシステムとのパフォーマンス比較
Cannylsを用いる理由は、このファイルシステムはそもそもCannylsの高速化を意図して作っているため。

Cannylsの重要な特性
* O_DIRECTを用いてFS層でのバッファリングを期待しない（＝Cannylsでデータのキャッシュをするならやる）
     * ホットデータに対する高速化などはより上位の層で行えば良いという考え方
* （現時点では）単一のファイルを作成し、その上で全ての作業を行う
     * 高速化のために複雑なindex treeを作る必要はない
* アプリケーション層でジャーナリングを行う
     * ファイルシステム層でのジャーナリングは、二重ジャーナリングとなるため不要（あっても良いが期待はしていない）
* 数MB級のデータを読み書きするKVSとして動くことを期待している     

## ベンチマーク環境
重要な情報だけ抜き出す
* 1TBのHDD
* 512e HDDと呼ばれる、論理1セクタ=512バイト、物理1セクタ=4096バイトな、容量が大きいHDDでは標準的となっているもの
* SATA rev 3.0


以下は`hdparm -I /dev/sdb`をしたもの
```
/dev/sdb:

ATA device, with non-removable media
        Model Number:       ST1000DM003-1ER162
        Serial Number:      ***
        Firmware Revision:  HP51
        Transport:          Serial, SATA 1.0a, SATA II Extensions, SATA Rev 2.5, SATA Rev 2.6, SATA Rev 3.0
Standards:
        Used: unknown (minor revision code 0x001f)
        Supported: 9 8 7 6 5
        Likely used: 9
Configuration:
        Logical         max     current
        cylinders       16383   16383
        heads           16      16
        sectors/track   63      63
        --
        CHS current addressable sectors:    16514064
        LBA    user addressable sectors:   268435455
        LBA48  user addressable sectors:  1953525168
        Logical  Sector size:                   512 bytes
        Physical Sector size:                  4096 bytes
        Logical Sector-0 offset:                  0 bytes
        device size with M = 1024*1024:      953869 MBytes
        device size with M = 1000*1000:     1000204 MBytes (1000 GB)
        cache/buffer size  = unknown
        Form Factor: 3.5 inch
        Nominal Media Rotation Rate: 7200
```

## ベンチマークソフトウェア
CannylsBencherという、Cannylsを呼び出して与えられたワークロードを動かすソフトウェアを用いる
* https://github.com/yuezato/cannyls_bencher

Linuxの一般的なベンチマークツールとしては IOZone や fio なども考えられるが、
これらの多くはマウントポイント下に複数ファイルが作成可能であることを前提としており、
単一ファイルによる利用を想定したminfsではこれらを動かすことができないという理由が大きい。
