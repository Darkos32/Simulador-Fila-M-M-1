
from math import log
from random import Random, randint, random


TAM_FASE_TRANSIENTE = None
TAM_RODADA = None
#tipos de operacao numa heap
INSERCAO = 1
REMOCAO = 2
#tipos de disciplinas de uma fila
LCFS = 1
FCFS =2
#Tipos de Evento
CHEGADA = 1
TERMINO_SERVICO = 2
#parametros da fila
RHO =0.2#utilizacao
MI = 1#taxa de serviço
LAMBDA = RHO#taxa de chegadas
#Estados do simulador
FASE_TRANSIENTE = 1
RODADAS_VALIDAS = 2
FIM_SIMULACAO = False

class VariaveisAleatorias:
    def __init__(self):
        pass
    def distribuicaoExponecial():
        uniforme = random()
        return log(1-uniforme)/(-LAMBDA)


class Cliente:
    def __init__(self):
        self.rodadaOrigem = numeroRodadaAtual
        self.tempoChegada = tempoAtual

class Rodada:
    def __init__(self) :
         
        self.tempoInicio = tempoAtual
        self.tempoTotal = None
        self.numeroAmostras = 0
        self.somatorioTempoEspera = 0.0
        self.somatorioTempoEsperaQuadrado =0.0
        self.mediaTempoEspera = 0.0
        self.varianciaTempoEspera = 0.0
        self.mediaNumeroPessoasEspera =0.0
        self.segundoMomentoNumeroPessoasEspera= 0.0
        self.varianciaNumeroPessoasEspera = 0.0
        self.chegadasNaRodada = 0
    def calculoMediaNumeroPessoasEspera(self):
        global pmfHub
        for i in range(0,len(pmfHub.pmf)):
            self.mediaNumeroPessoasEspera+= i* pmfHub.pmf[i]
    def calculoVarianciaNumeroPessoasEspera(self):
        global pmfHub
        #primeiro calculamos o segundo momento
        for i in range(0,len(pmfHub.pmf)):
            self.segundoMomentoNumeroPessoasEspera+= i*i* pmfHub.pmf[i]
        self.varianciaNumeroPessoasEspera = self.segundoMomentoNumeroPessoasEspera - (self.mediaNumeroPessoasEspera**2)
    def calculoDasMetricas(self):
        global pmfHub
        self.mediaTempoEspera = self.somatorioTempoEspera/self.numeroAmostras
        self.varianciaTempoEspera = self.somatorioTempoEsperaQuadrado/self.numeroAmostras
        pmfHub.calculoPmf()
        self.calculoMediaNumeroPessoasEspera()
        self.calculoVarianciaNumeroPessoasEspera()
    def fimRodada(self):
         
        global pmfHub
        numeroRodadaAtual+=1
        pmfHub.fimRodada()

        

class Evento:
    def __init__(self,tipo,tempoDeOcorrencia):
        self.tipo = tipo
        self.tempoDeOcorrencia =  tempoDeOcorrencia
    def antecedeEsseEvento(self,outroEvento):
        return self.tempoDeOcorrencia<outroEvento.tempoDeOcorrencia
