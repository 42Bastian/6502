DISCLAIMER:
Main part is taken from F#READY's sizecoding introduction adapted for 65C02/Lynx.

Checkout:
https://github.com/FreddyOffenga/sizecoding

# 65C02 overview
- 67 instructions
- No multiply, divide
- No trig instructions
- Three 8-bit registers from which only one can do arithmetics
- 1..4 MHz (even more today)
---

## Extended instrutions

For those coming over from 6502/6510, some nice new instructions are added to the 65C02:

* `stz` - Store Zero to ZP, absolute and indexed ZP and absolute
* `phy`/`ply` and `phx`/`plx` - no more destroing of `A` for pushes
* `bra` - finally relative branch always
* `trb`/`tsb` - Test and reset or set memory
* ZP indirect without `Y`, so no need for a `ldy #0`
* `inc A`/`dec A` - well ...

### Branch vs jump

Absolute jump

```
    jmp label	    ; 3
```

Relative branch 

```
    bra label	    ; 2
```

---
### Inline subroutine

```
	...main code...
	jsr subroutine	; 3

subroutine
	...subroutine code...
	rts				; 1
```

Saves 4 bytes

```
	...main code...
	...subroutine code...
```
---
### Loops
Countup

```
	org $8000
	
	ldx #0      ; 2
next
	lda foo,x   ; 3
	...
	inx         ; 1
	cpx #10	    ; 2
	bne next    ; 2
```
10 bytes

---
### Loops
Countdown

```
	org $8000
	
	ldx #9      ; 2
next
	lda foo,x   ; 3
	...
	dex         ; 1
	bpl next    ; 2
```
8 bytes

---	
### Loops
Countdown on zeropage

```
	org $80
	
	ldx #9      ; 2
next
	lda foo,x   ; 2 - zp,x
	...
	dex         ; 1
	bpl next    ; 2
```
7 bytes

---
### Copy 
Copy <= 129 bytes

```
	ldx #128
copy
	lda from,x		; from [128..0]
	sta to,x
	dex
	bpl copy
```
Easy!

---
### Copy
Copy 256 bytes

```
	ldx #0
copy
	lda from,x		; from [0, 255..1]
	sta to,x
	dex
	bne copy
```
Easy!

---
### Copy
Copy 129 to 256 bytes (e.g. 200)

```
	ldx #199
copy
	lda from,x		; from [199..0]
	sta to,x		; to [199..0]
	dex
	cpx #255		; want to include 0
	bne copy
```
Upcount doesn't help

---
### Copy
Getting rid of cpx

```
	ldx #200
copy
	lda from-1,x	; from [199..0]
	sta to-1,x
	dex
	bne copy
```
Saved 2 bytes

---
### Copy
Do not worry about extra bytes copied

```
	ldx #0
copy
	lda from,x		; from [0, 255..1]
	sta to,x
	dex
	bne copy
```
Copy 256 bytes is easy!

---
### Re-use x,y

```
	ldx #0
loop1
	... using x...
	ldy #0
loop2
	... using y...
	dey
	... need another index register :(
	
	bne loop2
	bne loop1
```
Out of registers already?

---
### Re-use x,y
Use the stack?

```
	... need another index register :(
	phy
	... use y again
	ply
```
2 bytes but slow

---
### Re-use x,y

Use zeropage

```
	... need another index register :(
	sty temp_y
	... use y again
	ldy temp_y
```
4 bytes + temp_y on zero page

---
### Re-use x,y
Self-modifying code

```
	... need another index register :(
	sty here
	... use y again
here = *+1
	ldy #0
```
4 bytes

---
### Initialise variables
Schoolbook example

```
some_var
	db 0               ; 1
	
	lda #42             ; 2
	sta some_var        ; 2 (zp)
	...
	
	lda some_var        ; 2 (zp)
	...
```
7 bytes

---
### Initialise variables
Self-modifying code, zero page

```
some_var = *+1
	lda #42             ; 2
	...

	lda some_var        ; 2
	...
```
4 bytes

---
### Add/substract

```
	lda some_var        ; 2
	clc                 ; 1
	adc #3              ; 2
	sta some_var        ; 2
```
Adding 1, 2 or 3? Use inc

```
	inc some_var        ; 2
	inc some_var        ; 2
	inc some_var        ; 2
```

Substract? Same...

---
### Use register state

```
	ldx #10
loop
	...
	dex
	bpl loop

	lda #$ff
	sta some_var
```
Can we use something else?

---
### Use register state

```
	ldx #10
loop
	...
	dex
	bpl loop

	stx some_var	; x = $ff after bpl
```
Saved 2 bytes

---
### Combine loops

Two loops (17 bytes)

```
	ldx #0
p1
  txa
	sta table256,x		; fill page 0..255
	inx
	bne p1

;	ldx #0  Use state of X
  lda #42
p2
	sta page42,x		; fill with 42
	inx
	bne p2
```

---
### Combine loops

One loop (14 bytes)

```
	ldx #0
p1
	txa
	sta table256,x		; fill page 0..255
  lda #42
	sta page42,x		; fill with 42
	inx
	bne p1
```
Saved 3 bytes

---
### Save branch

Typical if-then-else

```
	cmp #10			; 2
	beq _else		; 2
	ldx #10			; 2
	bra _endif		; 2
_else
	ldx #11			; 2
_endif
	; now use X
```
Total: 10 Bytes

#### Use _side-effect_ free opcode, for example `bit <absolute>`

```
	cmp #10			; 2
	beq _else		; 2
	ldx #10			; 2
	DB OPCODE_BIT_ABS	; 1
_else
	ldx #11			; 2
	; now use X
```

Total: 9 Bytes

The _if_ branch is:
```
	cmp #10			; 2
	beq _else		; 2
	ldx #10			; 2
	bit $0ca2
	; now use X
```

Important is that we do not need Z,N or V flag as `bit <abs>` will change it.

---
### Undocumented opcodes

There are none on the 65C02 besides multi-byte/multi-cycle NOPs
