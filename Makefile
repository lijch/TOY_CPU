all:generate_display generate_micro_instruction generate_instruction

clean:
	rm -rf ./out

generate_display: rom_utils.c micro_instruction.c
	mkdir -p ./out
	gcc -o ./out/segment_display rom_utils.c segment_display.c
	cd ./out && ./segment_display

generate_micro_instruction: rom_utils.c micro_instruction_new.c
		mkdir -p ./out
	gcc -o ./out/micro_instruction rom_utils.c micro_instruction_new.c
	cd ./out && ./micro_instruction

generate_instruction: rom_utils.c instruction.c
	mkdir -p ./out
	gcc -o ./out/instruction rom_utils.c instruction.c
	cd ./out && ./instruction