class Heap:
    def __init__(self):
        self.vetor = []
        self.ultimoIndexPreenchido = -1
    def getParentIndex(self,indexChild): # retorna a posição nó parent
        if(not indexChild):#nó é raiz
            return False
        else:
            indexParent = (indexChild-1)/2
            return int( indexParent)
    def getLeftChildIndex(self,indexParent):#retorna a posição do ramo a esquerda na arvore
        leftChildIndex = (2 *indexParent) + 1
        return int( leftChildIndex)
    def getRightChildIndex(self,indexParent):#retorna a posição do ramo a direita na arvore
        rightChildIndex = 2*(indexParent + 1)
        return int(rightChildIndex)
    def trocar_2_elementos(self,elemento1,elemento2):#inverte 2 elementos
        return elemento2,  elemento1

    def acharIndexMenorCrianca(self,indexParent):# acha quais dos filhos de um nó é o menor e retorna sua posição
        indexLeftChild = self.getLeftChildIndex(indexParent)
        indexRightChild= self.getRightChildIndex(indexParent)
        existeLeftChild = indexLeftChild<=self.ultimoIndexPreenchido
        existeRightChild = indexRightChild<=self.ultimoIndexPreenchido
        leftChild = self.vetor[indexLeftChild] if existeLeftChild else None
        rightChild = self.vetor[indexRightChild] if existeRightChild else None
        if existeLeftChild and existeRightChild:#ambos filhos existem
            return indexLeftChild if leftChild.antecedeEsseEvento(rightChild) else indexRightChild
        elif existeLeftChild or existeRightChild:#apenas um existe
            return indexLeftChild if existeLeftChild else indexRightChild
        else:#nó sem filhos
            return False


    def ordenaAposInsercao(self):#mantém a heap ordenada após uma inserção
        indexNovoElemento = self.ultimoIndexPreenchido
        novoElemento = self.vetor[indexNovoElemento]
        while True:
            if indexNovoElemento ==0: #nó é raiz
                return
            indexParent = self.getParentIndex(indexNovoElemento)#olha o pai desse elemento
            parent = self.vetor[indexParent]
            if novoElemento.antecedeEsseEvento(parent):#se a criança ocorre antes do parent troca os dois
                self.vetor[indexParent] , self.vetor[indexNovoElemento] = self.trocar_2_elementos(parent,novoElemento)
                indexNovoElemento = indexParent #atualiza a posição do novoElemento
                continue
            break
    def ordenaAposRemocao(self):#mantém a heap ordenada após uma remoção
        if len(self.vetor)== 0:
            return
        indexAtual = 0
        raiz = self.vetor[indexAtual]
        while indexAtual != self.ultimoIndexPreenchido:#enquanto não chegou ao final da heap
            indexMenorCrianca = self.acharIndexMenorCrianca(indexAtual)
            if not indexMenorCrianca:#nó sem filhos
                return
            menorCrianca =  self.vetor[indexMenorCrianca]
            if menorCrianca.antecedeEsseEvento(raiz): # é menor que parente troca os dois
                self.vetor[indexMenorCrianca],self.vetor[indexAtual] = self.trocar_2_elementos(menorCrianca,raiz)
                indexAtual = indexMenorCrianca
                continue
            return
    def ordenaHeap(self,operacao):#ordena a heap dependendo do tipo de alteração
        if operacao == INSERCAO:
            self.ordenaAposInsercao()
        else:
            self.ordenaAposRemocao()
    def inserirEvento(self,evento):#insere um novo evento
        self.vetor.append(evento)#insere evento na ultima folha
        self.ultimoIndexPreenchido +=1
        self.ordenaHeap(INSERCAO)
    def removerEvento(self):#remove o proximo evento
        self.vetor[0],self.vetor[self.ultimoIndexPreenchido] = self.trocar_2_elementos(self.vetor[0],self.vetor[self.ultimoIndexPreenchido])# troca a raiz e a ultima folha de lugar
        self.ultimoIndexPreenchido -=1
        eventoRemovido = self.vetor.pop()# pega o que antes foi raiz
        self.ordenaHeap(REMOCAO)
        return eventoRemovido

class FilaMM1:
    def __init__(self,disciplina):
        self.disciplina = disciplina # se a fila é LCFS ou FCFS
        self.filaDeEspera = [] #fila de espera
        self.numeroDeClientesEmEspera = 0
        self.servidor = None #servidor
        self.ultilizacao = RHO
        self.taxaServico = MI
    def alguemNaEspera(self):
        return  len( self.filaDeEspera) > 0
    def isServidorOcupado(self):
        return self.servidor!=None
    def insereNoServidor(self,cliente):
        self.servidor = cliente
    def insereNaFila(self,cliente):
        if self.disciplina == FCFS:#caso FCFS
            self.filaDeEspera.append(cliente)
        else:# caso LCFS
            self.filaDeEspera = [cliente] + self.filaDeEspera
        self.numeroDeClientesEmEspera +=1
        
            
    def terminoDeServico(self):
        clienteAcabado = self.servidor
        self.servidor = None
        return clienteAcabado
    def buscarProximoCliente(self):
        self.numeroDeClientesEmEspera-=1
        return self.filaDeEspera.pop(0)# retorna  o proximo a ser servido
        

