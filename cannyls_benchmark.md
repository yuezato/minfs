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
### ファイルシステム
O_DIRECTが使用可能かつ標準的なファイルシステムとして比較対象にext4とXFSを用いる。

ただしext4はjournalingをoffにできるので（XFSではできない）、offにします。

#### ext4の情報
```
[ext4 info]
% tune2fs -l /dev/sdb
tune2fs 1.45.3 (14-Jul-2019)
Filesystem volume name:   <none>
Last mounted on:          <not available>
Filesystem UUID:          e1de991a-5bd4-453e-a840-421c557434ed
Filesystem magic number:  0xEF53
Filesystem revision #:    1 (dynamic)
Filesystem features:      ext_attr resize_inode dir_index filetype extent 64bit flex_bg sparse_super large_file huge_file dir_nlink extra_isize metadata_csum
Filesystem flags:         signed_directory_hash
Default mount options:    user_xattr acl
Filesystem state:         not clean
Errors behavior:          Continue
Filesystem OS type:       Linux
Inode count:              61054976
Block count:              244190646
Reserved block count:     12209532
Free blocks:              240338100
Free inodes:              61054965
First block:              0
Block size:               4096
Fragment size:            4096
Group descriptor size:    64
Reserved GDT blocks:      1024
Blocks per group:         32768
Fragments per group:      32768
Inodes per group:         8192
Inode blocks per group:   512
Flex block group size:    16
```

#### xfsの情報
```
[xfs info]
meta-data=/dev/sdb               isize=512    agcount=4, agsize=61047662 blks
         =                       sectsz=4096  attr=2, projid32bit=1
         =                       crc=1        finobt=1, sparse=1, rmapbt=0
         =                       reflink=0
data     =                       bsize=4096   blocks=244190646, imaxpct=25
         =                       sunit=0      swidth=0 blks
naming   =version 2              bsize=4096   ascii-ci=0, ftype=1
log      =internal log           bsize=4096   blocks=119233, version=2
         =                       sectsz=4096  sunit=1 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
```

### OS
* Ubuntu 19.10
```
$ cat /etc/os-release
NAME="Ubuntu"
VERSION="19.10 (Eoan Ermine)"
ID=ubuntu
ID_LIKE=debian
PRETTY_NAME="Ubuntu 19.10"
```
* Kernel 5.0
```
$ uname -r
5.0.0-38-generic
```

### HDD
重要な情報だけ抜き出す
* 3.5inchの7200rpmで、1TBのHDD
* 512e HDDと呼ばれる、論理1セクタ=512バイト、物理1セクタ=4096バイトな、容量が大きいHDDでは標準的となっているもの
* SATA rev 3.0

より詳細には以下の`hdparm -I /dev/sdb`を参考に

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
        Logical  Sector size:                   512 bytes <- 論理セクタサイズが512バイト
        Physical Sector size:                  4096 bytes <- 物理セクタサイズが4096バイト
        Logical Sector-0 offset:                  0 bytes
        device size with M = 1024*1024:      953869 MBytes
        device size with M = 1000*1000:     1000204 MBytes (1000 GB)
        cache/buffer size  = unknown
        Form Factor: 3.5 inch
        Nominal Media Rotation Rate: 7200
```

### ベンチマークソフトウェア
CannylsBencherという、Cannylsを呼び出して与えられたワークロードを動かすソフトウェアを用いる
* https://github.com/yuezato/cannyls_bencher

Linuxの一般的なベンチマークツールとしては IOZone や fio なども考えられるが、
これらの多くはマウントポイント下に複数ファイルが作成可能であることを前提としており、
単一ファイルによる利用を想定したminfsではこれらを動かすことができないという理由が大きい。

## シーケンシャルライト1
```
# 256KiBのデータを10万件書き込む。総量で25.6GiB程度
# これでシーケンシャルライトになるかどうかは本来はCannylsの実装次第ではあるが
# 現段階の実装ではシーケンシャルになる
Command {
  100000.times {
    New(256K);
  };
}
```

| 項目 |  minfs  |  ext4  | xfs |
| ---- | ---- | --- | --- |
|  実行時間  |  117.62s  | 194.82s | 185.93s |
| minfsを1とした実行時間比率 | 1 | 1.65 | 1.58 |


### ベンチマークサマリ
```
[minfs]
Calculating Statistics...
kind = Put(262144), count = 100000, 50% = 1.06052ms, 90% = 1.567738ms, 95% = 1.605314ms, 99% = 1.666501ms
[Overall 100000] 50% = 1.06052ms, 90% = 1.567738ms, 95% = 1.605314ms, 99% = 1.666501ms
Total Elapsed Time by Commands = 117.627691811s

