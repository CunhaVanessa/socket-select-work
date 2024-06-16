#include <stdio.h>         // Biblioteca padrão de entrada e saída
#include <stdlib.h>        // Biblioteca padrão de utilitários
#include <sys/select.h>    // Biblioteca para uso da função select
#include <sys/socket.h>    // Biblioteca para uso de sockets
#include <arpa/inet.h>     // Biblioteca para manipulação de endereços IP
#include <unistd.h>        // Biblioteca para uso de funções POSIX, como close

#define PORT 8080          // Definição da porta na qual o servidor vai escutar

int main() {
    int server_fd, new_socket, max_sd, activity, valread;
    int client_socket[30] = {0};  // Array para armazenar descritores de socket dos clientes
    int max_clients = 30;         // Número máximo de clientes
    int sd;                       // Descritor de socket temporário
    struct sockaddr_in address;   // Estrutura para armazenar endereço do servidor
    fd_set readfds;               // Conjunto de descritores para leitura
    char buffer[1025];            // Buffer para armazenamento de dados recebidos

    // Criação do socket do servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;         // Configuração do tipo de endereço (IPv4)
    address.sin_addr.s_addr = INADDR_ANY; // Aceitar conexões de qualquer endereço IP
    address.sin_port = htons(PORT);       // Configuração da porta (convertida para rede)

    // Vinculação do socket do servidor ao endereço e porta especificados
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    // Configuração do socket do servidor para escutar conexões, com backlog de 3
    listen(server_fd, 3);

    while (1) {
        FD_ZERO(&readfds);         // Inicialização do conjunto de descritores
        FD_SET(server_fd, &readfds); // Adiciona o descritor do servidor ao conjunto
        max_sd = server_fd;        // Inicializa max_sd com o descritor do servidor

        // Adiciona descritores de cliente ao conjunto de descritores
        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i]; // Obtém o descritor de cliente
            if (sd > 0) FD_SET(sd, &readfds); // Adiciona ao conjunto se válido
            if (sd > max_sd) max_sd = sd;     // Atualiza max_sd se necessário
        }

        // Chama a função select para monitorar descritores de leitura
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // Verifica se há uma nova conexão de entrada
        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&address);
            for (int i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {    // Adiciona novo socket ao array de clientes
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        // Verifica se há dados disponíveis para leitura em sockets de cliente
        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                valread = read(sd, buffer, 1024); // Lê dados do socket do cliente
                buffer[valread] = '\0';           // Adiciona caractere nulo ao final da string
                send(sd, buffer, strlen(buffer), 0); // Envia os dados de volta ao cliente
            }
        }
    }

    return 0;
}
