/// Define servidor web simples
/// Incluir no Settings -> Compiler -> Linker Settings: -l wsock32

#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <process.h>
#include <stdint.h>
#include <sys/time.h>
#pragma comment(lib,"ws2_32.lib") //Biblioteca Winsock
#define PORT 80
#define MESSAGESIZE 1024
#define MAXSOCKETS 20

// Estrutura para a Lista Encadeada de Conexões
typedef struct cliente
{
    SOCKET socket;
    struct cliente *proximo;
} tp_cliente;

tp_cliente *lista_clientes = NULL;

//Para remover um cliente
void remover_cliente(tp_cliente *alvo)
{
    if (lista_clientes != NULL && alvo != NULL)
    {

        if (lista_clientes == alvo)
        {
            lista_clientes = lista_clientes->proximo;
        }
        else
        {
            tp_cliente *atual = lista_clientes;
            while (atual->proximo != NULL && atual->proximo != alvo)
            {
                atual = atual->proximo;
            }
            if (atual->proximo == alvo)
            {
                atual->proximo = alvo->proximo;
            }
        }
    }
}

//Função para inserir um novo cliente
void inserir_cliente(tp_cliente *novo_no)
{
    if (novo_no != NULL)
    {
        novo_no->proximo = lista_clientes;
        lista_clientes = novo_no;
    }
}

//Função para calcular o tamanho do arquivo
int calcular_tamanho_arquivo(char caminho[])
{

    long int tamanho_bytes;
    FILE *arquivo;
    arquivo = fopen(caminho, "r");
    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo");
        tamanho_bytes = -1;
    }
    else
    {
        fseek(arquivo, 0, SEEK_END);
        tamanho_bytes = ftell(arquivo);
        fclose(arquivo);
    }
    return (int) tamanho_bytes;
}

//Função para ler o conteudo do arquivo texto
char *ler_texto(char caminho[])
{
    int tamanho, lidos;
    FILE *arquivo;
    char *conteudo = NULL;

    arquivo = fopen(caminho, "r");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo");
    }
    else
    {
        tamanho = calcular_tamanho_arquivo(caminho);

        conteudo = malloc(sizeof(char) * (tamanho + 1));

        if (conteudo == NULL)
        {
            printf("Erro ao alocar a string conteudo");
        }
        else
        {
            lidos = fread(conteudo, 1, tamanho, arquivo);
            conteudo[lidos] = '\0';
        }

        fclose(arquivo);
    }
    return conteudo;
}

//Le os dadso dentro do arq binario
char *ler_binario(char caminho[])
{
    int tamanho;
    FILE *arquivo;
    char *conteudo = NULL;

    arquivo = fopen(caminho, "rb");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo");
    }
    else
    {
        tamanho = calcular_tamanho_arquivo(caminho);

        conteudo = malloc(tamanho);

        if (conteudo == NULL)
        {
            printf("Erro ao alocar memoria");
        }
        else
        {
            fread(conteudo, 1, tamanho, arquivo);
        }

        fclose(arquivo);
    }
    return conteudo;
}

