#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>

typedef void (*finalizer_t)(void *ptr, size_t size);

// Двусвязный список для хранения аллокации
struct Allocation {
    void *ptr; // указатель на память
    size_t size; // размер аллокации
    finalizer_t finalizer; // указатель на функцию, вызываемую при освобождении памяти
    bool alive; // флаг, отмечающий, нужно ли сохранить аллокацию при очередном проходе по памяти
    struct Allocation *next;
    struct Allocation *prev;
};

void gc_collect(); // реализация в обертке wrapper.S

void add_allocation(struct Allocation **head, struct Allocation **alloc) {
    if (*head != NULL) {
        (*head)->prev = *alloc;
    }
    (*alloc)->next = *head;
    (*alloc)->prev = NULL;
    *head = *alloc;
}

void remove_allocation(struct Allocation **head, struct Allocation **alloc) {
    if ((*alloc)->next != NULL) {
        (*alloc)->next->prev = (*alloc)->prev;
    }
    if ((*alloc)->prev != NULL) {
        (*alloc)->prev->next = (*alloc)->next;
    }
    if (*head == *alloc) {
        *head = (*head)->next;
    }
    free(*alloc);
    *alloc = NULL;
}

// Проверяем, находится ли память по указателю ptr внутри аллокации по указателю alloc
bool points_to(void *ptr, struct Allocation *alloc) {
    uintptr_t uptr = (uintptr_t)ptr, aptr = (uintptr_t)alloc->ptr;
    return (uptr >= aptr) && (uptr - aptr <= alloc->size);
}

// Основная структура, хранящая список аллокаций
// Объявляется только один глобальный объект gc
struct GarbageCollector {
    uintptr_t stack_bottom; // нижняя граница стека
    struct Allocation *allocations;
};

struct GarbageCollector gc;

// Инициализация коллектора. argv - указатель на нижнюю границу стека
void gc_init(char **argv) {
    gc.stack_bottom = (uintptr_t)argv;
    gc.allocations = NULL;
}

// Аналог malloc, добавляет аллокацию в коллектор
void *gc_malloc(size_t size, finalizer_t finalizer) {
    void *memory = malloc(size);
    if (memory == NULL) {
        return NULL;
    }
    struct Allocation *alloc = malloc(sizeof(struct Allocation));
    if (alloc == NULL) {
        free(memory);
        return NULL;
    }
    alloc->ptr = memory;
    alloc->size = size;
    alloc->finalizer = finalizer;
    alloc->alive = true;
    add_allocation(&gc.allocations, &alloc);
    return memory;
}

// Проход по памяти для разметки неосвобожденных аллокаций
void liven(uintptr_t start, uintptr_t end) {
    for (uintptr_t addr = start; addr + sizeof(void *) <= end; addr += alignof(void *)) {
        for (struct Allocation *cur = gc.allocations; cur != NULL; cur = cur->next) {
            if (!cur->alive && points_to(*(void **)addr, cur)) {
                cur->alive = true;
                liven((uintptr_t)cur->ptr, (uintptr_t)cur->ptr + cur->size);
            }
        }
    }
}

void gc_collect_impl(uintptr_t stack_top) {
    for (struct Allocation *cur = gc.allocations; cur != NULL; cur = cur->next) {
        cur->alive = false;
    }
    liven(stack_top, gc.stack_bottom);
    struct Allocation *cur = gc.allocations;
    while (cur != NULL) {
        struct Allocation *old_cur = cur;
        cur = cur->next;
        if (!old_cur->alive) {
            (*old_cur->finalizer)(old_cur->ptr, old_cur->size);
            free(old_cur->ptr);
            remove_allocation(&gc.allocations, &old_cur);
        }
    }
}