class Pmf:
    def __init__(self):
         
        self.mapaIntermediarioPmf = {}
        self.pmf = []
        self.ultimaAlteracao = tempoAtual
    def calculoIntermediarioDaPmf(self):
         
        intervaloTempo = tempoAtual - self.ultimaAlteracao#intervalo de tempo que a fila ficou com esse tamanho
        numeroAtual = fila.numeroDeClientesEmEspera
        if numeroAtual not in self.mapaIntermediarioPmf: #se ainda não foi gravado nenhum intervalo com esse numero de pessoas
            self.mapaIntermediarioPmf[numeroAtual] = intervaloTempo
        else:
            self.mapaIntermediarioPmf[numeroAtual]+=intervaloTempo
        self.ultimaAlteracao = tempoAtual
    def calculoPmf(self):
        intervaloTempo = rodadaAtual.tempoTotal
        for somaIntervalo in self.mapaIntermediarioPmf.values():
            self.pmf.append(somaIntervalo/intervaloTempo) if intervaloTempo!=0 else self.pmf.append(0.0)
    def fimRodada(self):
        self.mapaIntermediarioPmf = {}
        self.pmf = []



   
        
    
def novaChegada():
    global tempoAtual
    global listaEventos
    instanteChegada  = tempoAtual + VariaveisAleatorias.distribuicaoExponecial()
    novoEvento = Evento(CHEGADA,instanteChegada)
    listaEventos.inserirEvento(novoEvento)


def novoServico():
    global tempoAtual
    global listaEventos
    instanteTermino = tempoAtual + VariaveisAleatorias.distribuicaoExponecial()
    terminoServico = Evento(TERMINO_SERVICO,instanteTermino)
    listaEventos.inserirEvento(terminoServico)

def saidaFaseTransiente():
    global metricasFaseTransiente
    global mediasTempoEspera 
    global varianciasTempoEspera
    global mediasNumeroEspera
    global varianciaNumeroEspera
    global clientesParaDescartar
    global rodadaAtual
    
    if clientesParaDescartar!=0:
        mediasTempoEspera.append(rodadaAtual.mediaTempoEspera)
        mediasTempoEspera.append(" ")
        varianciasTempoEspera.append(rodadaAtual.varianciaTempoEspera)
        varianciasTempoEspera.append(" ")
        mediasNumeroEspera.append(rodadaAtual.mediaNumeroPessoasEspera)
        mediasNumeroEspera.append(" ")
        varianciaNumeroEspera.append(rodadaAtual.varianciaNumeroPessoasEspera)
        varianciaNumeroEspera.append(" ")
    else:
        mediasTempoEspera = map(str, mediasTempoEspera)
        mediasNumeroEspera.append("\n")
        mediasNumeroEspera = map(str,mediasTempoEspera)
        varianciaNumeroEspera.append("\n")
        varianciaNumeroEspera =map(str,varianciaNumeroEspera)
        varianciasTempoEspera.append("\n")
        varianciasTempoEspera = map(str,varianciasTempoEspera)
        
        metricasFaseTransiente.writelines(mediasTempoEspera)
        metricasFaseTransiente.writelines(mediasNumeroEspera)
        metricasFaseTransiente.writelines(varianciaNumeroEspera)
        metricasFaseTransiente.writelines(varianciasTempoEspera)
        
    
    
