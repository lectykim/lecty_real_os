# lectyos

lectyos는 Lecty Kim의 OS Test와 파일 시스템 논문 구현을 위해 만들어둔 sample 운영체제입니다.
리눅스 코드 0.11을 베이스로 개발할 예정이지만, 세세한 아키텍팅은 근거있게 해보려고 합니다.

## target hardware

* cpu: Intel i386(80386)
* ram: 64MB
* BIOS: SeaBIOS
* video: cirrus logic clgd 5446 PCI VGA
* storage: floppy(1.44MB)
* PIC: i8259 PIC pair

---

## Intro

/document 폴더에서 코드에 대한 설명을 확인할 수 있습니다.

