#include <stdio.h>
#include <stdlib.h>
// Costantes
#define UTILIZACAO 0.2    // taxa de utilização da fila (rho)
#define TAXA_DE_SERVICO 1 // taxa da variável do serviço(mi)

// Estruturas

// representação de um evento
typedef struct evento
{
    int tempo_de_ocorrencia;
    int tipo;
} Evento;

// represatção da lista de eventos
typedef struct heap_de_eventos
{
    Evento **lista; // array de representação da heap
    int ultimo_index_preenchido;
    int tamanho_heap;

} Heap;

// Variáveis globais
double tempo_atual = 0.0; // tempo de simulação
Heap lista_eventos;

// Funções de Tratamento de Evento

void criar_evento(int tempo, int tipo, Evento **evento)
{
    (*evento) = (Evento *)malloc(sizeof(Evento *));
    (*evento)->tempo_de_ocorrencia = tempo;
    (*evento)->tipo = tipo;
}

// Funções de Tratamento da heap

// inicializa heap
void inicializa_heap(int tam, Heap **heap)
{
    (*heap) = (Heap *)malloc(sizeof(Heap *));
    (*heap)->lista = malloc(tam * sizeof(Evento *));
    (*heap)->ultimo_index_preenchido = -1;
    (*heap)->tamanho_heap = tam;
}

// verifica se o nó esta na raiz
short is_raiz_index(int no_index)
{
    return no_index == 0 ? 1 : 0;
}

// retorna o nó parent
int get_parent_index(int index_child)
{
    if (!index_child) // nó é a raiz
    {
        return -1;
    }
    else
    {
        int index_parent = (index_child - 1) / 2;
        return index_parent;
    }
}

// retorna a left child
int get_left_child_index(int index_parent)
{
    int left_child_index = 2 * index_parent + 1;
    return left_child_index;
}

// retorna a right child
int get_right_child_index(int index_parent)
{
    int right_child_index = 2 * (index_parent + 1);
}

// reordena a heap após uma inserção
void ordena_heap(Heap *heap)
{
    short is_not_raiz;
    int index_parent, index_novo_elemento = heap->ultimo_index_preenchido;
    Evento *novo_evento, *parent, *temporario;

    novo_evento = heap->lista[index_novo_elemento]; // ultimo evento adicionado

    do
    {

        if (is_raiz_index(index_novo_elemento))
        {
            // termina se chegou na raiz
            return;
        }

        index_parent = get_parent_index(index_novo_elemento);
        parent = heap->lista[index_parent];

        if (parent->tempo_de_ocorrencia > novo_evento->tempo_de_ocorrencia)
        {
            // troca o parent e a child de lugar
            heap->lista[index_parent] = novo_evento;
            heap->lista[index_novo_elemento] = parent;

            index_novo_elemento = index_parent; // atualiza a posição do novo elemento

            continue;
        }
        break;

    } while (1);
}

// insere o novo elemento logo após o ultima folha
void insere_na_ultima_folha(Evento *novo_evento, Heap *heap)
{
    int prox_index = heap->ultimo_index_preenchido + 1;
    heap->lista[prox_index] = novo_evento;
    heap->ultimo_index_preenchido = prox_index; // atualiza a posição do ultimo elemento
}

//verifica se há espaço na heap
short is_heap_cheia(Heap *heap)
{
    return heap->tamanho_heap == heap->ultimo_index_preenchido + 1;
}

//aloca mais espaço para a heap
void realoca_lista_heap(Heap* heap){
    Evento **nova_lista;
    
    heap->tamanho_heap = 2 * heap->tamanho_heap;//aumenta o tamanho da lista
    nova_lista = (Evento **)malloc(heap->tamanho_heap * sizeof(Evento *));
    //transfere os eventos para a nova lista
    for (size_t i = 0; i < heap->tamanho_heap/2; i++)
    {
        nova_lista[i] = heap->lista[i];
    }
    free(heap->lista);//desaloca a lista antiga
    heap->lista = nova_lista;
}

// insere um novo elemento na heap
void inserir_evento(Evento *novo_evento, Heap *heap)
{
    if (is_heap_cheia(heap))
    {
        realoca_lista_heap(heap);
    }
    
    insere_na_ultima_folha(novo_evento, heap); // insere o novo elemento logo após o ultima folha
    ordena_heap(heap);
}

int main(int argc, char const *argv[])
{
    Heap *teste;
    Evento *teste2;

    inicializa_heap(10, &teste);
    for (size_t i = 0; i < 20; i++)
    {

        int t;
        scanf("%d", &t);

        criar_evento(t, 0, &teste2);

        inserir_evento(teste2, teste);

        for (size_t j = 0; j < teste->ultimo_index_preenchido + 1; j++)
        {
            printf("%d ", teste->lista[j]->tempo_de_ocorrencia);
        }
    }

    return 0;
}
