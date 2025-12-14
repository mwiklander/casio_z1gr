      ORG  2000H
START:
      ; Enable many keyboard rows: strobe = 0FFFh (write low then high byte)
      MOV  DX,0200H
      MOV  AX,0FFFH
      OUT  DX,AX

WAIT:
      ; Read 16-bit key state from 0202h/0203h
      MOV  DX,0202H
      IN   AX,DX
      CMP  AX,8000H
      JE   WAIT

      ; Beep ON
      MOV  DX,0206H
      MOV  AL,03H
      OUT  DX,AL

      ; Short delay
      MOV  CX,3000H
DLY:
      LOOP DLY

      ; Beep OFF
      MOV  AL,00H
      OUT  DX,AL

      IRET
      END
