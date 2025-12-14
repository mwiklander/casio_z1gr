      ORG  2000H

START:
      ; Read initial timer0 counter
      MOV  DX,0030H
      IN   AX,DX
      MOV  BX,AX

      ; Wait a bit for it to change (in case it’s already running)
      MOV  CX,4000H
WAIT1:
      IN   AX,DX
      CMP  AX,BX
      JNE  GOTICK
      LOOP WAIT1

      ; If no change seen, try to start/configure timer0
      ; intervalA = 1000h (arbitrary)
      MOV  DX,0032H
      MOV  AX,1000H
      OUT  DX,AX

      ; control = 8001h (enable + “repeat” bit0 guess)
      MOV  DX,0036H
      MOV  AX,8001H
      OUT  DX,AX

      ; Try again: wait for counter to change
      MOV  DX,0030H
      IN   AX,DX
      MOV  BX,AX
      MOV  CX,8000H
WAIT2:
      IN   AX,DX
      CMP  AX,BX
      JNE  GOTICK
      LOOP WAIT2

      ; If we still didn’t see it tick, just return quietly
      IRET

GOTICK:
      ; Beep “tick” to confirm timer is alive
      MOV  DX,0206H
      MOV  AL,03H
      OUT  DX,AL
      MOV  AL,00H
      OUT  DX,AL

      IRET
