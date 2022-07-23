#include <stdio.h>
#include <stdlib.h>
//!--------------------------------------- Costantes-----------------------------------------------------------------------------------------------
#define UTILIZACAO 0.2    // taxa de utilização da fila (rho)
#define TAXA_DE_SERVICO 1 // taxa da variável do serviço(mi)

#define INSERCAO 1
#define REMOCAO 2

//!-------------------------------------- Estruturas--------------------------------------------------------------------------------------------

// representação de um evento
typedef struct evento
{
    int tempo_de_ocorrencia;
    int tipo;
} Evento;

// represatação da lista de eventos
typedef struct heap_de_eventos
{
    Evento **lista; // array de representação da heap
    int ultimo_index_preenchido;
    int tamanho_heap;

} Heap;

typedef struct cliente
{
    int id;
    double tempo_chegada;
    double tempo_espera;
    double tempo_serviço;
} Cliente;

typedef struct mm1
{
    Cliente **fila_de_espera;
    Cliente *servidor;
    int prox_cliente_id;
    int tamanho_fila_de_espera;
    int numero_elementos;
    double utilizacao;
    double taxa_servico;
} Fila_m_m_1;

//!--------------------------------- Variáveis globais-------------------------------------------------------------------------------
double tempo_atual = 0.0; // tempo de simulação
Heap *lista_eventos;
Fila_m_m_1 *fila;

//!--------------------------------- Funções de Tratamento de Evento--------------------------------------------------------------

// cria uma instancia de evento
void criar_evento(int tempo, int tipo, Evento **evento)
{
    (*evento) = (Evento *)malloc(sizeof(Evento *));
    (*evento)->tempo_de_ocorrencia = tempo;
    (*evento)->tipo = tipo;
}

// retorna 1 se o evento passado como primeiro parametro antecede o passado como segundo, senão 0
short antecede_evento(Evento *evento_1, Evento *evento_2)
{
    return evento_1->tempo_de_ocorrencia < evento_2->tempo_de_ocorrencia ? 1 : 0;
}

//!-------------------------------- Funções de Tratamento da heap----------------------------------------------------------------

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

// troca 2 elementos de uma heap de lugar
void trocar_2_elementos(Evento *elemento_1, int index_elemento_1, Evento *elemento_2, int index_elemento_2, Heap *heap)
{
    heap->lista[index_elemento_1] = elemento_2;
    heap->lista[index_elemento_2] = elemento_1;
}

// ordenação de uma heap após inserção
void ordena_apos_insercao(Heap *heap)
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

        if (antecede_evento(novo_evento, parent))
        {
            // troca o parent e a child de lugar
            trocar_2_elementos(parent, index_parent, novo_evento, index_novo_elemento, heap);

            index_novo_elemento = index_parent; // atualiza a posição do novo elemento

            continue;
        }
        break;

    } while (1);
}

// acha o menor dos filhos de um nó
int achar_index_menor_crianca(int index_parent, Heap *heap)
{
    Evento *left_child, *right_child;
    int index_left_child, index_right_child;
    short existe_left_child, existe_right_child;

    index_left_child = get_left_child_index(index_parent);
    index_right_child = get_right_child_index(index_parent);

    existe_left_child = index_left_child <= heap->ultimo_index_preenchido;
    existe_right_child = index_right_child <= heap->ultimo_index_preenchido;

    left_child = existe_left_child ? heap->lista[index_left_child] : NULL;
    right_child = existe_right_child ? heap->lista[index_right_child] : NULL;

    if (existe_left_child && existe_right_child)
    {

        return antecede_evento(left_child, right_child) ? index_left_child : index_right_child;
    }
    else if (existe_left_child && existe_right_child)
    {
        return existe_left_child ? index_left_child : index_right_child;
    }
    else
    {
        return -1;
    }
}

// ordenação de uma heap após remoção
void ordena_apos_remocao(Heap *heap)
{
    int index_atual = 0, index_menor_crianca;
    short nao_existe_menor_crianca;
    Evento *raiz, *menor_crianca;
    raiz = heap->lista[index_atual];
    while (index_atual != heap->ultimo_index_preenchido)
    {
        index_menor_crianca = achar_index_menor_crianca(index_atual, heap);
        nao_existe_menor_crianca = index_menor_crianca == -1;
        if (nao_existe_menor_crianca)
        {
            return;
        }

        menor_crianca = heap->lista[index_menor_crianca];
        if (antecede_evento(menor_crianca, raiz))
        {
            trocar_2_elementos(raiz, index_atual, menor_crianca, index_menor_crianca, heap);
            index_atual = index_menor_crianca;
            continue;
        }
        return;
    }
}

