#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define MAX 100

// === ESTRUTURAS ===
typedef struct {
    int id;
    char nome[50];
    float preco_custo;
    float preco_venda;
    int quantidade;
    char validade[11]; // dd/mm/yyyy
} Produto;

typedef struct {
    char nome[50];
    char cpf[15];
} Cliente;

typedef struct {
    char nome[50];
    char login[20];
    char senha[20];
    int nivel; // 1 = ADM, 2 = FUNC
} Usuario;

// === FUNÇÕES AUXILIARES ===
int confirmar(char *mensagem) {
    char op;
    printf("%s (s/n): ", mensagem);
    scanf(" %c", &op);
    while(getchar() != '\n'); // limpar buffer
    return (op == 's' || op == 'S');
}

float sugerirPrecoVenda(float custo) {
    return custo * 1.5; // margem de 50%
}

int diasParaValidade(char *data) {
    struct tm validade = {0};
    time_t t = time(NULL);

    sscanf(data, "%d/%d/%d", &validade.tm_mday, &validade.tm_mon, &validade.tm_year);
    validade.tm_mon -= 1;
    validade.tm_year -= 1900;

    time_t tv = mktime(&validade);
    double diff = difftime(tv, t);
    return (int)(diff / (60 * 60 * 24));
}

void registrarCaixa(float valor, int tipo) {
    FILE *fp = fopen("caixa.txt", "a");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char data[11];
    sprintf(data, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900);

    if (fp) {
        fprintf(fp, "%s - %s: R$ %.2f\n", data, tipo == 1 ? "ENTRADA" : "SAIDA", valor);
        fclose(fp);
    }
}

