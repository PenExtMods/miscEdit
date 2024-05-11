# miscEdit

用于编辑词典笔的`Misc`分区内容的工具.

## 适配

|型号|状态|
|---|---|
|ydp021|支持|
|ydp022|支持|

## Misc结构

|偏移|名称|内容|大小|
|---|---|---|---|
|0x800|A/B slot data|ab槽的元数据|32|
|0x1800|Wipe command|清除`userdata`分区的命令|20|
|0x4000|Bootloader message|`Recovery`和`Bootloader`之间的共享数据|2048|
|0x100000|Update info|ab槽的版本信息和更新信息|512|

## Misc作用

- bootloader启动a槽或b槽的依据
- 有ab各槽的版本信息
- 决定下次启动是否清除`userdata`分区
- ...

## 使用方法

```
Usage: miscedit [option]
Option:
  -f (miscFile)               specify the file to edit. if not specify it will use "/dev/block/by-name/misc".
  -r                          read all info from misc.
  -w (name)=(value)           write value to property not for solt.
  -ws (slot),(name)=(value)   write value to property for slot.
  --no-crc32                  no update crc32 value after modify misc.
Example:
  miscedit -f ./misc.bin -ws 2,priority=14
  miscedit -w update.info=hello\ world!
  miscedit --no-crc32 -w crc32=112233ff
  miscedit -w update.info=test -ws 0,priority=15 -ws 0,tries_remaining=3
```

## 构建

环境要求: `linux`, 架构不限(你想在笔上使用则是`aarch64`)

```bash
sudo apt update
sudo apt install gcc g++ -y
chmod +x ./build.sh
./build.sh
```