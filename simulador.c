#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
//!--------------------------------------- Costantes-----------------------------------------------------------------------------------------------
#define UTILIZACAO 0.2      // taxa de utilização da fila (rho)
#define TAXA_DE_SERVICO 1   // taxa da variável do serviço(mi)
#define NUMERO_RODADAS 3200 // Quantidade de rodadas durante a simulação
#define TAM_RODADA 10       // quantidade de clientes atendidos numa rodada

#define INSERCAO 1
#define REMOCAO 2

#define CHEGADA 1
#define TERMINO_SERVICO 2

#define FCFS 1
#define LCFS 2

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
    int rodada_origem;
    double tempo_chegada;
    double tempo_espera;
    double tempo_servico;
} Cliente;

typedef struct mm1
{
    Cliente **fila_de_espera;
    Cliente *servidor;
    int prox_cliente_id;
    int tamanho_fila_de_espera;
    int numero_elementos_na_fila_de_espera;
    int proximo_a_ser_servido;
    int pos_proxima_insercao;
    double utilizacao;
    double taxa_servico;
} Fila_m_m_1;

typedef struct rodada
{
    double tempo_inicio;
    double tempo_total;
    double media_estimada_tempo_espera;
    double variancia_estimada_tempo_espera;
    double *pmf_rodada;
} Rodada;

typedef struct estrutura_aux_pmf
{
    int tam_vetor_aux_pmf;
    double *vetor_aux_pmf;
    double tempo_ultima_alteracao_na_fila_espera;
} Aux_pmf;

//!--------------------------------- Variáveis globais-------------------------------------------------------------------------------
short disciplina;
int numero_rodada_atual = 0;
double tempo_atual = 0.0;

double somatorio_tempo_espera = 0.0;
double somatorio_tempo_espera_ao_quadrado = 0.0;
Heap *lista_eventos;
Fila_m_m_1 *fila;
Rodada **rodadas, *rodada_atual;
Aux_pmf *auxiliar_calculo_pmf;
//!-------------------------------------------------Funções de Geração de Amostra------------------------------------------------------------------------------------------------//

// Atribui uma nova seed ao gerador de números aleatórios
void randomizar(double seed)
{
    srand(seed);
}

// cria uma amostra exponencial
double gerar_amostra_exponencial()
{
    double uniforme = rand() / (double)RAND_MAX;
    return (double)log(uniforme) / (-UTILIZACAO);
}
//!-------------------------------------------------Funções de Tratamento de Evento----------------------------------------------------------------------------------------------//
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
    (*fila)->numero_elementos_na_fila_de_espera = 0;
    (*fila)->pos_proxima_insercao = 0;
    (*fila)->proximo_a_ser_servido = 0;
    (*fila)->servidor = NULL;
    (*fila)->taxa_servico = TAXA_DE_SERVICO;
    (*fila)->utilizacao = UTILIZACAO;
}

// verifica se há espaço para iserção na fila de espera
short fila_de_espera_cheia(Fila_m_m_1 *fila)
{
    return fila->pos_proxima_insercao == fila->tamanho_fila_de_espera;
}

// verifica se há alguém na fila de espera
short alguem_na_espera(Fila_m_m_1 *fila)
{
    if (disciplina == FCFS)
    {
        return fila->pos_proxima_insercao != fila->proximo_a_ser_servido;
    }
    else
    {
        return fila->proximo_a_ser_servido >= 0;
    }
}

// aloca mais espaço para a fila de espera
void realoca_fila_de_espera(Fila_m_m_1 *fila)
{
    Cliente **nova_lista;
    fila->tamanho_fila_de_espera = 2 * fila->tamanho_fila_de_espera; // aumenta o tamanho da lista

    nova_lista = (Cliente **)malloc(fila->tamanho_fila_de_espera * sizeof(Cliente *));
    // tranfere os clientes para a nova fila;
    for (size_t i = 0; i < fila->tamanho_fila_de_espera / 2; i++)
    {
        nova_lista[i] = fila->fila_de_espera[i];
    }
    free(fila->fila_de_espera);
    fila->fila_de_espera = nova_lista;
}

// insere uma nova chegada na fila de espera
void insere_na_fila(Cliente *chegada, Fila_m_m_1 *fila)
{
    fila->fila_de_espera[fila->pos_proxima_insercao] = chegada;
    fila->pos_proxima_insercao++;
    if (disciplina == LCFS)
    {
        fila->proximo_a_ser_servido = fila->pos_proxima_insercao - 1; // o próximo a ser servido é o elemento que acabou de ser inserido
    }

    fila->numero_elementos_na_fila_de_espera++;
}

