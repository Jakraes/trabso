/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos
 **
 ** Aluno: Nº:       Nome:
 ** Nome do Módulo: cliente.c v1
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"
#define DEBUG_MODE FALSE // To disable debug messages, uncomment this line

/* Variáveis globais */
Passagem pedido;               // Variável que tem o pedido enviado do Cliente para o Servidor
int pidServidor;               // Variável que tem o PID do processo Servidor
char passagemIniciada = FALSE; // Variável que indica que o Servidor já enviou sinal de início da passagem

/* Protótipos de funções */
int getPidServidor();                // C1: Função a ser implementada pelos alunos
int armaSinais();                    // C2: Função a ser implementada pelos alunos
Passagem getDadosPedidoUtilizador(); // C3: Função a ser implementada pelos alunos
int escrevePedido(Passagem);         // C4: Função a ser implementada pelos alunos
int configuraTemporizador();         // C5: Função a ser implementada pelos alunos
void trataSinalSIGUSR1(int);         // C6: Função a ser implementada pelos alunos
void trataSinalSIGTERM(int);         // C7: Função a ser implementada pelos alunos
void trataSinalSIGHUP(int);          // C8: Função a ser implementada pelos alunos
void trataSinalSIGINT(int);          // C9: Função a ser implementada pelos alunos
void trataSinalSIGALRM(int);         // C10: Função a ser implementada pelos alunos

/**
 * Main: Processamento do processo Cliente
 *
 * @return int Exit value
 */
int main()
{ // Os alunos em princípio não deverão alterar esta função
    // C1
    pidServidor = getPidServidor();
    exit_on_error(pidServidor, FILE_SERVIDOR);
    // C2
    pedido = getDadosPedidoUtilizador();
    exit_on_error(pedido.tipo_passagem, "getDadosPedidoUtilizador");
    // C3
    exit_on_error(armaSinais(), "armaSinais");
    // C4
    exit_on_error(escrevePedido(pedido), "escrevePedido");
    // C5
    exit_on_error(configuraTemporizador(), "configuraTemporizador");
    // Aguarda processamento por parte do Servidor
    while (TRUE)
    {
        debug("", "Aguarda processamento por parte do Servidor");
        pause();
    }
}

/**
 *  O módulo Cliente é responsável pelo pedido das passagens. Este módulo será utilizado para solicitar a passagem das viaturas pelas portagens.
 *  Após identificação da viatura, é realizado o pedido da respetiva passagem, ficando este módulo a aguardar até que o processamento esteja concluído.
 *  Assim, definem-se as seguintes tarefas a desenvolver:
 */

/**
 * C1   Lê a informação acerca do PID do Servidor de Passagens que deve estar registado no ficheiro FILE_SERVIDOR.
 *      No caso de o ficheiro não existir ou de não ter um PID registado no ficheiro, dá error C1 "<Problema>" e termina o processo Cliente.
 *      Caso contrário, assume que a única informação no ficheiro (em formato de texto) é o PID do Servidor
 *      (pai, único PID do lado do Servidor que este Cliente conhece), e dá success C1 "<PID Servidor>";
 *
 * @return int Sucesso
 */
int getPidServidor()
{
    int pidServidor = -1; // Por omissão, retorna valor inválido
    FILE *ficheiro;
    if ((ficheiro = fopen(FILE_SERVIDOR, "r")) == NULL)
    {
        error("C1", "ficheiro nao existe");
        exit(0);
    }
    else
    {
        fscanf(ficheiro, "%d", &pidServidor);
        if (pidServidor == -1)
        {
            error("C1", "ficheiro vazio");
            exit(0);
        }
        else
        {
            success("C1", "%d", pidServidor);
        }
    }
    return pidServidor;
}

/**
 * C2   Pede ao Condutor (utilizador) que preencha os dados referentes à passagem da viatura (Matrícula e Lanço),
 *      criando um elemento do tipo Passagem com essas informações, e preenchendo o valor pid_cliente com o PID do seu próprio processo Cliente.
 *      Em caso de ocorrer qualquer erro, dá error C2 "<Problema>", e termina o processo Cliente;
 *      caso contrário dá success C2 "Passagem do tipo <Normal | Via Verde> solicitado pela viatura com matrícula <matricula> para o Lanço <lanco> e com PID <pid_cliente>";
 *
 * @return Passagem Elemento com os dados preenchidos. Se tipo_passagem = -1, significa que o elemento é imválido
 */
Passagem getDadosPedidoUtilizador()
{
    Passagem p;
    p.tipo_passagem = -1; // Por omissão, retorna valor inválido
    char buffer[2];
    printf("insira passagem: ");
    my_gets(buffer, 2);
    p.tipo_passagem = atoi(buffer);
    printf("insira matricula: ");
    my_gets(p.matricula, 9);
    printf("lanço: ");
    my_gets(p.lanco, 50);
    p.pid_cliente = getpid();
    switch (p.tipo_passagem)
    {
    case 1:
        success("C2", "passagem do tipo normal solicitado pela viatura com matricula %s para o lanço %s e com pid %d", p.matricula, p.lanco, getpid());
    case 2:
        success("C2", "passagem do tipo via verde solicitado pela viatura com matricula %s para o lanço %s e com pid %d", p.matricula, p.lanco, getpid());
    }
    return p;
}

