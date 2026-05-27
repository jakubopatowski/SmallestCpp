cmake_minimum_required(VERSION 3.13)
project(MinimalnyProjekt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definicja opcji. Domyœlnie jest wy³¹czona (OFF).
# U¿ytkownik mo¿e j¹ w³¹czyæ, wpisuj¹c: -DOPTIMIZE_SIZE=ON
option(OPTIMIZE_SIZE "W³¹cz agresywn¹ optymalizacjê rozmiaru pliku wykonywalnego" ON)

add_executable(program main.cpp)

# Sprawdzamy, czy u¿ytkownik w³¹czy³ opcjê OPTIMIZE_SIZE
if(OPTIMIZE_SIZE)
    message(STATUS ">>> Agresywna optymalizacja rozmiaru: W£¥CZONA <<<")

    # 1. W³¹czenie LTO / IPO (globalna optymalizacja linkera)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_property(TARGET program PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    # 2. Flagi dla GCC / Clang
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(program PRIVATE -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti)
        target_link_options(program PRIVATE -Wl,--gc-sections -s)

    # 3. Flagi dla MSVC (Windows)
    elseif(MSVC)
        target_compile_options(program PRIVATE /GR- /EHs-c-)
        target_link_options(program PRIVATE /OPT:REF /OPT:ICF)
    endif()
else()
    message(STATUS ">>> Agresywna optymalizacja rozmiaru: WY£¥CZONA (Standardowe budowanie) <<<")
endif()

if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WIN32)
        target_link_libraries(program PRIVATE kernel32)
        # Wy³¹czenie ochrony przed przepe³nieniem bufora (usuwa zale¿noœæ od __security_cookie)
        target_compile_options(program PRIVATE "/GS-")
        # 1. Definiujemy w³asny punkt wejœcia (odcinamy CRT)
        target_link_options(program PRIVATE "/ENTRY:main")
        
        # 2. Informujemy, ¿e nie chcemy linkowaæ domyœlnych bibliotek uruchomieniowych
        target_link_options(program PRIVATE "/NODEFAULTLIB")
        
        # 3. Dodatkowe agresywne flagi optymalizacji sekcji pliku PE (Windows)
        target_link_options(program PRIVATE "/ALIGN:512") # Zmniejsza wyrównanie sekcji w pliku

        # £¹czymy sekcjê danych tylko do odczytu z sekcj¹ kodu w jeden blok
        target_link_options(program PRIVATE "/MERGE:.rdata=.text")
    endif()
endif()