        ��  ��                  �   (   P A T C H   ��e     0         #engine [G2A]
	#patch
		#assembler [0x00791985]
			push eax
			push ecx
			push edx

			push dword ptr [esp+0x60]
			call $DoStackHook

			pop edx
			pop ecx
			pop eax

			orgcode
		#/assembler
	#/patch
#/engine
 