// Executa a operação de reordenação dependendo se foi uma inserção ou remoção
void ordena_heap(Heap *heap, int operacao)
{
    switch (operacao)
    {
    case INSERCAO:
        ordena_apos_insercao(heap);
        break;
    case REMOCAO:
        ordena_apos_remocao(heap);
        break;
    }
}

// insere o novo elemento logo após o ultima folha
void insere_na_ultima_folha(Evento *novo_evento, Heap *heap)
{
    int prox_index = heap->ultimo_index_preenchido + 1;
    heap->lista[prox_index] = novo_evento;
    heap->ultimo_index_preenchido = prox_index; // atualiza a posição do ultimo elemento
}

// verifica se há espaço na heap
short is_heap_cheia(Heap *heap)
{
    return heap->tamanho_heap == heap->ultimo_index_preenchido + 1;
}

// aloca mais espaço para a heap
void realoca_lista_heap(Heap *heap)
{
    Evento **nova_lista;

    heap->tamanho_heap = 2 * heap->tamanho_heap; // aumenta o tamanho da lista
    nova_lista = (Evento **)malloc(heap->tamanho_heap * sizeof(Evento *));
    // transfere os eventos para a nova lista
    for (size_t i = 0; i < heap->tamanho_heap / 2; i++)
    {
        nova_lista[i] = heap->lista[i];
    }
    free(heap->lista); // desaloca a lista antiga
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
    ordena_heap(heap, INSERCAO);
}

// remove o próximo evento da heap
Evento *remover_evento(Heap *heap)
{
    Evento *evento_removido, *ultima_folha;

    evento_removido = heap->lista[0];
    ultima_folha = heap->lista[heap->ultimo_index_preenchido];
    trocar_2_elementos(evento_removido, 0, ultima_folha, heap->ultimo_index_preenchido, heap);
    heap->ultimo_index_preenchido -= 1;
    ordena_heap(heap, REMOCAO);
    return evento_removido;
}

//!-------------------------------------------------Funções de tratamento da fila----------------------------------------------------------------------------------------

// cria uma instancia de fila M/M/1
void criar_fila(int tam, Fila_m_m_1 **fila)
{
    (*fila) = (Fila_m_m_1 *)malloc(sizeof(Fila_m_m_1 *));
    (*fila)->fila_de_espera = (Cliente **)malloc(tam * sizeof(Cliente *));
    (*fila)->prox_cliente_id = 1;
    (*fila)->tamanho_fila_de_espera = tam;
    (*fila)->numero_elementos = 0;
    (*fila)->servidor = NULL;
    (*fila)->taxa_servico = TAXA_DE_SERVICO;
    (*fila)->utilizacao = UTILIZACAO;
}

// aloca mais espaço para a fila de espera
void realoca_fila_de_espera(Fila_m_m_1 *fila)
{
    Cliente **nova_lista;
    fila->tamanho_fila_de_espera = 2 * fila->tamanho_fila_de_espera;// aumenta o tamanho da lista

    nova_lista = (Cliente **)malloc(fila->tamanho_fila_de_espera * sizeof(Cliente *));
    // tranfere os clientes para a nova fila;
    for (size_t i = 0; i < fila->tamanho_fila_de_espera/2; i++)
    {
        nova_lista[i] = fila->fila_de_espera[i];
    }
    free(fila->fila_de_espera);
    fila->fila_de_espera = nova_lista;
}
int main(int argc, char const *argv[])
{
    Heap *teste;
    Evento *teste2;

    inicializa_heap(10, &teste);
    for (size_t i = 0; i < 10; i++)
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
    remover_evento(teste);
    for (size_t j = 0; j < teste->ultimo_index_preenchido + 1; j++)
    {
        printf("\n%d ", teste->lista[j]->tempo_de_ocorrencia);
    }

    return 0;
}
