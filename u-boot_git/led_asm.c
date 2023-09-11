ENTRY(kcci_led_test)
        ldr r0,=0xFE200000
        ldr r1,=0x09240000
        str r1,[r0,#0x00]


        ldr r1,=0x00012249
        str r1,[r0,#0x04]

        ldr r1,=0x00000040

        mov r2, #8
ledloop:
        str r1, [r0,#0x1C]
        ldr r3,=0x400000

delay:
        ldr r4, [r0,#0x34]   # gpiolevel 주소의 데이터 r4에 저장
        ldr r5,=0x00010000   # gpio 16이 인식되는 값 r5에 저장
        ands r6,r4,r5        # r4와 r5의 and 연산 후 r6에 저장하고 r6가 0이 아니면 즉 버튼                      을 누르면 bne 실행
        bne value_is_0x10000

        subs r3,r3, #1
        bne delay
        str r1,[r0,#0x28]
        mov r1,r1,LSL #1
        subs r2,r2, #1
        bne ledloop
        mov pc, lr
value_is_0x10000:
        mov pc, lr

ENDPROC(kcci_led_test)