//Função que vai retornar o content type adequado do meu arquivo
void reconhecendo_content_type(char caminho[], char conteudo[])
{
    char *extensao;
    conteudo[0]=0;
    // Pega do último ponto para frente da string
    extensao = strrchr(caminho, '.');

    if (extensao == NULL)
    {
        // Tipo desconhecido
        strcpy(conteudo, "application/octet-stream");
    }
    else if (strcmp(extensao, ".html") == 0)
    {
        // HTML
        strcpy(conteudo, "text/html");
    }
    else if (strcmp(extensao, ".css") == 0)
    {
        // CSS
        strcpy(conteudo, "text/css");
    }
    else if (strcmp(extensao, ".js") == 0)
    {
        // JavaScript
        strcpy(conteudo, "text/javascript");
    }
    else if (strcmp(extensao, ".json") == 0)
    {
        // JSON
        strcpy(conteudo, "application/json");
    }
    else if (strcmp(extensao, ".txt") == 0)
    {
        // Arquivo de texto
        strcpy(conteudo, "text/plain");
    }
    else if (strcmp(extensao, ".xml") == 0)
    {
        // XML
        strcpy(conteudo, "application/xml");
    }
    else if (strcmp(extensao, ".jpg") == 0 || strcmp(extensao, ".jpeg") == 0)
    {
        // Imagem JPG / JPEG
        strcpy(conteudo, "image/jpeg");
    }
    else if (strcmp(extensao, ".png") == 0)
    {
        // Imagem PNG
        strcpy(conteudo, "image/png");
    }
    else if (strcmp(extensao, ".gif") == 0)
    {
        // Imagem GIF
        strcpy(conteudo, "image/gif");
    }
    else if (strcmp(extensao, ".ico") == 0)
    {
        // Favicon
        strcpy(conteudo, "image/x-icon");
    }
    else if (strcmp(extensao, ".webp") == 0)
    {
        // Imagem WebP
        strcpy(conteudo, "image/webp");
    }
    else if (strcmp(extensao, ".pdf") == 0)
    {
        // documento PDF
        strcpy(conteudo, "application/pdf");
    }
    else if (strcmp(extensao, ".zip") == 0)
    {
        // Aarquivo ZIP
        strcpy(conteudo, "application/zip");
    }
    else if (strcmp(extensao, ".mp3") == 0)
    {
        // audio MP3
        strcpy(conteudo, "audio/mpeg");
    }
    else if (strcmp(extensao, ".mp4") == 0)
    {
        // Video MP4
        strcpy(conteudo, "video/mp4");
    }
    else if (strcmp(extensao, ".webm") == 0)
    {
        // Vídeo WebM
        strcpy(conteudo, "video/webm");
    }
    else if (strcmp(extensao, ".woff") == 0)
    {
        // Fontes WOFF
        strcpy(conteudo, "font/woff");
    }
    else
    {
        // Tipo desconhecido
        strcpy(conteudo, "application/octet-stream");
    }
}

//Função vai verificar o tipo do content type para ver se o arquivo é do tipo texto ou binario
//0 -> Binarip
//1 -> Texto
int encontrar_tipo_arquivo(char content_type[])
{
    int tipo = -1;
    //Da lista de content type que coloquei somente quem tem esses 3 primeiros nomes ou comecam com isos é texto
    //Sendo assim o else seria os binarios, por isso retorna 0
    if (strncmp(content_type, "text/", 5) == 0 || strcmp(content_type, "application/json") ==0 || strcmp(content_type, "application/xml") == 0)
    {
        tipo =  1;
    }
    else
    {
        tipo = 0;
    }
    return tipo;
}

//Funcao que verifica se é arquivo ou diretorio que esta pedindo
//levando em consideracao que se for arquivo vai ter um .
int encontrar_tipo(char caminho_req[])
{
    char *ponto = strrchr(caminho_req, '.');
    // Se existe um ponto é um arquivo
    if (ponto != NULL)
    {
        return 1; // é um arquivo
    }
    return 0; // é uma pasta
}

//Função para encontrar a subpasta a partir do referer
void encontrar_subpasta(char subpasta[], char requisicao[])
{
    char  referer[100]="",*linha_referer;
    int i, j,cont;
    subpasta[0]='\0';

    linha_referer = strstr(requisicao, "Referer:");
    if (linha_referer != NULL)
    {
        sscanf(linha_referer, "Referer: %100s", referer);
        i=0;
        cont=0;
        while(cont < 3 && referer[i] != '\0')
        {
            if(referer[i] == '/')
            {
                cont++;
            }
            i++;
        }
        if (cont == 3)
        {
            i--;
            j = 0;
            while(referer[i] != '\0')
            {
                subpasta[j] = referer[i];
                j++;
                i++;
            }
        }
        subpasta[j] = '\0';
    }
}

