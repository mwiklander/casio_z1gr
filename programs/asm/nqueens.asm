      ORG  2000H
      MOV  WORD PTR CNT,1000
 L06: MOV  DX,8
      LEA  BX,ARY
      MOV  CX,0
      MOV  SI,0
 L00: CMP  DX,SI
      JZ   L05
      INC  SI
      MOV  [BX+SI],DL
 L01: INC  CX
      MOV  DI,SI
 L02: DEC  DI
      JZ   L00
      MOV  AL,[BX+SI]
      SUB  AL,[BX+DI]
      JZ   L04
      JNC  L03
      NEG  AL
 L03: XOR  AH,AH
      ADD  AX,DI
      SUB  AX,SI
      JNZ  L02
 L04: DEC  BYTE PTR [BX+SI]
      JNZ  L01
      DEC  SI
      JNZ  L04
 L05: DEC  WORD PTR CNT
      JNZ  L06
      MOV  CNT,CX
      IRET
 CNT  DW   0
 ARY  DB   9
      END