// verifica se há alguém no servidor
short is_fila_ocupada(Fila_m_m_1 *fila)
{
    return fila->servidor != NULL;
}

// retorna o primeiro elemento da fila de espera, ou seja, o próximo a ser servido
Cliente *buscar_na_fila_de_espera(Fila_m_m_1 *fila)
{
    Cliente *primeiro_da_fila;
    primeiro_da_fila = fila->fila_de_espera[fila->proximo_a_ser_servido];
    if (disciplina == LCFS)
    {
        fila->pos_proxima_insercao = fila->proximo_a_ser_servido;
        fila->proximo_a_ser_servido--;
    }
    else
    {
        fila->proximo_a_ser_servido++;
    }

    return primeiro_da_fila;
}

// no caso de uma fila de espera vazia o cliente vai diretamente ao servidor
void insercao_direta_no_servidor(Cliente *chegada, Fila_m_m_1 *fila)
{
    fila->servidor = chegada;
}

// O cliente no servidor vai embora da fila
void termino_de_servico(Fila_m_m_1 *fila)
{
    fila->servidor = NULL;
}
//!-----------------------------------------------------------Funções de Cliente-------------------------------------------------------------------------------------------------//

// cria instancia de cliente
void criar_cliente(Cliente **cliente, double tempo_chegada)
{
    (*cliente) = (Cliente *)malloc(sizeof(Cliente *));
    (*cliente)->rodada_origem = numero_rodada_atual;
    (*cliente)->tempo_chegada = tempo_chegada;
}
//!-----------------------------------------------------------Funções de Simulação-----------------------------------------------------------------------------------------------//

// inicializa uma estrutura para armazenar informações sobre a rodada atual;
void criar_rodada(Rodada **nova_rodada)
{
    (*nova_rodada) = (Rodada *)malloc(sizeof(Rodada *));
    (*nova_rodada)->tempo_inicio = tempo_atual;
}

void alocar_mais_espaco_vetor_auxiliar_pmf(Aux_pmf *auxiliar)
{
    double *temp;
    auxiliar->tam_vetor_aux_pmf = 2 * auxiliar->tam_vetor_aux_pmf;
    temp = (double *)malloc(auxiliar->tam_vetor_aux_pmf * sizeof(double));
    for (size_t i = 0; i < auxiliar->tam_vetor_aux_pmf; i++)
    {
        if (i < auxiliar->tam_vetor_aux_pmf / 2)
        {
            temp[i] = auxiliar->vetor_aux_pmf[i];
        }
        else
        {
            temp[i] = 0.0;
        }
    }
    free(auxiliar->vetor_aux_pmf);
    auxiliar->vetor_aux_pmf = temp;
}

void calculo_pmf()
{
    double intervalo_tempo = tempo_atual - auxiliar_calculo_pmf->tempo_ultima_alteracao_na_fila_espera;
    if (fila->numero_elementos_na_fila_de_espera > auxiliar_calculo_pmf->tam_vetor_aux_pmf)
    {
        alocar_mais_espaco_vetor_auxiliar_pmf(auxiliar_calculo_pmf);
    }
    auxiliar_calculo_pmf->vetor_aux_pmf[fila->numero_elementos_na_fila_de_espera] += intervalo_tempo;
    auxiliar_calculo_pmf->tempo_ultima_alteracao_na_fila_espera = tempo_atual;
}

// cria um evento de chegada e o adiciona na heap de eventos
void nova_chegada()
{
    Evento *chegada;
    double instante_chegada = tempo_atual + gerar_amostra_exponencial(); // o tempo de chegadas é uma exponencial
    criar_evento(instante_chegada, CHEGADA, &chegada);                   // cria  evento
    inserir_evento(chegada, lista_eventos);                              // insere o evento na heap de eventos
}

// cria um evento de termino de serviço e o adiciona na heap de eventos
void novo_servico()
{
    Evento *novo_termino_servico;

    double instante_termino = tempo_atual + gerar_amostra_exponencial();    // serviço com duração exponencial
    criar_evento(instante_termino, TERMINO_SERVICO, &novo_termino_servico); // cria o evento de termino do serviço
    inserir_evento(novo_termino_servico, lista_eventos);                    // insere o evento na heap de eventos
}