//Verifica se o cliente pediu para fechar a conexao
char *deve_fechar_conexao(char requisicao[])
{
    if (strstr(requisicao, "Connection: close") != NULL)
    {
        return "close"; // fecha a conexao depois desta resposta
    }
    return "keep-alive"; // mantem viva (keep-alive)
}

int formatar_resposta(char requisicao[], char resposta_buffer[], char fechar[])
{
    char metodo[16], caminho_req[256], caminho_fisico[300], content_type[30];
    char subpasta[50] = "";
    int tamanho_resposta = 0;
    int tamanho_arq = 0;
    int tamanho_cabecalho = 0;
    int tipo_arq = 0;
    char *conteudo_arquivo = NULL;
    char status[10];

    strcpy(status, "200");
    sscanf(requisicao, "%15s %255s", metodo, caminho_req);

    encontrar_subpasta(subpasta,requisicao);

    // GET
    if (strcmp(metodo, "GET") != 0)
    {
        sprintf(resposta_buffer, "HTTP/1.1 405 Method Not Allowed\r\nConnection: %s\r\nContent-Type: text/plain\r\n\r\nMetodo nao suportado ou endpoint invalido.", fechar);
        tamanho_resposta = strlen(resposta_buffer);
    }
    else
    {
       if (strcmp(caminho_req, "/") == 0)
        {
            strcpy(caminho_fisico, "arquivos/index.html");
        }
        else if (!encontrar_tipo(caminho_req))
        {
            // Pasta nova
            sprintf(caminho_fisico, "arquivos%s/index.html", caminho_req);
        }
        else
        {
            sprintf(caminho_fisico, "arquivos%s%s", subpasta, caminho_req);
        }

        // Content-Type
        reconhecendo_content_type(caminho_fisico, content_type);

        // Verifica o tamanho e se o arquivo existe
        tamanho_arq = calcular_tamanho_arquivo(caminho_fisico);
        if (tamanho_arq < 0)
        {
            strcpy(caminho_fisico, "arquivos/error404.html");
            strcpy(status, "404");
            reconhecendo_content_type(caminho_fisico, content_type);
            tamanho_arq = calcular_tamanho_arquivo(caminho_fisico);
        }

        // conteudo do arquivo (Texto ou Binário)
        tipo_arq = encontrar_tipo_arquivo(content_type);
        if (tipo_arq == 1)
        {
            conteudo_arquivo = ler_texto(caminho_fisico);
        }
        else
        {
            conteudo_arquivo = ler_binario(caminho_fisico);
        }

        // Monta o cabeçalho HTTP e anexa o conteúdo
        tamanho_cabecalho = sprintf(resposta_buffer,
                                    "HTTP/1.1 %s OK\r\n"
                                    "Server: GBServer\r\n"
                                    "Content-Length: %d\r\n"
                                    "Content-Type: %s\r\n"
                                    "Connection: %s\r\n"
                                    "Keep-Alive: timeout=5, max=100\r\n\r\n",
                                    status, tamanho_arq, content_type, fechar);

        // Copia o conteúdo do arquivo logo após os cabeçalhos (suporta dados binários e texto)
        memcpy(resposta_buffer + tamanho_cabecalho, conteudo_arquivo, tamanho_arq);
        tamanho_resposta = tamanho_cabecalho + tamanho_arq;
        free(conteudo_arquivo);
    }

    return tamanho_resposta;
}