[ext4]
kind = Put(262144), count = 100000, 50% = 1.771017ms, 90% = 3.067163ms, 95% = 3.409067ms, 99% = 4.213549ms
[Overall 100000] 50% = 1.771017ms, 90% = 3.067163ms, 95% = 3.409067ms, 99% = 4.213549ms
Total Elapsed Time by Commands = 194.820240863s

[xfs]
kind = Put(262144), count = 100000, 50% = 1.696811ms, 90% = 2.894626ms, 95% = 3.085776ms, 99% = 3.523285ms
[Overall 100000] 50% = 1.696811ms, 90% = 2.894626ms, 95% = 3.085776ms, 99% = 3.523285ms
Total Elapsed Time by Commands = 185.938885554s
```

## シーケンシャルライト2
```
# 1件で4MiBのデータを5千件書き込む。総量で20GB程度
# 通常のKVSではまずないが、Cannylsでは標準的なデータサイズ
Command {
  5000.times {
    New(4M);
  };
}
```

| 項目 |  minfs  |  ext4  | xfs |
| ---- | ---- | --- | --- |
|  実行時間  |  121.09s  | 166.02s | 157.55s |
| minfsを1とした実行時間比率 | 1 | 1.37 | 1.30 |

### ベンチマークサマリ
```
[minfs]
kind = Put(4194304), count = 5000, 50% = 23.423985ms, 90% = 34.543748ms, 95% = 34.742971ms, 99% = 36.12189ms
[Overall 5000] 50% = 23.423985ms, 90% = 34.543748ms, 95% = 34.742971ms, 99% = 36.12189ms
Total Elapsed Time by Commands = 121.095554819s

[ext4]
kind = Put(4194304), count = 5000, 50% = 14.144502ms, 90% = 125.305027ms, 95% = 174.428124ms, 99% = 193.200738ms
[Overall 5000] 50% = 14.144502ms, 90% = 125.305027ms, 95% = 174.428124ms, 99% = 193.200738ms
Total Elapsed Time by Commands = 166.027726849s

[xfs]
kind = Put(4194304), count = 5000, 50% = 13.683867ms, 90% = 119.780903ms, 95% = 156.354525ms, 99% = 184.138364ms
[Overall 5000] 50% = 13.683867ms, 90% = 119.780903ms, 95% = 156.354525ms, 99% = 184.138364ms
Total Elapsed Time by Commands = 157.553880924s
```

## ランダムアクセス1
```
# 1件1MiBのデータを5千件(= 5B)シーケンシャルに書き込んだ後に、
# 1MiBの書き込みとランダム位置読み込みを（約）2万件ずつ行う
# 同一の乱数を利用するので、
# 何度実行してもこのワークロードから生成されるコマンド列は完全に一致する

Command {
  5000.times {
    New(1M);
  };
}

Unordered[40000] {
  <50%> New(1M);
  <50%> Get;
}
```

| 項目 |  minfs  |  ext4  | xfs |
| ---- | ---- | --- | --- |
|  実行時間  |  474.93s  | 537.57s | 558.95s |
| minfsを1とした実行時間比率 | 1 | 1.13 | 1.17 |

### ベンチマークサマリ
```
[minfs]
kind = Put(1048576), count = 25000, 50% = 3.062402ms, 90% = 4.692518ms, 95% = 4.741815ms, 99% = 6.355645ms
kind = Get(1048576), count = 20000, 50% = 12.467199ms, 90% = 17.577923ms, 95% = 75.523944ms, 99% = 133.123716ms
[Overall 45000] 50% = 4.691692ms, 90% = 15.06109ms, 95% = 16.767875ms, 99% = 126.699784ms
Total Elapsed Time by Commands = 474.931562375s

