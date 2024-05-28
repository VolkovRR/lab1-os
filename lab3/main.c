#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#define PAGE_SIZE 256
#define COUNT_MEMORY_PAGES 256
#define COUNT_BACKING_STORE_PAGES 256
#define TLB_SIZE 16
#define MEMORY_SIZE PAGE_SIZE * COUNT_MEMORY_PAGES
#define BACKING_STORE_SIZE PAGE_SIZE * COUNT_BACKING_STORE_PAGES 

char memory[MEMORY_SIZE];
char backing_store[BACKING_STORE_SIZE];

int page_table[COUNT_BACKING_STORE_PAGES][2];
int tlb_table[TLB_SIZE][2];
int claim_table[COUNT_MEMORY_PAGES][2];
int tlb_size = 0;
int page_table_size = 0;


int main(int argc, char* argv[]) {
    //Инициализация таблиц: `page_table` и `tlb_table` устанавливаются со значениями -1 (что означает отсутствие данных), 
    //а `claim_table` заполняется нулями.
    memset(page_table, -1, sizeof(int) * COUNT_BACKING_STORE_PAGES * 2);
    memset(tlb_table, -1, sizeof(int) * TLB_SIZE * 2);
    memset(claim_table, 0, sizeof(int) * COUNT_MEMORY_PAGES * 2);
    //Открытие файлов: `file` для чтения файла подкачки в бинарном режиме, `addreses` для чтения виртуальных адресов, 
    //а `result` для записи результатов работы программы.
    FILE* file = fopen("backing_store.bin", "rb");
    FILE* addreses = fopen("addresses.txt", "r");
    FILE* result = fopen("out.txt", "w");

    int tlb_hits = 0;
    int page_faults = 0;
    int total = 0;

    int virtual_address;
    while (fscanf(addreses, "%d", &virtual_address) == 1) {
        int page = virtual_address >> 8;
        int offset = virtual_address & 0x00FF;
        long frame = -1;
        int value;
        int tlb_size_temp = tlb_size;
        int page_table_size_temp = page_table_size;
        //Этот цикл проходит по TLB и проверяет, содержится ли искомая страница в TLB. Если страница найдена, он извлекает 
        //номер фрейма, вычисляет физический адрес, считывает значение из памяти с этого адреса и увеличивает 
        //счетчик попаданий (tlb_hits). Оператор break прерывает цикл, чтобы не искать дальше после нахождения страницы.
        for (int x = 0; x < MIN(tlb_size_temp, TLB_SIZE); x++)
            if (tlb_table[x][0] == page) {
                frame = tlb_table[x][1];
                value = memory[frame * PAGE_SIZE + offset];
                tlb_hits++;
                break;
            }

        if (frame < 0) {//Если страница не найдена в TLB (frame остается равным -1), необходимо выполнить поиск в таблице страниц.
            //Проход по таблице страниц для нахождения нужной страницы. Если страница обнаруживается, 
            //извлекается номер фрейма, и через вычисление физического адреса считывается соответствующее значение.
            for (int x = 0; x < MIN(page_table_size_temp, COUNT_BACKING_STORE_PAGES); x++)
                if (page_table[x][0] == page) {
                    frame = page_table[x][1];
                    value = memory[frame * PAGE_SIZE + offset];
                    break;
                }

            if (frame < 0) {//Если страница не найдена ни в TLB, ни в таблице страниц, необходимо обработать ошибку страницы (page fault).
                //Поиск свободного фрейма в памяти. Если находится неиспользуемый фрейм, 
                //он помечается как используемый (claim_table[x][0] = 1).
                int free_page = -1;
                for (int x = 0; x < COUNT_MEMORY_PAGES; x++)
                    if (!claim_table[x][0]) {
                        free_page = x;
                        claim_table[x][0] = 1;
                        break;
                    }

                if (free_page < 0) {//Если свободных фреймов нет, необходимо запустить алгоритм замены страниц для освобождения места под новую страницу.
                    //Выбор наименее используемой страницы для замены, что из части алгоритма Least Recently Used (LRU).
                    int count_use_min = claim_table[0][1];
                    free_page = 0;
                    for (int x = 1; x < COUNT_MEMORY_PAGES; x++)
                        if (claim_table[x][1] <= count_use_min) {
                            count_use_min = claim_table[x][1];
                            free_page = x;
                        }

                    for (int x = 0; x < MIN(page_table_size_temp, COUNT_BACKING_STORE_PAGES); x++)
                        if (page_table[x][1] == free_page) {
                            page_table[x][0] = -1;
                            memcpy(page_table[x], page_table[(page_table_size - 1) % COUNT_BACKING_STORE_PAGES], 2);
                            page_table_size--;
                        }

                    for (int x = 0; x < MIN(tlb_size_temp, TLB_SIZE); x++)
                        if (tlb_table[x][1] == free_page) {
                            tlb_table[x][0] = -1;
                            memcpy(tlb_table[x], tlb_table[(tlb_size - 1) % TLB_SIZE], 2);
                            tlb_size--;
                        }
    
                }

                frame = free_page;//Номер заменяемого или свободного фрейма присваивается переменной frame для использования.

                fseek(file, page * PAGE_SIZE, SEEK_SET);
                fread(backing_store, 1, BACKING_STORE_SIZE, file);
                memcpy(memory + frame * PAGE_SIZE, backing_store, PAGE_SIZE);

                page_table[page_table_size % COUNT_BACKING_STORE_PAGES][0] = page;
                page_table[page_table_size % COUNT_BACKING_STORE_PAGES][1] = frame;
                page_table_size++;

                tlb_table[tlb_size % TLB_SIZE][0] = page;
                tlb_table[tlb_size % TLB_SIZE][1] = frame;
                tlb_size++;

                claim_table[frame][1]++;

                value = memory[frame * PAGE_SIZE + offset];

                page_faults++;
            }
        }

        int phys_addr = frame * PAGE_SIZE + offset;//Вычисление физического адреса памяти на базе номера фрейма и смещения.

        fprintf(result, "Virtual address: %d Physical address: %d Value: %d\n", virtual_address, phys_addr, value);
        total++;
    }

    float tlb_hit_rate = tlb_hits / (float)total;
    float page_fault_rate = page_faults / (float)total;
    printf("Частота попаданий в TLB: %f\nЧастота ошибок страниц: %f\n", tlb_hit_rate, page_fault_rate);

    fclose(file);
    fclose(addreses);
    fclose(result);

    return 0;
}