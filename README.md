# GBServidor

Servidor web HTTP/1.1 simples escrito em **C** com a API **Winsock2** do Windows. Atende requisições `GET`, serve arquivos estáticos (texto e binários) a partir da pasta `arquivos/` e trata múltiplos clientes em paralelo usando uma thread por conexão.

## Funcionalidades

- **Requisições GET** — outros métodos recebem `405 Method Not Allowed`.
- **Arquivos estáticos** — HTML, CSS, JS, JSON, XML, TXT, imagens (JPG, PNG, GIF, WebP, ICO), PDF, ZIP, MP3, MP4, WebM e fontes WOFF, com o `Content-Type` correto.
- **Texto e binário** — arquivos de texto são lidos em modo texto e os demais em modo binário, sem corromper o conteúdo.
- **Página padrão** — `/` e caminhos de pasta (ex.: `/blog`) servem o `index.html` correspondente.
- **Subpastas via `Referer`** — recursos relativos (CSS, imagens etc.) pedidos por uma página dentro de uma subpasta são buscados na mesma subpasta.
- **Erro 404 personalizado** — arquivos inexistentes retornam `arquivos/error404.html`.
- **Conexões persistentes (keep-alive)** — a conexão permanece aberta para várias requisições, sendo encerrada após 2 s de inatividade ou quando o cliente envia `Connection: close`.
- **Múltiplos clientes** — cada conexão roda em sua própria thread (`_beginthread`), e os clientes ativos são mantidos em uma lista encadeada.

## Requisitos

- Windows
- Compilador C com suporte a Winsock2 (MinGW/GCC, ex.: Code::Blocks, ou MSVC)

## Compilação

**GCC / MinGW (linha de comando):**

```bash
gcc GBServidor.c -o GBServidor.exe -lws2_32
```

**Code::Blocks:** em *Settings → Compiler → Linker Settings*, adicione a biblioteca `ws2_32` (ou `wsock32`).

**MSVC:** a diretiva `#pragma comment(lib, "ws2_32.lib")` já faz a ligação automaticamente.

## Estrutura do projeto

O executável procura os arquivos na pasta `arquivos/`, relativa ao diretório onde é executado:

```
GBServidor/
├── GBServidor.c            # código-fonte do servidor
├── README.md
└── arquivos/               # raiz do site
    ├── index.html          # página inicial (/)
    ├── style_index.css     # estilo da página inicial
    ├── error404.html       # página de erro 404
    └── www/                # site de exemplo, servido em /www
        ├── index.html
        ├── estilo.css
        ├── bolo.jpg
        └── favicon.ico
```

Para testar: <http://localhost/> (página inicial), <http://localhost/www> (exemplo com CSS e imagem) e <http://localhost/naoexiste.html> (erro 404).

## Uso

1. Crie a pasta `arquivos/` com pelo menos `index.html` e `error404.html`.
2. Execute o servidor (a porta 80 pode exigir execução como administrador e liberação no firewall):

   ```bash
   GBServidor.exe
   ```

3. Acesse no navegador: <http://localhost/>

O console exibe cada conexão aceita (IP e porta do cliente) e o conteúdo das requisições recebidas.

## Configuração

As constantes no início do código podem ser ajustadas:

| Constante     | Padrão | Descrição                                   |
|---------------|--------|---------------------------------------------|
| `PORT`        | `80`   | Porta em que o servidor escuta              |
| `MESSAGESIZE` | `1024` | Tamanho máximo da requisição lida (bytes)   |

## Como funciona

1. `main` inicializa o Winsock, faz `bind` na porta e fica em `accept` aguardando conexões.
2. Cada cliente aceito é inserido na lista encadeada e tratado em uma nova thread (`fazer_conexao`).
3. A thread usa `select` com timeout de 2 s; ao receber dados, `formatar_resposta` interpreta a requisição, resolve o caminho físico, identifica o `Content-Type` e monta a resposta com cabeçalhos e corpo.
4. Ao fim (timeout, erro ou `Connection: close`), o socket é fechado e o cliente removido da lista.

## Limitações conhecidas

Pontos ainda não tratados pelo servidor:

- Somente Windows (Winsock).
- Buffer de resposta fixo de ~1 MB: arquivos maiores não são suportados.
- Requisições acima de 1023 bytes são truncadas.
- Não há proteção contra *path traversal* (`../`).
- A lista de clientes é acessada por várias threads sem sincronização.
- A linha de status sempre usa a frase `OK` (inclusive em respostas 404).