[ext4]
kind = Put(1048576), count = 25000, 50% = 5.667546ms, 90% = 8.779701ms, 95% = 9.568266ms, 99% = 14.671184ms
kind = Get(1048576), count = 20000, 50% = 12.71268ms, 90% = 29.649958ms, 95% = 87.176569ms, 99% = 106.508152ms
[Overall 45000] 50% = 8.29748ms, 90% = 15.462737ms, 95% = 26.991051ms, 99% = 102.530537ms
Total Elapsed Time by Commands = 537.578451246s

[xfs]
kind = Put(1048576), count = 25000, 50% = 3.809773ms, 90% = 6.585354ms, 95% = 7.023913ms, 99% = 8.638045ms
kind = Get(1048576), count = 20000, 50% = 15.608369ms, 90% = 42.845267ms, 95% = 49.046564ms, 99% = 59.257277ms
[Overall 45000] 50% = 6.538871ms, 90% = 33.490151ms, 95% = 41.476953ms, 99% = 55.041431ms
Total Elapsed Time by Commands = 558.952363735s
```

## ランダムアクセス2
```
# ディスク内部を左右に徐々に大きく振動していくパターン

Command {
  5000.times {  # 1MiBのデータを5千件書き込み
    New(1M);
  };

  20000.times { # 2万回以下の動作をする
    New(1M);    # 1MiBのデータを書き
    Get(1, 5);  # 書き込まれた時間が古い方のデータ(1%-5%)からランダムにGET
                # 古いデータほどhotというふうに見ることもできる
  };
}
```

| 項目 |  minfs  |  ext4  | xfs |
| ---- | ---- | --- | --- |
|  実行時間  |  271.68s  | 360.20s | 361.21s |
| minfsを1とした実行時間比率 | 1 | 1.32 | 1.32 |

### ベンチマークサマリ

```
[minfs]
kind = Put(1048576), count = 15000, 50% = 4.748524ms, 90% = 8.63111ms, 95% = 9.690315ms, 99% = 11.268441ms
kind = Get(1048576), count = 10000, 50% = 17.537065ms, 90% = 23.897184ms, 95% = 25.930385ms, 99% = 56.28817ms
[Overall 25000] 50% = 7.411114ms, 90% = 20.289909ms, 95% = 23.106114ms, 99% = 27.495925ms
Total Elapsed Time by Commands = 271.684142348s

[ext4]
kind = Put(1048576), count = 15000, 50% = 5.44761ms, 90% = 9.119157ms, 95% = 11.146612ms, 99% = 114.518944ms
kind = Get(1048576), count = 10000, 50% = 23.995165ms, 90% = 28.776795ms, 95% = 29.874225ms, 99% = 32.188337ms
[Overall 25000] 50% = 8.033062ms, 90% = 26.958827ms, 95% = 28.95035ms, 99% = 85.097726ms
Total Elapsed Time by Commands = 360.208530248s


[xfs]
kind = Put(1048576), count = 15000, 50% = 3.76872ms, 90% = 6.891476ms, 95% = 7.317601ms, 99% = 8.667911ms
kind = Get(1048576), count = 10000, 50% = 28.272845ms, 90% = 44.706017ms, 95% = 47.557584ms, 99% = 56.569313ms
[Overall 25000] 50% = 4.776497ms, 90% = 36.041508ms, 95% = 43.356859ms, 99% = 51.603994ms
Total Elapsed Time by Commands = 361.218953556s
```

## 注意
シーケンシャルアクセスでないワークロドのベンチマークの結果は、
ファイルシステムの上の論理位置と物理的な位置のmappingがファイルシステム毎に異なる可能性があり、
実際のアクセスパターンが類型でないため公平性に欠ける（とはいえ、一般的に良い方法が分からない）。