// resolve um evento de chegada na fila
void tratar_evento_chegada()
{
    Cliente *novo_cliente;
    calculo_pmf();
    criar_cliente(&novo_cliente, tempo_atual); // instancia o novo cliente da fila
    if (!is_fila_ocupada(fila))                // se a fila estiver vazia
    {
        insercao_direta_no_servidor(novo_cliente, fila);
    }
    else
    {
        insere_na_fila(novo_cliente, fila); // cliente vai para fila de espera
    }
    // TODO calculo da pmf
    nova_chegada(); // a próxima chegada é gerada enquanto tratamos a anterior
}

void tratar_evento_termino_servico()
{
    termino_de_servico(fila); // cliente no servidor termina seu
    calculo_pmf();
    if (alguem_na_espera(fila))
    {
        Cliente *proximo_servido;                                  // proximo cliente a ser servido
        proximo_servido = buscar_na_fila_de_espera(fila);          // pega o primeiro da fila de espera
        if (proximo_servido->rodada_origem == numero_rodada_atual) // calcula as métricas de tempo de espera apenas se o cliente chegou nessa rodada
        {
            proximo_servido->tempo_espera = tempo_atual - proximo_servido->tempo_chegada;                          // calcula o tempo de espera
            somatorio_tempo_espera += proximo_servido->tempo_espera;                                               // calculo incremental do tempo de espera médio
            somatorio_tempo_espera_ao_quadrado += (proximo_servido->tempo_espera * proximo_servido->tempo_espera); // calculo incremental da variancia do tempo de espera
        }
        novo_servico();
    }
}

// ao fim de uma rodada calcula as metricas de tempo médio de espera e numero médio de pessoas na fila de espera
void calculo_das_metricas()
{
    double duracao_rodada, *temp;
    rodada_atual->media_estimada_tempo_espera = somatorio_tempo_espera / TAM_RODADA; // calculo do tempo de espera médio
    rodada_atual->variancia_estimada_tempo_espera = (somatorio_tempo_espera_ao_quadrado / (TAM_RODADA - 1)) + (somatorio_tempo_espera * somatorio_tempo_espera) / (TAM_RODADA * (TAM_RODADA - 1));
    duracao_rodada = tempo_atual - rodada_atual->tempo_inicio;

    // TODO pmf
}

void criar_estrutura_auxiliar_pmf(Aux_pmf **auxiliar)
{
    (*auxiliar) = (Aux_pmf *)malloc(sizeof(Aux_pmf *));
    (*auxiliar)->tam_vetor_aux_pmf = TAM_RODADA;
    (*auxiliar)->tempo_ultima_alteracao_na_fila_espera = tempo_atual;
    (*auxiliar)->vetor_aux_pmf = (double *)malloc((*auxiliar)->tam_vetor_aux_pmf * sizeof(double));
    for (size_t i = 0; i < (*auxiliar)->tam_vetor_aux_pmf; i++)
    {
        (*auxiliar)->vetor_aux_pmf[i] = 0.0;
    }
}
void simulacao()
{
    Evento *proximo_evento; // proximo evento a ser tratado
    criar_estrutura_auxiliar_pmf(&auxiliar_calculo_pmf);
    rodadas = (Rodada **)malloc(NUMERO_RODADAS * sizeof(Rodada *)); // inicializa vetor que guarda informação sobre todas as rodadas
    nova_chegada();                                                 // primeira chegada da simulação
    while (numero_rodada_atual < NUMERO_RODADAS)                    // simulação roda por NUMERO_RODADAS rodadas
    {
        criar_rodada(&rodada_atual);                       // inicializa a estrutura  que armazena as informações da rodada atual
        rodadas[numero_rodada_atual] = rodada_atual;       // guarda  a estrutura no vetor de rodadas
        proximo_evento = remover_evento(lista_eventos);    // pega o próximo evento da heap de eventos
        tempo_atual = proximo_evento->tempo_de_ocorrencia; // o tempo da simulaçao é avanaçado até o tempo do próximo evento
        switch (proximo_evento->tipo)                      // trata o evento de acordo com seu tipo
        {
        case CHEGADA:
            tratar_evento_chegada();
            break;

        case TERMINO_SERVICO:
            tratar_evento_termino_servico();
            break;
        default:
            puts("ERRO");
        }
    }
}
int main(int argc, char const *argv[])
{
}