def handleEventoChegada():
    global fila
    global pmfHub
    global clientesJaDescartados
    global clientesParaDescartar
    global rodadaAtual
    global faseAtual
    pmfHub.calculoIntermediarioDaPmf()
    novoCliente =Cliente()
    if(not fila.isServidorOcupado()):#caso não haja ninguem no servidor  inser-se diretamente nele
        fila.insereNoServidor(novoCliente)
        novoServico()#cria um novo evento de termino de servico
    else:
        fila.insereNaFila(novoCliente)
    if(faseAtual == FASE_TRANSIENTE):
        clientesParaDescartar-=1
        clientesJaDescartados+=1
        rodadaAtual.tempoTotal = clientesParaDescartar
        rodadaAtual.numeroAmostras+=1
        rodadaAtual.calculoDasMetricas()
        saidaFaseTransiente()

        if clientesParaDescartar ==0: #fase transiente terminou
            faseAtual = FIM_SIMULACAO if flagTesteDeFaseTransiente else RODADAS_VALIDAS
            rodadaAtual.numeroAmostras = TAM_RODADA
    if(faseAtual == RODADAS_VALIDAS):
        rodadaAtual.chegadasNaRodada+=1
        if rodadaAtual.chegadasNaRodada == TAM_RODADA +1:
            rodadaAtual.calculoDasMetricas()
            rodadaAtual.fimRodada()
    novaChegada()#a próxima chegada é gerada enquanto tratamos a anterior

def handleEventoTerminoDeServico():
    global fila
    global pmfHub
    global rodadaAtual
    fila.terminoDeServico()
    pmfHub.calculoIntermediarioDaPmf()
    if fila.alguemNaEspera():
        proximoNoServico = fila.buscarProximoCliente()
        if(proximoNoServico.rodadaOrigem == rodadaAtual):#calcula as métricas de tempo de espera apenas se o cliente chegou nessa rodada
            tempoEspera = tempoAtual - proximoNoServico.tempoChegada 
            rodadaAtual.somatorioTempoEspera+=tempoEspera
            rodadaAtual.somatorioTempoEsperaQuadrado += tempoEspera**2
        novoServico()
            

def simulacao():
    global faseAtual
    global rodadaAtual
    global listaEventos
    global tempoAtual
    rodadaAtual = Rodada()
    novaChegada()#primeira chegada
    while faseAtual!= FIM_SIMULACAO:
        if faseAtual==RODADAS_VALIDAS:
            rodadaAtual = Rodada()
        proximoEvento = listaEventos.removerEvento()
        tempoAtual = proximoEvento.tempoDeOcorrencia
        if proximoEvento.tipo == CHEGADA:
            handleEventoChegada()
        else:
            handleEventoTerminoDeServico()
        


def testesFaseTransiente():
    global flagTesteDeFaseTransiente
    global metricasFaseTransiente
    global clientesParaDescartar
    global mediasTempoEspera
    global varianciasTempoEspera
    global mediasNumeroEspera
    global varianciaNumeroEspera
    mediasTempoEspera = []
    varianciasTempoEspera =[]
    mediasNumeroEspera = []
    varianciaNumeroEspera= []
    clientesParaDescartar = int(input("Tamanho da Fase Transiente: "))
    metricasFaseTransiente = open("faseTransiente" + str(clientesParaDescartar)+".txt","w")
    flagTesteDeFaseTransiente =True
    simulacao()
    metricasFaseTransiente.close()

def main():
    global tempoAtual
    global clientesJaDescartados
    global clientesParaDescartar
    global pmfHub
    global numeroRodadaAtual
    global listaEventos
    global fila
    global todasAsRodadas 
    global faseAtual
    global flagTesteDeFaseTransiente
    global rodadaAtual
    tempoAtual = 0.0    
    clientesParaDescartar = TAM_FASE_TRANSIENTE
    clientesJaDescartados = 0
    flagTesteDeFaseTransiente = False
    pmfHub = Pmf()
    fila = FilaMM1(FCFS)
    todasAsRodadas = []
    faseAtual = FASE_TRANSIENTE
    numeroRodadaAtual = -1
    listaEventos = Heap()#lista de eventos
    rodadaAtual = Rodada()
    testesFaseTransiente()

if __name__ == "__main__":
    main()