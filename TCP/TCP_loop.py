#programa que recebe uma mensagem pela rede com socket TCP
import random
import socket

#Endereco IP do receptor e porta
HOST = '0.0.0.0'     # IP para onde será enviada a mensagem
PORT = 8000      # Porta para ligação com o socket
#Criação do socket
tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# O receptor sera um par de endereço e porta
origem = (HOST, PORT)
#Realiza a ligação do socket
tcp.bind(origem)
#Coloca o socket para escutar (aguardar) conexões
tcp.listen(1)
print("Servidor escutando na porta", PORT)
while True:
    #Aguarda uma conexão
    conexao, cliente = tcp.accept()
    print("Receptor conectado por", cliente)
    mensagem = conexao.recv(1024)
    print("Emissor:", cliente, "enviou a mensagem:", mensagem.decode())

    # Responde com número de 0 a 1000
    num = random.randint(0, 1000)
    mensagem = "Porto Alegre " + str(num)

    #Envia dados para o socket
    conexao.send(mensagem.encode())
    print("Mensagem respondida: ", mensagem)

    print("Encerrando conexao com o emissor", cliente)
    print("")
    #encerra a conexão
    conexao.close()