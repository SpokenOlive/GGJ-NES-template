.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, 8
		.word 0 ; terminator
.endmacro

.segment "PRG0"

inclz4 _CHR0, "chr/0.lz4"
inclz4 _BOBY, "chr/Boby.lz4"

.export _MAP_SPLASH
_MAP_SPLASH: .incbin "map/splash.bin"
