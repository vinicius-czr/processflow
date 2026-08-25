# ProcessFlow

Orquestrador de processos em C, desenvolvido para a disciplina de Infraestrutura de Software.
O ProcessFlow cadastra tarefas (programas do sistema) e as executa por meio de processos filhos,
usando `fork()`, `exec()`, `wait()`/`waitpid()`, `dup2()` e `pipe()` — sem depender de `system()`,
`popen()` ou de qualquer shell externo.

## Funcionalidades

- Modo **interativo** (prompt `processflow>`) e modo **workflow** (arquivo `.pf`)
- Cadastro de tarefas e execução única, sequencial, paralela e encadeada via pipe
- Redirecionamento de entrada e saída (sobrescrever ou acrescentar)
- Alteração do diretório de trabalho das tarefas
- Execução em background com controle de jobs (iniciar, listar, aguardar)
- Tratamento de erros fatais, não-fatais e casos especiais conforme especificação da disciplina

## Compilação

Requer `gcc` e um sistema Linux, Unix ou macOS.

```bash
make clean
make
```

Isso gera o executável `processflow` na raiz do projeto.

## Execução

**Modo interativo:**
```bash
./processflow
```

**Modo workflow** (lê e processa um arquivo `.pf` linha a linha, imprimindo cada linha antes de executá-la):
```bash
./processflow arquivo.pf
```

## Comandos suportados

| Comando | Descrição |
|---|---|
| `task <nome> <programa> [args...]` | Cadastra uma tarefa com um nome, programa e argumentos |
| `run <nome>` | Executa uma tarefa cadastrada e aguarda seu término |
| `run sequential <t1> <t2> ...` | Executa tarefas em sequência, uma após a outra |
| `run parallel <t1> <t2> ...` | Inicia todas as tarefas antes de aguardar o grupo |
| `run pipe <t1> <t2> ...` | Encadeia tarefas via pipe (saída de uma vira entrada da próxima) |
| `input <tarefa> <arquivo>` | Define o arquivo de entrada de uma tarefa |
| `output <tarefa> <arquivo>` | Define o arquivo de saída de uma tarefa (sobrescreve) |
| `append <tarefa> <arquivo>` | Define o arquivo de saída de uma tarefa (acrescenta) |
| `workdir <diretório>` | Altera o diretório de trabalho das tarefas executadas a partir de agora |
| `start <tarefa>` | Inicia uma tarefa em background e retorna o prompt imediatamente |
| `jobs` | Lista os jobs em background e seus status |
| `wait <jobId>` | Aguarda o término de um job específico |
| `exit` | Encerra o ProcessFlow |

## Arquivos de teste

O projeto inclui arquivos `.pf` de exemplo, cada um cobrindo uma funcionalidade específica:

| Arquivo | O que testa |
|---|---|
| `teste_task_run.pf` | Cadastro de tarefas e execução única, incluindo tarefa inexistente |
| `teste_seq_parallel.pf` | `run sequential` / `run parallel`, incluindo tarefa inexistente no grupo |
| `teste_pipe.pf` | `run pipe` (encadeamento via pipe/dup2) |
| `teste_redir.pf` | `output` / `append` / `input` |
| `teste_workdir.pf` | `workdir`, incluindo diretório inexistente |
| `teste_erros_io.pf` | Erros de I/O (arquivo de entrada/saída inválido) e workdir inexistente |
| `teste_start.pf` | `start` (execução em background sem bloquear o prompt) |
| `teste_jobs.pf` | `jobs` (listagem com status atualizado) |
| `teste_wait.pf` | `wait <jobId>`, incluindo jobId inexistente |
| `teste_sem_exit.pf` | Encerramento correto ao atingir EOF sem comando `exit` |

Exemplo de execução:
```bash
./processflow teste_pipe.pf
```

## Tratamento de erros

**Erros fatais** (mensagem + encerra o programa):
- Número incorreto de argumentos ao iniciar o ProcessFlow
- Arquivo workflow inexistente ou que não pode ser aberto

**Erros não-fatais** (mensagem + continua processando):
- Tarefa informada não existe
- Programa associado à tarefa não existe ou não pode ser executado
- Arquivo de entrada ou saída não pode ser aberto
- Job informado não existe
- Diretório informado em `workdir` não existe

**Casos especiais tratados de forma coerente:**
- Linha de comando vazia
- Múltiplos espaços em branco entre argumentos
- `workflowFile` sem comando `exit` (encerra sozinho ao atingir EOF)
- `CTRL-D` no modo interativo
- Processos que terminam com código de saída diferente de zero
- Processos em paralelo terminando em ordens diferentes

## Arquitetura

| Arquivo | Responsabilidade |
|---|---|
| `main.c` | Loop principal (interativo/workflow) e dispatch de comandos |
| `parser.c` / `parser.h` | Tokenização da linha de comando |
| `task.c` / `task.h` | Cadastro de tarefas e configuração de input/output/append |
| `executor.c` / `executor.h` | Execução via fork/exec/waitpid: única, sequencial, paralela, pipe, workdir, background |
| `job.c` / `job.h` | Controle de jobs em background (`start`, `jobs`, `wait`) |

## Requisitos técnicos

O projeto usa `_POSIX_C_SOURCE 200809L` para habilitar funções POSIX (`fork`, `exec`, `wait`,
`dup2`, `pipe`, `strdup`, `strtok_r`), que não fazem parte do padrão C11 puro, garantindo
portabilidade entre Linux, Unix e macOS.

## Autor

Vinícius Cezar Rodrigues Carvalho
