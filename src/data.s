.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, 8
		.word 0 ; terminator
.endmacro

.segment "PRG0"

inclz4 _CHR0, "chr/0.lz4"
inclz4 _BOBY, "chr/Boby.lz4"

.macro inclevel symbol, file
	.export symbol
	symbol: .incbin file
.endmacro

inclevel _MAP_SPLASH, "map/splash.bin"
inclevel _MAP_LEVEL1, "map/Level1.bin"
inclevel _MAP_LEVEL2, "map/Level2.bin"
inclevel _MAP_LEVEL3, "map/Level3.bin"
inclevel _MAP_LEVEL4, "map/Level4.bin"

; inclevel _MAP_LEVEL5, "map/Level5.bin"
; inclevel _MAP_LEVEL6, "map/Level6.bin"
; inclevel _MAP_LEVEL7, "map/Level7.bin"
; inclevel _MAP_LEVEL8, "map/Level8.bin"
; inclevel _MAP_LEVEL9, "map/Level9.bin"