void fazer_conexao(void *p)
{
    tp_cliente *cliente = (tp_cliente *)p;
    int len, valido, tamanho_resposta;
    char message[MESSAGESIZE];
    char *resposta_grande = malloc(1000000);
    char *fechar_conexao;
    fd_set set;
    struct timeval timeout;
    int rv;
    char *fim_cabecalho;
    int tamanho_cab;

    //Se for alocado corretamenti...
    if (resposta_grande != NULL)
    {

        valido = 1;
        while(valido)
        {
            // Reseta e configura o conjunto de sockets e o tempo a cada ciclo
            FD_ZERO(&set);
            FD_SET(cliente->socket, &set);

            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            rv = select(0, &set, NULL, NULL, &timeout);

            if (rv == SOCKET_ERROR)
            {
                valido = 0;
            }
            else if (rv == 0)
            {
                // Timeout atingido
                printf("\nConexao encerrada por inatividade (timeout).\n");
                valido=0;
            }
            else
            {
                message[0] = '\0';
                //Recepção da mensagem
                len = recv(cliente->socket, message, MESSAGESIZE - 1, 0);
                if(len > 0)
                {
                    message[len] = '\0';
                    printf("\n================ REQUISICAO RECEBIDA (%d bytes) ================\n", (int) strlen(message));
                    printf("%s", message);
                    printf("================== FIM DA REQUISICAO ==================\n");
                    fflush(stdout);
                    fechar_conexao = deve_fechar_conexao(message);
                    tamanho_resposta = formatar_resposta(message, resposta_grande, fechar_conexao);

                    // Envia a resposta completa para o cliente
                    send(cliente->socket, resposta_grande, tamanho_resposta, 0);

                    // Mostra no terminal apenas os cabecalhos da resposta (o corpo pode ser binario)
                    fim_cabecalho = strstr(resposta_grande, "\r\n\r\n");
                    tamanho_cab = (fim_cabecalho != NULL) ? (int)(fim_cabecalho - resposta_grande) + 4 : tamanho_resposta;
                    printf("\n---------------- RESPOSTA ENVIADA (%d bytes) ----------------\n", tamanho_resposta);
                    printf("%.*s", tamanho_cab, resposta_grande);
                    printf("[corpo: %d bytes]\n", tamanho_resposta - tamanho_cab);
                    printf("------------------ FIM DA RESPOSTA ------------------\n");
                    fflush(stdout);

                    if (strcmp(fechar_conexao, "close") == 0)
                    {
                        valido = 0;
                    }
                }
                else
                {
                    valido = 0;
                }
            }
        }

        free(resposta_grande);
        closesocket(cliente->socket);
        remover_cliente(cliente);
        free(cliente);
        _endthread();
    }
}

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server, client;
    int c, len, pos;
    char message[MESSAGESIZE];

    //Inicializa a bilioteca
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("\nFalha na inicializacao da biblioteca Winsock: %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //Inicializa o descritor do socket
    if((s = socket(AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET)
    {
        printf("\nNao e possivel inicializar o socket: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //Preparação da estrutura (struct) sockaddr_in
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; ///0.0.0.0
    server.sin_port = htons(PORT);

    //Constrói o socket -> solicita uso da porta ao SO
    if( bind(s,(struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("\nNao e possivel construir o socket: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //Começa a escutar na porta
    listen(s,3);

    c = sizeof(struct sockaddr_in);

    while(1)
    {
        tp_cliente *novo_no = (tp_cliente *)malloc(sizeof(tp_cliente));
        if (novo_no == NULL)
        {
            printf("\nErro ao alocar memoria para o cliente.\n");
        }
        else
        {
            //Caso chegue uma conexão, a função accept irá verificar se irá aceitá-la ou não
            novo_no->socket = accept(s, (struct sockaddr *)&client, &c);
            novo_no->proximo = NULL;

            if (novo_no->socket == INVALID_SOCKET)
            {
                printf("\nConexao nao aceita. Codigo de erro: %d", WSAGetLastError());
                free(novo_no);
            }
            else
            {
                printf("\nConexao aceita\n");
                printf("\nDados do cliente - IP: %s - Porta: %d\n", inet_ntoa(client.sin_addr), htons(client.sin_port));
                inserir_cliente(novo_no);

                _beginthread(fazer_conexao, 0, (void *)novo_no);
            }
        }
    }

    //Finalização da biblioteca
    WSACleanup();

    return 0;
}

