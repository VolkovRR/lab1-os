#pragma once

#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// Структура для представления элемента очереди
typedef struct node_s {
    void* data;
    struct node_s* next;
} node_t;

// Структура для представления очереди
typedef struct {
    node_t* head; // Указатель на начало очереди
    node_t* tail;  // Указатель на конец очереди
    int size;
} queue_t;

// Функция для инициализации очереди
void initialize_queue(queue_t* queue);

// Функция для добавления элемента в конец очереди
int enqueue(queue_t* queue, void* data);

// Функция для удаления элемента из начала очереди
void* dequeue(queue_t* queue);

// Функция для проверки, пуста ли очередь
int is_empty(queue_t* queue);

#endif