/**
 * C3   Arma os sinais SIGUSR1 (ver C6), SIGTERM (ver C7), SIGHUP (ver C8), SIGINT (ver C9), e SIGALRM (ver C10),
 *      dando, no fim de os armar, a mensagem success C3 "Armei sinais";
 *
 * @return int Sucesso
 */
int armaSinais()
{
    signal(SIGUSR1, trataSinalSIGUSR1);
    signal(SIGTERM, trataSinalSIGTERM);
    signal(SIGHUP, trataSinalSIGHUP);
    signal(SIGINT, trataSinalSIGINT);
    signal(SIGALRM, trataSinalSIGALRM);
    success("C3", "armei sinais");
    return 0;
}

/**
 * C4   Valida se o ficheiro com organização FIFO (named pipe) FILE_PEDIDOS existe.
 *      Se esse ficheiro FIFO não existir, dá error C4 e termina o processo Cliente.
 *      Caso contrário, escreve as informações do elemento Passagem (em formato binário) nesse FIFO FILE_PEDIDOS.
 *      Em caso de erro na escrita, dá error C4 e termina o processo Cliente, caso contrário, dá success C4 "Escrevi FIFO";
 *
 * @return int Sucesso
 */
int escrevePedido(Passagem dados)
{
    FILE *ficheiro;
    if ((ficheiro = fopen(FILE_PEDIDOS, "r")) == NULL)
    {
        error("C4", "ficheiro nao existe");
    }
    else
    {
        fclose(ficheiro);
        ficheiro = fopen(FILE_PEDIDOS, "wb");
        fprintf(ficheiro, "%d %s %s %d %d", dados.tipo_passagem, dados.matricula, dados.lanco, dados.pid_cliente, dados.pid_servidor_dedicado);
        fclose(ficheiro);
    }
    return 0;
}

/**
 * C5   Configura um alarme com o valor de MAX_ESPERA segundos (ver C10),
 *      dá success C5 "Inicia Espera de <MAX_ESPERA> segundos",
 *      e fica a aguardar o resultado do processamento do pedido por parte do Servidor.
 *
 * @return int Sucesso
 */
int configuraTemporizador()
{
    alarm(MAX_ESPERA);
    success("C5", "inicia espera de %d segundos", MAX_ESPERA);
    return 0;
}

/**
 * C6   O sinal armado SIGUSR1 serve para o Servidor Dedicado indicar que o processamento da passagem foi iniciado.
 *      Se o Cliente receber esse sinal, dá success C6 "Passagem Iniciada", assinala que o processamento iniciou,
 *      e retorna para continuar a aguardar a conclusão do processamento do lado do Servidor Dedicado;
 */
void trataSinalSIGUSR1(int sinalRecebido)
{
    passagemIniciada = TRUE;
    success("C6", "passagem iniciada");
}

/**
 * C7   O sinal armado SIGTERM serve para o Servidor Dedicado indicar que o processamento da passagem foi concluído.
 *      Se o Cliente receber esse sinal, dá success C7 "Passagem Concluída", e termina o processo Cliente.
 *      ATENÇÂO: Deverá previamente validar que anteriormente este Cliente já tinha recebido o sinal SIGUSR1 (ver C6),
 *               indicando que o processamento do lado do Servidor Dedicado teve início,
 *               caso contrário, em vez de sucesso, dá error C7 e termina o processo Cliente;
 */
void trataSinalSIGTERM(int sinalRecebido)
{
    success("C7", "passagem concluida");
    exit(0);
}

/**
 * C8   O sinal armado SIGHUP serve para o Servidor Dedicado indicar que o processamento a passagem não foi concluído.
 *      Se o Cliente receber esse sinal, dá success C8 "Processo Não Concluído e Incompleto", e termina o processo Cliente;
 */
void trataSinalSIGHUP(int sinalRecebido)
{
    success("C8", "processo nao incluido e nao completo");
    exit(0);
}

/**
 * C9   O sinal armado SIGINT serve para que, no caso de o veículo ter uma avaria, ou por outro motivo qualquer,
 *      o condutor (utilizador) possa cancelar o pedido do lado do Cliente, usando o atalho <CTRL+C>.
 *      Se receber esse sinal (do utilizador via Shell), o Cliente envia o sinal SIGHUP ao Servidor,
 *      para que esta passagem seja sinalizada como anomalia, dá success C9 "Processo Cancelado pelo Cliente",
 *      e retorna para aguardar que o Servidor Dedicado conclua o processo
 *      (o Servidor Dedicado deverá mais tarde enviar o sinal SIGHUP a este Cliente, ver C8);
 */
void trataSinalSIGINT(int sinalRecebido)
{
    kill(pidServidor, SIGINT);
    success("C9", "processo cancelado pelo cliente");
}

/**
 * C10  O sinal armado SIGALRM serve para que, se o Cliente em C5 esperou mais do que MAX_ESPERA segundos sem resposta,
 *      o Cliente envia o sinal SIGHUP ao Servidor, para que esta passagem seja sinalizada como anomalia,
 *      dá success C10 "Timeout Cliente", e retorna para aguardar que o Servidor Dedicado conclua o processo
 *      (o Servidor Dedicado deverá mais tarde enviar o sinal SIGHUP a este Cliente, ver C8).
 */
void trataSinalSIGALRM(int sinalRecebido)
{
    kill(pidServidor, SIGHUP);
    success("C10", "timeout cliente");
}