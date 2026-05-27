extern "C" {
    __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long nStdHandle);
    __declspec(dllimport) int __stdcall WriteFile(void* hFile, const void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten, void* lpOverlapped);
    __declspec(dllimport) void __stdcall ExitProcess(unsigned int uExitCode);
}

extern "C" int main() {
    asm(
        // 1. Rezerwacja przestrzeni na stosie (shadow space + miejsce na zmienn¹ bytesWritten)
        // sub $48, %rsp -> 32 bajty (shadow space) + 8 bajtów (lpOverlapped) + 8 bajtów (bytesWritten)
        "sub $48, %%rsp\n" 

        // 2. Pobranie uchwytu standardowego wyjœcia: GetStdHandle(-11)
        "mov $-11, %%rcx\n"
        "call GetStdHandle\n"
        "mov %%rax, %%rcx\n"          // hFile (pierwszy argument WriteFile)

        // 3. Przygotowanie wskaŸnika do tekstu za pomoc¹ skoku (trik na lokalny adres)
        "lea message_text(%%rip), %%rdx\n" // lpBuffer (drugi argument)
        "mov $34, %%r8d\n"            // nNumberOfBytesToWrite (trzeci argument - d³ugoœæ tekstu)
        
        // 4. bytesWritten umieszczamy na bezpiecznym miejscu na stosie
        "lea 40(%%rsp), %%r9\n"       // lpNumberOfBytesWritten (czwarty argument)
        
        // 5. lpOverlapped = NULL (pi¹ty argument przekazywany przez stos)
        "movq $0, 32(%%rsp)\n"        
        
        // 6. Wywo³anie WriteFile
        "call WriteFile\n"

        // 7. Wyjœcie z programu: ExitProcess(0)
        "xor %%ecx, %%ecx\n"
        "call ExitProcess\n"

        // Sekcja danych wstrzykniêta bezpoœrednio za kodem wykonywalnym
        ".section .text\n"
        "message_text:\n"
        ".ascii \"Czesc z assemblera na Windows 11!\\n\"\n"
        :
        :
        : "rax", "rcx", "rdx", "r8", "r9", "memory"
    );
    return 0;
}