int validarLogin(char login[], char senha[], int *nivel) {
    Usuario u;
    FILE *fp = fopen("usuarios.txt", "rb");
    if (!fp) return 0;

    while (fread(&u, sizeof(Usuario), 1, fp)) {
        if (strcmp(u.login, login) == 0 && strcmp(u.senha, senha) == 0) {
            *nivel = u.nivel;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int produtoExiste(char nome[]) {
    Produto temp;
    FILE *fp = fopen("produtos.txt", "rb");
    if (!fp) return 0;

    while (fread(&temp, sizeof(Produto), 1, fp)) {
        if (strcmp(temp.nome, nome) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int clienteExiste(char cpf[]) {
    Cliente c;
    FILE *fp = fopen("clientes.txt", "rb");
    if (!fp) return 0;

    while (fread(&c, sizeof(Cliente), 1, fp)) {
        if (strcmp(c.cpf, cpf) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int usuarioExiste(char login[]) {
    Usuario u;
    FILE *fp = fopen("usuarios.txt", "rb");
    if (!fp) return 0;

    while (fread(&u, sizeof(Usuario), 1, fp)) {
        if (strcmp(u.login, login) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// === FUNÇÕES PRINCIPAIS ===

void cadastrarProduto() {
    Produto p;
    FILE *fp = fopen("produtos.txt", "a+b");
    if (!fp) {
        printf("Erro ao abrir arquivo de produtos.\n");
        return;
    }

    printf("Nome do produto: ");
    fgets(p.nome, sizeof(p.nome), stdin);
    p.nome[strcspn(p.nome, "\n")] = 0;

    if (produtoExiste(p.nome)) {
        printf("Produto com esse nome ja existe.\n");
        fclose(fp);
        return;
    }

    char buffer[50];

    while (1) {
        printf("Preco de custo: R$ ");
        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        if (sscanf(buffer, "%f", &p.preco_custo) == 1 && p.preco_custo >= 0) break;
        printf("Entrada invalida. Tente novamente.\n");
    }

    p.preco_venda = sugerirPrecoVenda(p.preco_custo);

    while (1) {
        printf("Estoque inicial: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        int qtd;
        if (sscanf(buffer, "%d", &qtd) == 1 && qtd >= 0) {
            p.quantidade = qtd;
            break;
        }
        printf("Entrada invalida. Tente novamente.\n");
    }

    printf("Validade (dd/mm/aaaa): ");
    fgets(p.validade, sizeof(p.validade), stdin);
    p.validade[strcspn(p.validade, "\n")] = 0;

    p.id = rand() % 100000;

    fwrite(&p, sizeof(Produto), 1, fp);
    fclose(fp);
    printf("Produto cadastrado com sucesso!\n");
}

void atualizarProduto() {
    char nome[50];
    Produto p;
    int encontrado = 0;

    printf("Nome do produto a atualizar: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;

    FILE *fp = fopen("produtos.txt", "r+b");
    if (!fp) {
        printf("Erro ao abrir arquivo de produtos.\n");
        return;
    }

    char buffer[50];

    while (fread(&p, sizeof(Produto), 1, fp)) {
        if (strcmp(p.nome, nome) == 0) {
            encontrado = 1;
            printf("Atualizando produto '%s'\n", p.nome);

            while (1) {
                printf("Novo preco de custo: R$ ");
                if (!fgets(buffer, sizeof(buffer), stdin)) continue;
                if (sscanf(buffer, "%f", &p.preco_custo) == 1 && p.preco_custo >= 0) break;
                printf("Entrada invalida. Tente novamente.\n");
            }

            p.preco_venda = sugerirPrecoVenda(p.preco_custo);

            while (1) {
                printf("Nova quantidade em estoque: ");
                if (!fgets(buffer, sizeof(buffer), stdin)) continue;
                int qtd;
                if (sscanf(buffer, "%d", &qtd) == 1 && qtd >= 0) {
                    p.quantidade = qtd;
                    break;
                }
                printf("Entrada invalida. Tente novamente.\n");
            }

            printf("Nova validade (dd/mm/aaaa): ");
            fgets(p.validade, sizeof(p.validade), stdin);
            p.validade[strcspn(p.validade, "\n")] = 0;

            fseek(fp, -sizeof(Produto), SEEK_CUR);
            fwrite(&p, sizeof(Produto), 1, fp);
            printf("Produto atualizado com sucesso!\n");
            break;
        }
    }
    fclose(fp);

    if (!encontrado) {
        printf("Produto nao encontrado.\n");
    }
}

void excluirProduto() {
    char nome[50];
    Produto p;
    int encontrado = 0;

    printf("Nome do produto a excluir: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;

    FILE *fp = fopen("produtos.txt", "rb");
    if (!fp) {
        printf("Arquivo de produtos nao encontrado.\n");
        return;
    }
    FILE *temp = fopen("temp.txt", "wb");
    if (!temp) {
        fclose(fp);
        printf("Erro ao criar arquivo temporario.\n");
        return;
    }

    while (fread(&p, sizeof(Produto), 1, fp)) {
        if (strcmp(p.nome, nome) == 0) {
            encontrado = 1;
            if (!confirmar("Confirma exclusao do produto?")) {
                fwrite(&p, sizeof(Produto), 1, temp);
            } else {
                printf("Produto excluido.\n");
            }
        } else {
            fwrite(&p, sizeof(Produto), 1, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("produtos.txt");
    rename("temp.txt", "produtos.txt");

    if (!encontrado) printf("Produto nao encontrado.\n");
}

void cadastrarCliente() {
    Cliente c;
    FILE *fp = fopen("clientes.txt", "ab");
    if (!fp) {
        printf("Erro ao abrir arquivo de clientes.\n");
        return;
    }

    printf("Nome do cliente: ");
    fgets(c.nome, sizeof(c.nome), stdin);
    c.nome[strcspn(c.nome, "\n")] = 0;

    printf("CPF do cliente: ");
    fgets(c.cpf, sizeof(c.cpf), stdin);
    c.cpf[strcspn(c.cpf, "\n")] = 0;

    if (clienteExiste(c.cpf)) {
        printf("Cliente com esse CPF ja cadastrado.\n");
        fclose(fp);
        return;
    }

    fwrite(&c, sizeof(Cliente), 1, fp);
    fclose(fp);
    printf("Cliente cadastrado com sucesso!\n");
}

void excluirCliente() {
    char cpf[15];
    Cliente c;
    int encontrado = 0;

    printf("CPF do cliente a excluir: ");
    fgets(cpf, sizeof(cpf), stdin);
    cpf[strcspn(cpf, "\n")] = 0;

    FILE *fp = fopen("clientes.txt", "rb");
    if (!fp) {
        printf("Arquivo de clientes nao encontrado.\n");
        return;
    }
    FILE *temp = fopen("temp.txt", "wb");
    if (!temp) {
        fclose(fp);
        printf("Erro ao criar arquivo temporario.\n");
        return;
    }

    while (fread(&c, sizeof(Cliente), 1, fp)) {
        if (strcmp(c.cpf, cpf) == 0) {
            encontrado = 1;
            if (!confirmar("Confirma exclusao do cliente?")) {
                fwrite(&c, sizeof(Cliente), 1, temp);
            } else {
                printf("Cliente excluido.\n");
            }
        } else {
            fwrite(&c, sizeof(Cliente), 1, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("clientes.txt");
    rename("temp.txt", "clientes.txt");

    if (!encontrado) printf("Cliente nao encontrado.\n");
}

void cadastrarFuncionario() {
    Usuario u;
    FILE *fp = fopen("usuarios.txt", "ab");
    if (!fp) {
        printf("Erro ao abrir arquivo de usuarios.\n");
        return;
    }

    printf("Nome do funcionario: ");
    fgets(u.nome, sizeof(u.nome), stdin);
    u.nome[strcspn(u.nome, "\n")] = 0;

    do {
        printf("Login do funcionario: ");
        fgets(u.login, sizeof(u.login), stdin);
        u.login[strcspn(u.login, "\n")] = 0;
        if (usuarioExiste(u.login)) {
            printf("Login ja existe. Escolha outro.\n");
        } else {
            break;
        }
    } while (1);

    printf("Senha do funcionario: ");
    fgets(u.senha, sizeof(u.senha), stdin);
    u.senha[strcspn(u.senha, "\n")] = 0;

    u.nivel = 2; // funcionario

    fwrite(&u, sizeof(Usuario), 1, fp);
    fclose(fp);
    printf("Funcionario cadastrado com sucesso!\n");
}

void excluirFuncionario() {
    char login[20];
    Usuario u;
    int encontrado = 0;

    printf("Login do funcionario a excluir: ");
    fgets(login, sizeof(login), stdin);
    login[strcspn(login, "\n")] = 0;

    FILE *fp = fopen("usuarios.txt", "rb");
    if (!fp) {
        printf("Arquivo de usuarios nao encontrado.\n");
        return;
    }
    FILE *temp = fopen("temp.txt", "wb");
    if (!temp) {
        fclose(fp);
        printf("Erro ao criar arquivo temporario.\n");
        return;
    }

    while (fread(&u, sizeof(Usuario), 1, fp)) {
        if (strcmp(u.login, login) == 0) {
            encontrado = 1;
            if (!confirmar("Confirma exclusao do funcionario?")) {
                fwrite(&u, sizeof(Usuario), 1, temp);
            } else {
                printf("Funcionario excluido.\n");
            }
        } else {
            fwrite(&u, sizeof(Usuario), 1, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");

    if (!encontrado) printf("Funcionario nao encontrado.\n");
}

void registrarVenda() {
    char nomeProduto[50];
    int qtd;
    float valorPago, troco;
    Produto p;
    int encontrado = 0;

    printf("Produto para venda: ");
    fgets(nomeProduto, sizeof(nomeProduto), stdin);
    nomeProduto[strcspn(nomeProduto, "\n")] = 0;

    FILE *fp = fopen("produtos.txt", "r+b");
    if (!fp) {
        printf("Erro ao abrir arquivo de produtos.\n");
        return;
    }

    while (fread(&p, sizeof(Produto), 1, fp)) {
        if (strcmp(p.nome, nomeProduto) == 0) {
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Produto nao encontrado.\n");
        fclose(fp);
        return;
    }

    char buffer[50];

    while (1) {
        printf("Quantidade vendida: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        if (sscanf(buffer, "%d", &qtd) == 1 && qtd > 0) break;
        printf("Entrada invalida. Tente novamente.\n");
    }

    if (qtd > p.quantidade) {
        printf("Estoque insuficiente.\n");
        fclose(fp);
        return;
    }

    float valorTotal = qtd * p.preco_venda;
    printf("Valor total: R$ %.2f\n", valorTotal);

    while (1) {
        printf("Valor pago: R$ ");
        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        if (sscanf(buffer, "%f", &valorPago) == 1 && valorPago >= valorTotal) break;
        printf("Valor pago insuficiente ou invalido. Tente novamente.\n");
    }

    troco = valorPago - valorTotal;
    printf("Troco: R$ %.2f\n", troco);

    p.quantidade -= qtd;
    fseek(fp, -sizeof(Produto), SEEK_CUR);
    fwrite(&p, sizeof(Produto), 1, fp);
    fclose(fp);

    registrarCaixa(valorTotal, 1);

    FILE *vfp = fopen("vendas.txt", "a");
    if (vfp) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char data[20];
        sprintf(data, "%02d/%02d/%04d %02d:%02d:%02d",
                tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
                tm.tm_hour, tm.tm_min, tm.tm_sec);

        fprintf(vfp, "%s | Produto: %s | Quantidade: %d | Valor total: R$ %.2f\n",
                data, nomeProduto, qtd, valorTotal);
        fclose(vfp);
    }

    printf("Venda registrada com sucesso!\n");
}

void controleDesperdicio() {
    char nome[50];
    int qtd;
    int achou = 0;
    Produto p;

    printf("Produto com defeito/quebrado: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;

    char buffer[50];

    while (1) {
        printf("Quantidade perdida: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        if (sscanf(buffer, "%d", &qtd) == 1 && qtd > 0) break;
        printf("Entrada invalida. Tente novamente.\n");
    }

    FILE *fp = fopen("produtos.txt", "r+b");
    if (!fp) {
        printf("Erro ao abrir arquivo de produtos.\n");
        return;
    }

    while (fread(&p, sizeof(Produto), 1, fp)) {
        if (strcmp(p.nome, nome) == 0) {
            achou = 1;
            if (p.quantidade < qtd) {
                printf("Quantidade informada excede o estoque disponivel.\n");
                fclose(fp);
                return;
            }
            p.quantidade -= qtd;
            fseek(fp, -sizeof(Produto), SEEK_CUR);
            fwrite(&p, sizeof(Produto), 1, fp);
            break;
        }
    }
    fclose(fp);

    if (!achou) {
        printf("Produto nao encontrado.\n");
        return;
    }

    FILE *dfp = fopen("desperdicio.txt", "a");
    if (dfp) {
        fprintf(dfp, "%s - %d unidades\n", nome, qtd);
        fclose(dfp);
        printf("Desperdicio registrado.\n");
    } else {
        printf("Erro ao registrar desperdicio.\n");
    }
}

void verificarValidade() {
    Produto p;
    int alertas = 0;
    FILE *fp = fopen("produtos.txt", "rb");
    if (!fp) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    printf("Produtos proximos da validade (menos de 7 dias):\n");
    while (fread(&p, sizeof(Produto), 1, fp)) {
        int dias = diasParaValidade(p.validade);
        if (dias >= 0 && dias <= 7) {
            printf("Produto: %s | Validade: %s | Dias restantes: %d | Sugestao promocao: 20%% off\n",
                   p.nome, p.validade, dias);
            alertas++;
        }
    }
    fclose(fp);
    if (!alertas) {
        printf("Nenhum produto esta proximo da validade.\n");
    }
}

void visualizarEstoque() {
    Produto p;
    FILE *fp = fopen("produtos.txt", "rb");
    if (!fp) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }
    printf("=== Estoque Atual ===\n");
    while (fread(&p, sizeof(Produto), 1, fp)) {
        printf("Produto: %s | Quantidade: %d | Validade: %s | Preco Venda: R$ %.2f\n",
               p.nome, p.quantidade, p.validade, p.preco_venda);
    }
    fclose(fp);
}

void visualizarClientes() {
    Cliente c;
    FILE *fp = fopen("clientes.txt", "rb");
    if (!fp) {
        printf("Nenhum cliente cadastrado.\n");
        return;
    }
    printf("=== Clientes Cadastrados ===\n");
    while (fread(&c, sizeof(Cliente), 1, fp)) {
        printf("Nome: %s | CPF: %s\n", c.nome, c.cpf);
    }
    fclose(fp);
}

void visualizarFuncionarios() {
    Usuario u;
    FILE *fp = fopen("usuarios.txt", "rb");
    if (!fp) {
        printf("Nenhum funcionario cadastrado.\n");
        return;
    }
    printf("=== Funcionarios Cadastrados ===\n");
    while (fread(&u, sizeof(Usuario), 1, fp)) {
        printf("Nome: %s | Login: %s | Nivel: %s\n", u.nome, u.login, u.nivel == 1 ? "ADM" : "FUNC");
    }
    fclose(fp);
}

void visualizarHistoricoVendas() {
    FILE *fp = fopen("vendas.txt", "r");
    if (!fp) {
        printf("Nenhuma venda registrada.\n");
        return;
    }
    printf("=== Historico de Vendas ===\n");
    char linha[200];
    while (fgets(linha, sizeof(linha), fp)) {
        printf("%s", linha);
    }
    fclose(fp);
}

void visualizarHistoricoDesperdicio() {
    FILE *fp = fopen("desperdicio.txt", "r");
    if (!fp) {
        printf("Nenhum desperdicio registrado.\n");
        return;
    }
    printf("=== Historico de Desperdicio ===\n");
    char linha[200];
    while (fgets(linha, sizeof(linha), fp)) {
        printf("%s", linha);
    }
    fclose(fp);
}

// === MENUS ===

void menuADM() {
    int opc;
    char buffer[10];
    do {
        printf("\n--- MENU ADMINISTRADOR ---\n");
        printf("1. Cadastrar Produto\n");
        printf("2. Atualizar Produto\n");
        printf("3. Excluir Produto\n");
        printf("4. Registrar Venda\n");
        printf("5. Cadastrar Cliente\n");
        printf("6. Excluir Cliente\n");
        printf("7. Cadastrar Funcionario\n");
        printf("8. Excluir Funcionario\n");
        printf("9. Controle de Desperdicio\n");
        printf("10. Verificar Validade\n");
        printf("11. Visualizar Estoque\n");
        printf("12. Visualizar Clientes\n");
        printf("13. Visualizar Funcionarios\n");
        printf("14. Historico de Vendas\n");
        printf("15. Historico de Desperdicio\n");
        printf("0. Sair\n> ");

        if (!fgets(buffer, sizeof(buffer), stdin)) continue;

        if (sscanf(buffer, "%d", &opc) != 1) {
            printf("Opcao invalida. Tente novamente.\n");
            continue;
        }

        switch(opc) {
            case 1: cadastrarProduto(); break;
            case 2: atualizarProduto(); break;
            case 3: excluirProduto(); break;
            case 4: registrarVenda(); break;
            case 5: cadastrarCliente(); break;
            case 6: excluirCliente(); break;
            case 7: cadastrarFuncionario(); break;
            case 8: excluirFuncionario(); break;
            case 9: controleDesperdicio(); break;
            case 10: verificarValidade(); break;
            case 11: visualizarEstoque(); break;
            case 12: visualizarClientes(); break;
            case 13: visualizarFuncionarios(); break;
            case 14: visualizarHistoricoVendas(); break;
            case 15: visualizarHistoricoDesperdicio(); break;
            case 0: printf("Saindo...\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

void menuFuncionario() {
    int opc;
    char buffer[10];
    do {
        printf("\n--- MENU FUNCIONARIO ---\n");
        printf("1. Cadastrar Produto\n");
        printf("2. Atualizar Produto\n");
        printf("3. Registrar Venda\n");
        printf("4. Cadastrar Cliente\n");
        printf("11. Visualizar Estoque\n");
        printf("12. Visualizar Clientes\n");
        printf("14. Historico de Vendas\n");
        printf("0. Sair\n> ");

        if (!fgets(buffer, sizeof(buffer), stdin)) continue;

        if (sscanf(buffer, "%d", &opc) != 1) {
            printf("Opcao invalida. Tente novamente.\n");
            continue;
        }

        switch(opc) {
            case 1: cadastrarProduto(); break;
            case 2: atualizarProduto(); break;
            case 3: registrarVenda(); break;
            case 4: cadastrarCliente(); break;
            case 11: visualizarEstoque(); break;
            case 12: visualizarClientes(); break;
            case 14: visualizarHistoricoVendas(); break;
            case 0: printf("Saindo...\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

void loginSistema() {
    char login[20], senha[20];
    int nivel;

    printf("Login: "); fgets(login, sizeof(login), stdin);
    login[strcspn(login, "\n")] = 0;
    printf("Senha: "); fgets(senha, sizeof(senha), stdin);
    senha[strcspn(senha, "\n")] = 0;

    if (validarLogin(login, senha, &nivel)) {
        if (nivel == 1) menuADM();
        else menuFuncionario();
    } else {
        printf("Login ou senha incorretos.\n");
    }
}

int main() {
    srand(time(NULL));
    loginSistema();
    return 0;
}
