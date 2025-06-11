#!/bin/bash

set -e

IMG="floppy.img"

SECTOR_SIZE=512
TOTAL_SECTORS=2880

BOOTSECT="boot/bootsect"
SETUP="boot/setup"
KERNEL="tools/system"

#플로피 디스크 초기화
dd if=/dev/zero of="$IMG" bs=$SECTOR_SIZE count=$TOTAL_SECTORS

#부트 섹터 기록
dd if="$BOOTSECT" of="$IMG" bs="$SECTOR_SIZE" seek=0 conv=notrunc

#설정 코드 크기(바이트) 와 섹터 수 계산
SETUP_SIZE=$(stat -c%s "$SETUP")

#섹터 수: 정수 올림
SETUP_SECTORS=$(( (SETUP_SIZE + SECTOR_SIZE -1)/SECTOR_SIZE))
echo "Setup size: $SETUP_SIZE bytes, sectors: $SETUP_SECTORS"

#설정 코드 기록(seek=1)
dd if="$SETUP" of="$IMG" bs=$SECTOR_SIZE seek=1 conv=notrunc

KERNEL_SIZE=$(stat -c%s "$KERNEL")
KERNEL_SECTORS=$(( (KERNEL_SIZE+SECTOR_SIZE-1)/SECTOR_SIZE))
echo "Kernel size: $KERNEL_SIZE bytes, sectors: $KERNEL_SECTORS"

#커널 기록
dd if="$KERNEL" of="$IMG" bs=$SECTOR_SIZE seek=5 conv=notrunc

echo "Image '$IMG' created: bootsector + setup ($SETUP_SECTORS sectors) + kernel ($KERNEL_SECTORS sectors)"