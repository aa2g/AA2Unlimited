.386
.model flat, stdcall

.code

EXTERNDEF threadinjectproc:PROC

;DWORD WINAPI ThreadProc(
;  _In_ LPVOID lpParameter
;);
;so, this is a stdcall
;parameter will be pointer to
;struct {
; HMODULE(*LoadLibraryPtr)(LPCSTR);
; void (*GetLastErrorPtr)()
; TCHAR dllname[]
;}
;the loadlibraryptr will be replaced by GetLastError value if the call fails
threadinjectproc:
	mov ebx, [esp+04] ; get parameter
	lea eax, [ebx+8]
	push eax ; dll name
	mov eax, [ebx]
	call eax
	test eax,eax
	jnz threadinjectproc_retsuccess
	mov eax, [ebx+4]
	call eax
	mov [ebx], eax
	xor eax,eax
	ret 4
  threadinjectproc_retsuccess:
	mov eax, 1
	ret 4

	int 3
	int 3
	int 3
	int 3
	int 3
	int 3
	int 3
	int 3

END