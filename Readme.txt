PWM을 18번 핀으로 LED 켜는 시나리오
PWM 18-> ALT5번이 pwm0출력임 PWM의 채널1이 pwm0라고 가정 --> 아직 어느것인지 확>인하지 않았음 추후 수정필요

물리번호 12번, 32번, 33번, 35번 이 4핀만이_PWM출력가능

PWM출력 방법
1- 하드웨어PWM : 하드웨어에서 출력발생. data레지스터에 지정하면 지속적으로 펄스 발생
2 - 소프트웨어PWM : 타이머 인터럽트를 이용한 방법으로, 지속적으로 프로그램의 제>어가 필요. -> 멀티태스킹시 문제 발생 가능성 있음

1. PWM_CTL reg
(31:16)        (15 MSEN2)(14) (13 USEF2) (12 POLA2) (11 SBIT2) (10 RPTL2) (9 MODE2) (8 PWEN2) (7 MSEN1) (6 CLRF1) (5 USEF1) (4 POLA1) (3 SBIT1) (2 RPTL1) (1 MODE1) (0 PWEN1)
xxxxxxxxxxxxxxx    x       x      x          x          x            x         x     x          0           0         1     0         0      1           0          1
